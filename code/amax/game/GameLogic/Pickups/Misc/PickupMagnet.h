#ifndef PICKUP_MAGNET_H
#define PICKUP_MAGNET_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	class C_PickupFireMagnet : public C_PickupFireBase
	{
	public:
		C_PickupFireMagnet(U32 id, int level, int firer);

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();
		void CheckObstacles(Maths::HVec3 dir);

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_MAGNET; };

		float			mRadius;
		float			mTotalRadius;
		int				mTargettedVehicle;
		Maths::HVec3	mProjectilePos;
		Maths::HMat43	mMissMatrix;
		Maths::HVec3	mFireDir;
	};

	class C_PickupHitMagnet : public C_PickupHitBase
	{
	public:
		C_PickupHitMagnet(U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_MAGNET; };
	};
}

#endif //PICKUP_MAGNET_H