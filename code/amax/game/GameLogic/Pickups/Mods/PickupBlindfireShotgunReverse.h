#ifndef PICKUP_BLINDFIRE_SHOTGUN_REVERSE
#define PICKUP_BLINDFIRE_SHOTGUN_REVERSE

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupBlindfireShotgunReverse : public C_PickupBase
	{
	public:
		C_PickupBlindfireShotgunReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool Update();
		virtual void DebugDraw() {};

		virtual void PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
	};
}

#endif //PICKUP_BLINDFIRE_SHOTGUN_REVERSE