#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


namespace Solver2
{
void C_RevoluteConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{


	//static U32 frameCount(0);
	//frameCount++;
	//assert(frameCount != 50);
	jaco->m_pConstraint = m_pPPUConstraint;

	using namespace Maths;

	//const VF32 zeroVF(Const<VF32>::Zero());

	jaco->m_Baumgarte = m_Baumgarte;

	//Spu::MemBaseMarker marker;
	//SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	//SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode)); 
	SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;

	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

	/*SharedPhysics::C_GraphNode* graphNodeA = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
	SharedPhysics::C_GraphNode* graphNodeB = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);*/

	const Maths::Scalar_Bool scalarFlip(bFlip);

	//SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;

	/*jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
	jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);*/

	Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
	jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
	jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);


	const Maths::HVec3 _jointAxis1(Maths::Select(scalarFlip, m_JointAxis2, m_JointAxis1));
	const Maths::HVec3 _jointAxis2(Maths::Select(scalarFlip, m_JointAxis1, m_JointAxis2));

	const Maths::Quaternion rotationOffset(Maths::Select(scalarFlip, !m_rotationalOffSet, m_rotationalOffSet));

	BIZ_ASSERT(!jaco->m_pSpuConstraintBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject));

	Solver2::C_RevoluteJointJacobian* revoluteJ=(Solver2::C_RevoluteJointJacobian*)jaco;

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/

	SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;

	const HMat33 mat1(pRigid1->m_OrientationMatrix);
	const HMat33 mat2(pRigid2->m_OrientationMatrix);

	//const Vector3 jointAxis1(_jointAxis1 * mat1); //rigidbody1's joint axis in world space
	const Vector3 jointAxis2(_jointAxis2 * mat2); //rigidbody2's joint axist in world space


	/*const VF32 resVF(jointAxis2 | jointAxis2);
	const Vector3 res(MakeVector3(resVF, resVF, resVF));
	const Normal3 temp2(Normalise(jointAxis2 - res));
	const Normal3 t1(Normalise(temp2^jointAxis2));
	const Normal3 t2(Normalise(t1^jointAxis2));*/

    #if 0
	const Maths::HVec3 ja2(jointAxis2);
	Maths::HVec3 u(Maths::UNDEFINED), v(Maths::UNDEFINED);
	ja2.PerpendicularAxes(u, v);
	const Vector3 t1(u);
	const Vector3 t2(v);
	#else
	Normal3 t1, t2, t3;
	PerpendicularAxes( jointAxis2, &t1, &t2, &t3 );
	#endif

	//const HMat33 mat(ja2, u, v);



	//const Vector3 position2=pRigid2->m_Position;
	//const Vector3 position1=pRigid1->m_Position;
	const Quaternion rot1(pRigid1->m_Orientation);
	const Quaternion rot2(pRigid2->m_Orientation);

	
	//Vector3 rotationalErr(jointAxis2 ^ jointAxis1); //jointAxis1 X jointAxis2
	//const VF32 two(Const<VF32>::Two());
	const Quaternion targetB( rotationOffset * rot1);
	const Quaternion relQuaternion(Maths::NearestDifference(rot2, targetB));
	Normalise(relQuaternion);
	//const Vector3 rotationalErr(Imag(relQuaternion)* two); 
	const Normal3 imag(Normalise(Imag(relQuaternion)));

	/*const VF32 zeroVF(Const<VF32>::Zero());
	const VF32 pi(Const<VF32>::Pi());
	const VF32 twoPi(Const<VF32>::TwoPi());*/
	VF32 angle = RotationAngle( relQuaternion );
	//angle = Select( angle <= pi, angle, angle - twoPi );
	//angle = Select( (t3|imag) >= zeroVF, -angle, angle );
	
	const Vector3 rotationalErr(imag* angle);

	
	const VF32 err0=Dot(t1, rotationalErr);
	const VF32 err1=Dot(t2, rotationalErr);

	revoluteJ->_m_Error=MakeVector2(err0, err1);


	const Vector3 zeroVec3(Const<Vector3>::Zero());
	
	const Vector6 row0=MakeVector6(zeroVec3, t1);
	const Vector6 row1=MakeVector6(zeroVec3, t2);
	/*SetRow<0>(&revoluteJ->_m_J1, row0);
	SetRow<1>(&revoluteJ->_m_J1, row1);*/
	//SetRow<2>(&revoluteJ->_m_J1, Const<Vector6>::Zero());

	revoluteJ->_m_J1 = MakeMat26(row0, row1);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(HMat43(mat, pRigid2->m_Position), 1.f);
	

	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	{
		revoluteJ->_m_J2=Const<Mat26>::Zero();
	}
	else
	{
		const Vector6 row0=MakeVector6(zeroVec3, (-t1));
		const Vector6 row1=MakeVector6(zeroVec3, (-t2));

		/*SetRow<0>(&revoluteJ->_m_J2, row0);
		SetRow<1>(&revoluteJ->_m_J2, row1);*/
		revoluteJ->_m_J2 = MakeMat26(row0, row1);
		//SetRow<2>(&revoluteJ->_m_J2, Const<Vector6>::Zero());
	}
}

void C_RevoluteConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
{
	using namespace Maths;

	//Spu::MemBaseMarker marker;

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


	Solver2::C_RevoluteJointJacobian* revoluteJ=(Solver2::C_RevoluteJointJacobian*) j;
	Solver2::C_RevoluteJointInvertMJT* revoluteIMJT=(Solver2::C_RevoluteJointInvertMJT*)imjt;

	revoluteIMJT->m_Scale = MakeVF32(m_Scale);

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/
	SharedPhysics::C_RigidBody* pRigid1=&imjt->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2=&imjt->m_pSpuConstraintBody2->m_RigidBody;
	
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

	const Vector2 row0=Col<0>(revoluteJ->_m_J1) * mV1;
	const Vector2 row1=Col<1>(revoluteJ->_m_J1) * mV1;
	const Vector2 row2=Col<2>(revoluteJ->_m_J1) * mV1;

	const Vector3 jRow01=ExtractHiVector3(Row<0>(revoluteJ->_m_J1));
	const Vector3 jRow11=ExtractHiVector3(Row<1>(revoluteJ->_m_J1));
	//const Vector3 jRow12=ExtractHiVector3(Row<2>(revoluteJ->_m_J1));

	const VF32 row30=Dot(x, jRow01);
	const VF32 row31=Dot(x, jRow11);

	const VF32 row40=Dot(y, jRow01);
	const VF32 row41=Dot(y, jRow11);

	const VF32 row50=Dot(z, jRow01);
	const VF32 row51=Dot(z, jRow11);
	

	const Vector2 row3=MakeVector2(row30, row31);
	const Vector2 row4=MakeVector2(row40, row41);
	const Vector2 row5=MakeVector2(row50, row51);


	//SetRow<0>(&revoluteIMJT->_m_IMJT1, row0);
	//SetRow<1>(&revoluteIMJT->_m_IMJT1, row1);
	//SetRow<2>(&revoluteIMJT->_m_IMJT1, row2);
	//SetRow<3>(&revoluteIMJT->_m_IMJT1, row3);
	//SetRow<4>(&revoluteIMJT->_m_IMJT1, row4);
	//SetRow<5>(&revoluteIMJT->_m_IMJT1, row5);

	revoluteIMJT->_m_IMJT1 = MakeMat62(row0, row1, row2, row3, row4, row5);

	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		revoluteIMJT->_m_IMJT2=Const<Mat62>::Zero();

	}
	else
	{
		const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);

		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());

		const Vector2 row0=Col<0>(revoluteJ->_m_J2) * mV2;
		const Vector2 row1=Col<1>(revoluteJ->_m_J2) * mV2;
		const Vector2 row2=Col<2>(revoluteJ->_m_J2) * mV2;

		const Vector3 jRow01=ExtractHiVector3(Row<0>(revoluteJ->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(revoluteJ->_m_J2));
		//const Vector3 jRow12=ExtractHiVector3(Row<2>(revoluteJ->_m_J2));
													
		const VF32 row30=Dot(x2, jRow01);
		const VF32 row31=Dot(x2, jRow11);
		//const VF32 row32=Dot(x2, jRow12);

		const VF32 row40=Dot(y2, jRow01);
		const VF32 row41=Dot(y2, jRow11);
		//const VF32 row42=Dot(y2, jRow12);
		
		const VF32 row50=Dot(z2, jRow01);
		const VF32 row51=Dot(z2, jRow11);
		//const VF32 row52=Dot(z2, jRow12);
		
		const Vector2 row3=MakeVector2(row30, row31);
		const Vector2 row4=MakeVector2(row40, row41);
		const Vector2 row5=MakeVector2(row50, row51);


		/*SetRow<0>(&revoluteIMJT->_m_IMJT2, row0);
		SetRow<1>(&revoluteIMJT->_m_IMJT2, row1);
		SetRow<2>(&revoluteIMJT->_m_IMJT2, row2);
		SetRow<3>(&revoluteIMJT->_m_IMJT2, row3);
		SetRow<4>(&revoluteIMJT->_m_IMJT2, row4);
		SetRow<5>(&revoluteIMJT->_m_IMJT2, row5);*/

		revoluteIMJT->_m_IMJT2 = MakeMat62(row0, row1, row2, row3, row4, row5);

	}
}

};