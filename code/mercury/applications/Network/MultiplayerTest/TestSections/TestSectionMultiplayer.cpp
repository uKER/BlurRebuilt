
#include "NightwingPch.h"
#include "TestMultiplayerSection.h"
#include "GameWindow.h"

#ifndef CFG_GOLD 

#include <Game.h>
#include <GameControl.h>

#include "netApp/netApp.h"
#include "netGame/netTestGameWorld.h"
#include "netApp\netMessageEnums_Multiplayer.h"
#include "netServices\netServices.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


C_TestMultiplayerLayer::C_TestMultiplayerLayer( GameFlow::C_Section* pSection ) 
: C_GameLayerBase( pSection )
{
	netDebug::dbg_testGame = true;
	Multiplayer::C_Msg_StartMultiplayer start;
	netServices::instance()->BroadcastMessage(&start);
	UI::gUIScreenControl.SetNextScreen("Multiplayer\\MainDebug.lua");
}

C_TestMultiplayerLayer::~C_TestMultiplayerLayer()
{
}

C_TestMultiplayerSection::C_TestMultiplayerSection( GameFlow::C_Queue* pQueue, const U32 initParam ) 
:  C_UiSection( pQueue, initParam )
{
	LOGDEBUG( Flow::g_Log, __DEBUG_FUNC__ );

    C_TestMultiplayerLayer* pLayer = new C_TestMultiplayerLayer( this );
	SetupSimpleWindows( pLayer );    
}

C_TestMultiplayerSection::~C_TestMultiplayerSection()
{
}
void C_TestMultiplayerSection::CustomRender()
{
	C_GameSectionBase::CustomRender();

	float fps = 1.0f;
	float x = 40.0f;
	float y = 20.0f;
	float h = 12.0f;
	float w = 12.0f;
}

void C_TestMultiplayerLayer::Render( const U32 renderParam )
{
    GetGameApp()->m_pDevice->Clear( Maths::RGBA(0x00,0x00,0x00,0x00) );
	
	// network debug
	#ifndef CFG_GOLD
	{
		if(netApp::instance())
		{
			netApp::instance()->render(0, 0, 14, 14, 0);

			if(netDebug::dbg_testGame && !netGame::C_GameWorld::GetInstance())
			{
				DbUtil::I_DebugRenderPtr debugRenderPtr = DbUtil::DefaultDebugRenderSys()->Immediate();
				debugRenderPtr->Draw3dGrid( Maths::HMat43() );
			}

			if(netGame::C_GameWorld::GetInstance() != NULL)
			{
				netGame::C_GameWorld::GetInstance()->Draw(Maths::HVec3(0, 0, 0));
			}		
		}
	}
	#endif
}



#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

#endif

