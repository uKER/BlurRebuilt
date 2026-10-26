#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRule_HitXObjects.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <ui/Game/UILocaliser.h>

namespace Game
{

	bool RaceModeRule_DriverHitXObjects::IsTriggered(int vehicleIndex)
	{
		Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
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

		Game::Actors::C_VehicleLogicActor* logicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);
		Btl::Set<S32>::Iterator it = logicActor->mStatistics.mCriticalObjectsHit.Find(mGroupID);

		if(it != logicActor->mStatistics.mCriticalObjectsHit.End())
			return true;

		return false;
	}

	void RaceModeRule_DriverHitXObjects::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mDriverID << mGroupID;
		RaceModeRuleBase::Serialise(stream);
	}

	void RaceModeRule_DriverHitXObjects::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mDriverID >> mGroupID;
		RaceModeRuleBase::Deserialise(stream);
	}

} // namespace Game