
#include "stdafx.h"
#include "CameraSystem.h"

#include <camera2/debug/DebugTools.h>
#include <camera2/shake/ShakeWorld.h>
#include <camera2/stock/AnimCamera.h>
#include <Effect/PostProcess/DoFFgBg.h>
#include <framework/application/ApplicationBase.h>
#include <framework/application/CaptureService.h>
#include <framework/application/UserControls.h>
#include <GameApp.h>
#include <Game/GameWorld/PostProcessLayer.h>
#include <jpeghelper/JpegEncoder.h>
#include <jpeghelper/JpegDecoder.h>
#include <Render/Photo/AccumulateStrip.h>
#include <system/debug/I_DebugRender.h>

#include "CameraSwitcher.h"
#include "AppPhotoCapture.h"

// Mercury Game Camera system

namespace MGCam
{
using namespace Maths;
using namespace Util;

static Debug::Bool g_DrawCameras("Camera2.DrawCameras", true);
static Debug::Bool g_ShowInfo("Camera2.ShowInfo", false);
static Debug::Bool g_PfxInfo("Camera2.PfxInfo", false);
static Debug::Bool g_PhotoModeTest("Camera2.PhotoMode.PhotoModeTest", false);

C_CameraSystem::C_CameraSystem()
{
	m_DebugTools = 0;
	m_GameFeed = 0;
	m_Camera = 0;
}

void C_CameraSystem::Init()
{
	m_DebugTools = new Cam2::C_DebugTools;
	m_DebugTools->Init();

	m_GameFeed = new Cam2::C_Feed;

	Switcher().Init();

	m_Camera = new Cam2::C_Camera;
	m_GameFeed->Show(m_Camera);
	m_Camera->StartUpdate(0.f); //djh - why does this fix black screen on load up?
}

void C_CameraSystem::Exit()
{
	Switcher().Exit();
	SAFE_DELETE_PTR(m_Camera)
	SAFE_DELETE_PTR(m_GameFeed)
	SAFE_DELETE_PTR(m_DebugTools)
}

C_CameraSystem::~C_CameraSystem()
{
	Exit();
}

C_CameraSystem& System()
{
	static C_CameraSystem s_instance;
	return s_instance;
}

void C_CameraSystem::RealUpdate()
{
	PERF_EVENT("CameraUpdate");

	assert(m_DebugTools);
	if (m_DebugTools == 0)
		return;
	assert(m_GameFeed);
	if (m_GameFeed == 0)
		return;

	F32 period = Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime();

	const Cam2::C_CamObj& object = GameFeed().Live().Object();

	Switcher().Update(period);

	Cam2::C_ShakeWorld::Instance().Update(period);

	// Find DebugFeed:
	Cam2::C_DebugFeed& debugFeed = DebugFeedPriv();

	S32 pad = Switcher().DebugPad();
	
	Cam2::CamInput input;
	// Disable zoom/roll and triggers if we are in GravGun camera:
	if (Switcher().Choice() == E_DebugCam_GravGun)
		input = Framework::GetFlyCameraControls(pad, false);
	else
		input = Framework::GetFlyCameraControls(pad, true);

	debugFeed.SetInput(input);
	debugFeed.SetObject(object);

	m_DebugTools->SetLocalCamera(debugFeed.Live());
	m_DebugTools->SetObject(object);
	m_DebugTools->Update(period);

	switch (m_DebugTools->Notify())
	{
		case Cam2::E_NetNotify_Scene:
		case Cam2::E_NetNotify_Animation:
			// Activate the camera debug if we exported something
			g_DrawCameras = true;
		break;
		case Cam2::E_NetNotify_SetPos:
			// Set camera position from Maya:
			DebugFeedShow(E_DebugCam_FlyCamera);
			debugFeed.CopyFrom(m_DebugTools->RemoteCamera());
		break;
	}

	if (g_PhotoModeTest)
		PhotoModeTest();

	if (g_DrawCameras)
	{
		Cam2::g_RenderDeferred = true;
		m_DebugTools->DebugRender();
		m_GameFeed->Live().DebugRender(RGBA(0xff80ff80), true);
	}

	// Copy across depth of field info:
	Game::C_PostProcessLayer* postProcess = 0;
	if (C_GameApp::GetInstance())
		postProcess = C_GameApp::GetInstance()->PostProcessLayer();

	PostProcess::C_DoFFgBg* dof = postProcess ? postProcess->GetDepthOfField() : 0;

	const Cam2::C_Camera& camera = DebugFeed().Live();
	F32 nearLimit1 = camera.FindBokehLimit(-10.f / 720.f);
	F32 nearLimit2 = camera.FindBokehLimit( -5.f / 720.f);
	F32 farLimit1  = camera.FindBokehLimit(  5.f / 720.f);
	F32 farLimit2  = camera.FindBokehLimit( 10.f / 720.f);
	if (farLimit1 < 0.f)
		farLimit1 = 1e+30f;
	if (farLimit2 < 0.f)
		farLimit2 = 1e+30f;

	if (dof)
		dof->SetDepthOfField(0, nearLimit1, nearLimit2, farLimit1, farLimit2, 1.f);
}

void C_CameraSystem::PhotoModeTest()
{
	Input::C_GameInput* gameInput = Input::GetGameInput(0);
	PhotoMode::C_AppPhotoCapture* photoCapture = C_GameApp::GetInstance()->PhotoCapture();
	Debug::Logger& log = Debug::GetLogger("mercury.camera2");
	const char* jpegName = "PhotoMode.jpg";

	if (!gameInput || !photoCapture)
		return;
	
	if (gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
		photoCapture->TakePhoto();

	Render::Photo::C_AccumulateStrip* accumulate = photoCapture->Accumulate();
	if (!accumulate)
		return;

	if (gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadLeft))
	{
		// Save as JPEG:
		U8* bits = 0;
		S32 pitch = 0, width = 0, height = 0, format = 0;
		accumulate->LockBitmap(bits, pitch, width, height, format);
		
		JpegHelper::C_JpegEncoder encoder;
		const char* text = "This is a piece of text to go in the comment field";
		encoder.SetComments(text, strlen(text) + 1);
		encoder.EncodeFromBuffer(bits, pitch, width, height, format);
		accumulate->UnlockBitmap();

		Sys::I_File* file = Sys::C_FileSystem::Instance().Open(jpegName, Sys::E_FileAccess_Write);
		if (file)
		{
			file->Write(encoder.JpegData(), encoder.JpegLength());
			delete file;
			file = 0;

			LOGINFO(log, "Saved photo to %s (%dk)", jpegName, encoder.JpegLength() >> 10);
		}
	}

	if (gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadRight))
	{
		// Load from JPEG:
		Sys::I_File* file = Sys::C_FileSystem::Instance().Open(jpegName, Sys::E_FileAccess_Read);
		if (!file)
			return;

		S32 len = (S32)file->Length();
		U8* data = new U8[len];
		if (!data)
		{
			delete file;
			return;
		}

		file->Read(data, len);
		
		JpegHelper::C_JpegDecoder decoder;
		decoder.SetData(data, len);

		U8* bits = 0;
		S32 pitch = 0, width = 0, height = 0, format = 0;
		accumulate->LockBitmap(bits, pitch, width, height, format);
		bool ret = decoder.DecodeIntoBuffer(bits, pitch, width, height, format);
		accumulate->UnlockBitmap();

		const char* comments = (const char*)decoder.Comments();
		if (!comments)
			comments = "";

		LOGINFO(log, "%s %s (%dk) [%s]", ret ? "Loaded" : "Error loading", jpegName, len >> 10, comments);

		delete file;
	}
}

void C_CameraSystem::WorldUpdate()
{
}

Cam2::C_DebugFeed& C_CameraSystem::DebugFeed()
{
	return DebugFeedPriv();
}

Cam2::C_DebugFeed& C_CameraSystem::DebugFeedPriv()
{
	return Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(0);
}

void C_CameraSystem::DebugFeedShow(S32 debugType)
{
	// Find DebugFeed:
	Cam2::C_DebugFeed& debugFeed = DebugFeedPriv();

	switch (debugType)
	{
		case E_DebugCam_GameFeed:
			debugFeed.ShowFeed(m_GameFeed);
		break;
		case E_DebugCam_GravGun:
			debugFeed.SetDebugType(Cam2::E_FlyCamera);
		break;
		case E_DebugCam_AnimCamera:
			debugFeed.ShowFeed(m_DebugTools->AnimCamera());
		break;
		default:
			debugFeed.SetDebugType(debugType);
		break;
	}
}

// Show in the gamefeed a custom feed:
void C_CameraSystem::GameFeedShow(const Cam2::I_Feed* feed)
{
	assert(m_GameFeed);
	m_GameFeed->Show(feed);
}

// Set gamefeed to 0, but only if feed is being shown
void C_CameraSystem::GameFeedRemove(const Cam2::I_Feed* feed)
{
	if (m_GameFeed->Current() == feed)
		m_GameFeed->Show(0);
}

const Cam2::I_Feed& C_CameraSystem::GameFeed()
{
	assert(m_GameFeed);
	return *m_GameFeed;
}

void C_CameraSystem::RenderSplitscreen()
{
	// todo - put the debug render in here once immediate alpha blend works
	
	if (g_PfxInfo)
	{
		const Cam2::C_Camera& live = DebugFeed().Live();
		
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
}

void C_CameraSystem::RenderFullscreen()
{
	Switcher().RenderFullscreen();

	C_String string;
	const Cam2::C_Camera& camera = DebugFeed().Live();
	
	// Show camera information:
	if (g_ShowInfo)
	{
		string += "Cam: ";
		string += camera.Name();
		string += " " + DebugFeed().Live().InfoString() + " ";
	}

	// Show post effects information:
	if (g_PfxInfo)
	{
		F32 idepth = camera.Idepth();

		string += "Depth: ";
		
		C_String tmp;
		if (idepth < 1e-6f)
			tmp.Printf("Inf ");
		else
			tmp.Printf("%7.3f ", 1.f / idepth);

		string += tmp;

		tmp.Printf("%7.3f ", camera.Aperture());
		string += tmp;
		
		tmp.Printf("Shutter: %4.2f ", camera.ShutterAngle());
		string += tmp;

		tmp.Printf("Fov: %7.3f ", RadToDeg(camera.Fov()));
		string += tmp;

		tmp.Printf("Focal: %6.2fmm ", camera.FovAsFocalLength() * 1000.f);
		string += tmp;

		tmp.Printf("Imaging: %6.2fmm ", camera.ImagingLength() * 1000.f);
		string += tmp;
	}

	if (g_ShowInfo || g_PfxInfo)
	{
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();
		debugRender->Draw2dText(HVec2(80.f, 672.f), string, RGBA(0xffffffff), 32.f);
	}

	if (g_PhotoModeTest)
	{
		PhotoMode::C_AppPhotoCapture* photoCapture = C_GameApp::GetInstance()->PhotoCapture();
		if (!photoCapture)
			return;
		
		const Display::C_Texture2D* texture = photoCapture->Accumulate()->GetTexture();
		if (texture)
			Display::BasicRender()->DrawTextured2DRect(64.f, 32.f, 384.f, 288.f, RGBA(0xffffffff), texture);

		texture = photoCapture->Accumulate()->StripTexture();
		if (texture)
			Display::BasicRender()->DrawTextured2DRect(480.f, 32.f, 384.f, 12.f, RGBA(0xffffffff), texture);
	}
}

void C_CameraSystem::CopyFrom(const Cam2::C_Camera& other)
{
	DebugFeedPriv().CopyFrom(other);
}
 
} // End of namespace
