#ifndef PICKUP_NITROUS_H
#define PICKUP_NITROUS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	struct NitrousEffectInfo;
	struct NitrousDoubleTapEffectInfo;

	class C_PickupEffectsNitrous : public C_PickupLocalEffectsBase
	{
	public:
		C_PickupEffectsNitrous(E_PICKUP_TYPE type, int vehIdx, float time = 0.f);

		virtual bool	Update();

		float			CalcSpeedFromGraph(float graphVal);
		void			DisableSlowdown() { mDisableSlowdown = true; };
		void			StopEffect();

	private:
		NitrousEffectInfo*			mpNitrousData;
		float						mDuration;
		float						mClassMult;
		bool						mDisableSlowdown;
		float						mPrevSpeed;
	};
}

void DebugNitrousTestBedUpdate();
void DebugNitrousTestBedReset();

#endif //PICKUP_NITROUS_H