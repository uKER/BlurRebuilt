#ifndef PICKUP_BOMBARD_H
#define PICKUP_BOMBARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireBombard : public C_PickupFireBase
	{
	public:
		C_PickupFireBombard(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_BOMBARD; };

		float			mHeight;
		float			mTimer;
		float			mAngleTimer;

		Gravity::C_ExternalConstants mExternalConstants;
	};

	class C_PickupHitBombard  : public C_PickupHitBase
	{
	public:
		C_PickupHitBombard (U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_BOMBARD; };
	};
}

#endif //PICKUP_BOMBARD_H