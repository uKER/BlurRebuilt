#ifndef PICKUP_SHIELD_H
#define PICKUP_SHIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>


namespace Game
{
	class C_PickupShield : public C_PickupBase
	{
	public:
		C_PickupShield(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX);

		virtual bool			Update();
		virtual void			PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void			DebugDraw();
		void					Restart();

	private:
		void					ApplyShieldModMultipliers(float& shieldTime, float& shieldHits);
		RLShieldBalancingData*	mpShieldData;
		RLShieldSpecialFX*		mpShieldSFX;

		bool					mShieldModActive;
		bool					mBossShieldModActive;
		bool					mGenerateNewShield;
	};
}

#endif //PICKUP_SHIELD_H
