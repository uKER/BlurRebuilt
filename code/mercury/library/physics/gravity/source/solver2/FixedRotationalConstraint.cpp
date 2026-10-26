#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


namespace Solver2
{
	void C_FixedRotationalConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
	{
		jaco->m_pConstraint = m_pPPUConstraint;

		jaco->m_Baumgarte = m_Baumgarte;

		using namespace Maths;
		/*Spu::MemBaseMarker marker;
		SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
		SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode)); */

		SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
		SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;


		bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

		/*SharedPhysics::C_GraphNode* graphNodeA = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
		SharedPhysics::C_GraphNode* graphNodeB = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);*/

		const VBool scalarFlip(MakeVBool(bFlip));

		/*SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;
		jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
		jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);*/

		Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
		jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
		jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);


		const Quaternion rotationalOffset(Maths::Select(scalarFlip, !m_rotationalOffSet, m_rotationalOffSet));

		Solver2::C_FixedRotationalJacobian* fixedJ=(Solver2::C_FixedRotationalJacobian*)jaco;

		/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
		SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/

		SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
		SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;

		const Quaternion rot2(pRigid2->m_Orientation);
		const Quaternion rot1 (pRigid1->m_Orientation);


		//const Vector3 zeroVec3(Const<Vector3>::Zero());
		const VF32 two = Const<VF32>::Two();

		const Quaternion targetB( rotationalOffset * rot1);

		const Quaternion rotateError(Maths::NearestDifference(rot2, targetB));
		
		const Vector3 rotationalErr(Imag(rotateError)* two);
		 
		fixedJ->_m_Error=rotationalErr; 
		
		const Mat33 zero = Const<Mat33>::Zero();
		const Mat33 identity = Const<Mat33>::Identity();
	
		fixedJ->_m_J1 = MakeMat36(zero, identity);

		if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			//if body 2 is static, need to increase the number of iteration to solve it, otherwise we need to change the construction
			//method for the jacobian
			fixedJ->_m_J2=Const<Mat36>::Zero();
		}
		else
		{
			fixedJ->_m_J2 = MakeMat36(zero, -identity);
		}
	}


	void C_FixedRotationalConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
	{
		//DebugBreak();
		using namespace Maths;

	/*	Spu::MemBaseMarker marker;

		bool bFlip = j->m_pRigid1 != imjt->m_pRigid1;*/
		

		/*imjt->m_pRigid1 = j->m_pRigid1;
		imjt->m_pRigid2 = j->m_pRigid2;
		imjt->m_NodeBFlag = j->m_NodeBFlag;*/

		bool bFlip =(j->m_pSpuConstraintBody1 != imjt->m_pSpuConstraintBody1);
		imjt->m_pSpuConstraintBody1 = j->m_pSpuConstraintBody1;
		imjt->m_pSpuConstraintBody2 = j->m_pSpuConstraintBody2;
		imjt->m_NodeBFlag = j->m_NodeBFlag;

		/*SharedPhysics::C_GraphNode* graphNodeA = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
		SharedPhysics::C_GraphNode* graphNodeB = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));*/
		/*SharedPhysics::C_GraphNode* graphNodeA = &imjt->m_pSpuConstraintBody1->m_GraphNode;
		SharedPhysics::C_GraphNode* graphNodeB = &imjt->m_pSpuConstraintBody2->m_GraphNode;*/


		const Scalar_Bool scalarFlip(bFlip);

		/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));
		SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));*/
		
		SharedPhysics::C_RigidBody* pRigid1 = &imjt->m_pSpuConstraintBody1->m_RigidBody;
		SharedPhysics::C_RigidBody* pRigid2 = &imjt->m_pSpuConstraintBody2->m_RigidBody;

		Solver2::C_FixedRotationalJacobian* fixedJ=(Solver2::C_FixedRotationalJacobian*) j;
		Solver2::C_FixedRotationalInvertMJT* fixedIMJT=(Solver2::C_FixedRotationalInvertMJT*) imjt;

		//const VF32 scale = MakeVF32(m_Scale);

		fixedIMJT->SetScale(MakeVF32(m_Scale));

		const HF32 _massScaleA(GetBodyAMassScale());
		const HF32 _massScaleB(GetBodyBMassScale());
		const HF32 _inertiaScaleA(GetBodyAInertiaScale());
		const HF32 _inertiaScaleB(GetBodyBInertiaScale());
		const HF32 massScaleA(Maths::Select(scalarFlip, _massScaleB, _massScaleA));
		const HF32 massScaleB(Maths::Select(scalarFlip, _massScaleA, _massScaleB));
		const HF32 inertiaScaleA(Maths::Select(scalarFlip, _inertiaScaleB, _inertiaScaleA));
		const HF32 inertiaScaleB(Maths::Select(scalarFlip, _inertiaScaleA, _inertiaScaleB));


		const VF32 mV1(pRigid1->ReciprocalMass());// * massScaleA);

		const HMat33 scaledInverseInertiaA(pRigid1->GetApproxScaledInverseInertiaTensor(inertiaScaleA));
		const Vector3 x(scaledInverseInertiaA.AX());
		const Vector3 y(scaledInverseInertiaA.AY());
		const Vector3 z(scaledInverseInertiaA.AZ());

		const Vector3 row0=Col<0>(fixedJ->_m_J1) * mV1;
		const Vector3 row1=Col<1>(fixedJ->_m_J1) * mV1;
		const Vector3 row2=Col<2>(fixedJ->_m_J1) * mV1;
		

		const Vector3 jRow01=ExtractHiVector3(Row<0>(fixedJ->_m_J1));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(fixedJ->_m_J1));
		const Vector3 jRow21=ExtractHiVector3(Row<2>(fixedJ->_m_J1));


		const VF32 r30=Dot(x, jRow01);
		const VF32 r31=Dot(x, jRow11);
		const VF32 r32=Dot(x, jRow21);

		const VF32 r40=Dot(y, jRow01);
		const VF32 r41=Dot(y, jRow11);
		const VF32 r42=Dot(y, jRow21);
		
		const VF32 r50=Dot(z, jRow01);
		const VF32 r51=Dot(z, jRow11);
		const VF32 r52=Dot(z, jRow21);

		const Vector3 row3=MakeVector3(r30, r31, r32);
		const Vector3 row4=MakeVector3(r40, r41, r42);
		const Vector3 row5=MakeVector3(r50, r51, r52);

		fixedIMJT->_m_IMJT1 = MakeMat63(row0, row1, row2, row3, row4, row5);

	
		/*
			If the second object is static, we need to sent it corresponding jacobian matrix to zero,
			otherwise initialize it into the correct value
		*/
		//if(graphNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			fixedIMJT->_m_IMJT2 = Const<Mat63>::Zero();
		}
		else
		{
			const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);
			const HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
			const Vector3 x2(scaledInverseInertiaB.AX());
			const Vector3 y2(scaledInverseInertiaB.AY());
			const Vector3 z2(scaledInverseInertiaB.AZ());

			const Vector3 row0=Col<0>(fixedJ->_m_J2) * mV2;
			const Vector3 row1=Col<1>(fixedJ->_m_J2) * mV2;
			const Vector3 row2=Col<2>(fixedJ->_m_J2) * mV2;

			const Vector3 jRow01=ExtractHiVector3(Row<0>(fixedJ->_m_J2));
			const Vector3 jRow11=ExtractHiVector3(Row<1>(fixedJ->_m_J2));
			const Vector3 jRow21=ExtractHiVector3(Row<2>(fixedJ->_m_J2));

			const VF32 r30=Dot(x2, jRow01);
			const VF32 r31=Dot(x2, jRow11);
			const VF32 r32=Dot(x2, jRow21);

			const VF32 r40=Dot(y2, jRow01);
			const VF32 r41=Dot(y2, jRow11);
			const VF32 r42=Dot(y2, jRow21);

			const VF32 r50=Dot(z2, jRow01);
			const VF32 r51=Dot(z2, jRow11);
			const VF32 r52=Dot(z2, jRow21);
			

			const Vector3 row3=MakeVector3(r30, r31, r32);
			const Vector3 row4=MakeVector3(r40, r41, r42);
			const Vector3 row5=MakeVector3(r50, r51, r52);

			fixedIMJT->_m_IMJT2 = MakeMat63(row0, row1, row2, row3, row4, row5);
		}
	}

}
