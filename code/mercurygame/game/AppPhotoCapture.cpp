#include "StdAfx.h"
#include "AppPhotoCapture.h"

#include "Game/GameWorld/PostProcessLayer.h"
#include "Game/MGCam/CameraSystem.h"
#include "GameApp.h"

#include <camera2/camera.h>
#include <camera2/capture/PhotoCapture.h>
#include <Display/Context/Context.h>
#include <Framework/Application/CaptureService.h>
#include <fx/fx.h>
#include <Render/Photo/AccumulateStrip.h>
#include <Render/PostProcess/Manager.h>

// Debug variables that we turn on or off when taking Photo Mode shots:
extern Debug::Bool g_DebugDisableDebugHud;
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
namespace GameDebug { extern Debug::Bool g_debugMiniMonitorEnable; }
namespace PostProcess { extern Debug::Bool g_HDRBloomFreeze, g_debugMotionBlurDoF, g_HDRLuminanceCalculate, g_HDRBloomEnable; }

namespace PhotoMode
{
using namespace Maths;

C_AppPhotoCapture::C_AppPhotoCapture()
{
	m_Accumulate = new Render::Photo::C_AccumulateStrip;

	S32 i = 0, max = 7;
	m_OldBool = new bool[max];
	for (i = 0; i < max; i++)
		m_OldBool[i] = false;
	
	m_OffscreenTexture = 0;
}

C_AppPhotoCapture::~C_AppPhotoCapture()
{
	m_OffscreenTexture = 0;
	SAFE_DELETE_PTR(m_Accumulate)
	SAFE_DELETE_ARY(m_OldBool)
}

const Cam2::C_Camera* C_AppPhotoCapture::GetLiveCamera()
{
	return &MGCam::System().DebugFeed().Live();
}

void C_AppPhotoCapture::TurnStuffOff(bool largePhoto, bool restore)
{
	// Set a debug variable, or restore it to previous value if 'restore' is true:
#define DV_SET(var, old, value) \
	if (!restore) { old = var; var = value; } \
	else { var = old; }
	
#ifdef PLATFORM_XENON
	Display::g_AlphaToMaskOffsets = restore ? D3DALPHATOMASK_DITHERED : D3DALPHATOMASK_SOLID;
#endif

	DV_SET(g_DebugDisableDebugHud, m_OldBool[0], true)
	DV_SET(PostProcess::g_debugMotionBlurDoF, m_OldBool[1], false)
	DV_SET(GameDebug::g_debugMiniMonitorEnable, m_OldBool[2], false)

	if (largePhoto)
	{
		DV_SET(PostProcess::g_HDRBloomFreeze,        m_OldBool[3], true)
		DV_SET(PostProcess::g_HDRLuminanceCalculate, m_OldBool[4], false)
		DV_SET(DynamicEffect::Particle::g_LimitScreenSize, m_OldBool[5], false)
		DV_SET(DynamicEffect::Light::g_debugFreezeOcclusionQueries, m_OldBool[6], true)
	}
}

// Try and borrow one of Charlie's textures to resolve into:
static Display::C_Texture2D* GetOffscreenResolveTexture()
{
	C_GameApp* gameApp = C_GameApp::GetInstance();
	if (!gameApp)
		return 0;
	
	Game::C_PostProcessLayer* layer = gameApp->PostProcessLayer();
	PostProcess::C_Manager* manager = layer ? layer->GetManager() : 0;
	if (!manager)
		return 0;

	const PostProcess::C_RenderTarget& rtarg = manager->GetRenderTarget(0);
	if (rtarg.m_RenderTextures.Size() < 1)
		return 0;
	
#if USE_POSTPROCESS_RENDERBUFFERS
	Display2::C_RenderBuffer* renderTexture = rtarg.m_RenderTextures[0];
#else
	Display::C_RenderTexture2D* renderTexture = rtarg.m_RenderTextures[0];
#endif
	if (!renderTexture)
		return 0;

	Display::C_Texture2D& texture = renderTexture->GetTexture();

	// Check it's the right size:
	if (texture.GetWidth() != 1280 || texture.GetHeight() != 720)
		return 0;

	return &texture;
}

void C_AppPhotoCapture::RenderStart(S32 samples)
{
#ifdef PLATFORM_XENON
	m_OffscreenTexture = HideCapture() ? GetOffscreenResolveTexture() : 0;
#endif

#ifdef PLATFORM_PS3
	if (HideCapture())
	{
		Display::Device()->BeginScene();
		Display::BasicRender()->DrawColoured2DRect(0.f, 0.f, 1024.f, 768.f, RGBA(0xff000000));
		Display::Device()->EndScene();
		Display::Device()->Present(0);
	}
#endif

	m_Accumulate->Start(samples);
}

void C_AppPhotoCapture::RenderAddSample()
{
	FX::ReloadEffects();

	// First render the game:
	Display::Device()->BeginScene();
	C_GameApp::GetInstance()->CommonRender();

#ifdef PLATFORM_XENON
	if (m_OffscreenTexture)
	{
		// Resolve to Charlie's texture and render black over the top:
		Display::Device()->Resolve(Display::E_Resolve_Colour, m_OffscreenTexture);
		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Stencil, RGBA(0));
	}
	Display::Device()->EndScene();
	Display::Device()->Present();

	Display::Device()->BeginScene();
	m_Accumulate->AddSample(m_OffscreenTexture);
	Display::Device()->EndScene();
#endif

#ifdef PLATFORM_PS3
	if (HideCapture())
		Display::Device()->SyncCpuGpu();
	else
		Display::Device()->Present();

	m_Accumulate->AddSample(0);
	Display::Device()->EndScene();
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
}

void C_AppPhotoCapture::RenderStop()
{
#ifndef CFG_GOLD
	unsigned char* bits = 0;
	S32 pitch = 0, width = 0, height = 0, format = 0;
	m_Accumulate->LockBitmap(bits, pitch, width, height, format);
	Framework::g_CaptureService.SendLockedRect(bits, pitch, width, height, format);
	m_Accumulate->UnlockBitmap();
#endif

	m_OffscreenTexture = 0;
}

}
