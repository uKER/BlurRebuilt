#include "stdafx.h"

#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


namespace Solver2
{

//void C_TwoRotationalConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
//{
//
//		jaco->m_pConstraint = m_pPPUConstraint;
//
//	using namespace Maths;
//
//	const VF32 zeroVF(Const<VF32>::Zero());
//
//	jaco->m_Baumgarte = m_Baumgarte;
//
//	Spu::MemBaseMarker marker;
//	SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
//	SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));
//
//	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);
//
//	SharedPhysics::C_GraphNode* graphNodeA = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
//	SharedPhysics::C_GraphNode* graphNodeB = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);
//
//	const Maths::Scalar_Bool scalarFlip(bFlip);
//
//	SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;
//
//	jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
//	jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);
//
//
//	const Maths::HVec3 _jointAxis1(Maths::Select(scalarFlip, m_JointAxis2, m_JointAxis1));
//	const Maths::HVec3 _jointAxis2(Maths::Select(scalarFlip, m_JointAxis1, m_JointAxis2));
//
//	const Maths::Quaternion rotationOffset(Maths::Select(scalarFlip, !m_rotationalOffset, m_rotationalOffset));
//
//	assert(!graphNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject));
//
//	Solver2::C_TwoRotationalJacobian* revoluteJ=(Solver2::C_TwoRotationalJacobian*)jaco;
//
//	SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
//	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
//
//	const HMat33 mat1(pRigid1->m_OrientationMatrix);
//	const HMat33 mat2(pRigid2->m_OrientationMatrix);
//
//	const Vector3 jointAxis1(_jointAxis1 * mat1); //rigidbody1's joint axis in world space
//	const Vector3 jointAxis2(_jointAxis2 * mat2); //rigidbody2's joint axist in world space
//
//
//	/*const VF32 resVF(jointAxis2 | jointAxis2);
//	const Vector3 res(MakeVector3(resVF, resVF, resVF));
//	const Normal3 temp2(Normalise(jointAxis2 - res));
//	const Normal3 t1(Normalise(temp2^jointAxis2));
//	const Normal3 t2(Normalise(t1^jointAxis2));*/
//
//	/*const Maths::HVec3 ja2(jointAxis2);
//	Maths::HVec3 u(Maths::UNDEFINED), v(Maths::UNDEFINED);
//	ja2.PerpendicularAxes(u, v);
//	const Vector3 t1(u);
//	const Vector3 t2(v);*/
//
//	//const HMat33 mat(ja2, u, v);
//
//
//
//	const Vector3 position2=pRigid2->m_Position;
//	const Vector3 position1=pRigid1->m_Position;
//	const Quaternion rot1(pRigid1->m_Orientation);
//	const Quaternion rot2(pRigid2->m_Orientation);
//
//	
//	//Vector3 rotationalErr(jointAxis2 ^ jointAxis1); //jointAxis1 X jointAxis2
//	const VF32 two(Const<VF32>::Two());
//	const Quaternion targetB( rotationOffset * rot1);
//	const Quaternion relQuaternion(Maths::NearestDifference(rot2, targetB));
//	const Vector3 rotationalErr(Imag(relQuaternion)* two);
//
//	/*const VF32 err0=Dot(jointAxis2, rotationalErr);
//	const VF32 err1 = Const<VF32>::Zero();*/
//
//	//const VF32 err0=Dot(t1, rotationalErr);
//	//const VF32 err1=Dot(t2, rotationalErr);
//	const VF32 err2=Dot(jointAxis2, rotationalErr);
//
//	revoluteJ->_m_Error=MakeVector2(err2, Const<VF32>::Zero());
//
//
//	const Vector3 zeroVec3(Const<Vector3>::Zero());
//	const Vector6 zeroVec6(Const<Vector6>::Zero());
//
//	const Vector6 row0=MakeVector6(zeroVec3, jointAxis2);
//	
//	revoluteJ->_m_J1 = MakeMat26(row0, zeroVec6);
//
//	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pRigid2->m_Position, HVec3(jointAxis2), 2.f, 0.25f, RGBA(0, 255, 0, 0)); 
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pRigid2->m_Position, HVec3(t1), 2.f, 0.25f, RGBA(0, 0, 255, 0));
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pRigid2->m_Position, HVec3(t2), 2.f, 0.25f, RGBA(0, 255, 255, 0));*/
//
//	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(HMat43(mat, pRigid2->m_Position), 1.f);
//	
//
//	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
//	{
//		revoluteJ->_m_J2=Const<Mat26>::Zero();
//	}
//	else
//	{
//		const Vector6 row0=MakeVector6(zeroVec3, (-jointAxis2));
//		revoluteJ->_m_J2 = MakeMat26(row0, zeroVec6);
//	
//	}
//}
//
//
//
//void C_TwoRotationalConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
//{
//	using namespace Maths;
//
//	Spu::MemBaseMarker marker;
//
//	bool bFlip = j->m_pRigid1 != imjt->m_pRigid1;
//	//assert(!bFlip);
//
//	imjt->m_pRigid1 = j->m_pRigid1;
//	imjt->m_pRigid2 = j->m_pRigid2;
//	imjt->m_NodeBFlag = j->m_NodeBFlag;
//
//	SharedPhysics::C_GraphNode* graphNodeA = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
//	SharedPhysics::C_GraphNode* graphNodeB = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));
//
//	const Maths::Scalar_Bool scalarFlip(bFlip);
//
//
//	Solver2::C_TwoRotationalJacobian* revoluteJ=(Solver2::C_TwoRotationalJacobian*) j;
//	Solver2::C_TwoRotationalInvertMJT* revoluteIMJT=(Solver2::C_TwoRotationalInvertMJT*)imjt;
//
//	revoluteIMJT->m_Scale = MakeVF32(m_Scale);
//
//	SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
//	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
//	
//	const Maths::HF32 _massScaleA(GetBodyAMassScale());
//	const Maths::HF32 _massScaleB(GetBodyBMassScale());
//	const Maths::HVec3 _inertiaScaleA(GetBodyAInertiaScale());
//	const Maths::HVec3 _inertiaScaleB(GetBodyBInertiaScale());
//	const Maths::HF32 massScaleA(Maths::Select(scalarFlip, _massScaleB, _massScaleA));
//	const Maths::HF32 massScaleB(Maths::Select(scalarFlip, _massScaleA, _massScaleB));
//	const Maths::HVec3 inertiaScaleA(Maths::Select(scalarFlip, _inertiaScaleB, _inertiaScaleA));
//	const Maths::HVec3 inertiaScaleB(Maths::Select(scalarFlip, _inertiaScaleA, _inertiaScaleB));
//
//
//	const VF32 mV1(pRigid1->m_ReciprocalMass * massScaleA);
//
//	const Maths::HMat33 scaledInverseInertiaA(pRigid1->GetScaledInverseInertiaTensor(inertiaScaleA));
//	const Vector3 x(scaledInverseInertiaA.AX());
//	const Vector3 y(scaledInverseInertiaA.AY());
//	const Vector3 z(scaledInverseInertiaA.AZ());
//
//	const Vector2 row0=Col<0>(revoluteJ->_m_J1) * mV1;
//	const Vector2 row1=Col<1>(revoluteJ->_m_J1) * mV1;
//	const Vector2 row2=Col<2>(revoluteJ->_m_J1) * mV1;
//
//	const Vector3 jRow01=ExtractHiVector3(Row<0>(revoluteJ->_m_J1));
//	const Vector3 jRow11=ExtractHiVector3(Row<1>(revoluteJ->_m_J1));
//	//const Vector3 jRow12=ExtractHiVector3(Row<2>(revoluteJ->_m_J1));
//
//	const VF32 row30=Dot(x, jRow01);
//	const VF32 row31=Dot(x, jRow11);
//
//	const VF32 row40=Dot(y, jRow01);
//	const VF32 row41=Dot(y, jRow11);
//
//	const VF32 row50=Dot(z, jRow01);
//	const VF32 row51=Dot(z, jRow11);
//	
//
//	const Vector2 row3=MakeVector2(row30, row31);
//	const Vector2 row4=MakeVector2(row40, row41);
//	const Vector2 row5=MakeVector2(row50, row51);
//
//
//	revoluteIMJT->_m_IMJT1 = MakeMat62(row0, row1, row2, row3, row4, row5);
//
//	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
//	{
//		revoluteIMJT->_m_IMJT2=Const<Mat62>::Zero();
//
//	}
//	else
//	{
//		const VF32 mV2(pRigid2->m_ReciprocalMass * massScaleB);
//
//		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetScaledInverseInertiaTensor(inertiaScaleB));
//		const Vector3 x2(scaledInverseInertiaB.AX());
//		const Vector3 y2(scaledInverseInertiaB.AY());
//		const Vector3 z2(scaledInverseInertiaB.AZ());
//
//		const Vector2 row0=Col<0>(revoluteJ->_m_J2) * mV2;
//		const Vector2 row1=Col<1>(revoluteJ->_m_J2) * mV2;
//		const Vector2 row2=Col<2>(revoluteJ->_m_J2) * mV2;
//
//		const Vector3 jRow01=ExtractHiVector3(Row<0>(revoluteJ->_m_J2));
//		const Vector3 jRow11=ExtractHiVector3(Row<1>(revoluteJ->_m_J2));
//		//const Vector3 jRow12=ExtractHiVector3(Row<2>(revoluteJ->_m_J2));
//													
//		const VF32 row30=Dot(x2, jRow01);
//		const VF32 row31=Dot(x2, jRow11);
//		//const VF32 row32=Dot(x2, jRow12);
//
//		const VF32 row40=Dot(y2, jRow01);
//		const VF32 row41=Dot(y2, jRow11);
//		//const VF32 row42=Dot(y2, jRow12);
//		
//		const VF32 row50=Dot(z2, jRow01);
//		const VF32 row51=Dot(z2, jRow11);
//		//const VF32 row52=Dot(z2, jRow12);
//		
//		const Vector2 row3=MakeVector2(row30, row31);
//		const Vector2 row4=MakeVector2(row40, row41);
//		const Vector2 row5=MakeVector2(row50, row51);
//
//		revoluteIMJT->_m_IMJT2 = MakeMat62(row0, row1, row2, row3, row4, row5);
//
//	}
//}

void C_TwoRotationalConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{

	jaco->m_pConstraint = m_pPPUConstraint;

	using namespace Maths;

	//const VF32 zeroVF(Const<VF32>::Zero());

	jaco->m_Baumgarte = m_Baumgarte;

	/*Spu::MemBaseMarker marker;
	SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode)); */

	SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;

	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

	//SharedPhysics::C_GraphNode* graphNodeA = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
	//SharedPhysics::C_GraphNode* graphNodeB = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);

	const Maths::Scalar_Bool scalarFlip(bFlip);

	/*SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;
	jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
	jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);*/

	Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
	jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
	jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);


	//const Maths::HVec3 _jointAxis1(Maths::Select(scalarFlip, m_JointAxis2, m_JointAxis1));
	const Maths::HVec3 _jointAxis2(Maths::Select(scalarFlip, m_JointAxis1, m_JointAxis2));

	const Maths::Quaternion rotationOffset(Maths::Select(scalarFlip, !m_rotationalOffset, m_rotationalOffset));

	BIZ_ASSERT(!jaco->m_pSpuConstraintBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject));

	Solver2::C_TwoRotationalJacobian* revoluteJ=(Solver2::C_TwoRotationalJacobian*)jaco;

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/

	SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;

	/*const HMat33 mat1(pRigid1->m_OrientationMatrix);
	const HMat33 mat2(pRigid2->m_OrientationMatrix);*/

	//const Vector3 jointAxis1(_jointAxis1 * mat1); //rigidbody1's joint axis in world space
	

	//const Vector3 position2=pRigid2->m_Position;
	//const Vector3 position1=pRigid1->m_Position;
	const Quaternion rot1(pRigid1->m_Orientation);
	const Quaternion rot2(pRigid2->m_Orientation);

	
	/*const VF32 two(Const<VF32>::Two());
	const Quaternion targetB( rotationOffset * rot1);
	const Quaternion relQuaternion(NearestDifference(rot2, targetB));
	const Vector3 rotationalErr(Imag(relQuaternion)* two);*/

	const VF32 two(Const<VF32>::Two());
	const Quaternion targetB( rotationOffset * rot1);
	const Quaternion relQuaternion(Maths::NearestDifference(rot2, targetB));
	Normalise(relQuaternion);
	
	const Vector3 rotationalErr( Imag( relQuaternion ) * two );

	const Vector3 jointAxis2(_jointAxis2 * pRigid2->m_Orientation); //rigidbody2's joint axist in world space

	revoluteJ->_m_Error=Dot(jointAxis2, rotationalErr);

	const Vector3 zeroVec3(Const<Vector3>::Zero());
	
	revoluteJ->_m_J1 = MakeVector6(zeroVec3, jointAxis2);
	

	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	{
		revoluteJ->_m_J2=Const<Vector6>::Zero();
	}
	else
	{
		revoluteJ->_m_J2 = MakeVector6(zeroVec3, (-jointAxis2));
	
	}
}



void C_TwoRotationalConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
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

	/*SharedPhysics::C_GraphNode* graphNodeA = &imjt->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* graphNodeB = &imjt->m_pSpuConstraintBody2->m_GraphNode;*/

	const Maths::Scalar_Bool scalarFlip(bFlip);


	Solver2::C_TwoRotationalJacobian* revoluteJ=(Solver2::C_TwoRotationalJacobian*) j;
	Solver2::C_TwoRotationalInvertMJT* revoluteIMJT=(Solver2::C_TwoRotationalInvertMJT*)imjt;

	revoluteIMJT->m_Scale = MakeVF32(m_Scale);

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

	const VF32 row0=Element<0>(revoluteJ->_m_J1) * mV1;
	const VF32 row1=Element<1>(revoluteJ->_m_J1) * mV1;
	const VF32 row2=Element<2>(revoluteJ->_m_J1) * mV1;

	const Vector3 jRow01=ExtractHiVector3(revoluteJ->_m_J1);

	const VF32 row3=Dot(x, jRow01);

	const VF32 row4=Dot(y, jRow01);

	const VF32 row5=Dot(z, jRow01);

	revoluteIMJT->_m_IMJT1 = MakeVector6(row0, row1, row2, row3, row4, row5);

	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		revoluteIMJT->_m_IMJT2=Const<Vector6>::Zero();

	}
	else
	{
		const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);

		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());

		const VF32 row0=Element<0>(revoluteJ->_m_J2) * mV2;
		const VF32 row1=Element<1>(revoluteJ->_m_J2) * mV2;
		const VF32 row2=Element<2>(revoluteJ->_m_J2) * mV2;

		const Vector3 jRow01=ExtractHiVector3(revoluteJ->_m_J2);
													
		const VF32 row3=Dot(x2, jRow01);
		const VF32 row4=Dot(y2, jRow01);
		const VF32 row5=Dot(z2, jRow01);

		revoluteIMJT->_m_IMJT2 = MakeVector6(row0, row1, row2, row3, row4, row5);

	}
}

};
