#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_GetFastestLap.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverGetFastestLap::IsTriggered(int vehicleIndex)
	{
		Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		int vehIdx = vehicleIndex;

		if( mDriverID )
		{
			for(unsigned int a=0; a < C_Game::GetInstance()->GetLevelState().mNumCars; a++)
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

		if( pLogicActor->mLapsInfo.mCurrentLap <= 1 )
			return false;

		float mFastestLap = -1;
		int mFastestCarIdx = -1;

		for( U32 i = 0; i < pRLC->GetNumVehicles(); i++ )
		{
			if( mFastestCarIdx == -1 || pLogicActor->mLapsInfo.mBestLapTime > mFastestLap )
			{
				mFastestCarIdx = i;
				mFastestLap = pLogicActor->mLapsInfo.mBestLapTime;
			}
		}

		return( vehIdx == mFastestCarIdx );
	}

	void RaceModeRule_DriverGetFastestLap::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverGetFastestLap::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game