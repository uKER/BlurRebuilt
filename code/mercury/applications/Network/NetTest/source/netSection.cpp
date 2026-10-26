#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#include "stdafx.h"
#include "netSection.h"
#include "netApp/netApp.h"
#include "netServices\netServices.h"
#include "netCore\netPlatform.h"
#include "netCore\netStats\netLog.h"
#include "netCore\netSocket\netSocket.h"
#include "netEntities\netWorld.h"
#include "netGroup\netGroup.h"

#include "gadgets\UiNetPanel_Main.h"
#include "ui\Game\Gadgets\ButtonMessages.h"
#include "ui\UiManager.h"


#include "gameTestApp/game/netGameWorldTestApp.h"
#include "gameTestApp/game/netTestAppFactory.h"

#include "netServices/netProfiles/netMessageEnums_Notifications.h"
#include "netApp/netMessageEnums_Multiplayer.h"
#include "gadgets/netMessageEnums_UI.h"

#ifdef PLATFORM_XENON
#include <xaudio.h>
#endif

netSimulation* sim = NULL;

namespace Samples
{
	void SectionNetConnect_Start()
	{
		// ok
		// rand seed
		srand((U32)(netClock::instance()->now() & 0xFFFFFFFF));

		assert(sim == NULL);
		sim = new netSimulation();
	}

	void SectionNetConnect_End()
	{
		assert(sim != NULL);
		delete sim;
		sim = NULL;
	}

	bool SectionNetConnect_Step(float dt, const C_Input& input)
	{
		return sim->step(dt, input);
	}

	void SectionNetConnect_Render(float fps)
	{
		sim->render(fps);
	}
};

netSimulation::netSimulation()
{
	#ifdef PLATFORM_XENON
	XAUDIOENGINEINIT audioEngineInit = {0};
	XAudioInitialize(&audioEngineInit);
	#endif

	netGameEnums::dbg_testapp = true;

	m_pUiManager = new UI::C_UiManager( 0 );
	assert( m_pUiManager );
	m_pUiManager->CreateTextureBundle( false );

	// network game components
	I_Factory*					gameWorldFactory	= (I_Factory::GetInstance())? I_Factory::GetInstance() : new C_TestAppFactory();
	netGame::worldInterfaceRef	gameWorldInterface	= new netGame::worldTestApp();

	// multiplayer game object
	m_app = new netApp(gameWorldFactory, gameWorldInterface);
	
	m_padStatePtr = 0;

	assert(netServices::instance());
	SetSubscriberSystem(netServices::instance());
	AddSubscriber(Multiplayer::E_Msg_MultiplayerLeft);
	AddSubscriber(Notifications::E_Msg_GameInvitePending);
	AddSubscriber(Notifications::E_Msg_SignInChanged);
	AddSubscriber(Notifications::E_Msg_DuplicateSignIn);
	AddSubscriber(Notifications::E_Msg_ProfilesChanged);
	AddSubscriber(Notifications::E_Msg_ProfileSettingsChanged);
	AddSubscriber(Notifications::E_Msg_StorageDeviceChanged);
	AddSubscriber(Notifications::E_Msg_NewContentInstalled);
	AddSubscriber(Notifications::E_Msg_InputDeviceChanged);
	AddSubscriber(Notifications::E_Msg_MuteInfoChanged);
	AddSubscriber(Notifications::E_Msg_ServiceConnectionLost);
	AddSubscriber(Notifications::E_Msg_ServiceConnectionEstablished);
	AddSubscriber(Notifications::E_Msg_FriendRequestPending);
	AddSubscriber(Notifications::E_Msg_FriendSignedIn);
	AddSubscriber(Notifications::E_Msg_DeviceConfigChanged);
}

netSimulation::~netSimulation()
{
	RemoveAllSubscribers();
	delete m_app;
	m_pUiManager->DeleteTextureBundle( true );
	SAFE_DELETE_PTR( m_pUiManager );
}

bool netSimulation::step(float dt, const Samples::C_Input& input)
{
	processInputs(input);
	
	m_pUiManager->Process( Maths::C_TimeS( dt ), input );
	
	netServices::instance()->update(dt);
	
	// update platform
	netPlatform::instance()->update();

	// update profile manager
	netProfileManager::instance()->update();

	netDebug::dbg_timeoutEnabled = (buttonDown(Samples::E_Button_Back) && buttonPressed(Samples::E_Button_X)) || netDebug::dbg_botMode;

	if(buttonDown(Samples::E_Button_Back) && buttonPressed(Samples::E_Button_B))
	{
		switch(netDebug::dbg_renderMode)
		{
		case E_DbgRnd_NetApp:		
			{
				netDebug::dbg_renderParty = !netDebug::dbg_renderParty;
				break;
			}
		case E_DbgRnd_GameState:	
			{
				int val = ((int)netDebug::dbg_renderGameStateMode + 1) % E_DbgMode_Count;
				netDebug::dbg_renderGameStateMode = (E_DbgMode)val;		
				break;
			}
		}
	}

	if(buttonDown(Samples::E_Button_Back) && buttonPressed(Samples::E_Button_X))
	{
		if(netDebug::dbg_renderMode == E_DbgRnd_GameState)
		{
			netDebug::dbg_RenderGameState = (E_State_Type)(netDebug::dbg_RenderGameState+1);
			if(netDebug::dbg_RenderGameState >= E_State_Count)
				netDebug::dbg_RenderGameState = (E_State_Type)0;
		}
	}
	
	if(buttonDown(Samples::E_Button_Back) && buttonPressed(Samples::E_Button_A))
	{
		unsigned int rendermode = netDebug::dbg_renderMode;
		rendermode = (rendermode - 1) % E_DbgRnd_Count;
		netDebug::dbg_renderMode = (E_DbgRnd)rendermode;
	}
	if(buttonDown(Samples::E_Button_Back) && buttonPressed(Samples::E_Button_Y))
	{
		unsigned int rendermode = netDebug::dbg_renderMode;
		rendermode = (rendermode + 1) % E_DbgRnd_Count;
		netDebug::dbg_renderMode = (E_DbgRnd)rendermode;
	}
	
	// update game
	if(m_app) 
	{
		m_app->update(dt);
	}
	return true;
}

void netSimulation::render(float fps)
{
	m_pUiManager->Render();

	float x = 40.0f;
	float y = 20.0f;
	float h = 12.0f;
	float w = 12.0f;
	if(m_app) m_app->render(x, y, w, h, fps);
}

bool netSimulation::MessageHandler(const Messaging::C_MessageBase* msg)
{
	switch(msg->Type())
	{
	default:
		{
			break;
		}
	case Multiplayer::E_Msg_MultiplayerLeft:
		{
			//QuitSection();
			break;
		}
	}		
	return true;
}

bool netSimulation::buttonDown(int i) const
{
	if(i >= 128 || i < 0) return false;
	int cur_index = (m_padStatePtr > 0)?  ((m_padStatePtr-1) % 32) : 0;
	float cur_state = m_padState[cur_index][i];
	return (cur_state > 0.0f);
}

bool netSimulation::buttonPressed(int i) const
{
	if(i >= 128 || i < 0) return false;
	int old_index = (m_padStatePtr > 1)? ((m_padStatePtr-2) % 32)  : 0;
	int cur_index = (m_padStatePtr > 0)?  ((m_padStatePtr-1) % 32) : 0;
	
	float old_state = m_padState[old_index][i];
	float cur_state = m_padState[cur_index][i];

	return (cur_state > 0.0f && old_state == 0.0f);
}

bool netSimulation::processInputs(const Samples::C_Input& input)
{
	static int s_message[Samples::E_Button_Count] = 
	{
		UI::E_UiMsg_Start,						//E_Button_LtMouse = 0,
		UI::E_UiMsg_Start,						//E_Button_RtMouse,
		UI::E_UiMsg_CtrlButton_A,				//E_Button_A,
		UI::E_UiMsg_CtrlButton_B,				//E_Button_B,
		UI::E_UiMsg_CtrlButton_X,				//E_Button_X,
		UI::E_UiMsg_CtrlButton_Y,				//E_Button_Y,
		UI::E_UiMsg_CtrlButton_Start,			//E_Button_Start,
		UI::E_UiMsg_CtrlButton_Back,			//E_Button_Back,
		UI::E_UiMsg_CtrlButton_LeftShoulder,	//E_Button_LtShoulder,
		UI::E_UiMsg_CtrlButton_RightShoulder,	//E_Button_RtShoulder,
		UI::E_UiMsg_CtrlButton_LeftTrigger,		//E_Button_LtTrigger,
		UI::E_UiMsg_CtrlButton_RightTrigger,	//E_Button_RtTrigger,
		UI::E_UiMsg_Start,						//E_Button_LtThumb,
		UI::E_UiMsg_Start,						//E_Button_RtThumb,
		UI::E_UiMsg_CtrlButton_Left,			//E_Button_LtPad,
		UI::E_UiMsg_CtrlButton_Right,			//E_Button_RtPad,
		UI::E_UiMsg_CtrlButton_Up,				//E_Button_UpPad,
		UI::E_UiMsg_CtrlButton_Down,			//E_Button_DnPad,
	};

	int prev_index = (m_padStatePtr > 0)? ((m_padStatePtr-1) % 32) : 0;
	int new_index = (m_padStatePtr % 32);
	
	for(int j = 0; j < Samples::E_Button_Count; j ++)
	{
		float state = input.GetButtonState((Samples::E_Button)j);
		m_padState[new_index][j] = state;
	}
	m_padStatePtr++;

	for(int j = 0; j < Samples::E_Button_Count; j ++)
	{
		float prevval = m_padState[prev_index][j];
		float newval = m_padState[new_index][j];

		if(newval > 0.0f && prevval == 0.0f && !buttonDown(Samples::E_Button_Back))
		{
			Messaging::C_MessageBase message(s_message[j], sizeof(Messaging::C_MessageBase));
			UI::gButtons.GetSubscriberSystem().BroadcastMessage(&message);
		}
	}
	return true;
}


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif


