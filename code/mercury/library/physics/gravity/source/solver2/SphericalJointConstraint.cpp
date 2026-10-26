#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"
#include "gravity/sdk/Body.h"


namespace Solver2
{

void C_SphericalConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{
	//DebugBreak();
	jaco->m_pConstraint = m_pPPUConstraint;
	using namespace Maths;

	jaco->m_Baumgarte = m_Baumgarte;

/*	Spu::MemBaseMarker marker;
	SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));*/ 

	SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;

	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

	/*SharedPhysics::C_GraphNode* graphNode1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
	SharedPhysics::C_GraphNode* graphNode2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);*/

	const Maths::Scalar_Bool scalarFlip(bFlip);

	/*SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;

	jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
	jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);*/

	Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
	jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
	jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);

	const Maths::HVec3 anchorPoint1(Maths::Select(scalarFlip, m_AnchorPoint2, m_AnchorPoint1));
	const Maths::HVec3 anchorPoint2(Maths::Select(scalarFlip, m_AnchorPoint1, m_AnchorPoint2));

	Solver2::C_SphericalJointJacobian* sphericalJ=(Solver2::C_SphericalJointJacobian*)jaco;
	
	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNode1->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNode2->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));*/

	SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;
	
	Vector3 r1( (anchorPoint1 - pRigid1->m_CenterOfMassOffset) * pRigid1->m_OrientationMatrix ); //construct a skew matrix form and put it into the m_J1
	Vector3 r2( (anchorPoint2 - pRigid2->m_CenterOfMassOffset) * pRigid2->m_OrientationMatrix );//construct a skew matrix form and put it into the m_J2

	const Vector3 position2(pRigid2->CenterOfRotation());
	const Vector3 position1(pRigid1->CenterOfRotation());

	sphericalJ->_m_Error= (position2 + r2) - (position1 + r1);

	if((sphericalJ->_m_Error | sphericalJ->_m_Error) >= this->m_BreakErrorSq)
		m_Scale = 0.f;


	const Maths::Mat33 identity(Maths::Const< Mat33 >::Identity ());
	const Maths::Mat33 skew1=MakeMat33_SkewSymmetric(r1);
	sphericalJ->_m_J1=MakeMat36(identity, skew1);

	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	{
		sphericalJ->_m_J2=Maths::Const<Mat36>::Zero();
	}
	else
	{
		//const Maths::Mat33 identity(Maths::Const< Mat33 >::Identity ());
		const Maths::Mat33 skew2=MakeMat33_SkewSymmetric(-r2);
		sphericalJ->_m_J2=MakeMat36(-identity, skew2);
	}
	//sphericalJ->_m_J1 = Maths::Const<Mat36>::Zero();
}


void C_SphericalConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
{
	//DebugBreak();
	using namespace Maths;

	//Spu::MemBaseMarker marker;

	//bool bFlip = j->m_pRigid1 != imjt->m_pRigid1;

	/*imjt->m_pRigid1 = j->m_pRigid1;
	imjt->m_pRigid2 = j->m_pRigid2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;*/

	bool bFlip = (j->m_pSpuConstraintBody1 != imjt->m_pSpuConstraintBody1);

	imjt->m_pSpuConstraintBody1 = j->m_pSpuConstraintBody1;
	imjt->m_pSpuConstraintBody2 = j->m_pSpuConstraintBody2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;

	/*SharedPhysics::C_GraphNode* graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));*/
	/*SharedPhysics::C_GraphNode* graphNodeA = &imjt->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* graphNodeB = &imjt->m_pSpuConstraintBody2->m_GraphNode;*/

	const Maths::Scalar_Bool scalarFlip(bFlip);

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNode1->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNode2->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));*/
	SharedPhysics::C_RigidBody* pRigid1=&imjt->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2=&imjt->m_pSpuConstraintBody2->m_RigidBody;

	Solver2::C_SphericalJointJacobian* sphericalJ=(Solver2::C_SphericalJointJacobian*) j;
	Solver2::C_SphericalJointInvertMJT* sphericalIMJT=(Solver2::C_SphericalJointInvertMJT*) imjt;

	sphericalIMJT->m_Scale = MakeVF32(m_Scale);

	const Maths::HF32 _massScaleA(GetBodyAMassScale());
	const Maths::HF32 _massScaleB(GetBodyBMassScale());
	const Maths::HF32 _inertiaScaleA(GetBodyAInertiaScale());
	const Maths::HF32 _inertiaScaleB(GetBodyBInertiaScale());
	const Maths::HF32 massScaleA(Maths::Select(scalarFlip, _massScaleB, _massScaleA));
	const Maths::HF32 massScaleB(Maths::Select(scalarFlip, _massScaleA, _massScaleB));
	const Maths::HF32 inertiaScaleA(Maths::Select(scalarFlip, _inertiaScaleB, _inertiaScaleA));
	const Maths::HF32 inertiaScaleB(Maths::Select(scalarFlip, _inertiaScaleA, _inertiaScaleB));


	const VF32 mV1(pRigid1->ReciprocalMass() * massScaleA);

	const Maths::HMat33 scaledInverseInertiaA(pRigid1->GetApproxScaledInverseInertiaTensor(inertiaScaleA));
	const Vector3 x(scaledInverseInertiaA.AX());
	const Vector3 y(scaledInverseInertiaA.AY());
	const Vector3 z(scaledInverseInertiaA.AZ());

	const Vector3 row0=Col<0>(sphericalJ->_m_J1) * mV1;
	const Vector3 row1=Col<1>(sphericalJ->_m_J1) * mV1;
	const Vector3 row2=Col<2>(sphericalJ->_m_J1) * mV1;

	const Vector3 jRow01=ExtractHiVector3(Row<0>(sphericalJ->_m_J1));
	const Vector3 jRow11=ExtractHiVector3(Row<1>(sphericalJ->_m_J1));
	const Vector3 jRow21=ExtractHiVector3(Row<2>(sphericalJ->_m_J1));

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
	sphericalIMJT->_m_IMJT1 = MakeMat63(row0, row1, row2, row3, row4, row5);


	/*
		If the second object is static, we need to sent it corresponding jacobian matrix to zero,
		otherwise initialize it into the correct value
	*/
	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		sphericalIMJT->_m_IMJT2 = Const<Mat63>::Zero();
	}
	else
	{
		const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);
		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());

		const Vector3 row0=Col<0>(sphericalJ->_m_J2) * mV2;
		const Vector3 row1=Col<1>(sphericalJ->_m_J2) * mV2;
		const Vector3 row2=Col<2>(sphericalJ->_m_J2) * mV2;

		const Vector3 jRow01=ExtractHiVector3(Row<0>(sphericalJ->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(sphericalJ->_m_J2));
		const Vector3 jRow21=ExtractHiVector3(Row<2>(sphericalJ->_m_J2));

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

		sphericalIMJT->_m_IMJT2 = MakeMat63(row0, row1, row2, row3, row4, row5);
		

	}
}



void C_SphericalConstraint::PostStabilise()
{
	using namespace Maths;

	SharedPhysics::C_RigidBody* pRigidA = pNodeA->m_pRigidBody;
	SharedPhysics::C_RigidBody* pRigidB = pNodeB->m_pRigidBody;

	const Maths::HF32 massScaleA(pRigidA->ReciprocalMass() * GetBodyAMassScale());
	Maths::HF32 massScaleB(Maths::Constants<Maths::HF32>::Zero());
	
	if(pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
	{
		massScaleB = pRigidB->ReciprocalMass() * GetBodyBMassScale();
	}

	const Maths::HF32 recipSumMassScale(Maths::Reciprocal(massScaleA + massScaleB));
	const Maths::HF32 scaleA(massScaleA * recipSumMassScale);
	const Maths::HF32 scaleB(massScaleB * recipSumMassScale);
	//Get the world anchor point base on the new car body orientation
	Maths::HVec3 worldAnchorA = pRigidA->m_Position + m_AnchorPoint1 * pRigidA->m_OrientationMatrix;
	Maths::HVec3 worldAnchorB = pRigidB->m_Position + m_AnchorPoint2 * pRigidB->m_OrientationMatrix;

	//const Maths::HVec3 offset(worldAnchorA - worldAnchorB);
	const Maths::HVec3 offset(worldAnchorA - worldAnchorB);

	pRigidA->m_Position -= (offset * scaleA);
	pRigidB->m_Position += (offset * scaleB);
	
}

void C_SphericalConstraint::PostStabilise(Gravity::C_Body* _pBodyA, Gravity::C_Body* _pBodyB)
{
	using namespace Maths;

	Gravity::C_Body* pBodyA = SharedPhysics::Fast_Select<Gravity::C_Body>(_pBodyA->GetGraphNode() == this->pNodeA, _pBodyA, _pBodyB);
	Gravity::C_Body* pBodyB = SharedPhysics::Fast_Select<Gravity::C_Body>(_pBodyA->GetGraphNode() == this->pNodeA, _pBodyB, _pBodyA);
	SharedPhysics::C_RigidBody* pRigidA = &pBodyA->GetBody();
	SharedPhysics::C_RigidBody* pRigidB = &pBodyB->GetBody();

	const Maths::HF32 massScaleA(pRigidA->ReciprocalMass() * GetBodyAMassScale());
	Maths::HF32 massScaleB(Maths::Constants<Maths::HF32>::Zero());
	
	if(pBodyB->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active | SharedPhysics::C_GraphNode::E_Type_Sleeping))
	{
		massScaleB = pRigidB->ReciprocalMass() * GetBodyBMassScale();
	}

	const Maths::HF32 recipSumMassScale(Maths::Reciprocal(massScaleA + massScaleB));
	const Maths::HF32 scaleA(massScaleA * recipSumMassScale);
	const Maths::HF32 scaleB(massScaleB * recipSumMassScale);
	//Get the world anchor point base on the new car body orientation
	Maths::HVec3 worldAnchorA = pRigidA->m_Position + m_AnchorPoint1 * pRigidA->m_OrientationMatrix;
	Maths::HVec3 worldAnchorB = pRigidB->m_Position + m_AnchorPoint2 * pRigidB->m_OrientationMatrix;

	//const Maths::HVec3 offset(worldAnchorA - worldAnchorB);
	const Maths::HVec3 offset(worldAnchorA - worldAnchorB);

	pRigidA->m_Position -= (offset * scaleA);
	pRigidB->m_Position += (offset * scaleB);
	
}

#ifndef PLATFORM_SPU
bool C_SphericalConstraint::ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset)
{
	if (pNodeA && pNodeA->m_pBody == body)
	{
		m_AnchorPoint1 += offset;
		return true;
	}
	else if (pNodeB && pNodeB->m_pBody == body)
	{
		m_AnchorPoint2 += offset;
		return true;
	}

	return false;
}
#endif

};
