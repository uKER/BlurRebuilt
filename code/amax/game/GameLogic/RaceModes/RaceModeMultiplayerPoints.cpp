#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerPoints.h>
#include <GameLogic\MultiplayerUtilities.h>

#include <GameLogic\Messaging\GameLogicMessages.h>

#include <GameLogic\Messaging\GameLogicMessages.h>

#include <GameLogic\Structures\MpResults.h>

#include <Game\Messaging\GameflowMessages.h>
#include <Game\Messaging\GameMessages.h>
#include <Game\Messaging\PickupActorMessages.h>

#include <Game\GameWorld\GameWorld.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\GameComponents.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>

#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetServices.h>
#include <Network/player/NetPlayerManager.h>
#include <Network\Core.h>

#include <Game/GameActors/Components/Messages/AudioMessages.h>

namespace Game
{

void C_RaceModeMultiplayer_Points::Initialise(Game::C_LevelState *pLevelState)
{
	C_RaceModeMultiplayer::Initialise(pLevelState);

	SetInternalNameAndID("Multiplayer_Racing", E_RaceMode_Multiplayer_Points);

	mVehicleHasLeft = false;
}


void C_RaceModeMultiplayer_Points::StartLevel()
{
	C_RaceModeMultiplayer_Racing::StartLevel();

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	if(levelState.mLaps > 0)
		m_FinishType = UI::E_MpFinishType_Laps;
	else if(levelState.mTimeLimit > 0)
		m_FinishType = UI::E_MpFinishType_Time;
	else
	{
		m_FinishType = UI::E_MpFinishType_PointsTarget;
		m_PointsTarget = levelState.GetMultiplayerSettings().mScoreLimit;
	}
}

void C_RaceModeMultiplayer_Points::MpMode_Update()
{
	C_RaceModeMultiplayer_Racing::MpMode_Update();

	UpdateLocalFans();

	if(m_FinishType == UI::E_MpFinishType_PointsTarget)
	{
		CheckTarget();
	}
}

void C_RaceModeMultiplayer_Points::UpdateLocalFans()
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;
	for(int i = 0; i < localRacingList.Size(); i++)
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if(pLogicActor)
			VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_Fans] = pLogicActor->GetScoring().mFinalScoreForUI;
	}
}

void C_RaceModeMultiplayer_Points::CheckTarget()
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;

	if(AnyVehiclesFinished())
	{
		//	A vehicle has finished, and since this is a Points Target race, all other vehicles now finish too
		for(int i = 0; i < localRacingList.Size(); i++)
		{
			VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
		}
	}
	else
	{
		for(int i = 0; i < localRacingList.Size(); i++)
		{
			if(VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_Fans] >= m_PointsTarget)
			{
				VehicleFinished(localRacingList[i], Game::E_VEHICLE_STATE_FINISHED);
			}
		}
	}
}

void C_RaceModeMultiplayer_Points::MpMode_SortResults()
{
	NetVehicleIndexStack baseStack;
	SortAllNetJoinIds(baseStack);

	//	Every vehicle is sorted together, except for those which have disconnected.
	NetVehicleIndexStack vehiclesScoring;
	NetVehicleIndexStack vehiclesLeftRace;

	for(S32 i = 0; i < baseStack.Size(); i++)
	{
		switch(VehicleData(baseStack[i]).m_State)
		{
		case E_VEHICLE_STATE_LEFT_RACE:
			vehiclesLeftRace.PushBack(baseStack[i]);
			break;
		default:
			vehiclesScoring.PushBack(baseStack[i]);
			break;
		}
	}

	//	Since it's points-based, we don't really care what the vehicles finish types were, so just sort them all together.
	SortByStatistic(vehiclesScoring, UI::E_MultiplayerStat_Fans, true);
	SortByStatistic(vehiclesLeftRace, UI::E_MultiplayerStat_Fans, true);
	
	//	Now we've sorted the vehicles, we put them into the results structure and give them final positions
	S8 position = 0;
	CopyFromStackIntoResults(vehiclesScoring, position);
	CopyFromStackIntoResults(vehiclesLeftRace, position);
}

bool C_RaceModeMultiplayer_Points::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if(!ShouldHandleMessages())
		return false;

	return C_RaceModeMultiplayer::MessageHandler(pMessage);
}

bool C_RaceModeMultiplayer_Points::AddModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex )
{
	bool ok = netMsg.WriteF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_Fans]);
	return ok;
}

bool C_RaceModeMultiplayer_Points::HandleModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex )
{
	bool ok = netMsg.ReadF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_Fans]);
	return ok;
}

} // namespace Game