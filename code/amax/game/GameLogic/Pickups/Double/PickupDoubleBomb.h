#ifndef PICKUP_DOUBLE_BOMB_H
#define PICKUP_DOUBLE_BOMB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>	// [Blur2PC]

namespace Game
{
	class C_PickupDoubleBomb : public C_PickupBase
	{
	public:
		C_PickupDoubleBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool Update();
		virtual void DebugDraw() {};

		virtual void PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);

	private:
		bool CanPlaceMine(Maths::HMat43& matrix);
	};
}

#endif //PICKUP_DOUBLE_BOMB_H