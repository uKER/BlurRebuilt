#ifndef GAMEAPP_H
#define GAMEAPP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <Framework/Application/Application.h>
#include <Game/GameUtil/ResourceBagDeclare.h>
#include <Resource/Resource.h>

namespace Cam2 { class C_CaptureService; }

namespace FX
{
	class I_Effect;
}

class GameAutomationMessageController;

namespace DynamicEffect
{
	class Textures;
}

namespace Game
{
	class C_Game;
	class C_GameLayer;
	class C_SimpleUiLayer;
	class C_SystemLayer;
	class C_BootLayer;

	class C_RaceBookLayer;
	class C_GameAudioLayer;
};

namespace Framework
{
	class C_DebugLayer;
	class C_ApplicationParams;
};

namespace UI
{
	class C_UiLayer;
};

namespace AmaxPerfLogging
{
	class MessageController;
}

class C_NormalBuffer;

//! C_TestApp

class C_GameApp : public Framework::C_Application
{
public:

	C_GameApp();
	~C_GameApp();

	static C_GameApp* GetInstance()
	{
		return (C_GameApp*)Framework::C_Application::GetInstance();
	}

	HRESULT PlatformInitialise(const Framework::C_ApplicationParams& params);
	void PlatformShutdown();

	virtual void ExitChecks();
#if PLATFORM_PC
	virtual void ApplyGraphicalDetail(const U32 graphicalDetail);	// [Blur2PC] retail 0x0042e1c0
#endif

	HRESULT OnInitialise();
	void OnShutdown();

	void SetupUserControls();

	void OnUpdate();
	void OnRender();
	void RenderNow();

	void OnLoadingScreenUpdate();
	void OnLoadingScreenRender();

	Game::C_Game* GetGame() {return m_pGame;}

	void CreateGlobalXtDatabase();
	XtDb* GetGlobalXtDatabase(){return mGlobalXtDatabase;};

	UI::C_UiLayer*				GetUiLayer()			{ return m_pUiLayer; }
	Game::C_RaceBookLayer*		GetRaceBookLayer()		{ return m_pRaceBookLayer; }
	Game::C_GameAudioLayer*		GetGameAudioLayer()		{ return m_pGameAudioLayer; }
	Framework::C_DebugLayer*	GetDebugLayer()			{ return m_pDebugLayer; }
	Game::C_SystemLayer*		GetSystemLayer()		{ return m_pSystemLayer; }

	const C_ResourceBagHandle&	GetSpecialFXResourceBag() { return m_hSpecialFXResourceBag; }
	const DynamicEffect::Textures*	GetParticleTextures() { return m_pParticleTextures; }
	
	F32	GetElapsedRaceTime() { return m_ElapsedRaceTime; }
	F32 GetTimeStepRaw() { return GetTimer().GetFrameTime(); }
	F32 GetDebugFrameTime() { return DebugTimer().GetFrameTime(); }
	F32 GetTimeStep() { return m_TimeStep; }
	F32 GetTimeScale() { return m_TimeScale; }
	void SetTimeScale(F32 p_TimeScale);
	void SetTimeScaleTimeOut(F32 p_TimeScaleTimeOut) { m_TimeScaleTimeOut = p_TimeScaleTimeOut; }
	void ResetElapsedRaceTime() { m_ElapsedRaceTime = 0.0f; }

	F32 GetLoadingScreenTimestep() { return m_LoadingScreenTimeStep;}

	void CacheResources();

	bool IsBootLayerActive() const { return (m_pBootLayer != NULL); }

	void ReleaseCachedResources();
	void DumpCachedResources();

	void CacheBootGlobalResources();
	void LockCacheBootGlobalResources(
		DynamicEffect::Textures**		ppParticleTextures,
		Btl::Vector<FX::I_Effect*>**	ppEffectPointers);
	void UnlockCacheBootGlobalResources();
	void FinishCacheBootGlobalResources();

	void CachePostBootFxResourceBag();
	void CachePostBootAnimations();
	void SetCachePostBootFxResourceBagStarted();
	void SetCachePostBootAnimationsStarted();
	bool CachePostBootFxResourceBagReady();

	bool IsBootPreCachingComplete();
	void SetBootPreCachingComplete();
	void EnableInstallerScreen(bool enable);
	void EnableLoadingSeg(bool enable);
	bool IsInstallerScreenEnabled() const;
	bool IsLoadingSegEnabled() const;

	//Only for use by GameAppResources.cpp
	void LockCachePostBootAnimations(Resource::C_HandleList** ppResourceList);
	void UnlockCachePostBootAnimations();

	//Wait for all resource caching to finish.
	void WaitForBootResources();

	//Intended to free all cached global resources, regardless of when loaded.
	void ReleaseAllGlobalResources();


	virtual void PollPlatformCallbacks();


	void BootUiScene();
	bool UiSceneReady();
	void CreateMainLayers();
	bool MainLayersReady();
	void EnableMainLayers();

	void DestroyBootLayer();

#ifdef PLATFORM_PS3
	void SystemMenuOpen();
	void SystemMenuClose();
	bool m_SystemMenuActive;
	bool m_QuitApplication;
#endif
protected:

	void SetupDeviceMemoryParams();

private:
#ifdef _SIMPLE_UI_
	Game::C_SimpleUiLayer* m_pSimpleUiLayer;
#endif
	Game::C_Game* m_pGame;
	UI::C_UiLayer* m_pUiLayer;
	Framework::C_DebugLayer* m_pDebugLayer;

	Game::C_SystemLayer* m_pSystemLayer;

	Game::C_RaceBookLayer* m_pRaceBookLayer;
	Game::C_GameAudioLayer* m_pGameAudioLayer;

	Game::C_BootLayer* m_pBootLayer;

#ifndef CFG_GOLD
	GameAutomationMessageController*	m_pGameAutomationMessageController;
	AmaxPerfLogging::MessageController*	m_pGamePerformanceMessageController;
#endif

	XtCallbackContext* mXtCC;
	XtDb* mGlobalXtDatabase;

	bool m_NetworkStartedByMercury;

	C_ResourceBagHandle m_hSpecialFXResourceBag;

	F32 m_TimeStep;
	F32 m_TimeScale;
	F32 m_TimeScaleTimeOut;
	F32 m_ElapsedRaceTime;

	F32 m_LoadingScreenTimeStep;
	Time::C_Time m_LoadingScreenPrevTime;

	Resource::C_HandleList m_CachedResourceHandleList;

	Resource::C_HandleList m_GlobalCachedResourceHandleList;
	MT::Lock m_globalCachedResourceLock;
	
	DynamicEffect::Textures*	m_pParticleTextures;
	Btl::Vector<FX::I_Effect*>	m_EffectPointers;
	MT::Lock					m_bootGlobalResourcesLock;

	//Indicates whether resource caching has commenced.
	bool m_cachePostBootFxResourceBagStarted;
	bool m_cachePostBootAnimationsStarted;
	bool m_setBootPreCachingComplete;
	bool m_installerScreenEnabled;
	bool m_loadingSegEnabled;
	
#ifndef CFG_GOLD
	static void AppAudioCapture(Cam2::C_CaptureService* self, bool start);
	static S32 AudioCallback(F32 *audio, S32 channels, S32 len);
#endif
};

#endif // GAMEAPP_H
