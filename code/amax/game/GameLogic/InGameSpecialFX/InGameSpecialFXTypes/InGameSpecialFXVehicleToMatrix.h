#ifndef INGAME_SPECIALFX_VEHICLETOMATRIX_H
#define INGAME_SPECIALFX_VEHICLETOMATRIX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXBase.h>

namespace Game
{
	class C_InGameSpecialFXVehicleToMatrix : public C_InGameSpecialFXBase
	{
	public:
		C_InGameSpecialFXVehicleToMatrix(const RLPickupSpecialFX& sfxBase, Maths::HMat43 mat, int vehicle_index, U32 id, float time, 
			int viewport, int firerViewport, Maths::HVec3 scale = Maths::HVec3(1,1,1)) 
			: mMatrix(mat), C_InGameSpecialFXBase(E_InGameSpecialFXBase_VehicleToMatrix, sfxBase, id, vehicle_index, time, viewport, firerViewport, scale) {};

		void							Update();
		void							GetMatrixVelocityCollision(Maths::HMat43& matrix, Maths::HVec3& velocity, Maths::HVec3& planePos, Maths::C_Normal& planeNormal) const;

		Maths::HMat43					mMatrix;
	};
}

#endif //INGAME_SPECIALFX_VEHICLETOMATRIX_H