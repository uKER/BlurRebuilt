#ifndef PICKUP_RIFT_H
#define PICKUP_RIFT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireRift : public C_PickupFireBase
	{
	public:
		C_PickupFireRift(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mTimer(0) {};

		virtual bool Update(bool p_IsMultiplayer = false);

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_RIFT; };

		float mTimer;
	};
}

#endif //PICKUP_RIFT_H