#ifndef PICKUP_GHOST_H
#define PICKUP_GHOST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireGhost : public C_PickupFireBase
	{
	public:
		C_PickupFireGhost(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mTimer(0) {};

		virtual bool Update(bool p_IsMultiplayer = false);

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_GHOST; };

		float mTimer;
	};
}

#endif //PICKUP_GHOST_H