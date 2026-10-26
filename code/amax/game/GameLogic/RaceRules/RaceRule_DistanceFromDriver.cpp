#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic\RaceRules\RaceRule_DistanceFromDriver.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>
#include <ai/AI_Amax.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverDistanceFromDriver::IsTriggered(int vehicleIndex)
	{
		C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		int vehIdx			= vehicleIndex;
		int vehToBeatIdx	= -1;

		for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
		{
			const LevelStateCar& car = levelState.mCars[a];
			if( car.mCharacterID == mDriverID )
				vehIdx = a;

			if( car.mCharacterID == mDriverDistanceID )
				vehToBeatIdx = a;
		}

		if(vehToBeatIdx == -1)
			return false;

		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		const Game::Actors::C_VehicleLogicActor* pLogicActorToBeat = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehToBeatIdx);

		float lapDistance = AIAmax::AmaxAIR::Instance().GetTrackDataSystem()->GetTrackData()->m_TrackLogicData.m_noSamples * 
							AIAmax::AmaxAIR::Instance().GetTrackDataSystem()->GetTrackData()->m_TrackLogicData.m_stepSize;

		float myDistance	= pLogicActor->mPositionInfo.mLogicPositionCombined * lapDistance;
		float hisDistance	= pLogicActorToBeat->mPositionInfo.mLogicPositionCombined * lapDistance;

		bool distanceMet = false;
		switch(mQuantifier)
		{
		case E_QUANTIFIER_LESSTHAN:
			distanceMet =	hisDistance - myDistance <= mDistance;
			break;

		case E_QUANTIFIER_LESSTHANOREQUAL:
			distanceMet =	hisDistance - myDistance <= mDistance;
			break;

		case E_QUANTIFIER_EQUAL:
			distanceMet =	hisDistance - myDistance <= mDistance;
			break;

		case E_QUANTIFIER_GREATERTHANOREQUAL:
			distanceMet =	hisDistance - myDistance <= mDistance;
			break;

		case E_QUANTIFIER_GREATERTHAN:
			distanceMet =	hisDistance - myDistance <= mDistance;
			break;
		}

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		if(distanceMet)
			mCurrentTimer[vehicleIndex] += frameTime;
		else
			mCurrentTimer[vehicleIndex] = 0;

		if(mCurrentTimer[vehicleIndex] > mTime)
		{
			mDontShowOnHUD[vehicleIndex] = true;
			return true;
		}

		return false;
	}

	void RaceModeRule_DriverDistanceFromDriver::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID string("RR_DistanceFromDriver_HUD_Current");
				str.Format(string.String(), mDriverLocString.Chars() );
			}
			break;

			/*
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
			{
				if( isPassRule )
				{
					static UI::C_TextStringID string("RR_DistanceFromDriver");
					str.Format(string.String(), mQuantifierString.Chars(), mDistance, mDriverLocString.Chars());
				}
				else
				{
					static UI::C_TextStringID string("RR_DistanceFromDriver_Not");
					str.Format(string.String(), mQuantifierString.Chars(), mDistance, mDriverLocString.Chars());
				}			
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_DistanceFromDriver_HUD_Target");
				str.Format(string.String(), mQuantifierString.Chars(), mDistance, mDriverLocString.Chars() );
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				int vehToBeatIdx = -1;
				for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
				{
					const LevelStateCar& car = levelState.mCars[a];
					if( car.mCharacterID == mDriverID )
						idx = a;

					if( car.mCharacterID == mDriverDistanceID )
						vehToBeatIdx = a;
				}

				if(vehToBeatIdx != -1)
				{
					const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(idx);
					const Game::Actors::C_VehicleLogicActor* pLogicActorToBeat = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehToBeatIdx);

					float dist = Maths::Sqrt(pLogicActor->mVehicleProximities[vehToBeatIdx].mDistanceSquared);
					static UI::C_TextStringID string("RR_DistanceFromDriver_HUD_Current");
					str.Format(string.String(), dist);
				}
			}
			break;
			*/
		}
	}

	void RaceModeRule_DriverDistanceFromDriver::CacheStrings()
	{
		mDriverLocString.Format(L"%s", UI::C_TextStringID(mDriverDistanceLocID.id).String());

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

	void RaceModeRule_DriverDistanceFromDriver::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mDistance;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverDistanceFromDriver::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mDistance;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game