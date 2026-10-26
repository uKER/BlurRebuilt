#include "stdafx.h"
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_Network_Damage.h>

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <shared_actors/SharedActorUtils.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

namespace Components
{

bool C_VehicleComponent_MP_Network_Damage::CreateImpl()
{
	const I_VehicleComponent_MP_Network_Damage_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Network_Damage_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mActorHandle=creation_data->mParentActorHandle;
		mVehicleIndex=creation_data->mVehicleIndex;
	}

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( E_VehicleLogicMsg_SetHealth );
	AddSubscriber( E_VehicleLogicMsg_Repair );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( Game::E_GameLogicMessage_Impact );

	Initialise();

	// Cache pointer to owner vehicle's abilities (on vlc_damage parent part of class)
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	BIZ_ASSERT_MSG(levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities(),"Error.. VLC_Damage CreateImpl() unable to obtain pointer to Multiplayer Abilities");
	CacheMultiplayerAbilities(levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities());
	return true;
}

	// -------------------------------------------------------------------------------------------


void C_VehicleComponent_MP_Network_Damage::SetCurrentHealth(int p_Health)
{
	Game::Actors::C_VehicleLogicActor* pLogicActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	pLogicActor->mCurrentHealth = p_Health;

	SetCurrentHealthLevel();

	E_HEALTH_LEVEL currentHealthLevel = pLogicActor->GetCurrentHealthLevel();
	if( currentHealthLevel != mLastKnownHealthLevel && currentHealthLevel == E_HEALTH_LEVEL_WRECKED)
	{
		pLogicActor->GetGraphicalActor(mVehicleIndex)->PlayWreckedEffect();
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Scrambler, false);
	}

	mLastKnownHealthLevel = pLogicActor->GetCurrentHealthLevel();
}

// -------------------------------------------------------------------------------------------

//	We only deal with the visual effect of repairing on network vehicles, as gameplay health is sent around in all regular updates.
void C_VehicleComponent_MP_Network_Damage::RepairVehicle( int repairAmount /*= 0*/ )
{
	Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	RLPickupType* pPickup						= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_REPAIR);
	const RLDamageSystem* pRLDamageSystem		= pActor->GetRLC()->GetRLSystem()->mDamageSystem;

	int previousHealth				= pActor->mCurrentHealth - repairAmount; 
	int amountToRepair				= pActor->mCurrentHealth - previousHealth;
	float previousHealthPercent		= (float)(pActor->mCurrentHealth - repairAmount) / (float)pActor->GetMaxHealth();
	float previousDamagePercent		= 1 - previousHealthPercent;

	float newHealthPercent	= (float)pActor->mCurrentHealth / (float)pActor->GetMaxHealth();
	float repairPercent		= newHealthPercent - previousHealthPercent;

	//Tell the vehicle to repair itself visually
	Actor::C_ActorHandle handle = pActor->GetGraphicalActor(mVehicleIndex)->GetHandle();
	Game::Actors::C_VehicleRepairDamageMsg repairMsg(handle, mVehicleIndex, repairPercent, newHealthPercent, 1.f, (float)amountToRepair);
	pActor->SendMessageToActor(handle, &repairMsg);
}

// -------------------------------------------------------------------------------------------

bool C_VehicleComponent_MP_Network_Damage::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	switch(pMessage->Type())
	{

	//case GameMessaging::E_GameMsg_Update:
	//	{
	//		Update();
	//		return true;
	//	}
	//	break;

	case Game::E_GameflowMessage_RaceStarted:
		{
			mEnabled = true;
			return true;
		}
		break;

	case E_VehicleLogicMsg_SetHealth:
		{
			const C_VehicleLogicSetHealthMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicSetHealthMsg>(pMessage);
			Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

			SetCurrentHealth(pMsg->mHealthPoints);
			return true;
		}

		//	If we get a Repair message, we only repair the visual deformation of the vehicle body, since the gameplay damage is sent in each update
	case E_VehicleLogicMsg_Repair:
		{
			const C_VehicleLogicRepairMsg* pMsg		= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);

			RepairVehicle(pMsg->mHealAmount);
			return true;
		}

		//	We listen to the impact message in order to carry out the physical deformation
	case Game::E_GameLogicMessage_Impact:
			{
				Game::C_ImpactMsg* pMsg						= const_cast<Game::C_ImpactMsg*>(Messaging::MessageCast<Game::C_ImpactMsg>(pMessage));
				Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				Game::Actors::C_VehicleActor* vehActor		= pActor->GetGraphicalActor(mVehicleIndex);

				if(Game::C_Game::GetInstance()->GetLevelState().mDamageMode == E_DAMAGE_MODE_INVULNERABLE)
					return true;

				if( pActor->mShieldTime <= 0.f )
				{
					// int damagePoints = ApplyDamageFromImpulse(pMsg->mCollisionInfo.m_GameplayImpulse);

					if(pMsg->mCollisionInfo.m_ContactPoints.Size() /*&& damagePoints*/)
					{
						PopulateDamageFilter(pMsg->mDamageFilter, pMsg->mCollisionInfo.m_GameplayImpulse);
					}
				}
				else
					pMsg->mCollisionInfo.m_GameplayImpulse = 0.0f;
				
				vehActor->GetActorList()->SendMessageToActor(vehActor->GetHandle(),pMsg);
				
				return true;
			}

	case E_VehicleLogicMsg_HitByPerk:
		{
			const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);
			if( mVehicleIndex == msg->mVehicleIndex )
				HandlePickupDamage(msg->mPickupHitType, msg->mPickupType);
			return true;
		}
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

} // namespace Components
