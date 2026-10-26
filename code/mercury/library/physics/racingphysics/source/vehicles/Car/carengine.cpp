#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

//using namespace Maths;

void CarEngine::InitialiseConstants(Car* pCar, Rotor* pRotor)
{
	m_pRotor = pRotor;
	m_pCar = pCar;
}

void CarEngine::Initialise()
{
	m_pRotor->Initialise();
	SetMassVariables();

	m_dd.powerCut = false;

	m_dd.tractionControl.Initialise();
	m_dd.adjustedThrottle.Initialise();
}

void CarEngine::SetMassVariables()
{
	m_pRotor->SetMassVariables(Dynamics().engine.engineInertia);
}

void CarEngine::SetCarAxis(const Axis& axis)
{
	Axis a = axis;

	if (!Dynamics().engine.transverse)
		a.Rotate(axis.AxisY(), Constants<float>::Pi() * 0.5f);

	m_pRotor->SetRotation(a);
}

void CarEngine::BeginIntegrate()
{
	SetMassVariables();

	float rpm = RPM();

	// rev limiter
	if (!m_dd.powerCut && rpm > LimiterRPM())
		m_dd.powerCut = true;
	else if (m_dd.powerCut && rpm < LimiterRPM() * Dynamics().engine.powerCutFraction)
		m_dd.powerCut = false;

	m_dd.tractionControl.Begin();
	m_dd.adjustedThrottle.Begin();
}

void CarEngine::EndIntegrate()
{
	float s_tractionControlSmoothTime = 0.1f;
	m_dd.tractionControl.End(m_pCar->TimeStep(), s_tractionControlSmoothTime);
	m_dd.adjustedThrottle.End();
}

FloatVR CarEngine::RPM() const
{
	Normal3R x = GetDriveAxis();
	Vector3R w = GetRotor().AngularVelocity();

	FloatVR rpm = Abs(x | w) / PhysicsConstants::RPM;
	//RP_ASSERT(rpm < Dynamics().engine.limiterRPM * 2.f);
	return rpm;
}

FloatVR CarEngine::AngularVelocity1D() const
{
	Normal3R x = GetDriveAxis();
	Vector3R w = GetRotor().AngularVelocity() - m_pCar->Body().AngularVelocity();
	return -(x | w);
}

float CarEngine::GetThrottle(float rpm, float targetRPM) const
{
	float maxRPM = Dynamics().engine.engineTorque_Rpm.x1;
	float s = Dynamics().engine.engineInertia * (maxRPM * PhysicsConstants::RPM);
	float engineBrakingAtTarget = Dynamics().engine.engineBraking_RpmFraction.Evaluate(targetRPM / maxRPM, false) * s;
	float torque = GetTorque(rpm);
	float throttle = engineBrakingAtTarget / (torque + engineBrakingAtTarget + 1.f);
	return Clamped(1.f - rpm / targetRPM + throttle, 0.f, 1.f);
}

float CarEngine::GetTorque(float rpm) const
{
	Normal3R x = GetDriveAxis();
	Vector3R w = GetRotor().AngularVelocity();

	float av = (x | w);

	float t = Dynamics().engine.engineTorque_Rpm.Evaluate(rpm);

	t *= m_pCar->PerformanceModifiers().peakTorqueScale;

	// limit torque according to speed
	float power = t * Abs(av);
	float tmax = m_pCar->Mass() * m_pCar->MaxAcceleration_Step() * 4.f * (Magnitude(m_pCar->GetVelocity() - m_pCar->m_dd.frame.groundVelocity) + 4.f) / (Abs(av) + 1.f);
	t = Min(t, tmax + (t - tmax) * AddSmooth(m_pCar->Brake(), m_pCar->Handbrake()));

	return t;
}

void CarEngine::EvaluateControlForces(ArticulatedBody::Derivative& derivative, FloatVR maxTotalEngineTorque)
{
	RP_ASSERT(maxTotalEngineTorque >= 0.f);

	float engineTorque = 0.f;

	Normal3R x = GetDriveAxis();
	Vector3R w = GetRotor().AngularVelocity();

	float av = (x | w);
	float throttle = m_pCar->Throttle();

	float rpm = RPM();
	float maxRPM = Dynamics().engine.engineTorque_Rpm.x1;

	RP_ASSERT(throttle >= 0.f && throttle <= 1.f);

	// idle and limiter
	if (m_dd.powerCut)
	{
		throttle = 0.f;
	}
	else if (rpm < Dynamics().engine.idleRPM * 1.1f) 
	{
		throttle = AddSmooth(throttle, GetThrottle(rpm, Dynamics().engine.idleRPM));
	}
	else if (m_pCar->Transmission().GearChange())
	{
		float targetRPM = m_pCar->Transmission().TargetRPM();
		throttle = GetThrottle(rpm, targetRPM);
	}

	RP_ASSERT(throttle >= 0.f && throttle <= 1.f);

	// engine braking
	{
		float d = Clamped(av * Dynamics().engine.engineInertia / m_pCar->TimeStep(), -1.f, 1.f);
		float s = Dynamics().engine.engineInertia * (maxRPM * PhysicsConstants::RPM);
		engineTorque = Dynamics().engine.engineBraking_RpmFraction.Evaluate(rpm / maxRPM, false) * d * s;	
		engineTorque *= (1.f - throttle);
	}

	m_dd.adjustedThrottle.Accumulate(throttle * m_pCar->AccumulationFactor());

	// engine torque
	float t = GetTorque(rpm);

	engineTorque += t * throttle;

	float power_Hp = engineTorque * Abs(av) / PhysicsConstants::HORSEPOWER;

	Vector3R torque = x * engineTorque;
	m_pCar->ApplyTorque(derivative, m_pCar->BodyIndex(), m_pCar->EngineIndex(), torque);
}

void CarEngine::GetDmaData(void** pDmaData)
{
	*pDmaData = &m_dd;
}

void CarEngine::SpuFetch(const void* pDmaData)
{
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));
}

void CarEngine::SpuStore(void* pDmaData) const
{
	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));
}

}
