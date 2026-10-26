#include "StdAfx.h"
#include "gravity/solver2/constraint.h"
#include "gravity/sharedphysics/Rigidbody.h"
namespace Solver2
{
#ifndef CFG_SPEED
#pragma optimize ("", off)
#endif
	void C_SphericalConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		if(m_Scale == 0.f)
			return;
		SharedPhysics::C_RigidBody* pRigidA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = pNodeB->m_pRigidBody;

		const Maths::HVec3 anchorAW = pRigidA->m_Position + m_AnchorPoint1 * pRigidA->m_Orientation;
		const Maths::HVec3 anchorBW = pRigidB->m_Position + m_AnchorPoint2 * pRigidB->m_Orientation;
		DebugRender(pRender, anchorAW, anchorBW);
		pRender->Draw3dText(anchorAW, 0.025f, "Sperical");
		
	}

	void C_SphericalConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorAW, const Maths::HVec3 PCREF anchorBW)
	{
		pRender->Draw3dPoint(anchorAW, 0.05f, Maths::RGBA(255, 0, 0));
		pRender->Draw3dCross(anchorBW, 0.05f, Maths::RGBA(0, 0, 255));
		pRender->Draw3dLine(anchorAW, anchorBW, Maths::RGBA(255, 0, 0), Maths::RGBA(0, 0, 255));
	}


	void C_SphericalLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		
	}

	void C_SphericalLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorPointAW, const Maths::HVec3 PCREF anchorPointBW)
	{
		
	}
	

	void C_DistanceConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pRigidA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = pNodeB->m_pRigidBody;

		
		const Maths::HVec3 anchorAW =pRigidA->m_Position + m_AnchorPoint1 * pRigidA->m_Orientation;
		const Maths::HVec3 anchorBW =pRigidB->m_Position + m_AnchorPoint2 * pRigidB->m_Orientation;

		DebugRender(pRender, anchorAW, anchorBW);
		
		pRender->Draw3dText(anchorAW, 0.025f, "Distance");
	}

	void C_DistanceConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorAW, const Maths::HVec3 PCREF anchorBW)
	{
		pRender->Draw3dPoint(anchorAW, 0.05f, Maths::RGBA(255, 0, 0));
		pRender->Draw3dCross(anchorBW, 0.05f, Maths::RGBA(0, 0, 255));
		pRender->Draw3dLine(anchorAW, anchorBW, Maths::RGBA(255, 0, 0), Maths::RGBA(0, 0, 255));
	}


	void C_PrismaticConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pBodyA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pBodyB = pNodeB->m_pRigidBody;

		const Maths::HVec3 jointA(m_JointAxis1 * pBodyA->m_Orientation);
		const Maths::HVec3 jointB(m_JointAxis2 * pBodyB->m_Orientation);
		
		DebugRender(pRender, pBodyA->m_Position, jointA, pBodyB->m_Position, jointB);

		pRender->Draw3dText(pBodyA->m_Position, 0.025f, "PrismaticComponenet");

	}

	void C_PrismaticConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB)
	{
		pRender->Draw3dArrow(positionA, jointA, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));
		pRender->Draw3dArrow(positionB, jointB, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));
	}


	void C_PrismaticLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pBodyA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pBodyB = pNodeB->m_pRigidBody;

		const Maths::HVec3 jointA(m_JointAxis1 * pBodyA->m_Orientation);
		const Maths::HVec3 jointB(m_JointAxis2 * pBodyB->m_Orientation);
		
		DebugRender(pRender, pBodyA->m_Position, jointA, pBodyB->m_Position, jointB);

		pRender->Draw3dText(pBodyA->m_Position, 0.025f, "PrismaticLimitComponenet");

	}
	
	void C_PrismaticLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB)
	{
	
		pRender->Draw3dArrow(positionA, jointA, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));
		pRender->Draw3dArrow(positionB, jointB, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));

	}

	void C_CarWheelConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pBodyA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pBodyB = pNodeB->m_pRigidBody;

		//const Maths::HVec3 anchorPoint((m_OffSetA * pBodyB->m_Orientation) + pBodyA->m_Position);
		const Maths::HVec3 jointA(m_JointAxis1 * pBodyA->m_Orientation);
		const Maths::HVec3 jointB(m_JointAxis2 * pBodyB->m_Orientation);
		DebugRender(pRender, pBodyA->m_Position, jointA, pBodyB->m_Position, jointB);
		pRender->Draw3dText(pBodyA->m_Position, 0.025f, "CarWheelLimitComponenet");

	}

	void C_CarWheelConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB)
	{
		pRender->Draw3dArrow(positionA, jointA, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));
		pRender->Draw3dArrow(positionB, jointB, 0.25f, 0.02f, Maths::RGBA(0, 255, 0));
	}
	

	void C_RevoluteConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pBodyA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pBodyB = pNodeB->m_pRigidBody;

		const Maths::HVec3 hingeAxisA(m_JointAxis1 * pBodyA->m_OrientationMatrix);
		const Maths::HVec3 hingeAxisB(m_JointAxis2 * pBodyB->m_OrientationMatrix);

		DebugRender(pRender, pBodyA->m_Position, hingeAxisA,  pBodyB->m_Position, hingeAxisB);

		pRender->Draw3dText(pBodyA->m_Position, 0.025f, "RevoluteComponenet");
	}

	void C_RevoluteConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB)
	{
		pRender->Draw3dArrow(positionA, jointA, 1.f, 0.02f, Maths::RGBA(255, 0, 0));
		pRender->Draw3dArrow(positionB, jointB, 0.5f, 0.02f, Maths::RGBA(255, 255, 255));
	}

	void C_RevoluteLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
	}

	void C_RevoluteLimitConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB)
	{
	}

	void C_FixedRotationalConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		SharedPhysics::C_RigidBody* pBodyA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pBodyB = pNodeB->m_pRigidBody;

		DebugRender(pRender, Maths::HMat43(pBodyA->m_OrientationMatrix, pBodyA->m_Position), Maths::HMat43(pBodyB->m_OrientationMatrix, pBodyB->m_Position));
		pRender->Draw3dText(pBodyA->m_Position, 0.025f, "FixedComponenet");
	}

	void C_FixedRotationalConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HMat43& matA, const Maths::HMat43& matB)
	{

		pRender->Draw3dMatrix(matA, 0.15f);
		pRender->Draw3dMatrix(matB, 0.15f);
	}

	void C_TwoRotationalConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{
	}

	void C_TwoRotationalConstraint::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF positionB)
	{
	}


	void C_ProxyConstraint::DebugRender(DbUtil::I_DebugRender* pRender)
	{

		/*
			if E_ConstraintType == E_Type_SphericalLimit
					m_Constraints[0] = sphericalJoint;
					m_Constraints[1] = sphericalJointLimit;
			if E_ConstraintType == E_Type_Fix
					m_Constraints[0] = fixedRotationalJoint;
					m_Constraints[1] = sphericalJoint;
			if E_ConstraintType == E_Type_Revolute
					m_Constraints[0] = revoluteJoint;
					m_Constraints[1] = sphericalJoint;
			if E_ConstraintType == E_Type_RevoluteLimit
					m_Constraints[0] = revoluteJoint;
					m_Constraints[1] = sphericalJoint;
					m_Constraints[2] = revoluteJointLimit;
			if E_ConstraintType == E_Type_Prismatic
					m_Constraints[0] = fixedRotationalJoint;
					m_Constraints[1] = prismaticJoint;
			if E_ConstraintType == E_Type_PrismaticLimit
					m_Constraints[0] = fixedRotationalJoint;
					m_Constraints[1] = prismaticJointLimit;
			if E_ConstraintType == E_Type_CarWheel
					m_Constraints[0] = revoluteJoint;
					m_Constraints[1] = carWheelJoint;
			if E_ConstraintType == E_Type_TwoRotational
					m_Constraints[0] = sphericalJoint;
					m_Constraints[1] = twoRotational;
		*/
		
		using namespace Maths;
		SharedPhysics::C_RigidBody* pRigidA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = pNodeB->m_pRigidBody;

		//constraint off
		if(m_Scale == 0.f)
		{
			return;
		}

		switch(m_ProxyConstraintType)
		{
		case SharedPhysics::C_Constraint::E_Type_Fixed:
			{
				//fix joint include spherical joint and fixed rotational joint
				Solver2::C_FixedRotationalConstraint* fixedConstraint=(Solver2::C_FixedRotationalConstraint*)m_Constraints[0];
				Solver2::C_SphericalConstraint* sphericalConstraint = (Solver2::C_SphericalConstraint*)m_Constraints[1];
				const HVec3 anchorA(pRigidA->m_Position + sphericalConstraint->m_AnchorPoint1 * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->m_Position + sphericalConstraint->m_AnchorPoint2 * pRigidB->m_OrientationMatrix);
				const HMat43 matA(pRigidA->m_OrientationMatrix, pRigidA->m_Position);
				const HMat43 matB(pRigidB->m_OrientationMatrix, pRigidB->m_Position);
				fixedConstraint->DebugRender(pRender, matA, matB);
				sphericalConstraint->DebugRender(pRender, anchorA, anchorB);
				pRender->Draw3dText(anchorA, 0.025f, "Fixed");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
			{

				Solver2::C_SphericalConstraint* sphericalConstraint = (Solver2::C_SphericalConstraint*)m_Constraints[0];
				Solver2::C_SphericalLimitConstraint* limitConstraint= (Solver2::C_SphericalLimitConstraint*)m_Constraints[1];
				const HVec3 anchorA(pRigidA->m_Position + sphericalConstraint->m_AnchorPoint1 * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->m_Position + sphericalConstraint->m_AnchorPoint2 * pRigidB->m_OrientationMatrix);
				sphericalConstraint->DebugRender(pRender, anchorA, anchorB);
				limitConstraint->DebugRender(pRender, anchorA, anchorB);
 
				//Btl::String str;
				//str = Btl::String::Format("(%f - %f, %f - %f, %f - %f)\n", (F32)Maths::X(limitConstraint->m_LowJointLimit), 
				//	(F32)Maths::X(limitConstraint->m_HighJointLimit), (F32)Maths::Y(limitConstraint->m_LowJointLimit), (F32)Maths::Y(limitConstraint->m_HighJointLimit),
				//	(F32)Maths::Z(limitConstraint->m_LowJointLimit), (F32)Maths::Z(limitConstraint->m_HighJointLimit));
				////pRender->Draw3dText(pRigidB->m_Position, str.Chars());
				//pRender->Draw3dText(anchorA, str.Chars());
				pRender->Draw3dText(anchorA, 0.025f, "SphericalLimit");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Revolute:
			{
				Solver2::C_RevoluteConstraint* revoluteConstraint=(Solver2::C_RevoluteConstraint*)m_Constraints[0];
				Solver2::C_SphericalConstraint* sphericalConstraint=(Solver2::C_SphericalConstraint*)m_Constraints[1];
				const HVec3 anchorA(pRigidA->m_Position + sphericalConstraint->m_AnchorPoint1 * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->m_Position + sphericalConstraint->m_AnchorPoint2 * pRigidB->m_OrientationMatrix);
				const HVec3 jointA(revoluteConstraint->m_JointAxis1 * pRigidA->m_OrientationMatrix);
				const HVec3 jointB(revoluteConstraint->m_JointAxis2 * pRigidB->m_OrientationMatrix);
				revoluteConstraint->DebugRender(pRender, anchorA, jointA, anchorB, jointB);
				sphericalConstraint->DebugRender(pRender, anchorA, anchorB);
				pRender->Draw3dText(anchorA, 0.025f, "Revolute");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
			{
				Solver2::C_RevoluteConstraint* revoluteConstraint=(Solver2::C_RevoluteConstraint*)m_Constraints[0];
				Solver2::C_SphericalConstraint* sphericalConstraint=(Solver2::C_SphericalConstraint*)m_Constraints[1];
				Solver2::C_RevoluteLimitConstraint* limitConstraint=(Solver2::C_RevoluteLimitConstraint*)m_Constraints[2]; 
				const HVec3 anchorA(pRigidA->CenterOfRotation() + (sphericalConstraint->m_AnchorPoint1 - pRigidA->m_CenterOfMassOffset) * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->CenterOfRotation() + (sphericalConstraint->m_AnchorPoint2 - pRigidB->m_CenterOfMassOffset) * pRigidB->m_OrientationMatrix);
				/*const HVec3 anchorA(pRigidA->m_Position + sphericalConstraint->m_AnchorPoint1) * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->m_Position + sphericalConstraint->m_AnchorPoint2) * pRigidB->m_OrientationMatrix);*/
				const HVec3 jointA(revoluteConstraint->m_JointAxis1 * pRigidA->m_OrientationMatrix);
				const HVec3 jointB(revoluteConstraint->m_JointAxis2 * pRigidB->m_OrientationMatrix);
				revoluteConstraint->DebugRender(pRender, anchorA, jointA, anchorB, jointB);
				sphericalConstraint->DebugRender(pRender, anchorA, anchorB);
				//limitConstraint->DebugRender(pRender, anchorA, jointA, anchorB, jointB);

				/*Btl::String str;
				str = Btl::String::Format("(%f - %f)\n", (F32)limitConstraint->m_LowJointLimit, (F32)limitConstraint->m_HighJointLimit);
				pRender->Draw3dText(anchorA, str.Chars());*/
				pRender->Draw3dText(anchorA, 0.025f, "RevoluteLimit");
				pRender->Draw3dStar(pRigidA->m_Position, 0.25f, RGBA(135, 0, 135));
				pRender->Draw3dStar(pRigidB->m_Position, 0.25f, RGBA(135, 135, 0));
				pRender->Draw3dText(pRigidA->m_Position, "rigidA");
				pRender->Draw3dText(pRigidB->m_Position, "rigidB");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_CarWheel:
			{
				Solver2::C_RevoluteConstraint* revoluteConstraint = (Solver2::C_RevoluteConstraint*)m_Constraints[0];
				Solver2::C_CarWheelConstraint* carConstraint = (Solver2::C_CarWheelConstraint*)m_Constraints[1];
				const HVec3 jointA(revoluteConstraint->m_JointAxis1 * pRigidA->m_OrientationMatrix);
				const HVec3 jointB(revoluteConstraint->m_JointAxis2 * pRigidB->m_OrientationMatrix);     
				revoluteConstraint->DebugRender(pRender, pRigidA->m_Position, jointA, pRigidA->m_Position, jointB);
				pRender->Draw3dText(pRigidA->m_Position, "carWheel-revoluteA");
				pRender->Draw3dText(pRigidB->m_Position, "carWheel-revoluteB");
				carConstraint->DebugRender(pRender, pRigidA->m_Position, jointA, pRigidA->m_Position, jointB);

				/*Btl::String str;
				str = Btl::String::Format("A (%f, %f, %f, %x )\n", (F32)pRigidA->m_Position.X(),(F32)pRigidA->m_Position.Y(), (F32)pRigidA->m_Position.Z(), pRigidA);
				//pRender->Draw3dText(pRigidB->m_Position, str.Chars());
				const Maths::HVec3 positionA(pRigidA->m_Position + Maths::HVec3(0.f, 1.f, 0.f));
				pRender->Draw3dText(positionA, str.Chars());
				printf("%s\n", str.Chars());
				str = Btl::String::Format("B (%f - %f, %f, %x)\n", (F32)pRigidB->m_Position.X(),(F32)pRigidB->m_Position.Y(), (F32)pRigidB->m_Position.Z(), pRigidB);
				const Maths::HVec3 positionB(pRigidB->m_Position + Maths::HVec3(0.f, 1.f, 0.f));
				pRender->Draw3dText(positionB, str.Chars());
				printf("%s\n", str.Chars());*/
				pRender->Draw3dText(pRigidA->m_Position, 0.025f, "CarWheel");

				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Prismatic:
			{
				Solver2::C_FixedRotationalConstraint* fixConstraint = (Solver2::C_FixedRotationalConstraint*)m_Constraints[0];
				Solver2::C_PrismaticConstraint* prismaticConstraint = (Solver2::C_PrismaticConstraint*)m_Constraints[1];

				const HMat43 matA(pRigidA->m_OrientationMatrix, pRigidA->m_Position);
				const HMat43 matB(pRigidB->m_OrientationMatrix, pRigidB->m_Position);
				const HVec3 jointA(prismaticConstraint->m_JointAxis1 * pRigidA->m_OrientationMatrix);
				const HVec3 jointB(prismaticConstraint->m_JointAxis2 * pRigidB->m_OrientationMatrix);

				fixConstraint->DebugRender(pRender, matA, matB);
				prismaticConstraint->DebugRender(pRender, pRigidA->m_Position, jointA, pRigidB->m_Position, jointB);
				pRender->Draw3dText(pRigidA->m_Position, 0.025f, "Prismatic");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
			{
				Solver2::C_FixedRotationalConstraint* fixConstraint = (Solver2::C_FixedRotationalConstraint*)m_Constraints[0];
				Solver2::C_PrismaticLimitConstraint* limitConstraint = (Solver2::C_PrismaticLimitConstraint*)m_Constraints[1];
				const HMat43 matA(pRigidA->m_OrientationMatrix, pRigidA->m_Position);
				const HMat43 matB(pRigidB->m_OrientationMatrix, pRigidB->m_Position);
				const HVec3 jointA(limitConstraint->m_JointAxis1 * pRigidA->m_OrientationMatrix);
				const HVec3 jointB(limitConstraint->m_JointAxis2 * pRigidA->m_OrientationMatrix);
				fixConstraint->DebugRender(pRender, matA, matB);
				limitConstraint->DebugRender(pRender, pRigidA->m_Position, jointA, pRigidB->m_Position, jointB);
				pRender->Draw3dText(pRigidA->m_Position, 0.025f, "PrismaticLimit");
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_TwoRotational:
			{
				Solver2::C_SphericalConstraint* sphericalConstraint=(Solver2::C_SphericalConstraint*)m_Constraints[0];
				Solver2::C_TwoRotationalConstraint* rotationalConstraint = (Solver2::C_TwoRotationalConstraint*)m_Constraints[1];

				const HVec3 anchorA(pRigidA->m_Position + sphericalConstraint->m_AnchorPoint1 * pRigidA->m_OrientationMatrix);
				const HVec3 anchorB(pRigidB->m_Position + sphericalConstraint->m_AnchorPoint2 * pRigidB->m_OrientationMatrix);
				sphericalConstraint->DebugRender(pRender, anchorA, anchorB);
				rotationalConstraint->DebugRender(pRender, pRigidA->m_Position, pRigidB->m_Position);
				pRender->Draw3dText(anchorA, 0.025f, "TwoRotational");
				break;
			}
		default:
			{
				break;
			}
		}
	
	}
#ifndef CFG_SPEED
#pragma optimize ("", on)
#endif
}

