#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

///////////////////////////////////////////////////////////

namespace RacingPhysics
{

void CarTransmission::InitialiseConstants(Car* pCar)
{
	m_pCar = pCar;
}

void CarTransmission::Initialise()
{
	m_dd.currentGear = 1;
	m_dd.gearDest = 1;
	m_dd.clutchEngaged = 0.f;
	m_dd.gearChange = false;
	m_dd.gearChangeClutchDelay = 0.f;
}

void CarTransmission::Setup()
{
	// 0 - 7 = driveshaft index
	// D = differential
	// C = clutch
	// E = engine
	//
	// 4WD:          FWD:     RWD:
	// [0 D 1]       [0 D 1]     E
	//    4             6        7
	//    D 6 C 7 E     C        C
	//    5             7        6
	// [2 D 3]          E     [2 D 3]

	// initialise the driveshafts
	m_dd.driveShafts.Resize(8);

	// wheel driveshafts
	m_dd.driveShafts[0].inertia = Dynamics().mass.frontWheelMI;
	m_dd.driveShafts[1].inertia = Dynamics().mass.frontWheelMI;
	m_dd.driveShafts[2].inertia = Dynamics().mass.rearWheelMI;
	m_dd.driveShafts[3].inertia = Dynamics().mass.rearWheelMI;

	// engine driveshaft
	m_dd.driveShafts[7].inertia = Dynamics().engine.engineInertia;

	if (TransmissionDynamics().driveType == CarDynamicsXt::DRIVE_TYPE_REARWHEEL)
	{
		// differential between the two wheels
		m_dd.differentials.Resize(1);
		m_dd.differentials.Back().input = 6;
		m_dd.differentials.Back().output1 = 2;
		m_dd.differentials.Back().output2 = 3;

		m_dd.differentials.Back().type = TransmissionDynamics().rearDifferential.differentialType;
		m_dd.differentials.Back().forwardsTorque = TransmissionDynamics().rearDifferential.forwardsTorque;
		m_dd.differentials.Back().backwardsTorque = TransmissionDynamics().rearDifferential.backwardsTorque;
		m_dd.differentials.Back().viscosity = TransmissionDynamics().rearDifferential.viscosity;
	}

	if (TransmissionDynamics().driveType == CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL)
	{
		// differential between the two wheels
		m_dd.differentials.Resize(1);
		m_dd.differentials.Back().input = 6;
		m_dd.differentials.Back().output1 = 0;
		m_dd.differentials.Back().output2 = 1;

		m_dd.differentials.Back().type = TransmissionDynamics().frontDifferential.differentialType;
		m_dd.differentials.Back().forwardsTorque = TransmissionDynamics().frontDifferential.forwardsTorque;
		m_dd.differentials.Back().backwardsTorque = TransmissionDynamics().frontDifferential.backwardsTorque;
		m_dd.differentials.Back().viscosity = TransmissionDynamics().frontDifferential.viscosity;
	}

	if (TransmissionDynamics().driveType == CarDynamicsXt::DRIVE_TYPE_FOURWHEEL)
	{
		// front
		m_dd.differentials.Resize(3);
		m_dd.differentials[0].input = 4;
		m_dd.differentials[0].output1 = 0;
		m_dd.differentials[0].output2 = 1;

		m_dd.differentials[0].type = TransmissionDynamics().frontDifferential.differentialType;
		m_dd.differentials[0].forwardsTorque = TransmissionDynamics().frontDifferential.forwardsTorque;
		m_dd.differentials[0].backwardsTorque = TransmissionDynamics().frontDifferential.backwardsTorque;
		m_dd.differentials[0].viscosity = TransmissionDynamics().frontDifferential.viscosity;

		// rear
		m_dd.differentials[1].input = 5;
		m_dd.differentials[1].output1 = 2;
		m_dd.differentials[1].output2 = 3;

		m_dd.differentials[1].type = TransmissionDynamics().rearDifferential.differentialType;
		m_dd.differentials[1].forwardsTorque = TransmissionDynamics().rearDifferential.forwardsTorque;
		m_dd.differentials[1].backwardsTorque = TransmissionDynamics().rearDifferential.backwardsTorque;
		m_dd.differentials[1].viscosity = TransmissionDynamics().rearDifferential.viscosity;

		// mid
		m_dd.differentials[2].input = 6;
		m_dd.differentials[2].output1 = 4;
		m_dd.differentials[2].output2 = 5;

		m_dd.differentials[2].type = TransmissionDynamics().midDifferential.differentialType;
		m_dd.differentials[2].forwardsTorque = TransmissionDynamics().midDifferential.forwardsTorque;
		m_dd.differentials[2].backwardsTorque = TransmissionDynamics().midDifferential.backwardsTorque;
		m_dd.differentials[2].viscosity = TransmissionDynamics().midDifferential.viscosity;
	}

	// clutch between the engine and the differential input
	m_dd.clutch.input = 7;
	m_dd.clutch.output = 6;
}

void CarTransmission::BeginFrame()
{
	// manual gears
	{
		if (m_pCar->Controls().gearDown)
		{
			m_dd.gearDest = Max(m_dd.currentGear - 1, -1);
			if (m_dd.gearDest == 0) m_dd.gearDest = -1;
		}

		if (m_pCar->Controls().gearUp)
		{
			m_dd.gearDest = Min(m_dd.currentGear + 1, Dynamics().gears.topGear);
			if (m_dd.gearDest == 0) m_dd.gearDest = 1;
		}
	}

	// automatic gears
	if (m_pCar->AutoGears() && m_dd.gearChange == false && m_dd.clutchEngaged > 0.5f)
	{
		bool hasTraction = false;
		float count = 0;
		float wheelAv = 0.f;
		float groundAv = 0.f;
	
		for (int i = 0; i < 4; i++)
		{
			if (m_pCar->Wheel(i).OnGround() && m_pCar->Wheel(i).Driven())
			{
				count += 1.f;

				if (m_pCar->Wheel(i).WheelSpin() > -m_pCar->Wheel(i).LongitudinalTyreSlip())
					hasTraction = true;

				wheelAv += m_pCar->Wheel(i).CalculateAngularVelocity1D();
				Normal3R groundZ = Normalised(m_pCar->Wheel(i).GetDriveAxis() ^ m_pCar->Wheel(i).GroundNormal());
				float speed = Abs((m_pCar->GetVelocity() - m_pCar->GroundVelocity()) | groundZ);
				groundAv += speed / m_pCar->Wheel(i).Radius();
			}
		}

		if (count > 0.0f)
		{
			wheelAv /= count;
			groundAv /= count;
		}

		float minRpm = m_pCar->Engine().LimiterRPM() * Dynamics().gears.gearDownRPMFraction;
		float minAv = minRpm * PhysicsConstants::RPM;
		float maxGroundAv = m_pCar->Engine().LimiterRPM() * 0.8f * PhysicsConstants::RPM / GearRatio(m_dd.gearDest);

		if (m_dd.gearDest < Dynamics().gears.topGear 
			&& m_pCar->Engine().PowerCut() 
			&& hasTraction
			&& groundAv > maxGroundAv)
		{
			m_dd.gearDest++;
		}
		else
		{
			float speed = Abs(m_pCar->GroundSpeed_LocalZ());
			float rpm = m_pCar->Engine().RPM();

			while (m_dd.gearDest > 1
					&& (m_dd.gearDest > 2 || speed < Dynamics().gears.maxSpeedForChangeToFirst_Mph * PhysicsConstants::MPH)
					&& rpm < minRpm * GearRatio(m_dd.gearDest) / GearRatio(m_dd.gearDest - 1)
					&& groundAv * GearRatio(m_dd.gearDest - 1) < minAv 
					&& (!hasTraction || wheelAv * GearRatio(m_dd.gearDest - 1) < minAv))
				m_dd.gearDest--;
		}

		m_dd.targetRPM = groundAv * GearRatio(m_dd.gearDest) / PhysicsConstants::RPM;
	}

	// reverse
	if (m_pCar->Reverse())
	{
		m_dd.gearDest = -1;
	}
	else if (m_pCar->DriverSettings().autoGears && m_dd.currentGear == -1 && m_pCar->Controls().throttle > 0.f)
	{
		m_dd.gearDest = 1;
	}
}

float CarTransmission::TargetRPM() const
{
	return m_dd.targetRPM;
}

void CarTransmission::BeginIntegrate(float timeStep)
{
	if (m_dd.gearDest != m_dd.currentGear)
	{
		m_dd.gearChange = true;
		m_dd.clutchEngaged = 0.f;
		m_dd.gearChangeClutchDelay = Dynamics().engine.clutchDelay + m_pCar->PerformanceModifiers().clutchDelay;
		m_dd.clutchEngaged = 0.f;
		m_dd.currentGear = m_dd.gearDest;
	}
	else
	{
		if (m_dd.gearChangeClutchDelay > 0.f)
		{
			m_dd.gearChangeClutchDelay -= timeStep;
		}
		else
		{
			float cp = timeStep / Dynamics().engine.clutchSmoothTime;
			m_dd.clutchEngaged = Min(m_dd.clutchEngaged + cp, 1.f);
			m_dd.gearChange = false;
		}
	}

	if (m_pCar->Engine().RPM() < Dynamics().engine.idleRPM)
	{
		m_dd.clutchEngaged = 0.f;
		m_dd.gearChange = false;
	}

	if (!m_pCar->Air())
	{
		float speedFactor = Clamped((Maths::Abs(m_pCar->GroundSpeed_LocalZ()) - 1.f) / (10.f * PhysicsConstants::MPH), FloatVR(0.f), FloatVR(1.f));
		m_dd.clutchEngaged = Min(m_dd.clutchEngaged, 1.f - m_pCar->Handbrake() * (1.f - speedFactor));
	}

	if (m_dd.currentGear == 1 || m_dd.currentGear == -1)
	{
		float throttle = m_pCar->Throttle();
		float clutchFraction = ClutchRPMFraction();
		float startRPM = Maths::Lerp(Dynamics().engine.idleRPM * 1.1f, m_pCar->Engine().LimiterRPM(), clutchFraction * throttle);
		float endRPM = Min(startRPM * (1.1f + throttle), m_pCar->Engine().LimiterRPM());

		float f = (m_pCar->Engine().RPM() - startRPM) / (endRPM - startRPM);
		f = Clamped(f, 0.f, 1.f);
		m_dd.clutchEngaged = Min(m_dd.clutchEngaged, AddSmooth(f, 1.f - throttle));
	}
}

void CarTransmission::Finish()
{
	m_dd.driveShafts.Clear();
	m_dd.differentials.Clear();
}

FloatVR CarTransmission::FinalRatio() const
{
	return FloatVR(Dynamics().gears.finalRatio) * m_pCar->PerformanceModifiers().finalRatioScale;
}

FloatVR CarTransmission::ClutchRPMFraction()
{
	return (m_dd.currentGear == -1 && Dynamics().engine.clutchRPMFractionReverse > 0.f) ? Dynamics().engine.clutchRPMFractionReverse : Dynamics().engine.clutchRPMFraction;
}

FloatVR CarTransmission::GearRatio(int gear) const
{
	if (gear < 1)
	{
		if (Dynamics().gears.reverseGearRatio > 0.f)
			return -FloatVR(FinalRatio() * Dynamics().gears.reverseGearRatio);
		else
			return -FloatVR(FinalRatio() * Dynamics().gears.gearRatios[0]);
	}
	else
		return FloatVR(FinalRatio() * Dynamics().gears.gearRatios[gear - 1]);
}

void CarTransmission::CalculateViscousForces(float timeStep)
{
	for (int i = 0; i < m_dd.differentials.Size(); i++)
	{
		// output driveshaft velocity is set in advance
		float averageAV = (m_dd.driveShafts[m_dd.differentials[i].output1].angularVelocity
						+ m_dd.driveShafts[m_dd.differentials[i].output2].angularVelocity) * 0.5f;

		// input driveshaft velocity is the average
		m_dd.driveShafts[m_dd.differentials[i].input].angularVelocity = averageAV;

		// input driveshaft velocity is the average
		m_dd.driveShafts[m_dd.differentials[i].input].angularVelocity = averageAV;

		float totalInertia = m_dd.driveShafts[m_dd.differentials[i].output1].inertia
							+ m_dd.driveShafts[m_dd.differentials[i].output2].inertia;

		m_dd.driveShafts[m_dd.differentials[i].input].inertia = totalInertia;
	}

	for (int i = m_dd.differentials.Size() - 1; i >= 0; i--)
	{
		float driveTorque = m_dd.driveShafts[m_dd.differentials[i].input].torque;

		// output torque is split
		m_dd.driveShafts[m_dd.differentials[i].output1].torque = driveTorque * 0.5f;
		m_dd.driveShafts[m_dd.differentials[i].output2].torque = driveTorque * 0.5f;

		// clutch LSD
		if (m_dd.differentials[i].type == CarDynamicsXt::DIFFERENTIAL_VISCOUS)
		{
			float gearRatio = GearRatio();
			float avDifference = m_dd.driveShafts[m_dd.differentials[i].output1].angularVelocity - m_dd.driveShafts[m_dd.differentials[i].output2].angularVelocity;
			float invInertia = 1.f / m_dd.driveShafts[m_dd.clutch.input].inertia + (gearRatio * gearRatio) / m_dd.driveShafts[m_dd.clutch.output].inertia;

			float scale = m_dd.differentials[i].viscosity * Dynamics().mass.mass * PhysicsConstants::GRAVITY * 0.001f;
			float torque = avDifference * scale;

			m_dd.driveShafts[m_dd.differentials[i].output1].torque -= torque;
			m_dd.driveShafts[m_dd.differentials[i].output2].torque += torque;
		}
	}
}

void CarTransmission::CalculateConstraintForces(float timeStep)
{
	// calculate input velocities and inertias
	// (this works as long as the differentials are in the right order)
	for (int i = 0; i < m_dd.differentials.Size(); i++)
	{
		// output driveshaft velocity is set in advance
		float averageAV = (m_dd.driveShafts[m_dd.differentials[i].output1].angularVelocity
						+ m_dd.driveShafts[m_dd.differentials[i].output2].angularVelocity) * 0.5f;

		// input driveshaft velocity is the average
		m_dd.driveShafts[m_dd.differentials[i].input].angularVelocity = averageAV;

		float totalInertia = m_dd.driveShafts[m_dd.differentials[i].output1].inertia
							+ m_dd.driveShafts[m_dd.differentials[i].output2].inertia;

		m_dd.driveShafts[m_dd.differentials[i].input].inertia = totalInertia;
	}

	// find the torque across the clutch
	if (m_dd.currentGear != 0)
	{
		float gearRatio = GearRatio();
		float avDifference = m_dd.driveShafts[m_dd.clutch.input].angularVelocity - m_dd.driveShafts[m_dd.clutch.output].angularVelocity * gearRatio;
		float invInertia = 1.f / m_dd.driveShafts[m_dd.clutch.input].inertia + (gearRatio * gearRatio) / m_dd.driveShafts[m_dd.clutch.output].inertia;
		float torque = (avDifference / invInertia) / timeStep;

		float maxTorque = pow(m_dd.clutchEngaged, 4.f) * 10000.f;
		torque = ClampedAbs(torque, maxTorque);

		m_dd.driveShafts[m_dd.clutch.input].torque = -torque;
		m_dd.driveShafts[m_dd.clutch.output].torque = torque * gearRatio;

		float power_Hp = m_dd.driveShafts[m_dd.clutch.output].torque * m_dd.driveShafts[m_dd.clutch.output].angularVelocity / PhysicsConstants::HORSEPOWER;
		int breakhere = 0;
	}
	else
	{
		m_dd.driveShafts[m_dd.clutch.input].torque = 0.f;
		m_dd.driveShafts[m_dd.clutch.output].torque = 0.f;
	}

	// set the output torques (in reverse order)
	for (int i = m_dd.differentials.Size() - 1; i >= 0; i--)
	{
		float driveTorque = m_dd.driveShafts[m_dd.differentials[i].input].torque;

		// output torque is split
		m_dd.driveShafts[m_dd.differentials[i].output1].torque = driveTorque * 0.5f;
		m_dd.driveShafts[m_dd.differentials[i].output2].torque = driveTorque * 0.5f;

		// clutch LSD
		if (m_dd.differentials[i].type != CarDynamicsXt::DIFFERENTIAL_VISCOUS)
		{
			float avDifference = m_dd.driveShafts[m_dd.differentials[i].output1].angularVelocity 
								- m_dd.driveShafts[m_dd.differentials[i].output2].angularVelocity;
			float invInertia = 1.f / m_dd.driveShafts[m_dd.differentials[i].output1].inertia 
								+ 1.f / m_dd.driveShafts[m_dd.differentials[i].output2].inertia;
			float clutchTorque = (avDifference / invInertia) * 0.5f / timeStep;

			if (m_dd.differentials[i].type == CarDynamicsXt::DIFFERENTIAL_TORQUE)
				clutchTorque = ClampedAbs(clutchTorque, Abs(driveTorque) * Maths::Select_GT(driveTorque, 0.f, m_dd.differentials[i].forwardsTorque, m_dd.differentials[i].backwardsTorque));

			m_dd.driveShafts[m_dd.differentials[i].output1].torque -= clutchTorque;
			m_dd.driveShafts[m_dd.differentials[i].output2].torque += clutchTorque;
		}
	}
}

void CarTransmission::SetWheelAngularVelocity(int i, float av)
{
	m_dd.driveShafts[i].angularVelocity = av;
}

void CarTransmission::SetEngineAngularVelocity(float av)
{
	m_dd.driveShafts[7].angularVelocity = av;
}

float CarTransmission::GetEngineTorque() const
{
	return m_dd.driveShafts[7].torque;
}

float CarTransmission::GetWheelTorque(int i) const
{
	return m_dd.driveShafts[i].torque;
}

void CarTransmission::GetDmaData(void** pDmaData)
{
	*pDmaData = &m_dd;
}

void CarTransmission::SpuFetch(const void* pDmaData)
{
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));
}

void CarTransmission::SpuStore(void* pDmaData) const
{
	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));
}

}
