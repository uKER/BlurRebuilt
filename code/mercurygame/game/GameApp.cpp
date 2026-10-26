#include "StdAfx.h"
#include "GameApp.h"

#include <Framework/Application/CaptureService.h>
#include <Framework/shared_layers/DebugLayer.h>
#include <Game/MGCam/CameraSystem.h>
#include <Game/Game.h>
#include <Game/SimpleUi/SimpleUiLayer.h>
#include <Game/GameWorld/PostProcessLayer.h>
#include <Game/SystemLayer.h>
#include <Game/GameWorld/GameWorldLayer.h>

#include <Game/GameControls/GameControls.h>

#include <fx/I_Effect.h>
#include <fx/fx.h>
#include <fx/RenderMutex.h>
#define FX_ALLOW_INCLUDE // required because we are dealing directly with FX::Technique
#include <fx/Effect.h>
#include <fx/Technique.h>

#include <shader/envsettings/LightingSettings.h>
#include "AppPhotoCapture.h"

#include <Display/Context/Context.h>

using namespace Game;

const int MAX_GLOBAL_XT_DATABASE_RECORDS=2500;

C_GameApp::C_GameApp() 
	: m_pGame(NULL)
	, m_pSimpleUiLayer(NULL)
	, m_pDebugLayer(NULL)
	, m_pPostProcessLayer(NULL)
	, m_pSystemLayer(NULL)
	, m_pGameWorldLayer(NULL)
{
#ifdef PLATFORM_PC
	m_Class.m_WndClassName = "Mercury Game Class";
	m_Window.m_Name = "Mercury Game";
#endif
	m_pGame = new C_Game();
	m_PhotoCapture = 0;
}

C_GameApp::~C_GameApp()
{
	delete m_pXtCC;

	CloseXtDatabase();

	XtShutdown();

	SAFE_DELETE_PTR(m_pGame);
}

Game::C_Game* C_GameApp::GetGame()
{
	return m_pGame;
}

C_GameApp* C_GameApp::GetInstance()
{
	return (C_GameApp*)Framework::C_Application::GetInstance();
}

HRESULT C_GameApp::OnInitialise()
{
	MGCam::System().Init();

	::EnvironmentSettings::g_vIntensities = Maths::HVec4(1.0f, 10.0f, 1.0f, 1.0f);

	//create the global Xt database for the whole game
	CreateXtDatabase();

	Display::RingBuffer()->Initialise(5*1024*1024);

	FX::Initialise();

	m_pSimpleUiLayer = new Game::C_SimpleUiLayer( &C_Game::GetInstance()->GetSubscriberSystem() );
	InsertLayer(m_pSimpleUiLayer);

	m_pDebugLayer = new Framework::C_DebugLayer();
	InsertLayer( m_pDebugLayer );

	m_pSystemLayer = new Game::C_SystemLayer();
	InsertLayer( m_pSystemLayer );

	GetGame()->Initialise();

	//Register model type
	GameResource::g_ModelTypeManager.RegisterModelType("model", new GameResource::C_ModelType());
	GameResource::RegisterDefaultModelType();

	m_PhotoCapture = new PhotoMode::C_AppPhotoCapture;

	return S_OK;
}

void C_GameApp::OnShutdown()
{
	SAFE_DELETE_PTR(m_PhotoCapture)

	FX::Shutdown();

	Display::RingBuffer()->Shutdown();

	if(m_pSystemLayer)
	{
		DestroyLayer( m_pSystemLayer );
		m_pSystemLayer = NULL;
	}

	if(m_pSimpleUiLayer)
	{
		DestroyLayer( m_pSimpleUiLayer );
		m_pSimpleUiLayer = NULL;
	}
	
	if(m_pDebugLayer)
	{
		DestroyLayer( m_pDebugLayer );
		m_pDebugLayer = NULL;
	}
}

struct GameXtCallbackContext : public XtCallbackContext
{
	inline GameXtCallbackContext() : XtCallbackContext( 'GAME' )	{  }
};

void C_GameApp::CreateXtDatabase()
{
	XtInitialise();

	XtSchema* pXtSchema = XtGetApplicationSchema();
	m_pXtCC = new GameXtCallbackContext;

	m_pXtDatabase = XtDbCreate( pXtSchema, MAX_GLOBAL_XT_DATABASE_RECORDS, m_pXtCC );

	XtNetworkStart( m_pXtDatabase );
}

void C_GameApp::CloseXtDatabase()
{
	XtDbDestroy(m_pXtDatabase);
}

XtDb* C_GameApp::GetXtDatabase()
{
	return m_pXtDatabase;
}

void C_GameApp::SetupUserControls()
{
	SetInputButtonCount( Game::E_Button_Count );
	SetInputAxisCount( Game::E_Axis_Count );

	assert(m_pGameInputManager);
	m_pGameInputManager->Resize(m_InputUserCount, m_InputButtonCount, m_InputAxisCount);
	
	for(S32 i=0; i<m_InputUserCount; i++)
	{
		Framework::SetupApplicationUserControls(Input::GetGameInput(i), m_pKeyboardList[i], m_pMouseList[i], m_pPadList[i]);
		Game::SetupUserControls(Input::GetGameInput(i), m_pKeyboardList[i], m_pMouseList[i], m_pPadList[i]);
	}
}

void C_GameApp::OnUpdate()
{
	CommonUpdate();
}

void C_GameApp::OnRender()
{
	FX::LockRenderMutex();

	Framework::C_CaptureService& captureService = Framework::g_CaptureService;

	if (captureService.TakePhoto())
		m_PhotoCapture->TakePhoto(0, false, false);

	if (captureService.PhotoMovie())
		m_PhotoCapture->TakePhoto(0, false, true);

	bool captured = false;
	if (m_PhotoCapture)
		captured = m_PhotoCapture->OnAppRender();

	if (!captured)
		RenderNow();

	// Update screenshot capture service:
	captureService.Update();

	FX::ReleaseRenderMutex();
}


void C_GameApp::RenderNow()
{
	if(BeginRender())
	{
		CommonRender();

		EndRender();

		Display::RingBuffer()->EndFrame();
	}
}

void C_GameApp::CreatePostProcessLayer()
{
//	#ifdef PLATFORM_XENON
	assert( m_pPostProcessLayer == NULL );
	m_pPostProcessLayer = new Game::C_PostProcessLayer();
	InsertLayer( m_pPostProcessLayer );
//	#endif
}

void C_GameApp::DestroyPostProcessLayer()
{
//	#ifdef PLATFORM_XENON
	DestroyLayer( m_pPostProcessLayer );
	m_pPostProcessLayer = NULL;
//	#endif
}

void C_GameApp::CreateGameWorldLayer()
{
	assert( m_pGameWorldLayer == NULL );
	m_pGameWorldLayer = new C_GameWorldLayer( &C_Game::GetInstance()->GetSubscriberSystem() );
	InsertLayer( m_pGameWorldLayer );
}

void C_GameApp::DestroyGameWorldLayer()
{
	DestroyLayer( m_pGameWorldLayer );
	m_pGameWorldLayer = NULL;
}