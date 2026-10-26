#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic\RaceModes\RaceModeMultiplayerElimination.h>
#include <GameLogic\MultiplayerUtilities.h>

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

#include <ui\UiInterface.h>
#include <ui\SoundEffects.h>

Debug::Float g_debugEliminateMultiplier("Multiplayer.EliminationMultiplier", 2.0f);
Debug::Float g_debugEliminateMultiplier2("Multiplayer.EliminationMultiplier2", 1.0f);
Debug::Int g_debugEliminateMultiplierNumber("Multiplayer.EliminationMultiplierNumber", 1);
Debug::Int g_debugEliminateMultiplierNumber2("Multiplayer.EliminationMultiplierNumber2", 1);

Debug::Float g_debugBarrageLength("Multiplayer.BarrageLength", 5.0f);
Debug::Float g_debugTimeBetweenBarrageShots("Multiplayer.TimeBetweenBarrageShots", .4f);
Debug::Float g_debugBarrageZoneRadius("Multiplayer.BarrageZoneRadiusMetres", 18.0f);
Debug::Float g_debugTestShockWidth("Multiplayer.TestShockWidth", .4f);
Debug::Float g_debugMetresForwardPerMph("Multiplayer.MetresForwardsPerMph", .6f);
Debug::Int g_debugMinimumRandomiseDomeSpeedThreshhold("Multiplayer.MinimumRandomiseDomeSpeedThreshhold", 10);

Debug::Bool g_debugRandomiseShockPosition("Multiplayer.RandomiseShockPosition", true);

namespace Game
{

void C_RaceModeMultiplayer_Elimination::Initialise(Game::C_LevelState *pLevelState)
{
	C_RaceModeMultiplayer::Initialise(pLevelState);

	SetInternalNameAndID("Multiplayer_Elimination", E_RaceMode_Multiplayer_Elimination);
}

void C_RaceModeMultiplayer_Elimination::StartLevel()
{
	C_RaceModeMultiplayer::StartLevel();

	m_EliminationType	= C_GameApp::GetInstance()->GetGame()->GetLevelState().GetMultiplayerSettings().mEliminationType;
	m_EliminationEvent	= C_GameApp::GetInstance()->GetGame()->GetLevelState().GetMultiplayerSettings().mEliminationEvent;
	m_EliminationPeriod = (F32)C_GameApp::GetInstance()->GetGame()->GetLevelState().mTimeLimit;
	m_EliminationTimer	= m_EliminationPeriod * g_debugEliminateMultiplier;
	m_RemainingBarrageTimer = .0f;
	m_TimeToNextShotInBarrage = .0f;
	m_EliminationLapCounter = 0;
	m_BarrageInProgress = false;
	m_EliminationNumber = 1;
	m_CurrentBarrageTargetIndex = 0;
	m_Random.Seed((U32)Time::GetCurrentTime());

	m_EliminationPositions.Clear();
}

//	Race Mode-specific updating...
void C_RaceModeMultiplayer_Elimination::MpMode_Update()
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	UpdateEliminationPoints();

	if(mFinalDelayTimerActive)
	{
		DecrementFinalDelayTimer();
		return;
	}

	if(m_EliminationEvent == UI::E_EliminationEvent_Time)
	{
		if(!m_BarrageInProgress)
		{
			const F32 ot = floorf( m_EliminationTimer );
			m_EliminationTimer -= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
			if(m_EliminationTimer < 0.0f)
			{
				InitiateElimination();
			}

			/*
			const F32 nt = floorf( m_EliminationTimer );
			if( nt != ot )
			{
				if ( nt <= 3.0f )
					UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDown03ActionIn );
				else
					UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_CountDownActionIn );
			}
			*/
		}
		else
		{
			m_RemainingBarrageTimer -= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
			m_TimeToNextShotInBarrage -= (F32)C_GameApp::GetInstance()->GetTimeStepRaw();
			if(m_RemainingBarrageTimer <= .0f)
			{
				if(m_EliminationNumber++ < g_debugEliminateMultiplierNumber)
					m_EliminationTimer = m_EliminationPeriod * g_debugEliminateMultiplier;
				else if(m_EliminationNumber < g_debugEliminateMultiplierNumber + g_debugEliminateMultiplierNumber2)
					m_EliminationTimer = m_EliminationPeriod * g_debugEliminateMultiplier2;
				else
					m_EliminationTimer = m_EliminationPeriod;

				m_BarrageInProgress = false;

				RewardSurvivingVehicles();
			}
			else if(m_TimeToNextShotInBarrage <= .0f)
			{
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
				if (pHost && pHost->LocalPlayer())
				{
					DropShockDome();
				}
				m_TimeToNextShotInBarrage = g_debugTimeBetweenBarrageShots;
			}
		}
	}
}

bool C_RaceModeMultiplayer_Elimination::AddModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex )
{
	bool ok = netMsg.WriteF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_EliminationPoints]);
	return ok;
}

bool C_RaceModeMultiplayer_Elimination::HandleModeSpecificNetMessageBasicVehicleData( RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex )
{
	bool ok = netMsg.ReadF32(VehicleData(p_VehicleIndex).m_Statistics[UI::E_MultiplayerStat_EliminationPoints]);
	return ok;
}

void C_RaceModeMultiplayer_Elimination::UpdateEliminationPoints()
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0)
		return;
	for(int i = 0; i < localRacingList.Size(); i++)
	{
		VehicleData(localRacingList[i]).m_Statistics[UI::E_MultiplayerStat_EliminationPoints] = GetEliminationPoints(localRacingList[i]);
	}
}

void C_RaceModeMultiplayer_Elimination::UpdateEliminationOrder( NetVehicleIndexStack& racers )
{
	if(racers.Size() > 1)
	{
		//	This gets a bit confusing here, but basically we sort the stack depending on whether this mode is eliminating on a high or low value.
		//	Essentially - the person who's in "last" place will end up at the end (back) of the list :)
		if(m_EliminationType == UI::E_EliminationType_FirstPlace
			|| m_EliminationType == UI::E_EliminationType_MostHealth
			|| m_EliminationType == UI::E_EliminationType_MostFans)
		{
			//	These elimination types are looking for the highest numerical value to eliminate on (at the end)
			SortByStatistic(racers, UI::E_MultiplayerStat_EliminationPoints, false);
		}
		else
		{
			//	These elimination types are looking for the lowest numerical value to eliminate on (at the end)
			SortByStatistic(racers, UI::E_MultiplayerStat_EliminationPoints, true);
		}

		//	[GavinY] - Do we really care about this level of fairness? No, no we don't...so let's just leave this unless it becomes an issue.

		////	We need to check for any equal values at the bottom of the list (i.e. any vehicles tied for Elimination)
		//NetVehicleIndexStack eliminationCandidates;

		//int i = m_EliminationPositions.Size() - 1;
		//while(VehicleData(m_EliminationPositions[i]).m_Statistics[UI::E_MultiplayerStat_EliminationPoints] == VehicleData(m_EliminationPositions[m_EliminationPositions.Size() - 1]).m_Statistics[UI::E_MultiplayerStat_EliminationPoints])
		//{
		//	eliminationCandidates.PushBack(m_EliminationPositions[i]);
		//	i--;
		//	if(i < 0)
		//		break;
		//}

		////	Now we've pushed the elimination candidates into a separate list we need to sort them based on distance travelled (if there's more than one candidate of course!).
		//if(eliminationCandidates.Size() > 1)
		//{
		//	Btl::Vector<F32, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> > logicDistances;
		//	for(S32 i = 0; i < eliminationCandidates.Size(); i++)
		//	{
		//		logicDistances[eliminationCandidates[i]] = (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(eliminationCandidates[i])->GetPositionInfo().mLogicPositionCombined);
		//	}

		//	for (S32 i=0; i < eliminationCandidates.Size()-1; i++)
		//	{
		//		S32 swap = -1;
		//		F32 greatestDistance = logicDistances[eliminationCandidates[i]];
		//		for (S32 j=i+1; j<eliminationCandidates.Size(); j++)
		//		{
		//			if (greatestDistance < logicDistances[eliminationCandidates[j]])
		//			{
		//				greatestDistance = logicDistances[eliminationCandidates[j]];
		//				swap = j;
		//			}
		//		}

		//		if (swap != -1)
		//			Btl::Swap(eliminationCandidates[i], eliminationCandidates[swap]);
		//	}
		//}

		////	Ok, last step! Now we've sorted the candidates we plug them back into the m_EliminationPositions list in their new order (at the end)

		//for(int i = 0; i < eliminationCandidates.Size(); i++)
		//{
		//	m_EliminationPositions[(m_EliminationPositions.Size() - eliminationCandidates.Size()) + i] = eliminationCandidates[i];
		//}
	}
}


int C_RaceModeMultiplayer_Elimination::GetEliminationStartPos()
{
	int startElimPos;

	if(m_EliminationNumber <= g_debugEliminateMultiplierNumber + g_debugEliminateMultiplierNumber2)
		startElimPos = ceil((F32)m_EliminationPositions.Size() / 2);
	else
		startElimPos = m_EliminationPositions.Size() - 1;

	return startElimPos;
}

void C_RaceModeMultiplayer_Elimination::InitiateElimination()
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	UpdateRacePositions();

	/*if(m_EliminationPositions.Size() < 2)
	return;*/
	if(!m_EliminationPositions.Size())
		return;

	/*int startElimPos = GetEliminationStartPos();

	NetVehicleIndexStack eliminatedVehicleNetJoinIds;

	for( int i = startElimPos; i < m_EliminationPositions.Size(); i++ )
	{
	U8 eliminationIndex = m_EliminationPositions[i];
	eliminatedVehicleNetJoinIds.PushBack(levelState.mCars[eliminationIndex].mNetJoinId);

	if(VehicleIsLocal(eliminationIndex))
	{		
	VehicleFinished(eliminationIndex, E_VEHICLE_STATE_ELIMINATED);
	}
	}*/
	////	Tell the other players
	//C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_Eliminated(eliminatedVehicleNetJoinIds);
	////	Give any local players who survived a bonus
	//LocalEliminationSurvival();

	m_CurrentBarrageTargetIndex = m_EliminationPositions[m_EliminationPositions.Size() - 1];
	m_BarrageInProgress = true;
	m_RemainingBarrageTimer = g_debugBarrageLength;
	m_TimeToNextShotInBarrage = 0.f;	//	Fire the first shot right away
}

void C_RaceModeMultiplayer_Elimination::EliminateVehicle( U8 p_Index )
{
	C_RaceModeMultiplayer::EliminateVehicle(p_Index);
	m_EliminationLapCounter = 0;
	m_EliminationTimer = m_EliminationPeriod;
}

void C_RaceModeMultiplayer_Elimination::MpMode_SortResults()
{
	NetVehicleIndexStack baseStack;
	SortAllNetJoinIds(baseStack);

	m_EliminationPositions.Clear();

	NetVehicleIndexStack vehiclesFinished;
	NetVehicleIndexStack vehiclesEliminated;
	NetVehicleIndexStack vehiclesWrecked;
	NetVehicleIndexStack vehiclesLeftRace;

	for(S32 i = 0; i < baseStack.Size(); i++)
	{
		switch(VehicleData(baseStack[i]).m_State)
		{
		case E_VEHICLE_STATE_RACING:
			m_EliminationPositions.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_FINISHED:
			vehiclesFinished.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_ELIMINATED:
			vehiclesEliminated.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_FATALLY_DAMAGED:
			vehiclesWrecked.PushBack(baseStack[i]);
			break;
		case E_VEHICLE_STATE_LEFT_RACE:
			vehiclesLeftRace.PushBack(baseStack[i]);
			break;
		}
	}

	SortByStatistic(vehiclesFinished, UI::E_MultiplayerStat_RaceTime, false);
	UpdateEliminationOrder(m_EliminationPositions);
	SortByStatistic(vehiclesEliminated, UI::E_MultiplayerStat_DistanceTravelled, true);
	SortByStatistic(vehiclesWrecked, UI::E_MultiplayerStat_DistanceTravelled, true);
	SortByStatistic(vehiclesLeftRace, UI::E_MultiplayerStat_DistanceTravelled, true);

	//	Now we've sorted the vehicles, we put them into the results structure and give them final positions
	S8 position = 0;
	
	CopyFromStackIntoResults(vehiclesFinished, position);
	CopyFromStackIntoResults(m_EliminationPositions, position);
	CopyFromStackIntoResults(vehiclesEliminated, position);
	CopyFromStackIntoResults(vehiclesWrecked, position);
	CopyFromStackIntoResults(vehiclesLeftRace, position);
}

bool C_RaceModeMultiplayer_Elimination::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if(!ShouldHandleMessages())
		return false;

 	switch(pMessage->Type())
	{
	case E_GameLogicMessage_LapIncremented:
		{
			const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
			if(pMsg->mLap <= 1)
				break;

			if(m_EliminationEvent == UI::E_EliminationEvent_LapFirst)
			{
				U8 vehicleIndex = pMsg->mVehicleIndex;
				Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
				if(pLogicActor)
				{
					if(pLogicActor->GetPositionInfo().mRacePosition == 1)
					{
						InitiateElimination();
					}
				}
			}
			else if(m_EliminationEvent == UI::E_EliminationEvent_LapSecondLast)
			{
				m_EliminationLapCounter++;
				if(m_EliminationLapCounter >= GetNumGlobalRacing() - 1)
				{
					InitiateElimination();
					m_EliminationLapCounter = 0;
				}
			}
			//	Don't return as we want RaceModeMultiplayer to handle this message too
		}
		break;
	}
	return C_RaceModeMultiplayer::MessageHandler(pMessage);
}

/*********************************************	ELIMINATION POINTS CALCULATORS	*********************************/

F32 C_RaceModeMultiplayer_Elimination::GetEliminationPoints( U8 p_VehicleIndex )
{
	F32 eliminationPoints = 0;
	switch(m_EliminationType)
	{
	case UI::E_EliminationType_LastPlace:	//	Fall-through
	case UI::E_EliminationType_FirstPlace:
		{
			//	Find race position
			Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(p_VehicleIndex);
			if(pLogicActor)
			{
				eliminationPoints = pLogicActor->GetPositionInfo().mLogicPositionCombined;
			}
		}
		break;

	case UI::E_EliminationType_LeastHealth:	//	Fall-through
	case UI::E_EliminationType_MostHealth:
		{
			//	Find gameplay health
			Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(p_VehicleIndex);
			if(pLogicActor)
			{
				eliminationPoints = (F32)pLogicActor->GetCurrentHealth();
			}
		}
		break;

	case UI::E_EliminationType_LeastFans:	//	Fall-through
	case UI::E_EliminationType_MostFans:
		{
			//	Find vehicle's on-track fans
			Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(p_VehicleIndex);
			if(pLogicActor)
			{
				eliminationPoints = (F32)pLogicActor->GetScoring().mFinalScore;
			}
		}
		break;

	default:
		break;
	}
	return eliminationPoints;
}

void C_RaceModeMultiplayer_Elimination::DropShockDome()
{
	//	Get world matrix of target vehicle
	Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_CurrentBarrageTargetIndex);
	if(pLogicActor)
	{
		float speed = pLogicActor->GetPositionInfo().mForwardSpeed;
		Maths::HMat43 vehicleMat = pLogicActor->GetWorldMatrix();
		Maths::HVec3 forwardVec = vehicleMat.AZ();
		Maths::HVec3 shockPos = vehicleMat.AW();

		if(speed > g_debugMinimumRandomiseDomeSpeedThreshhold)	//	Magic number wheeeee! If the vehicle's sitting still or moving REALLY slowly, just drop a shock dome right on top of them (catch idle players).
		{
			forwardVec.Normalise();
			F32 distPerMph = (g_debugMetresForwardPerMph * speed);
			forwardVec *= distPerMph;

			shockPos += forwardVec;

			if(g_debugRandomiseShockPosition)
				RandomiseShockPosition(shockPos);
		}

		Game::C_PickupsManager::Instance().GetShockManager().TraceToGround(shockPos);
		vehicleMat.AW(shockPos);

		PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHOCK, E_PICKUP_SUB_TYPE_NORMAL, SYSTEM_FIRING_INDEX);
		pickupInfo.mTargetIdx			= m_CurrentBarrageTargetIndex;
		pickupInfo.mMatrix				= vehicleMat;
		pickupInfo.mDisableVehicleFX	= true;
		pickupInfo.mOverrideMatrix		= true;
		pickupInfo.mVal					= g_debugTestShockWidth;
		pickupInfo.mVal2				= 1.0;
		pickupInfo.mPickupId			= C_PickupsManager::Instance().AllocateUniqueId(SYSTEM_FIRING_INDEX);
		pickupInfo.mHostIsController	= true;
		pickupInfo.mDelay				= 0.0f;
		pickupInfo.mNoPhysicalEffect	= true;	//	Remove speed scrub from our barrage shocks.

		Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
	}
}

void C_RaceModeMultiplayer_Elimination::RandomiseShockPosition( Maths::HVec3& pos )
{
	F32 xDif = (F32(m_Random.Rand(g_debugBarrageZoneRadius * 1000 *2)) / 1000) - g_debugBarrageZoneRadius;
	F32 zDif = (F32(m_Random.Rand(g_debugBarrageZoneRadius * 1000 *2)) / 1000) - g_debugBarrageZoneRadius;
	pos.X(pos.X() + xDif);
	pos.Z(pos.Z() + zDif);
}

void C_RaceModeMultiplayer_Elimination::RewardSurvivingVehicles()
{
	NetVehicleIndexStack localRacingList;
	if(GetLocalRacingList(localRacingList) == 0 ||(m_EliminationNumber - 1) < 2)
		return;

	C_VehicleLogicMsg_Survived_EliminationMsg surviveBonusMsg(m_EliminationNumber - 1, 10);
	for(int i = 0; i < localRacingList.Size(); i++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(localRacingList[i]);
		if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring))
		{
			//Tell the scoring component
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &surviveBonusMsg);
		}
	}
	
}

} // namespace Game