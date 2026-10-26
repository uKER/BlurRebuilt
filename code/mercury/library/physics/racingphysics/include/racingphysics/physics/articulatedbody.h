#pragma once

#include "btl/containers/vector.h"
#include "rigidbody.h"
#include "surfaces.h"

namespace RacingPhysics
{

class ArticulatedBody
{
public:
	ArticulatedBody() 
	{
		m_dd.derivativeTimeOffset = 0.f;
		m_dd.frameTime = 0.f;
		m_dd.m_NumIntegrationSteps = 4;
	}

	void Initialise(int nRigidBodyParts, int nRotorParts);
	void Integrate(float dt);

	////////////////////////////////////////////////

	const static int MAX_RIGID_BODIES = 5;
	const static int MAX_ROTORS = 5;
	const static int MAX_PARTS = 6;

	struct State
	{
		Btl::Vector<RigidBody::State, Btl::VectorParameters_Static<MAX_RIGID_BODIES> > parts;
		Btl::Vector<Rotor::State, Btl::VectorParameters_Static<MAX_ROTORS> > rotors;
	};

	struct Derivative
	{
		Derivative(int n=0) {parts.Resize(n);}

		Btl::Vector<RigidBody::Derivative, Btl::VectorParameters_Static<MAX_PARTS> > parts;
	};

	////////////////////////////////////////////////
	// apply forces to derivatives

	void OverrideNumIntegrationSteps(S32 numIntegrationSteps);

	void ApplyForce(Derivative& d, int b1, int b2, const Vector3R& force, const Point3R& position) const
	{
		RP_ASSERT(Maths::Finite(force));
		d.parts[b1].force += force;
		d.parts[b2].force -= force;

		d.parts[b1].torque += (position - m_dd.rigidBodies[b1].CofG()) ^ force;
		d.parts[b2].torque -= (position - m_dd.rigidBodies[b2].CofG()) ^ force;
	}

	void ApplyTorque(Derivative& d, int b1, int b2, const Vector3R& torque) const
	{
		RP_ASSERT(Maths::Finite(torque));
		d.parts[b1].torque += torque;
		d.parts[b2].torque -= torque;
	}

	void ApplyExternalTorque(Derivative& d, int b1, const Vector3R& torque) const
	{
		RP_ASSERT(Maths::Finite(torque));
		d.parts[b1].torque += torque;
	}

	void ApplyExternalForce(Derivative& d, int b1, const Vector3R& force, const Point3R& position) const
	{
		RP_ASSERT(Maths::Finite(force));
		d.parts[b1].force += force;
		d.parts[b1].torque += (position - m_dd.rigidBodies[b1].CofG()) ^ force;
	}

	void ApplyGravity(Derivative& d, int bi) const
	{
		d.parts[bi].force -= Maths::Constants<Normal3R>::Y() * (PhysicsConstants::GRAVITY * GetRigidBody(bi).Mass());
	}

#ifndef CFG_SPEED
	bool DebugTest() const;
#endif

protected:
	///////////////////////////////////////////////
	// integration

	void GetState(State& state) const;
	void RestoreState(const State& state);

	void EvaluateDerivative(Derivative& output, const Derivative& previous, float dt);

	Derivative CalculateRK4(const Derivative& a, const Derivative& b, const Derivative& c, const Derivative& d);
	void UpdateState(const Derivative& derivative, float time);
	void UpdateVelocities(const Derivative& derivative, float time);

	virtual void EvaluateDerivative(Derivative& output) = 0;
	virtual void BeginIntegrate() = 0;
	virtual void EndIntegrate() = 0;
	virtual void GetIntegrationType(int& integrationType, int& nSteps) = 0;
	virtual float ConstrainInternal() = 0;

	float TimeStep() const {return m_dd.timeStep;}
	float FrameTime() const {return m_dd.frameTime;}
	float DerivativeTimeOffset() const {return m_dd.derivativeTimeOffset;}
	const float& AccumulationFactor() const {return m_dd.accumulationFactor;}
	int StepIndex() const {RP_ASSERT(m_dd.stepIndex >= 0); return m_dd.stepIndex;}

	void Stop();

	void DampVelocity(float f);
	void SetVelocity(const Vector3R& v);

	//////////////////////////////////////////////

	void CalculateMass(); // call when mass changes
	FloatVR Mass() const {return m_dd.mass;}
	Point3R CofG() const;

	int NumRigidBodies() const {return m_dd.rigidBodies.Size();}
	const RigidBody& GetRigidBody(int i) const {return m_dd.rigidBodies[i];}
	RigidBody& GetRigidBody(int i) {return m_dd.rigidBodies[i];}

	int NumRotors() const {return m_dd.rotors.Size();}
	const Rotor& GetRotor(int i) const {return m_dd.rotors[i];}
	Rotor& GetRotor(int i) {return m_dd.rotors[i];}

	//////////////////////////////////////////////

	int GetNumParts() const {return NumRigidBodies() + NumRotors();}
	const Derivative& GetLastDerivative() const {return m_dd.lastDerivative;}

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void SpuStore(void* pMain) const;

private:
	void BeginStep(float f, int step) {m_dd.accumulationFactor = f; m_dd.stepIndex = step;}
	void EndFrame() {m_dd.stepIndex = -1;}
	void IntegrateSteps(int nIntegrationSteps, float timePerStep);

	//////////////////////////////////////////////////////////
	// Data that is directly copied between SPU and PPU code
	//////////////////////////////////////////////////////////

	struct ALIGN16 DmaData
	{
		Btl::Vector<RigidBody, Btl::VectorParameters_Static<MAX_RIGID_BODIES> > rigidBodies;
		Btl::Vector<Rotor, Btl::VectorParameters_Static<MAX_ROTORS> > rotors;
		float derivativeTimeOffset;
		float accumulationFactor;
		float timeStep;
		float frameTime;
		float mass;
		int stepIndex;
		S32 m_NumIntegrationSteps;
		Derivative lastDerivative;
	};

	DmaData m_dd;
};

// Accumulate forces over a time step, keeping a copy from the previous timestep.
template <typename T>
class Accumulator
{
public:
	void Initialise() {value = Maths::Constants<T>::Zero();}

	const T& GetValue() const {return value;}
	const T& GetAccumulatedValue() const {return accumulator;}

	void Begin() {accumulator = Maths::Constants<T>::Zero();}
	void Accumulate(const T& v) {accumulator += v;}

	void End(float time, float smoothTime) 
	{
		float f = exp(-time / smoothTime);
		value = value * f + accumulator * (1.f - f);
	}

	void End(float time, float smoothTimeUp, float smoothTimeDown) 
	{
		float smoothTime = value - accumulator > 0.f ? smoothTimeUp : smoothTimeDown;
		float f = exp(-time / smoothTime);
		value = value * f + accumulator * (1.f - f);
	}

	void End() {value = accumulator;}

	void SetValue(const T& f) {value = f;}

private:
	T value;
	T accumulator;
};

}
