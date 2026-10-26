#include "stdafx.h"
#include <system/io/async.h>
#include <system/time/simpletimer.h>
#include <system/time/scopedtimer.h>
#include <OverlayFs/OverlayFs.h>
#include <GameFs/GameHelpers.h>
#include <GameSystems.h>

#include "GameLogic\Layers\BootLayer.h"
#include "Game\SystemLayer.h"
#include "GameApp.h"

#include <Game/GameControls/GameControls.h>
#include <Game/Game.h>
#include <PakNames.h>

#include <Game\Messaging\GameflowMessages.h>
#include "PlatformCallbacks/PlatformCallbacks.h"
#include <framework/application/CoreSystems.h>

#include "GameAudio/GameAudioLayer.h"
#include "ui/uimanager.h"

#ifdef PLATFORM_PS3
#include <PS3Installer/PS3InstallData.h>
#include <cell/l10n.h>
#include <sys/paths.h>
#endif

void PrintTimeSinceAppStart();

//This prevents thrashing occuring before ram-disks are mounted, it will slow down layer creation as the UI layer will block on this.

namespace Game
{

Debug::Bool g_bootResourceManagerPause("Amax.BootLayer.PauseResourceManager", false, Debug::Persist());

Debug::Bool g_bootPakEnable("Amax.BootLayer.BootPakEnable", true, Debug::Persist());
Debug::Bool g_bootRamPakEnable("Amax.BootLayer.BootRamPakEnable", true, Debug::Persist());

Debug::Int g_bootPakBufferSize("Amax.BootLayer.BootPakBufferSize", 128*1024, Debug::Persist());
Debug::Int g_bootPakBufferCount("Amax.BootLayer.BootPakBufferCount", 0, Debug::Persist());

#ifdef ASYNC_CACHING
#define USE_BOOT_DISK_ACCESS_MUTEX
#endif

#ifdef USE_BOOT_DISK_ACCESS_MUTEX

	MT::Mutex g_bootDiskAccessMutex;

	void AcquireBootDiskAccessMutex()
	{
		LOGINFO(g_Log, "AcquireBootDiskAccessMutex %u", (U32)MT::GetThreadId());
		g_bootDiskAccessMutex.Wait();
	}

	void ReleaseBootDiskAccessMutex()
	{
		LOGINFO(g_Log, "ReleaseBootDiskAccessMutex %u", (U32)MT::GetThreadId());
		g_bootDiskAccessMutex.Release();
	}

	void WaitUntilInitialBootCachingCompleted()
	{
		Time::C_ScopedTimer st("WaitUntilInitialBootCachingCompleted()");
		LOGINFO(g_Log, "WaitUntilInitialBootCachingCompleted %u", (U32)MT::GetThreadId());
		g_bootDiskAccessMutex.Wait();
		g_bootDiskAccessMutex.Release();
	}

#else

	void AcquireBootDiskAccessMutex()
	{
	}

	void ReleaseBootDiskAccessMutex()
	{
	}

	void WaitUntilInitialBootCachingCompleted()
	{
	}

#endif


C_BootLayer::C_BootLayer(Messaging::C_MessageSubscriberSystem* pSubscriberSystem) : Framework::C_Layer("C_BootLayer"),
	m_FrameCount(0),
	m_initialCachingStarted(false)
{
	SetSubscriberSystem( pSubscriberSystem );


	//m_DoCacheSpecialFx = false;

#ifdef PLATFORM_PS3
		m_eBootMode = E_WaitingForCaching;
#else
		m_DoBootUiScene = true;

#ifndef CFG_GOLD
		if( g_UiDebugLaunchMode == UI::C_UiManager::E_LaunchMode_Debug || g_UiDebugLaunchMode == UI::C_UiManager::E_LaunchMode_Automation )
			m_DoBootUiScene = false;
#endif

#endif

	m_releaseDiskAccessMutexOnMainThread = false;
	AcquireBootDiskAccessMutex();

#ifndef CFG_GOLD
	m_pBootTimer = new Time::C_ScopedTimer("Boot layer life");
#endif
}

C_BootLayer::~C_BootLayer()
{
#ifndef CFG_GOLD
	SAFE_DELETE_PTR(m_pBootTimer);
#endif
	PrintTimeSinceAppStart();

}

HRESULT C_BootLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch(message)
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;
		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;
		case Framework::E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;
		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;
		case Framework::E_LayerMessage_RenderOnce:
			return RenderOnce();
		break;
	}

	return S_OK;
}

void C_BootLayer::CacheCommon()
{	
#ifndef PLATFORM_PC
	if (g_bootRamPakEnable)
	{
		Time::C_SimpleTimer simpleTimer;
		Sys::C_FileInfo info;

		Sys::C_FilePath ramPak = LogicalFs::GameHelpers::GetPotentiallyInstalledFilePath(BOOT_RAMPAK_FILEPATH);
		LogicalFs::GameHelpers::GetGameFs()->FileInfo(ramPak, &info);
		LogicalFs::GameHelpers::MountRamPakLayer(
			Sys::C_FileSystem::Instance(),
			*C_GameSystems::Instance().GetMemoryFs(),
			*LogicalFs::GameHelpers::GetGameFs(),
			ramPak,
			BOOT_RAMPAK_FILEPATH,
			"/",
			"/");
		F32 sizeMb = (F32)info.m_FileSize / (1024.f*1024.f);
		F32 deltaSecs = simpleTimer.Stop() / 1000.f;

		LOGINFO(g_Log, "%s read in %.2f secs @ %.2fmb/s", ramPak.Chars(), deltaSecs, sizeMb / deltaSecs);
	}
#endif
#ifndef PLATFORM_PC
	Sys::C_OpenParams bootPakParams(
		g_bootPakBufferSize, //massive buffer
		true, //iscritical
		true, //asserteof
		g_bootPakBufferCount, // use default # buffers
		true //use serial access
	);

	//mount boot pak here.
	if (g_bootPakEnable)
	{
		m_bootPakFilePath = 
			LogicalFs::GameHelpers::GetPotentiallyInstalledFilePath(BOOT_PAK_FILEPATH);
		//360 and PS3 in the case of no hdd install present.
		{
			LogicalFs::GameHelpers::MountPakLayer(
				Sys::C_FileSystem::Instance(),
				*LogicalFs::GameHelpers::GetGameFs(),
				m_bootPakFilePath, 
				"/", 
				"/",
				NULL,
				bootPakParams);
		}
	}
#endif

	m_releaseDiskAccessMutexOnMainThread = true;

	MEMORY_TRACKER_PUSH("game.cachedresources");
	C_GameApp::GetInstance()->CacheBootGlobalResources();
	MEMORY_TRACKER_POP("game.cachedresources");

}	

#ifdef ASYNC_CACHING
void cInitPaksThread::ThreadMain()
{
	if (!m_pBootLayer) return;

	m_pBootLayer->CacheCommon();
}
#endif

void C_BootLayer::InitPaks()
{
	m_initialCachingStarted = true;

#ifdef ASYNC_CACHING
	m_InitPaksThread.SetBootLayer( this );
	m_InitPaksThread.Start();
#else
	CacheCommon();
#endif
}

HRESULT C_BootLayer::Create()
{
	//AddSubscriber( E_GameflowMessage_CachePostBootFxResourceBag );

	Resource::C_Manager::GetInstance()->WaitForAllResourceData();

	C_GameApp::GetInstance()->GetSystemLayer()->m_UseRenderMutex = false;


#ifdef PLATFORM_PS3
	C_PS3InstallData::Instance().Initialise();
#endif

	InitMovie();
#ifndef ASYNC_CACHING
	InitPaks();
#endif
	ResetFade();


	C_GameApp::GetInstance()->CreateMainLayers();

	m_StartTime = Time::GetPerformanceCounter();

	return S_OK;
}

HRESULT C_BootLayer::Destroy(bool shuttingDown)
{
	//RemoveSubscriber( E_GameflowMessage_CachePostBootFxResourceBag );

#ifndef PLATFORM_PC
	if (!shuttingDown)
	{
		if (g_bootPakEnable)
		{
			LogicalFs::GameHelpers::UnmountPakLayer(
				LogicalFs::E_MediaLayer_0,
				m_bootPakFilePath,
				*LogicalFs::GameHelpers::GetGameFs());
		}

		if (g_bootRamPakEnable)
		{
			LogicalFs::GameHelpers::UnmountRamPakLayer(
				*C_GameSystems::Instance().GetMemoryFs(),
				*LogicalFs::GameHelpers::GetGameFs(),
				BOOT_RAMPAK_FILEPATH);

			Game::C_GameSystems::Instance().UnloadGamedataRampak();
		}
	}
#endif
	CloseMovie();

	C_GameApp::GetInstance()->GetSystemLayer()->m_UseRenderMutex = true;

	#ifdef ASYNC_CACHING
	if (!shuttingDown)
	{
		m_InitPaksThread.Join();
	}
	#endif
	return S_OK;
}

//#define DISABLE_BOOT_MOVIE
//#define USE_BINK_TO_LOAD_MOVIE_INTO_MEMORY

void C_BootLayer::InitMovie()
{
#ifndef DISABLE_BOOT_MOVIE
	m_CurrentMovie = E_BootMovie_ATVI;


	const char* filenames[E_BootMovie_Count] = 
	{	"Ui\\movies\\atvi.bik",
		"Ui\\movies\\BizarreSplash.bik"
	};


	for(S32 i=0; i<E_BootMovie_Count; i++)
	{
		// rename for 50Hz versions?
		//if ( Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ )
		//	RenameMovieFor50Hz( fileString.Chars(), pFileName, sBufferSize );

		#ifdef USE_BINK_TO_LOAD_MOVIE_INTO_MEMORY
		{
			Movie::C_MovieParams movieParams;

			movieParams.m_CreateFlags = Movie::E_MovieCreateFlags_LoadIntoMemory;
			movieParams.m_MemoryFlags = Movie::E_MovieMemoryFlags_Graphics;

			Sys::C_FilePath systemFp = Sys::GetDefaultFileSystem()->GetSystemFilePath(filenames[i]);

			if ( !m_Movie[i].Open( systemFp.Chars(), movieParams ) )
			{
				LOGERROR(g_Log, "Failed to cerate Movie %s", filenames[i]);
			}
		}
		#else // USE BIZARRE FILE SYSTEM TO LOAD INTO MEMORY
		{
			Movie::C_MovieParams movieParams;

			movieParams.m_CreateFlags = Movie::E_MovieCreateFlags_PlayFromMemory;
			movieParams.m_MemoryFlags = Movie::E_MovieMemoryFlags_Graphics;
		
			if ( !m_Movie[i].Open( filenames[i], movieParams ) )
			{
				LOGERROR(g_Log, "Failed to cerate Movie %s", filenames[i]);
			}
		}

		#endif

		m_Movie[i].SetLooping(false); // Don't want these movies to loop

		// Set the volumes of the movies.
		F32 vol = C_GameApp::GetInstance()->GetGameAudioLayer()->FormatBinkMovieVolume(32768);
		m_Movie[i].SetVolume((S32)vol);
	}

#endif
}

void C_BootLayer::CloseMovie()
{
#ifndef DISABLE_BOOT_MOVIE
	for(S32 i=0; i<E_BootMovie_Count; i++)
	{
		m_Movie[i].Close();
	}
#endif
}

void C_BootLayer::ResetFade()
{
	m_Fade = 1.0f;
	m_DeltaFade = 0.0f;
}

bool C_BootLayer::UpdateMovie()
{
#ifndef DISABLE_BOOT_MOVIE

	//C_GameApp::GetInstance()->MainLayersReady()

	if(m_CurrentMovie < E_BootMovie_Count)
	{
		bool readError = m_Movie[m_CurrentMovie].Update();

		return ( readError || m_Movie[m_CurrentMovie].HasReachedEnd() );
	}
#endif

	return false;
}

void C_BootLayer::RenderMovie()
{
#ifndef DISABLE_BOOT_MOVIE
	if(m_CurrentMovie < E_BootMovie_Count)
	{
		const Display::C_Texture2D* pTexture = m_Movie[m_CurrentMovie].GetTexture();

		Maths::RGBA colour(m_Fade, m_Fade, m_Fade, 1.0f);

		if(Display::Device()->GetWideScreen())
		{
			Display::BasicRender()->DrawTextured2DRect(0.0f, 0.0f, 1024.0f, 768.0f, colour, pTexture);
		}
		else
		{
			//Display::BasicRender()->DrawTextured2DRect(0.0f, 0.0f, 1024.0f, 768.0f, Display::WHITE, pTexture);
			Display::BasicRender()->DrawColoured2DRect(0.0f, 0.0f, 1024.0f, 768.0f, Display::BLACK);

			F32 fHeightRatio = (9 / 16.0f) * (4.0f / 3.0f);
			F32 fBorder = 768.0f * (1.0f - fHeightRatio);

			Display::BasicRender()->DrawTextured2DRect(0.0f, fBorder * 0.5f, 1024.0f, 768.0f * fHeightRatio, colour, pTexture);
		}
	}
#endif
}

void C_BootLayer::RenderMovieFrame()
{
#ifdef PLATFORM_PS3
	if (PlatformCallbacks::SystemMenuOpen)
		return;
#endif
#ifndef DISABLE_BOOT_MOVIE
	if(m_CurrentMovie < E_BootMovie_Count)
	{
		m_Movie[m_CurrentMovie].Render();
	}
#endif
}

bool C_BootLayer::SkipButtonPressed()
{
	if (Input::GetGameInputManager())
	{
#ifdef PLATFORM_PC
		for ( S32 i = 0; i < 1; i++ )
#else
		for ( S32 i = 0; i < 4; i++ )
#endif
		{
			Input::C_GameInput* pInput = Input::GetGameInputManager()->GetGameInput(i);

			if (!pInput)
				continue;

			if (pInput->GetButtonPressedDebounced(Game::E_UiButton_Confirm) || pInput->GetButtonPressedDebounced(Game::E_UiButton_Start))
				return true;
		}
	}

	return false;
}

void C_BootLayer::StartFade()
{
	const F32 frameCount = 16.0f;
	m_DeltaFade = -1.0f / frameCount;
}

void C_BootLayer::UpdateFade()
{
	m_Fade += m_DeltaFade;

	if(m_Fade < 0.0f)
		m_Fade = 0.0f;

	if(m_Fade > 1.0f)
		m_Fade = 1.0f;
}

Debug::Bool g_AllowBoot("Test.AllowBoot", true);
Debug::Bool g_ShowBootInfo("Test.ShowBootInfo", false, Debug::Persist());

HRESULT	C_BootLayer::GlobalUpdate()
{
//	Time::C_Time timeNow = Time::GetPerformanceCounter();
//	Time::C_Time timeSinceStart = timeNow - m_StartTime;
//	Util::C_String elapsedTimeString = Time::FormatProfileDurationString(timeSinceStart);
//	LOGINFO(g_Log, "Boot Screen Running [%d] - %s\n", m_FrameCount, elapsedTimeString.Chars());

	if (m_releaseDiskAccessMutexOnMainThread)
	{
		m_releaseDiskAccessMutexOnMainThread = false;
		ReleaseBootDiskAccessMutex();
	}

	if (!m_initialCachingStarted)
		InitPaks();

	PlatformCallbacks::Poll();

	bool canContinue = C_GameApp::GetInstance()->MainLayersReady();

#ifndef PLATFORM_PS3
	bool bagReady = C_GameApp::GetInstance()->CachePostBootFxResourceBagReady();
	bool layersReady = C_GameApp::GetInstance()->MainLayersReady();
	if (m_initialCachingStarted && bagReady && layersReady)
	{
		C_GameApp::GetInstance()->SetBootPreCachingComplete();
	}
#endif

#ifdef PLATFORM_PS3
	// Wait
	// Don't update if the XMB is open, as blocking occurs
	if (!C_PS3InstallData::Instance().GetXMBActiveState())
	{
		//Give exclusive access to caching until done.
		if (m_eBootMode != E_WaitingForCaching)
		{
			C_PS3InstallData::Instance().Update();
		}

		if( canContinue )
		{
			switch( m_eBootMode )
			{
			case E_WaitingForCaching:
				{
					bool bagReady = C_GameApp::GetInstance()->CachePostBootFxResourceBagReady();
					bool layersReady = C_GameApp::GetInstance()->MainLayersReady();
					if (m_initialCachingStarted && bagReady && layersReady)
					{
						C_GameApp::GetInstance()->SetBootPreCachingComplete();
						m_eBootMode = E_CheckingSpace;
					}
				}
				break;
			case E_CheckingSpace:
				if( C_PS3InstallData::Instance().GetHasCheckedForFreeSpaceUI() == true )
				{
					if( C_PS3InstallData::Instance().ShowInstallingMessage() )
					{
						LOGDEBUG( g_Log, "PS3 - Copy attract" );
						C_PS3InstallData::Instance().CopyAttractMovie();
						m_eBootMode = E_CopyingAttract;
						//Enable the display of the installer screen. Is disabled initially.
						C_GameApp::GetInstance()->EnableInstallerScreen(true);
						LOGINFO(g_Log, "Installer state change: E_CopyingAttract");
					}
					else
					{
						LOGDEBUG( g_Log, "PS3 - skip install" );
						m_eBootMode = E_BootUiLayer;
						LOGINFO(g_Log, "Installer state change: E_BootUiLayer");
					}
				}
				break;

			case E_CopyingAttract:
				if( !C_PS3InstallData::Instance().IsCopyInProgress() )
				{
					if( C_PS3InstallData::Instance().ShowInstallingMessage() )
					{
						LOGDEBUG( g_Log, "PS3 - Copy data" );
						m_eBootMode = E_CopyingData;
						LOGINFO(g_Log, "Installer state change: E_CopyingData");
						C_PS3InstallData::Instance().CopyMainInstallData();
					}
					else
					{
						LOGDEBUG( g_Log, "PS3 - Skip data copy" );
						m_eBootMode = E_BootUiLayer;
						LOGINFO(g_Log, "Installer state change: E_BootUiLayer");
					}
				}
				break;

			case E_CopyingData:
				if( !C_PS3InstallData::Instance().IsCopyInProgress() )
				{
					LOGDEBUG( g_Log, "PS3 - Done" );
					m_eBootMode = E_BootUiLayer;
					LOGINFO(g_Log, "Installer state change: E_BootUiLayer");
				}
				break;

			case E_BootUiLayer:
				m_eBootMode = E_None;
				LOGINFO(g_Log, "Installer state change: E_None");
				C_GameApp::GetInstance()->BootUiScene();
				break;

			default:
				break;
			}

			// We can progress if we're at the data copy stage
			canContinue = ( m_eBootMode == E_WaitingForCaching || m_eBootMode == E_CopyingData || m_eBootMode == E_None );
		}
	}
#else
	if( canContinue )
	{	
		if( m_DoBootUiScene )
		{
			C_GameApp::GetInstance()->BootUiScene();
			m_DoBootUiScene = false;
		}
	}
#endif
//	if( canContinue )
//	{	
//		if (m_DoCacheSpecialFx)
//		{
//			C_GameApp::GetInstance()->CachePostBootFxResourceBag();
//			m_DoCacheSpecialFx = false;
//		}
//	}

	m_FrameCount++;

	if (m_initialCachingStarted && C_GameApp::GetInstance()->CachePostBootFxResourceBagReady() && C_GameApp::GetInstance()->MainLayersReady())
	{
		if(SkipButtonPressed())
		{
			StartFade();
		}
	}

	UpdateFade();

	bool goToNextMovie = false;

	if(UpdateMovie())
	{
		goToNextMovie = true;
	}

	if(m_Fade == 0.0f)
	{
		goToNextMovie = true;
	}

	if(goToNextMovie)
	{
		if(m_CurrentMovie < E_BootMovie_Count)
		{
			m_CurrentMovie++;
			ResetFade();
		}
	}

	bool allMoviesFinished = (m_CurrentMovie == E_BootMovie_Count) ? true : false;

#ifdef DISABLE_BOOT_MOVIE
	allMoviesFinished = true;
#endif

#ifndef CFG_GOLD
	// Skip movies for debug
	if( g_UiDebugLaunchMode == UI::C_UiManager::E_LaunchMode_Debug || g_UiDebugLaunchMode == UI::C_UiManager::E_LaunchMode_Automation )
		allMoviesFinished = true;
#endif

	bool startGame = true;
	if(allMoviesFinished && canContinue)
	{
		startGame = true;
	}
	else
	{
		startGame = false;
	}

	if(!g_AllowBoot)
		startGame = false;

	if(startGame)
	{
		StartGame();
	}

	return S_OK;
}

void DrawBorder(Maths::RGBA colour)
{
	//DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1024.0f, 768.0f), colour);

	F32 xSize = 64.0f;
	F32 ySize = 64.0f;
	
	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(xSize, 768.0f), colour);
	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(1024.0f-xSize, 0.0f), Maths::HVec2(xSize, 768.0f), colour);
	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(1024.0f, ySize), colour);
	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(0.0f, 768.0f - ySize), Maths::HVec2(1024.0f, ySize), colour);
}

HRESULT	C_BootLayer::RenderOnce()
{
	RenderMovieFrame();

	return S_OK;
}

HRESULT	C_BootLayer::RenderFullscreen()
{
	RenderMovie();

#ifndef CFG_GOLD
	if(g_ShowBootInfo)
	{
		if(C_GameApp::GetInstance()->MainLayersReady())
		{		
			DrawBorder(Maths::RGBA(0,0xff,0,0xff));
			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(32.0f, 720.0f), "Loaded", Maths::RGBA(0xff,0xff,0xff,0xff), 24.0f);
		}
		else
		{
			DrawBorder(Maths::RGBA(0xff,0,0,0xff));
			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(32.0f, 720.0f), "Loading...", Maths::RGBA(0xff,0xff,0xff,0xff), 24.0f);
		}

		Time::C_Time timeNow = Time::GetPerformanceCounter();
		Time::C_Time timeSinceStart = timeNow - m_StartTime;
		Util::C_String elapsedTimeString = Time::FormatDurationString(timeSinceStart);

		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(512.0f, 720.0f), elapsedTimeString.Chars(), Maths::RGBA(0xff,0xff,0xff,0xff), 24.0f);

		F32 timeS = (F32)timeSinceStart / 1000000.0f;
		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dBar(Maths::HVec2(512.0f + sinf(timeS) * 400.0f, 32.0f), Maths::HVec2(16.0f, 16.0f), Maths::RGBA(0xff,0xff,0xff,0xff));
	}
#endif

	return S_OK;
}

void C_BootLayer::StartGame()
{
	C_GameApp::GetInstance()->EnableMainLayers();

	if (!C_GameApp::GetInstance()->UiSceneReady())
		return;

	C_GameApp::GetInstance()->DestroyBootLayer();
}

bool C_BootLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_BootLayer::MessageHandler");
/*
	switch( pMessage->Type() )
	{
		case E_GameflowMessage_CachePostBootFxResourceBag:
			m_DoCacheSpecialFx = true;
			C_GameApp::GetInstance()->CachePostBootFxResourceBag();
			return true;

		default:
			break;
	};
*/
	return false;
}

} // namespace Game
