#ifndef PICKUP_DOUBLE_NITROUS_H
#define PICKUP_DOUBLE_NITROUS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupDoubleNitrous : public C_PickupBase
	{
	public:
		C_PickupDoubleNitrous(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool			Update();
		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void			DebugDraw() {};

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

	private:
		NitrousDoubleTapEffectInfo*	mpNitrousData;
		float						mTimer;
	};
}

#endif //PICKUP_DOUBLE_NITROUS_H
