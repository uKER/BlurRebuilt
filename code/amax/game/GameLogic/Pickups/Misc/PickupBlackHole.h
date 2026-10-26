#ifndef PICKUP_BLACKHOLE_H
#define PICKUP_BLACKHOLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\Pickups\PickupMineBase.h>

namespace Game
{
	class C_PickupFireBlackHole : public C_PickupFireMineBase
	{
	public:
		C_PickupFireBlackHole(U32 id, U8 level, int firer) : C_PickupFireMineBase(id, level, firer) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();
		void Explode();
		virtual void Trigger(int idx, int mine_idx);

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_BLACK_HOLE; };

	};
}

#endif //PICKUP_BLACKHOLE_H