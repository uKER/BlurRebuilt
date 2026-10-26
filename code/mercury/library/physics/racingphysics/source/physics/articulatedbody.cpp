#include "stdafx.h"
#include "racingphysics/physics/articulatedbody.h"
#include "racingphysics/physics/constraints.h"
#include "racingphysics/physics/surfaces.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off) // DEBUG_RACING_PHYSICS
#endif

////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

//using namespace Maths;

DECLARE_DEBUG_BOOL(g_useRK4, "RacingPhysics.Integration.UseRK4", true);
DECLARE_DEBUG_BOOL(g_useMidpoint, "RacingPhysics.Integration.UseMidpoint", true);
DECLARE_DEBUG_INT(g_nIntegrationSteps, "RacingPhysics.Integration.IntegrationSteps", 0, Debug::Range(0, 100), Debug::Step(1));

EXTERN_DEBUG_BOOL(g_updatePhysics);
EXTERN_DEBUG_FLOAT(g_timeStepScale);

void ArticulatedBody::OverrideNumIntegrationSteps(S32 numIntegrationSteps)
{
	m_dd.m_NumIntegrationSteps = numIntegrationSteps;
}

void ArticulatedBody::Initialise(int nRigidBodyParts, int nRotorParts)
{
	m_dd.rigidBodies.Resize(nRigidBodyParts);
	m_dd.rotors.Resize(nRotorParts);

	for (int i = 0; i < NumRigidBodies(); i++)
		m_dd.rigidBodies[i].Initialise(0);

	for (int i = 0; i < NumRotors(); i++)
		m_dd.rotors[i].Initialise();

	m_dd.timeStep = 0.f;
	m_dd.stepIndex = -1;
}

void ArticulatedBody::Stop()
{
	for (int i = 0; i < NumRigidBodies(); i++)
		m_dd.rigidBodies[i].Stop();

	for (int i = 0; i < NumRotors(); i++)
		m_dd.rotors[i].Stop();
}

void ArticulatedBody::CalculateMass()
{
	float totalMass = 0.f;

	for (int i = 0; i < NumRigidBodies(); i++)
		totalMass += m_dd.rigidBodies[i].Mass();

	m_dd.mass = totalMass;
}

Point3R ArticulatedBody::CofG() const
{
	Vector3R sum(0.f, 0.f, 0.f);
	FloatVR mass = 0.f;

	for (int i = 0; i < NumRigidBodies(); i++)
	{
		sum += MakeVector(m_dd.rigidBodies[i].CofG()) * m_dd.rigidBodies[i].Mass();
		mass += m_dd.rigidBodies[i].Mass();
	}
	
	sum /= mass;
	return MakePoint(sum);
}

void ArticulatedBody::GetState(State& state) const
{
	state.parts.Resize(NumRigidBodies());
	state.rotors.Resize(NumRotors());

	for (int i = 0; i < NumRigidBodies(); i++)
		m_dd.rigidBodies[i].GetState(state.parts[i]);

	for (int i = 0; i < NumRotors(); i++)
		m_dd.rotors[i].GetState(state.rotors[i]);
}

void ArticulatedBody::RestoreState(const State& state)
{
	for (int i = 0; i < NumRigidBodies(); i++)
		m_dd.rigidBodies[i].RestoreState(state.parts[i]);

	for (int i = 0; i < NumRotors(); i++)
		m_dd.rotors[i].RestoreState(state.rotors[i]);
}

void ArticulatedBody::EvaluateDerivative(Derivative& output, const Derivative &previous, float dt)
{
	if (!g_updatePhysics) return;

	State oldState;
	GetState(oldState);

	m_dd.derivativeTimeOffset = dt;
	UpdateState(previous, dt);
	EvaluateDerivative(output);
	m_dd.derivativeTimeOffset = 0.f;

	if (!g_updatePhysics) return;
	
	RestoreState(oldState);
}

ArticulatedBody::Derivative ArticulatedBody::CalculateRK4(const Derivative& a, const Derivative& b, const Derivative& c, const Derivative& d)
{
	int n = GetNumParts();

	Derivative rk4(n);

	for (int i = 0; i < n; i++)
		rk4.parts[i] = RacingPhysics::CalculateRK4(a.parts[i], b.parts[i], c.parts[i], d.parts[i]);

	//rk4.rotor = RacingPhysics::CalculateRK4(a.rotor, b.rotor, c.rotor, d.rotor);

	return rk4;
}

void ArticulatedBody::UpdateState(const Derivative& derivative, float dt)
{
	for (int i = 0; i < NumRigidBodies(); i++)
	{
		RP_ASSERT(GetRigidBody(i).GetAxis().DebugTest());
		RP_ASSERT(Magnitude(GetRigidBody(i).Velocity()) < 1000.f);
		RP_ASSERT(Magnitude(GetRigidBody(i).AngularVelocity()) < 1000.f);

		GetRigidBody(i).UpdateState(derivative.parts[i], dt);

		RP_ASSERT(GetRigidBody(i).GetAxis().DebugTest());
		RP_ASSERT(Magnitude(GetRigidBody(i).Velocity()) < 1000.f);
		RP_ASSERT(Magnitude(GetRigidBody(i).AngularVelocity()) < 1000.f);
	}

	int partIndex = NumRigidBodies();

	for (int i = 0; i < NumRotors(); i++, partIndex++)
	{
		RP_ASSERT(Magnitude(m_dd.rotors[i].AngularVelocity()) < 1000000.f);

		RP_ASSERT(m_dd.rotors[i].GetRotationMatrix().DebugTest());
		m_dd.rotors[i].UpdateState(derivative.parts[partIndex], dt);
		RP_ASSERT(m_dd.rotors[i].GetRotationMatrix().DebugTest());

		RP_ASSERT(Magnitude(m_dd.rotors[i].AngularVelocity()) < 1000000.f);
	}

	m_dd.lastDerivative = derivative;

	//CheckConstraints();
}

void ArticulatedBody::UpdateVelocities(const Derivative& derivative, float dt)
{
	for (int i = 0; i < NumRigidBodies(); i++)
		GetRigidBody(i).UpdateVelocities(derivative.parts[i], dt);

	int partIndex = NumRigidBodies();

	for (int i = 0; i < NumRotors(); i++, partIndex++)
		m_dd.rotors[i].UpdateVelocities(derivative.parts[partIndex], dt);
}

bool g_derivativeTest = false;

void ArticulatedBody::Integrate(float time)
{
	if (time == 0.f)
		return;

	if (!g_updatePhysics) 
		return;

	m_dd.frameTime = time;

	// One integration step per 1/60th.
	S32 integrateStepsPer60 = (g_nIntegrationSteps > 0) ? g_nIntegrationSteps : m_dd.m_NumIntegrationSteps;
	int nIntegrationSteps = Max((int)((integrateStepsPer60 * time + 0.001f) * 60.f), 1);
	float timePerStep = time / nIntegrationSteps;

	// Divide into 1, 2 or 4.
	while (nIntegrationSteps > 4)
	{
		IntegrateSteps(4, timePerStep);
		nIntegrationSteps -= 4;
	}

	if (nIntegrationSteps == 3)
	{
		IntegrateSteps(2, timePerStep);
		IntegrateSteps(1, timePerStep);
	}
	else
		IntegrateSteps(nIntegrationSteps, timePerStep);

	ConstrainInternal();
}

void ArticulatedBody::IntegrateSteps(int nIntegrationSteps, float timePerStep)
{
	RP_ASSERT(nIntegrationSteps == 1 || nIntegrationSteps == 2 || nIntegrationSteps == 4);

	int integrationType = 0; // Euler
	float time = nIntegrationSteps * timePerStep;

	// Turn multiple integration steps into higher-order integration.
	if (nIntegrationSteps == 4 && g_useRK4)
	{
		nIntegrationSteps /= 4;
		integrationType = 2; // rk4
	}
	else if (nIntegrationSteps == 2 && g_useMidpoint)
	{
		nIntegrationSteps /= 2;
		integrationType = 1; // midpoint
	}

	// Allow the vehicle to override the integration type.
	GetIntegrationType(integrationType, nIntegrationSteps);

	// Work out the time per integration.
	m_dd.timeStep = time * g_timeStepScale / nIntegrationSteps;
	int nParts = GetNumParts();

	for (int is = 0; is < nIntegrationSteps; is++)
	{
		BeginIntegrate();

		if (integrationType == 0) // euler
		{
			Derivative derivative(GetNumParts());
			BeginStep(1.f, 0); EvaluateDerivative(derivative);

			if (!g_updatePhysics) 
				return;

			UpdateState(derivative, TimeStep());
		}
		else if (integrationType == 1) // midpoint
		{
			Derivative a(nParts), b(nParts);
			BeginStep(0.f, 0); EvaluateDerivative(a);
			BeginStep(1.f, 1); EvaluateDerivative(b, a, TimeStep() * 0.5f);

			if (g_derivativeTest)
			{
				Derivative test(nParts);
				BeginStep(0.f, 1);
				EvaluateDerivative(test, a, TimeStep() * 0.5f);

				for (int i = 0; i < nParts; i++)
					if (!(test.parts[i] == b.parts[i]))
						int breakhere = 0;
			}

			if (!g_updatePhysics) 
				return;

			UpdateState(b, TimeStep());
		}
		else // rk4
		{
			Derivative a(nParts), b(nParts), c(nParts), d(nParts);
			BeginStep(1.f / 6.f, 0); 
			EvaluateDerivative(a);
			BeginStep(2.f / 6.f, 1); 
			EvaluateDerivative(b, a, TimeStep() * 0.5f);
			BeginStep(2.f / 6.f, 2); 
			EvaluateDerivative(c, b, TimeStep() * 0.5f);
			BeginStep(1.f / 6.f, 3); 
			EvaluateDerivative(d, c, TimeStep());

			if (g_derivativeTest)
			{
				Derivative test(nParts);
				BeginStep(0.f, 3);
				EvaluateDerivative(test, c, TimeStep());

				for (int i =0; i < nParts; i++)
					if (!(test.parts[i] == d.parts[i]))
						int breakhere = 0;
			}

			if (!g_updatePhysics) 
				return;

			Derivative derivative = CalculateRK4(a, b, c, d);

			if (!g_updatePhysics) 
				return;

			UpdateState(derivative, TimeStep());

			//Vector3R acceleration = derivative.parts[0].force / (m_rigidBodies[0].Mass());
			//float s = Magnitude(acceleration) / PhysicsConstants::GRAVITY;
			//DBPrint("Acceleration = %0.2f g", s);
		}
		
		EndIntegrate();
	}
}

//Debug::Float g_bikeCollisionMaxFriction("RacingPhysics.Bikes.BikeCollisionMaxFriction", 3.f, Debug::Range(0.f, 10.f), Debug::Step(0.01f));
//Debug::Float g_bikeCollisionFrictionSpeedScale("RacingPhysics.Bikes.BikeCollisionFrictionSpeedScale", 0.1f, Debug::Range(0.f, 10.f), Debug::Step(0.01f));

void ArticulatedBody::DampVelocity(float f)
{
	for (int i = 0; i < NumRigidBodies(); i++)
		GetRigidBody(i).DampVelocity(f);
}

void ArticulatedBody::SetVelocity(const Vector3R& v)
{
	RP_ASSERT(Magnitude(v) < 10000.f);

	for (int i = 0; i < NumRigidBodies(); i++)
		GetRigidBody(i).SetVelocity(v);
}

#ifndef CFG_SPEED
#pragma optimize("", off)

bool ArticulatedBody::DebugTest() const
{
	for (int i = 0; i < NumRigidBodies(); i++)
	{
		RP_ASSERT(GetRigidBody(i).GetAxis().DebugTest());
		RP_ASSERT(Magnitude(GetRigidBody(i).Velocity()) < 1000.f);
	}

	return true;
}

#pragma optimize("", on)
#endif

void ArticulatedBody::GetDmaData(void** pDmaData)
{
	*pDmaData = &m_dd;
}

void ArticulatedBody::SpuFetch(const void* pDmaData)
{
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));
}

void ArticulatedBody::SpuStore(void* pDmaData) const
{
	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));
}

}


