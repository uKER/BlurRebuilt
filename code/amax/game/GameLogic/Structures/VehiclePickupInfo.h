#ifndef VEHICLE_PICKUP_INFO_H
#define VEHICLE_PICKUP_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <actor_system/ActorSys.h>

typedef struct PickupStore
{
	E_PICKUP_TYPE	mType;
	char			mDroppedByIndex;
	int				mNumShots;
	bool			mInfiniteAmmo;
	int				mPickupGroupId;
	float			mDistancePickedUp;
}PickupStore;

struct VehiclePickupInfo
{
	bool						mCharging;
	bool						mUsingNitrousReverse;
	
	E_PICKUP_TYPE				mCurrentlySelectedType;
	E_PICKUP_TYPE				mCurrentlyUsingType;
	int							mCurrentlyUsingLevel;

	Btl::Vector<PickupStore>	mPickupStore;
	int							mPickupStoreIndex;

	float						mTimeSinceTouchedLastPickup;

	int							mCurrentTargetIdx;

	Btl::Set<Actor::C_ActorHandle> mNumFanPickupsCollected;				// Used for fan demand achievements (not used for fan demand logic)
	Btl::Set<Actor::C_ActorHandle> mNumFanPickupsCompleted;				// Store any that have been completed and therefore hidden this race
};

#endif //VEHICLE_PICKUP_INFO_H
