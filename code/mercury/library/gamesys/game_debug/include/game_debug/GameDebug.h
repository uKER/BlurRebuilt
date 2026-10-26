#ifndef GAMESYS_GAME_DEBUG_GAME_DEBUG_H
#define GAMESYS_GAME_DEBUG_GAME_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Gravity
{
	struct C_ExternalConstants;
}

namespace GameResource
{
	class C_LevelSceneManager;
	class C_ModelData;
}

namespace WorldRender
{
	class C_RenderModelData;
};

namespace GameDebug
{

extern Debug::Logger& g_Log;

class C_DebugWorldTextures;
class C_DebugSamplerStates;
class C_LightSamplePointDebug;
class C_CubemapPointDebug;
class C_DebugModelRender;
class C_DebugPlatformRender;
class C_DebugTextureOverlay;
class C_DebugResourceRender;
class C_MiniPerfMonitor;

class C_GameDebug
{
public:

	C_GameDebug();
	~C_GameDebug();

	void	Init( GameResource::C_LevelSceneManager* pLevelSceneManager, Gravity::C_ExternalConstants* pExternalConstants );
	void	InitPlatformDebug( GameResource::C_ModelData* pModelData, WorldRender::C_RenderModelData* pRenderModelData );
	void	Update( const Maths::HMat43& mCameraToWorld, const bool gamePaused = false );
	void	Render2d();
	void	Close();

	void	SetAlbedoGamma(const bool srgb);
	void	SetNormalGamma(const bool srgb);
	void	SetSpecularGamma(const bool srgb);
	void	SetEmissiveGamma(const bool srgb);

	/// set perf budget limits in milliseconds
	void	SetPerfLimits(float fGPU, float fCPU, float fPhysics, float fUpdate, float fGraphics);

	void	SetMiniMonitorEnable(bool b);

	void SetEnvMapOverride(const void* pEnvMapGPUTextureOverride);

private:
	GameDebug::C_DebugWorldTextures* GetDebugWorldTextures(); 

private:
	GameDebug::C_DebugWorldTextures*		m_pDebugWorldTextures;
	GameDebug::C_DebugSamplerStates*		m_pDebugSamplerStates;
	GameDebug::C_DebugModelRender*			m_pDebugModelRender;
	GameDebug::C_DebugPlatformRender*		m_pDebugPlatformRender;
	GameDebug::C_LightSamplePointDebug*		m_pLightSamplePointDebug;
	GameDebug::C_CubemapPointDebug*			m_pCubemapPointDebug;
	GameDebug::C_DebugTextureOverlay*		m_pDebugTextureOverlay;
	GameDebug::C_DebugResourceRender*		m_pDebugResourceRender;
	GameDebug::C_MiniPerfMonitor*			m_pMiniMonitor;

	GameResource::C_LevelSceneManager*		m_pLevelSceneManager;
	Gravity::C_ExternalConstants*			m_pExternalConstants;
	bool m_bEnableMiniMonitor;
};

}; // GameDebug

#endif // GAMESYS_GAME_DEBUG_GAME_DEBUG_H
