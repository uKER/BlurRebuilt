#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <GameLogic/EnvironmentSettingsManager.h>

#include <Game\GameWorld\GameWorld.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <Game\Messaging\GameflowMessages.h>

namespace Game
{

C_EnvironmentSettingsManager C_EnvironmentSettingsManager::s_instance;

void C_EnvironmentSettingsManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	
	AddSubscriber( E_GameflowMessage_LoadResources);
	AddSubscriber( E_GameflowMessage_IntroFinished);
	AddSubscriber( E_GameflowMessage_OutroStarted);
	AddSubscriber( E_GameflowMessage_OutroFinished);
}

void C_EnvironmentSettingsManager::Update()
{
	// this code re-creates the environment settings from Sorcery when someone dynamically edits the settings
	if (g_bEnvironmentSettingsChangedInSorcery)
	{
		if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() )
		{
			if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
			{
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery();
			}
		}
		
		g_bEnvironmentSettingsChangedInSorcery = false;
	}

}

bool C_EnvironmentSettingsManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		// set the initial environment before the load finishes
		case E_GameflowMessage_LoadResources:
			{
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery();
			}
			break;

		case E_GameflowMessage_IntroStarted:
		case E_GameflowMessage_OutroStarted:
			{
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery();
			}
			break;

		case E_GameflowMessage_OutroFinished:
		case E_GameflowMessage_IntroFinished:
			{
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery();
			}
			break;

		default :
			break;
	}

	return handledMessage;
}

}; // namespace Game
