#include "stdafx.h"

#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Network_Pickups.h>

#include <GameApp.h>
#include <Game/Game.h>

#include <Game/Messaging/GameMessages.h>
#include <Game\Messaging\GameflowMessages.h>
#include <Game/Messaging/PickupActorMessages.h>

#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <shared_actors/SharedActorUtils.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>


#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\Structures\NetGameVars.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic\MultiplayerUtilities.h>

#include <GameLogic\Pickups\Normal\PickupShock.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <ui/UiMessages.h>

#include "GameAudio/GameAudioLayer.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"


extern Debug::Bool g_debugShowShockDebug;
extern Debug::Bool g_debugShowShoulderBargeDebug;
extern Debug::Bool g_debugShowMineDebug;

extern Debug::Bool g_debugShufflePerks;

extern Debug::Bool g_debugBankRepairPerk;

extern Debug::Logger &g_debugPickupMpSyncLogger;

namespace Components
{

bool C_VehicleComponent_MP_Network_Pickups::CreateImpl()
{
	const I_VehicleComponent_MP_Network_Pickups_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Network_Pickups_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	//AddSubscriber( GameMessaging::E_GameMsg_Update );

	AddSubscriber( Game::E_PickupMessage_GrantPickup );
	AddSubscriber( Game::E_PickupMessage_RemovePower );
	AddSubscriber( Game::E_PickupMessage_Shuffle );

	AddSubscriber( Game::E_VehicleMessage_FirePickup );

	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_InstaPickupFire );

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_QuitRace );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );

	Initialise();

	mIsMultiplayer = true;

	return true;
}

void C_VehicleComponent_MP_Network_Pickups::DropAndRemoveUsedPickup(E_PICKUP_TYPE pickup)
{
	//	We don't actually do anything here because the HandleDropPickupOnTrack() call will happen when a DropPickupOnTrack net event is received.
}

bool C_VehicleComponent_MP_Network_Pickups::OnMessage( const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case Game::E_PickupMessage_PickupPickedUp:
		{
			//	If we get in here, a network vehicle has driven through an on-track pickup.
			//	That's all well and good, but we don't want to actually give it this pickup - that would be MADNESS.
			return true;
		}
		break;
	}

	return C_VehicleComponent_Pickups::OnMessage(pMessage);
}

void C_VehicleComponent_MP_Network_Pickups::AttemptHitByPickup(E_PICKUP_TYPE pickupType,E_PICKUP_SUB_TYPE pickupSubType, E_PICKUP_HIT_TYPE hitType, int firingIdx, E_TARGETTING_DIRECTION hitDir, U32 pickupId, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId )
{
	//	This method will only be called if a network vehicle has potentially hit with a pickup, which can only happen with certain power-ups where the attacker judges who has been hit.
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	Game::C_NetGameVars& netGameVars = Game::C_Game::GetInstance()->GetNetGameVars();

	U8 myId = Game::GetJoinIdFromLocalIndex(mVehicleIndex);
	U8 firerId = Game::GetJoinIdFromLocalIndex(firingIdx);

	float timeShielded = pActor->mShieldTime;
	if( timeShielded > 0.f && pickupType != E_PICKUP_TYPE_NITROUS)
	{
		//Tell the other vehicle we think we hit their shield
		netGameVars.m_EventManager->Add_JudgedPowerupShielded(myId, firerId, pickupType, pickupId);
	}
	else
		//Tell the other vehicle we think we hit them
		netGameVars.m_EventManager->Add_HitByPickup(true, myId, firerId, pickupType, pickupSubType, hitType, hitDir, pickupId, groupId, hitInfo);

	//	Let's tell pass this message on ASAP.
	Game::ReduceUpdateTimestep(true);
}

} // namespace Components
