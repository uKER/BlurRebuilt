#include "stdafx.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/LCP.h"


//using namespace Maths;
//using namespace SharedPhysics;

namespace SharedPhysics
{
	void C_Constraint::StoreWarmstartValues(Maths::VF32 PCREF lambda)
	{
		const Maths::VF32 zero(Maths::Const<Maths::VF32>::Zero());
		const Maths::VF32 newLambda(Maths::Select(Maths::Finite(lambda), lambda, zero));
		m_WarmStartValues = Maths::MakeVector8(newLambda);
	}

	void C_Constraint::StoreWarmstartValues(Maths::Vector2 PCREF lambda)
	{
		const Maths::Vector4 zero(Maths::Const<Maths::Vector4>::Zero());
		const Maths::Vector4 newLambda(Maths::Select(Maths::Finite(lambda), Maths::MakeVector4(lambda.m_V), zero));
		m_WarmStartValues = Maths::MakeVector8(newLambda, newLambda);
	}

	void C_Constraint::StoreWarmstartValues(Maths::Vector3 PCREF lambda)
	{
		const Maths::Vector4 zero(Maths::Const<Maths::Vector4>::Zero());
		const Maths::Vector4 newLambda(Maths::Select(Maths::Finite(lambda), Maths::MakeVector4(lambda), zero));
		m_WarmStartValues = Maths::MakeVector8(newLambda, newLambda);
	}

	void C_Constraint::StoreWarmstartValues(Maths::Vector4 PCREF lambda)
	{
		const Maths::Vector4 zero(Maths::Const<Maths::Vector4>::Zero());
		const Maths::Vector4 newLambda(Maths::Select(Maths::Finite(lambda), lambda, zero));
		m_WarmStartValues = Maths::MakeVector8(newLambda, newLambda);
	}

	void C_Constraint::StoreWarmstartValues(Maths::Vector6 PCREF lambda)
	{
		const Maths::Vector8 zero(Maths::Const<Maths::Vector8>::Zero());
		const Maths::Vector8 newLambda(Maths::Select(Maths::Finite(lambda), Maths::MakeVector8(lambda), zero));
		m_WarmStartValues = newLambda;
	}

	void C_Constraint::StoreWarmstartValues(Maths::Vector8 PCREF lambda)
	{
		const Maths::Vector8 zero(Maths::Const<Maths::Vector8>::Zero());
		const Maths::Vector8 newLambda(Maths::Select(Maths::Finite(lambda), lambda, zero));
		m_WarmStartValues = newLambda;
	}

	/*void C_Constraint::ZeroWarmStartValues()
	{
		m_WarmStartValues = Maths::Const<Maths::Vector8>::Zero();
	}*/
}


namespace Solver2
{
void C_ContactConstraint::CreateJacobian(Solver2::C_Jacobian* jaco)
{

	using namespace Maths;
	BIZ_ASSERT(jaco->m_pSpuConstraintBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia) == false);
	const VF32 one(Maths::Const<VF32>::One());
	jaco->m_Baumgarte = m_Baumgarte;
	jaco->m_NodeBFlag = jaco->m_pSpuConstraintBody2->m_GraphNode.m_NodeFlags;

	Solver2::C_ContactJacobian* contact = (Solver2::C_ContactJacobian*)jaco;

	contact->m_StaticConstraint = false;
	contact->m_pConstraint = m_pPPUConstraint;

	SharedPhysics::C_RigidBody* pRigidBody1 = &jaco->m_pSpuConstraintBody1->m_RigidBody;
	SharedPhysics::C_RigidBody* pRigidBody2 = &jaco->m_pSpuConstraintBody2->m_RigidBody;

	

	const Normal3 normal=m_Normal;
	const Vector3 contactPoint(m_ContactPoint);
	const Vector3 position1(pRigidBody1->CenterOfRotation());//CenterOfRotation());
	const Vector3 position2(pRigidBody2->CenterOfRotation());//CenterOfRotation());
	const Vector3 r1(contactPoint - position1);
	const Vector3 r2(contactPoint - position2);

	const Vector3 lVelocity1(pRigidBody1->m_LinearVelocity);
	const Vector3 lVelocity2(pRigidBody2->m_LinearVelocity);
	const Vector3 aVelocity1(pRigidBody1->m_AngularVelocity);
	const Vector3 aVelocity2(pRigidBody2->m_AngularVelocity);

	const Vector3 velocity1(lVelocity1 + (r1^aVelocity1) );
	const Vector3 velocity2(lVelocity2 + (r2^aVelocity2) );
	
	const Vector3 relativeVelocity((velocity1 - velocity2));// + eps);// + Maths::Vector3(Maths::Constants<Maths::HVec3>::Epsilon2()));
	const VF32 resVF(relativeVelocity |normal);
	//const Vector3 res=MakeVector3(resVF);
	const Vector3 res= normal * resVF;

	
	const Vector3 temp20(relativeVelocity - res);
	//No need to normalise - we normalise the result afterwards!!!
	//const Normal3 temp2(Normalise(temp20));
	const Vector3 tV1(Maths::MakeVector3(Normalise(normal^temp20)));
	const Vector3 tV2(normal^tV1);
	//Maths::Store(GetFriction(), &contact->m_Coefficient);
	//contact->m_CoefficientSetting.X(GetFriction());
	const Maths::Vector3 friction(GetFriction());

	const Vector3 t1Sq(tV1 * tV1);
	const Vector3 t2Sq(tV2 * tV2);
	const VF32 frictionU(friction | t1Sq);
	const VF32 frictionV(friction | t2Sq);


	//SetX(&contact->m_CoefficientSetting, (Maths::VF32)GetFriction());
	//Maths::Store(m_PenetrationDepth, &jaco->m_Error[0]);//=this->m_PenetrationDepth;
	//Maths::Store(GetRestitution(), &contact->m_CoefficientOfResititution);// = this->m_Restitution;

	const HF32 hZero(Maths::Const<VF32>::Zero());
	const VF32 zero(Maths::Const<VF32>::Zero());
	const VF32 threshold(SharedPhysics::Constants::RestitutionVelocityThreshold());
	const VF32 _restitution(GetRestitution());
	const Maths::VF32 normSpeed(relativeVelocity | normal);
	const VF32 restitution = Maths::Select(Abs(normSpeed) < threshold, zero, _restitution);

	//Maths::Store(restitution, &contact->m_CoefficientOfRestitution);// = this->m_Restitution;
	//contact->m_CofRSetting = 1.f + contact->m_CoefficientOfResititution;

	contact->m_CoefficientSetting = Maths::MakeVector4(frictionU, frictionV, restitution, one + restitution);
	/*SetZ(&contact->m_CoefficientSetting, restitution);
	SetW(&contact->m_CoefficientSetting, one + restitution);*/
	//contact->m_CoefficientSetting.Y(restitution);
	//contact->m_CoefficientSetting.Z(Constants<HF32>::One() + contact->m_CoefficientSetting.Y());

	contact->_m_Error=GetPenetrationDepth();

	/*const Vector3 tV1=t1);
	const Vector3 tV2=MakeVector3(t2);*/
	
	
	
	const Vector3 nT1(-tV1);
	const Vector3 nT2(-tV2);
	const Normal3 nNormal(-normal);
	const Vector3 nR1Xn(nNormal ^ r1); //-r1Xn
	const Vector3 nR1Xt1(r1^tV1);    //-r1Xt1
	const Vector3 nR1Xt2(r1^tV2);    //-r1Xt2


	Vector6 row1=MakeVector6(nNormal, nR1Xn);
	Vector6 row2=MakeVector6(nT1, nR1Xt1);
	Vector6 row3=MakeVector6(nT2, nR1Xt2);
	contact->_m_J1=MakeMat36(row1, row2, row3);


	/*
		If the second object is static, we need to sent it corresponding jacobian matrix to zero,
		otherwise initialize it into the correct value
	*/

	////Q: Do we actually need this now? Perhaps not!
	//contact->m_StaticConstraint = (GetBodyAMassScale().EQ(hZero) | GetBodyBMassScale().EQ(hZero));
		


	if(jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	{
		BIZ_ASSERT(!jaco->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
		contact->_m_J2=Maths::Const<Maths::Mat36>::Zero();
		contact->m_StaticConstraint = true;
	}
	else
	{
		const Vector3 r2Xn(normal^r2); //r2Xn
		const Vector3 r2Xt1(tV1^r2);	     //r2Xt1
		const Vector3 r2Xt2(tV2^r2);		 //r2Xt2

		const Vector6 row1=MakeVector6(normal, r2Xn);
		const Vector6 row2=MakeVector6(tV1, r2Xt1);
		const Vector6 row3=MakeVector6(tV2, r2Xt2);

		contact->_m_J2=MakeMat36(row1, row2, row3);
	}

}


void C_ContactConstraint::CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* j)
{

	using namespace Maths;
	imjt->m_NodeBFlag = j->m_pSpuConstraintBody2->m_GraphNode.m_NodeFlags;
	
	const SharedPhysics::C_RigidBody* pRigid1 = &j->m_pSpuConstraintBody1->m_RigidBody;
	const SharedPhysics::C_RigidBody* pRigid2 = &j->m_pSpuConstraintBody2->m_RigidBody;

	const HF32 massScaleA(GetBodyAMassScale());
	const HF32 massScaleB(GetBodyBMassScale());
	const HF32 inertiaScaleA(GetBodyAInertiaScale());
	const HF32 inertiaScaleB(GetBodyBInertiaScale());

	const Maths::HMat33 scaledInverseInertiaA(pRigid1->GetApproxScaledInverseInertiaTensor(inertiaScaleA));
	const Vector3 x(scaledInverseInertiaA.AX());
	const Vector3 y(scaledInverseInertiaA.AY());
	const Vector3 z(scaledInverseInertiaA.AZ());

	Solver2::C_ContactInvertMJT* imjtN=(Solver2::C_ContactInvertMJT*)imjt;
	
	Solver2::C_ContactJacobian* jacoN=(Solver2::C_ContactJacobian*)j;

	const VF32 mV(HF32(pRigid1->ReciprocalMass() * massScaleA));

	const Vector3 row0=Col<0>(jacoN->_m_J1)*mV;
	const Vector3 row1=Col<1>(jacoN->_m_J1)*mV;
	const Vector3 row2=Col<2>(jacoN->_m_J1)*mV;


	const Vector3 jRow01=ExtractHiVector3(Row<0>(jacoN->_m_J1));
	const Vector3 jRow11=ExtractHiVector3(Row<1>(jacoN->_m_J1));
	const Vector3 jRow21=ExtractHiVector3(Row<2>(jacoN->_m_J1));

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

	imjtN->_m_IMJT1 = MakeMat63(row0, row1, row2, row3, row4, row5);

	
	/*SetRow<0>(&imjtN->_m_IMJT1, row0);
	SetRow<1>(&imjtN->_m_IMJT1, row1);
	SetRow<2>(&imjtN->_m_IMJT1, row2);
	SetRow<3>(&imjtN->_m_IMJT1, row3);
	SetRow<4>(&imjtN->_m_IMJT1, row4);
	SetRow<5>(&imjtN->_m_IMJT1, row5);*/


	/*
		If the second object is static, we need to sent it corresponding jacobian matrix to zero,
		otherwise initialize it into the correct value
	*/
	if(!imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	{
		imjtN->_m_IMJT2=Maths::Const<Maths::Mat63>::Zero();
	}
	else
	{

		const VF32 m2(Maths::HF32(pRigid2->ReciprocalMass() * massScaleB));
		const Maths::HMat33 scaledInverseInertiaB(pRigid2->GetApproxScaledInverseInertiaTensor(inertiaScaleB));
		const Vector3 x2(scaledInverseInertiaB.AX());
		const Vector3 y2(scaledInverseInertiaB.AY());
		const Vector3 z2(scaledInverseInertiaB.AZ());
		
		const Vector3 row0=Col<0>(jacoN->_m_J2) * m2;
		const Vector3 row1=Col<1>(jacoN->_m_J2) * m2;
		const Vector3 row2=Col<2>(jacoN->_m_J2) * m2;

		const Vector3 jRow01=ExtractHiVector3(Row<0>(jacoN->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(jacoN->_m_J2));
		const Vector3 jRow21=ExtractHiVector3(Row<2>(jacoN->_m_J2));
	
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

		imjtN->_m_IMJT2 = MakeMat63(row0, row1, row2, row3, row4, row5);
	}
	
	/*imjtN->_m_IMJT2=Maths::Const<Maths::Mat63>::Zero();
	if(imjt->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	{
		const VF32 m2(Maths::HF32(pRigid2->m_ReciprocalMass * massScaleB));
		const Vector3 x2(Maths::HVec3(pRigid2->m_InverseInertiaTensor.AX() * inertiaScaleB.X()));
		const Vector3 y2(Maths::HVec3(pRigid2->m_InverseInertiaTensor.AY() * inertiaScaleB.Y()));
		const Vector3 z2(Maths::HVec3(pRigid2->m_InverseInertiaTensor.AZ() * inertiaScaleB.Z()));
		
		const Vector3 row0=Maths::Col<0>(jacoN->_m_J2) * m2;
		const Vector3 row1=Maths::Col<1>(jacoN->_m_J2) * m2;
		const Vector3 row2=Maths::Col<2>(jacoN->_m_J2) * m2;

		const Vector3 jRow01=ExtractHiVector3(Row<0>(jacoN->_m_J2));
		const Vector3 jRow11=ExtractHiVector3(Row<1>(jacoN->_m_J2));
		const Vector3 jRow21=ExtractHiVector3(Row<2>(jacoN->_m_J2));
	
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

		SetRow<0>(&imjtN->_m_IMJT2, row0);
		SetRow<1>(&imjtN->_m_IMJT2, row1);
		SetRow<2>(&imjtN->_m_IMJT2, row2);
		SetRow<3>(&imjtN->_m_IMJT2, row3);
		SetRow<4>(&imjtN->_m_IMJT2, row4);
		SetRow<5>(&imjtN->_m_IMJT2, row5);

	}*/
}


};
