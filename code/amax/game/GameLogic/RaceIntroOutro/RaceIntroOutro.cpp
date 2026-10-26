#include "stdafx.h"

#include "RaceIntroOutro.h"

#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameApp.h>

#include <GameLogic\profile\GameProfileManager.h>

#include "ui/Graphics/SimpleTimer.h"

#include <Game/Messaging/GameflowMessages.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <ui\Game\Gadgets\ButtonMessages.h>

#include "Game/AmaxCam/CameraSystem.h"

#include <GameLogic/GameData/GameEventData.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <GameLogic\ActorStudioActors\CutSceneMessages.h>

static Debug::Logger &gGameRaceIntroOutro = Debug::GetLogger("oldtech.gamelogic.RaceIntroOutro");

Debug::Float g_StreetRaceIntroSpeed("Game.RaceLogicCore.RaceIntro.VehicleSpeed", 30.0f, Debug::Range(1.f, 60.f));
Debug::Bool g_StreetRaceOverideAISpeed("Game.RaceLogicCore.RaceIntro.UseForAllRace", false);

namespace Game
{

RaceIntroOutro RaceIntroOutro::s_instance;

void RaceIntroOutro::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
	AddSubscriber( Game::E_GameflowMessage_IntroStarted );
	AddSubscriber( Game::E_GameflowMessage_OutroStarted );
}

void RaceIntroOutro::StartIntro()
{
//	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() == Game::E_GameMode_SinglePlayer)
//		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(ActorMsg::E_CutSceneMsg_StartTheIntro);
}

void RaceIntroOutro::StartOutro()
{
	// Drop out for now
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_OutroFinished );
	_UI_MESSAGE(E_GameflowMessage_OutroFinished);

	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() == Game::E_GameMode_SinglePlayer)
	{
	}
}

void RaceIntroOutro::DoIntroSection(const AmaxIntroSection *section)
{
}

void RaceIntroOutro::OverideControls()
{
}

void RaceIntroOutro::Reset()
{

}

void RaceIntroOutro::PostUpdate()
{
	if(mStartIntro)
	{
		mStartIntro=false;
		mPlayingIntro=true;
	}

	if(mPlayingIntro)
	{
		//FinishIntro();
	}

	if(mStartOutro)
	{
		mStartOutro=false;
		mPlayingOutro=true;
	}

	if(mPlayingOutro)
	{
		//FinishOutro();
	}
}

void RaceIntroOutro::FinishOutro()
{
	mPlayingOutro=false;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	pRLC->SetViewportFocusVehicleIndex(0,0);

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_OutroFinished );
	_UI_MESSAGE(E_GameflowMessage_OutroFinished);
}

void RaceIntroOutro::FinishIntro()
{
	mPlayingIntro=false;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
//	pRLC->SetViewportFocusVehicleIndex(0,0);

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_IntroFinished );
	_UI_MESSAGE(E_GameflowMessage_IntroFinished);

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for(unsigned int i = 0; i < levelState.mNumViewports; i++)
	{
		AmaxCam::CameraSystem().GameFeedShow(i, AmaxCam::E_GameCam_VehicleFeed);
	}
}

bool RaceIntroOutro::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_GameflowMessage_RaceLoaded:
			break;

		case Game::E_GameflowMessage_IntroStarted:
			StartIntro();
			break;

		case Game::E_GameflowMessage_OutroStarted:
			StartOutro();
			break;

		default :
			break;
	}

	return handledMessage;
}

} // namespace Game
