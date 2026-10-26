#include "stdafx.h"
#include "gravity\sdk\RagdollController.h"
#include "gravity\sdk\ConstraintDescriptors.h"
#include <Core/Maths/HVec/New/IncludePhysics.h>
#include <Core/Maths/HVec/New/IncludePhysics.inl>
#include "TestPhysics.h"

namespace Samples
{
	Debug::Float g_dbScale("Test.RagdollJointScale", 1.f, Debug::Range(0.f, 1.f));

	void C_TestPhysics::InitialiseFixedJointRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		using namespace Maths;

		Mat33 rot = Const< Mat33 >::Identity();
	
		//
		Quaternion orientation = MakeUnitQuaternion(rot);
		//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
		const VF32 mass(Maths::Constants<Maths::HF32>::Three());

		const Vector3 head = MakeVector3(0.f, 0.3f, 0.f) + (Vector3)position;

		//Gravity::C_Body* pHead(AddBox(Maths::HVec3(0.f, 1.5f, 0.f) + position, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), Maths::HF32(2.f), pFilter));
		Gravity::C_Body* pHead(AddSphere(HVec3(head), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter));
		//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter(), Maths::Constants<Maths::HF32>::Quarter());
		

		//Gravity::C_Body* pHead(AddBox(HVec3(head), rot, HVec3(0.05f, 0.05f, 0.05f), HF32(2.f), pFilter));
		//pHead->MakeStatic();
		
		const Vector3 torso = (Vector3)position;
		const Maths::HVec3 extentsTorso(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
		//Gravity::C_Body* pTorso(AddBox(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HVec3(1.f, 1.f, 1.f), Maths::HF32(5.f), pFilter ));
		Gravity::C_Body* pTorso(AddCapsule(HVec3(torso), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.2f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pTorso(AddSphere(HVec3(torso),  Maths::HF32(0.2f),  Maths::HF32(10.f), pFilter ));
		//Gravity::C_Body* pTorso(AddBox(HVec3(torso),  rot,  HVec3(0.1f, 0.2f, 0.1f), Maths::HF32(10.f), pFilter ));
		

		Mat33 rot_90 = Const< Mat33 >::RotateZ90CW();
		////RotZ rot_90(Const<VF32>::Pi()*Const<VF32>::Half());
		orientation = MakeUnitQuaternion(rot_90);

		const Vector3 hip = MakeVector3(0.f, -0.45f, 0.f) + (Vector3)position;
		////Gravity::C_Body* pHip(AddBox(Maths::HVec3(0.f, -2.f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f,1.f), Maths::HF32(1.f), pFilter));
		//Gravity::C_Body* pHip(AddCapsule(HVec3(hip),  C_Quaternion(orientation), Maths::HF32(0.1f), Maths::HF32(0.08f), Maths::HF32(2.f), pFilter));
		////pHip->MakeStatic();
		//
		////Gravity::C_Body* pLeftArmTop(AddBox(Maths::HVec3(-2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//
		const Vector3 leftArmTop = MakeVector3(-0.41f, 0.2f, 0.f) + (Vector3)position;
		Gravity::C_Body* pLeftArmTop(AddCapsule(HVec3(leftArmTop), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		////pLeftArmTop->MakeStatic();

		////Gravity::C_Body* pLeftArmBottom(AddBox(Maths::HVec3(-4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//const Vector3 leftArmBottom = MakeVector3(-0.82f, 0.15f, 0.f) + (Vector3)position;
		//Gravity::C_Body* pLeftArmBottom(AddCapsule(HVec3(leftArmBottom), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pLeftArmBottom->MakeStatic();

		//const Vector3 rightArmTop = MakeVector3(0.41f, 0.2f, 0.f) + (Vector3)position;
		//Gravity::C_Body* pRightArmTop(AddCapsule(HVec3(rightArmTop), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pRightArmTop->MakeStatic();

		//const Vector3 rightArmBottom = MakeVector3(0.82f, 0.15f, 0.f) + (Vector3)position;
		//////Gravity::C_Body* pRightArmBottom(AddBox(Maths::HVec3(4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightArmBottom(AddCapsule(HVec3(rightArmBottom), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pRightArmBottom->MakeStatic();

		//orientation = MakeUnitQuaternion(rot);

		//const Vector3 leftLegTop = MakeVector3(-0.1f, -0.75f, 0.f) + (Vector3)position;
		//////Gravity::C_Body* pLeftLegTop(AddBox(Maths::HVec3(-0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pLeftLegTop(AddCapsule(HVec3(leftLegTop), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pLeftLegTop->MakeStatic();

		//const Vector3 leftLegBottom = MakeVector3(-0.1f, -1.15f, 0.f) + (Vector3)position;
		//////Gravity::C_Body* pLeftLegBottom(AddBox(Maths::HVec3(-0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pLeftLegBottom(AddCapsule(HVec3(leftLegBottom), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pLeftLegBottom->MakeStatic();

		//const Vector3 rightLegTop = MakeVector3(0.1f, -0.75f, 0.f) + (Vector3)position;
		//////Gravity::C_Body* pRightLegTop(AddBox(Maths::HVec3(0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightLegTop(AddCapsule(HVec3(rightLegTop), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pRightLegTop->MakeStatic();

		//const Vector3 rightLegBottom = MakeVector3(0.1f, -1.15f, 0.f) + (Vector3)position;
		//////Gravity::C_Body* pRightLegBottom(AddBox(Maths::HVec3(0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightLegBottom(AddCapsule(HVec3(rightLegBottom), C_Quaternion(orientation), Maths::HF32(0.2f), Maths::HF32(0.1f), Maths::HF32(1.f), pFilter ));
		//////pRightLegBottom->MakeStatic();

		//Gravity::FixedJointDescriptor desc;
		Gravity::SphericalJointDescriptor desc;
		//Gravity::RevoluteJointDescriptor desc;
	
		
		/*Maths::HVec3 jointAxis1 = (HVec3(head) - HVec3(torso))* pTorso->GetBody().m_OrientationMatrix.InverseRotation();
		jointAxis1.Normalise();
		Maths::HVec3 jointAxis2 = (HVec3(torso) - HVec3(head))*pHead->GetBody().m_OrientationMatrix.InverseRotation();
		jointAxis2.Normalise();*/

		SharedPhysics::C_Constraint* constraint(NULL);
		desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.07f, 0.f), Maths::HVec3(0.f, 0.23f, 0.f));
		//desc.DefineJointAxis(jointAxis1, jointAxis2);
		//desc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHead, pTorso);
		//m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);


		desc.DefineAnchorPoints(Maths::HVec3(-0.21f, 0.15f, 0.f), pLeftArmTop->Orientation() * Maths::HVec3(0.21f, 0.f, 0.f));
		constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
		////m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(pLeftArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f), pLeftArmBottom->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(Maths::HVec3(0.21f, 0.15f, 0.f), pRightArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);


		//desc.DefineAnchorPoints(pRightArmTop->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f), pRightArmBottom->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.31f, 0.f), pHip->Orientation()*Maths::HVec3(0.f, 0.085f, 0.f));
		//
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(-0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		////
		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);

		//desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		//constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
		//////m_Constraints.AddToEnd(&constraint);
		//m_pScene->AddConstraint(constraint);


	//	using namespace Maths;

	//	Mat33 rot = Const< Mat33 >::Identity();
	//	
	//	Maths::RotX _rot(0.707f);
	//	HMat33 headRot(_rot);
	//	
	//	
	//	Quaternion orientation = MakeUnitQuaternion(rot);
	//	//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
	//	const VF32 mass(Maths::Constants<Maths::HF32>::Three());

	//	const HVec3 head = HVec3(0.f, 0.3f, 0.f) + position;

	//	Gravity::C_Body* pHead(AddBox(head, headRot, HVec3(0.05f, 0.05f, 0.05f), HF32(1.f), pFilter));
	//	//pHead->MakeStatic();
	//	
	//	const HVec3 torso =position;
	//	const Maths::HVec3 extentsTorso(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
	//	Gravity::C_Body* pTorso(AddBox(torso,  rot,  HVec3(0.1f, 0.2f, 0.1f), Maths::HF32(10.f), pFilter ));
	//	//pTorso->MakeStatic();
	//	

	//	const HVec3 hip = HVec3(0.f, -0.3f, 0.f) + position;
	//	Gravity::C_Body* pHip(AddBox(hip, rot, HVec3(0.1f, 0.05f,0.1f),HF32(1.f), pFilter));
	//	//pHip->MakeStatic();
	//	
	//	const HVec3 leftArmTop =HVec3(-0.25f, 0.15f, 0.f) + position;
	//	Gravity::C_Body* pLeftArmTop(AddBox(leftArmTop, rot, HVec3(0.1f, 0.05f, 0.05f),  HF32(1.f), pFilter));
	//	//pLeftArmTop->MakeStatic();

	//	const HVec3 leftArmBottom = HVec3(-0.5f, 0.15f, 0.f) + position;
	//	Gravity::C_Body* pLeftArmBottom(AddBox(leftArmBottom, rot, HVec3(0.1f, 0.05f, 0.05f), HF32(1.f), pFilter));
	//	//pLeftArmBottom->MakeStatic();

	//	const HVec3 rightArmTop = HVec3(0.25f, 0.15f, 0.f) + position;
	//	Gravity::C_Body* pRightArmTop(AddBox(rightArmTop, rot, HVec3(0.1f, 0.05f, 0.05f), HF32(1.f), pFilter));
	//	//pRightArmTop->MakeStatic();

	//	const HVec3 rightArmBottom = HVec3(0.5f, 0.15f, 0.f) + position;
	//	Gravity::C_Body* pRightArmBottom(AddBox(rightArmBottom, rot, HVec3(0.1f, 0.05f, 0.05f), HF32(1.f), pFilter));
	//	//pRightArmBottom->MakeStatic();
	//
	//	const HVec3 leftLegTop = HVec3(-0.1f, -0.5f, 0.f) + position;
	//	Gravity::C_Body* pLeftLegTop(AddBox(leftLegTop, rot, HVec3(0.05f, 0.1f, 0.05f), HF32(1.f), pFilter));
	//	//pLeftLegTop->MakeStatic();

	//	const HVec3 leftLegBottom = HVec3(-0.1f, -0.75f, 0.f) + position;
	//	////Gravity::C_Body* pLeftLegBottom(AddBox(Maths::HVec3(-0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
	//	Gravity::C_Body* pLeftLegBottom(AddBox(leftLegBottom, rot, HVec3(0.05f, 0.1f, 0.05f), Maths::HF32(1.f), pFilter ));
	//	//pLeftLegBottom->MakeStatic();

	//	const HVec3 rightLegTop = HVec3(0.1f, -0.5f, 0.f) + position;
	//	////Gravity::C_Body* pRightLegTop(AddBox(Maths::HVec3(0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
	//	Gravity::C_Body* pRightLegTop(AddBox(rightLegTop, rot, HVec3(0.05f, 0.1f, 0.05f), Maths::HF32(1.f), pFilter ));
	//	//pRightLegTop->MakeStatic();

	//	const HVec3 rightLegBottom = HVec3(0.1f, -0.75f, 0.f) + position;
	//	Gravity::C_Body* pRightLegBottom(AddBox(rightLegBottom, rot, HVec3(0.05f, 0.1f, 0.05f), Maths::HF32(1.f), pFilter ));
	//	
	//	Gravity::FixedJointDescriptor desc;
	//	desc.SetScale(g_dbScale);

	//	desc.DefineAnchorPoints(headRot * Maths::HVec3(0.f, -0.075f, 0.f), Maths::HVec3(0.f, 0.225f, 0.f));
	////	desc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
	//	SharedPhysics::C_Constraint* constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHead, pTorso);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);


	//	desc.DefineAnchorPoints(Maths::HVec3(-0.125f, 0.15f, 0.f), Maths::HVec3(0.125f, 0.f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
	//	constraint->NormaliseMassRatios(2.f, 100.f);

	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(-0.125f, 0.f, 0.f), Maths::HVec3(0.125f, 0.f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(0.125f, 0.15f, 0.f), Maths::HVec3(-0.125f, 0.f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);


	//	desc.DefineAnchorPoints(Maths::HVec3(0.15f, 0.f, 0.f), Maths::HVec3(-0.125f, 0.f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.225f, 0.f), Maths::HVec3(0.f, 0.075f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(-0.1f, -0.075f, 0.f), Maths::HVec3(0.f, 0.125f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.125f, 0.f), Maths::HVec3(0.f, 0.125f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.075f, 0.f), Maths::HVec3(0.f, 0.125f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);

	//	desc.DefineAnchorPoints(Maths::HVec3(0.f, -0.125f, 0.f), Maths::HVec3(0.f, 0.125f, 0.f));
	//	constraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
	//	constraint->NormaliseMassRatios(2.f, 100.f);
	//	m_pScene->AddConstraint(constraint);
		
	}

}
