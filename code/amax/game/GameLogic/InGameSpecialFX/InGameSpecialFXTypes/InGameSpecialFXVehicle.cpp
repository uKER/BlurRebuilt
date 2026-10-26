#include "stdafx.h"
#include <GameLogic/Layers/RaceLogicLayer.h>	// [Blur2PC]

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXVehicle.h>

extern Debug::Logger& g_InGameSFXLogger;

namespace Game
{
	void C_InGameSpecialFXVehicle::GetMatrixVelocityCollision(Maths::HMat43& matrix, Maths::HVec3& velocity, Maths::HVec3& planePos, Maths::C_Normal& planeNormal) const
	{
		C_RaceLogicLayer* pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pRaceLogicLayer->GetLogicVehicleActor(mVehicleIndex);

		if(pActor)
		{
			Actors::C_VehicleActor* pVehicleActor = pActor->GetGraphicalActor(mVehicleIndex);
			matrix = pVehicleActor->GetMatrix();
			velocity = pVehicleActor->GetVelocity();
			planePos = pVehicleActor->GetGroundPosition();
			planeNormal = pVehicleActor->GetGroundNormal();

			if(mInBumperCam && mViewport != -1 && !mForceChaseCam)
			{
				AmaxCam::C_VehicleFeed* pFeed	= AmaxCam::CameraSystem().VehicleFeed(mViewport);

				if(pFeed)
				{
					Maths::HVec3 pos			= pFeed->Position();
					matrix.AW(pos);

					Maths::HMat43 invMatrix		= matrix.InverseRotTrans();
					Maths::HMat43 localMatrix	= invMatrix * matrix;

					localMatrix.AW(localMatrix.AW() + mOffset);

					// Should make this a local variable if more than just shield is going to use it
					if(mTrigger == E_InGameSpecialFX_Trigger_Shield)
					{
						F32 lookAngle = pFeed->GetLookAngle();

						S32 lookDir = (S32)(lookAngle * 2.f / Maths::Pi + 2.5f); // 0 - 4
						lookDir &= 3; // 0 - 3
						lookAngle = (F32)(lookDir - 2) * Maths::Pi * 0.5f;

						Maths::HMat43 rotMat = Maths::HMat43(Maths::RotY(lookAngle));
						localMatrix = localMatrix * rotMat;
					}

					matrix = localMatrix * matrix;
				}

			}
			else
			{
				ScaleMatrix(matrix);
			}

		}
		else
		{
			LOGWARN(g_InGameSFXLogger, "C_InGameSpecialFXVehicle::Update Trying to access an actor that does not exist.");
		}
	}

	void C_InGameSpecialFXVehicle::Update()
	{
		PERF_EVENT("InGameSFX.Vehicle.Update");
		
		Maths::HMat43 matrix(Maths::UNDEFINED);
		Maths::HVec3 velocity(Maths::UNDEFINED);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);
		GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);

		if(mBumperSpecialFX)
			mBumperSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);

		if(mChaseSpecialFX)
			mChaseSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);

		if(mGlobalSpecialFX)
			mGlobalSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);
	
		C_InGameSpecialFXBase::Update();
	}
}