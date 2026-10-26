////////////////////////////////////////////////////////////
// Rigid Body 
////////////////////////////////////////////////////////////

#pragma once

#include "physics.h"
#include "racingphysics\collisions\collisionobjects.h"

namespace RacingPhysics
{

// Has a rotation matrix and an angular momentum, but no mass or position.
class Rotor
{
public:
	Rotor() {}

	// initialisation
	void Initialise() {Clear(&m_state.angularMomentum); m_state.rotationMatrix.SetIdentityMatrix();}
	void SetMassVariables(float mi) {m_MI = FloatVR(mi);}
	void SetAngularMomentum(const Vector3R& m) {m_state.angularMomentum = m;}
	void SetAngularVelocity(const Vector3R& v) {m_state.angularMomentum = v * m_MI;}
	void Stop() {Clear(&m_state.angularMomentum);}
	void SetRotation(const Quaternion& rot) {m_state.rotationMatrix.SetRotation(rot);}
	void SetRotation(const RotationMatrix& rot) {m_state.rotationMatrix.SetRotation(rot);}

	// information 
	__forceinline const Vector3R& AngularMomentum() const {return m_state.angularMomentum;}
	__forceinline const Vector3R AngularVelocity() const {return m_state.angularMomentum / m_MI;}
	__forceinline FloatVR MI() const {return m_MI;}
	__forceinline const RotationMatrix& GetRotationMatrix() const {return m_state.rotationMatrix;}

	struct Derivative
	{
		Vector3R angularVelocity;
		Vector3R torque;
	};

	// update
	void Rotate(const Vector3R& v) {m_state.rotationMatrix.Rotate(v);}
	void EvaluateDerivative(Derivative& d) const {Clear(&d.torque); d.angularVelocity = AngularVelocity();}
	void ApplyInstantTorque(const Vector3R& instantTorque) {m_state.angularMomentum += instantTorque;}
	void UpdateState(const Derivative& d, float time);
	void UpdateVelocities(const Derivative& d, float time) {m_state.angularMomentum += d.torque * time;}
	void UpdatePositions(const Derivative& d, float time);

	// state variables
	struct State
	{
		RotationMatrix rotationMatrix;
		Vector3R angularMomentum;	
	};

	void GetState(State& s) const {s = m_state;}
	void RestoreState(const State& s) {m_state = s;}

private:
	State m_state;

	// constants
	FloatVR m_MI;
};

inline Rotor::Derivative CalculateRK4(const Rotor::Derivative& a, const Rotor::Derivative& b, const Rotor::Derivative& c, const Rotor::Derivative& d)
{
	Rotor::Derivative rk4;
	rk4.angularVelocity = RK4(a.angularVelocity, b.angularVelocity, c.angularVelocity, d.angularVelocity);
	rk4.torque = RK4(a.torque, b.torque, c.torque, d.torque);
	return rk4;
}

inline void Clear(Rotor::Derivative& d)
{
	Clear(&d.torque);
	Clear(&d.angularVelocity);
}

inline void Rotor::UpdateState(const Rotor::Derivative& d, float time)
{
	Rotate(d.angularVelocity * time);
	m_state.angularMomentum += d.torque * time;
}

// Update the axis using force and torque from the derivative,
// ignoring state velocities.
inline void Rotor::UpdatePositions(const Derivative& d, float dt)
{
	// calculate velocities
	Vector3R angularMomentum = d.torque * dt;
	Vector3R angularVelocity = angularMomentum / m_MI;

	// update position variables
	Rotate(angularVelocity * dt);
}

/////////////////////////////////////////////////////////////////////////////

class RigidBody
{
public:
	__forceinline RigidBody() {}

	// initialisation
	void Initialise(COObject* pCollisionObject);
	void SetMassVariables(float mass, const Vector3R& mi, const Point3R& cofG);
	void Translate(const Vector3R& d) {m_state.axis.Translate(d); ClearTransformState();}
	void SetPosition(const Point3R& p) {m_state.axis.SetPosition(p); ClearTransformState();}
	void SetDirection(const Normal3R& dir) {m_state.axis.LookAt(Point3R(m_state.axis.Position() + (Vector3R&) dir)); ClearTransformState();}
	void SetAxis(const Axis& a) {m_state.axis.Set(a); ClearTransformState();}
	void SetRotation(const Quaternion& rot) {m_state.axis.SetRotation(rot); ClearTransformState();}
	void SetRotation(const RotationMatrix& rot) {m_state.axis.SetRotation(rot); ClearTransformState();}
	void SetVelocity(const Vector3R& v) {m_state.velocity = v; m_state.momentum = m_state.velocity * m_mass;}
	void SetVelocityXZ(const Vector3R& v) {m_state.velocity.X(v.X()); m_state.velocity.Z(v.Z()); m_state.momentum = m_state.velocity * m_mass;}
	void SetMomentum(const Vector3R& m) {m_state.momentum = m; m_state.velocity = m_state.momentum * m_inverseMass;}
	void SetAngularMomentum(const Vector3R& m);
	void SetAngularVelocity(const Vector3R& v);

	// information
	__forceinline const COObject* CollisionObject() const {return m_pCollisionObject;}
	__forceinline COObject* CollisionObject() {return m_pCollisionObject;}
	__forceinline const Point3R& Position() const {return m_state.axis.Position();}
	__forceinline Point3R CofG() const {return m_state.global_CofG;}
	__forceinline Point3R CofG_Local() const {return m_CofG;}
	__forceinline const Axis& GetAxis() const {return m_state.axis;}
	__forceinline const Vector3R& Momentum() const {return m_state.momentum;}   
	__forceinline const Vector3R& AngularMomentum() const {return m_state.angularMomentum;}
	__forceinline const Vector3R& Velocity() const {return m_state.velocity;}
	__forceinline const Vector3R& AngularVelocity() const {return m_state.angularVelocity;}
	__forceinline FloatVR Mass() const {return m_mass;}
	__forceinline FloatVR InverseMass() const {return m_inverseMass;}
	__forceinline const Matrix33R& InverseMI() const {return m_inverseMI;}
	__forceinline const Matrix33R& MI() const {return m_MI;}
	//__forceinline bool Stationary() const {return stationary;}

	Vector3R PointVelocity(const Point3R& p) const;
	float Energy() const;

	FloatVR CalculateImpulse(const Point3R& p, const Normal3R& n, FloatVR finalSpeed, FloatVR torqueFactor) const;
	FloatVR CalculateImpulse(const Point3R& p, const Normal3R& n, const RigidBody& other, FloatVR finalSpeed) const;
	FloatVR CalculateImpulse(const Point3R& p, const Normal3R& n, const RigidBody& other, FloatVR finalSpeed, FloatVR torqueFactor, FloatVR f1, FloatVR f2) const;

	float CalculateInstantTorque(const Normal3R& n, const RigidBody& other, float finalAngularVelocity) const;
	float CalculateInstantTorque(const Normal3R& n, float finalAngularVelocity) const;

	const Matrix33R& InverseMIGlobal() const; // cache transformed inverseMI matrix

	struct Derivative : public Rotor::Derivative
	{
		Vector3R velocity;
		Vector3R force;
	};

	// standard update routines
	void EvaluateDerivative(Derivative& d) const;
	void ApplyForceAndTorque(const Vector3R& force, const Vector3R& torque, float time);
	void ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor);
	void ApplyImpulse(const Point3R& p, const Vector3R& impulse);
	void ApplyInstantTorque(const Vector3R& instantTorque);
	void UpdatePositions(float dt);
	void UpdatePositions(const Derivative& d, float dt);
	void ApplyGravity(float time);
	void UpdateState(const Derivative& d, float time);
	void UpdateVelocities(const Derivative& d, float time);
	void Rotate(const Vector3R& v);
	
	// useful for constraints
	void Push(const Point3R& p, const Vector3R& v) {Push(m_state.axis, p, v); ClearTransformState();}
	void DampVelocity(float f);
	void DampRotation(float f);
	void StopVelocity();
	void StopVelocityXZ();
	void StopRotation();
	void Stop();
	void ConstrainAxisRotationY(float y);
	float ConstrainAxisY(float y, float f);
	//void Activate() {stationary = false;};

	// state variables, all in world coordinates
	struct State
	{
		Axis axis;
		Vector3R momentum;
		Vector3R angularMomentum;	

		// dependent variables
		Vector3R velocity;
		Vector3R angularVelocity;
		mutable Matrix33R globalInverseMI;
		Point3R global_CofG;
	};

	void GetState(State& s) const {s = m_state;}
	void RestoreState(const State& s) {m_state = s;}

	static void Push(Axis& axis, const Point3R& p, const Vector3R& v);

private:
	void ClearTransformState();
	void ClearTransformState_PreserveAV();

	State m_state;

	// constants
	Point3R m_CofG;
	Matrix33R m_MI;
	Matrix33R m_inverseMI;
	float m_mass;
	float m_inverseMass;

protected:
	// collisions
	COObject* m_pCollisionObject;
	//bool stationary; // for optimisation (cones)
};

/////////////////////////////////////////////////////

inline bool operator == (const RigidBody::Derivative& d1, const RigidBody::Derivative& d2)
{
	if (d1.velocity != d2.velocity) return false;
	if (d1.angularVelocity != d2.angularVelocity) return false;
	if (d1.force != d2.force) return false;
	if (d1.torque != d2.torque) return false;
	return true;
}

///////////////////////////////////////////////////////

inline void RigidBody::Rotate(const Vector3R& v)
{
	RP_ASSERT(m_state.axis.DebugTest());
	m_state.axis.Rotate(v);
	ClearTransformState(); // clear this whenever the axis changes
	RP_ASSERT(m_state.axis.DebugTest());
}

inline void RigidBody::EvaluateDerivative(RigidBody::Derivative& d) const
{
	d.velocity = m_state.velocity;
	d.angularVelocity = m_state.angularVelocity;
	Clear(&d.force);
	Clear(&d.torque);
}

inline void Clear(RigidBody::Derivative& d)
{
	Clear(&d.velocity);
	Clear(&d.angularVelocity);
	Clear(&d.force);
	Clear(&d.torque);
}

inline void RigidBody::ApplyGravity(float time) 
{
	m_state.momentum.Y(m_state.momentum.Y() - PhysicsConstants::GRAVITY * m_mass * time);
	m_state.velocity = m_state.momentum * m_inverseMass;
}

inline void RigidBody::SetAngularMomentum(const Vector3R& m) 
{
	m_state.angularMomentum = m; 
	m_state.angularVelocity = InverseMIGlobal() * m_state.angularMomentum;
}

inline void RigidBody::SetAngularVelocity(const Vector3R& v) 
{
	m_state.angularVelocity = v; 
	m_state.angularMomentum = m_state.axis.Global(m_MI) * m_state.angularVelocity;
}

inline const Matrix33R& RigidBody::InverseMIGlobal() const 
{
	return m_state.globalInverseMI;
}

// velocity of any point on the body including angular component 
inline Vector3R RigidBody::PointVelocity(const Point3R& p) const 
{
	RP_ASSERT(Maths::Finite(p));

	Vector3R pn = p - CofG();
	Vector3R v = m_state.velocity + (m_state.angularVelocity ^ pn);

	RP_ASSERT(Maths::Finite(v));

	return v;
}

inline RigidBody::Derivative CalculateRK4(const RigidBody::Derivative& a, const RigidBody::Derivative& b, const RigidBody::Derivative& c, const RigidBody::Derivative& d)
{
	RigidBody::Derivative rk4;
	rk4.velocity = RK4(a.velocity, b.velocity, c.velocity, d.velocity);
	rk4.angularVelocity = RK4(a.angularVelocity, b.angularVelocity, c.angularVelocity, d.angularVelocity);
	rk4.force = RK4(a.force, b.force, c.force, d.force);
	rk4.torque = RK4(a.torque, b.torque, c.torque, d.torque);
	return rk4;
}

// instantaneously change momenta and velocities
__forceinline void RigidBody::ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor) 
{
	RP_ASSERT(Maths::Finite(impulse));
	RP_ASSERT(Maths::Abs(m_state.angularVelocity.Y()) < 1000.f);

	m_state.momentum += impulse;
	m_state.angularMomentum += ((p - CofG()) ^ impulse) * torqueFactor;
	m_state.velocity = m_state.momentum * m_inverseMass;
	m_state.angularVelocity = (InverseMIGlobal() * m_state.angularMomentum);

	RP_ASSERT(Maths::Abs(m_state.angularVelocity.Y()) < 1000.f);
}

__forceinline void RigidBody::ApplyImpulse(const Point3R& p, const Vector3R& impulse) 
{
	RP_ASSERT(Maths::Finite(impulse));
	RP_ASSERT(Maths::Abs(m_state.angularVelocity.Y()) < 1000.f);

	m_state.momentum += impulse;
	m_state.angularMomentum += ((p - CofG()) ^ impulse);
	m_state.velocity = m_state.momentum * m_inverseMass;
	m_state.angularVelocity = (InverseMIGlobal() * m_state.angularMomentum);

	RP_ASSERT(Maths::Abs(m_state.angularVelocity.Y()) < 1000.f);
}

}
