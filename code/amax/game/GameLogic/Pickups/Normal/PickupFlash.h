#ifndef PICKUP_FLASH_H
#define PICKUP_FLASH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupMineBase.h>
#include <gravity/sdk/Body.h>

namespace Game
{
	class C_PickupFireFlash : public C_PickupFireMineBase
	{
	public:
		C_PickupFireFlash(U32 id, U8 level, int firer) : C_PickupFireMineBase(id, level, firer) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();
		
		void Explode();
		virtual void Trigger(int idx, int mine_idx);

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_FLASH; };
	};

	class C_PickupHitFlash : public C_PickupHitBase
	{
	public:
		C_PickupHitFlash(U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer), mTimer(0.f), mPlayEffect(false) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_FLASH; };

		float mTimer;
		bool mPlayEffect;
	};

}

#endif //PICKUP_FLASH_H