#ifndef PICKUP_DOUBLE_SHUNT
#define PICKUP_DOUBLE_SHUNT

#include <GameLogic/Pickups/PickupBase.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	class C_PickupDoubleShunt : public C_PickupBase
	{
	public:
		C_PickupDoubleShunt(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual void				PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		//void						SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);

		virtual bool				Update();
		virtual void				DebugDraw() {};

	private:
		float						mTimer;
		int							mShotsFired;
		RLShuntDoubleBalancingData*	mpShuntData;
	};
}

#endif //PICKUP_DOUBLE_BOMB_H