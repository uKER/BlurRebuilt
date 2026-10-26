#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_DriverLastInRace.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverLastInRace::IsTriggered(int vehicleIndex)
	{
		C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		int vehIdx = vehicleIndex;

		if( mDriverID )
		{
			for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
			{
				const LevelStateCar& car = levelState.mCars[a];
				if(car.mCharacterID == mDriverID)
				{
					vehIdx = a;
					break;
				}
			}
		}

		Game::C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor* pLogicActor	= pLayer->GetLogicVehicleActor(vehIdx);

		bool triggered = ( pLayer->GetNumVehicles() - C_RaceRulesManager::Instance().GetNumEliminated() <= 1 && !pLogicActor->IsWreckedOrEliminated());

		if(triggered)
			mDontShowOnHUD[vehicleIndex] = true;

		return triggered;
	}

	void RaceModeRule_DriverLastInRace::GetRuleString(U32 vehicleIndex, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_LastInRace");
				str.Format(string.String());
			}
			break;

			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				UI::C_WideString16 passStr;
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_LastInRace");
					str.Format(string.String());
				}
				else
				{
					static UI::C_TextStringID string("RR_LastInRace_Not");
					str.Format(string.String());
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_LastInRace_HUD_Target");
				str.Format(string.String());
			}
			break;
			*/

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Game::Actors::C_VehicleLogicActor* pLogicActor = pLayer->GetLogicVehicleActor(vehicleIndex);

				int driversRemaining = pLayer->GetNumVehicles() - C_RaceRulesManager::Instance().GetNumEliminated();

				if(!pLogicActor->IsWreckedOrEliminated())
					driversRemaining--;

				static UI::C_TextStringID string("RR_LastInRace_HUD_Current");
				static UI::C_TextStringID stringS("RR_LastInRace_HUD_Current_S");

				if(driversRemaining == 1)
					str.Format(stringS.String(), driversRemaining);
				else
					str.Format(string.String(), driversRemaining);
			}
			break;
		}
	}

	void RaceModeRule_DriverLastInRace::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverLastInRace::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game