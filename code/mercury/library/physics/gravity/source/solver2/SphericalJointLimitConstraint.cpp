#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


namespace Solver2
{


void C_SphericalLimitConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{
	jaco->m_pConstraint = m_pPPUConstraint;

	using namespace Maths;

	jaco->m_Baumgarte = m_Baumgarte;

	/*Spu::MemBaseMarker marker;
	SharedPhysics::C_GraphNode* _graphNode1 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* _graphNode2 = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(jaco->m_pRigid2, sizeof(SharedPhysics::C_GraphNode)); */

	SharedPhysics::C_GraphNode* _graphNode1 = &jaco->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* _graphNode2 = &jaco->m_pSpuConstraintBody2->m_GraphNode;

	bool bFlip = FlipConstraint(_graphNode1, _graphNode2, jaco->m_NodeBFlag);

	/*SharedPhysics::C_GraphNode* graphNodeA = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode2, _graphNode1);
	SharedPhysics::C_GraphNode* graphNodeB = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, _graphNode1, _graphNode2);*/

	const Maths::Scalar_Bool scalarFlip(bFlip);

	/*SharedPhysics::C_GraphNode* pNodeA = jaco->m_pRigid1;
	jaco->m_pRigid1 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, jaco->m_pRigid2, jaco->m_pRigid1);
	jaco->m_pRigid2 = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(bFlip, pNodeA, jaco->m_pRigid2);*/

	Gravity::C_ConstraintBody* pSpuConstraintBody1 = jaco->m_pSpuConstraintBody1;
	jaco->m_pSpuConstraintBody1 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, jaco->m_pSpuConstraintBody2, jaco->m_pSpuConstraintBody1);
	jaco->m_pSpuConstraintBody2 = SharedPhysics::Fast_Select<Gravity::C_ConstraintBody>(bFlip, pSpuConstraintBody1, jaco->m_pSpuConstraintBody2);


	//const Maths::HVec3 _jointAxis1(Maths::Select(scalarFlip, m_JointAxis2, m_JointAxis1));
	//const Maths::HVec3 _jointAxis2(Maths::Select(scalarFlip, m_JointAxis1, m_JointAxis2));

	//const Maths::Quaternion rotationOffset(Maths::Select(scalarFlip, !m_rotationalOffSet, m_rotationalOffSet));
	Maths::Mat33 rotationalOffset;
	if(bFlip)
	{
		rotationalOffset = Maths::Transpose(m_rotationalOffSet);
	}
	else
	{
		rotationalOffset = m_rotationalOffSet;
	}


	BIZ_ASSERT(!jaco->m_pSpuConstraintBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject));

	Solver2::C_SphericalJointLimitJacobian* revoluteJ=(Solver2::C_SphericalJointLimitJacobian*)jaco;

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));*/

	SharedPhysics::C_RigidBody* pRigid1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;

	const Mat33 mat1(pRigid1->m_OrientationMatrix);
	const Mat33 mat2(pRigid2->m_OrientationMatrix);

	//const Vector3 t1 =mat2.AX();//ColX(mat2);
	//const Vector3 t2 =mat2.AY(); //ColY(mat2);
	//const Vector3 t3 =mat2.AZ(); //ColZ(mat2);

	const Vector3 t1 =RowX(mat2);
	const Vector3 t2 =RowY(mat2);
	const Vector3 t3 =RowZ(mat2);


	const Mat33 targetB( rotationalOffset * mat1);
	//const Mat33 relMat( Maths::OrthoNormalise(mat2 * Transpose(targetB)));
	const Mat33 relMat( mat2 * Transpose(targetB));
	

	//VF32 angleX, angleY, angleZ;
	Vector3 angle;

	switch(m_AxisOrder)
	{
	case SharedPhysics::C_Constraint::E_AxisOrder_XYZ:
		{
			const EulerXYZ e(MakeEulerXYZ(relMat));
			/*angleX = Maths::X(e);
			angleY = Maths::Y(e);
			angleZ = Maths::Z(e);*/
			angle = e.m_V;
			break;
		}
	case SharedPhysics::C_Constraint::E_AxisOrder_XZY:
		{
			const EulerXZY e(MakeEulerXZY(relMat));
			/*angleX = Maths::X(e);
			angleY = Maths::Y(e);
			angleZ = Maths::Z(e);*/
			angle = e.m_V;
			break;
		}
	/*case SharedPhysics::C_Constraint::E_AxisOrder_ZXY:*/
	default:
		{
			const EulerZXY e(MakeEulerZXY(relMat));
			/*angleX = Maths::X(e);
			angleY = Maths::Y(e);
			angleZ = Maths::Z(e);*/
			angle = e.m_V;
			break;
		}
	}
	
	//	//The limit is in degree
	/*const Vector3 lowLimit(Maths::Select(scalarFlip, -m_HighJointLimit, m_LowJointLimit));
	const Vector3 highLimit(Maths::Select(scalarFlip, -m_LowJointLimit, m_HighJointLimit));*/
	const Vector3 lowLimit(m_LowJointLimit);
	const Vector3 highLimit(m_HighJointLimit);

	/*const VF32 twoPiR = Const<VF32>::TwoPi();
	const VF32 twoPiD = MakeVF32(360.f);
	const VF32 lowLimitX = (X(lowLimit)/twoPiD)*twoPiR;
	const VF32 lowLimitY = (Y(lowLimit)/twoPiD)*twoPiR;
	const VF32 lowLimitZ = (Z(lowLimit)/twoPiD)*twoPiR;

	const VF32 highLimitX = (X(highLimit)/twoPiD)*twoPiR;
	const VF32 highLimitY = (Y(highLimit)/twoPiD)*twoPiR;
	const VF32 highLimitZ = (Z(highLimit)/twoPiD)*twoPiR;*/

	
	/*const VF32 xLowLimit = X(lowLimit) - angleX;
	const VF32 yLowLimit = Y(lowLimit) - angleY;
	const VF32 zLowLimit = Z(lowLimit) - angleZ;

	const VF32 xHighLimit = angleX - X(highLimit);
	const VF32 yHighLimit = angleY - Y(highLimit);
	const VF32 zHighLimit = angleZ - Z(highLimit);*/

	const Vector3 lowLimitVec = lowLimit - angle;
	const Vector3 highLimitVec = angle - highLimit;


	//revoluteJ->_m_Error=MakeVector6(xLowLimit, xHighLimit, yLowLimit, yHighLimit, zLowLimit, zHighLimit);
	revoluteJ->_m_Error=MakeVector6(X(lowLimitVec), X(highLimitVec), Y(lowLimitVec), Y(highLimitVec), Z(lowLimitVec), Z(highLimitVec));
	//revoluteJ->_m_Error=MakeVector6(lowLimitVec, highLimitVec);


	const Vector3 zeroVec3(Const<Vector3>::Zero());
	
	const Vector6 row0=MakeVector6(zeroVec3,  t1);
	const Vector6 row1=MakeVector6(zeroVec3, -t1);
	const Vector6 row2=MakeVector6(zeroVec3,  t2);
	const Vector6 row3=MakeVector6(zeroVec3, -t2);
	const Vector6 row4=MakeVector6(zeroVec3,  t3);
	const Vector6 row5=MakeVector6(zeroVec3, -t3);


	revoluteJ->_m_J1 = MakeMat66(row0, row1, row2, row3, row4, row5);


	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	{
		revoluteJ->_m_J2=Const<Mat66>::Zero();
	}
	else
	{
		const Vector6 row0=MakeVector6(zeroVec3, -t1);
		const Vector6 row1=MakeVector6(zeroVec3,  t1);
		const Vector6 row2=MakeVector6(zeroVec3, -t2);
		const Vector6 row3=MakeVector6(zeroVec3,  t2);
		const Vector6 row4=MakeVector6(zeroVec3, -t3);
		const Vector6 row5=MakeVector6(zeroVec3,  t3);

		revoluteJ->_m_J2 = MakeMat66(row0, row1, row2, row3, row4, row5);
		
	}

}

void C_SphericalLimitConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
{
	using namespace Maths;

	//Spu::MemBaseMarker marker;

	/*bool bFlip = j->m_pRigid1 != imjt->m_pRigid1;

	imjt->m_pRigid1 = j->m_pRigid1;
	imjt->m_pRigid2 = j->m_pRigid2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;*/

	bool bFlip =(j->m_pSpuConstraintBody1 != imjt->m_pSpuConstraintBody2);
	imjt->m_pSpuConstraintBody1 = j->m_pSpuConstraintBody1;
	imjt->m_pSpuConstraintBody2 = j->m_pSpuConstraintBody2;
	imjt->m_NodeBFlag = j->m_NodeBFlag;

	/*SharedPhysics::C_GraphNode* graphNodeA = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid1, sizeof(SharedPhysics::C_GraphNode));
	SharedPhysics::C_GraphNode* graphNodeB = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(imjt->m_pRigid2, sizeof(SharedPhysics::C_GraphNode));*/
	/*SharedPhysics::C_GraphNode* graphNodeA = &imjt->m_pSpuConstraintBody1->m_GraphNode;
	SharedPhysics::C_GraphNode* graphNodeB = &imjt->m_pSpuConstraintBody2->m_GraphNode;*/

	const Maths::Scalar_Bool scalarFlip(bFlip);

	/*SharedPhysics::C_RigidBody* pRigid1=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeA->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));
	SharedPhysics::C_RigidBody* pRigid2=(SharedPhysics::C_RigidBody*)Spu::FetchAlloc(graphNodeB->m_pRigidBody, sizeof (SharedPhysics::C_RigidBody));*/
	SharedPhysics::C_RigidBody* pRigid1 = &imjt->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigid2 = &imjt->m_pSpuConstraintBody2->m_RigidBody;
	

	Solver2::C_SphericalJointLimitJacobian* sphericalJ=(Solver2::C_SphericalJointLimitJacobian*) j;
	Solver2::C_SphericalJointLimitInvertMJT* sphericalIMJT=(Solver2::C_SphericalJointLimitInvertMJT*) imjt;

	//const VF32 scale = MakeVF32(m_Scale);

	sphericalIMJT->SetScale(MakeVF32(m_Scale));

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

	const Vector6 row0=Col<0>(sphericalJ->_m_J1) * mV1;
	const Vector6 row1=Col<1>(sphericalJ->_m_J1) * mV1;
	const Vector6 row2=Col<2>(sphericalJ->_m_J1) * mV1;
							  

	const Vector3 jRow01=ExtractHiVector3(Row<0>(sphericalJ->_m_J1));
	const Vector3 jRow11=ExtractHiVector3(Row<1>(sphericalJ->_m_J1));
	const Vector3 jRow21=ExtractHiVector3(Row<2>(sphericalJ->_m_J1));
	const Vector3 jRow31=ExtractHiVector3(Row<3>(sphericalJ->_m_J1));
	const Vector3 jRow41=ExtractHiVector3(Row<4>(sphericalJ->_m_J1));
	const Vector3 jRow51=ExtractHiVector3(Row<5>(sphericalJ->_m_J1));


	const VF32 r30=Dot(x, jRow01);
	const VF32 r31=Dot(x, jRow11);
	const VF32 r32=Dot(x, jRow21);
	const VF32 r33=Dot(x, jRow31);
	const VF32 r34=Dot(x, jRow41);
	const VF32 r35=Dot(x, jRow51);

	const VF32 r40=Dot(y, jRow01);
	const VF32 r41=Dot(y, jRow11);
	const VF32 r42=Dot(y, jRow21);
	const VF32 r43=Dot(y, jRow31);
	const VF32 r44=Dot(y, jRow41);
	const VF32 r45=Dot(y, jRow51);

	const VF32 r50=Dot(z, jRow01);
	const VF32 r51=Dot(z, jRow11);
	const VF32 r52=Dot(z, jRow21);
	const VF32 r53=Dot(z, jRow31);
	const VF32 r54=Dot(z, jRow41);
	const VF32 r55=Dot(z, jRow51);

	const Vector6 row3=MakeVector6(r30, r31, r32, r33, r34, r35);
	const Vector6 row4=MakeVector6(r40, r41, r42, r43, r44, r45);
	const Vector6 row5=MakeVector6(r50, r51, r52, r53, r54, r55);
	sphericalIMJT->_m_IMJT1 = MakeMat66(row0, row1, row2, row3, row4, row5);


	/*
		If the second object is static, we need to sent it corresponding jacobian matrix to zero,
		otherwise initialize it into the correct value
	*/
	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
	{
		sphericalIMJT->_m_IMJT2 = Const<Mat66>::Zero();
	}
	else
	{
		const VF32 mV2(pRigid2->ReciprocalMass() * massScaleB);
		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());

		const Vector6 row0=Col<0>(sphericalJ->_m_J2) * mV2;
		const Vector6 row1=Col<1>(sphericalJ->_m_J2) * mV2;
		const Vector6 row2=Col<2>(sphericalJ->_m_J2) * mV2;
								  

		const Vector3 jRow01=ExtractHiVector3(Row<0>(sphericalJ->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(sphericalJ->_m_J2));
		const Vector3 jRow21=ExtractHiVector3(Row<2>(sphericalJ->_m_J2));
		const Vector3 jRow31=ExtractHiVector3(Row<3>(sphericalJ->_m_J2));
		const Vector3 jRow41=ExtractHiVector3(Row<4>(sphericalJ->_m_J2));
		const Vector3 jRow51=ExtractHiVector3(Row<5>(sphericalJ->_m_J2));

		const VF32 r30=Dot(x2, jRow01);
		const VF32 r31=Dot(x2, jRow11);
		const VF32 r32=Dot(x2, jRow21);
		const VF32 r33=Dot(x2, jRow31);
		const VF32 r34=Dot(x2, jRow41);
		const VF32 r35=Dot(x2, jRow51);

		const VF32 r40=Dot(y2, jRow01);
		const VF32 r41=Dot(y2, jRow11);
		const VF32 r42=Dot(y2, jRow21);
		const VF32 r43=Dot(y2, jRow31);
		const VF32 r44=Dot(y2, jRow41);
		const VF32 r45=Dot(y2, jRow51);

		const VF32 r50=Dot(z2, jRow01);
		const VF32 r51=Dot(z2, jRow11);
		const VF32 r52=Dot(z2, jRow21);
		const VF32 r53=Dot(z2, jRow31);
		const VF32 r54=Dot(z2, jRow41);
		const VF32 r55=Dot(z2, jRow51);

		const Vector6 row3=MakeVector6(r30, r31, r32, r33, r34, r35);
		const Vector6 row4=MakeVector6(r40, r41, r42, r43, r44, r45);
		const Vector6 row5=MakeVector6(r50, r51, r52, r53, r54, r55);

		sphericalIMJT->_m_IMJT2 = MakeMat66(row0, row1, row2, row3, row4, row5);



	}
}


};
