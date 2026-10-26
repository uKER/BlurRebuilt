#include "stdafx.h"

#include <GameLogic/Pickups/PickupPositionManager.h>
#include "GameLogic/RaceLogicCore/Database/RLDatabase.h"
#include <GameLogic/Layers/RaceLogicLayer.h>

namespace Game
{
	void C_PickupPositionManager::Init(C_RaceLogicLayer* pLayer)
	{
		mpRaceLogicLayer = pLayer;
		Reset();
	}

	// ------------------------------------------------------------------------------

	void C_PickupPositionManager::Reset()
	{
		mPickupInfoList.Clear();
	}

	// ------------------------------------------------------------------------------

	U64 C_PickupPositionManager::GetMapId(U32 pickupId, XtId recordId)
	{
		U64 vehicleUniqueId = pickupId;
		vehicleUniqueId = (vehicleUniqueId << 32) | recordId;
		return vehicleUniqueId;
	}

	// ------------------------------------------------------------------------------

	E_PICKUP_TYPE C_PickupPositionManager::AddPickupActor(U32 pickupId, const RLPickupPositionRow* pickupRow)
	{
		E_PICKUP_TYPE assignedType = E_PICKUP_TYPE_NONE;

		if(pickupRow)
		{
			U64 mapId = GetMapId(pickupId, pickupRow->mID);

			Btl::Map<U64, PickupPositionInfo>::Iterator it = mPickupInfoList.Find(mapId);
			if(it != mPickupInfoList.End())
			{
				assignedType = it->GetValue().AssignPickup(mpRaceLogicLayer->GetRandomGenerator());	
			}
			else
			{
				PickupPositionInfo pickupPosInfo(pickupRow->mPickupFlagList.value);
				Btl::Map<U64, PickupPositionInfo>::InsertionReturnType insRet = mPickupInfoList.Insert(mapId, pickupPosInfo);

				if(insRet.inserted)
					assignedType = insRet.iterator->GetValue().AssignPickup(mpRaceLogicLayer->GetRandomGenerator());
			}	
		}

		return assignedType;
	}

	// ------------------------------------------------------------------------------

	void PickupPositionInfo::Init()
	{
		for( int i = 0; i < E_PICKUP_TYPE_FAN; i++ )
		{
			if( mPickupsAllowed & (1 << i) )
			{
				mPickupsAvailable.PushBack((E_PICKUP_TYPE)i);
			}
		}
	}

	// ------------------------------------------------------------------------------

	E_PICKUP_TYPE PickupPositionInfo::AssignPickup(Util::C_Random& randGen)
	{
		E_PICKUP_TYPE assignedType = E_PICKUP_TYPE_NONE;

		if(!mPickupsAvailable.Size())
			Init();

		int randIdx = randGen.Rand() % mPickupsAvailable.Size();

		if(randIdx >= 0 && randIdx < mPickupsAvailable.Size())
		{
			assignedType = mPickupsAvailable[randIdx];

			mPickupsAvailable.Swap<E_PICKUP_TYPE>(mPickupsAvailable.Begin() + randIdx, mPickupsAvailable.End() - 1);
			mPickupsAvailable.PopBack();
		}

		return assignedType;
	}
}