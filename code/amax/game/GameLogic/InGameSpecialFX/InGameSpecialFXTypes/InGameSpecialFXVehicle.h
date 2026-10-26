#ifndef INGAME_SPECIALFX_VEHICLE_H
#define INGAME_SPECIALFX_VEHICLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXBase.h>

namespace Game
{
	class C_InGameSpecialFXVehicle : public C_InGameSpecialFXBase
	{
	public:
		C_InGameSpecialFXVehicle(const RLPickupSpecialFX& sfxBase, int idx, U32 id, float time, int viewport, int firerViewport,
			E_InGameSpecialFX_Trigger trigger, Maths::HVec3 scale, Maths::HVec3 offset) 
			: mTrigger(trigger), mOffset(offset), C_InGameSpecialFXBase(E_InGameSpecialFXBase_Vehicle, sfxBase, id, idx, time, viewport, firerViewport, scale) {};

		void							Update();
		void							GetMatrixVelocityCollision(Maths::HMat43& matrix, Maths::HVec3& velocity, Maths::HVec3& planePos, Maths::C_Normal& planeNormal) const;

		E_InGameSpecialFX_Trigger		mTrigger;
		Maths::HVec3					mOffset;
	};
}

#endif //INGAME_SPECIALFX_VEHICLE_H