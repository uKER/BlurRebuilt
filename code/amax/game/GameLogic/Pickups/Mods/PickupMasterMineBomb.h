#ifndef PICKUP_MASTERMINE_BOMB_H
#define PICKUP_MASTERMINE_BOMB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>	// [Blur2PC]

namespace Game
{
	class C_PickupMasterMineBomb : public C_PickupBase
	{
	public:
		C_PickupMasterMineBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, U32 seed);

		virtual bool Update();
		virtual void DebugDraw() {};

		virtual void PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);

	private:
		bool CanPlaceMine(Maths::HMat43& matrix);

		void MinePlacementFixed();
		void MinePlacementRandom();

		Util::C_Random mRandGen;
	};
}

#endif //PICKUP_MASTERMINE_BOMB_H