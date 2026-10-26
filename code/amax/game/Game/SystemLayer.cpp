#include "stdafx.h"

#define RENDERINGDATA_ALLOW_INCLUDE

#include "GameApp.h"
#include <game/SystemLayer.h>
#include <game/AmaxCam/CameraSystem.h>
#include <DebugSystems/InputView.h>
#include <debugsystems/ProfilerView.h>
#include "renderingdata/renderingdata.h"
#include "renderingdata/reloadabletextures.h"
#include "game_resource\ModelResource.h"
#include "system/debug/I_DebugRender.h"
#include "animation/util/DebugRender.h"

#include <fx/fx.h>
#include <fx/RenderMutex.h>

namespace Game
{

// C_SystemLayer
C_SystemLayer::C_SystemLayer() : C_Layer("C_SystemLayer")
{
	m_ProfilerView = new DebugSystems::C_ProfilerView;
	if (m_ProfilerView)
		m_ProfilerView->Init();

	m_InputView = new DebugSystems::C_InputView;
	if (m_InputView)
		m_InputView->Init();

	m_UseRenderMutex = true;
}

C_SystemLayer::~C_SystemLayer()
{
	SAFE_DELETE_PTR(m_InputView)
	SAFE_DELETE_PTR(m_ProfilerView)
}

HRESULT C_SystemLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		case Framework::E_LayerMessage_Update:
			return Update();
		break;

		case Framework::E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;

		case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
			return RenderSplitscreenWithoutAA();
		break;

		case Framework::E_LayerMessage_RenderFullscreenDebug:
			return RenderFullscreen();
		break;
	}

	return S_OK;
}

HRESULT C_SystemLayer::Create()
{
	return S_OK;
}

HRESULT C_SystemLayer::Destroy()
{
	return S_OK;
}

HRESULT C_SystemLayer::Update()
{
	if(C_GameApp::GetInstance()->IsPaused())
	{
		return S_OK;
	}

	return S_OK;
}

Debug::Bool g_ReloadTextures("Graphics.Debug.ReloadTextures", false);

HRESULT C_SystemLayer::GlobalUpdate()
{
	AmaxCam::CameraSystem().RealUpdate();

	if(m_UseRenderMutex)
	{
		FX::LockSerialiseMutex();
		FX::ReloadEffects();
		FX::ReleaseSerialiseMutex();
	}

	#ifndef CFG_GOLD
	// reloadable textures
	RenderingData::Target::ReloadableTextureManager::Instance().RefreshTextures("user\\textures\\", g_ReloadTextures);
	if( g_ReloadTextures )
	{
		//Display::RefreshTextures("user\\textures\\");
	}

	{
		PERF_EVENT("Update.RefreshModelTextures");
		GameResource::RefreshModelTextures();
	}
	#endif

	return S_OK;
}

HRESULT C_SystemLayer::RenderSplitscreenWithoutAA()
{
	AmaxCam::CameraSystem().RenderSplitscreen();
	return S_OK;
}

HRESULT C_SystemLayer::RenderFullscreen()
{
	AmaxCam::CameraSystem().RenderFullscreen();
	
	if (m_ProfilerView)
		m_ProfilerView->Render();

	if (m_InputView)
		m_InputView->Render();

#ifndef CFG_GOLD
	Animation::DebugRenderStats(DbUtil::DefaultDebugRenderSys()->Immediate());
#endif
	
	return S_OK;
}

} // namespace Game
