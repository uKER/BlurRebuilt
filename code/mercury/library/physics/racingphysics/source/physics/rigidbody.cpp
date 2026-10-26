//////////////////////////////////////////////////////////////////////////////////////////
// Rigid Body
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "racingphysics/physics/rigidbody.h"
//#include "racingphysics/collisions/collisionmesh.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

static Matrix33R CalcInverseGlobalMI(const Axis& axis, const Matrix33R& localMI)
{
	return axis.Global(localMI);
}

//////////////////////////////////////////////////////

void RigidBody::ClearTransformState() 
{
	if (m_pCollisionObject)
		m_pCollisionObject->ClearTransformState(); 

    m_state.globalInverseMI = CalcInverseGlobalMI(m_state.axis, m_inverseMI);
	m_state.global_CofG = m_state.axis.Global(m_CofG);
	m_state.angularVelocity = InverseMIGlobal() * m_state.angularMomentum;
}

void RigidBody::ClearTransformState_PreserveAV() 
{
	if (m_pCollisionObject)
		m_pCollisionObject->ClearTransformState(); 

    m_state.globalInverseMI = CalcInverseGlobalMI(m_state.axis, m_inverseMI);
	m_state.global_CofG = m_state.axis.Global(m_CofG);
	m_state.angularMomentum = m_state.axis.Global(m_MI) * m_state.angularVelocity;
}

void RigidBody::SetMassVariables(float mass, const Vector3R& mi, const Point3R& cofG)
{
	// Calculate MI of a box the size of the car
	// not done like this in the game because the size of the model may change
	//Vector3R s2(s.Y() * s.Y() + s.Z() * s.Z(),
	//				 s.vx * s.vx + s.Z() * s.Z(),
	//				 s.Y() * s.Y() + s.vx * s.X());
	//s2 /= 12.f;

	m_CofG = cofG;

	Vector3R miCorrect = mi;
	if (miCorrect.X() + miCorrect.Z() < miCorrect.Y())
		miCorrect.X(miCorrect.Y() - miCorrect.Z());

	Diagonal(&m_MI, miCorrect);
	InverseDiagonal(&m_inverseMI, miCorrect);

	//m_MI.AX(mass * (Maths::Constants<Vector3R>::X() * (cofG.X() * cofG.X() + miCorrect.X()) - cofG.X() * MakeVector(cofG)));
	//m_MI.AY(mass * (Maths::Constants<Vector3R>::Y() * (cofG.Y() * cofG.Y() + miCorrect.Y()) - cofG.Y() * MakeVector(cofG)));
	//m_MI.AZ(mass * (Maths::Constants<Vector3R>::Z() * (cofG.Z() * cofG.Z() + miCorrect.Z()) - cofG.Z() * MakeVector(cofG)));
	//Invert3x3(&m_inverseMI, m_MI);

	m_mass = FloatVR(mass);
	m_inverseMass = Maths::Constants<FloatVR>::One() / m_mass;

	ClearTransformState_PreserveAV();
}

void RigidBody::Initialise(COObject* pCollisionObject)
{
	m_state.axis.SetIdentityMatrix();
	m_state.axis.SetPosition(Point3R(0.f, 0.f, 0.f));
	//quaternionRotation.SetIdentity();

	Clear(&m_CofG);
	Clear(&m_state.momentum);
	Clear(&m_state.velocity);
	Clear(&m_state.angularMomentum);
	Clear(&m_state.angularVelocity);

	m_pCollisionObject = pCollisionObject;

	ClearTransformState();
}

// returns size of impulse required to get finalSpeed at the given point
FloatVR RigidBody::CalculateImpulse(const Point3R& p, const Normal3R& n, FloatVR finalSpeed, FloatVR torqueFactor) const 
{  
	Vector3R pn = p - CofG();
	FloatVR pointVelocity = ((m_state.velocity + (m_state.angularVelocity ^ pn)) | n) - FloatVR(finalSpeed);
  
	// calculate impulse
	FloatVR impulseScale = m_inverseMass + ((pn ^ Vector3R(InverseMIGlobal() * Vector3R(n ^ pn))) | n) * FloatVR(torqueFactor);
	return -pointVelocity / impulseScale;
}

// same as above but for collision with moving object
FloatVR RigidBody::CalculateImpulse(const Point3R& p, const Normal3R& n, const RigidBody& other, FloatVR finalSpeed, FloatVR torqueFactor, FloatVR f1, FloatVR f2) const 
{
	Vector3R ra = p - CofG();
	Vector3R rb = p - other.CofG();
	FloatVR va = ((m_state.velocity + (m_state.angularVelocity ^ ra)) | n);
	FloatVR vb = ((other.m_state.velocity + (other.m_state.angularVelocity ^ rb)) | n);
	FloatVR rv = va - vb - FloatVR(finalSpeed);

	// calculate impulse
	FloatVR impulseScale = f1 * m_inverseMass + f2 * other.m_inverseMass
								+ (((ra ^ Vector3R(InverseMIGlobal() * Vector3R(n ^ ra))) | n) * f1
								+ ((rb ^ Vector3R(other.InverseMIGlobal() * Vector3R(n ^ rb))) | n) * f2
									) * FloatVR(torqueFactor);
	return -rv / impulseScale;
}

// same as above but for collision with moving object
FloatVR RigidBody::CalculateImpulse(const Point3R& p, const Normal3R& n, const RigidBody& other, FloatVR finalSpeed) const 
{
	Vector3R ra = p - CofG();
	Vector3R rb = p - other.CofG();
	FloatVR va = ((m_state.velocity + (m_state.angularVelocity ^ ra)) | n);
	FloatVR vb = ((other.m_state.velocity + (other.m_state.angularVelocity ^ rb)) | n);
	FloatVR rv = va - vb - FloatVR(finalSpeed);

	// calculate impulse
	FloatVR impulseScale = m_inverseMass + other.m_inverseMass
								+ (((ra ^ Vector3R(InverseMIGlobal() * Vector3R(n ^ ra))) | n)
								+ ((rb ^ Vector3R(other.InverseMIGlobal() * Vector3R(n ^ rb))) | n));
	return -rv / impulseScale;
}

// return the angular momentum change along the axis to have the final angular velocity around the axis
float RigidBody::CalculateInstantTorque(const Normal3R& n, float finalAngularVelocity) const
{
	FloatVR f1 = FloatVR(finalAngularVelocity) - (m_state.angularVelocity | n);
	FloatVR f2 = (InverseMIGlobal() * n) | n;

	return f1 / f2;
}

float RigidBody::CalculateInstantTorque(const Normal3R& n, const RigidBody& other, float finalAngularVelocity) const
{
	FloatVR f1 = FloatVR(finalAngularVelocity) - (m_state.angularVelocity | n) + (other.m_state.angularVelocity | n);
	FloatVR f2 = ((InverseMIGlobal() * n) | n) + ((other.InverseMIGlobal() * n) | n);

	return f1 / f2;
}

void RigidBody::ApplyForceAndTorque(const Vector3R& force, const Vector3R& torque, float time)
{
	// calculate new momenta
	m_state.momentum += force * time;
	m_state.angularMomentum += torque * time;

	// calculate new velocities
	m_state.velocity = m_state.momentum * m_inverseMass;
	m_state.angularVelocity = (InverseMIGlobal() * m_state.angularMomentum);

	RP_ASSERT(Maths::Finite(m_state.velocity.Y()));
}

// instantaneously change angular momentum
void RigidBody::ApplyInstantTorque(const Vector3R& instantTorque) 
{
	m_state.angularMomentum += instantTorque;
	m_state.angularVelocity = (InverseMIGlobal() * m_state.angularMomentum);
	//float newpointVelocity = (velocity + (angularVelocity && pn)) || vn;
}

// moves point p distance d in direction v by rotation and translation
void RigidBody::Push(Axis& axis, const Point3R& p, const Vector3R& v)
{
	RP_ASSERT(Maths::Finite(v));
	RP_ASSERT(Maths::Finite(p));

	Vector3R pn = (p - axis.Position());
	FloatVR ps = Magnitude(pn);

	RP_ASSERT(ps > 0.f);

	// Ed: if ps == 0 i.e. p == position, this will explode.
	// position is at wheel height at the centre of the car.
	// It should be virtually impossible to hit this point, 
	// so won't fix this for now.
	Vector3R vl = pn * ((v | pn) / (ps * ps));
	Vector3R vt = (v - vl) * Maths::Constants<FloatVR>::Half();

	Vector3R r = (pn ^ vt) / (ps * ps);

	RP_ASSERT(Maths::Finite(r));

	axis.Translate(vl + vt);
	axis.Rotate(r);

	RP_ASSERT(axis.DebugTest());
}

void RigidBody::DampVelocity(float f)
{
	RP_ASSERT(f > 0.f && f < 1.f);

	m_state.velocity *= f;
	m_state.momentum *= f;
}

void RigidBody::DampRotation(float f)
{
	RP_ASSERT(f > 0.f && f < 1.f);

	m_state.angularVelocity *= f;
	m_state.angularMomentum *= f;
}

void RigidBody::StopVelocity()
{
	Clear(&m_state.velocity);
	Clear(&m_state.momentum);	
}

void RigidBody::StopVelocityXZ()
{
	m_state.velocity.X(0.f);
	m_state.velocity.Z(0.f);
	m_state.momentum.X(0.f);
	m_state.momentum.Z(0.f);
}

void RigidBody::StopRotation()
{
	Clear(&m_state.angularVelocity);
	Clear(&m_state.angularMomentum);
}

void RigidBody::Stop()
{
	Clear(&m_state.velocity);
	Clear(&m_state.momentum);	
	Clear(&m_state.angularVelocity);
	Clear(&m_state.angularMomentum);

	//stationary = true;
}

void RigidBody::ConstrainAxisRotationY(float y)
{
	float d = m_state.axis.AxisY().Y();
	if (d < y)
	{
		Vector3R a(m_state.axis.AxisY().Z(), 0.f, -m_state.axis.AxisY().X()); // axis && world y
		a = Normalised(a);
		float r = m_state.angularMomentum | a;

		if (r > 0.f) 
		{
			m_state.angularMomentum -= a * r;
			m_state.angularVelocity = (InverseMIGlobal() * m_state.angularMomentum);
		}
	}
}

float RigidBody::ConstrainAxisY(float y, float f)
{
	float d = m_state.axis.AxisY().Y();

	if (d < y)
	{
		float th = (y * y - d * d) / 2.f + 0.001f;

		Vector3R a(-m_state.axis.AxisY().Z(), 0.f, m_state.axis.AxisY().X());
		a *= (th * f);
		m_state.axis.Rotate(a);

		ClearTransformState();

		return th * f;
	}

	return 0.f;
}

float RigidBody::Energy() const 
{
	float e = (Momentum() | Velocity()) * (0.5f);
	e += (AngularMomentum() | AngularVelocity()) * (0.5f);

	return e;
}

// Update the axis using state velocities.
void RigidBody::UpdatePositions(float dt)
{
	FloatVR t = FloatVR(dt);

	// update position variables
	Rotate(m_state.angularVelocity * t);
	m_state.axis.Translate((m_state.velocity + (m_state.angularVelocity ^ (m_state.axis.Position() - CofG()))) * t);
	
	// calculate dependent variables
	ClearTransformState();
}

// Update the axis using force and torque from the derivative,
// ignoring state velocities.
void RigidBody::UpdatePositions(const Derivative& d, float dt)
{
	FloatVR t = FloatVR(dt);

	// calculate velocities
	Vector3R momentum = d.force * t;
	Vector3R angularMomentum = d.torque * t;
	Vector3R velocity = momentum * m_inverseMass;
	Vector3R angularVelocity = InverseMIGlobal() * angularMomentum;

	// update position variables
	Rotate(angularVelocity * t);
	m_state.axis.Translate((velocity + (angularVelocity ^ (m_state.axis.Position() - CofG()))) * t);

	// calculate dependent variables
	ClearTransformState();
}

// Update state velocities using force and torque from the derivative.
void RigidBody::UpdateVelocities(const Derivative& d, float dt)
{
	FloatVR t = FloatVR(dt);

	// update velocity variables
	m_state.momentum += d.force * t;
	m_state.angularMomentum += d.torque * t;
	
	// calculate dependent variables
	m_state.velocity = m_state.momentum * m_inverseMass;
	m_state.angularVelocity = InverseMIGlobal() * m_state.angularMomentum;
}

// Complete update of state using the derivative.
void RigidBody::UpdateState(const Derivative& d, float dt)
{
	FloatVR t = FloatVR(dt);

	// update position variables
	Rotate(d.angularVelocity * t);
	m_state.axis.Translate((d.velocity + (d.angularVelocity ^ (m_state.axis.Position() - CofG()))) * t);

	// update velocity variables
	m_state.momentum += d.force * t;
	m_state.angularMomentum += d.torque * t;
	
	// calculate dependent variables
	ClearTransformState();
	m_state.velocity = m_state.momentum * m_inverseMass;
}

}

