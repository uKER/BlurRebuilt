#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\RaceRules\RaceRule_CauseXDamageToDriver.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_CauseXDamageToDriver::IsTriggered(int vehicleIndex)
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

		Game::Actors::C_VehicleLogicActor* logicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
		mDriverToBeatDamage[vehicleIndex] = (float)logicActor->mStatistics.mDualVehicleStats[vehIdx].mDamageCausedToDriver / (float)logicActor->GetMaxHealth();

		switch(mQuantifier)
		{
		case E_QUANTIFIER_LESSTHAN:
			return( mDriverToBeatDamage[vehicleIndex] < mDamage );

		case E_QUANTIFIER_LESSTHANOREQUAL:
			return( mDriverToBeatDamage[vehicleIndex] <= mDamage );

		case E_QUANTIFIER_EQUAL:
			return( mDriverToBeatDamage[vehicleIndex] == mDamage );

		case E_QUANTIFIER_GREATERTHANOREQUAL:
			return( mDriverToBeatDamage[vehicleIndex] >= mDamage );

		case E_QUANTIFIER_GREATERTHAN:
			return( mDriverToBeatDamage[vehicleIndex] > mDamage );
		}

		return false;
	}

	void RaceModeRule_CauseXDamageToDriver::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		// PR - not used!
		/*
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_CauseDamageOfX");
					str.Format(string.String(),mDriverLocString.Chars(), mQuantifierString.Chars(), mDamage * 100);
				}
				else
				{
					static UI::C_TextStringID string("RR_CauseDamageOfX_Not");
					str.Format(string.String(),mDriverLocString.Chars(), mQuantifierString.Chars(), mDamage * 100);
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_CauseDamageOfX_HUD_Target");
				str.Format(string.String(), mDriverLocString.Chars(), mDriverToBeatDamage[idx] * 100 );
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_CauseDamageOfX_HUD_Current");
				str.Format(string.String(), mDriverLocString.Chars(), mDamage * 100 );
			}
			break;
		}
		*/
	}

	void RaceModeRule_CauseXDamageToDriver::CacheStrings()
	{
		mDriverLocString.Format(L"%s", UI::C_TextStringID(mDriverLocId.id).String());

		switch(mQuantifier)
		{
		case E_QUANTIFIER_LESSTHAN:
			mQuantifierString.Format(L"%s", UI::C_TextStringID("CMN_LESSTHAN").String());
			break;

		case E_QUANTIFIER_LESSTHANOREQUAL:
			mQuantifierString.Format(L"%s", UI::C_TextStringID("CMN_LESSTHANOREQUAL").String());
			break;

		case E_QUANTIFIER_EQUAL:
			mQuantifierString.Format(L"%s", UI::C_TextStringID("CMN_EQUAL").String());
			break;

		case E_QUANTIFIER_GREATERTHANOREQUAL:
			mQuantifierString.Format(L"%s", UI::C_TextStringID("CMN_GREATERTHANOREQUAL").String());
			break;

		case E_QUANTIFIER_GREATERTHAN:
			mQuantifierString.Format(L"%s", UI::C_TextStringID("CMN_GREATERTHAN").String());
			break;
		}
	}

	void RaceModeRule_CauseXDamageToDriver::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mDriverLocId.id << mDamage << (S32)mQuantifier;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_CauseXDamageToDriver::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mDriverLocId.id >> mDamage >> (S32&)mQuantifier;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace G