#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\RaceRules\RaceRule_HitXMph.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverHitXMph::IsTriggered(int vehicleIndex)
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

		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

		bool speedMet = false;
		switch(mQuantifier)
		{
		case E_QUANTIFIER_LESSTHAN:
			speedMet =	pLogicActor->GetPositionInfo().mSpeedInMph < mSpeedNeeded;
			break;

		case E_QUANTIFIER_LESSTHANOREQUAL:
			speedMet =	pLogicActor->GetPositionInfo().mSpeedInMph <= mSpeedNeeded;
			break;

		case E_QUANTIFIER_EQUAL:
			speedMet =	pLogicActor->GetPositionInfo().mSpeedInMph == mSpeedNeeded;
			break;

		case E_QUANTIFIER_GREATERTHANOREQUAL:
			speedMet =	pLogicActor->GetPositionInfo().mSpeedInMph > mSpeedNeeded;
			break;

		case E_QUANTIFIER_GREATERTHAN:
			speedMet =	pLogicActor->GetPositionInfo().mSpeedInMph >= mSpeedNeeded;
			break;
		}

		return( ( mAreaRef == -1 || pLogicActor->GetPositionInfo().mCurrentRaceRuleArea == mAreaRef ) && speedMet );
	}

	void RaceModeRule_DriverHitXMph::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_HitXMph_HUD_Current");
				str.Format(string.String(),mSpeedNeeded);
			}
			break;

			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_HitXMph");
					str.Format(string.String(),mSpeedNeeded);
				}
				else
				{
					static UI::C_TextStringID string("RR_HitXMph_Not");
					str.Format(string.String(),mSpeedNeeded);
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_HitXMph_HUD");
				str.Format(string.String(),mSpeedNeeded );
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_HitXMph_HUD_Current");
				str.Format(string.String(),mSpeedNeeded);
			}
			break;
			*/
		}
	}

	void RaceModeRule_DriverHitXMph::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mSpeedNeeded;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverHitXMph::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mSpeedNeeded;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game