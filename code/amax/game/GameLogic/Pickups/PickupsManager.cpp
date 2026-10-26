#include "stdafx.h"

#include <GameLogic/Pickups/PickupsManager.h>
#include <GameLogic\Pickups\Normal\PickupShunt.h>
#include <GameLogic\Pickups\Normal\PickupBlindfire.h>
#include <GameLogic\Pickups\Normal\PickupBarge.h>
#include <GameLogic\Pickups\Normal\PickupShock.h>
#include <GameLogic/Pickups/Normal/PickupNitrous.h>
#include <GameLogic/Pickups/Normal/PickupShield.h>
#include <GameLogic/Pickups/Normal/PickupSuperShield.h>

#include <GameLogic/Pickups/Upgraded/PickupBomb.h>

#include <GameLogic\Pickups\Double\PickupDoubleShock.h>
#include <GameLogic\Pickups\Double\PickupDoubleBomb.h>
#include <GameLogic\Pickups\Double\PickupDoubleShunt.h>
#include <GameLogic\Pickups\Double\PickupDoubleNitrous.h>
#include <GameLogic\Pickups\Double\PickupDoubleShield.h>

#include <GameLogic\Pickups\Reverse\PickupBlindfireReverse.h>
#include <GameLogic\Pickups\Reverse\PickupBombReverse.h>
#include <GameLogic\Pickups\Reverse\PickupShuntReverse.h>
#include <GameLogic\Pickups\Reverse\PickupNitrousReverse.h>

#include <GameLogic\Pickups\DoubleReverse\PickupDoubleShuntReverse.h>

#include <GameLogic\Pickups\Mods\PickupBlindfireShotgun.h>
#include <GameLogic\Pickups\Mods\PickupBlindfireShotgunReverse.h>
#include <GameLogic\Pickups\Mods\PickupMasterMineBomb.h>
#include <GameLogic\Pickups\Misc\PickupMiniBomb.h>
#include <GameLogic\Pickups\PickupClasses.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\StickerManager.h>

#include <Game/Game.h>
#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Structures/LevelState.h>
#include <GameAudio/GameAudioLayer.h>
#include <GameAudio/PickupSubsystem.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

Debug::Int g_debugMineNumberCap("Perks.NumberCap.Mine", 30);
Debug::Int g_debugMiniMineNumberCap("Perks.NumberCap.MiniMine", 50);
Debug::Int g_debugShockNumberCap("Perks.NumberCap.Shock", 12);

Debug::Bool g_disableNetworkKillPickupAudio("Multiplayer.DisableNetworkKillPickupAudio", false);

extern Debug::Bool g_ForceAllShieldsToBeSuper;

namespace Game
{
	struct PickupPools
	{	
		Btl::DynamicPool<C_PickupShuntProjectile>				shuntProjectile;
		Btl::DynamicPool<C_PickupShuntProjectileReverse>		shuntProjectileReverse;
		Btl::DynamicPool<C_PickupDoubleShunt>					doubleShunt;
		Btl::DynamicPool<C_PickupDoubleShuntReverse>			doubleShuntReverse;
		Btl::DynamicPool<C_PickupNitrousReverse>				nitrousReverse;
		Btl::DynamicPool<C_PickupDoubleNitrous>					doubleNitrous;
		Btl::DynamicPool<C_PickupBlindfireProjectile>			blindfireProjectile;
		Btl::DynamicPool<C_PickupBlindfireProjectileReverse>	blindfireProjectileReverse;
		Btl::DynamicPool<C_PickupBlindfireShotgun>				blindfireShotgun;
		Btl::DynamicPool<C_PickupBlindfireShotgunReverse>		blindfireShotgunReverse;
		Btl::DynamicPool<C_PickupBarge>							barge;
		Btl::DynamicPool<C_PickupShock>							shock;
		Btl::DynamicPool<C_PickupBomb>							bomb;
		Btl::DynamicPool<C_PickupDoubleBomb>					doubleBomb;
		Btl::DynamicPool<C_PickupBombReverse>					bombReverse;
		Btl::DynamicPool<C_PickupMasterMineBomb>				masterMineBomb;
		Btl::DynamicPool<C_PickupMiniBomb>						miniBomb;
		Btl::DynamicPool<C_PickupShield>						shield;
		Btl::DynamicPool<C_PickupSuperShield>					superShield;
	};

	C_PickupsManager C_PickupsManager::s_instance;

	void C_PickupsManager::Init(C_RaceLogicLayer* pLayer, const C_LevelState* pLevelState)
	{
		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
		AddSubscriber(GameMessaging::E_GameMsg_Update);

		mUniqueId = 1;	//	[GavinY] - We use 0 as our default parameter value so start at 1 so as not to confuse things
		Clear();

		mpRaceLogicLayer	= pLayer;
		mpLevelState		= (C_LevelState*)pLevelState;
		mIsNetwork			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork();
		mpEventManager		= C_Game::GetInstance()->GetNetGameVars().m_EventManager;

		BIZ_ASSERT(mpRaceLogicLayer);
		BIZ_ASSERT(mpEventManager);

		mShockManager.Init(pLayer);

		mSchedulerGroupHandle = Scheduler::CreateGroup();
		mpPickupPools = new PickupPools;
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::Clear()
	{
		KillAllPickups();

#ifndef CFG_GOLD
		DebugNitrousTestBedReset();
#endif
		if (mSchedulerGroupHandle != Scheduler::JQ_GROUP_NONE)
		{
			Scheduler::DestroyGroup(mSchedulerGroupHandle);
			mSchedulerGroupHandle = Scheduler::JQ_GROUP_NONE;
		}
		SAFE_DELETE_PTR(mpPickupPools);
	}

	void C_PickupsManager::KillAllPickups()
	{
		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); it++ )
		{
			//SAFE_DELETE_PTR(it->GetValue());
			Delete(it->GetValue());
		}
		mPickupList.Clear();

		for( BufferedPickupList::Iterator it = mBufferedList.Begin(); it != mBufferedList.End(); it++ )
		{
			//SAFE_DELETE_PTR(it->GetValue().mpPickup);
			Delete(it->GetValue().mpPickup);
		}
		mBufferedList.Clear();

		mShockManager.ClearShocks();
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::PreUpdateOne()
	{
		PERF_EVENT("PickupsManager.PreUpdate");

		if (mSchedulerGroupHandle == Scheduler::JQ_GROUP_NONE)
			return;

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		for( BufferedPickupList::Iterator it = mBufferedList.Begin(); it != mBufferedList.End(); )
		{
			BufferedPickup& buffer = it->GetValue();

			buffer.mDelay -= frameTime;

			if(buffer.mDelay <= 0.f)
			{
				if(buffer.mpPickup)
				{
					int firingIdx						= buffer.mpPickup->GetFiringIdx();
					Actors::C_VehicleLogicActor* pActor = mpRaceLogicLayer->GetLogicVehicleActor(firingIdx);

					if(buffer.mpPickup->IsMatrixSet())
						buffer.mpPickup->PostDelayInitialise(buffer.mpPickup->GetMatrix());
					else
						buffer.mpPickup->PostDelayInitialise(pActor->GetWorldMatrix());
					
					mPickupList.Insert(it->GetKey(), buffer.mpPickup);
				}
				mBufferedList.Erase( it++ );
			}
			else
				++it;
		}

		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); it++ )
		{
			it->GetValue()->PreUpdateOne();
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::PreUpdateTwo()
	{
		if (mSchedulerGroupHandle == Scheduler::JQ_GROUP_NONE)
			return;

		Scheduler::Flush(mSchedulerGroupHandle);

		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); it++ )
		{
			it->GetValue()->PreUpdateTwo();
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::Update()
	{
		PERF_EVENT("PickupsManager.Update");

		if (mSchedulerGroupHandle == Scheduler::JQ_GROUP_NONE)
			return;

		Scheduler::Flush(mSchedulerGroupHandle);

		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); )
		{
			bool remove = it->GetValue()->Update();

			if( remove )
			{
				if(!it->GetValue()->GetNinjaKill() && mIsNetwork)
				{
					E_PICKUP_TYPE type = it->GetValue()->GetType();
					if(type == E_PICKUP_TYPE_SHUNT ||
						type == E_PICKUP_TYPE_BLINDFIRE ||
						type == E_PICKUP_TYPE_BOMB ||
						type == E_PICKUP_TYPE_SHIELD ||
						type == E_PICKUP_TYPE_SHOCK ||
						type == E_PICKUP_TYPE_SUPER_SHIELD)
					{
						// Lets check out why this guy died, and let everyone else know. 
						mpEventManager->Add_PickupKilled(it->GetValue()->GetUid(),it->GetValue()->GetAutopsyReport());
					}
				}
				//SAFE_DELETE_PTR(it->GetValue());
				Delete(it->GetValue());
				mPickupList.Erase( it++ );
			}
			else
				++it;
		}

#ifndef CFG_GOLD
		DebugNitrousTestBedUpdate();
		HandleScripts();
#endif

		if(HostIsLocal() || !mIsNetwork)
		{
			mShockManager.Update();
		}
	}

	void C_PickupsManager::PhotoMovePickups(F32 frac)
	{
		for (ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); ++it)
		{
			it->GetValue()->PhotoMove(frac);
		}
	}
	
	// --------------------------------------------------------------------------------

	C_PickupBase* C_PickupsManager::GetPickup(U32 id)
	{
		ActivePickupList::Iterator activeIt = mPickupList.Find(id);
		if(activeIt != mPickupList.End())
			return activeIt->GetValue();

		BufferedPickupList::Iterator buffIt = mBufferedList.Find(id);
		if(buffIt != mBufferedList.End())
			return buffIt->GetValue().mpPickup;

		return NULL;
	}

	const C_PickupBase* C_PickupsManager::GetPickupByIndex(U32 index) const
	{
		S32 i = 0;
		for( ActivePickupList::ConstIterator it = mPickupList.Begin(); it != mPickupList.End(); ++it, i++)
		{
			if (i == index)
				return it->GetValue();
		}
		return 0;
	}

	// --------------------------------------------------------------------------------

	U32 C_PickupsManager::CreatePickup(PickupCreationStruct pickupInfo)
	{
		PERF_EVENT("PickupsManager.CreatePickup");
		C_PickupBase* pPickup = NULL;

		if(!pickupInfo.mPickupId)
			pickupInfo.mPickupId = AllocateUniqueId(pickupInfo.mFiringIdx);

		//DEBUG DEBUG DEBUG
		if(pickupInfo.mPickupType == E_PICKUP_TYPE_SHIELD && g_ForceAllShieldsToBeSuper)
		{
			pickupInfo.mPickupType = E_PICKUP_TYPE_SUPER_SHIELD;
			pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_NORMAL;
		}
		//DEBUG DEBUG DEBUG

		const bool masterMineEquiped = IsMasterMineEquipped();
		bool sendToOtherKits = true;

		switch( pickupInfo.mPickupType )
		{
		case E_PICKUP_TYPE_SHUNT:
			{
				switch(pickupInfo.mPickupSubType)
				{
				case E_PICKUP_SUB_TYPE_NORMAL:
					pPickup = mpPickupPools->shuntProjectile.AllocateNoConstructor();
					new (pPickup) C_PickupShuntProjectile(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, pickupInfo.mPickupSubType, 
															pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED:
					pPickup = mpPickupPools->shuntProjectileReverse.AllocateNoConstructor();
					new (pPickup) C_PickupShuntProjectileReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
					pPickup = mpPickupPools->doubleShunt.AllocateNoConstructor();
					new (pPickup) C_PickupDoubleShunt(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
					pPickup = mpPickupPools->doubleShuntReverse.AllocateNoConstructor();
					new (pPickup) C_PickupDoubleShuntReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				default:
					pPickup = mpPickupPools->shuntProjectile.AllocateNoConstructor();
					new (pPickup) C_PickupShuntProjectile(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, E_PICKUP_SUB_TYPE_NORMAL, 
															pickupInfo.mDisableVehicleFX);
					break;
				}
				break;
			}

		case E_PICKUP_TYPE_NITROUS:
			{
				switch(pickupInfo.mPickupSubType)
				{
				case E_PICKUP_SUB_TYPE_REVERSED:
					pPickup = mpPickupPools->nitrousReverse.AllocateNoConstructor();
					new (pPickup) C_PickupNitrousReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
				case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
					pPickup = mpPickupPools->doubleNitrous.AllocateNoConstructor();
					new (pPickup) C_PickupDoubleNitrous(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;
				}
				break;
			}

		case E_PICKUP_TYPE_BLINDFIRE:
			{
				switch(pickupInfo.mPickupSubType)
				{
				case E_PICKUP_SUB_TYPE_NORMAL:
					pPickup = mpPickupPools->blindfireProjectile.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireProjectile(E_PickupClass_BlindfireProjectile, pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mGroupPickupId);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED:
					pPickup = mpPickupPools->blindfireProjectileReverse.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireProjectileReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mGroupPickupId);
					break;

				case E_PICKUP_SUB_TYPE_MOD:
					pPickup = mpPickupPools->blindfireShotgun.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireShotgun(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_MOD_REVERSED:
					pPickup = mpPickupPools->blindfireShotgunReverse.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireShotgunReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_SUB_MOD:
				case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
					pPickup = mpPickupPools->blindfireProjectile.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireProjectile(E_PickupClass_BlindfireProjectile, pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mGroupPickupId);
					break;

				default:
					pPickup = mpPickupPools->blindfireProjectile.AllocateNoConstructor();
					new (pPickup) C_PickupBlindfireProjectile(E_PickupClass_BlindfireProjectile, pickupInfo.mPickupId, pickupInfo.mFiringIdx, E_PICKUP_SUB_TYPE_NORMAL, pickupInfo.mDisableVehicleFX, pickupInfo.mGroupPickupId);
					break;
				}
				break;
			}

		case E_PICKUP_TYPE_BARGE:
			{
				switch(pickupInfo.mPickupSubType)
				{
					case E_PICKUP_SUB_TYPE_MOD:
						{
							pPickup = mpPickupPools->barge.AllocateNoConstructor();
							new (pPickup) C_PickupBarge(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
							break;
						}
					default:
						{
							pPickup = mpPickupPools->barge.AllocateNoConstructor();
							new (pPickup) C_PickupBarge(pickupInfo.mPickupId, pickupInfo.mFiringIdx, E_PICKUP_SUB_TYPE_NORMAL, pickupInfo.mDisableVehicleFX);
							break;
						}
				}
				break;
			}

		case E_PICKUP_TYPE_SHOCK:
			{
				switch(pickupInfo.mPickupSubType)
				{
				case E_PICKUP_SUB_TYPE_NORMAL:
					{
						pPickup = mpPickupPools->shock.AllocateNoConstructor();
						new (pPickup) C_PickupShock(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, pickupInfo.mPickupSubType, 
							pickupInfo.mDisableVehicleFX, pickupInfo.mVal, pickupInfo.mVal2, pickupInfo.mVal4);
						break;
					}

				case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
				case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
					{
						if(mIsNetwork)
						{
							if(HostIsLocal())
								mShockManager.RegisterShocks(pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, pickupInfo.mPickupId);

							// Send even if I'm the host so everyone plays the FX
							U8 firerId = GetJoinIdFromLocalIndex(pickupInfo.mFiringIdx);
							U8 targetId = GetJoinIdFromLocalIndex(pickupInfo.mTargetIdx);
							mpEventManager->Add_RegisterShocksWithHost(firerId, targetId, pickupInfo.mPickupId);
						}
						else
							mShockManager.RegisterShocks(pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, pickupInfo.mPickupId);

						C_PickupsManager::Instance().GetShockManager().PlayFX(pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, pickupInfo.mPickupId);

						break;
					}

				default:
					pPickup = mpPickupPools->shock.AllocateNoConstructor();
					new (pPickup) C_PickupShock(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mTargetIdx, E_PICKUP_SUB_TYPE_NORMAL, pickupInfo.mDisableVehicleFX);
					break;
				}				
				break;
			}

		case E_PICKUP_TYPE_BOMB:
			{
				switch(pickupInfo.mPickupSubType)
				{
				case E_PICKUP_SUB_TYPE_NORMAL:
					CapNumMinesOnTrack(pickupInfo.mPickupSubType, g_debugMineNumberCap);
					// Fall through
				case E_PICKUP_SUB_TYPE_DECOY:
					pPickup = mpPickupPools->bomb.AllocateNoConstructor();
					new (pPickup) C_PickupBomb(E_PickupClass_Bomb, pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mVal2, masterMineEquiped);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
				case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
					pPickup = mpPickupPools->doubleBomb.AllocateNoConstructor();
					new (pPickup) C_PickupDoubleBomb(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, true);
					break;

				case E_PICKUP_SUB_TYPE_REVERSED:
					pPickup = mpPickupPools->bombReverse.AllocateNoConstructor();
					new (pPickup) C_PickupBombReverse(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
					break;

				case E_PICKUP_SUB_TYPE_MOD:
					pPickup = mpPickupPools->masterMineBomb.AllocateNoConstructor();
					new (pPickup) C_PickupMasterMineBomb(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mVal3);
					break;

				case E_PICKUP_SUB_TYPE_MINI:
					CapNumMinesOnTrack(pickupInfo.mPickupSubType, g_debugMiniMineNumberCap);

					pPickup = mpPickupPools->miniBomb.AllocateNoConstructor();
					new (pPickup) C_PickupMiniBomb(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX, pickupInfo.mVal2, pickupInfo.mVal3, pickupInfo.mVal);
					sendToOtherKits = false;
					break;


				default:
					pPickup = mpPickupPools->bomb.AllocateNoConstructor();
					new (pPickup) C_PickupBomb(E_PickupClass_Bomb, pickupInfo.mPickupId, pickupInfo.mFiringIdx, E_PICKUP_SUB_TYPE_NORMAL, pickupInfo.mDisableVehicleFX, pickupInfo.mVal);
					break;
				}					
				break;
			}

		case E_PICKUP_TYPE_SHIELD:
			{
				pPickup = mpPickupPools->shield.AllocateNoConstructor();
				new (pPickup) C_PickupShield(pickupInfo.mPickupId, pickupInfo.mFiringIdx, pickupInfo.mPickupSubType, pickupInfo.mDisableVehicleFX);
				break;
			}

		case E_PICKUP_TYPE_SUPER_SHIELD:
			{
				pPickup = mpPickupPools->superShield.AllocateNoConstructor();
				new (pPickup)C_PickupSuperShield(pickupInfo.mPickupId, pickupInfo.mFiringIdx, E_PICKUP_SUB_TYPE_NORMAL, pickupInfo.mDisableVehicleFX);
				break;
			}

		default:
			BIZ_ASSERT_MSG(false, "Attempted to create a pickup of invalid type!");
			break;
		}

		if(pPickup)
		{
			pPickup->FireChargingSpecialFX();

			bool vehIsController = (pickupInfo.mHostIsController && HostIsLocal()) || (!pickupInfo.mHostIsController && VehicleIsLocal(pickupInfo.mFiringIdx));

			//	If we're playing a network game we need to let everyone else know about this power-up
			//	Recording of this net event MUST come before calling PostDelayInitialise( ) on the pickup, as this would result in the creation and initialisation events
			//	arriving in the wrong order on other kits.
			if(mIsNetwork && vehIsController && sendToOtherKits)
			{
				U8 firerId = GetJoinIdFromLocalIndex(pickupInfo.mFiringIdx);
				U8 targetId = GetJoinIdFromLocalIndex(pickupInfo.mTargetIdx);
				mpEventManager->Add_PickupFired(firerId, targetId, pickupInfo);
			}

#ifndef CFG_GOLD
			//	[GavinY] The replay system will have this.
			Game::C_ReplayDirector::Instance().AddPickupFired(pickupInfo);
#endif // #ifndef CFG_GOLD

			if(pickupInfo.mParentPickupId)
				pPickup->SetParentId(pickupInfo.mParentPickupId);

			if(pickupInfo.mOverrideMatrix)
			{
				pPickup->SetReplacement();
				pPickup->PreSetMatrix(pickupInfo.mMatrix);
			}

			// Always buffer the pickup, even with 0 delay, so the pre-update gets run
			BufferedPickup buffer;
			buffer.mDelay = pickupInfo.mDelay;
			buffer.mpPickup = pPickup;
			mBufferedList.Insert(pickupInfo.mPickupId, buffer);
		}

		return pickupInfo.mPickupId;
	}

	// -------------------------------------------------------------------------------

	void C_PickupsManager::ReplacePickup(U32 oldId, U32 newId, C_PickupBase* pPickup)
	{
		ActivePickupList::Iterator activeIt = mPickupList.Find(oldId);
		if(activeIt != mPickupList.End())
		{
			C_PickupBase* pOldPickup = activeIt->GetValue();
			Maths::HMat43 matrix = pOldPickup->GetMatrix();

			pOldPickup->KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_REPLACED);
			pOldPickup->SetToBeReplaced();
			
			pPickup->SetReplacement();

			if(pPickup->IsMatrixSet())
				pPickup->PostDelayInitialise(pPickup->GetMatrix());
			else
				pPickup->PostDelayInitialise(matrix);

			mPickupList.Insert(newId, pPickup);

			return;
		}

		BufferedPickupList::Iterator buffIt = mBufferedList.Find(oldId);
		if(buffIt != mBufferedList.End())
		{
			//SAFE_DELETE_PTR(buffIt->GetValue().mpPickup);
			Delete(buffIt->GetValue().mpPickup);
			buffIt->GetValue().mpPickup = pPickup;

			return;
		}
	}

	// -------------------------------------------------------------------------------

	U32 C_PickupsManager::AllocateUniqueId(int vehIdx)
	{
		// We need to use the NetJoinId because the local vehicle is always 0 in MP
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		U32 netId = (vehIdx != SYSTEM_FIRING_INDEX) ? levelState.mCars[vehIdx].mNetJoinId : 0;

		// First 8 bits define vehicle
		// Not having a unique id per vehicle is a tiny bit wasteful, but it's pretty negligible
		U32 vehicleUniqueId = (netId << 24) | mUniqueId;

		U32 maxIds = 0x0FFFFFFF;
		if(++mUniqueId >= maxIds)
			mUniqueId = 1;

		return vehicleUniqueId;
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::NinjaKillPickup( U32 id , const Game::PickupAutopsyReport& autopsyReport)
	{
		C_PickupBase* pPickup = GetPickup(id);
		if(pPickup)
		{
			if(!g_disableNetworkKillPickupAudio)
			{
				ProcessPickupDeathAudio(pPickup, autopsyReport); 
			}

			// Was this a mine that we fired?
			if( pPickup->GetType() == E_PICKUP_TYPE_BOMB  &&
				VehicleIsLocal(pPickup->GetFiringIdx()) )
			{
				AssessMineNinjaKillData(pPickup, autopsyReport);
			}

			pPickup->NinjaKill(autopsyReport);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::CapNumMinesOnTrack(E_PICKUP_SUB_TYPE subType, int mineCap)
	{
		C_PickupBase* pOldestPickup = NULL;
		int numMines = 0;

		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); it++ )
		{
			C_PickupBase* pPickup = it->GetValue();
			if(pPickup && pPickup->GetType() == E_PICKUP_TYPE_BOMB)
			{
				if(pPickup->GetSubType() == subType && !pPickup->IsSetToDie())
				{
					C_PickupBomb* pBomb = (C_PickupBomb*)pPickup;
					numMines++;

					if(!pOldestPickup || pPickup->GetCreationTime() < pOldestPickup->GetCreationTime() )
					{
						pOldestPickup = pPickup;
					}
				}
			}	
		}

		if(numMines >= mineCap)
		{
			pOldestPickup->KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::AssessMineNinjaKillData(C_PickupBase* pPickup, const Game::PickupAutopsyReport& autopsyReport)
	{
		// Lets do some fun-tastic forensic analysis. 
		// If somebody hit a mine we fired, and it was 'recently lobbed' we may get sticker progress..
		if( autopsyReport.mCauseOfDeath == E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE && 
			autopsyReport.mVehicleIdHit != pPickup->GetFiringIdx() &&
			((C_PickupBomb*)(pPickup))->GetHasBeenRecentlyLobbed() )
		{
			// If this mine has recently come to rest... tell the stickers about it!
			Game::C_Sticker* pSticker = Game::C_StickerManager::Instance().GetSticker(BLUR_STICKER_MINE_WITH_LOBBED);
			if(pSticker)
			{
				pSticker->AddProgress();
			}
		}
	}

	// --------------------------------------------------------------------------------
	void C_PickupsManager::ProcessPickupDeathAudio(const C_PickupBase* pPickup, const Game::PickupAutopsyReport& autopsyReport)
	{
		switch(autopsyReport.mCauseOfDeath)
		{
			case E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE:
			{
				// We are ninja killing a pickup that has hit us. Ensure hit sound correctly played. Needs fair bit of information. 
				C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pPickup->GetType(),0,autopsyReport.mVehicleIdHit,pPickup->GetFiringIdx(),pPickup->GetUid(), -1, autopsyReport.mTargettedVehicleId,-1,pPickup->GetSubType());
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, &hit_by_msg);
			}
			break;

			case E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT: 		// deliberate fall through
			case E_PICKUP_CAUSE_OF_DEATH_HIT_DYNAMIC_OBJECT:
			case E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT:
			{
				C_VehicleLogicMsg_PerkEnded pMsg(pPickup->GetFiringIdx(),pPickup->GetUid(),pPickup->GetType(),Game::E_BodyUserDataType_Unknown );
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Projectile_Hit_World, 0, &pMsg);
			}
			break;


			case E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP: 			// deliberate fall through
			case E_PICKUP_CAUSE_OF_DEATH_OLD_AGE:
			{
				C_VehicleLogicMsg_PerkEnded pMsg(pPickup->GetFiringIdx(),pPickup->GetUid(),pPickup->GetType(),Game::E_BodyUserDataType_Actor,pPickup->GetSubType());
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);	
			}
			break;

			case E_PICKUP_CAUSE_OF_DEATH_NONE:		  // deliberate fall through
			case E_PICKUP_CAUSE_OF_DEATH_UNKNOWN:
			case E_PICKUP_CAUSE_OF_DEATH_RESET:		
			case E_PICKUP_CAUSE_OF_DEATH_REPLACED:	
			{
				// We do nothing!
			}
			break;

			default:
			{
				// We do nothing!
			}
			break;
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::CheckVehicleTargetted(int vehIdx)
	{
		PERF_EVENT("PickupsManager.CheckVehicleTargetted");

		Actors::C_VehicleLogicActor* pActor = mpRaceLogicLayer->GetLogicVehicleActor(vehIdx);

		for( ActivePickupList::Iterator it = mPickupList.Begin(); it != mPickupList.End(); it++ )
		{
			C_PickupBase* pPickup = it->GetValue();
			if(pPickup)
			{
				pActor->mTargetLockedOn = pPickup->CheckVehicleTargetted(vehIdx);
				if(pActor->mTargetLockedOn)
					return;
			}	
		}

		// Buffered things don't cause targetting at present
		//for( BufferedPickupList::Iterator it = mBufferedList.Begin(); it != mBufferedList.End(); it++ )
		//{
		//	C_PickupBase* pPickup = it->GetValue().mpPickup;
		//	if(pPickup)
		//	{
		//		lockedOn = false;
		//		dirOverride = 0;

		//		bool targetted = pPickup->CheckVehicleTargetted(matrix, vehIdx);
		//		if(targetted)
		//			pActor->SetTargettedBits(matrix, lockedOn, dirOverride);
		//	}
		//}
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::HandleScripts()
	{
		PickupFiringScriptManager* pScriptManager = C_GameData::Instance().mPickupFiringScriptManager.Object();
		if(pScriptManager)
		{
			float frameTime = C_GameApp::GetInstance()->GetTimeStep();
			for( int i = 0; i < pScriptManager->mScripts.count; i++ )
			{
				PickupFiringScript* pScript = pScriptManager->mScripts[i].Object();

				if(pScript)
				{
					pScript->mCurrentTime += frameTime * 1000;

					if(pScript->mCurrentIdx >= 0 && pScript->mCurrentIdx < pScript->mNodes.count)
					{
						PickupFiringScriptNode& node = pScript->mNodes[pScript->mCurrentIdx];

						if(pScript->mCurrentTime >= node.mTime)
						{
							Actors::C_VehicleLogicActor* pLogicActor		= mpRaceLogicLayer->GetLogicVehicleActor(node.mVehIdx);

							if(pLogicActor)
							{
								pLogicActor->GetPickupInfo().mCurrentTargetIdx	= node.mTargetIdx;

								Actors::C_VehicleFirePickupMsg pickupFireMsg(node.mVehIdx, node.mPickupType);
								pickupFireMsg.mForwards = (node.mPickupType == E_PICKUP_TYPE_BOMB && node.mReverseFire);
								pickupFireMsg.mBackwards = (node.mPickupType != E_PICKUP_TYPE_BOMB && node.mReverseFire);
								pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pickupFireMsg);
							}

							pScript->mCurrentIdx++;
						}
					}
				}

			}
		}
	}

	// --------------------------------------------------------------------------------

	const bool C_PickupsManager::IsMasterMineEquipped() const
	{
		//I think it's safe to assume that 
		int vehicleIndex = 0; 

		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = mpLevelState->mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* pMasterMineAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Mastermine);

		return pMasterMineAbility->GetActive();
	}

	// --------------------------------------------------------------------------------

	bool C_PickupsManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
	{
		bool handledMessage = true;

		switch( pMessage->Type() )
		{

		case GameMessaging::E_GameMsg_Update:
			{
				Update();
				break;
			}

		default:
			{
				handledMessage = false;
				break;
			}
		}

		return handledMessage;
	}

	// --------------------------------------------------------------------------------

	void C_PickupsManager::Delete(C_PickupBase* pPickup)
	{
		switch(pPickup->GetClassType())
		{
			case E_PickupClass_ShuntProjectile:
				mpPickupPools->shuntProjectile.Free((C_PickupShuntProjectile*)pPickup);
				break;
			case E_PickupClass_ShuntProjectileReverse:
				mpPickupPools->shuntProjectileReverse.Free((C_PickupShuntProjectileReverse*)pPickup);
				break;
			case E_PickupClass_DoubleShunt:		
				mpPickupPools->doubleShunt.Free((C_PickupDoubleShunt*)pPickup);
				break;
			case E_PickupClass_DoubleShuntReverse:	
				mpPickupPools->doubleShuntReverse.Free((C_PickupDoubleShuntReverse*)pPickup);
				break;
			case E_PickupClass_NitrousReverse:
				mpPickupPools->nitrousReverse.Free((C_PickupNitrousReverse*)pPickup);
				break;
			case E_PickupClass_DoubleNitrous:	
				mpPickupPools->doubleNitrous.Free((C_PickupDoubleNitrous*)pPickup);
				break;
			case E_PickupClass_BlindfireProjectile:	
				mpPickupPools->blindfireProjectile.Free((C_PickupBlindfireProjectile*)pPickup);
				break;
			case E_PickupClass_BlindfireProjectileReverse:
				mpPickupPools->blindfireProjectileReverse.Free((C_PickupBlindfireProjectileReverse*)pPickup);
				break;
			case E_PickupClass_BlindfireShotgun:		
				mpPickupPools->blindfireShotgun.Free((C_PickupBlindfireShotgun*)pPickup);
				break;
			case E_PickupClass_BlindfireShotgunReverse:
				mpPickupPools->blindfireShotgunReverse.Free((C_PickupBlindfireShotgunReverse*)pPickup);
				break;
			case E_PickupClass_Barge:	
				mpPickupPools->barge.Free((C_PickupBarge*)pPickup);
				break;
			case E_PickupClass_Shock:
				mpPickupPools->shock.Free((C_PickupShock*)pPickup);
				break;
			case E_PickupClass_Bomb:							
				mpPickupPools->bomb.Free((C_PickupBomb*)pPickup);
				break;
			case E_PickupClass_DoubleBomb:	
				mpPickupPools->doubleBomb.Free((C_PickupDoubleBomb*)pPickup);
				break;
			case E_PickupClass_BombReverse:	
				mpPickupPools->bombReverse.Free((C_PickupBombReverse*)pPickup);
				break;
			case E_PickupClass_MasterMineBomb:
				mpPickupPools->masterMineBomb.Free((C_PickupMasterMineBomb*)pPickup);
				break;
			case E_PickupClass_MiniBomb:	
				mpPickupPools->miniBomb.Free((C_PickupMiniBomb*)pPickup);
				break;
			case E_PickupClass_Shield:	
				mpPickupPools->shield.Free((C_PickupShield*)pPickup);
				break;
			case E_PickupClass_SuperShield:
				mpPickupPools->superShield.Free((C_PickupSuperShield*)pPickup);
				break;
		default:
				SAFE_DELETE_PTR(pPickup);
				break;
		}
	}
}; // namespace Game

// --------------------------------------------------------------------------------

static void ActivatePickupScriptsFunc(void *)
{
	PickupFiringScriptManager* pScriptManager = C_GameData::Instance().mPickupFiringScriptManager.Object();
	if(pScriptManager)
	{
		for( int i = 0; i < pScriptManager->mScripts.count; i++ )
		{
			PickupFiringScript* pScript = pScriptManager->mScripts[i].Object();

			if(pScript)
			{
				pScript->mCurrentIdx = 0;
				pScript->mCurrentTime = 0.f;
			}

		}
	}
}
Debug::Function	g_debugActivatePickupScripts("Perks.Scripts.ActivatePickupScripts", ActivatePickupScriptsFunc, 0);
