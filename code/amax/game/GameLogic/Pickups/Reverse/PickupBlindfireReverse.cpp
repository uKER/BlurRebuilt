#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Reverse/PickupBlindfireReverse.h>

namespace Game
{
	C_PickupBlindfireProjectileReverse::C_PickupBlindfireProjectileReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, int group_id) 
		: C_PickupBlindfireProjectile(E_PickupClass_BlindfireProjectileReverse, id, firingIdx, subType, noFiringVehicleFX, group_id)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE);
		mpBlindfireData						= (RLBlindfireBalancingData*)pPickup->mReversed->mBalancingData;
		mpBlindfireSFX						= (RLBlindfireSpecialFX*)pPickup->mReversed->mSpecialFX;

		BIZ_ASSERT(mpBlindfireData);
		BIZ_ASSERT(mpBlindfireSFX);

		mSpeed		= mpBlindfireData->mSpeed;
		mFallRate	= mpBlindfireData->mFallRate;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBlindfireProjectileReverse::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		Maths::HMat43 reverseMatrix = matrix;
		reverseMatrix.AZ(-matrix.AZ());
		C_PickupBlindfireProjectile::PostDelayInitialise(reverseMatrix, sfx);
	}
}