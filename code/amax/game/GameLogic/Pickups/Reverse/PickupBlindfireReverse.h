#ifndef PICKUP_BLINDFIRE_REVERSE_H
#define PICKUP_BLINDFIRE_REVERSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic/Pickups/Normal/PickupBlindfire.h>

namespace Game
{
	class C_PickupBlindfireProjectileReverse : public C_PickupBlindfireProjectile
	{
	public:
		C_PickupBlindfireProjectileReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, int group_id);

		virtual void PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);

		bool		IsDefenderCentric() { return false; }
	};
}

#endif //PICKUP_BLINDFIRE_REVERSE_H
