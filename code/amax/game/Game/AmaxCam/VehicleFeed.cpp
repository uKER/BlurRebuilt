
#include "stdafx.h"

#include <camera2/stock/BlendCamera.h>
#include <../library/kanzen/camera/HollywoodVehicleCamera.h>

#include "AmaxChaseCamera.h"
#include "BoltedCamera.h"
#include "CameraSystem.h"
#include "CameraKeyFrame.h"
#include "Game/GameControls/GameControls.h"
#include "Game/GameActors/Vehicles/VehicleActor.h"
#include "GameLogic/Profile/GameProfileManager.h"
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"
#include "Helper.h"
#include "VehicleFeed.h"
#include "VehicleFeedParam.h"
#include "Vehicle/VehicleModel.h"
#include "VFCamera.h"
#include <Game\Game.h>

namespace Game
{
	extern bool g_SorceryRecordsModified;
}

DEBUG_ENUM(AmaxCam::E_VFCamera)
DE_OPTION(AmaxCam::E_VFCamera_Hollywood,		"Hollywood")
DE_OPTION(AmaxCam::E_VFCamera_Bumper,			"Bumper")
END_DEBUG_ENUM

namespace AmaxCam
{

using namespace Game::Actors;
using namespace Maths;

static Debug::Bool LookAround("Camera2.VehicleFeed.LookAround", false);
static Debug::Float g_futureFeedPredictionTime("Camera2.VehicleFeed.FutureFeedPredictionTime", 0.5f);

Debug::Bool g_debugEnableLookAroundInBumper("Camera2.VehicleFeed.EnableLookAroundInBumper", true);
Debug::Float g_debugGlobalBumperCamHeightOffset("Camera2.VehicleFeed.BumperHeightOffset", 0.1f, Debug::Range(-0.1f, 0.5f), Debug::Step(0.001f));

class C_ShutterCurve
{
public:
	static void SorceryCallback(XtEventType evt, XtRecord*, C_ShutterCurve* self, XtCallbackContext*);

	XtArray<C_CameraKeyFrame> m_Curve;
};
static C_ShutterCurve* g_ShutterCurve = 0;


C_VehicleFeed::C_VehicleFeed()
{
	m_Choice = 0;
	m_BlendCamera = 0;
	m_Chase = 0;
	m_Bumper = 0;
	m_pHollywoodCamera = 0;
	m_CurrentVehicleCamera = (E_VFCamera)0;
	m_LookX = m_LookY = m_LookAngle = 0.f;
	m_Future = false;
	m_PadIndex = 0;
	m_VehicleParam = 0;
	m_ChangeViewAllowed = true;
	m_Shown = false;
}

C_VehicleFeed::~C_VehicleFeed()
{
	Exit();
}

void C_VehicleFeed::Init()
{
	if (m_Future)
	{	
		// Old chase camera (just used for future camera):
		m_Chase = new C_AmaxChaseCamera;
		if (!m_Chase)
			return;
		m_Cameras.PushBack(m_Chase);
	}
	else
	{
		// Kanzen chase camera:
		m_pHollywoodCamera = new Kanzen::C_HollywoodVehicleCamera;
		if (!m_pHollywoodCamera)
			return;
		m_pHollywoodCamera->SetRaytrace(true);
		m_pHollywoodCamera->m_debugIndex = 0;
		m_Cameras.PushBack(m_pHollywoodCamera);
	}
	
	// Bumper camera:
	m_Bumper = new C_BoltedCamera;
	m_Bumper->SetName("Bumper");
	if (!m_Bumper)
		return;
	m_Cameras.PushBack(m_Bumper);

	m_Choice = 0;
	m_CurrentVehicleCamera = (E_VFCamera)0;

	m_BlendCamera = new Cam2::C_BlendCamera;
	if (!m_BlendCamera)
		return;
	
	if (m_Future)
		m_BlendCamera->SetFeedSlot(E_VFCamera_Hollywood, m_Chase);
	else
		m_BlendCamera->SetFeedSlot(E_VFCamera_Hollywood, m_pHollywoodCamera);

	m_BlendCamera->SetFeedSlot(E_VFCamera_Bumper, m_Bumper);
	
	m_BlendCamera->BlendTo(m_Choice, 0.f);

	m_smoothAcceleration = HVec3(0.f, 0.f, 0.f);
	m_smoothVelocity = HVec3(0.f, 0.f, 0.f); 
	m_smoothAngularVelocity = HVec3(0.f, 0.f, 0.f);
}

void C_VehicleFeed::Exit()
{
	if (m_BlendCamera)
	{
		for (S32 i = 0; i < m_Cameras.Size(); i++)
			m_BlendCamera->SetFeedSlot(i, 0);
	}
	
	for (S32 i = 0; i < m_Cameras.Size(); i++)
		m_Cameras[i] = 0;

	SAFE_DELETE_PTR(m_Bumper)
	SAFE_DELETE_PTR(m_Chase)
	SAFE_DELETE_PTR(m_pHollywoodCamera)
}

void C_VehicleFeed::SetHandle(const Actor::C_ActorHandle& handle)
{
	m_Handle = handle;
	if (m_pHollywoodCamera)
		m_pHollywoodCamera->SetHandle(m_Handle);
	if (m_Chase)
		m_Chase->SetHandle(m_Handle);
	if (m_Bumper)
		m_Bumper->SetHandle(m_Handle);
	// Store vehicle params:	
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (vehicleActor == 0)
		return;

	m_VehicleParam = vehicleActor->GetCameraParam();
	UpdateParam();
}

void C_VehicleFeed::Reset()
{
	// Restore player's camera setting in splitscreen:
	Game::C_GameProfile* profile = GetGameProfile();
	if (profile)
		ChangeCamera(profile->mOptions.mCameraType);

	if (m_pHollywoodCamera)
		m_pHollywoodCamera->VehicleReset();

	if (m_Chase)
		m_Chase->VehicleReset();
}

void C_VehicleFeed::AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov)
{
	if (m_pHollywoodCamera)
		m_pHollywoodCamera->AddOffset(offset, offsetrot, fov);

	if (m_Chase)
		m_Chase->AddOffset(offset, offsetrot, fov);

	if (m_Bumper)
		m_Bumper->AddOffset(offset, offsetrot, fov);
}

void C_VehicleFeed::CameraChangeUpdate(F32 period)
{
	// Cycle between the different camera types:
	bool changeCameraButton = false;

	if (m_ChangeViewAllowed && m_Shown)
	{
		Input::C_GameInput* input = GameInput();
		if ( input &&
			 input->GetButtonPressedDebounced(Game::E_Button_ChangeView) &&
			!input->GetButtonPressedDebounced(Framework::E_Button_CameraMenu))
		{
			changeCameraButton = true;
		}
	}
	
	if (changeCameraButton)
	{
		m_Choice++;

		if (m_Choice >= E_VFCamera_Count)
			m_Choice = 0;
		
		F32 blendTime = 0.5f;
		if (m_Choice == E_VFCamera_Bumper)
			blendTime = 0.0f;
		m_BlendCamera->BlendTo(m_Choice, blendTime);

		// Keep track of each players camera settings in splitscreen
		Game::C_GameProfile* profile = GetGameProfile();
		if (profile)
			profile->mOptions.mCameraType = (U8)m_Choice;
	}
}

void C_VehicleFeed::Update(F32 period)
{
	BIZ_ASSERT(m_BlendCamera);
	BIZ_ASSERT(m_Bumper);
	
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (!vehicleActor)
		return;

	if (Game::g_SorceryRecordsModified)
	{
		m_VehicleParam = vehicleActor->GetCameraParam();
		UpdateParam();
	}

	// Get the camera object (the vehicle) and set it in all of the cameras:
	UpdateCameraObject(vehicleActor);

	m_LookX = m_LookY = m_LookAngle = 0.f;

	Input::C_GameInput* input = GameInput();
	if (input)
	{		
#ifdef PLATFORM_PC
		// [Blur2PC] retail Blur PC (0x00413b20): the rear view button looks straight back
		if (input->GetButtonPressed( Game::E_Button_RearView ))
		{
			m_LookX = 0.f;
			m_LookY = -1.f;
		}
		else
#endif
		{
		m_LookX = input->GetAxisSignedValue( Framework::E_Axis_FlyRotateX );
		m_LookY = input->GetAxisSignedValue( Framework::E_Axis_FlyRotateY );
		}
	}

	// Also calculate look angle:
	F32 mag = m_LookX * m_LookX + m_LookY * m_LookY; 
	if (mag >= 0.2f * 0.2f)
		m_LookAngle = atan2(m_LookX, m_LookY); // Right stick being used to look around

	if (m_pHollywoodCamera)
		m_pHollywoodCamera->SetLookAround(m_LookX, m_LookY);

	if (m_Chase)
		m_Chase->SetLookAround(m_LookX, m_LookY);

	if (g_debugEnableLookAroundInBumper)
		m_Bumper->SetLookAngle(m_LookAngle);

	// Update all of the cameras:
	if (m_pHollywoodCamera)
		m_pHollywoodCamera->Update(period);
	if (m_Chase)
		m_Chase->Update(period);

	m_Bumper->Update(period);	

	// Lerp Shutter Angle based on speed:
	if (m_pHollywoodCamera)
		LerpShutterAngle(m_pHollywoodCamera, g_ShutterCurve);

	LerpShutterAngle(m_Bumper, g_ShutterCurve);
}

void C_VehicleFeed::Update2(F32 period)
{
	// Correct for any deferred raytraces:
	if (m_Chase)
		m_Chase->Update2();

	// Update blend camera which blends between the game cameras:	
	UpdateBlendCamera(period);
}

// Get the camera object (the vehicle) and set it in all of the cameras:
void C_VehicleFeed::UpdateCameraObject(const C_VehicleActor* vehicleActor)
{
	Cam2::C_CamObj object;

	if (!m_Future)
	{
		object = VehicleActorToCameraObject(vehicleActor);
	}
	else
	{
		// Project ahead the car's position for future camera:
		Vehicle::C_VehicleModel* model = vehicleActor->GetVehicleModel();
		Vehicle::C_ModelInput* modelInput = model ? model->GetModelInput() : 0;
		
		HVec3 angularVelocity;

		if (modelInput)
		{
			m_smoothVelocity = m_smoothVelocity * 0.9f + modelInput->m_vVelocity * 0.1f;
			m_smoothAcceleration = m_smoothAcceleration * 0.9f + modelInput->m_vAcceleration * 0.1f;
			m_smoothAngularVelocity = m_smoothAngularVelocity * 0.9f + modelInput->m_vAngularVelocity * 0.1f;
		}

		HF32 t = (float)g_futureFeedPredictionTime;
		
		object.position = vehicleActor->GetMatrix().AW() + m_smoothVelocity * t + m_smoothAcceleration * t * t * 0.5f;
		float a = m_smoothAngularVelocity.Length();
		C_Quaternion r(m_smoothAngularVelocity, -a);
		C_Quaternion q(vehicleActor->GetMatrix().Rotation());
		object.orient = q * r;
	}

	// Set the focus object for all of the cameras:
	for (S32 i = 0; i < m_Cameras.Size(); i++)
		m_Cameras[i]->SetObject(object);

}

void C_VehicleFeed::UpdateBlendCamera(F32 period)
{
	m_BlendCamera->Update(period);
}

void C_VehicleFeed::ChangeCamera(S32 index, const F32 blendPeriod) // Blend value defaults to 0 to immediately switch
{
	m_Choice = index;
	if (m_BlendCamera)
		m_BlendCamera->BlendTo(m_Choice, blendPeriod);	
}

Cam2::C_Camera* C_VehicleFeed::GetCamera(E_VFCamera p_Type)
{
	switch (p_Type)
	{
	case E_VFCamera_Hollywood:
		return m_pHollywoodCamera;
	case E_VFCamera_Bumper:
		return m_Bumper;
	}
	return NULL;
}

void C_VehicleFeed::UpdateParam()
{
	if (!m_VehicleParam)
		return;

	if (m_Chase)
	{
		const C_AmaxChaseParam* param = m_VehicleParam->m_Chase.Object();
		if (param)
			m_Chase->SetAmaxParam(*param);
		else
			m_Chase->SetAmaxParam(C_AmaxChaseParam());
	}
	
	if (m_Bumper)
	{
		m_Bumper->SetParam(m_VehicleParam->m_Bonnet); //[CWright] Switch bumper/bonnet cam Sorcery settings around
		m_Bumper->SetOffset(MathsStorage::Float3(0.0f, (float)g_debugGlobalBumperCamHeightOffset, 0.0f));
	}
}

void C_VehicleFeed::DebugRender() const
{
	for (S32 i = 0; i < m_Cameras.Size(); i++)
		m_Cameras[i]->DebugRender(RGBA(0xff808000), false);

	if (m_BlendCamera)
		m_BlendCamera->DebugRender(RGBA(0xff00ff00));
}

const Cam2::C_Camera& C_VehicleFeed::Live() const
{
	if (m_BlendCamera)
		return *m_BlendCamera;
	else
		return Cam2::NullCamera;
}

//Hack to keep Audio camera mapping the same
S32 C_VehicleFeed::GetAudioMapping() const
{
	switch (m_Choice)
	{
	case E_VFCamera_Hollywood:
		return 0;
	case E_VFCamera_Bumper:
		return 4;
	}
	return 0;
}

void C_VehicleFeed::SetMotionBlur(const HVec3& motion)
{
	if (m_BlendCamera)
	{
		m_BlendCamera->SetPrevPosition(m_BlendCamera->Position() - motion);
		m_BlendCamera->SetPrevShaken(m_BlendCamera->Shaken());
	}
}

// Get the game input device for this VehicleFeed, return 0 if the DebugFeed is using the pad
// or is the Pad index is -1:
Input::C_GameInput* C_VehicleFeed::GameInput()
{
	// Check for the "paused" menu in a network game:
	RaceLogicCore* rlc = GetRaceLogicCore();
	if (rlc && rlc->IsPaused())
		return 0;

	if (m_PadIndex >= 0 && m_PadIndex < 4 && !CameraSystem().PadUsedForDebug(m_PadIndex))
		return Input::GetGameInput(m_PadIndex);
	else
		return 0;
}

Game::C_GameProfile* C_VehicleFeed::GetGameProfile()
{
	return Game::C_GameProfileManager::Instance().GetProfile(m_PadIndex);
}

const Cam2::C_Camera* C_VehicleFeed::ChaseCamera() const
{
	return m_pHollywoodCamera;
}

HMat43 C_VehicleFeed::LastKnownSafeMatrix() const
{
	const C_VehicleActor* vehicle = GetVehicleActor(m_Handle);
	if (!vehicle)
		return HMat43();
	
	return vehicle->LastKnownSafeMatrix();
}

HVec3 C_VehicleFeed::VehicleVelocity() const
{
	const C_VehicleActor* vehicle = GetVehicleActor(m_Handle);

	HVec3 veloc;
	if (vehicle)
		veloc = AmaxCam::GetVehicleVelocity(vehicle);
	return veloc;
}

void C_VehicleFeed::SetShown(bool shown)
{
	m_Shown = shown;
}

void C_VehicleFeed::LerpShutterAngle(Cam2::C_Camera* camera, const C_ShutterCurve* shutterCurve)
{
	if (!camera || !shutterCurve)
		return;
	
	HVec3 velocity = camera->Velocity();
	F32 speed = velocity.Length() * 2.23693629f;

	F32 shutterAngle = C_CameraKeyFrame::Evaluate(shutterCurve->m_Curve, speed);

	// Constant shutter time independent of frame outs:
	F32 camPeriod = Maths::Max(camera->Period(), 1.0f / 30.0f);
	camera->SetShutterAngle(shutterAngle / 30.f / camPeriod);
}

C_VehicleFeedParam::C_VehicleFeedParam()
{
	// Set some defaults:
	m_Bonnet.m_Position = XtVec3(0.f, 0.91f, 0.72f);
}

void C_ShutterCurve::SorceryCallback(XtEventType evt, XtRecord*, C_ShutterCurve* self, XtCallbackContext*)
{
	if (evt == kXtEtConstruct)
		g_ShutterCurve = self;
	
	if (evt == kXtEtDestruct)
		g_ShutterCurve = 0;
}

} // End of namespace

XT_TYPE(AmaxCam::C_VehicleFeedParam, "AmaxVehicleFeedParam")
	XT_FIELD(m_Chase,  "Chase")
	XT_FIELD(m_Bonnet, "Bonnet")
XT_END_TYPE

XT_TYPE(AmaxCam::C_ShutterCurve, "AmaxShutterCurve")
	XT_CALLBACK(AmaxCam::C_ShutterCurve::SorceryCallback)
	XT_FIELD(m_Curve, "Curve")
XT_END_TYPE
