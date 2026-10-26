//#include "gravity/sharedphysics/RigidBody.h"

#ifndef RIGIDBODY_INL
#define RIGIDBODY_INL

#include "gravity/sharedphysics/integrator.h"


namespace SharedPhysics
{

//Note with this integration step - we defer the multiplication orientationDt by inverseInertiaTensor to
//calculate the angular velocity. This will be done at the final apply stage

inline Maths::HVec3 C_RigidBody::CenterOfRotation() const
{
	return m_CenterOfMass;
}

inline void C_RigidBody::EulerIntegrate(const SharedPhysics::C_Integrator& integrator, const Maths::C_TimeS PCREF time)
{
	integrator.EulerIntegrate <C_RigidBody, C_RigidBodyState>(*this, time);
}

inline void C_RigidBody::RK4Integrate(const SharedPhysics::C_Integrator& integrator, const Maths::C_TimeS PCREF time)
{
	integrator.RK4Integrate<C_RigidBody, C_RigidBodyState>(*this, time);
}

NOALIAS __forceinline const Maths::HMat33 C_RigidBody::GetApproxScaledInverseInertiaTensor(const Maths::HF32 PCREF scale) const
{
	return Maths::HMat33(	m_InverseInertiaTensor.AX() * scale,
							m_InverseInertiaTensor.AY() * scale,
							m_InverseInertiaTensor.AZ() * scale	);
}


};

#endif

