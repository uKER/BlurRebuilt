#include "stdafx.h"

#include "VLC_Pickups_Targetting.h"

#include <Game/Messaging/GameMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include "Ui\UiMessages.h"

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/RaceLogicCore/Data/RLVehicleData.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic/Pickups/PickupsManager.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

static Debug::Logger &g_PickupTargettingLogger = Debug::GetLogger("Vehicle.Components.Targetting");
Debug::Float gTargettingWarningRange("Vehicle.Components.Targetting.WarningRange", 1.1f, Debug::Persist());

namespace Components
{

	bool C_VehicleComponent_Pickups_Targetting::CreateImpl()
	{
		const I_VehicleComponent_Pickup_Targetting_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Pickup_Targetting_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
			mpRaceLogicLayer=creation_data->mpRaceLogicLayer;
		}

		return true;
	}

	void C_VehicleComponent_Pickups_Targetting::Update()
	{
		PERF_EVENT("C_VehicleComponent.PickupsTargetting.Update");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(mpRaceLogicLayer->GetPerksActivatedIn())
			return;

		// Check if I am in range of an enemy with a direct pickup
		//for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		//{
		//	if( mVehicleIndex == i )
		//		continue;

		//	Game::Actors::C_VehicleLogicActor* otherVehActor = (Game::Actors::C_VehicleLogicActor*)mpRaceLogicLayer->GetLogicVehicleActor(i);
		//	if(otherVehActor->mTargetLockedOn)
		//	{
		//		UI::C_UiVehicleInRangeMessage vehInRangeMsg (mVehicleIndex, otherVehActor->mVehicleIndex);
		//		_UI_MESSAGE_FULL( vehInRangeMsg );
		//	}
		//}

		if(pActor->GetPickupInfo().mCurrentTargetIdx != -1 && pActor->GetPickupInfo().mPickupStoreIndex != -1 && 
			pActor->GetPickupInfo().mPickupStore[pActor->GetPickupInfo().mPickupStoreIndex].mType == E_PICKUP_TYPE_SHUNT) // Are powerup bays enabled???
		{
			// Check if powerup bays are disabled.. (through a mod effect.. or being wrecked) if so .. dont target
			Components::C_VehicleComponent_Pickups* pPickups = (Components::C_VehicleComponent_Pickups*)pActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups);
			if( pPickups && !pPickups->GetPickupsDisabled() &&  pActor->GetCurrentHealthLevel() != E_HEALTH_LEVEL_WRECKED)
			{
				UI::C_UiVehicleInRangeMessage vehInRangeMsg (mVehicleIndex, pActor->GetPickupInfo().mCurrentTargetIdx);
				_UI_MESSAGE_FULL( vehInRangeMsg );
				
			}
		}

		if(pActor->mTargetLockedOn != E_PICKUP_LOCK_ON_NONE)
		{
			UI::C_UiPlayerInRangeWarningMessage warningMsg( pActor->mVehicleIndex, pActor->mTargetLockedOn );
			_UI_MESSAGE_FULL( warningMsg );
		}
	}

	bool C_VehicleComponent_Pickups_Targetting::OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		return false;
	}

}
