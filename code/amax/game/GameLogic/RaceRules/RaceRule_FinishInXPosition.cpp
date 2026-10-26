#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_FinishInXPosition.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

bool RaceModeRule_DriverFinishInXPosition::IsTriggered(int vehicleIndex)
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

	return( pLogicActor->GetPositionInfo().mRacePosition <= mPositionNeeded );
}

bool RaceModeRule_DriverFinishInXPosition::IsValid()
{
	// A goal that you can always pass
	if( mPositionNeeded == 99 )
		return false;

	return true;
}

void RaceModeRule_DriverFinishInXPosition::GetRuleString(U32 vehicleIndex, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
{
	switch(stringType)
	{
	case RACE_MODE_RULE_STRING_TYPE_RESULTS:
	case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
	case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
		{
			if (mDriverID == 0)
			{
				static UI::C_TextStringID string1("RR_FinishInXPosition_HUD_Current_You_1");
				static UI::C_TextStringID string("RR_FinishInXPosition_HUD_Current_You");

				if (mPositionNeeded == 1)
					str.Format(string1.String());
				else
					str.Format(string.String(), mPositionNeeded);
			}
			else
			{
				static UI::C_TextStringID string("RR_FinishInXPosition_HUD_Current_Other");
				str.Format(string.String(), mDriverToBeatLocString.Chars());
			}
		}
		break;

		/*
	case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		{
			UI::C_WideString16 passStr;
			if( isPassRule )
			{
				if(mDriverID)
				{
					static UI::C_TextStringID string("RR_FinishInXPosition_Driver");
					str.Format(string.String(), mDriverToBeatLocString.Chars(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mPositionNeeded ));
				}
				else
				{
					static UI::C_TextStringID string("RR_FinishInXPosition");
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mPositionNeeded ));
				}
			}
			else
			{
				if(mDriverID)
				{
					static UI::C_TextStringID string("RR_FinishInXPosition_Not_Driver");
					str.Format(string.String(), mDriverToBeatLocString.Chars(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mPositionNeeded ));
				}
				else
				{
					static UI::C_TextStringID string("RR_FinishInXPosition");
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mPositionNeeded ));
				}
			}			
		}
		break;

	case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		{
			static UI::C_TextStringID string("RR_FinishInXPosition_HUD");
			str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mPositionNeeded ));
		}
		break;

	case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
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

			if(mDriverID)
			{
				static UI::C_TextStringID string("RR_FinishInXPosition_HUD_Current_Driver");
				str.Format(string.String(), mDriverToBeatLocString.Chars(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition ), C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumVehicles());
			}
			else
			{
				static UI::C_TextStringID string("RR_FinishInXPosition_HUD_Current");
				str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition ), C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetNumVehicles());
			}
		}
		break;
		*/
	}
}

void RaceModeRule_DriverFinishInXPosition::CacheStrings()
	{
		mDriverToBeatLocString.Format(L"%s", UI::C_TextStringID(mDriverToBeatLocId.id).String());
	}

void RaceModeRule_DriverFinishInXPosition::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	out << mDriverID << mPositionNeeded;
	RaceModeRuleBase::Serialise(stream);
}

void RaceModeRule_DriverFinishInXPosition::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	in >> mDriverID >> mPositionNeeded;
	RaceModeRuleBase::Deserialise(stream);
}

} // namespace Game