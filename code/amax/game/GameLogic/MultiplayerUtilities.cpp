#include "stdafx.h"

#include <Network\Core\NetServices.h>
#include <Network\Player\NetPlayer.h>
#include <Network\Player\NetPlayerManager.h>

#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic/Structures/LevelState.h>
#include <GameLogic\Structures\MpResults.h>
#include <Game/Game.h>
#include <GameApp.h>

#include "GameLogic\GameData\MultiplayerData.h"
#include "GameLogic\Structures\NetGameVars.h"

namespace Game
{

	const U32 NET_UPDATE_TIMESTEP_INCREMENT_VALUE = 10;

bool VehicleIsLocal( U8 p_VehicleIndex )
{
	if(p_VehicleIndex != SYSTEM_FIRING_INDEX)
	{
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		if(levelState.mCars[p_VehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER ||
			levelState.mCars[p_VehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_AI)
			return true;
	}
	else if(HostIsLocal())
		return true;
	
	return false;
}

bool HostIsLocal()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
	if (pHost && pHost->LocalPlayer())
	{
		return true;
	}
	return false;
}

bool VehiclesAreTeammates(S8 vehicleOne, S8 vehicleTwo)
{
	if(vehicleOne != -1 && vehicleTwo != -1 && (vehicleOne != vehicleTwo))
	{
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		return(levelState.mCars[vehicleOne].mTeam == levelState.mCars[vehicleTwo].mTeam);
	}
	return false;
}

S32 GetDriverRacePoints( U8 position, U8 numVehicles )
{
	//convert to array index
	int points_index = position - 1;
	//the fewer cars we have the further down the points we go
	int start = MAX_LEVELSTATE_CARS - numVehicles;

	S32 numRacePoints = 0;

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;
	if((start + points_index) < 0)
		BIZ_ASSERT_FAIL("Unable to award multiplayer race position points - Index out of bounds (low)");
	else if((start + points_index) >= multiplayerConfig->m_RacePositionPoints.count)
		BIZ_ASSERT_FAIL("Unable to award multiplayer race position points - Index out of bounds (high)");
	else
		numRacePoints = multiplayerConfig->m_RacePositionPoints[start + points_index];

	return numRacePoints;
}

S8 GetLocalIndexFromJoinId( U8 joinId )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if (levelState.mCars[i].mNetJoinId == joinId)
			return (S8) i;
	}
	return -1;
}

U8 GetJoinIdFromLocalIndex( S8 vehicleIndex )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	return (vehicleIndex != Game::SYSTEM_FIRING_INDEX && vehicleIndex != -1) ? levelState.mCars[vehicleIndex].mNetJoinId : 0;
}

U32 GetLocalVehicleList(NetVehicleIndexStack& localList)
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		Game::LevelStateCar& carState = levelState.mCars[i];

		if (levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER || levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_AI)
			localList.PushBack(i);
	}

	return localList.Size();
}

U32 GetNetworkVehicleList( NetVehicleIndexStack& networkList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		Game::LevelStateCar& carState = levelState.mCars[i];

		if (levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
			networkList.PushBack(i);
	}

	return networkList.Size();
}

//	Find the indices of local Player vehicles
U8 GetLocalPlayerList( NetVehicleIndexStack& localPlayerList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		Game::LevelStateCar& carState = levelState.mCars[i];

		if (levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			localPlayerList.PushBack(i);
	}

	return localPlayerList.Size();
}


//	Just find the number of vehicles still Racing.
U32 GetNumGlobalRacing()
{
	U32 numRacing = 0;
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].IsRacing())
			numRacing++;
	}

	return numRacing;
}

//	Find the indices of all the vehicles still Racing.
U32 GetGlobalRacingList( NetVehicleIndexStack& globalRacingList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].IsRacing())
			globalRacingList.PushBack(i);
	}
	return globalRacingList.Size();
}	

//	Find the indices of all the LOCAL vehicles still Racing
U32 GetLocalRacingList( NetVehicleIndexStack& localRacingList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		Game::LevelStateCar& carState = levelState.mCars[i];

		if (carState.mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER || carState.mControlType == E_VEHICLE_CONTROL_TYPE_AI)
		{
			if(C_MpResults::Instance().m_LiveVehicleData[i].IsRacing())
				localRacingList.PushBack(i);
		}
	}

	return localRacingList.Size();
}

//	Find the indices of all the vehicles still in-race (finished, timed out or racing)
U32 GetGlobalInRaceList( NetVehicleIndexStack& globalList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(!C_MpResults::Instance().m_LiveVehicleData[i].IsOutOfRace())
			globalList.PushBack(i);
	}
	return globalList.Size();
}

U32 GetNumConnected()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	U32 numConnected = 0;
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].m_State != E_VEHICLE_STATE_LEFT_RACE)
			numConnected++;
	}

	return numConnected;
}

//	Just get the number of vehicles that have disconnected from the race.
U32	GetNumDisconnected()
{
	U32 numDisconnected = 0;
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].m_State == E_VEHICLE_STATE_LEFT_RACE)
			numDisconnected++;
	}

	return numDisconnected;
}

//	Find the indices of all the vehicles that have disconnected from the race.
U32 GetDisconnectedList( NetVehicleIndexStack& disconnectedList )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].m_State == E_VEHICLE_STATE_LEFT_RACE)
			disconnectedList.PushBack(i);
	}

	return disconnectedList.Size();
}

//	Return true if any vehicles in the race have FINISHED
bool AnyVehiclesFinished()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if(C_MpResults::Instance().m_LiveVehicleData[i].m_State == E_VEHICLE_STATE_FINISHED)
			return true;
	}
	return false;
}

void IncrementUpdateTimestep()
{
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if(netGameVars.m_InstantUpdate)
	{
		netGameVars.m_InstantUpdate = false;
		netGameVars.m_NetUpdateTimestep = RacerNet::NET_VEHICLE_UPDATE_TIME;
	}
	else
	{
		netGameVars.m_NetUpdateTimestep += NET_UPDATE_TIMESTEP_INCREMENT_VALUE;

		if(netGameVars.m_NetUpdateTimestep > RacerNet::NET_VEHICLE_UPDATE_TIME)
			netGameVars.m_NetUpdateTimestep = RacerNet::NET_VEHICLE_UPDATE_TIME;
	}
}

void ReduceUpdateTimestep( bool sendASAP /*= false*/ )
{
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if(sendASAP)
	{
		netGameVars.m_NetUpdateTimestep = 0;
		netGameVars.m_InstantUpdate = true;
	}
	else
	{
		netGameVars.m_NetUpdateTimestep = RacerNet::NET_VEHICLE_UPDATE_TIME / 2;
	}
}

} // namespace Game
