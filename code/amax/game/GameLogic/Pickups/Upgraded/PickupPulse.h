#ifndef PICKUP_PULSE_H
#define PICKUP_PULSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFirePulse : public C_PickupFireBase
	{
	public:
		C_PickupFirePulse(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mTimer(0) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_PULSE; };

		float mTimer;
	};

	class C_PickupHitPulse : public C_PickupHitBase
	{
	public:
		C_PickupHitPulse(U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer), mDistanceToPlayer(0) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_PULSE; };

		float mDistanceToPlayer;
	};
}

#endif //PICKUP_PULSE_H