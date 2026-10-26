#include "Stdafx.h"

#include "system/time/simpletimer.h"

#include <Game/GameControls/GameControls.h>
#include <Game/Messaging\GameFlowMessages.h>
#include "Game/Game.h"
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <RaceBook/RaceBookManager.h>

#include <framework/application/ApplicationBase.h>
#include "GameApp.h"

#include "Ui/UiMessages.h"
#include "Ui/UiLayer.h"
#include "Ui/UiRenderBuffer.h"
#include "Ui/UiPostProcessBuffer.h"
#include "Ui/UiXtConfig.h"

#include <Render/Render/GridRender.h>

#include <Camera2.h>

#include <Game\GameWorld\PostProcessLayer.h>
#include <Game\GameWorld\ViewManager.h>
#include <Render/PostProcess/Manager.h>
#include <Render/View/ViewManager.h>
#include <Render/PostProcess/Bloom/Bloom.h>

#include "Ui/Game/Gadgets/Vinyl/UIColourPicker.h"
#include "Ui/UIHelpers.h"

#include "Automation\GamePerfLogging.h"

#include "Ui/LightPaintSystem.h"

#include <Scenegraph/View.h>
#include <Resource/Resource.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#ifndef CFG_GOLD
Debug::Bool g_DebugShowUiTexture("UI.ShowUiTexture", false);
Debug::Bool g_DebugDownsampleFirst( "UI.DownsampleFirst", true );
Debug::Bool g_DebugShowBloom( "UI.ShowBloomed", true );
Debug::Bool g_DebugShowUnBloomed( "UI.ShowUnBloomed", true );
#endif

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
extern Debug::Bool g_debugOKToDumpDumpResources;
#endif

// Global UI render buffer switch (DOF + bloom + render buffer)
// [Blur2PC] The snapshot defined _DISABLE_GFX_BUFFERS on PC. Retail Blur.exe 1.2 compiles and calls
// both C_UiLayer::CreateFePostProcessBuffer and CreateIgPostProcessBuffer (Ghidra: the
// "UiPostProcessBuffer" renderer constructor at 0x00c30210 is called from 0x00c303e0 and 0x00c30470),
// so retail keeps the UI render buffers on. Without them, RenderOnce drew the HUD with no target of
// its own, into whatever the blob shadow pre-render left bound. Define removed to match retail.

EXTERN_SHADER(psBloomComposite);

namespace Game
{
	extern Debug::Bool g_bootResourceManagerPause;
	//From BootLayer.cpp
	void WaitUntilInitialBootCachingCompleted();
}

namespace UI
{

C_UiLayer::C_UiLayer( ) : Framework::C_Layer("C_UiLayer"),
	m_IsFrontend( true ),
	m_LoadingFrontend( false ),
	m_PostProcessing( true ),
	m_pUiManager( NULL ),
	m_pFeCompositeRB( NULL ),
	m_pFeCompositeRT( NULL ),
	m_pIgCompositeRT( NULL ),
	m_pBloom( NULL ),
	m_pPostProcessMgr( NULL )
{
	// Game messages - These are *only* dealt with by the main thread
	SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );

	AddSubscriber( Game::E_GameflowMessage_StartUILoad );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
	AddSubscriber( Game::E_GameflowMessage_LevelDumped );
	AddSubscriber( Game::E_GameflowMessage_UILoaded );
	AddSubscriber( Game::E_GameflowMessage_XtInitialised );
	AddSubscriber( Game::E_GameflowMessage_SyncBootAssets );
	AddSubscriber( Game::E_GameflowMessage_StreamerFull );

	// UI messages - Note : These can be received on the loading thread
	SetSubscriberSystem( &gButtons.GetSubscriberSystem() );

	AddSubscriber( E_UiGameMsg_IngameRenderAA );
	AddSubscriber( E_UiGameMsg_UiCreateResources );
	AddSubscriber( E_UiGameMsg_UiDestroyResources );

	m_SchedulerGroup = Scheduler::CreateGroup();

	for( U32 i = 0; i < m_MaxLocalPlayers; i++ )
		m_pMiniMapRT[ i ] = NULL;
}

C_UiLayer::~C_UiLayer( )
{
	Scheduler::DestroyGroup( m_SchedulerGroup );
}

void C_UiLayer::CreateBloom( )
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	if( m_pPostProcessMgr )
		return;

	MEMORY_TRACKER_CONTEXT("ui.rendertargets");

	m_pPostProcessMgr = new PostProcess::C_Manager();
	m_pPostProcessMgr->SetName("UI");

	m_pBloom = new PostProcess::C_Bloom( *m_pPostProcessMgr );
	m_pBloomParameters = new PostProcess::BloomParameters;
	
	/* create pixel shader for fast UI composite. */
	Display::CreatePixelShaderFromFunc("Shaders/ui/psBloomComposite", 0, &m_fastBloomCompositeShader, REF_SHADER(psBloomComposite));
	BIZ_ASSERT(m_fastBloomCompositeShader.IsCreated());		/* hopefully this got created. */

	SetupViews();

	m_pPostProcessMgr->Create();
	m_pPostProcessMgr->CreateVolatile();
}

void C_UiLayer::SetupViews()
{
	m_pPostProcessMgr->ClearViews();

	// [Blur2PC] in-game size follows the composite buffer (back buffer / 4, as retail sizes it); the
	// fixed 1280/4 x 720/4 constants only matched a 1280x720 back buffer
	const U32 sizeX = m_IsFrontend ? Display::Device()->GetBackBuffer().GetWidth() : (Display::Device()->GetBackBuffer().GetWidth() >> 2);
	const U32 sizeY = m_IsFrontend ? Display::Device()->GetBackBuffer().GetHeight() : (Display::Device()->GetBackBuffer().GetHeight() >> 2); 

	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(1);
	if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnablePostProcess)
	{
		const Display::C_Viewport& viewport = pView->m_View.GetViewport();
		m_pPostProcessMgr->AddView(sizeX, sizeY, 0, PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
	}
}

void C_UiLayer::DestroyBloom( )
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	if( !m_pPostProcessMgr )
		return;

#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif	

	m_pBloom->DestroyVolatile();
	m_pBloom->Destroy();

	/* destroy pixel shader */
	Display::Device()->DestroyPixelShader(&m_fastBloomCompositeShader);

	SAFE_DELETE_PTR( m_pBloom );
	SAFE_DELETE_PTR( m_pBloomParameters );
	SAFE_DELETE_PTR( m_pPostProcessMgr );
}

HRESULT C_UiLayer::Create( )
{
	if (Game::g_bootResourceManagerPause)	Resource::C_Manager::GetInstance()->PauseLoadingThread();

	Game::WaitUntilInitialBootCachingCompleted();

#ifdef CFG_GOLD
	m_pUiManager = new C_UiManager( C_UiManager::E_LaunchMode_Master );
#else
	m_pUiManager = new C_UiManager( g_UiDebugLaunchMode );
#endif
	if( !m_pUiManager )
	{
		LOGFATAL( g_Log, DBSTR "Failed to allocated UI Manager");
		return 1;
	}

	CreateBloom();
	CreateRenderBuffer();
	CreateFePostProcessBuffer();

	LightPaint_Init(32, 60);

	m_pUiManager->LoadFrontendTextureBundle( true );

	//MEMORY_TRACKER_PUSH("game.cachedresources");
	//C_GameApp::GetInstance()->CachePostBootAnimations();
	//MEMORY_TRACKER_POP("game.cachedresources");

	if (Game::g_bootResourceManagerPause)	Resource::C_Manager::GetInstance()->ResumeLoadingThread();

	//C_GameApp::GetInstance()->CachePostBootFxResourceBag();

	return S_OK;
}

void C_UiLayer::CreateMiniMapRenderTarget( const U32 localPlayerIndex, const U32 size )
{
	// Already exists?
	if( m_pMiniMapRT[ localPlayerIndex ] )
		return;

	C_UiRenderBuffer* p = CreateRenderBufferImpl( size, size );
	 if( !p )
		 return;

	 // Hook up texture
	if( m_pUiManager )
		m_pUiManager->SetMiniMapRenderTarget( &p->GetTexture(), localPlayerIndex ); // Note clear this when we go into the game by pasing NULL

	// Store
	m_pMiniMapRT[ localPlayerIndex ] = p;
}

void C_UiLayer::CreateRenderBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	if( m_pFeCompositeRB )
		return;

	m_pFeCompositeRB = CreateRenderBufferImpl();

	if( m_pUiManager && m_pFeCompositeRB )
		m_pUiManager->SetRenderBuffer( &m_pFeCompositeRB->GetTexture() ); // Note clear this when we go into the game by pasing NULL
}

C_UiRenderBuffer* C_UiLayer::CreateRenderBufferImpl( U32 sizeX, U32 sizeY )
{
	C_UiRenderBuffer* rb = NULL;

	RESOURCE_MEMORY_ID( "UiRenderBuffer" );
	MEMORY_TRACKER_CONTEXT("ui.rendertargets");

	if( rb )
	{
		LOGWARN( g_Log, DBSTR "UI render buffer already created" );
		return rb;
	}

	rb = new C_UiRenderBuffer();
	if( !rb )
	{
		LOGWARN( g_Log, DBSTR "Failed to create UI render buffer" );
		return NULL;
	}
	rb->SetWidth( (!sizeX) ? Display::Device()->GetBackBuffer().GetWidth() : sizeX );
	rb->SetHeight( (!sizeY) ? Display::Device()->GetBackBuffer().GetHeight() : sizeY );

#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	rb->SetMultisample( Display::E_Multisample_None );
#else
	rb->SetMultisample( Display::E_Multisample_4 );
#endif

	// Manually do these as this function can be called anywhere
	rb->Create();
	rb->CreateVolatile();

	return rb;
}

void C_UiLayer::DestroyMiniMapRenderTarget( const U32 localPlayerIndex )
{
	// Doesn't exist
	if( !m_pMiniMapRT[ localPlayerIndex ] )
		return;

	DestroyRenderBufferImpl( m_pMiniMapRT[ localPlayerIndex ] );
	m_pMiniMapRT[ localPlayerIndex ] = NULL;

	// Clear
	if( m_pUiManager )
		m_pUiManager->SetMiniMapRenderTarget( NULL, localPlayerIndex );
}

void C_UiLayer::DestroyRenderBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	if( !m_pFeCompositeRB )
		return;

	DestroyRenderBufferImpl( m_pFeCompositeRB );
	m_pFeCompositeRB = NULL;

	if( m_pUiManager )
		m_pUiManager->SetRenderBuffer( NULL );
}

void C_UiLayer::DestroyRenderBufferImpl( C_UiRenderBuffer* rb )
{
	if( !rb )
	{
		LOGWARN( g_Log, DBSTR "UI render buffer already destroyed" );
		return;
	}

	// Manually do these as this function can be called anywhere
	rb->Destroy();
	rb->DestroyVolatile();

	SAFE_DELETE_PTR(rb);
}


void C_UiLayer::CreateFePostProcessBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	RESOURCE_MEMORY_ID( "UiPostProcess" );
	MEMORY_TRACKER_CONTEXT("ui.rendertargets");

	if( m_pFeCompositeRT )
	{
		LOGWARN( g_Log, DBSTR "UI post process buffer already created" );
		return;
	}

	m_pFeCompositeRT = new C_UiPostProcessBuffer();
	if( !m_pFeCompositeRT )
	{
		LOGWARN( g_Log, DBSTR "Failed to create UI post process buffer" );
		return;
	}
	m_pFeCompositeRT->SetWidth( Display::Device()->GetBackBuffer().GetWidth() );
	m_pFeCompositeRT->SetHeight( Display::Device()->GetBackBuffer().GetHeight() );

	// Manually do these as this function can be called anywhere
	m_pFeCompositeRT->Create();
	m_pFeCompositeRT->CreateVolatile();
}

void C_UiLayer::CreateIgPostProcessBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif
	
	RESOURCE_MEMORY_ID( "UiPostProcess" );
	MEMORY_TRACKER_CONTEXT("ui.rendertargets");

	if( m_pIgCompositeRT )
	{
		LOGWARN( g_Log, DBSTR "Ingame post process buffer already created" );
		return;
	}

	m_pIgCompositeRT = new C_UiPostProcessBuffer();
	if( !m_pIgCompositeRT )
	{
		LOGERROR( g_Log, DBSTR "Failed to create Ingame post process buffer" );
		return;
	}
	if( m_IsFrontend )
	{
		LOGERROR( g_Log, DBSTR " : We're in frontend mode" );
		return;
	}

	// [Blur2PC] retail (0x00c30470) sizes the in-game composite from the back buffer, a quarter each way
	m_pIgCompositeRT->SetWidth( Display::Device()->GetBackBuffer().GetWidth() >> 2 );
	m_pIgCompositeRT->SetHeight( Display::Device()->GetBackBuffer().GetHeight() >> 2 );
	m_pIgCompositeRT->SetUseDepth( false );
#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)
	m_pIgCompositeRT->SetMultiSample( Display::E_Multisample_None );
#else
	m_pIgCompositeRT->SetMultiSample( Display::E_Multisample_4 );
#endif

	if( m_pUiManager )
		m_pUiManager->SetIgRenderTarget( &m_pIgCompositeRT->GetTexture() );
	
	// Manually do these as this function can be called anywhere
	m_pIgCompositeRT->Create();
	m_pIgCompositeRT->CreateVolatile();
}

void C_UiLayer::DestroyIgPostProcessBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif
	if( !m_pIgCompositeRT )
	{
		LOGERROR( g_Log, DBSTR "Ingame post process buffer already destroyed" );
		return;
	}
	if( m_IsFrontend )
	{
		LOGERROR( g_Log, DBSTR " : We're in frontend mode" );
		return;
	}

	if( m_pUiManager )
		m_pUiManager->SetIgRenderTarget( NULL );

	// Manually do these as this function can be called anywhere
	m_pIgCompositeRT->Destroy();
	m_pIgCompositeRT->DestroyVolatile();

	SAFE_DELETE_PTR( m_pIgCompositeRT );
}

void C_UiLayer::DestroyFePostProcessBuffer()
{
#if defined ( _DISABLE_GFX_BUFFERS )
	return;
#endif

	if( !m_pFeCompositeRT )
	{
		LOGWARN( g_Log, DBSTR "UI post process buffer already destroyed" );
		return;
	}

	// Manually do these as this function can be called anywhere
	m_pFeCompositeRT->Destroy();
	m_pFeCompositeRT->DestroyVolatile();

	SAFE_DELETE_PTR( m_pFeCompositeRT );
}

HRESULT C_UiLayer::Destroy( )
{
	DestroyBloom();
	DestroyRenderBuffer();

	DestroyFePostProcessBuffer();
	DestroyIgPostProcessBuffer();

	if( m_pPostProcessMgr )
	{
		m_pPostProcessMgr->Destroy();
		m_pPostProcessMgr->DestroyVolatile();

		SAFE_DELETE_PTR( m_pPostProcessMgr );
	}

	SAFE_DELETE_PTR( m_pUiManager );

	return S_OK;
}

HRESULT C_UiLayer::OnResize()
{
	if( !m_pPostProcessMgr )
		return S_OK;

	m_pPostProcessMgr->DestroyVolatile();

	// [Blur2PC] retail (Blur.exe 1.2 0x00c30f00) also remakes the UI composite buffers, which are sized from
	// the back buffer when created: the in-game one, or the front end's render buffer and post-process buffer
	if( !m_IsFrontend )
	{
		DestroyIgPostProcessBuffer();
	}
	else
	{
		DestroyRenderBuffer();
		DestroyFePostProcessBuffer();
	}

	SetupViews();

	if( m_IsFrontend )
	{
		CreateRenderBuffer();
		CreateFePostProcessBuffer();
		m_pPostProcessMgr->CreateVolatile();
		return S_OK;
	}

	CreateIgPostProcessBuffer();
	m_pPostProcessMgr->CreateVolatile();

	return S_OK;
}

HRESULT C_UiLayer::GlobalUpdate()
{
	PERF_EVENT("UI.UiLayer::Update");
	
	if( Framework::C_ApplicationBase::GetInstance()->IsPaused(Framework::E_LayerPause_DebugMenu) )
		return S_OK;

	Time::C_SimpleTimer uiUpdateTimer;

//	if( Framework::C_ApplicationBase::GetInstance()->IsPaused(Framework::E_LayerPause_Freeze) )
//		return S_OK;

	RefreshXtConfig();

	Maths::C_TimeS frameTime = C_GameApp::GetInstance()->GetTimeStepRaw();

	if(C_GameApp::GetInstance()->LoadingScreenEnabledState())
		frameTime = C_GameApp::GetInstance()->GetLoadingScreenTimestep();

	m_pUiManager->SetSchedulerGroup( m_SchedulerGroup );

	m_pUiManager->Process( frameTime );

	float uiUpdateTimerDuration = uiUpdateTimer.Stop();
	AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kUiUpdate, uiUpdateTimerDuration);

	return S_OK;
}

void C_UiLayer::IgRenderMiniMap( const U32 localPlayerIndex )
{
	PERF_EVENT("UI.RenderMiniMap");

	if( !gSimple2D.ShouldRender( E_GameSpritePanel_RT0 + localPlayerIndex ) )
		return;

	C_UiRenderBuffer* p = m_pMiniMapRT[ localPlayerIndex ];
	if( !p || !p->IsRendererCreated() ) 
		return;

	if( !p->Bind() )
		return;

	Display::Device()->SetupView(&m_View.m_View, m_View.m_ViewSetup, p->GetWidth(), p->GetHeight());
	Display::Device()->SetView(&m_View.m_View);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));

	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	
	m_pUiManager->RenderRT( localPlayerIndex );

	p->Unbind();
}

void C_UiLayer::FeRenderComposite()
{
	PERF_EVENT("UI.RenderComposite");

	if( !m_pFeCompositeRB || !m_pFeCompositeRB->IsRendererCreated() )
		return;

	if( !m_pFeCompositeRB->Bind() )
		return;

	Display::Device()->SetupView(&m_View.m_View, m_View.m_ViewSetup, m_pFeCompositeRB->GetWidth(), m_pFeCompositeRB->GetHeight());
	Display::Device()->SetView(&m_View.m_View);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(0, 0, 0, 0));

	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	//---------------------------------------
	// Render the AA layer to a texture
	//---------------------------------------
	m_pUiManager->RenderAA();

	m_pFeCompositeRB->Unbind();
}

void C_UiLayer::FeRenderPost()
{
	PERF_EVENT("UI.RenderPost");

	if( !m_pPostProcessMgr || !m_pFeCompositeRB )
		return;

	if( !m_PostProcessing )
		return;

	// Resolve back to be composited later (in RenderFullScreen)
#ifndef PLATFORM_PS3
	if( m_pFeCompositeRT )
		m_pFeCompositeRT->Bind();
#endif
	
	// Begin post
	const Display::C_View* pView = Display::Device()->GetView();
	static Display::C_Viewport vp;
	vp.m_X = 0;
	vp.m_Y = 0;
	vp.m_W = Display::Device()->GetBackBuffer().GetWidth();
	vp.m_H = Display::Device()->GetBackBuffer().GetHeight();

	m_pPostProcessMgr->Begin(0, vp);

	// Render post
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));

	Display::Quad::DrawFullScreen( &m_pFeCompositeRB->GetTexture() );

	// End Post
	Display::Device()->SetBlendMode( Display::E_BlendMode_Solid );

	// Do Bloom
	m_pBloomTexture = m_pBloom->Display(*m_pBloomParameters);
	m_pPostProcessMgr->SetActive(false);
	m_pPostProcessMgr->End();

#ifndef PLATFORM_PS3
	if( m_pFeCompositeRT )
		m_pFeCompositeRT->Unbind();
#endif
}

void C_UiLayer::IgRenderComposite()
{
	PERF_EVENT("UI.RenderComposite");

	if( !m_pIgCompositeRT || !m_pIgCompositeRT->IsRendererCreated() )
		return;

	if( !m_pIgCompositeRT->Bind() )
		return;

	Display::Device()->SetupView(&m_View.m_View, m_View.m_ViewSetup, m_pIgCompositeRT->GetWidth(), m_pIgCompositeRT->GetHeight());
	Display::Device()->SetView(&m_View.m_View);

	// [Blur2PC] retail (C_UiLayer::IgRenderComposite, Blur.exe 0x00c30c20) clears the colour only. The
	// composite buffer has no depth surface, and on D3D9 a clear that names the depth or stencil buffer
	// when none is bound fails as a whole, so the snapshot's Colour|Depth|Stencil clear never cleared
	// the buffer and the HUD accumulated across frames.
	Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0, 0, 0, 0));
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	//---------------------------------------
	// Render the AA layer to a texture
	//---------------------------------------
	m_pUiManager->RenderAA();

	m_pIgCompositeRT->Unbind( true );
}

void C_UiLayer::IgRenderPost()
{
	PERF_EVENT("UI.RenderPost");

	if( !m_pPostProcessMgr || !m_pIgCompositeRT )
		return;

	if( !m_PostProcessing )
		return;

	// Begin post
	const Display::C_View* pView = Display::Device()->GetView();
	static Display::C_Viewport vp;
	vp.m_X = 0;
	vp.m_Y = 0;
	vp.m_W = m_pIgCompositeRT->GetWidth();
	vp.m_H = m_pIgCompositeRT->GetHeight();

	m_pPostProcessMgr->Begin(0, vp);

	Display::Device()->SetBlendMode( Display::E_BlendMode_Solid );
	Display::Quad::DrawFullScreen( &m_pIgCompositeRT->GetTexture() );

	// Do Bloom
	// [Blur2PC] retail (0x00c2fb20) keeps the bloom result for RenderFullscreen and deactivates the manager
	m_pBloomTexture = m_pBloom->Display(*m_pBloomParameters);
	m_pPostProcessMgr->SetActive(false);
	m_pPostProcessMgr->End();
}

// Thread safe Deferred game messages sent for loading screen
void C_UiLayer::AddDeferredGameMessage( Messaging::TMsgType type )
{
	m_CriticalSecion.Enter();
	
	m_DeferredMessages.PushBack( type );

	m_CriticalSecion.Exit();
}

void C_UiLayer::ProcessDeferredGameMessages()
{
	m_CriticalSecion.Enter();

	if( m_DeferredMessages.Size() )
	{
		for( S32 i = 0; i < m_DeferredMessages.Size(); i++ )
		{
			Messaging::TMsgType type = m_DeferredMessages[ i ];

			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( type );
			RaceBook::C_RaceBookManager::Instance().GetSubscriberSystem()->BroadcastMessage( type );
		}

		m_DeferredMessages.Clear();
	}

	m_CriticalSecion.Exit();
}

HRESULT C_UiLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	Memory::MemoryTrackerContext mtUi("ui");

	switch( message )
	{
	case Framework::E_LayerMessage_Create:
		return Create();

	case Framework::E_LayerMessage_Destroy:
		return Destroy();

	case Framework::E_LayerMessage_GlobalUpdate:
		{
			HRESULT ret = S_OK;

			if( !C_GameApp::GetInstance()->LoadingScreenEnabledState() )
				ret = GlobalUpdate();

			ProcessDeferredGameMessages();

			return ret;
		}
	
	case Framework::E_LayerMessage_Resize:
		return OnResize();

	case Framework::E_LayerMessage_RenderOnce:
		return RenderOnce();

	case Framework::E_LayerMessage_BeginRenderSplitscreenWithoutAA:
		break;

	case Framework::E_LayerMessage_RenderSplitscreenWithoutAA:
		break;

	case Framework::E_LayerMessage_EndRenderSplitscreenWithoutAA:
		break;

	case Framework::E_LayerMessage_RenderFullscreen:
		return RenderFullscreen();
	}

	return S_OK;
}

HRESULT C_UiLayer::RenderOnce()
{
	Scheduler::Flush( m_SchedulerGroup );

	m_pUiManager->PreRender();

	if( m_IsFrontend )
	{
		// Render stuff to be composted over game world
		FeRenderComposite();
		FeRenderPost();
	}
	else
	{
		if( m_pIgCompositeRT )
		{
			// Render the mini map stuff to RT
			Game::C_LevelState& levelState = GetLevelState();
			for( U32 i = 0; i < levelState.mNumViewports; i++ )
				IgRenderMiniMap( i );

			// Render ingame bloomed HUD stuff
			IgRenderComposite();

			// Bloom and resolve
			m_pIgCompositeRT->Bind();
			IgRenderPost();
			m_pIgCompositeRT->Unbind();

#if PLATFORM_PS3
			m_pIgCompositeRT->Bind();
			Display::Device()->SetupView(&m_View.m_View, m_View.m_ViewSetup, m_pIgCompositeRT->GetWidth(), m_pIgCompositeRT->GetHeight());
			Display::Device()->SetView(&m_View.m_View);
			Display::Device()->Clear(Display::E_Clear_Colour|Display::E_Clear_Depth|Display::E_Clear_Stencil, Maths::RGBA(0, 0, 0, 0));
			Display::Device()->SetDepthEnable(Display::E_Bool_False);
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
			Display::Quad::DrawFullScreen(&m_pPostProcessMgr->GetRenderTexture()->GetTexture());
			m_pIgCompositeRT->Unbind();
#endif
		}
		else
		{
			// Render ingame composite stuff
			m_pUiManager->RenderAA();
		}
	}
	return S_OK;
}

HRESULT C_UiLayer::RenderFullscreen()
{
	Scheduler::Flush( m_SchedulerGroup );

	// Pre render 2D
	{
		PERF_EVENT("UI.Render2DBG");
		m_pUiManager->Render2DBG();
	}

	if( m_IsFrontend )
	{
		if( m_pFeCompositeRB && m_pFeCompositeRT && m_PostProcessing )
		{
			/* changed the shader to do single pass composite, bloom RT in sampler 1
			 *
			 * Stand back kiddos it's pre-multiplied alpha man to the rescue! 
			 */
			Display::Device()->SetTexture(1, m_pBloomTexture);
			Display::Device()->SetBlendMode( Display::E_BlendMode_AlphaPreMultiply );
			Display::Quad::DrawFullScreen(&m_pFeCompositeRB->GetTexture(), &m_fastBloomCompositeShader);
		}
		else
			m_pUiManager->RenderAA();
	}
	else
	{
#ifndef CFG_GOLD
		if( g_DebugShowUnBloomed )
#endif
			m_pUiManager->RenderAA();

#ifndef CFG_GOLD
		if( g_DebugShowBloom )
#endif
			if( m_pIgCompositeRT )
			{
				Display::Device()->SetBlendMode( Display::E_BlendMode_Additive );
				Display::Device()->SetAlphaTestEnable( Display::E_Bool_False );

#if !defined ( _DISABLE_GFX_BUFFERS )
				// [Blur2PC] retail (C_UiLayer::RenderFullscreen, 0x00c305c0) adds the bloom result here,
				// not the composite itself (which would draw the HUD a second time)
				Display::Quad::DrawFullScreen( m_pBloomTexture );
#endif

			}
	}

	// Render non-AA
	{
		PERF_EVENT("UI.RenderNonAA");
		m_pUiManager->RenderNonAA();
	}

	if( m_IsFrontend )		// [STOVE]: Only in frontend, can't afford to pay ~2ms main thread time in game.
	{
		if (!C_GameApp::GetInstance()->LoadingScreenEnabledState())
		{
			if (Game::C_GameWorld* gw = GetGameWorld())
			{
				if (Game::C_ViewManager* vm = gw->GetViewManager())
				{
					if (SceneGraph::C_View* view = vm->GetLightTrailsView())
					{
						PERF_EVENT("UI.LightTrailParticles");

						const Framework::C_View& view3d = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);

						Display::Device()->SetView(&view3d.m_View);

						view->SetViewData(view3d.m_View.GetViewSettings3D());
						view->SetViewFrustum(view3d.m_View.GetViewFrustum());

						view->BeginGather();
						view->EndGather();

						view->BeginRender();
						view->RenderPass(Game::E_RenderPass_Solid);
						view->RenderPass(Game::E_RenderPass_EffectDecal);
						view->RenderPass(Game::E_RenderPass_Alpha);
						view->EndRender();
					}
				}
			}
		}
	}

	return S_OK;
}


void C_UiLayer::SetBloomParameters( const C_UiXtBloomParameters& xtParams )
{
	if ( !m_pBloom )
		return;

	PostProcess::BloomParameters params;

	memset( &params, sizeof( PostProcess::BloomParameters ), 0 );

	// Copy from Sorcery params
	params.fBloomDeviation = 0.0f;
	for( S32 i = 0; i < xtParams.m_BloomScale.count && i < 4; i++ )
		params.fBloomScale[ i ] = xtParams.m_BloomScale[ i ];
	for( S32 i = 0; i < xtParams.m_DownsampleScale.count && i < 4; i++ )
		params.fDownsampleScale[ i ] = xtParams.m_DownsampleScale[ i ];

#ifndef CFG_GOLD
	params.bDownsample[0] = g_DebugDownsampleFirst;
#endif

	*m_pBloomParameters = params;
	m_pBloomParameters->Update();
		
	//m_pBloom->SetBloomParameters( params );
}

void C_UiLayer::RefreshXtConfig()
{
	C_UiXtConfig* pConfig = C_UiXtConfig::Instance();

	if ( !pConfig || !pConfig->m_bDirty )
		return;

	pConfig->m_bDirty = false;

	// bloom parameters
	SetBloomParameters( m_IsFrontend ? pConfig->m_FrontendBloomParameters : pConfig->m_IngameBloomParameters );

	// colour picker params
	C_UIColourPicker::Instance().BuildColours( 
		pConfig->m_ColourPickerParameters.m_sBoxSize,
		pConfig->m_ColourPickerParameters.m_sSizeV,
		pConfig->m_ColourPickerParameters.m_sSpin
	);

	// Perks
	pConfig->m_PerkDisplayParameters.m_PerkInfoNitrous.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoShock.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoShunt.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoBarge.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoBomb.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoBlindfire3.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoBlindfire2.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoBlindfire1.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoRepair.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoShield.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoSuperShield.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoEmpty.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoExtraBlindfire4.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoExtraBlindfire3.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoExtraBlindfire2.Resolve();
	pConfig->m_PerkDisplayParameters.m_PerkInfoExtraBlindfire1.Resolve();
}

Debug::Bool g_EnableLoadingScreen("UI.EnableLoadingScreen", true);

bool C_UiLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	MEMORY_TRACKER_PUSH("ui");

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
	case Game::E_GameflowMessage_XtInitialised:
		RefreshXtConfig();
		m_pUiManager->PostXtInit();
		break;

	case E_UiGameMsg_IngameRenderAA:
		{
			const C_UiGameMsg_IngameRenderAA* pIngameRenderAA = Messaging::MessageCast<C_UiGameMsg_IngameRenderAA>( pMessage );

			m_pUiManager->RenderIngameAA( pIngameRenderAA->m_bPostWorldRender );
		}
		break;

	// Need to wait until the amax world layer has destroyed itself.
	case Game::E_GameflowMessage_LevelDumped: // Sent from main thread
		if( m_LoadingFrontend )
		{
			// Send to the game
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_UILoaded );

			// Send to Lua
			_UI_MESSAGE(Game::E_GameflowMessage_UILoaded); 
		}
		break;

	// Going from the the game to the frontend : ( [LevelDumped] -> StartUILoad -> UILoaded -> CreateResources)
	case Game::E_GameflowMessage_StartUILoad:
		BIZ_ASSERT_MSG( !m_IsFrontend, "This should never be called in the frontend" );

		LOGINFO( g_Log, "Loading frontend. Waiting for AmaxWorldLayer to dump" );

		//m_IsFrontend = true;
		m_LoadingFrontend = true;
		m_pUiManager->SetGameWorld( NULL );
		m_pUiManager->DeleteTextureBundle( false );

		// Clear up mini map render targets
		{
			Game::C_LevelState& levelState = GetLevelState();
			for( U32 i = 0; i < levelState.mNumViewports; i++ )
				DestroyMiniMapRenderTarget( i );
		}

		if(g_EnableLoadingScreen)
		{
			C_GameApp::GetInstance()->EnableLoadingScreen();
		}
		break;

	case Game::E_GameflowMessage_UILoaded:
		// Returned from game : Post load
		m_pUiManager->LoadFrontendTextureBundle();
		m_pUiManager->LoadFrontendShapes();

		C_GameApp::GetInstance()->DisableLoadingScreen();
		break;

	case UI::E_UiGameMsg_UiCreateResources:
		// Returning from game - post load
		if ( m_LoadingFrontend )
		{
			LOGINFO( g_Log, "Switching UI to FE PostProcessing" );

			Display::Device()->AcquireThreadOwnership();
			
			DestroyBloom();
			DestroyIgPostProcessBuffer();

			LightPaint_Init(32, 60);

			m_IsFrontend = true;
			m_LoadingFrontend = false;

			CreateBloom();
			CreateRenderBuffer();
			CreateFePostProcessBuffer();

			Display::Device()->ReleaseThreadOwnership();

			SetBloomParameters( C_UiXtConfig::Instance()->m_FrontendBloomParameters );
		}		
		break;

	// Going from the frontend to the game or to next event ( ([DumpLevel] -> [LevelDumped]) -> DestroyResources -> StartRaceLoad -> RaceLoaded
	case UI::E_UiGameMsg_UiDestroyResources:
		//  Going into game : Pre load
		if ( m_IsFrontend || C_GameApp::GetInstance()->GetRaceBookLayer()->GetLoadState() == Game::E_LoadState_IntoRace)
		{
			LOGINFO( g_Log, "Switching UI to Game PostProcessing" );
			
			Display::Device()->AcquireThreadOwnership();

			// Free memory
			m_pUiManager->FreeFrontendShapes();
			m_pUiManager->DeleteTextureBundle( false );

			m_LoadingFrontend = false;

			// Kill UI Bloom
			DestroyBloom();
			DestroyFePostProcessBuffer();

			// Destroy composite
			DestroyRenderBuffer();

			// Create the game bloom
			m_IsFrontend = false;

			CreateBloom();
			CreateIgPostProcessBuffer();

			LightPaint_Destroy();

			Display::Device()->ReleaseThreadOwnership();

			SetBloomParameters( C_UiXtConfig::Instance()->m_IngameBloomParameters );
		}
		// Start the async loading (needs to be here for retry)
		if(g_EnableLoadingScreen)
		{
			C_GameApp::GetInstance()->EnableLoadingScreen();
		}
		break;

	case Game::E_GameflowMessage_RaceLoaded:
		{
			if( !m_IsFrontend && (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer() && !C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->IsQuickRestart()))
			{
				m_pUiManager->LoadGameTextureBundle();

				Game::C_LevelState& levelState = GetLevelState();
				const U32 size = (levelState.mNumViewports <= 2) ? 128 : 64;
				for( U32 i = 0; i < levelState.mNumViewports; i++ )
					CreateMiniMapRenderTarget( i, size );
			}

			m_pUiManager->SetGameWorld( C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld() );
	#ifndef CFG_GOLD
			if (g_debugSuperSpecialCutSceneVideoMode)
				g_debugOKToDumpDumpResources = true;
	#endif
		}
		break;

	case Game::E_GameflowMessage_SyncBootAssets:
		//It shouldn't be possible to get here quicker than we can load the resource bag, but clearly we need to be mindful of that case
		//and block while we wait for it.
		C_GameApp::GetInstance()->WaitForBootResources();
		break;

	case Game::E_GameflowMessage_StreamerFull:
		//This way we ensure we load our resource bag after the streamer has been filled.
		//C_GameApp::GetInstance()->CachePostBootAnimations();
		//C_GameApp::GetInstance()->CachePostBootFxResourceBag();
		break;

	default:
		handledMessage = false;
		break;
	}

	MEMORY_TRACKER_POP("ui");

	return handledMessage;
}

} // namespace UI

