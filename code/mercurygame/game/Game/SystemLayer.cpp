#include "stdafx.h"

#define RENDERINGDATA_ALLOW_INCLUDE

#include <game/SystemLayer.h>
#include <game/MGCam/CameraSystem.h>
#include "GameApp.h"

#include <game_resource/ModelResource.h>

#include <fx/Fx.h>
#include <DebugSystems/ProfilerView.h>
#include "renderingdata/renderingdata.h"
#include "renderingdata/reloadabletextures.h"

namespace Game
{

// C_SystemLayer
C_SystemLayer::C_SystemLayer()
{
	m_ProfilerView = new DebugSystems::C_ProfilerView;
	if (m_ProfilerView)
		m_ProfilerView->Init();
}

C_SystemLayer::~C_SystemLayer()
{
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

		case Framework::E_LayerMessage_RenderSplitscreenWithAA:
		case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
			return RenderSplitscreen();
		break;

		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;
	}

	return S_OK;
}

HRESULT C_SystemLayer::Create()
{
	m_GameData.Init( C_GameApp::GetInstance()->GetXtDatabase(), "gamedata/xt/GameDef.bin" );
	return S_OK;
}

HRESULT C_SystemLayer::Destroy()
{
	m_GameData.Close();
	return S_OK;
}

HRESULT C_SystemLayer::Update()
{
	m_GameData.Update();
	return S_OK;
}

Debug::Bool g_ReloadTextures("Graphics.Debug.ReloadTextures", false);

HRESULT C_SystemLayer::GlobalUpdate()
{
	MGCam::System().RealUpdate();

	FX::ReloadEffects();

	// reloadable textures
	RenderingData::Target::ReloadableTextureManager::Instance().RefreshTextures("user\\textures\\", g_ReloadTextures);
	if( g_ReloadTextures )
	{
		Display::RefreshTextures("user\\textures\\");
	}

	GameResource::RefreshModelTextures();

	return S_OK;
}

HRESULT C_SystemLayer::RenderFullscreen()
{
	MGCam::System().RenderFullscreen();
	
	if (m_ProfilerView)
		m_ProfilerView->Render();
	
	return S_OK;
}

HRESULT C_SystemLayer::RenderSplitscreen()
{
	MGCam::System().RenderSplitscreen();
	return S_OK;
}


} // namespace Game
