#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


namespace Solver2
{

void C_PrismaticConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{
	//const Maths::VF32 zero(Maths::Const<Maths::VF32>::Zero());
	jaco->m_pConstraint = m_pPPUConstraint;
	using namespace Maths;

	jaco->m_Baumgarte = m_Baumgarte;


	SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;

	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

	const Maths::Scalar_Bool scalarFlip(bFlip);


	Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
	jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
	jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);

	const Maths::HVec3 _jointAxis2(Maths::Select(scalarFlip, m_JointAxis1, m_JointAxis2));
	const Maths::HVec3 _offset(Maths::Select(scalarFlip, m_OffSetB, m_OffSetA));

	SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;


	Solver2::C_PrismaticJointJacobian* prismaticJ=(Solver2::C_PrismaticJointJacobian*)jaco;


	HMat33 mat1(pRigid1->m_Orientation);
	HMat33 mat2(pRigid2->m_Orientation);
	
	const Vector3 jointAxis(_jointAxis2 * mat2); //rigidbody2's joint axis in world space - tha
	

	const Vector3 position1(pRigid1->m_Position);
	const Vector3 position2(pRigid2->m_Position);
	const Vector3 c(position2 - position1); 
	const Normal3 c2=MakeNormal3(MakeVector3(Normalise(c)));

#if 0
	const HVec3 jointAxis(Normalise(jointAxis));
	HVec3 u, v;
	jointAxis.PerpendicularAxes(u, v);
	const Vector3 t1(u);
	const Vector3 t2(v);
	const Vector3 t3(jointAxis);
#else
	Normal3 t1, t2, t3;
	PerpendicularAxes(jointAxis, &t1, &t2, &t3 );
	//t3 = Maths::Select(Maths::MakeVBool(scalarFlip), -t3, t3);
#endif
	
	//const Vector3 c(position2 - position1);
	const VF32 half(Const<VF32>::Half());
	const Vector3 halfC(half*c2);
	const Vector3 cXt1(t1^halfC);//c X t1
	const Vector3 cXt2(t2^halfC); //c X t2

	const Vector3 tOffSet(_offset * mat2);
	const Vector3 offset(c - tOffSet);
	const VF32 t1Do(t1|offset);
	const VF32 t2Do(t2|offset);


	prismaticJ->_m_Error= MakeVector2(t1Do, t2Do);

	const Vector3 tV1(MakeVector3(t1));
	const Vector3 tV2(MakeVector3(t2));
	const Vector6 row0=MakeVector6(tV1, cXt1);
	const Vector6 row1=MakeVector6(tV2, cXt2);
	
	prismaticJ->_m_J1 = MakeMat26(row0, row1);

	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		prismaticJ->_m_J2= Maths::Const<Mat26>::Zero();
	}
	else
	{
		const Vector6 row0=MakeVector6(-tV1, cXt1);
		const Vector6 row1=MakeVector6(-tV2, cXt2);
		
		prismaticJ->_m_J2 = MakeMat26(row0, row1);
	}
}




void C_PrismaticConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
{
	using namespace Maths;

	Spu::MemBaseMarker marker;

	/*bool bFlip = j->m_pRigid1 != imjt->m_pRigid1;

	imjt->m_pRigid1 = j->m_pRigid1;
	imjt->m_pRigid2 = j->m_pRigid2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;*/

	bool bFlip = (j->m_pSpuConstraintBody1 != imjt->m_pSpuConstraintBody1);
	imjt->m_pSpuConstraintBody1 = j->m_pSpuConstraintBody1;
	imjt->m_pSpuConstraintBody2 = j->m_pSpuConstraintBody2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;

	/*SharedPhysics::C_GraphNode* graphNodeA = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* graphNodeB = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));*/

	//SharedPhysics::C_GraphNode* graphNodeA = &imjt->m_pSpuConstraintBody1->m_GraphNode;
	//SharedPhysics::C_GraphNode* graphNodeB = &imjt->m_pSpuConstraintBody2->m_GraphNode;

	const Maths::Scalar_Bool scalarFlip(bFlip);

	Solver2::C_PrismaticJointJacobian* prismaticJ=(Solver2::C_PrismaticJointJacobian*) j;
	Solver2::C_PrismaticJointInvertMJT* prismaticIMJT=(Solver2::C_PrismaticJointInvertMJT*)imjt;

	prismaticIMJT->m_Scale = MakeVF32(m_Scale);

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/

	SharedPhysics::C_RigidBody* pRigid1 = &imjt->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &imjt->m_pSpuConstraintBody2->m_RigidBody;

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

	const Vector2 row0=Col<0>(prismaticJ->_m_J1) * mV1;
	const Vector2 row1=Col<1>(prismaticJ->_m_J1) * mV1;
	const Vector2 row2=Col<2>(prismaticJ->_m_J1) * mV1;

	const Vector3 jRow01=ExtractHiVector3(Row<0>(prismaticJ->_m_J1));
	const Vector3 jRow11=ExtractHiVector3(Row<1>(prismaticJ->_m_J1));
	
	const VF32 row30=Dot(x, jRow01);
	const VF32 row31=Dot(x, jRow11);

	const VF32 row40=Dot(y, jRow01);
	const VF32 row41=Dot(y, jRow11);

	const VF32 row50=Dot(z, jRow01);
	const VF32 row51=Dot(z, jRow11);
	
	const Vector2 row3=MakeVector2(row30, row31);
	const Vector2 row4=MakeVector2(row40, row41);
	const Vector2 row5=MakeVector2(row50, row51);

	prismaticIMJT->_m_IMJT1 = MakeMat62(row0, row1, row2, row3, row4, row5);

	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		prismaticIMJT->_m_IMJT2=Const<Mat62>::Zero();

	}
	else
	{
		const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);

		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());

		const Vector2 row0=Col<0>(prismaticJ->_m_J2) * mV2;
		const Vector2 row1=Col<1>(prismaticJ->_m_J2) * mV2;
		const Vector2 row2=Col<2>(prismaticJ->_m_J2) * mV2;

		const Vector3 jRow01=ExtractHiVector3(Row<0>(prismaticJ->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(prismaticJ->_m_J2));

		const VF32 row30=Dot(x2, jRow01);
		const VF32 row31=Dot(x2, jRow11);
		
		const VF32 row40=Dot(y2, jRow01);
		const VF32 row41=Dot(y2, jRow11);
		
		const VF32 row50=Dot(z2, jRow01);
		const VF32 row51=Dot(z2, jRow11);

		const Vector2 row3=MakeVector2(row30, row31);
		const Vector2 row4=MakeVector2(row40, row41);
		const Vector2 row5=MakeVector2(row50, row51);

		prismaticIMJT->_m_IMJT2 = MakeMat62(row0, row1, row2, row3, row4, row5);

	}

}

#ifndef PLATFORM_SPU
bool C_PrismaticConstraint::ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset)
{
	if (pNodeA && pNodeA->m_pBody == body)
	{
		m_OffSetA += offset;
		m_OffSetB -= offset;
		return true;
	}
	else if (pNodeB && pNodeB->m_pBody == body)
	{
		m_OffSetA -= offset;
		m_OffSetB += offset;
		return true;
	}

	return false;
}
#endif

};
