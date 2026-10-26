
#include "stdafx.h"
#include "CameraSystem.h"

#include <actor_system/ActorDeclare.h>
#include <Camera2/CameraInput.h>
#include <camera2/EffectInfo.h>
#include <camera2/capture/ScreenToFile.h>
#include <camera2/debug/DebugTools.h>
#include <camera2/shake/ShakeWorld.h>
#include <camera2/stock/AnimCamera.h>
#include <camera2/stock/DebugFeed.h>
#include <camera2/stock/TvFeed.h>
#include <camera2/tv/CameraScene.h>

#include <Effect/PostProcess/DoFFgBg.h>
#include <framework/application/ApplicationBase.h>
#include <framework/application/CaptureService.h>
#include <framework/application/UserControls.h>
#include <Graphics/HDR.h>
#include <SceneGraph/SceneGraph.h>
#include <system/debug/I_DebugRender.h>
#include <system/debug/BuildStamp.h>

#include "AmaxChaseCamera.h"
#include "CameraSwitcher.h"
#include "CubeCapture.h"
#include "DirectorActor.h"
#include "FatalCamera.h"
#include "SetPieceCamera.h"
#include "GameApp.h"
#include "Game/GameActors/Vehicles/VehicleActor.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"
#include "Game/GameWorld/GameWorld.h"
#include "Game/GameWorld/PostProcessLayer.h"
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"
#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic/Pickups/PickupsManager.h"
#include "GameLogic/Pickups/PickupBase.h"
#include "Helper.h"
#include "HOGResetCamera.h"
#include "PhotoCamera2.h"
#include "PassingShake.h"
#include "PhysicsChecks.h"
#include "RayGroup.h"
#include "VehicleFeed.h"
#include "ViewportFovScale.h"
#include "ViewportScreenOffset.h"

#include "Lod/LodManager.h"
#include "SplashCamera/SplashCamera.h"
#include "game/GameWorld/ViewType.h"

namespace Lod
{
	extern Debug::Bool g_LodEnabled;
}

// Amax Camera system

namespace AmaxCam
{
using namespace Game::Actors;
using namespace Maths;
using namespace Util;

#ifndef CFG_GOLD
static Debug::Bool DrawCameras("Camera2.DrawCameras", false);
static Debug::Bool g_ShowInfo("Camera2.ShowInfo", false);
static Debug::Bool g_ExtraInfo("Camera2.ExtraInfo", false);
static Debug::Bool g_PfxInfo("Camera2.PfxInfo", false);
static Debug::Int g_FocusCar("Camera2.SetFocusCar", -1, Debug::Range(-1, 19));
static Debug::Int g_FocusPickup("Camera2.FocusPickup", -1, Debug::Range(-1, 3));
static Debug::Bool g_UseFocusObjectRotation("Camera2.UseFocusObjectRotation", true);
#endif
Debug::Bool LookAtEnable("Camera2.LookAt.Enable", false);
Debug::Int LookAtIndex  ("Camera2.LookAt.Target", 0, Debug::Range(0, 19));
Debug::Float LookFov    ("Camera2.LookAt.Fov", 30.f, Debug::Range(0.f, 90.f));
Debug::Float LookOffsetX("Camera2.LookAt.OffsetX", 0.f, Debug::Range(-4.f, 4.f));
Debug::Float LookOffsetY("Camera2.LookAt.OffsetY", 0.f, Debug::Range(-4.f, 4.f));
Debug::Float LookOffsetZ("Camera2.LookAt.OffsetZ", 0.f, Debug::Range(-4.f, 4.f));
#ifndef CFG_GOLD
static Debug::Bool  g_SpotLightEnable("Camera2.SpotLight.Enable", false);
static Debug::Bool  g_SpotLightUpdate("Camera2.SpotLight.Update", true);
static Debug::Float g_SpotLightWidth ("Camera2.SpotLight.Width", 20.f, Debug::Range(0.f, 90.f));
static Debug::Float g_SpotLightIntensity("Camera2.SpotLight.Intensity", 1.f, Debug::Range(0.f, 4.f));
static Debug::String g_AnimName("Camera2.Anim.Name", "");
static Debug::Bool g_AnimPlay("Camera2.Anim.Play", false);

static Debug::Bool g_PosLogRecord("Camera2.DebugTools.PositionLog.Record", false);
static Debug::Bool g_PosLogRecordWhenCapturing("Camera2.DebugTools.PositionLog.RecordWhenCapturing", false);
#endif


C_CameraSystem::C_CameraSystem()
{
	m_DebugTools		= new Cam2::C_DebugTools;
	m_DebugTools->Init();

	Switcher().Init();
	
	m_FatalCamera		= 0;
	m_pSetPieceCamera	= 0;
	m_PhotoCamera2		= 0;
	m_Lod				= false;
#ifndef CFG_GOLD
	m_CubeCapture		= new C_CubeCapture;
#endif

	m_Viewport.Resize(4);
	g_RayGroup.Init();
	m_ViewConfig = 1;
}

void C_CameraSystem::Exit()
{
#ifndef CFG_GOLD
	SAFE_DELETE_PTR(m_CubeCapture)
#endif

	g_RayGroup.Exit();
	m_Viewport.Clear();

	Switcher().Exit();
	SAFE_DELETE_PTR(m_DebugTools)

	SAFE_DELETE_PTR(m_PhotoCamera2)
}

C_CameraSystem::~C_CameraSystem()
{
	Exit();
}

C_CameraSystem& CameraSystem()
{
	static C_CameraSystem s_instance;
	return s_instance;
}

bool C_CameraSystem::IsOkay()
{
	bool pointers = m_DebugTools != 0;
	BIZ_ASSERT(pointers);
	return pointers;
}

void C_CameraSystem::RealUpdate()
{
	if (!IsOkay())
		return;

	PERF_EVENT("CameraSystem.Update");

	F32 period = C_GameApp::GetInstance()->GetDebugFrameTime();

#ifndef CFG_GOLD
	Switcher().Update(period);
	ProcessInput();
	GetCurrentFocusObject();
	UpdateDebugTools(period);
	DoLookAt();
#endif

	if (m_PhotoCamera2)
	{
		m_PhotoCamera2->Update(period);
		m_PhotoCamera2->SetPrevPosition(m_PhotoCamera2->Position() - m_PhotoCamera2->MotionBlur());
	}

	CheckVehicleFeeds();
	CopyDepthOfFieldInfo();

#ifndef CFG_GOLD
	UpdateSpotlight();
	m_CubeCapture->RealUpdate();
	PassingShake().Update();

#ifdef PLATFORM_PS3
	Input::C_GameInput* input = Input::GetGameInput(0);

	if (input->GetButtonPressed(Framework::E_Button_Debug_ButtonRightStickClick) &&
	    input->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonX))
	{
		Cam2::ScreenToNextTga();
	}
#endif
#endif
}

void C_CameraSystem::CheckVehicleFeeds()
{
	int i = 0;	
	int views = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();

	// If there are any vehicle feeds shown in viewports higher than the splitscreen view count
	// stop showing them (this solves them lingering around after a splitscreen race)
	for (i = 0; i < m_Viewport.Size(); i++)
	{
		C_Viewport& vp = m_Viewport[i];
		if (vp.m_CameraType == E_GameCam_VehicleFeed && i >= views)
			GameFeedShow(i, 0);

		// Check if the vehiclefeed for this viewport is being shown now:
		bool shown = vp.m_CameraType == E_GameCam_VehicleFeed;
		if (vp.m_VehicleFeed)
			vp.m_VehicleFeed->SetShown(shown);
	}
}

void C_CameraSystem::CameraChangeUpdate(F32 period)
{
	for (S32 i = 0; i < m_Viewport.Size(); i++)
	{
		C_Viewport& vp = m_Viewport[i];
		vp.m_VehicleFeed->CameraChangeUpdate(period);
		
		// Sync the future feed camera type with the normal one:
		S32 normalType = vp.m_VehicleFeed->CurrentType();
		S32 futureType = vp.m_VehicleFuture->CurrentType();
		if (futureType != normalType)
			vp.m_VehicleFuture->ChangeCamera(normalType, 0.0f);
	}
}

void C_CameraSystem::WorldUpdate(F32 period)
{
	if (!IsOkay())
		return;

	// Start a new frame of raytraces:
	g_RayGroup.StartFrame();

	Cam2::C_ShakeWorld::Instance().Update(period);

	C_ActorHandle handle = FocusHandle();

	for (S32 i = 0; i < m_Viewport.Size(); i++)
	{
		C_ActorHandle handle = FocusHandle(i);

		C_Viewport& vp = m_Viewport[i];
		vp.m_VehicleFeed->SetHandle(handle);
		vp.m_VehicleFuture->SetHandle(handle);

		vp.m_VehicleFeed->Update(period);
		vp.m_VehicleFuture->Update(period);

		if (vp.m_HOGResetCamera)
			vp.m_HOGResetCamera->Update(period);

		Maths::HVec2 screenOffset = ScreenOffset(i);
		DebugFeed(i).SetScreenOffset( screenOffset.X(), screenOffset.Y());
	}

	if (m_FatalCamera)
		m_FatalCamera->Update(period);

	if (m_pSetPieceCamera)
		m_pSetPieceCamera->Update(period);

	UpdateWreckedEffect();

#ifndef CFG_GOLD
	if (g_AnimPlay)
	{
		Cam2::CameraScene().Load("cams.cse");
		Cam2::CameraScene().PlayAnimation(g_AnimName.Value());
		GameFeedShow(0, Cam2::CameraScene().Camera());
		g_AnimPlay = false;
	}
#endif
	
	Cam2::CameraScene().Update(period);
}

void C_CameraSystem::WorldUpdate2(F32 period)
{
	// Flush the deferred raytraces, so they are ready to read:
	g_RayGroup.Flush();

	// Now correct any cameras which need correcting:
	for (S32 i = 0; i < m_Viewport.Size(); i++)
	{
		C_Viewport& vp = m_Viewport[i];
		vp.m_VehicleFeed->Update2(period);
		vp.m_VehicleFuture->Update2(period);
	}
}

Cam2::C_DebugFeed& C_CameraSystem::DebugFeed(S32 viewport)
{
	S32 max = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	if (viewport < 0 || viewport >= max)
		viewport = 0;

	return Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(viewport);
}

const Cam2::C_DebugFeed& C_CameraSystem::DebugFeed(S32 viewport) const
{
	S32 max = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	if (viewport < 0 || viewport >= max)
		viewport = 0;

	return Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(viewport);
}

void C_CameraSystem::DebugFeedShow(S32 debugViewport, S32 debugType, S32 gameViewport)
{
	if (!IsOkay())
		return;

	S32 max = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	if (debugViewport < 0 || debugViewport >= max)
		return;
	
	// Find DebugFeed:
	Cam2::C_DebugFeed& debugFeed = DebugFeed(debugViewport);

	// Find game viewport:
	C_Viewport* vp = 0;
	if (gameViewport >= 0 && gameViewport < m_Viewport.Size())
		vp = &m_Viewport[gameViewport];

	switch (debugType)
	{
		default:
			debugFeed.SetDebugType(debugType);
		break;
		case E_DebugCam_GameFeed:
			if (vp)
				debugFeed.ShowFeed(vp->m_GameFeed);
		break;
		case E_DebugCam_AnimCamera:
			debugFeed.ShowFeed(m_DebugTools->AnimCamera());
		break;
		case E_DebugCam_FatalCamera:
			debugFeed.ShowFeed(m_FatalCamera);
		break;
		case E_DebugCam_HOGResetCamera:
			if (vp)
				debugFeed.ShowFeed(vp->m_HOGResetCamera);
		break;
		case E_DebugCam_FutureFeed:
			if (vp)
				debugFeed.ShowFeed(vp->m_FutureFeed);
		break;
		case E_DebugCam_GravGun:
			debugFeed.SetDebugType(Cam2::E_FlyCamera);
		break;
		case E_DebugCam_OrbitCamera:
			debugFeed.SetDebugType(Cam2::E_OrbitCamera);
		break;
		case E_DebugCam_TvFeed:
			debugFeed.ShowFeed(m_DebugTools->TvFeed());
		break;
		case E_DebugCam_VehicleFeed:
			if (vp)
				debugFeed.ShowFeed(vp->m_VehicleFeed);
		break;
	}
}

// Show in the gamefeed one of the E_GameCam cameras:
void C_CameraSystem::GameFeedShow(S32 viewport, E_GameCam cameraType, int whichFeed)
{
	if (viewport < 0 || viewport >= m_Viewport.Size())
		return;

	C_Viewport& vp = m_Viewport[viewport];

	const Cam2::I_Feed* feed = 0, *futureFeed = 0;

	switch (cameraType)
	{
		case E_GameCam_VehicleFeed:
			feed = vp.m_VehicleFeed;
			futureFeed = vp.m_VehicleFuture; // Automatically show the vehicle future feed:
		break;
	}

	if (!futureFeed)
		futureFeed = feed;

	vp.m_CameraType = cameraType;
	
	// Show the camera feed in either the current feed, future feed, or both feeds:
	if (whichFeed == E_WhichFeed_Both || whichFeed == E_WhichFeed_Current)
	{
		if (vp.m_GameFeed)
			vp.m_GameFeed->Show(feed);
	}
	
	if (whichFeed == E_WhichFeed_Both || whichFeed == E_WhichFeed_Future)
	{
		if (vp.m_FutureFeed)
			vp.m_FutureFeed->Show(futureFeed);
	}	
}

// Show in the gamefeed a custom feed:
void C_CameraSystem::GameFeedShow(S32 viewport, const Cam2::I_Feed* feed, int whichFeed)
{
	if (viewport < 0 || viewport >= m_Viewport.Size())
		return;

	C_Viewport& vp = m_Viewport[viewport];

	// Show the camera feed in either the current feed, future feed, or both feeds:
	if (whichFeed == E_WhichFeed_Both || whichFeed == E_WhichFeed_Current)
	{
		if (vp.m_GameFeed)
			vp.m_GameFeed->Show(feed);

		vp.m_CameraType = E_GameCam_None;
	}
	
	if (whichFeed == E_WhichFeed_Both || whichFeed == E_WhichFeed_Future)
	{
		if (vp.m_FutureFeed)
			vp.m_FutureFeed->Show(feed);
	}
}

// Set gamefeed to 0, but only if feed is being shown
void C_CameraSystem::GameFeedRemove(const Cam2::I_Feed* feed)
{
	for (S32 i = 0; i < m_Viewport.Size(); i++)
	{
		C_Viewport& vp = m_Viewport[i];
		if (vp.m_GameFeed && vp.m_GameFeed->Current() == feed)
			vp.m_GameFeed->Show(0);
		if (vp.m_FutureFeed && vp.m_FutureFeed->Current() == feed)
			vp.m_FutureFeed->Show(0);
	}
}

const Cam2::I_Feed& C_CameraSystem::GameFeed(S32 vp) const
{
	BIZ_ASSERT(vp >= 0 && vp < m_Viewport.Size());

	BIZ_ASSERT(m_Viewport[vp].m_GameFeed);
	return *m_Viewport[vp].m_GameFeed;
}

const Cam2::I_Feed& C_CameraSystem::FutureFeed(S32 vp) const
{
	BIZ_ASSERT(vp >= 0 && vp < m_Viewport.Size());

	BIZ_ASSERT(m_Viewport[vp].m_FutureFeed);
	return *m_Viewport[vp].m_FutureFeed;
}

void C_CameraSystem::RenderSplitscreen()
{
#ifndef CFG_GOLD
	DebugRender();
#endif
}

void C_CameraSystem::RenderFullscreen()
{
#ifndef CFG_GOLD
	// Preserve fullscreen viewport
	Display::C_Viewport viewport = Display::Device()->GetViewport();

	// Render debug for all splitscreen views:
	S32 viewCount = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	for (S32 i = 0; i < viewCount; i++)
	{
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i);

		Display::Device()->SetViewport(view.m_View.GetViewport());
		RenderCameraInfo(i);
		if (i == Switcher().DebugViewport())
			Switcher().RenderFullscreen();
	}

	Display::Device()->SetViewport(viewport);
#endif
}

void C_CameraSystem::RenderCameraInfo(S32 viewport)
{
#ifndef CFG_GOLD
	C_String string;
	
	const Cam2::C_Camera& camera = DebugFeed(viewport).Live();

	// Show camera positional information:
	bool posInfo = g_ShowInfo || g_ExtraInfo;
	if (posInfo)
	{
		string += "Cam: ";
		string += camera.Name();
		string += " " + camera.InfoString(g_ExtraInfo) + " ";
	}

	// Show post effects information:
	if (g_PfxInfo)
	{
		const Cam2::C_EffectInfo* effect = camera.EffectInfo();
		
		C_String depthString, tmp;
		if (effect)
		{
			depthString.Printf("%.2f %.2f %.2f %.2f %.2f",
			    effect->NearStart(), effect->NearEnd(),
			    effect->FarStart(), effect->FarEnd(), effect->FarScale()
			);
		}
		else
		{
			float idepth = camera.Idepth();
			if (idepth < 1e-6f)
				depthString.Printf("Inf ");
			else
				depthString.Printf("%7.3f ", 1.f / idepth);
			
			tmp.Printf("%7.3f", camera.Aperture());
			depthString += tmp;
		}
		
		string += "Depth: " + depthString + " ";

		tmp.Printf("%4.2f", camera.ShutterAngle());
		string += "Shutter: " + tmp + " ";
	}

	if (posInfo || g_PfxInfo)
	{
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();
		debugRender->Draw2dText(HVec2(180.f, 672.f), string);
	}
#endif
}

void C_CameraSystem::SetFocusHandle(S32 vp, const Actor::C_ActorHandle handle)
{
	if (vp >= 0 && vp < m_Viewport.Size())
		m_Viewport[vp].m_FocusHandle = handle;
}

void C_CameraSystem::ResetVehicleFeed(S32 vp)
{
	if (vp < 0 || vp >= m_Viewport.Size())
		return;

	if (m_Viewport[vp].m_VehicleFeed)
		m_Viewport[vp].m_VehicleFeed->Reset();

	if (m_Viewport[vp].m_VehicleFuture)
		m_Viewport[vp].m_VehicleFuture->Reset();
}

C_VehicleFeed* C_CameraSystem::VehicleFeed(S32 vp)
{
	if (vp < 0 || vp >= m_Viewport.Size())
		return 0;
	
	return m_Viewport[vp].m_VehicleFeed;
}

void C_CameraSystem::ProcessInput()
{
	Cam2::CamInput input;
	
	if (!Switcher().UsingInput())
	{
		S32 pad = Switcher().DebugPad();

		// Disable zoom/roll and triggers if we are in GravGun camera:
		if (Switcher().Choice() == E_DebugCam_GravGun)
			input = Framework::GetFlyCameraControls(pad, false);
		else
			input = Framework::GetFlyCameraControls(pad, true);
	}

	S32 viewport = Switcher().DebugViewport();
	DebugFeed(viewport).SetInput(input);
}

C_ActorHandle C_CameraSystem::FocusHandle(S32 vp) const
{
	if (vp < 0 || vp >= m_Viewport.Size())
		return C_ActorHandle();

#ifndef CFG_GOLD
	if (GetRaceLogicCore() && g_FocusCar >= 0 && vp == Switcher().DebugViewport())
		return GetRaceLogicCore()->GetActorHandleFromVehicleIndex((U8)g_FocusCar);
#endif

	return m_Viewport[vp].m_FocusHandle;
}

void C_CameraSystem::GetCurrentFocusObject()
{
#ifndef CFG_GOLD
	if (!IsOkay())
		return;

	// Get the debug focus object for each viewport:
	S32 max = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
	for (S32 i = 0; i < max; i++)
	{
		Cam2::C_CamObj object;
		
		const Game::C_PickupBase* pickup = 0;
		if (g_FocusPickup >= 0)
			pickup = Game::C_PickupsManager::Instance().GetPickupByIndex(g_FocusPickup);

		if (pickup)
		{
			HMat43 matrix = pickup->GetMatrix();
			object.position = matrix.AW();
			object.orient = C_Quaternion(matrix.Rotation());
		}
		else
		{
			C_ActorHandle handle = FocusHandle(i);
			const C_VehicleActor* vehicle = GetVehicleActor(handle);
			if (vehicle)
				object = VehicleActorToCameraObject(vehicle);
		}
		
		if (!g_UseFocusObjectRotation)
			object.orient = C_Quaternion(); // Don't follow the rotation of the object
		
		DebugFeed(i).SetObject(object);

		if (i == 0)
			m_DebugTools->SetObject(object);
	}
#endif
}

void C_CameraSystem::UpdateDebugTools(F32 period)
{
#ifndef CFG_GOLD
	if (!IsOkay())
		return;

	Cam2::C_DebugFeed& debugFeed = DebugFeed();
	m_DebugTools->SetLocalCamera(debugFeed.Live());
	m_DebugTools->Update(period);
	// Activate the camera debug if we exported something
	switch (m_DebugTools->Notify())
	{
		case Cam2::E_NetNotify_Scene:
			DrawCameras = true;
		break;
		case Cam2::E_NetNotify_Animation:
			// Activate the camera debug if we exported something
			DebugFeedShow(0, E_DebugCam_AnimCamera);
		break;
		case Cam2::E_NetNotify_SetPos:
			// Set camera position from Maya:
			DebugFeedShow(0, E_DebugCam_FlyCamera);
			debugFeed.CopyFrom(m_DebugTools->RemoteCamera());
		break;
	}

	bool logFrame = false;
	if (g_PosLogRecord)
		logFrame = true;
	
	if (g_PosLogRecordWhenCapturing && Framework::g_CaptureService.Capturing())
		logFrame = true;
	
	if (logFrame)
		m_DebugTools->LogFrame(m_DebugTools->LocalCamera());
#else
	(void)period;
#endif
}

void C_CameraSystem::DoLookAt()
{
	Cam2::C_DebugFeed& debugFeed = DebugFeed();
	bool look = false;
	RaceLogicCore* rlc = GetRaceLogicCore();
	
	if (LookAtEnable && rlc)
	{
		S32 count = rlc->GetNumVehicles();

		S32 index = LookAtIndex;
		if (index > count - 1)
			index = count - 1;

		if (index >= 0)
		{
			// Look at a vehicle:
			const C_VehicleActor* vehicle = rlc->GetVehicleActor((U8)index);
			
			if (vehicle)
			{
				HMat43 matrix = vehicle->GetMatrix();
				HVec3 pos = matrix.AW();
				
				pos += HVec3((F32)LookOffsetX, (F32)LookOffsetY, (F32)LookOffsetZ) * matrix.Rotation();
				F32 fov = DegToRad((F32)LookFov);
				debugFeed.SetLookTarget(pos, fov);
				look = true;
			}
		}
	}

	// Switch the debug feed to the look camera if we are using it:
	debugFeed.ShowLook(look);
}

void C_CameraSystem::CopyDepthOfFieldInfo()
{
	Game::C_GameWorldPostProcess* postProcess = GetGameWorld();
	if (postProcess == NULL)
		return;

	const Cam2::C_Camera& camera = DebugFeed().Live();

	bool dofActive = false;
	float nearStart = 0.0f, nearEnd = 0.0f, farStart = 0.0f, farEnd = 0.0f, farScale = 0.0f;

	const Cam2::C_EffectInfo* effect = camera.EffectInfo();
	if (effect)
	{
		// Depth of field ranges explicitly set:
		dofActive = true;
		nearStart = effect->NearStart();
		nearEnd   = effect->NearEnd();
		farStart  = effect->FarStart();
		farEnd    = effect->FarEnd();
		farScale  = effect->FarScale();
	}
	else if (camera.Aperture() > 0.0f)
	{
		// Calculate depth of field ranges from camera Idepth and Aperture:
		dofActive = true;
		
		F32 farBokeh = camera.CalculateBokeh(0.0f);
		
		// The width in pixels where the Bg / Fg DOF effect should kick in:
		F32 widtha =  2.f / 720.f;
		F32 widthb = 10.f / 720.f;
		
		// Find depth limits to feed into the shader:
		nearStart = camera.FindBokehLimit(-widthb);
		nearEnd   = camera.FindBokehLimit(-widtha);
		farStart  = camera.FindBokehLimit( farBokeh * 0.25f);
		farEnd    = camera.FindBokehLimit( farBokeh * 0.75f);
		if (farStart < 0.f)
			farStart = 1e+30f;
		if (farEnd < 0.f)
			farEnd = 1e+30f;
		farScale = Min(farBokeh * 72.0f, 1.0f);
	}

	if (dofActive)
	{
		postProcess->SetDepthOfField();
		PostProcess::C_DoFFgBg* dof = postProcess->GetDepthOfField();
		if (dof)
			dof->SetDepthOfField(0, nearStart, nearEnd, farStart, farEnd, farScale);
	}
	else
	{
		postProcess->SetMotionBlur();
	}
}

void C_CameraSystem::DebugRender()
{
	if (!IsOkay())
		return;

#ifndef CFG_GOLD
	if (DrawCameras)
	{
		Cam2::g_RenderDeferred = false;
		m_DebugTools->DebugRender();
		
		for (S32 i = 0; i < m_Viewport.Size(); i++)
		{
			C_Viewport& vp = m_Viewport[i];
			vp.m_VehicleFeed->DebugRender();
			vp.m_GameFeed->Live().DebugRender(RGBA(0xff80ff80), true);
			if (vp.m_HOGResetCamera)
				vp.m_HOGResetCamera->DebugRender(RGBA(0xffff0000));
		}
		
		if (m_FatalCamera)
			m_FatalCamera->DebugRender(RGBA(0xffff0000));

		if (m_PhotoCamera2)
			m_PhotoCamera2->DebugRender(RGBA(0xffffff00));
	}

	if (g_PfxInfo)
	{
		const Cam2::C_Camera& live = DebugFeed().Live();
		
		// Draw a debug sphere to show the depth:
		F32 depth = live.Idepth();
		if (depth > 1e-5f)
		{
			depth = 1.f / depth;
			HVec3 pos = HVec3(0.f, 0.f, depth);
			
			pos = pos * live.Mat43();

			DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();
			debugRender->Draw3dSphere(pos, 0.2f, RGBA(0xff00ff00));
		}
	}
#endif
}

U32 C_CameraSystem::GetViewportCount() const
{
	return m_Viewport.Size();
}

void C_CameraSystem::StartFatalCamera()
{
	if (!m_FatalCamera)
	{
		m_FatalCamera = new C_FatalCamera;
		if (!m_FatalCamera)
			return;
	}
	
	m_FatalCamera->Start(FocusHandle(), GameFeed().Live());
	GameFeedShow(0, m_FatalCamera);
}

void C_CameraSystem::StopFatalCamera()
{
	GameFeedShow(0, E_GameCam_VehicleFeed);
	SAFE_DELETE_PTR(m_FatalCamera)
}

void C_CameraSystem::StartHOGResetCamera(bool objectTracking, int viewport)
{
	if (viewport < 0 || viewport >= m_Viewport.Size())
		return;

	C_Viewport& vp = m_Viewport[viewport];

	if (!vp.m_HOGResetCamera)
	{
		vp.m_HOGResetCamera = new C_HOGResetCamera;
		if (!vp.m_HOGResetCamera)
			return;
	}
	
	vp.m_HOGResetCamera->SetCameraTracking(objectTracking);
	vp.m_HOGResetCamera->Start(FocusHandle(viewport), GameFeed(viewport).Live());
	GameFeedShow(viewport, vp.m_HOGResetCamera);
}

void C_CameraSystem::StopHOGResetCamera(int viewport)
{
	if (viewport < 0 || viewport >= m_Viewport.Size())
		return;

	C_Viewport& vp = m_Viewport[viewport];

	GameFeedShow(viewport, E_GameCam_VehicleFeed);
	SAFE_DELETE_PTR(vp.m_HOGResetCamera)
}

bool C_CameraSystem::HOGResetCameraIsActive(int viewport) const
{
	if (viewport < 0 || viewport >= m_Viewport.Size())
		return false;
	
	return m_Viewport[viewport].m_HOGResetCamera != 0;
}

void C_CameraSystem::StartSetPieceCamera(HVec3 p_Position, HVec3 p_LookAt, float p_Fov, bool p_TrackObject)
{
	if (!m_pSetPieceCamera)
	{
		m_pSetPieceCamera = new C_SetPieceCamera;
		if (!m_pSetPieceCamera)
			return;
	}
	
	m_pSetPieceCamera->Start(FocusHandle(), p_Position, p_LookAt, p_Fov, p_TrackObject);
	GameFeedShow(0, m_pSetPieceCamera);
}

void C_CameraSystem::StopSetPieceCamera()
{
	GameFeedShow(0, E_GameCam_VehicleFeed);
	SAFE_DELETE_PTR(m_pSetPieceCamera)
}

void C_CameraSystem::StartPhotoCamera()
{
	SAFE_DELETE_PTR(m_PhotoCamera2)
	m_PhotoCamera2 = new C_PhotoCamera2;
	if (!m_PhotoCamera2)
		return;
	
	m_PhotoCamera2->Init();
	m_PhotoCamera2->CopyFromVehicleFeed(VehicleFeed());
	GameFeedShow(0, m_PhotoCamera2);

	m_Lod = Lod::g_LodEnabled;
	Lod::g_LodEnabled = false;
	Lod::C_LodManager::Instance().Update(0.0f, false, true);

	Game::C_GameWorld *pGameWorld = GetGameWorld();
	if (pGameWorld)
		pGameWorld->GetSplashCamera()->SetVisibilityFlags(0);
}

void C_CameraSystem::StopPhotoCamera()
{
	if (m_pSetPieceCamera)
		GameFeedShow(0, m_pSetPieceCamera);
	else
		GameFeedShow(0, E_GameCam_VehicleFeed);

	SAFE_DELETE_PTR(m_PhotoCamera2)

	Lod::g_LodEnabled = m_Lod;
	Lod::C_LodManager::Instance().Update(0.0f, false, true);

	Game::C_GameWorld *pGameWorld = GetGameWorld();
	if (pGameWorld)
		pGameWorld->GetSplashCamera()->SetVisibilityFlags(Game::E_ViewTypeFlag_Main);
}

// This function returns 0x0000000 if a pointer is null, and 0xffffffff if a pointer is non-null
// in a cool branchless way :-)
static inline U32 PointerToMask(void* pointer)
{
	// [Blur2PC][x64] Test the whole pointer; its low 32 bits alone can be zero.
	const S64 pos = (S64)(INTPTR)pointer;
	const S64 neg = -pos;
	const S64 np  = pos | neg;
	const S64 sat = np >> 63;
	return (U32)sat;
}

bool C_CameraSystem::IsPhotoCamera() const
{
	return m_PhotoCamera2 != 0;
}

U32 C_CameraSystem::IsPhotoCamera_Mask() const
{
	return PointerToMask(m_PhotoCamera2);
}

bool C_CameraSystem::GameViewIsInsideCar(U32 viewport) const
{
	if (viewport >= (U32)m_Viewport.Size())
		return false;

	if (m_Viewport[viewport].m_CameraType != E_GameCam_VehicleFeed)
		return false;
	
	C_VehicleFeed* vehicleFeed = m_Viewport[viewport].m_VehicleFeed;
	return vehicleFeed && vehicleFeed->CurrentType() == E_VFCamera_Bumper;
}

bool C_CameraSystem::PadUsedForDebug(S32 pad) const
{
#ifndef CFG_GOLD
	S32 viewport = Switcher().DebugViewport();
	bool used = DebugFeed(viewport).PadUsed() == pad;
	return used;
#else
	(void)pad;
	return false;
#endif
}

void C_CameraSystem::UpdateSpotlight()
{
#ifndef CFG_GOLD
	Game::C_GameWorld* world = GetGameWorld();
	SceneGraph::C_SceneGraph* sceneGraph = world ? world->GetSceneGraph() : 0;
	
	if (!sceneGraph)
	{
		SAFE_DELETE_PTR(m_SpotLight)
		return;
	}

	if (g_SpotLightEnable)
	{
		if (!m_SpotLight)
		{
			// Create a spotlight:
			m_SpotLight = new SceneGraph::C_SpotLightHandle; 
			if (m_SpotLight)
				*m_SpotLight = SceneGraph::AddSpotLight(sceneGraph);
		}
		
		// Get pointer to the spotlight object:
		SceneGraph::C_SpotLight* light = m_SpotLight ? m_SpotLight->Object() : 0;

		if (light)
		{
			// Update spotlight with the camera's position
			const Cam2::C_Camera& camera = DebugFeed().Live();
			F32 width = DegToRad((F32)g_SpotLightWidth);
			if (g_SpotLightUpdate)
				light->SetDimensions(camera.Mat43(), 100.f, width, width);

			F32 intensity = g_SpotLightIntensity;
			light->SetColour(intensity, intensity, intensity);
		}
	}

	if (!g_SpotLightEnable && m_SpotLight)
	{
		// Delete spotlight:
		SceneGraph::RemoveSpotLight(sceneGraph, *m_SpotLight);
		SAFE_DELETE_PTR(m_SpotLight)
	}

#endif
}

static C_ViewportFovScale* g_ViewportFovScale = 0;
static C_ViewportScreenOffset* g_ViewportScreenOffset = 0;


void C_ViewportFovScale::SorceryCallback(XtEventType evt, XtRecord*, C_ViewportFovScale* self, XtCallbackContext*)
{
	if (evt == kXtEtConstruct)
		g_ViewportFovScale = self;
	
	if (evt == kXtEtDestruct)
		g_ViewportFovScale = 0;
}

void C_ViewportScreenOffset::SorceryCallback(XtEventType evt, XtRecord*, C_ViewportScreenOffset* self, XtCallbackContext*)
{
	if (evt == kXtEtConstruct)
		g_ViewportScreenOffset = self;

	if (evt == kXtEtDestruct)
		g_ViewportScreenOffset = 0;
}

// For the current view configuration, return a scale factor for the FOV:
F32 C_CameraSystem::FovScale() const
{
	if (!g_ViewportFovScale)
		return 1.f;

	F32 fovScale = 1.0f;
	switch (m_ViewConfig)
	{
		case 2:
			fovScale = g_ViewportFovScale->m_TwoPlayer;
		break;
		case 3:
			fovScale = g_ViewportFovScale->m_ThreePlayer;
		break;
		case 4:
			fovScale = g_ViewportFovScale->m_FourPlayer;
		break;
	}
	
	fovScale = Clamped(fovScale, 0.1f, 1.0f); // Sanity check
	return fovScale;
}

// For the current view configuration, return an offset for the projection matrix:
Maths::HVec2 C_CameraSystem::ScreenOffset(int viewportIdx ) const
{
	if (g_ViewportScreenOffset)
	{

		switch (m_ViewConfig)
		{
		case 2:
			{
				switch (viewportIdx)
				{
				case 3:
					return Maths::HVec2( g_ViewportScreenOffset->m_TwoViewsTop.vx, g_ViewportScreenOffset->m_TwoViewsTop.vy);
					break;
				case 1:
					return Maths::HVec2( g_ViewportScreenOffset->m_TwoViewsBottom.vx, g_ViewportScreenOffset->m_TwoViewsBottom.vy);
					break;
				}
			}
			break;
		case 3:
			{
				switch (viewportIdx)
				{
				case 3:
					return Maths::HVec2( g_ViewportScreenOffset->m_ThreeViewsTop.vx, g_ViewportScreenOffset->m_ThreeViewsTop.vy);
					break;
				case 1:
					return Maths::HVec2( g_ViewportScreenOffset->m_ThreeViewsBottomLeft.vx, g_ViewportScreenOffset->m_ThreeViewsBottomLeft.vy);
					break;
				case 2:
					return Maths::HVec2( g_ViewportScreenOffset->m_ThreeViewsBottomRight.vx, g_ViewportScreenOffset->m_ThreeViewsBottomRight.vy);
					break;
				}
			}
			break;
		case 4:
			{
				switch (viewportIdx)
				{
				case 0:
					return Maths::HVec2( g_ViewportScreenOffset->m_FourViewsTopLeft.vx, g_ViewportScreenOffset->m_FourViewsTopLeft.vy);
					break;
				case 1:
					return Maths::HVec2( g_ViewportScreenOffset->m_FourViewsTopRight.vx, g_ViewportScreenOffset->m_FourViewsTopRight.vy);
					break;
				case 2:
					return Maths::HVec2( g_ViewportScreenOffset->m_FourViewsBottomLeft.vx, g_ViewportScreenOffset->m_FourViewsBottomLeft.vy);
					break;
				case 3:
					return Maths::HVec2( g_ViewportScreenOffset->m_FourViewsBottomRight.vx, g_ViewportScreenOffset->m_FourViewsBottomRight.vy);
					break;
				}
			}
			break;
		};

	}
	return Maths::HVec2(0.0f,0.0f);
}

void C_CameraSystem::UpdateWreckedEffect()
{
	// See which viewports should have broken glass on them:
	C_HighDynamicRange* hdr = GetHighDynamicRange();

	for (int i = 0; i < m_Viewport.Size(); i++)
	{
		float smash = 0.0f;
		
		// First see if the camera is showing the VehicleFeed (Wrecked camera is just Kanzen camera in a special state):
		if (m_Viewport[i].m_CameraType == E_GameCam_VehicleFeed)
		{
			// If it is, check if the vehicle is in a wrecked state:
			const C_VehicleActor* vehicleActor = GetVehicleActor(FocusHandle(i));

			const Game::Actors::C_VehicleLogicActor* vla = 0;
			if (vehicleActor)
				vla = GetVehicleLogicActor(vehicleActor->GetVehicleIndex());

			if (vla && vla->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
				smash = 1.0f;
		}

		if (hdr)
			hdr->SetOverlay(i, smash);
	}
}

// ---------------------------------------------------------
C_Viewport::C_Viewport()
{
	m_GameFeed = new Cam2::C_Feed;
	m_FutureFeed = new Cam2::C_Feed;
	m_VehicleFeed = new C_VehicleFeed;
	m_VehicleFuture = new C_VehicleFeed;
	m_CameraType = E_GameCam_None;
	m_HOGResetCamera = 0;

	if (!m_GameFeed || !m_FutureFeed || !m_VehicleFeed || !m_VehicleFuture)
		Exit();
	
	m_VehicleFeed->Init();
	m_VehicleFuture->SetFuture(true);
	m_VehicleFuture->Init();
}

C_Viewport::~C_Viewport()
{
	Exit();
}

void C_Viewport::Exit()
{
	SAFE_DELETE_PTR(m_GameFeed)
	SAFE_DELETE_PTR(m_FutureFeed)
	SAFE_DELETE_PTR(m_VehicleFeed)
	SAFE_DELETE_PTR(m_VehicleFuture)
}

} // End of namespace

XT_TYPE(AmaxCam::C_ViewportFovScale, "AmaxViewportFovScale")
	XT_CALLBACK(AmaxCam::C_ViewportFovScale::SorceryCallback)
	XT_FIELD(m_TwoPlayer,   "TwoPlayer")
	XT_FIELD(m_ThreePlayer, "ThreePlayer")
	XT_FIELD(m_FourPlayer,  "FourPlayer")
XT_END_TYPE


XT_TYPE(AmaxCam::C_ViewportScreenOffset, "AmaxViewportScreenOffset")
	XT_CALLBACK(AmaxCam::C_ViewportScreenOffset::SorceryCallback)
	XT_FIELD(m_TwoViewsTop,			"TwoViewsTop")
	XT_FIELD(m_TwoViewsBottom,		"TwoViewsBottom")
	XT_FIELD(m_ThreeViewsTop,			"ThreeViewsTop")
	XT_FIELD(m_ThreeViewsBottomLeft,	"ThreeViewsBottomLeft")
	XT_FIELD(m_ThreeViewsBottomRight,	"ThreeViewsBottomRight")
	XT_FIELD(m_FourViewsTopLeft,		"FourViewsTopLeft")
	XT_FIELD(m_FourViewsTopRight,		"FourViewsTopRight")
	XT_FIELD(m_FourViewsBottomLeft,	"FourViewsBottomLeft")
	XT_FIELD(m_FourViewsBottomRight,	"FourViewsBottomRight")

XT_END_TYPE
