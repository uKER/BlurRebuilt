#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRule_GetLapTimeOfX.h>

#include <ui/Game/UILocaliser.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>


namespace Game
{

	bool RaceModeRule_DriverGetLapTimeOfX::IsTriggered(int vehicleIndex)
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
		return( pLogicActor->GetLapInfo().mBestLapTime > 0 && pLogicActor->GetLapInfo().mBestLapTime <= mLapTime );
	}

	void RaceModeRule_DriverGetLapTimeOfX::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		// PR - not used!
		/*
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				UI::C_WideString16 passStr;
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_LapTimeOfX");
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( mLapTime ) );
				}
				else
				{
					static UI::C_TextStringID string("RR_LapTimeOfX_Not");
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( mLapTime ) );
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(idx);
				static UI::C_TextStringID string("RR_LapTimeOfX_HUD_Target");

				str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( mLapTime ) );
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(idx);
				static UI::C_TextStringID string("RR_LapTimeOfX_HUD_Current");

				if(pLogicActor->GetLapInfo().mBestLapTime > 0)
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( pLogicActor->GetLapInfo().mBestLapTime ) );
				else
					str.Format(string.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( 0 ) );
			}
			break;
		}
		*/
	}

	void RaceModeRule_DriverGetLapTimeOfX::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mLapTime;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverGetLapTimeOfX::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mLapTime;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game