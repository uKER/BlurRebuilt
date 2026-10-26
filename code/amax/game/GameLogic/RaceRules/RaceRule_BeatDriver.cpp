#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRule_BeatDriver.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_BeatDriver::IsTriggered(int vehicleIndex)
	{
		Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		int vehIdx = vehicleIndex;
		int vehToBeatIdx = 0;

		for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
		{
			const LevelStateCar& car = levelState.mCars[a];
			if(car.mCharacterID == mDriverOverrideID)
				vehIdx = a;

			if(car.mCharacterID == mDriverToBeatID)
				vehToBeatIdx = a;
		}

		if(!vehToBeatIdx)
			return false;

		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		const Game::Actors::C_VehicleLogicActor* pLogicActorToBeat = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehToBeatIdx);

		mDriverToBeatPos = pLogicActorToBeat->GetPositionInfo().mRacePosition;

		if(pLogicActor->GetPositionInfo().mRacePosition < mDriverToBeatPos || mDriverToBeatPos == 0)
			return true;

		return false;
	}

	void RaceModeRule_BeatDriver::GetRuleString(U32 vehicleIndex, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_ComeAheadOfX_HUD_Current");
				str.Format(string.String(), mDriverLocString.Chars());
			}
			break;

			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				UI::C_WideString16 passStr;
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_ComeAheadOfX");
					str.Format(string.String(),mDriverLocString.Chars());
				}
				else
				{
					static UI::C_TextStringID string("RR_ComeAheadOfX_Not");
					str.Format(string.String(),mDriverLocString.Chars());
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_ComeAheadOfX_HUD_Target");
				str.Format(string.String(), mDriverLocString.Chars(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( mDriverToBeatPos ));
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				int vehIdx = vehicleIndex;

				if( mDriverOverrideID )
				{
					for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
					{
						const LevelStateCar& car = levelState.mCars[a];
						if(car.mCharacterID == mDriverOverrideID)
						{
							vehIdx = a;
							break;
						}
					}
				}

				const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

				static UI::C_TextStringID string("RR_ComeAheadOfX_HUD_Current");
				str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition ));
			}
			break;
			*/
		}
	}

	void RaceModeRule_BeatDriver::CacheStrings()
	{
		mDriverLocString.Format(L"%s", UI::C_TextStringID(mDriverToBeatLocId.id).String());
	}

	void RaceModeRule_BeatDriver::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverToBeatID << mDriverOverrideID << mDriverToBeatLocId.id;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_BeatDriver::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverToBeatID >> mDriverOverrideID >> mDriverToBeatLocId.id;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game