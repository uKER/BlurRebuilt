#ifndef PICKUP_MINI_BOMB_H
#define PICKUP_MINI_BOMB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\Pickups\Upgraded\PickupBomb.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupMiniBomb : public C_PickupBomb
	{
	public:
		C_PickupMiniBomb(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float dirId, U32 seed, float timerOverride = -1.f);

	protected:
		virtual void					CreateCollisionBodies();	
		virtual void					PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx);

		void							Prime(int vehicleIndex) {};//Don't do nuffin'

		Util::C_Random					mRandGen;
		int								mMiniMineId;	
	};
}

#endif //PICKUP_MINI_BOMB_H