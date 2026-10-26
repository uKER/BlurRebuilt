#ifndef PICKUP_SHOCK_H
#define PICKUP_SHOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupShock : public C_PickupBase
	{
	public:
		C_PickupShock(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float shockWidth = 1.f, float lifetimeMod = 0.f, int logicalIndex = -1, bool noPhysical = false);
		virtual ~C_PickupShock();
		
		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void					SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);

		virtual bool			Update();

		virtual void			DebugDraw();

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

	private:
		void					CheckDynamicObjects();

		RLShockBalancingData*	mpShockData;

		float					mDelay;
		float					mCurrentLifetime;
		float					mCurrentOuterRadius;
		float					mCurrentInnerRadius;
		float					mSizeMultiplier;

		float					mTotalOuterRadius;
		float					mTotalInnerRadius;
		bool					mHasHitTarget;

		float					mTrackSegmentWidth;
		float					mTotalLifetime;
		int						mTargetIdx;
	

		Btl::Set<Actor::C_ActorHandle> mDynamicsHit;

	};

	class C_PickupEffectsShock : public C_PickupLocalEffectsBase
	{
	public:
		C_PickupEffectsShock(E_PICKUP_TYPE type, int vehIdx, float time = 0.f);

		virtual bool Update();

	private:
		bool mPhotoTaken;
	};
}

#endif //PICKUP_SHOCK_H
