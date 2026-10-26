#ifndef PICKUP_DOUBLE_SHIELD_H
#define PICKUP_DOUBLE_SHIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\Pickups\Normal\PickupShield.h>

namespace Game
{
	class C_PickupDoubleShield : public C_PickupShield
	{
	public:
		C_PickupDoubleShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool Update();

	private:
		float	mOldMass;
	};
}

#endif //PICKUP_DOUBLE_BOMB_H