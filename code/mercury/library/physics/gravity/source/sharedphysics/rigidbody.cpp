#include "StdAfx.h"
#include "gravity/sharedphysics/RigidBody.h"

namespace SharedPhysics
{

#ifndef CFG_SPEED
#pragma optimize("", off)

bool C_RigidBody::ValidState()
{

	BIZ_ASSERT(m_Position.Finite());
	if(!m_Position.Finite())
	{
		return false;
	}
	BIZ_ASSERT(m_Orientation.Finite());
	if(!m_Orientation.Finite())
	{
		return false;
	}
	BIZ_ASSERT(m_LinearVelocity.Finite());
	if(!m_LinearVelocity.Finite())
	{
		return false;
	}
	BIZ_ASSERT(m_AngularVelocity.Finite());
	if(!m_AngularVelocity.Finite())
	{
		return false;
	}
	BIZ_ASSERT(m_Force.Finite());
	if(!m_Force.Finite())
	{
		return false;
	}
	BIZ_ASSERT(m_Torque.Finite());
	if(!m_Torque.Finite())
	{
		return false;
	}

	return true;
}
#pragma optimize("", on)
#endif

void C_RigidBody::CalculateBVector(const Maths::HF32 PCREF time)
{
	const Maths::HVec3 vel(m_LinearVelocity + m_Force * time * ReciprocalMass());
	const Maths::HVec3 ang(m_AngularVelocity + (m_Torque * m_InverseInertiaTensor) * time);
	const Maths::Vector3 nvel(vel);
	const Maths::Vector3 nang(ang);
	m_BVector = Maths::MakeVector6(nvel, nang);
}

void C_RigidBody::CopyVelocityToBVector()
{
	const Maths::Vector3 nvel(m_LinearVelocity);
	const Maths::Vector3 nang(m_AngularVelocity);
	m_BVector = Maths::MakeVector6(nvel, nang);
}

const Maths::HMat33 C_RigidBody::GetScaledInverseInertiaTensor(const Maths::HVec3 PCREF scale) const
{
	Maths::HMat33 scaledIBodyInverse(Maths::UNDEFINED);
	/*scaledIBodyInverse.AX(m_InverseInertiaTensor.AX() * scale.X());
	scaledIBodyInverse.AY(m_InverseInertiaTensor.AY() * scale.Y());
	scaledIBodyInverse.AZ(m_InverseInertiaTensor.AZ() * scale.Z());*/
	scaledIBodyInverse.AX(m_IBodyInverse.AX() * scale.X());
	scaledIBodyInverse.AY(m_IBodyInverse.AY() * scale.Y());
	scaledIBodyInverse.AZ(m_IBodyInverse.AZ() * scale.Z());
	const Maths::HMat33 transRot(m_OrientationMatrix.Transposed());
	return transRot * scaledIBodyInverse * m_OrientationMatrix;
	//return m_InverseInertiaTensor;
}
 
NOALIAS void C_RigidBody::StoreLinearVelocity(const Maths::HVec3 PCREF linearVelocity)
{
	const Maths::HVec3 linMom(linearVelocity * Mass());
	const Maths::HVec3 keLin(linearVelocity * linMom);
	m_LinearVelocity = linearVelocity;
	m_LinearMomentum = linMom;
}

NOALIAS void C_RigidBody::StoreAngularVelocity(const Maths::HVec3 PCREF angularVelocity)
{
	const Maths::HVec3 angMom(angularVelocity * m_InertiaTensor);
	const Maths::HVec3 keAng(angularVelocity * angMom );
	m_AngularVelocity = angularVelocity;
	m_AngularMomentum = angMom;
}

NOALIAS void C_RigidBody::StoreVelocitiesAndPosition(const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
													 const Maths::C_Quaternion PCREF orientationQuat, const Maths::HVec3 PCREF position)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	BIZ_ASSERT(ValidState());

	const Maths::HVec3 linMom(linearVelocity * Mass());
	const Maths::HVec3 keLin(linearVelocity * linMom); // 1/2 m (v ^ 2) || linMom = mv, so just multiply by 1/2v to save one multiply

	const Maths::HMat33 orientation(orientationQuat);
	const Maths::HMat33 transRot(orientation.Transposed());
	const Maths::HMat33 inverseInertiaTensor( transRot * m_IBodyInverse * orientation);
	const Maths::HMat33 inertiaTensor( transRot * m_IBody * orientation );

	
	const Maths::HVec3 angMom(angularVelocity * inertiaTensor);

	//Calculate kinetic energy for this body...
	const Maths::HVec3 keAng(angularVelocity * angMom ); // 1/2 I (a ^ 2) || angMom = Ia, so just multiply by 1/2a to save a vector-matrix multiply. Hope this one is right
	//Removed the half in both equations - just multiply by half at the end - save one multiply


	//Decide when to pick -ve quaternion!
	
	
	//const Maths::HMat33 normalised(orientation.NormalisedZY());
	m_Orientation = orientationQuat;
	m_OrientationMatrix = orientation; 
	m_KineticEnergy = (keLin + keAng) * half;
	m_InverseInertiaTensor = inverseInertiaTensor;
	m_InertiaTensor = inertiaTensor;
	m_Position = position;
	m_LinearVelocity = linearVelocity;
	m_AngularVelocity = angularVelocity;
	m_LinearMomentum = linMom;
	m_AngularMomentum = angMom;
	m_CenterOfMass = m_CenterOfMassOffset * orientation + position;
	BIZ_ASSERT(ValidState());
}
 

void C_RigidBody::Apply( const C_RigidBodyState& derivative )
{
	BIZ_ASSERT(ValidState());
	const Maths::HVec3 oldPosition(m_Position);
	const Maths::C_Quaternion oldOrientation(m_Orientation);
	const Maths::HVec3 oldLinearVelocity(m_LinearVelocity);
	const Maths::HVec3 oldAngularVelocity(m_AngularVelocity);

	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	const Maths::HVecW oneW(Maths::Constants<Maths::HF32>::One());
	const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());

	const Maths::HVec3 pos (m_Position + derivative.m_PositionDt);
	//const Maths::C_Quaternion newOrientation((Maths::C_Quaternion((derivative.m_OrientationDt) * half, oneW)) * m_Orientation);
	const Maths::C_Quaternion newOrientation(derivative.m_OrientationDt + m_Orientation);
	const Maths::C_Quaternion orientation(newOrientation.Normalised());

	const Maths::HVec3 linMom((m_LinearMomentum + derivative.m_LinearMomentumDt) * LinearDamping());
	const Maths::HVec3 angMom((m_AngularMomentum + derivative.m_AngularMomentumDt) * AngularDamping());

	//const Maths::HVec3 linMom((m_LinearMomentum + derivative.m_LinearMomentumDt) );
	//const Maths::HVec3 angMom((m_AngularMomentum + derivative.m_AngularMomentumDt));

	m_OrientationMatrix = Maths::HMat33( orientation );
	const Maths::HMat33 transRot(m_OrientationMatrix.Transposed());
	const Maths::HMat33 inverseInertiaTensor( transRot * m_IBodyInverse * m_OrientationMatrix);
	const Maths::HMat33 inertiaTensor( transRot * m_IBody * m_OrientationMatrix );

	/*const Maths::HMat33 inverseInertiaTensor( m_OrientationMatrix * m_IBodyInverse * transRot);
	const Maths::HMat33 inertiaTensor( m_OrientationMatrix * m_IBody * transRot);*/

	
	/*const Maths::HVec3 linVel(linMom * m_ReciprocalMass);
	const Maths::HVec3 angVel(angMom * inverseInertiaTensor);

	const Maths::HVec3 maxLin(m_MaxLinearSpeed);
	const Maths::HVec3 maxAng(m_MaxAngularSpeed);*/

	/*Maths::Vector3 linVel((Maths::HVec3)(linMom * m_ReciprocalMass));
	Maths::Vector3 angVel((Maths::HVec3)(angMom * inverseInertiaTensor));*/
	/*Maths::LimitLength3(&linVel.m_V, MakeVF32(m_MaxLinearSpeed));
	Maths::LimitLength3(&angVel.m_V, MakeVF32(m_MaxAngularSpeed)); */
	
	const Maths::HVec3 newVel(linMom * ReciprocalMass());//Maths::Min(maxLin, Maths::Max(-maxLin, linVel)));
	const Maths::HVec3 newAng(angMom * inverseInertiaTensor);//Maths::Min(maxAng, Maths::Max(-maxAng, angVel)));

	m_LinearMomentum = linMom; //newVel * m_Mass;//linMom;
	m_AngularMomentum = angMom; //newAng * inertiaTensor;//angMom;

	//Calculate kinetic energy for this body...
	const Maths::HVec3 keLin(newVel * m_LinearMomentum); // 1/2 m (v ^ 2) || linMom = mv, so just multiply by 1/2v to save one multiply
	const Maths::HVec3 keAng(newAng * m_AngularMomentum ); // 1/2 I (a ^ 2) || angMom = Ia, so just multiply by 1/2a to save a vector-matrix multiply. Hope this one is right
	//Removed the half in both equations - just multiply by half at the end - save one multiply

	//Write back this to the rigid body
	m_KineticEnergy = (keLin + keAng) * half;
	m_Position = pos;
	m_Orientation = orientation;
	m_InverseInertiaTensor = inverseInertiaTensor;
	m_InertiaTensor = inertiaTensor;
	m_LinearVelocity = newVel;
	m_AngularVelocity = newAng;
	m_CenterOfMass = m_CenterOfMassOffset * m_OrientationMatrix + m_Position;
	if(!ValidStateVector())
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		StoreVelocitiesAndPosition(oldLinearVelocity, oldAngularVelocity, oldOrientation, oldPosition);
	}
}


void C_RigidBody::ClampLinearAndAngularVelocity(const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxRotation)
{
	Maths::Vector3 linVel(m_LinearVelocity);
	Maths::Vector3 angVel(m_AngularVelocity);
	Maths::LimitLength3(&linVel.m_V, MakeVF32(maxSpeed));
	Maths::LimitLength3(&angVel.m_V, MakeVF32(maxRotation));
	

	const Maths::HVec3 linearVelocity(linVel);
	const Maths::HVec3 angularVelocity(angVel);

	m_LinearVelocity = linearVelocity;
	m_AngularVelocity = angularVelocity;
	
	m_LinearMomentum = linearVelocity * Mass();
	m_AngularMomentum = angularVelocity * m_InertiaTensor;
}

void C_RigidBody::Damp(const Maths::HF32 PCREF linearDamp, const Maths::HF32 PCREF angularDamp)
{
	m_LinearVelocity *= linearDamp;
	m_LinearMomentum *= linearDamp;
	m_AngularVelocity *= angularDamp;
	m_AngularMomentum *= angularDamp;
}

NOALIAS void C_RigidBody::Step(const Maths::C_TimeS PCREF time, C_RigidBodyState& result) const
{
	using namespace Maths;
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	//result.m_PositionDt = m_LinearVelocity - ((m_CenterOfMassOffset * m_OrientationMatrix)^ m_AngularVelocity);
	result.m_PositionDt = m_LinearVelocity + ((m_Position - m_CenterOfMass)^ m_AngularVelocity);
	//const Vector4 angVel(MakeVector4(Maths::HVec4(m_AngularVelocity)));
	//const Vector4 maxLength(MakeVector4(Maths::Const<VF32>::TwoPi()));
	const Maths::HVec3 newAngVel(m_AngularVelocity);//Maths::Mod(angVel, maxLength).m_V);
	result.m_OrientationDt = Maths::C_Quaternion(newAngVel * half) * m_Orientation;

	//NB - we assume a low Reynold's number to avoid a sqrt!
	//result.m_LinearMomentumDt = m_Force - (m_LinearVelocity * Magnitude(linVel) * LinearDrag());
	result.m_LinearMomentumDt = m_Force - (m_LinearVelocity * LinearDrag());
	result.m_AngularMomentumDt = m_Torque - (m_AngularVelocity * AngularDrag());
}

NOALIAS void C_RigidBody::Step(const Maths::C_TimeS PCREF time, const C_RigidBodyState& initialDV, C_RigidBodyState& result, const Maths::HF32 PCREF qScale) const
{
	using namespace Maths;
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	const Maths::HVec3 newAngVel(Maths::HVec4(m_AngularVelocity + (initialDV.m_AngularMomentumDt *m_InverseInertiaTensor * qScale)));
	//const Vector4 angVel(MakeVector4(_angVel));
	//const Vector4 maxLength(MakeVector4(Maths::Const<VF32>::TwoPi()));
	//const Maths::HVec3 newAngVel(Maths::Mod(angVel, maxLength).m_V);
	const Maths::HVec3 linVel(m_LinearVelocity + (initialDV.m_LinearMomentumDt * ReciprocalMass() * qScale));
	result.m_PositionDt = linVel + ((m_Position - m_CenterOfMass) ^ m_AngularVelocity);
	result.m_OrientationDt = Maths::C_Quaternion(newAngVel * half) * m_Orientation;
//	result.m_LinearMomentumDt = m_Force - (linVel * Magnitude(linVel) * LinearDrag());
	result.m_LinearMomentumDt = m_Force - (linVel * LinearDrag());
	result.m_AngularMomentumDt = m_Torque - (newAngVel * AngularDrag());
}


NOALIAS void C_RigidBody::StoreVelocitiesAndPosition(const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
													 const Maths::HMat33 PCREF orientation, const Maths::HVec3 PCREF position)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	BIZ_ASSERT(ValidState());
	const Maths::HVec3 linMom(linearVelocity * Mass());
	const Maths::HVec3 keLin(linearVelocity * linMom); // 1/2 m (v ^ 2) || linMom = mv, so just multiply by 1/2v to save one multiply

	const Maths::HMat33 transRot(orientation.Transposed());
	const Maths::HMat33 inverseInertiaTensor( transRot * m_IBodyInverse * orientation);
	const Maths::HMat33 inertiaTensor( transRot * m_IBody * orientation );

	
	const Maths::HVec3 angMom(angularVelocity * inertiaTensor);

	//Calculate kinetic energy for this body...
	const Maths::HVec3 keAng(angularVelocity * angMom ); // 1/2 I (a ^ 2) || angMom = Ia, so just multiply by 1/2a to save a vector-matrix multiply. Hope this one is right
	//Removed the half in both equations - just multiply by half at the end - save one multiply


	//Decide when to pick -ve quaternion!
	//const Maths::HMat33 normalised(orientation.NormalisedZY());
	m_Orientation = Maths::C_Quaternion( orientation );//normalised );
	//Maths::HMat33 convertedMatrix( m_Orientation );
	/*const Maths::C_Quaternion quat(m_Orientation.Imag(), zero);
	const Maths::HF32 dot(quat | m_Orientation);*/
	//m_Orientation = Maths::Select(m_Orientation.Real().W() < zero, -m_Orientation, m_Orientation);
	//Maths::C_Quaterion temp(Maths::C_Quaterion(orientation));
	m_OrientationMatrix = orientation; ///*normalised;*/Maths::HMat33(m_Orientation);
	//m_OrientationMatrix = Maths::HMat33(m_Orientation);
	//Write back this to the rigid body
	m_KineticEnergy = (keLin + keAng) * half;
	m_InverseInertiaTensor = inverseInertiaTensor;
	m_InertiaTensor = inertiaTensor;
	m_Position = position;
	m_LinearVelocity = linearVelocity;
	m_AngularVelocity = angularVelocity;
	m_LinearMomentum = linMom;
	m_AngularMomentum = angMom;
	m_CenterOfMass = m_CenterOfMassOffset * orientation + position;
	BIZ_ASSERT(ValidState());
}

}
