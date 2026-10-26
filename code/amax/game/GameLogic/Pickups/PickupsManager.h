#ifndef PICKUPS_MANAGER_H
#define PICKUPS_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <misc_shared/Messaging/Messaging.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\Pickups\ShockManager.h>
#include <System/scheduler.h>

namespace Game
{
	class C_PickupBase;
	class C_NetEventManager;
	class C_RaceLogicLayer;
	struct PickupAutopsyReport;
	class C_LevelState;

	struct BufferedPickup
	{
		C_PickupBase*	mpPickup;
		float			mDelay;
	};

	struct PickupCreationStruct
	{
		PickupCreationStruct(E_PICKUP_TYPE pickupType,  E_PICKUP_SUB_TYPE subType, int firingIdx) 
			: mPickupType(pickupType), mPickupSubType(subType), mFiringIdx(firingIdx), mTargetIdx(-1), mDelay(0.f), mPickupId(0), mParentPickupId(0), mGroupPickupId(0),
			mDisableVehicleFX(false), mNoPhysicalEffect(false), mOverrideMatrix(false), mVal(1.f), mVal2(1.f), mVal3(0), mVal4(-1),mHostIsController(false) {};

		E_PICKUP_TYPE		mPickupType;
		E_PICKUP_SUB_TYPE	mPickupSubType;
		int					mFiringIdx;
		int					mTargetIdx;
		float				mDelay;
		U32					mPickupId;
		U32					mGroupPickupId;
		U32					mParentPickupId;
		bool				mDisableVehicleFX;
		bool				mNoPhysicalEffect;
		bool				mOverrideMatrix;
		bool				mHostIsController;
		Maths::HMat43		mMatrix;

		// General pickup-specific values
		float				mVal; 
		float				mVal2;
		U32					mVal3;
		int					mVal4;

	};

	typedef Btl::Map<U32, C_PickupBase*>	ActivePickupList;
	typedef Btl::Map<U32, BufferedPickup>	BufferedPickupList;

	struct PickupPools;

	class ALIGN16 C_PickupsManager : public Messaging::C_MessageHandler
	{
	public:

		C_PickupsManager() : mSchedulerGroupHandle(Scheduler::JQ_GROUP_NONE) {};

		void			Init(C_RaceLogicLayer* pLayer, const C_LevelState* pLevelState);
		void			PreUpdateOne();
		void			PreUpdateTwo();
		void			Update();
		void			Clear();

		void			CapNumMinesOnTrack(E_PICKUP_SUB_TYPE subType, int mineCap);

		void			KillAllPickups();
		C_PickupBase*	GetPickup(U32 id);
		const C_PickupBase* GetPickupByIndex(U32 index) const;

		ActivePickupList& GetActivePickupList() { return mPickupList; };

		void			ReplacePickup(U32 oldId, U32 newId, C_PickupBase* pPickup);
		U32				CreatePickup(PickupCreationStruct pickupInfo);
		void			RemoveShock(U32 id) { mShockManager.RemoveShock(id); };

		void			NinjaKillPickup(U32 id, const Game::PickupAutopsyReport& autopsyReport);
		U32				AllocateUniqueId(int vehIdx);
		void			IncrementUniqueId(int incNum) { mUniqueId += (incNum + 1); }; // +1 for safety

		C_ShockManager&	GetShockManager() { return mShockManager; };

		void			CheckVehicleTargetted(int vehIdx);

		static C_PickupsManager& Instance(){return s_instance;};

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		void PhotoMovePickups(F32 frac);

		void HandleScripts();

		Scheduler::GroupHandle& GetPickupSchedulerGroupHandle() { return mSchedulerGroupHandle; };

	private:
		static C_PickupsManager s_instance;

		void ProcessPickupDeathAudio(const C_PickupBase* pPickup, const Game::PickupAutopsyReport& autopsyReport); // Used to ensure that if this kit is told to ninja kill a pickup, it plays the correct audio
		const bool IsMasterMineEquipped() const;

		ActivePickupList	mPickupList;
		BufferedPickupList	mBufferedList;

		U32					mUniqueId; 
		C_ShockManager		mShockManager;

		// Cache
		C_RaceLogicLayer*	mpRaceLogicLayer;
		C_LevelState*		mpLevelState;

		bool				mIsNetwork;
		C_NetEventManager*	mpEventManager;

		void				AssessMineNinjaKillData( C_PickupBase* pPickup, const Game::PickupAutopsyReport& autopsyReport);

		Scheduler::GroupHandle	mSchedulerGroupHandle;

		void				Delete(C_PickupBase* pPickup);
		PickupPools*		mpPickupPools;

		
	};

} // namespace Game

#endif // PICKUPS_MANAGER_H
