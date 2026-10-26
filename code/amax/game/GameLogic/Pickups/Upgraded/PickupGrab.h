#ifndef PICKUP_GRAB_H
#define PICKUP_GRAB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireGrab : public C_PickupFireBase
	{
	public:
		C_PickupFireGrab(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mRadius(0) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_GRAB; };

		float			mRadius;
		float			mTotalRadius;
		int				mTargettedVehicle;
	};

	class C_PickupHitGrab  : public C_PickupHitBase
	{
	public:
		C_PickupHitGrab (U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_GRAB; };
	};
}

#endif //PICKUP_GRAB_H