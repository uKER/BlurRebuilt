#include "StdAfx.h"
#include "gravity/sharedphysics/Spring.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/GraphNode.h"

namespace SharedPhysics
{
	Maths::HVec3 C_Spring::GetSpringForce(Maths::HVec3& bodyBTorque)
	{
		using namespace Maths;


		const HVec3 contactA=m_pRigidA->m_CenterOfMass + m_LocalContactA * m_pRigidA->m_OrientationMatrix;
		const HVec3 contactB=m_pRigidB->m_CenterOfMass + m_LocalContactB * m_pRigidB->m_OrientationMatrix;

		const C_Normal dir(contactB - contactA);

		
		const HF32 dist = (contactB - contactA).Length();
		const HF32 dif = dist - m_RestLength;
		
		const HVec3 rA(m_pRigidA->m_CenterOfMass - contactA);
		const HVec3 rB(m_pRigidB->m_CenterOfMass - contactB);
		const HVec3 velocityA(m_pRigidA->m_LinearVelocity + (rA^m_pRigidA->m_AngularVelocity));
		const HVec3 velocityB(m_pRigidB->m_LinearVelocity + (rB^m_pRigidB->m_AngularVelocity));
		const HVec3 relativeV(velocityB - velocityA);

		const HVec3 partA(m_LinearSpringCoefficient* dif * dir);
		const HVec3 partB(m_LinearDampingCoefficient*relativeV);

		const HVec3 bodyBForce = -partA - partB;
		
		
		const C_Quaternion rot1 (m_pRigidA->m_Orientation);
		const C_Quaternion rot2(rot1 * m_RotationalOffset);

		//const C_Quaternion rotateError(rot1 * !rot2);
		//const C_Quaternion rotateError(Maths::NearestDivide(rot1, rot2));		
		const C_Quaternion rotateError(Maths::NearestDivide(rot2, m_pRigidB->m_Orientation));
		const HVec3 rotationAxis(Maths::RotationAxis(rotateError));
		const Maths::HF32 angle(Maths::RotationAngle(rotateError));

		const HF32 two(Maths::Constants<HF32>::Two());
		const HF32 zero(Maths::Constants<HF32>::Zero());

		
		const Maths::C_Quaternion angVelA(m_pRigidA->m_AngularVelocity, Maths::HVecW(Maths::Constants<HF32>::One()));
		const Maths::C_Quaternion angVelB(m_pRigidB->m_AngularVelocity, Maths::HVecW(Maths::Constants<HF32>::One()));

		//const Maths::C_Quaternion difQuat(Maths::NearestDivide(angVelA, angVelB));
		const Maths::HVec3 relativeAngularVelocity(Maths::Imag(angVelA * !angVelB));
		//bodyBTorque = Maths::Imag(rotateError) * two * m_AngularSpringCoefficient + relativeAngularVelocity * m_AngularDampingCoefficient;
		bodyBTorque = rotationAxis * angle * m_AngularSpringCoefficient + relativeAngularVelocity * m_AngularDampingCoefficient;
		return bodyBForce;
	}
}

