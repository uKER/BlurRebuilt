#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\RaceRules\RaceRule_DriftingInTriggerRegion.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverDriftingInTriggerRegion::IsTriggered(int vehicleIndex)
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

		return ( pLogicActor->GetPositionInfo().mCurrentRaceRuleArea == mAreaRef && pLogicActor->GetStatistics().mIsDriftingWithScore );
	}

	void RaceModeRule_DriverDriftingInTriggerRegion::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverDriftingInTriggerRegion::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace G
