#ifndef PICKUP_SHUNT_REVERSE_H
#define PICKUP_SHUNT_REVERSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupProjectileBase.h>

namespace Game
{
	class C_PickupShuntProjectileReverse : public C_PickupProjectile
	{
	public:
		C_PickupShuntProjectileReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual void				PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void						SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);
		virtual bool				Update();

		virtual void				Explode(Maths::HVec3 pos, Maths::C_Normal normal);
		virtual void				Trigger(int idx);
		virtual bool				NinjaKillIfAnyHit() { return true; }

		virtual void				DebugDraw();

	protected:
		RLShuntReverseBalancingData*	mpShuntData;
		RLShuntSpecialFX*				mpShuntSFX;

		float							mAliveTime;
		float							mDropFXTimer;
	};
}

#endif //PICKUP_SHUNT_REVERSE_H
