#include "stdafx.h"

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXVehicleToMatrix.h>
#include <Gamelogic/Layers/RaceLogicLayer.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

extern Debug::Logger& g_InGameSFXLogger;

namespace Game
{
	void C_InGameSpecialFXVehicleToMatrix::GetMatrixVelocityCollision(Maths::HMat43& matrix, Maths::HVec3& velocity, Maths::HVec3& planePos, Maths::C_Normal& planeNormal) const
	{
		C_RaceLogicLayer* pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pRaceLogicLayer->GetLogicVehicleActor(mVehicleIndex);

		if(pActor)
		{
			Actors::C_VehicleActor* pVehicleActor = pActor->GetGraphicalActor(mVehicleIndex);
			const Maths::HMat43& carMatrix = pVehicleActor->GetMatrix();
			velocity = pVehicleActor->GetVelocity();
			planePos = pVehicleActor->GetGroundPosition();
			planeNormal = pVehicleActor->GetGroundNormal();

			Maths::HVec3 direction = mMatrix.AW() - carMatrix.AW();
			direction.Normalise();

			//Better safe than crashed
			if(abs(direction.X() - carMatrix.AY().X())  <= 0.001f && abs(direction.Y() - carMatrix.AY().Y())  <= 0.001f && abs(direction.Z() - carMatrix.AY().Z())  <= 0.001f)
			{
				direction.X(direction.X()+0.001f);
			}

			Maths::HVec3 dirCrossCarUp = carMatrix.AY() ^ direction;
			dirCrossCarUp.Normalise();


			Maths::HVec3 result = direction ^ dirCrossCarUp;
			result.Normalise();

			Maths::HMat43 endMatrix(dirCrossCarUp, result, direction, carMatrix.AW());
			matrix = endMatrix;
		}
		else
		{
			LOGWARN(g_InGameSFXLogger, "C_InGameSpecialFXVehicleToMatrix::Update Trying to access an actor that does not exist.");
		}
	}

	void C_InGameSpecialFXVehicleToMatrix::Update()
	{
		Maths::HMat43 endMatrix(Maths::UNDEFINED);
		Maths::HVec3 velocity(Maths::UNDEFINED);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);
		GetMatrixVelocityCollision(endMatrix, velocity, planePos, planeNormal);
		
		if(mBumperSpecialFX)
			mBumperSpecialFX->UpdateWorldMatrix(endMatrix, velocity, planePos, planeNormal);

		if(mChaseSpecialFX)
			mChaseSpecialFX->UpdateWorldMatrix(endMatrix, velocity, planePos, planeNormal);

		if(mGlobalSpecialFX)
			mGlobalSpecialFX->UpdateWorldMatrix(endMatrix, velocity, planePos, planeNormal);
		
		C_InGameSpecialFXBase::Update();
	}
}