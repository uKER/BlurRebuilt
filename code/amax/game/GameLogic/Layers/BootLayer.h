#ifndef BOOT_LAYER_H
#define BOOT_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>
#include <Core/Threads/Thread.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <Resource/Resource.h>
#include <game_resource/LevelResource.h>
#include <GameLogic/GameData/GameEventData.h>
#include <Movie/Movie.h>

namespace Time
{
	class C_ScopedTimer;
}

namespace Game
{

enum E_BootMovie
{
	E_BootMovie_ATVI = 0,
	E_BootMovie_Bizarre,
	E_BootMovie_Count
};

#ifdef PLATFORM_PS3
#define ASYNC_CACHING
#endif

#ifdef ASYNC_CACHING
class C_BootLayer;
class cInitPaksThread : public MT::Thread
{
	C_BootLayer	*m_pBootLayer;
public:
	cInitPaksThread() { m_pBootLayer = NULL; }
	void SetBootLayer( C_BootLayer *pBootLayer ) { m_pBootLayer = pBootLayer; }
	void ThreadMain();
};
#endif

void WaitUntilInitialBootCachingCompleted();

class C_BootLayer : public Framework::C_Layer, public Messaging::C_MessageHandler
{
	friend class cInitPaksThread;
//Functions
public:

	C_BootLayer(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);
	virtual ~C_BootLayer();

	HRESULT										ProcessMessage(const Framework::E_LayerMessage message) OVERRIDE;

	HRESULT										Create();
	HRESULT										Destroy(bool shuttingDown = false);
	HRESULT										GlobalUpdate();
	HRESULT										RenderFullscreen();
	HRESULT										RenderOnce();

	void										InitPaks();
	void										InitMovie();
	void										CloseMovie();
	void										RenderMovieFrame();
	void										RenderMovie();
	bool										UpdateMovie();

	bool										SkipButtonPressed();
	void										ResetFade();
	void										StartFade();
	void										UpdateFade();

	void										StartGame();

	virtual bool								MessageHandler(const  Messaging::C_MessageBase* pMessage);

private:
#ifdef PLATFORM_PS3
		typedef enum 
		{
			E_WaitingForCaching,
			E_CheckingSpace,
			E_CopyingAttract,
			E_CopyingData,
			E_BootUiLayer,
			E_None
		} E_BootMode;

		E_BootMode								m_eBootMode;
#else
	bool										m_DoBootUiScene;
#endif
	//bool										m_DoCacheSpecialFx;
	S32											m_FrameCount;
	Time::C_Time								m_StartTime;

	F32											m_Fade;
	F32											m_DeltaFade;

	U32											m_CurrentMovie;
	Movie::C_Movie								m_Movie[E_BootMovie_Count];
	U8*											m_pMovieData[E_BootMovie_Count];
#ifdef ASYNC_CACHING
	cInitPaksThread								m_InitPaksThread;
#endif
#ifndef CFG_GOLD
	Time::C_ScopedTimer*						m_pBootTimer;
#endif
	Sys::C_FilePath								m_bootPakFilePath;

	void										CacheCommon();
	bool										m_initialCachingStarted;
	bool										m_releaseDiskAccessMutexOnMainThread;

};

} // namespace Game

#endif // BOOT_LAYER_H
