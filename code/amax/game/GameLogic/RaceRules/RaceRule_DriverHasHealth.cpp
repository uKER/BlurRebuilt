#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\RaceRules\RaceRule_DriverHasHealth.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverHasHealth::IsTriggered(int vehicleIndex)
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

			bool triggered = CheckQuantifier(vehIdx);

			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)mDriverID];
			bool isPlayer = (character && character->type == E_CHARACTER_TYPE_PLAYER);

			if(triggered && !isPlayer)
				mDontShowOnHUD[vehicleIndex] = true;

			return triggered;
		}
		else
		{
			for( unsigned int i = 0; i < levelState.mNumCars; i++ )
			{
				if( i != vehicleIndex && CheckQuantifier(i))
					return true;
			}
		}

		return false;
	}

	bool RaceModeRule_DriverHasHealth::CheckQuantifier(int vehIdx)
	{
		Game::Actors::C_VehicleLogicActor* logicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		mDriverToBeatHealth = (float)logicActor->GetCurrentHealth() / (float)logicActor->GetMaxHealth();

		bool triggered = false;

		switch(mQuantifier)
		{
		case E_QUANTIFIER_LESSTHAN:
			triggered = ( mDriverToBeatHealth < mHealth );
			break;

		case E_QUANTIFIER_LESSTHANOREQUAL:
			triggered = ( mDriverToBeatHealth <= mHealth );
			break;

		case E_QUANTIFIER_EQUAL:
			triggered = ( mDriverToBeatHealth == mHealth );
			break;

		case E_QUANTIFIER_GREATERTHANOREQUAL:
			triggered = ( mDriverToBeatHealth >= mHealth );
			break;

		case E_QUANTIFIER_GREATERTHAN:
			triggered = ( mDriverToBeatHealth > mHealth );
			break;
		}

		return triggered;
	}

	void RaceModeRule_DriverHasHealth::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)mDriverID];
		bool isPlayer = (character && character->type == E_CHARACTER_TYPE_PLAYER);

		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				if( mDriverID )
				{
					static UI::C_TextStringID string("RR_HealthLevelOfX_HUD_Current");
					static UI::C_TextStringID stringYou("RR_HealthLevelOfX_HUD_Current_You");

					if(isPlayer)
						str.Format(stringYou.String());
					else
						str.Format(string.String(), mDriverLocString.Chars());
				}
				else
				{
					static UI::C_TextStringID string("RR_HealthLevelOfX_NoDriver_HUD_Current");
					str.Format(string.String());
				}

			}
			break;

			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_HealthLevelOfX");
					str.Format(string.String(),mDriverLocString.Chars(), mQuantifierString.Chars(), mHealth * 100);
				}
				else
				{
					static UI::C_TextStringID string("RR_HealthLevelOfX_Not");
					str.Format(string.String(),mDriverLocString.Chars(), mQuantifierString.Chars(), mHealth * 100);
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_HealthLevelOfX_HUD_Target");
				str.Format(string.String(), mDriverLocString.Chars(), mHealth * 100 );
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_HealthLevelOfX_HUD_Current");
				str.Format(string.String(), mDriverLocString.Chars());
			}
			break;
			*/
		}
	}

	void RaceModeRule_DriverHasHealth::CacheStrings()
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

	void RaceModeRule_DriverHasHealth::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mDriverLocId.id << mHealth << (S32)mQuantifier;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverHasHealth::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mDriverLocId.id >> mHealth >> (S32&)mQuantifier;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game