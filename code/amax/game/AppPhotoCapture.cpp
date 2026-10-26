#include "StdAfx.h"
#include "AppPhotoCapture.h"

#include <camera2/Camera.h>
#include <camera2/stock/DebugFeed.h>
#include <Display/Context/Context.h>
#include <Framework/Application/CaptureService.h>
#include <fx/fx.h>
#include "Game/AmaxCam/CameraSystem.h"
#include "Game/AmaxCam/Helper.h"
#include "Game/GameActors/Drivers/DriverManager.h"
#include "Game/GameActors/Vehicles/VehicleActor.h"
#include "Game/GameWorld/GameWorld.h"
#include "Game/GameWorld/PostProcessLayer.h"
#include "GameApp.h"
#include "GameLogic/ActorStudioActors/VehicleProxyActor.h"
#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic/Pickups/PickupsManager.h"
#include <Graphics/HDR.h>
#include <Render/Photo/AccumulateStrip.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <Vehicle/VehicleModel.h>
#include <Lod/LodManager.h>

#ifdef PLATFORM_PS3
#include <display/buffer/gcm/GCMDeferredCommandBufferManager.h>
#endif

// Debug variables that we turn on or off when taking Photo Mode shots:
extern Debug::Bool g_DebugDisableDebugHud, g_SceneGraphRenderTwoDimensional, g_SceneGraphFreezeTwoDimensional, g_UiDebugDoRender;
#ifndef CFG_GOLD
extern Debug::Bool g_UiShouldRenderPanel_2DAA, g_UiShouldRenderPanel_2DIG_A, g_UiShouldRenderPanel_3DAA_0;
#endif

extern bool g_GameDebugRender2d;
namespace Display {	extern Debug::Int g_AlphaToMaskOffsets; }
namespace DynamicEffect
{
	namespace Particle { extern Debug::Bool g_LimitScreenSize; }
	namespace Light
	{
		extern Debug::Bool g_debugFreezeOcclusionQueries;
		extern Debug::Float g_debugGlobalOffsetX, g_debugGlobalOffsetY;
	}
}
namespace Game
{
	extern Debug::Bool g_ReflectionEnable; // This is really Reflection 'Update'
	extern Debug::Bool g_MakeSlowmoCaptureWork;
}
namespace PostProcess { extern Debug::Bool g_HDRBloomFreeze, g_debugMotionBlur, g_debugDepthOfField, g_HDRLuminanceCalculate, g_HDRBloomEnable; }
namespace SceneGraph
{
	extern Debug::Float g_DrawDistanceScale;
	EXTERN_DEBUG_FLOAT (g_fModelPVSDistanceScale);
	EXTERN_DEBUG_FLOAT (g_fModelLODDistanceScale);
}

namespace Lod
{
	extern Debug::Bool g_LodEnabled;
}

#ifndef CFG_GOLD
namespace UI { extern Maths::HVec3 g_PerkStashOffset; }
#endif

namespace PhotoMode
{
using namespace Maths;

static Debug::Bool g_MoveCars("Camera2.PhotoMode.MoveCars", true);

C_AppPhotoCapture::C_AppPhotoCapture()
{
	m_Accumulate = 0;
}

C_AppPhotoCapture::~C_AppPhotoCapture()
{
	Exit();
}

bool C_AppPhotoCapture::Init()
{
	m_Accumulate = new Render::Photo::C_AccumulateStrip;
	if (!m_Accumulate)
		return false;

	S32 i = 0, max = 15;
	m_OldBool = new bool[max];
	for (i = 0; i < max; i++)
		m_OldBool[i] = false;
	
	max = 1;
	m_OldInt = new S32[max];
	for (i = 0; i < max; i++)
		m_OldInt[i] = 0;
	
	max = 2;
	m_OldFloat = new F32[max];
	for (i = 0; i < max; i++)
		m_OldFloat[i] = 0.f;
	
	m_OffscreenTexture = 0;
	m_ToneMapShader = 0;
	return true;
}

void C_AppPhotoCapture::Exit()
{
	m_OffscreenTexture = 0;
	SAFE_DELETE_ARY(m_OldFloat)
	SAFE_DELETE_ARY(m_OldInt)
	SAFE_DELETE_ARY(m_OldBool)
	SAFE_DELETE_PTR(m_Accumulate)
}

Display::C_Texture2D* C_AppPhotoCapture::GetTexture()
{
	if (!m_Accumulate)
		return 0;
	
	return m_Accumulate->GetTexture();
}

const Cam2::C_Camera* C_AppPhotoCapture::GetLiveCamera()
{
	return &AmaxCam::CameraSystem().DebugFeed().Live();
}

void C_AppPhotoCapture::TurnStuffOff(bool restore)
{
	// Set a debug variable, or restore it to previous value if 'restore' is true:
#define DV_SET(var, old, value) \
	if (!restore) { old = var; var = value; } \
	else { var = old; }
	
	DV_SET(g_DebugDisableDebugHud,				m_OldBool[0], true)
	DV_SET(g_GameDebugRender2d,					m_OldBool[1], false)
	DV_SET(Lod::g_LodEnabled,					m_OldBool[2], false)

	DV_SET(SceneGraph::g_fModelPVSDistanceScale, m_OldFloat[0], 1000000000000000.f)
	DV_SET(SceneGraph::g_fModelLODDistanceScale, m_OldFloat[1], 1000000000000000.f)

	if (LargePhoto())
	{
		DV_SET(PostProcess::g_debugDepthOfField, m_OldBool[3], false)
		DV_SET(g_SceneGraphRenderTwoDimensional,     m_OldBool[4], false)
		DV_SET(PostProcess::g_HDRBloomFreeze,        m_OldBool[5], true)
		DV_SET(PostProcess::g_debugMotionBlur,       m_OldBool[6], false)
		DV_SET(PostProcess::g_HDRLuminanceCalculate, m_OldBool[7], false)
		DV_SET(Game::g_ReflectionEnable,             m_OldBool[8], false) // This is really Reflection 'Update'
		DV_SET(DynamicEffect::Particle::g_LimitScreenSize, m_OldBool[9], false)
		DV_SET(DynamicEffect::Light::g_debugFreezeOcclusionQueries, m_OldBool[10], true)
		DV_SET(g_SceneGraphFreezeTwoDimensional,     m_OldBool[11], true)

#ifndef CFG_GOLD
		// Turn off the UI which doesn't work with large screenshots:
		DV_SET(g_UiShouldRenderPanel_2DAA, m_OldBool[12], false)
		DV_SET(g_UiShouldRenderPanel_2DIG_A, m_OldBool[13], false)
		DV_SET(g_UiShouldRenderPanel_3DAA_0, m_OldBool[14], false)
#endif
	}

	if (ForceDisableUI())
	{
		// Turn off the UI:
		DV_SET(g_UiDebugDoRender, m_OldBool[12], false)
	}

	Lod::C_LodManager::Instance().Update(0.0f, false, true);
}


void C_AppPhotoCapture::RenderStart(S32 samples)
{
	if (!m_Accumulate)
		return;

	
	m_Accumulate->Start(samples);

	DynamicEffect::Light::g_debugGlobalOffsetX = DeflectX();
	DynamicEffect::Light::g_debugGlobalOffsetY = DeflectY();

	C_HighDynamicRange *hdr = GetHighDynamicRange();
	if (hdr && LargePhoto())
	{
		m_ToneMapShader = hdr->GetToneMapShader();
		hdr->SetToneMapShader(E_ToneMapShader_LargeScreenshot);
		F32 zoom = Zoom();
		hdr->SetLargeScreenshotBloomScale(zoom, -DeflectX() / zoom, -DeflectY() / zoom, 1.125f);
	}
}

void C_AppPhotoCapture::RenderAddSample()
{
	if (!m_Accumulate)
		return;

#if !CFG_GOLD
	//#define SHOW_PHOTOMODE_FRAMETIME
#endif

#ifdef SHOW_PHOTOMODE_FRAMETIME
	static Time::C_Time oldTime = Time::GetCurrentTime();
#endif

	MoveCars(ShutterTime() - 1.f);
	
	FX::ReloadEffects();

	C_GameApp::GetInstance()->PollPlatformCallbacks();

	// First render the game:
	Display::Device()->BeginScene();
	C_GameApp::GetInstance()->CommonRender();


#ifdef PLATFORM_PS3
	Display::Device()->EndScene();

	// wait until all deferred command buffers have been flushed
	while (!Display::g_DeferredCommandBufferManager.Flush())
	{
		Scheduler::Flush(Scheduler::JQ_GROUP_NONE, true, 30);
		sys_timer_usleep(30);
	}

	Display::Device()->BeginScene();
	m_Accumulate->AddSample(0);
	Display::Device()->EndScene();
	
	Display::Device()->BeginScene();
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Stencil, RGBA(0));
	Display::Device()->EndScene();
	Display::Device()->Present();
#endif

#ifdef PLATFORM_PC
	m_Accumulate->AddSample(0);

	if (HideCapture())
	{
		Display::Device()->SetColourDepthStencilSurface();
		Display::Device()->Clear(Display::E_Clear_Colour);
	}
	
	Display::Device()->EndScene();
	Display::Device()->Present();
#endif

	Display::RingBuffer()->EndFrame();

#ifdef SHOW_PHOTOMODE_FRAMETIME
	Time::C_Time newTime = Time::GetCurrentTime();;
	F32 theTime = (F32)(newTime-oldTime) / 1000.0f;
	LOGINFO(Framework::g_Log, "PhotoModeFrameTime %.2fms\n", theTime);
	oldTime = newTime;
#endif
}

void C_AppPhotoCapture::RenderStop()
{
#ifndef CFG_GOLD
	if (!m_Accumulate)
		return;

	if (Framework::g_CaptureService.ClientCount() > 0)
	{
		// Send image to XenWatch:
		unsigned char* bits = 0;
		S32 pitch = 0, width = 0, height = 0, format = 0;
		m_Accumulate->LockBitmap(bits, pitch, width, height, format);
		Framework::g_CaptureService.SendLockedRect(bits, pitch, width, height, format);
		m_Accumulate->UnlockBitmap();
	}
#endif

	C_HighDynamicRange *hdr = GetHighDynamicRange();
	if (hdr && LargePhoto())
	{
		hdr->SetToneMapShader((E_ToneMapShader)m_ToneMapShader);
		hdr->SetLargeScreenshotBloomScale(1.f, 0.f, 0.f, 1.f);
	}
	
	MoveCars(0.f); // Reset cars back to their position

	m_OffscreenTexture = 0;
}

void C_AppPhotoCapture::MoveCars(F32 frac)
{
using namespace Game::Actors;
using namespace Vehicle;

	if (!g_MoveCars)
		return;
	
	F32 gamePeriod = 1.f / 30.f;

	Game::C_GameWorld* world = AmaxCam::GetGameWorld();
	if (world)
		gamePeriod = world->GameStep() / 60.f;

	if (Game::g_MakeSlowmoCaptureWork)
		gamePeriod = C_GameApp::GetInstance()->GetTimeStepRaw();
	
	S32 i = 0;
	for (i = 0; i < 20; i++)
	{
		const C_VehicleActor* vehicle = AmaxCam::GetVehicleActor(AmaxCam::GetVehicleHandle(i));
		if (!vehicle)
			continue;
		
		HVec3 offset = AmaxCam::GetVehicleVelocity(vehicle) * gamePeriod * frac;

		((C_VehicleActor*)vehicle)->PhotoMoveCar(offset);
		C_DriverManager::Instance()->PhotoMoveDriver(vehicle->GetVehicleIndex(), offset);
	}

#ifndef CFG_GOLD
	// Move the animations which are children of the VehicleProxyActors:
	Actor::C_ActorList* actorList = AmaxCam::GetActorList();

	S32 max = Actors::g_VPAList.Size();
	for (i = 0; i < max; i++)
	{
		Actors::VehicleProxyActor* vpa = Actors::g_VPAList[i];
		S32 vehicleIndex = vpa->m_VehicleIndex;

		// Find out the offset of the vehicle through time:
		const C_VehicleActor* vehicle = AmaxCam::GetVehicleActor(AmaxCam::GetVehicleHandle(vehicleIndex));
		HVec3 offset = AmaxCam::GetVehicleVelocity(vehicle) * gamePeriod * frac;

		// Update the children of the VehicleProxyActor with the offset:
		for (S32 n = 0; n < vpa->m_Children.Size(); n++)
		{
			Maths::C_QuatTrans quatTrans(vpa->GetMatrix());
			quatTrans.m_Translation += offset;
			
			ActorMsg::C_ActorMovementUpdateMsg msg(0.f, quatTrans);
			actorList->SendMessageToActor(vpa->m_Children[n], &msg);
		}
	}

	MovePerkStash(gamePeriod * frac);
#endif

	Game::C_PickupsManager::Instance().PhotoMovePickups(frac);
}

#ifndef CFG_GOLD
void C_AppPhotoCapture::MovePerkStash(F32 timeValue)
{
using namespace Game::Actors;
using namespace Vehicle;
	// Update the position of the Perk Stash correctly:
	const C_VehicleActor* vehicle = AmaxCam::GetVehicleActor(AmaxCam::GetVehicleHandle(0));
	HVec3 playerVelocity;
	HMat33 playerOrient;
	if (vehicle)
	{
		playerVelocity = AmaxCam::GetVehicleVelocity(vehicle);
		playerOrient = vehicle->GetMatrix().Rotation();
	}
	
	// Get the velocity into the space of the car:
	playerVelocity = playerVelocity * playerOrient.InverseRotation();
	
	// Set it into the global perk stash offset so that UI draws it in the right place
	// The UI coordinate system, though 3D, seems to be 20x smaller for some reason...
	UI::g_PerkStashOffset = playerVelocity * timeValue * 20.f;
}
#endif

bool C_AppPhotoCapture::LockBitmap(unsigned char*& retBits, S32& retPitch, S32& retWidth, S32& retHeight, S32& retFormat)
{
	if (!m_Accumulate)
		return false;

	return m_Accumulate->LockBitmap(retBits, retPitch, retWidth, retHeight, retFormat);
}

void C_AppPhotoCapture::UnlockBitmap()
{
	if (m_Accumulate)
		m_Accumulate->UnlockBitmap();
}

PhotoMode::C_AppPhotoCapture& AppPhotoCapture()
{
	static C_AppPhotoCapture s_AppPhotoCapture;
	return s_AppPhotoCapture;
}

}
