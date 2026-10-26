#ifndef PICKUP_TELEPORT_H
#define PICKUP_TELEPORT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireTeleport : public C_PickupFireBase
	{
	public:
		C_PickupFireTeleport(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mRadius(0) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_TELEPORT; };

		float			mRadius;
		float			mTotalRadius;
		int				mTargettedVehicle;
	};

	class C_PickupHitTeleport  : public C_PickupHitBase
	{
	public:
		C_PickupHitTeleport (U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_TELEPORT; };
	};
}

#endif //PICKUP_TELEPORT_H