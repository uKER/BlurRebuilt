#include "stdafx.h"

// All these are need to access the racelogiccore... Must be a better way?
#include <GameApp.h>
#include <Game/Game.h>

#include <Game\Messaging\GameflowMessages.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <Game\GameActors\Components\GameComponents.h>
#include "VLC_Fan_Power.h"

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <actor_system/ActorSys.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

// [GY] - temp
#include <GameLogic\RaceModes\RaceModeDefines.h>
#include <GameLogic\Structures\NetGameVars.h>

// ! temp

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <game/gamecontrols/GameControls.h>

#include <RaceBook\RaceBookManager.h>
#include <RaceBook\Objects\Group.h>
#include <RaceBook\Data\RaceBookData.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include "Ui/UiMessages.h"
#include "Automation\GameDataLogging.h"

#include <System/Misc/Random.h>
  
static Util::C_Random g_CriticalMassRandom;

Debug::Float g_debugShieldRepelForce("Perks.Shield.RepelForce", 1000.0f, Debug::Persist());
Debug::Float g_debugShieldRepelLastCollisionTime("Perks.Shield.RepelCollisionTime", 0.5f, Debug::Persist());

Debug::Int g_debugShieldsEarnedRepairSinglePlayer("Perks.Shield.ShieldsEarnedRepairSinglePlayer", 3);
Debug::Int g_debugShieldsEarnedHitByPerkSinglePlayer("Perks.Shield.ShieldsEarnedHitByPerkSinglePlayer", 1);
Debug::Int g_debugShieldsEarnedFansSinglePlayer("Perks.Shield.ShieldsEarnedFansSinglePlayer", 0);
Debug::Int g_debugShieldsEarnedFansRequiredSinglePlayer("Perks.Shield.ShieldsEarnedFansRequiredSinglePlayer", 0);
Debug::Int g_debugShieldsInitialSinglePlayer("Perks.Shield.InitialShieldSinglePlayer", 1);

Debug::Int g_debugShieldsEarnedRepairMultiPlayer("Perks.Shield.ShieldsEarnedRepairMultiPlayer", 3);
Debug::Int g_debugShieldsEarnedHitByPerkMultiPlayer("Perks.Shield.ShieldsEarnedHitByPerkMultiPlayer", 0);
Debug::Int g_debugShieldsEarnedFansMultiPlayer("Perks.Shield.ShieldsEarnedFansMultiPlayer", 0);
Debug::Int g_debugShieldsEarnedFansRequiredMultiPlayer("Perks.Shield.ShieldsEarnedFansRequiredMultiPlayer", 45);
Debug::Int g_debugShieldsInitialMultiPlayer("Perks.Shield.InitialShieldMultiPlayer", 1);

Debug::Int g_debugShieldsEarnedRepairSplitScreen("Perks.Shield.ShieldsEarnedRepairSplitScreen", 3);
Debug::Int g_debugShieldsEarnedHitByPerkSplitScreen("Perks.Shield.ShieldsEarnedHitByPerkSplitScreen", 0);
Debug::Int g_debugShieldsEarnedFansSplitScreen("Perks.Shield.ShieldsEarnedFansSplitScreen", 0);
Debug::Int g_debugShieldsEarnedFansRequiredSplitScreen("Perks.Shield.ShieldsEarnedFansRequiredSplitScreen", 45);
Debug::Int g_debugShieldsInitialSplitScreen("Perks.Shield.InitialShieldSplitScreen", 1);

extern Debug::Bool g_ForceShieldsTiedToPickups;
extern Debug::Logger &g_MultiplayerLog;
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;

//Debug::Float g_debugShieldOverloadDegradeFactor("Perks.Shield.OverloadDegradeFactor", 0.25f, Debug::Persist());
//Debug::Float g_debugShieldOverloadAmount("Perks.Shield.OverloadAmount", 0.25f, Debug::Persist());

//Debug::Float g_debugShieldCriticalMassTime("Perks.Shield.CriticalMassTime", 10.f, Debug::Persist());

namespace Components
{

bool C_VehicleComponent_Fan_Power::CreateImpl()
{
	const I_VehicleComponent_Fan_Power_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Fan_Power_CreateInfo *>(GetCreator().GetData());
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	vehActor->mShieldOverload=0.f;
	vehActor->mFanPower=0.f;
	
	vehActor->mCriticalMassTimer=0.f;
	vehActor->mCriticalMassTimer=0.f;
	vehActor->mGoneCriticalMass=false;

	mUsingShieldTimer=0.f;

	mRaceStarted=false;

	mDisableShield=0.f;

	// This needs initialising. Incredibly long loops are bad. 
	mCurrentFans = 0; 

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	if(levelState.mCars[mVehicleIndex].mUpgrades.Shield())
	{
		//[mtaylor] - start with no shield slots (testing Craig's new shield system, 21 July 09)
		if( Game::C_Game::GetInstance()->GetLevelState().mShieldsTiedToPickups || g_ForceShieldsTiedToPickups )
		{
			vehActor->mFanPower = 0.f;
		}
		else
		{
			vehActor->mFanPower = (float)levelState.mCars[mVehicleIndex].mShieldsInitial;
			vehActor->mFanPower = Maths::Clamp(vehActor->mFanPower,0.f,(float)levelState.mCars[mVehicleIndex].mUpgrades.NumShieldSlots());
		}
	}

	mFansRequired = levelState.mCars[mVehicleIndex].mShieldsEarnedFansRequired;
	for( int i = 0; i < RaceBook::C_RaceBookManager::Instance().GetNumGroups(); i++ )
	{
		RaceBook::C_RaceBookGroup* grp = RaceBook::C_RaceBookManager::Instance().GetGroup(i);
		for( int j = 0; j < grp->GetNumEvents(); j++ )
		{
			if(grp->GetEvent(j)->GetID() == levelState.mEvent)
			{
				RaceBookGroup *rb_grp	= C_RaceBookData::Instance().mGroups[(GAMEDATA_ID)grp->GetID()];
				mFansRequired *= (int)(Maths::Floor(rb_grp->score_multiplier));
				break;
			}
		}
	}

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( E_VehicleLogicMsg_FansAwarded );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
	AddSubscriber( E_VehicleLogicMsg_Repair );
	AddSubscriber( E_VehicleLogicMsg_EnableDisablePickupSystem );

	AddSubscriber( E_VehicleLogicMsg_CriticalMass );

	AddSubscriber( Game::E_GameflowMessage_QuitRace );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );

	// Cache pointer to owner vehicle's abilities
	mpMultiplayerAbilities = levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(mpMultiplayerAbilities,"Error.. VLC_Fan_Power CreateImpl() unable to obtain pointer to Multiplayer Abilities");

	//	[GY] - temp
	AddSubscriber( E_VehicleLogicMsg_ShieldTime );

	return true;
}

void C_VehicleComponent_Fan_Power::RepelNearbyVehicles()
{
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
	{
		if( i == mVehicleIndex )
			continue;

		if( pActor->mTimeSinceLastCollision[i] <= g_debugShieldRepelLastCollisionTime )
		{
			Game::Actors::C_VehicleLogicActor* pOtherActor = mRaceLogicLayerPtr->GetLogicVehicleActor(i);

			Game::Actors::VehiclePerformanceModifiers mods;
			mods.Initialise();

			Maths::HVec3 repelVec = pActor->mPositionInfo.mWorldPosition - pOtherActor->mPositionInfo.mWorldPosition;
			Maths::C_Normalised repelNormal = repelVec.Normalised();

			mods.mForce.Set(true, repelNormal.Normal() * g_debugShieldRepelForce);

			Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pOtherActor->GetHandle(), i, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
			pActor->SendMessageToActor(pOtherActor->GetGraphicalActor(i)->GetHandle(), &perfModMsg);
		}

	}
}

void C_VehicleComponent_Fan_Power::Update()
{
	if(!mRaceStarted)
		return;

	mDeltaTime = C_GameApp::GetInstance()->GetTimeStep();

	RLPickupType* pPickup = (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_SHIELD);

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	vehActor->mShieldOverload -= (mDeltaTime * ((RLShieldBalancingData *)pPickup->mLevels[0]->mBalancingData)->mOverloadDegradeFactor);
	if(vehActor->mShieldOverload < 0.f)
	{
		vehActor->mShieldOverload=0.f;
	}

	if(vehActor->mCriticalMassTimer > 0.f)
	{
		vehActor->mCriticalMassTimer-=mDeltaTime;

		if(vehActor->mCriticalMassTimer < 0.5f)
		{
			for(U32 a=0;a<levelState.mNumCars;a++)
			{
				if(a == mVehicleIndex)
					continue;

				UI::C_UiPlayerInRangeWarningMessage msg( a, mVehicleIndex );
				_UI_MESSAGE_FULL( msg );
			}
		}

		if(vehActor->mCriticalMassTimer < 0.f)
		{
			vehActor->mCriticalMassTimer=0.f;
			vehActor->mGoneCriticalMass=true;	//set this to true, the RaceLogicLayer sorts everything out and resets it back
		}
	}

	if(mDisableShield > 0.f)
	{
		mDisableShield-=mDeltaTime;
		if(mDisableShield < 0.f)
		{
			mDisableShield=0.f;
		}
	}

	if(mUsingShieldTimer)
	{
		//[mtaylor] -- drop a perk in split screen for Craig's new shield system (21 July 09)
		if( (Game::C_Game::GetInstance()->GetLevelState().mShieldsTiedToPickups || g_ForceShieldsTiedToPickups)
			&& mRaceLogicLayerPtr->mVehicleShielded[mVehicleIndex] == 0.f ) //i.e. the moment when the shield is used
		{
			//drop the pickup on track
			if(vehActor->mPickupInfo.mPickupStoreIndex != -1)
			{
				if(Game::VehicleIsLocal(mVehicleIndex))
				{
					E_PICKUP_TYPE type = vehActor->mPickupInfo.mPickupStore[vehActor->mPickupInfo.mPickupStoreIndex].mType;
					C_VehicleLogicDropPickupOnTrackMsg dropPickupMsg( type , mVehicleIndex );
					vehActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &dropPickupMsg);

					//remove it from the inventory of the player
					//Game::Actors::PickupRemovePowerMsg removePickupMsg(mVehicleIndex, 1);
					//vehActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removePickupMsg);#
				}
			}

			
		}

		mUsingShieldTimer -= mDeltaTime;
		mRaceLogicLayerPtr->mVehicleShielded[mVehicleIndex] += mDeltaTime;

		if(mUsingShieldTimer < 0.f)
		{
			mUsingShieldTimer=0.f;
			mRaceLogicLayerPtr->mVehicleShielded[mVehicleIndex]=0.f;

			AmaxDataLogging::LogShieldUse(mVehicleIndex, false);

			C_VehicleLogicMsg_PerkEnded finishedMsg(mVehicleIndex,0,E_PICKUP_TYPE_SHIELD);
			vehActor->GetActorList()->BroadcastMessage(&finishedMsg);
		}
	}

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	//need to do an AI version of this for Steve?
	if(levelState.mCars[mVehicleIndex].mHuman && levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER
		&& pRLC && !pRLC->IsPaused())
	{
		int controller=levelState.mCars[mVehicleIndex].mController;

		if(levelState.mCars[mVehicleIndex].mUpgrades.Shield())
		{
			bool do_shield= Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_UseShield);

			if (do_shield && Input::GetGameInput(controller)->GetButtonValue(Framework::E_Button_Debug_ButtonBack) > 0.0f)
				do_shield = false;

			Game::C_MultiplayerVehicleAbility* shieldingEfficiencyAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency);
			float shieldLengthMultiplierUsed = 1.0f;

			if(do_shield && mUsingShieldTimer == 0.f && mDisableShield == 0.f)
			{
				Game::Actors::C_VehicleLogicActor *veh_actor =(Game::Actors::C_VehicleLogicActor *)GetActor();
				if(veh_actor->mFanPower)
				{
					veh_actor->mFanPower-=1.f;
					//Multiply by shield length multiplier
					float shield_time=((RLShieldBalancingData *)pPickup->mLevels[0]->mBalancingData)->mTime ;
					if(shieldingEfficiencyAbility->GetActive())
					{
						shieldLengthMultiplierUsed  = shieldingEfficiencyAbility->GetValue();;
						shield_time *= shieldLengthMultiplierUsed;
					}
					mUsingShieldTimer+=shield_time;

					Maths::HMat43 mat;
					mat.SetIdentity();
					C_VehicleLogicMsg_FiredPerkMsg fired_msg(E_PICKUP_TYPE_SHIELD, 0, veh_actor->GetHandle(), mVehicleIndex, 0, 
						mat, 1 * shieldLengthMultiplierUsed , false, false, 
						1.0 / shieldLengthMultiplierUsed ); 
					/*THIS SHOULDN'T BE BROADCAST*/veh_actor->GetActorList()->BroadcastMessage(&fired_msg);
					Game::C_InGameSpecialFXManager::Instance().MessageHandler(&fired_msg);

					AmaxDataLogging::LogShieldUse(mVehicleIndex, true);

					_UI_MESSAGE_FULL( fired_msg );

					RepelNearbyVehicles();

					if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
					{
						const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
						netGameVars.m_EventManager->Add_Shielded(levelState.mCars[mVehicleIndex].mNetJoinId, (U8)shield_time);
					}
				}
			}
		}
	}
}

// Message Handlers
bool C_VehicleComponent_Fan_Power::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	PERF_EVENT("C_VehicleComponent.Fan_Power.MessageHandler");

	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	Game::C_LevelState& levelState				= Game::C_Game::GetInstance()->GetLevelState();

	switch(pMessage->Type())
	{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
		//		Update();

		//		return true;
		//	}
		//	break;

		case E_VehicleLogicMsg_FansAwarded:
			{
				const C_VehicleLogicMsg_FansAwarded* msg = Messaging::MessageCast<C_VehicleLogicMsg_FansAwarded>(pMessage);
				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

				if(msg->vehicleIndex == mVehicleIndex && mFansRequired > 0 && levelState.mCars[mVehicleIndex].mUpgrades.Shield() && 
					!levelState.mShieldsTiedToPickups && !g_ForceShieldsTiedToPickups)
				{
					mCurrentFans += msg->numFans;

					int shieldToAdd = 0;
					while( mCurrentFans >= mFansRequired )
					{
						shieldToAdd += levelState.mCars[mVehicleIndex].mShieldsEarnedFans;
						mCurrentFans -= mFansRequired;
					}

					vehActor->mFanPower += shieldToAdd;
					vehActor->mFanPower = Maths::Clamp(vehActor->mFanPower,0.f,(float)levelState.mCars[mVehicleIndex].mUpgrades.NumShieldSlots());
				}

				return true;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

				//[mtaylor] - disabled for Craig's new shield system (21 July 09)
				if(msg->mFiringVehicleIndex == mVehicleIndex && levelState.mCars[mVehicleIndex].mUpgrades.Shield() && 
					!levelState.mShieldsTiedToPickups && !g_ForceShieldsTiedToPickups)
				{
					vehActor->mFanPower += levelState.mCars[mVehicleIndex].mShieldsEarnedHitByPerk;
					vehActor->mFanPower = Maths::Clamp(vehActor->mFanPower,0.f,(float)levelState.mCars[mVehicleIndex].mUpgrades.NumShieldSlots());
				}

				return true;
			}
			break;

		case E_VehicleLogicMsg_Repair:
			{
				const C_VehicleLogicRepairMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);
				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

				//[mtaylor] - only do this if shields not tied to pickups (new test shield system)
				if(levelState.mCars[mVehicleIndex].mUpgrades.Shield() && !levelState.mShieldsTiedToPickups && !g_ForceShieldsTiedToPickups)
				{
					float shieldBoost	= levelState.mCars[mVehicleIndex].mUpgrades.RepairBoostLevel3() ? 2.0f : 0.f;
					vehActor->mFanPower	+= (levelState.mCars[mVehicleIndex].mShieldsEarnedRepair + shieldBoost);
					vehActor->mFanPower	= Maths::Clamp(vehActor->mFanPower,0.f,(float)levelState.mCars[mVehicleIndex].mUpgrades.NumShieldSlots());
				}

				return true;
			}
			break;

		case E_VehicleLogicMsg_ShieldedPerk:
			{
				//const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
				//Game::Actors::C_VehicleLogicActor* vehActor		= ((Game::Actors::C_VehicleLogicActor*)GetActor());
				//RLPickupType* pPickup = (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_SHIELD);

				//if( pMsg->mDefendingVehicleIndex == mVehicleIndex && vehActor->mCriticalMassTimer == 0.f)
				//{
				//	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
				//	RaceBook::C_RaceBookProfile *rb_profile=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[mVehicleIndex].mCharacterID);

				//	if(rb_profile && rb_profile->GetShopUpgradeValue(E_RACEBOOK_SHOP_UPGRADES_SHIELD_OVERDRIVE) > 0)
				//	{
				//		float overload_amount=((RLShieldBalancingData *)pPickup->mLevels[0]->mBalancingData)->mOverloadAmountPerShield;

				//		if(rb_profile->GetShopUpgradeValue(E_RACEBOOK_SHOP_UPGRADES_SHIELD_OVERDRIVE_LEVEL2) > 0)
				//		{
				//			overload_amount*=1.2f;
				//		}

				//		vehActor->mShieldOverload+=overload_amount;

				//		if(vehActor->mShieldOverload > 1.0f)
				//		{
				//			vehActor->mShieldOverload=1.0f;
				//			
				//			float cm_time=g_CriticalMassRandom.FRand(((RLShieldBalancingData *)pPickup->mLevels[0]->mBalancingData)->mCriticalMassMinTime,
				//														((RLShieldBalancingData *)pPickup->mLevels[0]->mBalancingData)->mCriticalMassMaxTime);
				//			vehActor->mCriticalMassTimer=cm_time;
				//		}
				//	}
				//}
		
				return true;
			}
			break;

		//case E_VehicleLogicMsg_CriticalMass:
		//	{
		//		const C_VehicleLogicMsg_CriticalMass* msg = Messaging::MessageCast<C_VehicleLogicMsg_CriticalMass>(pMessage);
		//						
		//		if(mVehicleIndex == msg->mVehicleIndex && mRaceLogicLayerPtr->mVehicleShielded[mVehicleIndex] == 0.f)
		//		{
		//			mDisableShield=msg->mCriticalMassTime;
		//		}
		//	}
		//	break;

		case E_VehicleLogicMsg_EnableDisablePickupSystem:
			{
				const C_VehicleLogicMsg_EnableDisablePickupSystem* msg = Messaging::MessageCast<C_VehicleLogicMsg_EnableDisablePickupSystem>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					mDisableShield=msg->mDisableShieldTime;
				}

				return true;
			}
			break;

		case E_VehicleLogicMsg_ShieldTime:
			{
				const C_VehicleLogicShieldTimeMsg* msg = Messaging::MessageCast<C_VehicleLogicShieldTimeMsg>(pMessage);
				if(msg->m_VehicleIndex == mVehicleIndex && msg->m_ShieldTime > 0)
				{
					Game::Actors::C_VehicleLogicActor *veh_actor =(Game::Actors::C_VehicleLogicActor *)GetActor();
					Maths::HMat43 mat;
					mat.SetIdentity();
					C_VehicleLogicMsg_FiredPerkMsg fired_msg(E_PICKUP_TYPE_SHIELD, 0, veh_actor->GetHandle(), mVehicleIndex, 0, mat, msg->m_ShieldTime - 1.0f, false, false, 2.0f / msg->m_ShieldTime);
					/*THIS SHOULDN'T BE BROADCAST*/veh_actor->GetActorList()->BroadcastMessage(&fired_msg);
					Game::C_InGameSpecialFXManager::Instance().MessageHandler(&fired_msg);

					_UI_MESSAGE_FULL( fired_msg );

					RepelNearbyVehicles();

					if(msg->m_UseFanPower)
						veh_actor->mFanPower -= 1.0f;

					Game::C_LevelStateMultiplayerVehicleAbilities* mpVehAbilities = levelState.mCars[msg->m_VehicleIndex].mUpgrades.GetMultiplayerAbilities();
					Game::C_MultiplayerVehicleAbility* shieldingEfficiencyAbility = mpVehAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency);
					float shieldLengthMultiplierUsed = 1.0f;
					if(shieldingEfficiencyAbility ->GetActive())
					{
						shieldLengthMultiplierUsed = shieldingEfficiencyAbility->GetValue(); 
					}

					mUsingShieldTimer += msg->m_ShieldTime * shieldLengthMultiplierUsed;
				}
				return true;
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				mRaceStarted=true;
				return true;
			}

		case Game::E_GameflowMessage_RestartRace:		//	Mass fall-through
		case Game::E_GameflowMessage_QuitAndWinRace:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_RaceFinished:
		case Game::E_GameflowMessage_VehicleFinished:
			{
				mRaceStarted=false;
				return true;
			}

	}

	return I_GameActorComponent::OnMessage(pMessage);
}

} // namespace Components
