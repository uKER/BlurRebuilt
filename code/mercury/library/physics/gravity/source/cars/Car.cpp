#include "stdafx.h"
#include "gravity/cars/car.h"
#include "gravity/cars/CarConstants.h"

namespace Gravity
{
	void C_CarWheel::SetConstraint(SharedPhysics::C_Constraint* pConstraint)
	{
		m_Constraint = pConstraint;
		m_Car->m_pScene->AddConstraint(m_Constraint);
	}

	void C_CarWheel::SetUpWheel(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, Gravity::C_ExternalConstants PCREF externalConstants, Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = 0.001f;
		deactivator.m_SleepThresholdSq = 0.0005f;

		C_Body* cylinderBody(m_Car->m_pScene->CreateBody());
		cylinderBody->SetExternalConstants(&externalConstants);
		Gravity::C_CylinderDesc cylinderDesc;
		cylinderDesc.m_Height=height;
		cylinderDesc.m_Radius=radius;
		const Maths::HF32 margin(Maths::Min(radius, height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, height) * 0.001f);
		cylinderDesc.m_Margin = margin;
		cylinderDesc.m_Epsilon = epsilon;
		Gravity::C_BodyHelper bodyHelper(&cylinderDesc);
		bodyHelper.CreateBody(mass, position, orientation, cylinderBody, pFilter, 0, 0);
		cylinderBody->SetDeactivator(deactivator);
		cylinderBody->Slop(0.00000005f);
		cylinderBody->Baumgarte(0.8f);

		m_pBody=cylinderBody;
		m_Car->m_pScene->AddBody(m_pBody);
	}


	F32 C_Car::CalculateTractiveForceWithoutInertialLoss(const F32 rpm, const U32 index)
	{
		const F32 engineTorque = m_Engine.GetTorque(rpm);
		const F32 combinedRatio(m_Engine.m_Xt.m_TransmissionGearRatio * m_Engine.m_Xt.m_FinalDriveRatio);
		const F32 combinedEfficient(m_Engine.m_Xt.m_TransmissionEfficient * m_Engine.m_Xt.m_FinalDriveRatio);
		return (engineTorque * combinedRatio * combinedEfficient)/m_Wheels[index].m_Radius;
	}

	//Calculate the translational speed at the circumference of the tire
	F32 C_Car::CalculateWheelTranslationalSpeed(const U32 wheelIndex)
	{
		F32 engineSpeed = m_Engine.GetRPM() * CarConstants::RPM; // rev per second
		const F32 combinedRatio(m_Engine.m_Xt.m_TransmissionGearRatio * m_Engine.m_Xt.m_FinalDriveRatio);
		F32 wheelRotationalSpeed = engineSpeed / combinedRatio; //radius per second
		return wheelRotationalSpeed * m_Wheels[wheelIndex].m_Radius;
	}


	//Calculate the weight on wheel
	F32 C_Car::CalculateLoadWeightOnFrontWheel(const F32 acceleration, const F32 theta)
	{
		//const F32 base = (wheelIndex < 2)? m_CarXt.m_FrontWheelBase : m_CarXt.m_BackWheelBase;
		const F32 mass(m_pCarBody->GetBody().Mass());
		const F32 weight = CarConstants::GRAVITY * mass;
		const F32 cosTheta(cos(theta));
		const F32 sinTheta(sin(theta));
		//static loads
		const F32 staticLoad(weight *(m_CarXt.m_FrontWheelBase/ m_CarXt.m_WheelBase) * cosTheta);
		//low speed acceleration
		const F32 accelerationLoad(mass * acceleration * (m_CarXt.m_CGHeight * m_CarXt.m_WheelBase));
		//Loads on grades
		const F32 gradesLoad(weight*m_CarXt.m_CGHeight * sinTheta);
		
		return staticLoad - accelerationLoad - gradesLoad;

		//return (wheelIndex < 2) ? (staticLoad - accelerationLoad - gradesLoad) : (staticLoad + accelerationLoad + gradesLoad);

	}

	F32 C_Car::CalculateLoadWeightOnBackWheel(const F32 acceleration, const F32 theta)
	{
		//const F32 base = (wheelIndex < 2)? m_CarXt.m_FrontWheelBase : m_CarXt.m_BackWheelBase;
		const F32 mass(m_pCarBody->GetBody().Mass());
		const F32 weight = CarConstants::GRAVITY * mass;
		const F32 cosTheta(cos(theta));
		const F32 sinTheta(sin(theta));
		//static loads
		const F32 staticLoad(weight *(m_CarXt.m_BackWheelBase/ m_CarXt.m_WheelBase) * cosTheta);
		//low speed acceleration
		const F32 accelerationLoad(mass * acceleration * (m_CarXt.m_CGHeight * m_CarXt.m_WheelBase));
		//Loads on grades
		const F32 gradesLoad(weight*m_CarXt.m_CGHeight * sinTheta);

		return staticLoad + accelerationLoad + gradesLoad;

	}

	//Calculate the rolling resistance
	F32 C_Car::CalculateRollingResistance(const F32 acceleration, const F32 theta)
	{
		const F32 weightOnFront = CalculateLoadWeightOnFrontWheel(acceleration, theta) * 2.f;
		const F32 weightOnRear  = CalculateLoadWeightOnBackWheel(acceleration, theta) * 2.f;
		return C_Car::m_CarWheelXt.m_RollingResistanceCoefficient *(weightOnFront + weightOnRear);
	}

	void C_Car::DisplayDebug()
	{
	};

}