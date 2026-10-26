#ifndef PICKUP_DOUBLE_SHUNT_REVERSE
#define PICKUP_DOUBLE_SHUNT_REVERSE

#include <GameLogic/Pickups/PickupBase.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	class C_PickupDoubleShuntReverse : public C_PickupBase
	{
	public:
		C_PickupDoubleShuntReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual void				PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);

		virtual bool				Update();
		virtual void				DebugDraw() {};
	};
}

#endif //PICKUP_DOUBLE_BOMB_H