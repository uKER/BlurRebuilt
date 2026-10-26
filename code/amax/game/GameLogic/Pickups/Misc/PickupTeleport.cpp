#include "stdafx.h"

#include <GameLogic\Pickups\Misc\PickupTeleport.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/Messaging/PickupActorMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

#include <Ui\UiMessages.h>

namespace Game
{
	bool C_PickupFireTeleport::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_TELEPORT);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count)
			return true;

		RLTeleportBalancingData* pTeleportData = (RLTeleportBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay	=0.f;
				SendFirePerkMessage();
			}

			UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_CHARGING,mDelay,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_NONE);
			_UI_MESSAGE_FULL( uimsg );

			return false;
		}




		return true;
	}

	void C_PickupFireTeleport::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			if(mFiringVehicle == a)
				continue;

			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			float inRange = pActor->InRangeOfPickup(GetType(), mLevel, a);
			if(inRange <= 1.0f)
				pOtherActor->mTargetInDanger[mFiringVehicle] = true;
			else
				pOtherActor->mTargetInDanger[mFiringVehicle] = false;
		}
	}
	


	bool C_PickupHitTeleport::Update(int vehIdx, bool p_IsMultiplayer)
	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleLogicActor* pHitActor		= pLayer->GetLogicVehicleActor(vehIdx);


		return true;
	}
}
