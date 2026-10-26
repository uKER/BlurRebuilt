#include "stdafx.h"

#include <GameLogic/Pickups/PickupBase.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>
#include <game/GameDef/Polyflags.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/AchievementsManager.h>
#include <GameLogic\StickerManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include "Game/GameActors/Components/VehicleLogic/VLC_Pickups.h"

#include <ui/UiMessages.h>

#include "Automation\GameDataLogging.h"

#include <GameLogic\Profile\GameProfileManager.h>


#include "GameAudio/GameAudioLayer.h"
#include "GameAudio/PickupSubsystem.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
extern Debug::Logger& g_ShuntInfoLogger;
#endif // #ifndef CFG_GOLD

Debug::Float g_debugPerkStartDelayLong("Perks.PerkStartDelayLow_NoPersist",0.75);
Debug::Float g_debugPerkStartDelay("Perks.PerkStartDelay_NoPersist",0.1);

namespace Game
{
	C_PickupBase::C_PickupBase(S32 classType, U32 id, int firingIdx, E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: mClassType(classType), mPickupType(type), mPickupSubType(subType), mUniqueId(id), mFiringIdx(firingIdx), mKillPickup(false), mNinjaKill(false), mActivated(false), 
			mHealth(0.f), mNoFiringVehicleFX(noFiringVehicleFX), mNoPhysicalEffect(false), mIsBeingReplaced(false), mIsReplacement(false), mParentId(0), 
			mPreSetMatrix(false)
	{
		mScale = Maths::HVec3(1.f, 1.f, 1.f);

		for( int i = 0; i < mVehiclesHitMajor.Size(); i++ )
		{
			mVehiclesHitMajor[i] = false;
			mVehiclesHitMinor[i] = false;
			mVehiclesShielded[i] = false;
		}
			
		//We need to tell the cars so they can keep track of challenges
		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		C_VehicleLogicMsg_FiredPerkMsg fired_msg(mPickupType, 0, mFiringIdx, mUniqueId, mMatrix, 1, false, mPickupSubType, false, 1);

		//[cian] don't play the activate if no vehicle fired the perk (i.e. mine spawned from a forward bomb)
		fired_msg.mNoVehicleFiringIndex  = mNoFiringVehicleFX;

		if(mFiringIdx != SYSTEM_FIRING_INDEX && type != E_PICKUP_TYPE_NITROUS)
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ::PickUpSubsystem::E_PickUpSubsystem_Fire, 0, &fired_msg);

		// NOTE: Nitrous (reverse) executes pickupBase constructor && pickupLocalEffectsBase constructor (below!) Since it runs through 
		// both constructors it sends two lots of 'fired' messages to listening components!! (THIS IS WRONG! giving 2* challenge progress
		// / stat counts etc). Since nitro reverse is the only problem case.. for NOW we will knock it out here.. keeping all nitro fired messages
		// sent from the pickupLocalEffectsBase constructor. 
		// TODO... all this fired logic should probably be sent from one central place (e.g. pickups manager)
		bool sendFiredMsg = true;

		if((type == E_PICKUP_TYPE_NITROUS && subType == E_PICKUP_SUB_TYPE_REVERSED) )
		{
			sendFiredMsg = false;
		}

		if( sendFiredMsg )
		{
			for( U32 vehicleIndex = 0; vehicleIndex < pCore->GetNumVehicles(); vehicleIndex++ )
			{			
				Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(vehicleIndex);
				if(pVehicleLogicActor)
				{
					if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
						pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &fired_msg);
					if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Statistics))
						pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Statistics, &fired_msg);
					if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand))
						pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Fan_Demand, &fired_msg);
					if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring))
						pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &fired_msg);
					if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Abilities))
						pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Abilities, &fired_msg);

				}
			}
		}
		

		// Some stickers need to know when this happens
		Game::C_StickerManager::Instance().MessageHandler(&fired_msg);

		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel)
			mHealth = mHealth = pLevel->mBalancingData->mHealth;	

		mAITrackableData.mAILogicIndex = -1;
		mAITrackableData.mIsTrackable	= false;
		mAITrackableData.mIsShootable	= true;
		mAITrackableData.mVelocity		= Maths::HVec3(0.f, 0.f, 0.f);
		mAITrackableData.mFiringIdx		= mFiringIdx;
		mAITrackableData.mRadius		= 1.f;
		mCreationTime = pCore->GetRaceTime();

		if(firingIdx != -1)
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(firingIdx);
			mMatrix = mStartingMatrix			= pActor->GetWorldMatrix();
		}
	}

	// --------------------------------------------------------------------------------

	C_PickupBase::~C_PickupBase()
	{
		if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
		{
			Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			if(pCore)
			{
				Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				if(pLogicLayer )
				{
					//for( U32 vehicleIndex = 0; vehicleIndex < pCore->GetNumVehicles(); vehicleIndex++ )
					if (VehicleIsLocal(mFiringIdx))
					{		
						//Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(vehicleIndex);
						Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(mFiringIdx);
						if(pVehicleLogicActor)
						{
							C_VehicleLogicMsg_PowerUpDeleted deleted_msg(mFiringIdx, mPickupType, mUniqueId, mGroupId);
							if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring))
								pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &deleted_msg);

							if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
								pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &deleted_msg);
						}
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBase::FireChargingSpecialFX()
	{
		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel)
		{
			if(!mNoFiringVehicleFX && pLevel->mSpecialFX)
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(pLevel->mSpecialFX->mChargingSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Charging);
		}
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBase::ApplyDamage(float damage, U32 id)
	{
		// Parents cannot kill children. Unless they are naughty.
		if(id == mParentId)
			return false;

		// Don't let a pickup hurt me more than once. Particularly useful for MP.
		Btl::Set<U32>::InsertionReturnType insRet = mPickupCollisionSet.Insert(id);
		if(!insRet.inserted)
			return false;

		mHealth -= damage;
		if(mHealth <= 0)
			KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);

		return mKillPickup;
	}

	int C_PickupBase::GetDamageToApply()
	{
		RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);
		return pPickup->mNormal->mBalancingData->GetDamageInflicted();
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBase::CheckCanHit(int vehIdx, E_PICKUP_HIT_TYPE hitType)
	{	
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(vehIdx);

		bool alreadyHit = (hitType == E_PICKUP_HIT_TYPE_MAJOR && mVehiclesHitMajor[vehIdx]) || (hitType == E_PICKUP_HIT_TYPE_MINOR && mVehiclesHitMinor[vehIdx]);
		bool ghosted	= pActor->IsGhosted() && !pActor->IsSecretlyGhosted();
		bool networkCanHit = (CheckLegalHit(vehIdx) || NinjaKillIfAnyHit());

		return (!mKillPickup && !alreadyHit && networkCanHit && !pActor->IsWreckedOrEliminated() && !ghosted);
	}

	void C_PickupBase::NinjaKill(const Game::PickupAutopsyReport& autoReport)	
	{ 
		mKillPickup		= true; 
		mNinjaKill		= true; 
		mAutopsyReport	= autoReport; 

#ifndef CFG_GOLD
		if(mPickupType == E_PICKUP_TYPE_SHUNT)
		{
			LOGDEBUG(g_ShuntInfoLogger, "Shunt %u was told to kill itself quietly - probably by another kit", mUniqueId);
		}
#endif
	};

	// --------------------------------------------------------------------------------

	void C_PickupBase::RegisterHit(int vehIdx, E_PICKUP_HIT_TYPE hitType)
	{			
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(vehIdx);

		if(CheckLegalHit(vehIdx))
		{
			if (pActor->GetAlwaysShieldState())
			{
				Game::PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHIELD, E_PICKUP_SUB_TYPE_NORMAL, vehIdx);
				pickupInfo.mTargetIdx	= vehIdx;
				Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
				return;
			}
		
			if(pActor && pActor->pPickupComponent)
				pActor->pPickupComponent->AttemptHitByPickup(GetType(), mPickupSubType, hitType, mFiringIdx, GetHitDirection(pActor), mUniqueId, mNoPhysicalEffect, mHitInfo, mGroupId);

			RecordHit(vehIdx, hitType);
		}
	}

	// --------------------------------------------------------------------------------
	void C_PickupBase::RecordHit(int vehIdx, E_PICKUP_HIT_TYPE hitType)
	{
		if(hitType == E_PICKUP_HIT_TYPE_MAJOR)
			mVehiclesHitMajor[vehIdx] = true;
		else if(hitType == E_PICKUP_HIT_TYPE_MINOR)
			mVehiclesHitMinor[vehIdx] = true;
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupBase::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;

		if(pActor)
		{
			Maths::HVec3 dir = Maths::Normalise(mMatrix.AW() - pActor->GetWorldMatrix().AW());
			float dotProdZ = Maths::DotProduct(dir, pActor->GetWorldMatrix().AZ());

			if( dotProdZ < -Maths::PiBy4 )
			{
				hitDir = E_TARGETTING_DIRECTION_BEHIND;
			}
			else if( dotProdZ > Maths::PiBy4 )
			{
				hitDir = E_TARGETTING_DIRECTION_AHEAD;
			}
			else
			{
				float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());
				if( dotProdX < 0 )
				{
					hitDir = E_TARGETTING_DIRECTION_LEFT;
				}
				else
				{
					hitDir = E_TARGETTING_DIRECTION_RIGHT;
				}
			}
		}

		return hitDir;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBase::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		if(!mNoFiringVehicleFX)
		{
			if(sfx)
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(*sfx, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing, -1.f, mScale);
			}
			else
			{
				RLPickupLevel* pLevel = GetRLSubTypeLevel();

				if(pLevel && pLevel->mSpecialFX)
				{
					C_InGameSpecialFXManager::Instance().CreateSpecialFX(pLevel->mSpecialFX->mFiredSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Firing, -1.f, mScale);
				}
			}
		}
	}

	void C_PickupBase::SetPostDelayMatrix( Maths::HMat43 matrix, float fVal )
	{
		mMatrix = mStartingMatrix = matrix;
		mActivated = true;

#ifndef CFG_GOLD
		Game::C_ReplayDirector::Instance().AddPickupPostDelay(mUniqueId, matrix, fVal);
#endif
	}

	// --------------------------------------------------------------------------------

	void C_PickupBase::KillPickup(int killed_by_vehicle_index, E_PICKUP_CAUSE_OF_DEATH causeOfDeath )
	{
		mKillPickup = true;

		// save the reason we have been killed for future inspection
		mAutopsyReport.mCauseOfDeath = causeOfDeath;

		if(killed_by_vehicle_index != -1 && killed_by_vehicle_index != mFiringIdx)
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			
			C_VehicleLogicMsg_PowerUpDestroyedByPowerUp msg(killed_by_vehicle_index, mFiringIdx, GetType(), GetSubType(), mUniqueId);

			pLayer->GetLogicVehicleActor(killed_by_vehicle_index)->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &msg);
		}

		//	Don't tell other players to kill a Power-up just because we think it hit a wall/the ground, as this can often result in them suddenly disappearing on remote kits which looks weird
		if(causeOfDeath == E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT || causeOfDeath == E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT)
		{
			NinjaKill(mAutopsyReport);
		}
#ifndef CFG_GOLD
		Game::C_ReplayDirector::Instance().AddKillPickup(mUniqueId);

		if(mPickupType == E_PICKUP_TYPE_SHUNT)
		{
			float xPos = mMatrix.AW().X();
			float yPos = mMatrix.AW().Y();
			float zPos = mMatrix.AW().Z();

			switch(causeOfDeath)
			{
			case E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT:
			case E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT:
				LOGDEBUG(g_ShuntInfoLogger, "Shunt %u collided with with the environment at point (%f, %f, %f)", mUniqueId, xPos, yPos, zPos);
				break;

			case E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP:
				LOGDEBUG(g_ShuntInfoLogger, "Shunt %u collided with with another pickup at point (%f, %f, %f)", mUniqueId, xPos, yPos, zPos);
				break;

			case E_PICKUP_CAUSE_OF_DEATH_OLD_AGE:
				LOGDEBUG(g_ShuntInfoLogger, "Shunt %u died of old age", mUniqueId);
				break;

			case E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE:
				LOGDEBUG(g_ShuntInfoLogger, "Shunt %u collided with a vehicle", mUniqueId);
				break;

			default:
				LOGDEBUG(g_ShuntInfoLogger, "Shunt %u died of an unknown cause", mUniqueId);
				break;
			}
		}
#endif
	}

	// --------------------------------------------------------------------------------

	RLPickupLevel* C_PickupBase::GetRLSubTypeLevel(const E_PICKUP_SUB_TYPE eOverridePickupSubType) const
	{
		RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(mPickupType);

		E_PICKUP_SUB_TYPE ePickupSubType;
		if(eOverridePickupSubType > E_PICKUP_SUB_TYPE_NONE)
			ePickupSubType = eOverridePickupSubType;
		else
			ePickupSubType = mPickupSubType;

		switch(ePickupSubType)
		{
		case E_PICKUP_SUB_TYPE_NORMAL:
		case E_PICKUP_SUB_TYPE_DECOY:
			return pPickup->mNormal.Object();

		case E_PICKUP_SUB_TYPE_REVERSED:
			return pPickup->mReversed.Object();

		case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
			return pPickup->mDoubleTap.Object();

		case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
			return pPickup->mReversedDoubleTap.Object();

		case E_PICKUP_SUB_TYPE_MINI:
			return pPickup->mMini.Object();

		case E_PICKUP_SUB_TYPE_SUB_MOD:
		case E_PICKUP_SUB_TYPE_MOD:
			return pPickup->mModified.Object();

		case E_PICKUP_SUB_TYPE_MOD_REVERSED:
		case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
			return pPickup->mReverseModified.Object();
		}

		return NULL;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBase::SetAbilityActive(const UI::E_MultiplayerVehicleAbilities type, int TargetIdx, bool active)
	{
		// If we ae not blind firing and the target is the local vehicle
		if(TargetIdx != -1 && Game::VehicleIsLocal(TargetIdx))
		{
			//Check for mod
			Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
			Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[TargetIdx].mUpgrades.GetMultiplayerAbilities();

			return pAbilities->ActivateAbilityFiringIcon(type, active);
		}	

		return false;
	}

	bool C_PickupBase::HasAbility(const UI::E_MultiplayerVehicleAbilities type, int vehicleIdx) const
	{
		if(vehicleIdx != -1 && vehicleIdx != SYSTEM_FIRING_INDEX)
		{
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();	
			Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[vehicleIdx].mUpgrades.GetMultiplayerAbilities();

			return pAbilities->GetAbility(type)->GetActive();
		}	

		return false;
	}

	bool C_PickupBase::AbilityActiveOnPickupType(const UI::E_MultiplayerVehicleAbilities type) const
	{
		switch(type)
		{
			case UI::E_MultiplayerVehicleAbilities_Iron_Fist:
				{
					return C_Game::GetInstance()->GetMultiplayerAbilities().IronFistActivatedOnPickupType(mPickupType) > 0;
				}
			//No special case then all powerups allowed
			default:
				return true;
		}
	}

	bool C_PickupBase::GetDidHitTarget( int vehIdx )
	{
		if(vehIdx >= 0)
			return mVehiclesHitMajor[vehIdx] || mVehiclesHitMinor[vehIdx];

		// Use -1 to check for any vehicle
		Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		for( int i = 0; i < pLogicLayer->GetNumVehicles(); i++ )
		{
			if( mVehiclesHitMajor[i] || mVehiclesHitMinor[i] )
				return true;
		}

		return false;
	}

	float C_PickupBase::GetCreationTime() const
	{ 
		return mCreationTime;
	}

	// --------------------------------------------------------------------------------

	C_PickupLocalEffectsBase::C_PickupLocalEffectsBase(E_PICKUP_TYPE type, int vehIdx, float time, E_PICKUP_SUB_TYPE subType, U32 id)
		 : mPickupType(type), mTimer(time), mVehicleIndex(vehIdx), mUniqueId(id)
	{
		//We need to tell the cars so they can keep track of challenges. We should also tell the scoring component.
		Maths::HMat43 matrix;
		C_VehicleLogicMsg_FiredPerkMsg fired_msg(type, 0, vehIdx, -1, matrix, 1, false,subType);

		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::C_RaceLogicLayer* pLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		//send pickup inititated to wheel audio component
		if(pCore && pCore->GetVehicleActor(mVehicleIndex)->QueryInterface(Components::E_AudioEntity_Spawned))
			pCore->GetVehicleActor(mVehicleIndex)->SendMessageToComponent(Components::E_AudioEntity_Spawned,&fired_msg);

		for( U32 vehicleIndex = 0; vehicleIndex < pCore->GetNumVehicles(); vehicleIndex++ )
		{			
			Game::Actors::C_VehicleLogicActor* pVehicleLogicActor = pLogicLayer->GetLogicVehicleActor(vehicleIndex);
			if(pVehicleLogicActor)
			{
				if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
					pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &fired_msg);
				if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Statistics))
					pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Statistics, &fired_msg);
				if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand))
					pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Fan_Demand, &fired_msg);
				if(pVehicleLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring))
					pVehicleLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &fired_msg);
			}
		}

		mPickupSubType = subType;
	}
}
