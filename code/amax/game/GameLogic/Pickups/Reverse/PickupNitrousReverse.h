#ifndef PICKUP_NITROUS_REVERSE_H
#define PICKUP_NITROUS_REVERSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupNitrousReverse : public C_PickupBase
	{
	public:
		C_PickupNitrousReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool					Update();
		virtual void					PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void					DebugDraw();
		virtual E_TARGETTING_DIRECTION	GetHitDirection(Actors::C_VehicleLogicActor* pActor);

	private:
		RLNitrousReverseBalancingData*	mpNitrousData;
		float							mTimer;
	};

	class C_PickupEffectsNitrousReverse : public C_PickupLocalEffectsBase
	{
	public:
		C_PickupEffectsNitrousReverse(E_PICKUP_TYPE type, int vehIdx, float time = 0.f);

		virtual bool Update();

		virtual void KillEffect();

	private:
		RLNitrousReverseBalancingData*	mpNitrousData;
		float							mPreviousSpeed;
		float							mStraightenDelay;
		bool							mDisableForwardBoost;
	};
}

#endif //PICKUP_NITROUS_REVERSE_H