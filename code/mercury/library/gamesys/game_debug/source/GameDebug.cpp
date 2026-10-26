#include "GameDebugPch.h"

#include <game_debug/GameDebug.h>

#include <game_debug/DebugWorldTextures.h>
#include <game_debug/DebugSamplerStates.h>
#include <game_debug/DebugModelRender.h>
#include <game_debug/LightSamplePointDebug.h>
#include <game_debug/CubemapPointDebug.h>
#include <game_debug/DebugTextureOverlay.h>
#include <game_debug/DebugResourceRender.h>
#include <game_debug/DumpResourceStats.h>
#include <game_debug/MiniPerfMonitor.h>
#include <input/GameInput.h>

namespace GameDebug
{

extern Debug::Bool g_debugMiniMonitorEnable;

Debug::Logger& g_Log = Debug::GetLogger( "mercury.gamesys.gamedebug" );

Debug::Bool g_DebugDumpResourceStats("GameSys.DumpResourceStats", false);

C_GameDebug::C_GameDebug()
	: m_pLevelSceneManager(NULL)
	, m_pExternalConstants(NULL)
{
	m_pDebugWorldTextures = new GameDebug::C_DebugWorldTextures;
	m_pDebugSamplerStates = new GameDebug::C_DebugSamplerStates;
	m_pDebugModelRender = new GameDebug::C_DebugModelRender;
	m_pDebugPlatformRender = new GameDebug::C_DebugPlatformRender;
	m_pLightSamplePointDebug = new GameDebug::C_LightSamplePointDebug;
	m_pCubemapPointDebug = new GameDebug::C_CubemapPointDebug;
	m_pDebugTextureOverlay = new GameDebug::C_DebugTextureOverlay;
	m_pDebugResourceRender = new GameDebug::C_DebugResourceRender;
	m_pMiniMonitor = new GameDebug::C_MiniPerfMonitor;
	m_bEnableMiniMonitor = true;
}

C_GameDebug::~C_GameDebug()
{
	SAFE_DELETE_PTR( m_pDebugWorldTextures );
	SAFE_DELETE_PTR( m_pDebugSamplerStates );
	SAFE_DELETE_PTR( m_pDebugModelRender );
	SAFE_DELETE_PTR( m_pDebugPlatformRender );
	SAFE_DELETE_PTR( m_pCubemapPointDebug );
	SAFE_DELETE_PTR( m_pLightSamplePointDebug );
	SAFE_DELETE_PTR( m_pDebugTextureOverlay );
	SAFE_DELETE_PTR( m_pDebugResourceRender );
	SAFE_DELETE_PTR( m_pMiniMonitor );
}

/// set perf budget limits in milliseconds
void C_GameDebug::SetPerfLimits(float fGPU, float fCPU, float fPhysics, float fUpdate, float fGraphics)
{
	m_pMiniMonitor->SetLimits(fGPU, fCPU, fPhysics, fUpdate, fGraphics);
}

void C_GameDebug::SetMiniMonitorEnable(bool b)
{
	m_bEnableMiniMonitor = b;
}

void C_GameDebug::SetAlbedoGamma(const bool srgb)
{
	m_pDebugWorldTextures->SetAlbedoGamma(srgb);
}

void C_GameDebug::SetNormalGamma(const bool srgb)
{
	m_pDebugWorldTextures->SetNormalGamma(srgb);
}

void C_GameDebug::SetSpecularGamma(const bool srgb)
{
	m_pDebugWorldTextures->SetSpecularGamma(srgb);
}

void C_GameDebug::SetEmissiveGamma(const bool srgb)
{
	m_pDebugWorldTextures->SetEmissiveGamma(srgb);
}

void C_GameDebug::Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants )
{
	m_pLevelSceneManager = pLevelSceneManager;
	m_pExternalConstants = pExternalConstants;

	m_pDebugModelRender->Init( pLevelSceneManager, pExternalConstants );
	m_pLightSamplePointDebug->Init( pLevelSceneManager );
	m_pCubemapPointDebug->Init( pLevelSceneManager );
	m_pDebugTextureOverlay->Init();
	m_pDebugResourceRender->Init( pLevelSceneManager, Input::C_GameInput::GetInstance(0) );
	m_pMiniMonitor->Initialise();
}

void C_GameDebug::InitPlatformDebug( GameResource::C_ModelData* pModelData, WorldRender::C_RenderModelData* pRenderModelData )
{
	m_pDebugPlatformRender->Init( m_pLevelSceneManager, m_pExternalConstants, pModelData, pRenderModelData );
}

void C_GameDebug::Update( const Maths::HMat43& mCameraToWorld, const bool gamePaused )
{
	// set up the debug textures for rendering
	m_pDebugWorldTextures->Update();

	// set up the debug sampler states
	m_pDebugSamplerStates->Update();

	m_pDebugModelRender->Update( mCameraToWorld, gamePaused );
	m_pDebugPlatformRender->Update( gamePaused );

	m_pLightSamplePointDebug->Update( mCameraToWorld.AW() );
	m_pCubemapPointDebug->Update( mCameraToWorld.AW() );

	m_pDebugTextureOverlay->Update();

	m_pDebugResourceRender->Update(mCameraToWorld);

	if(g_DebugDumpResourceStats)
	{
		DumpResourceStats();
		g_DebugDumpResourceStats = false;
	}
}

void C_GameDebug::Render2d()
{
	m_pDebugTextureOverlay->Render2d();

	m_pDebugResourceRender->Render2d();

	if (m_bEnableMiniMonitor && g_debugMiniMonitorEnable)
		m_pMiniMonitor->Render();
}

void C_GameDebug::Close()
{
	m_pLightSamplePointDebug->Close();

	m_pLightSamplePointDebug->Close();

	m_pDebugModelRender->Close();
	m_pDebugPlatformRender->Close();

	m_pDebugTextureOverlay->Close();

	m_pDebugResourceRender->Close();

	m_pMiniMonitor->Free();
}

GameDebug::C_DebugWorldTextures* C_GameDebug::GetDebugWorldTextures()
{
	return m_pDebugWorldTextures;
}

void C_GameDebug::SetEnvMapOverride(const void* pEnvMapGPUTextureOverride)
{
	m_pDebugModelRender->SetEnvMapOverride(pEnvMapGPUTextureOverride);
}

} // namespace GameDebug
