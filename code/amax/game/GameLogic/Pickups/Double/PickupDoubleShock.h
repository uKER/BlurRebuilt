#ifndef PICKUP_DOUBLE_SHOCK_H
#define PICKUP_DOUBLE_SHOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

namespace Game
{
	class C_PickupDoubleShock : public C_PickupBase
	{
	public:
		C_PickupDoubleShock(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void					SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);

		virtual bool			Update();
		virtual void			DebugDraw();


	private:
		//RLShockBalancingData*	mpShockData;
		float					mTotalLifetime;
		float					mTimeToNextShock;
		Util::C_Random			mRandGenerator;
		float					mBombardRadius;
		int						mShocksDropped;
		int						mTargetIdx;
	};
}

#endif //PICKUP_DOUBLE_SHOCK_H