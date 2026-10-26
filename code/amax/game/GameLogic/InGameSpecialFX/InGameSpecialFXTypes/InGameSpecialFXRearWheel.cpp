#include "stdafx.h"
#include <GameLogic/Layers/RaceLogicLayer.h>	// [Blur2PC]

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXRearWheel.h>
#include <vehicle/input.h>

extern Debug::Logger& g_InGameSFXLogger;

namespace Game
{
	void C_InGameSpecialFXRearWheel::GetMatrixVelocityCollision(Maths::HMat43& matrix, Maths::HVec3& velocity, Maths::HVec3& planePos, Maths::C_Normal& planeNormal) const
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

			const Vehicle::C_ModelInput *pModelInput = pActor->GetGraphicalActor(mVehicleIndex)->GetVehicleInput();
			const Vehicle::C_WheelInput& wheel2 = pModelInput->m_Wheels[2];
			const Vehicle::C_WheelInput& wheel3 = pModelInput->m_Wheels[3];

			Maths::HMat43 wheelMat(mLeftWheel ? carMatrix.AX() : -carMatrix.AX(), carMatrix.AY(), carMatrix.AZ(), mLeftWheel ? wheel2.m_mMatrix.AW() : wheel3.m_mMatrix.AW());
			matrix = wheelMat;
		}
		else
		{
			LOGWARN(g_InGameSFXLogger, "C_InGameSpecialFXRearWheel::Update Trying to access an actor that does not exist.");
		}
	}


	void C_InGameSpecialFXRearWheel::Update()
	{
		PERF_EVENT("InGameSFX.Vehicle.Update");
		
		Maths::HMat43 wheelMat(Maths::UNDEFINED);
		Maths::HVec3 velocity(Maths::UNDEFINED);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);
		GetMatrixVelocityCollision(wheelMat, velocity, planePos, planeNormal);

		if(mBumperSpecialFX)
			mBumperSpecialFX->UpdateWorldMatrix(wheelMat, velocity, planePos, planeNormal);

		if(mChaseSpecialFX)
			mChaseSpecialFX->UpdateWorldMatrix(wheelMat, velocity, planePos, planeNormal);

		if(mGlobalSpecialFX)
			mGlobalSpecialFX->UpdateWorldMatrix(wheelMat, velocity, planePos, planeNormal);
		

		C_InGameSpecialFXBase::Update();
	}
}