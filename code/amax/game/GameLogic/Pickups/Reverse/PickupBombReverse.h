#ifndef PICKUP_BOMB_REVERSE_H
#define PICKUP_BOMB_REVERSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupBombReverse : public C_PickupProjectile
	{
	public:
		C_PickupBombReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual void				PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void						SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);
		virtual bool				Update();

		bool						IsDefenderCentric() { return false; }
		virtual bool				NinjaKillIfAnyHit() { return true; }

		virtual void				Explode(Maths::HVec3 pos, Maths::C_Normal normal);
		virtual void				Trigger(int idx);

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

		virtual void				DebugDraw();
		virtual void				CollideWithEnvironment(Maths::HVec3 hitPoint, Maths::C_Normal normal);

	private:
		RLBombReverseBalancingData*	mpBombData;
		RLBombSpecialFX*			mpBombSFX;
		RLBalancingGraph*			mpBombGraph;

		float						mLiveTime;
		float						mInitialSpeed;
	};
}

#endif //PICKUP_BOMB_REVERSE_H
