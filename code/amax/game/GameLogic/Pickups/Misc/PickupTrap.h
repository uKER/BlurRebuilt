#ifndef PICKUP_TRAP_H
#define PICKUP_TRAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>

namespace Game
{
	const int NUM_FRICKING_LASER_BEAMS = 8;

	typedef struct PickupTrapLaserBeam
	{
		bool mDirection;
		float mPos;

	}PickupTrapLaserBeam;

	class C_PickupFireTrap : public C_PickupFireBase
	{
	public:
		C_PickupFireTrap(U32 id, U8 level, int firer) : C_PickupFireBase(id, level, firer), mCreateGrid(false) {};

		virtual bool Update(bool p_IsMultiplayer = false);
		void WarnPlayers();

		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_TRAP; };

		bool mCreateGrid;
		float mTimer;

		bool			mGridActive;
		
		Maths::HVec3	mGridLeftPos;
		Maths::HVec3	mGridRightPos;

		Btl::Vector<PickupTrapLaserBeam, Btl::VectorParameters_Array<NUM_FRICKING_LASER_BEAMS * 2> > mLeftHeights;
		Btl::Vector<PickupTrapLaserBeam, Btl::VectorParameters_Array<NUM_FRICKING_LASER_BEAMS * 2> > mRightHeights;

	private:
		const bool TrapTriggered(Maths::HVec3 currentPosition, Maths::HVec3 lastPosition, Maths::HVec3 lineStart, Maths::HVec3 lineEnd);
	};

	class C_PickupHitTrap  : public C_PickupHitBase
	{
	public:
		C_PickupHitTrap (U32 id, U8 level, int firer) : C_PickupHitBase(id, level, firer) {};

		virtual bool Update(int vehIdx, bool p_IsMultiplayer);
		virtual E_PICKUP_TYPE GetType() { return E_PICKUP_TYPE_TRAP; };
	};
}

#endif //PICKUP_TRAP_H