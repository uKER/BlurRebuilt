#ifndef PICKUP_BARGE_H
#define PICKUP_BARGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupBarge : public C_PickupBase
	{
	public:
		C_PickupBarge(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);
		
		virtual void			FireChargingSpecialFX();

		virtual bool			Update();
		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void					CheckObstacles();

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

		virtual void			DebugDraw();

	private:
		bool					IsDefenderCentric() { return false; }
		void					ReportProjectileDestroyedEvent(C_PickupBase* pPickup);
		bool					IsInsideEllipse(Maths::HVec3 targetPos, float distAddition = 0.f);
		void					CheckDynamicObjects();

		RLBargeBalancingData*	mpBargeData;
		F32						mSizeMultiplier;
		float					mTimer;

		float					mMajorRadius;
		float					mMinorRadius;

		bool					mFixedPosition;

		Btl::Set<Actor::C_ActorHandle> mDynamicsHit;
	};
}

#endif //PICKUP_BARGE_H
