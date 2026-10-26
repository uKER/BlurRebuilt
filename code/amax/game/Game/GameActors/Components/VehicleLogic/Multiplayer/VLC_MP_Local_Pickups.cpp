#include "stdafx.h"

#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Local_Pickups.h>

#include <GameApp.h>
#include <Game/Game.h>

#include <Game/Messaging/GameMessages.h>
#include <Game\Messaging\GameflowMessages.h>

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

#include <GameLogic\Pickups\Normal\PickupShock.h>
#include <GameLogic\Pickups\PickupMineBase.h>

#include <GameLogic\MultiplayerUtilities.h>

#include <ui/UiMessages.h>

#include "GameAudio/GameAudioLayer.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"

extern Debug::Float g_debugPerkStartDelay;

extern Debug::Bool g_debugShowShockDebug;
extern Debug::Bool g_debugShowShoulderBargeDebug;
extern Debug::Bool g_debugShowMineDebug;

extern Debug::Bool g_debugShufflePerks;

extern Debug::Bool g_debugBankRepairPerk;

extern Debug::Logger &g_debugPickupMpSyncLogger;

extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

extern Debug::Float g_debugDropPickupBackMultiplier;

namespace Components
{

bool C_VehicleComponent_MP_Local_Pickups::CreateImpl()
{
	const I_VehicleComponent_MP_Local_Pickups_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Local_Pickups_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	//AddSubscriber( GameMessaging::E_GameMsg_Update );

	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );
	AddSubscriber( Game::E_PickupMessage_GrantPickup );
	AddSubscriber( Game::E_VehicleMessage_FirePickup );
	AddSubscriber( Game::E_VehicleMessage_DebugRender );

	AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_Ghosted );

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitRace );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );

	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
	AddSubscriber( Game::E_GameLogicMessage_SplitCrossed );
	AddSubscriber( Game::E_PickupMessage_ActivatePickups );

	Initialise();

	mIsMultiplayer = true;

	return true;
}

E_PICKUP_TYPE C_VehicleComponent_MP_Local_Pickups::HandleDropPickupOnTrack(SpawnInformation& information)
{
	E_PICKUP_TYPE type = C_VehicleComponent_Pickups::HandleDropPickupOnTrack(information);
	if(type != E_PICKUP_TYPE_NONE)
	{
		//Only spawn if we have a valid ID (ie. Not 0)
		if(information.mUniqueId)
		{
			const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

			netGameVars.m_EventManager->Add_PickupDroppedOnTrack(information.mUniqueId, information.mType, Game::GetJoinIdFromLocalIndex(mVehicleIndex), information.mPosition);
		}
	}

	return information.mType;
}

E_PICKUP_TYPE C_VehicleComponent_MP_Local_Pickups::HandleDropDecoyOnTrack(SpawnInformation& information)
{
	if(C_VehicleComponent_Pickups::HandleDropDecoyOnTrack(information) != E_PICKUP_TYPE_NONE && information.mUniqueId)
	{
		MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;

		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

		netGameVars.m_EventManager->Add_DecoyDroppedOnTrack(information.mUniqueId, information.mType, Game::GetJoinIdFromLocalIndex(mVehicleIndex), information.mPosition);

		m_DecoyUniqueIds.PushBack(information.mUniqueId);

		if(modSettings.m_DecoyDropsAllowedPerPlayer > 10)
		{
			BIZ_ASSERT_MSG(modSettings.m_DecoyDropsAllowedPerPlayer <= 10, "C_VehicleComponent_MP_Local_Pickups::HandleDropDecoyOnTrack: Decoy drops allowed value above 10 in sorcery. Bad designer! Bad!");
			modSettings.m_DecoyDropsAllowedPerPlayer = 3;
		}

		//Kill our oldest decoy if we have gone over our quota
		if(m_DecoyUniqueIds.Size() > modSettings.m_DecoyDropsAllowedPerPlayer)
		{
			U32 idToKill = m_DecoyUniqueIds.Front();

			//Tell everyone else
			C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_DroppedPickupPickedUp(idToKill);

			//And destroy locally
			mRaceLogicLayerPtr->DestroyDroppedPickupPositionActor(idToKill);

			//Remove it from the list
			m_DecoyUniqueIds.PopFront();
		}
	}

	return information.mType;
}

bool C_VehicleComponent_MP_Local_Pickups::OnMessage( const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case E_VehicleLogicMsg_Ghosted:
		{
			const C_VehicleLogicMsg_GhostedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_GhostedMsg>(pMessage);
			if(mVehicleIndex == pMsg->m_VehicleIndex)
			{
				C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_Ghosted(Game::GetJoinIdFromLocalIndex(pMsg->m_VehicleIndex), pMsg->m_GhostTime);
			}
			return true;
		}
		break;
	}

	return C_VehicleComponent_Pickups::OnMessage(pMessage);
}

bool C_VehicleComponent_MP_Local_Pickups::RegisterHitByPickup(E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE hitType, int firingIdx, U32 pickupId, E_TARGETTING_DIRECTION hitDir, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId)
{
	if(C_VehicleComponent_Pickups::RegisterHitByPickup(pickupType, subType, hitType, firingIdx, pickupId, hitDir, noPhysical, hitInfo, groupId))
	{
		//	The pickup successfully hit so record this event to send to other players
		U8 victimId = Game::GetJoinIdFromLocalIndex(mVehicleIndex);
		U8 firerId = Game::GetJoinIdFromLocalIndex(firingIdx);

		Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_HitByPickup(false, victimId, firerId, pickupType, subType, hitType, hitDir, pickupId, groupId, hitInfo);
		
		//	Let's tell pass this message on ASAP.
		Game::ReduceUpdateTimestep(true);
		return true;
	}
	else
		return false;
}

void C_VehicleComponent_MP_Local_Pickups::RegisterShieldedPickup( E_PICKUP_TYPE pickupType, int firingIdx, U32 pickupId )
{
	C_VehicleComponent_Pickups::RegisterShieldedPickup(pickupType, firingIdx, pickupId);

	U8 shieldedId = Game::GetJoinIdFromLocalIndex(mVehicleIndex);
	U8 firerId = Game::GetJoinIdFromLocalIndex(firingIdx);
	Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_ShieldedPickup(shieldedId, firerId, pickupType, pickupId);
}

bool C_VehicleComponent_MP_Local_Pickups::StartPerk( E_PICKUP_TYPE p_PickupType, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire )
{
	bool fireResult = C_VehicleComponent_Pickups::StartPerk(p_PickupType, forceBackwardFire, forceForwardFire, forceDoubleFire);
	if(fireResult)
	{
		//	Let's tell pass this message on ASAP.
		Game::ReduceUpdateTimestep(true);
	}
	
	return fireResult;
}

} // namespace Components
