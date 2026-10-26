#include "stdafx.h"
#include "ConstraintGraphTest.h"
#include "gravity/solver2/Featherstone.h"
#include <Core/Maths/HVec/New/IncludePhysics.h>
#include <Core/Maths/HVec/New/IncludePhysics.inl>

namespace Samples
{
	Debug::Float g_JointVelocity( "Featherstone.JointVelocity", -10.0f,  Debug::Range(-10.0f, 10.0f) );
	Debug::Float g_JointForce( "Featherstone.JointForce", -50.0f,  Debug::Range(-50.0f, 50.0f) );

	void C_TestPhysics::InitialiseSerialLink(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		using namespace Maths;

		/*m_pScene->m_pMultiBody->SetNumJoints(2);
		m_pScene->m_pMultiBody->SetNumLinks(3);*/
	
		const HF32 linkMass(1.f);

		
		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);

		HVec3 link0(position);
		HMat43 link0Mat(rot, link0);
		
		Gravity::C_Body* pBody0(AddBox(link0, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), linkMass, pFilter));
		
		Solver2::C_Link* plink0 = new Solver2::C_Link;
		plink0->m_pBody = pBody0;
		plink0->m_Type = Solver2::HEAD;
		plink0=m_pScene->m_pMultiBody->AddLink(plink0);
	
		HVec3 link1(HVec3(1.5f, 0.f, 0.f) + position);
		HMat43 link1Mat(rot, link1);
		
		//Gravity::C_Body* pTorso(AddBox(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HVec3(1.f, 1.f, 1.f), Maths::HF32(5.f), pFilter ));
		Gravity::C_Body* pBody1(AddSphere(link1, HF32(0.5f),linkMass, pFilter ));
		
		Solver2::C_Link* plink1 = new Solver2::C_Link;
		plink1->m_pBody = pBody1;
		plink1->m_Type = Solver2::TORSO;
		plink1 = m_pScene->m_pMultiBody->AddLink(plink1);

		/*HVec3 link2(link1 + HVec3(2.5f, 0.f, 0.f));
		HMat43 link2Mat(rot, link2);
		Gravity::C_Body* pBody2(AddSphere(link2, HF32(0.5f), linkMass, pFilter));
		Solver2::C_Link* plink2 = new Solver2::C_Link;
		plink2->m_pBody = pBody2;
		plink2 = m_pScene->m_pMultiBody->AddLink(plink2);*/


		Solver2::C_Joint* joint1 = new Solver2::C_Joint;
		joint1->m_position = HVec3(link1.X() - 0.5f, link1.Y(), link1.Z()) * link1Mat.InverseRotTrans();
		joint1->m_spatialscalarForce = MakeVF32(0.0f);

		//joint->m_d = -joint->m_position;
		joint1->type = Solver2::REVOLUTEJOINT;
		joint1->m_jointAxis = HVec3(0.f, 0.f, 1.f) * link1Mat.Rotation().Inverse();
		joint1->m_scalarVelocity = MakeVF32(0.0f);
		Normalise(joint1->m_jointAxis);
		joint1->IniSpatialJointAxis(-joint1->m_position);
		joint1->m_InBoundD = (-joint1->m_position)*link0Mat.Rotation().Inverse();
		joint1->m_OutBoundD = (-joint1->m_position)*link1Mat.Rotation().Inverse();
		
		joint1=m_pScene->m_pMultiBody->AddJoint(joint1);
		m_pScene->m_pMultiBody->Connect(plink0, plink1, *joint1);


		/*Solver2::C_Joint* joint2 = new Solver2::C_Joint;
		joint2->m_position = HVec3(link2.X() - 0.5f, link2.Y(), link2.Z()) * link2Mat.InverseRotTrans();
		joint2->type = Solver2::REVOLUTEJOINT;
		joint2->m_jointAxis = HVec3(0.0f, 0.f, 1.f) * link2Mat.Rotation().Inverse();
		joint2->m_scalarVelocity = MakeVF32(0.05f);
		Normalise(joint2->m_jointAxis);
		joint2->IniSpatialJointAxis(-joint2->m_position);

		joint2 = m_pScene->m_pMultiBody->AddJoint(joint2);
		m_pScene->m_pMultiBody->Connect(plink1, plink2, *joint2);*/
		

		Solver2::C_Link* pStack[100];
		m_pScene->m_pMultiBody->EnumerateJointIndices(pStack);
		m_pScene->m_pMultiBody->RearrangeJoint();
		m_pScene->m_pMultiBody->InitiLInksSpatialIsolatedInertia();
		pBody0->GetGraphNode()->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		pBody1->GetGraphNode()->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoIntegrate);
		Collision::C_CollisionFilter nullFilter(0, 0, 0, 0, 0);
		pBody0->GetShape()->SetFilter(&nullFilter);
		pBody1->GetShape()->SetFilter(&nullFilter);

		pBody0->MakeStatic();



	}

	void C_TestPhysics::InitiliseFeatherstoneRagdoll(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		using namespace Maths;

		/*m_pScene->m_pMultiBody->SetNumJoints(1);
		m_pScene->m_pMultiBody->SetNumLinks(2);*/
	
		/*const*/ HF32 headMass(0.5f);
		const HF32 torsoMass(2.f);
		const HF32 hipMass(2.f);
		const HF32 armMass(1.f);
		const HF32 legMass(1.f);
		
		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);
		//Quaternion orientationNew(MakeUnitQuaternion((Mat33)rot));
		const HF32 mass(Maths::Constants<HF32>::Three());

	
		Maths::HVec3 torso(position);
		HMat43 torsoMat(rot, position);
		
		
		//Gravity::C_Body* pTorso(AddBox(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HVec3(1.f, 1.f, 1.f), Maths::HF32(5.f), pFilter ));
		Gravity::C_Body* pTorso(AddCapsule(torso, orientation, Maths::HF32(0.2f), Maths::HF32(0.2f), torsoMass, pFilter ));
		
		Solver2::C_Link* pTorsoLink = new Solver2::C_Link;
		pTorsoLink->m_pBody = pTorso;
		pTorsoLink->m_Type = Solver2::TORSO;
		pTorsoLink=m_pScene->m_pMultiBody->AddLink(pTorsoLink);


		HVec3 head(HVec3(0.f, 0.44f, 0.f) + position);
		HMat43 headMat(rot, head);
		
		//Gravity::C_Body* pHead(AddBox(Maths::HVec3(0.f, 1.5f, 0.f) + position, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), Maths::HF32(2.f), pFilter));
		Gravity::C_Body* pHead(AddSphere(head, Maths::HF32(0.1f),headMass, pFilter));
		
		Solver2::C_Link* pHeadLink = new Solver2::C_Link;
		pHeadLink->m_pBody = pHead;
		pHeadLink->m_Type = Solver2::HEAD;
		pHeadLink = m_pScene->m_pMultiBody->AddLink(pHeadLink);


		//Maths::RotZ rot_90(Maths::Constants<Maths::HF32>::Pi()*Maths::Constants<Maths::HF32>::Half());
		//const HMat33 newRot=HMat33(rot_90);
		//orientation = Maths::C_Quaternion(newRot);
		////orientationNew = Maths::C_Quaternion(MakeUnitQuaternion((Mat33)newRot));

		//HVec3 hip(HVec3(0.f, -0.45f, 0.f) + position);
		//HMat43 hipMat(newRot, hip);
	
		//
		//
		////Gravity::C_Body* pHip(AddBox(Maths::HVec3(0.f, -2.f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f,1.f), Maths::HF32(1.f), pFilter));
		//Gravity::C_Body* pHip(AddCapsule(hip, orientation, Maths::HF32(0.1f), Maths::HF32(0.08f), hipMass, pFilter));
		////pHip->MakeStatic();
		//Solver2::C_Link* pHipLink = m_pScene->m_pMultiBody->GetLink(2);
		//pHipLink->m_pBody = pHip;
		//pHipLink->m_Type = Solver2::HIP;

		//
		//
		////Gravity::C_Body* pLeftArmTop(AddBox(Maths::HVec3(-2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//
		//HVec3 leftArmTop(HVec3(-0.41f, 0.2f, 0.f) + position);
		//HMat43 leftArmTopMat(newRot, leftArmTop);
	
		//
		//
		//Gravity::C_Body* pLeftArmTop(AddCapsule(leftArmTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f),armMass, pFilter ));
		////pLeftArmTop->MakeStatic();
		//Solver2::C_Link* pLeftArmTopLink = m_pScene->m_pMultiBody->GetLink(3);
		//pLeftArmTopLink->m_pBody = pLeftArmTop;
	
		//pLeftArmTopLink->m_Type = Solver2::LEFTARMTOP;


		//HVec3 leftArmBottom(HVec3(-0.82f, 0.15f, 0.f) + position);
		//HMat43 leftArmBottomMat(newRot, leftArmBottom);
	
		////Gravity::C_Body* pLeftArmBottom(AddBox(Maths::HVec3(-4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pLeftArmBottom(AddCapsule(leftArmBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), armMass, pFilter ));
		////pLeftArmBottom->MakeStatic();
		//Solver2::C_Link* pLeftArmBottomLink =m_pScene->m_pMultiBody->GetLink(4);
		//pLeftArmBottomLink->m_pBody = pLeftArmBottom;
		//pLeftArmBottomLink->m_Type = Solver2::LEFTARMBOTTOM;


		//HVec3 rightArmTop(HVec3(0.41f, 0.2f, 0.f) + position);
		//HMat43 rightArmTopMat(newRot, rightArmTop);
	

		//
		////Gravity::C_Body* pRightArmTop(AddBox(Maths::HVec3(2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightArmTop(AddCapsule(rightArmTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), armMass, pFilter ));
		////pRightArmTop->MakeStatic();
		//Solver2::C_Link* pRightArmTopLink = m_pScene->m_pMultiBody->GetLink(5);
		//pRightArmTopLink->m_pBody = pRightArmTop;
		//pRightArmTopLink->m_Type = Solver2::RIGHTARMTOP;

		//

		//HVec3 rightArmBottom(HVec3(0.82f, 0.15f, 0.f) + position);
		//HMat43 rightArmBottomMat(newRot, rightArmBottom);
	

		////Gravity::C_Body* pRightArmBottom(AddBox(Maths::HVec3(4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightArmBottom(AddCapsule(rightArmBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), armMass, pFilter ));
		////pRightArmBottom->MakeStatic();
		//Solver2::C_Link* pRightArmBottomLink = m_pScene->m_pMultiBody->GetLink(6);
		//pRightArmBottomLink->m_pBody = pRightArmBottom;
		//pRightArmBottomLink->m_Type = Solver2::RIGHTARMBOTTOM;
	

		//orientation = Maths::C_Quaternion(rot);
		////orientationNew = Maths::C_Quaternion(orientation);

		//HVec3 leftLegTop(HVec3(-0.1f, -0.75f, 0.f) + position);

		//HMat43 leftLegTopMat(rot, leftLegTop);
	
		////Gravity::C_Body* pLeftLegTop(AddBox(Maths::HVec3(-0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pLeftLegTop(AddCapsule(leftLegTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), legMass, pFilter ));
		////pLeftLegTop->MakeStatic();
		//Solver2::C_Link* pLeftLegTopLink = m_pScene->m_pMultiBody->GetLink(7);
		//pLeftLegTopLink->m_pBody = pLeftLegTop;

		//pLeftLegTopLink->m_Type = Solver2::LEFTLEGTOP;

		//

		//HVec3 leftLegBottom(HVec3(-0.1f, -1.15f, 0.f) + position);
		//HMat43 leftLegBottomMat(rot, leftLegBottom);
	

		////Gravity::C_Body* pLeftLegBottom(AddBox(Maths::HVec3(-0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pLeftLegBottom(AddCapsule(leftLegBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), legMass, pFilter ));
		////pLeftLegBottom->MakeStatic();
		//Solver2::C_Link* pLeftLegBottomLink = m_pScene->m_pMultiBody->GetLink(8);
		//pLeftLegBottomLink->m_pBody = pLeftLegBottom;
		//pLeftLegBottomLink->m_Type = Solver2::LEFTLEGBOTTOM;

	

		//HVec3 rightLegTop(HVec3(0.1f, -0.75f, 0.f) + position);
		//HMat43 rightLegTopMat(rot, rightLegTop);
		//

		////Gravity::C_Body* pRightLegTop(AddBox(Maths::HVec3(0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightLegTop(AddCapsule(rightLegTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), legMass, pFilter ));
		////pRightLegTop->MakeStatic();
		//Solver2::C_Link* pRightLegTopLink = m_pScene->m_pMultiBody->GetLink(9);
		//pRightLegTopLink->m_pBody = pRightLegTop;
		//pRightLegTopLink->m_Type = Solver2::RIGHTLEGTOP;
	

		//
		//HVec3 rightLegBottom(HVec3(0.1f, -1.15f, 0.f) + position);
		//HMat43 rightLegBottomMat(rot, rightLegBottom);
		//
		////Gravity::C_Body* pRightLegBottom(AddBox(Maths::HVec3(0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		//Gravity::C_Body* pRightLegBottom(AddCapsule(rightLegBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), legMass, pFilter ));
		//Solver2::C_Link* pRightLegBottomLink =m_pScene->m_pMultiBody->GetLink(10);
		//pRightLegBottomLink->m_pBody = pRightLegBottom;
		//
		//pRightLegBottomLink->m_Type = Solver2::RIGHTLEGBOTTOM;

	
		
		Solver2::C_Joint* joint0 = new Solver2::C_Joint;
		joint0->m_position = HVec3(head.X(), head.Y() - 0.10f, head.Z()) * headMat.InverseRotTrans();
		//joint->m_d = -joint->m_position;
		joint0->type = Solver2::REVOLUTEJOINT;
		joint0->m_jointAxis = HVec3(1.0f, 0.f, 0.f) * headMat.Rotation().Inverse();
		Normalise(joint0->m_jointAxis);
		joint0->IniSpatialJointAxis(-joint0->m_position);
	
		//joint->IniSpatialJointAxis(-joint->m_position);
		joint0 = m_pScene->m_pMultiBody->AddJoint(joint0);
		m_pScene->m_pMultiBody->Connect(pTorsoLink, pHeadLink, *joint0);

		//joint = m_pScene->m_pMultiBody->GetJoint(1);
		//joint->m_position = HVec3(leftArmTop.X(), leftArmTop.Y() + 0.2f, leftArmTop.Z())*leftArmTopMat.InverseRotTrans();
		////joint->m_d= - joint->m_position;
		//joint->m_jointAxis = HVec3(0.0f, 0.f, 1.f) * leftArmTopMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
	
		//
		///*joint->m_spatialJointAxis1 = joint->m_jointAxis;
		//joint->m_spatialJointAxis2 = joint->m_d ^ joint->m_jointAxis;*/
		//m_pScene->m_pMultiBody->Connect(pTorsoLink, pLeftArmTopLink, *joint);
		//
		//joint = m_pScene->m_pMultiBody->GetJoint(2);
		//joint->m_position = HVec3(rightArmTop.X(), rightArmTop.Y() - 0.2f, rightArmTop.Z())*rightArmTopMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(0.f, 0.f, 1.f)*rightArmTopMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//
		//m_pScene->m_pMultiBody->Connect(pTorsoLink, pRightArmTopLink, *joint);

		//joint = m_pScene->m_pMultiBody->GetJoint(3);
		//joint->m_position =HVec3(hip.X()- 0.1f, hip.Y(), hip.Z())*hipMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(1.f, 0.f, 0.f)*hipMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//
		//
		//m_pScene->m_pMultiBody->Connect(pTorsoLink, pHipLink, *joint);

	

		//joint = m_pScene->m_pMultiBody->GetJoint(4);
		//joint->m_position =HVec3(leftArmBottom.X(), leftArmBottom.Y() + 0.2f, leftArmBottom.Z())*leftArmBottomMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(0.f, 0.f, 1.f)*leftArmBottomMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//
		//m_pScene->m_pMultiBody->Connect(pLeftArmTopLink, pLeftArmBottomLink, *joint);

		//joint = m_pScene->m_pMultiBody->GetJoint(5);
		//joint->m_position = HVec3(rightArmBottom.X(), rightArmBottom.Y()- 0.2f, rightArmBottom.Z())*rightArmBottomMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(0.f, 0.f, 1.f)*rightArmBottomMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//m_pScene->m_pMultiBody->Connect(pRightArmTopLink, pRightArmBottomLink, *joint);


		//joint = m_pScene->m_pMultiBody->GetJoint(6);
		//joint->m_position = HVec3(leftLegTop.X(), leftLegTop.Y() + 0.2f, leftLegTop.Z())*leftLegTopMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(1.f, 0.f, 0.f)*leftLegTopMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//m_pScene->m_pMultiBody->Connect(pHipLink, pLeftLegTopLink, *joint);

		//joint =m_pScene->m_pMultiBody->GetJoint(7);
		//joint->m_position = HVec3(rightLegTop.X(),  rightLegTop.Y() + 0.2f, rightLegTop.Z())*rightLegTopMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(1.f, 0.f, 0.f)*rightLegTopMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);

		//m_pScene->m_pMultiBody->Connect(pHipLink, pRightLegTopLink, *joint);

		//joint = m_pScene->m_pMultiBody->GetJoint(8);
		//joint->m_position = HVec3(leftLegBottom.X(), leftLegBottom.Y() + 0.2f, leftLegBottom.Z())*leftLegBottomMat.InverseRotTrans();
		////joint->m_d = -joint->m_position;
		//joint->m_jointAxis = HVec3(1.f, 0.f, 0.f)*leftLegBottomMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//m_pScene->m_pMultiBody->Connect(pLeftLegTopLink, pLeftLegBottomLink, *joint);

		//joint =m_pScene->m_pMultiBody->GetJoint(9);
		//joint->m_position = HVec3(rightLegBottom.X(), rightLegBottom.Y() + 0.2f, rightLegBottom.Z())*rightLegBottomMat.InverseRotTrans();
		////joint->m_d = - joint->m_position;
		//joint->m_jointAxis = HVec3(1.f, 0.f, 0.f)*rightLegBottomMat.Rotation().Inverse();
		//joint->type = Solver2::REVOLUTEJOINT;
		//joint->IniSpatialJointAxis(-joint->m_position);
		//m_pScene->m_pMultiBody->Connect(pRightLegTopLink, pRightLegBottomLink, *joint);

		Solver2::C_Link* pStack[100];
		m_pScene->m_pMultiBody->EnumerateJointIndices(pStack);
		m_pScene->m_pMultiBody->RearrangeJoint();
		m_pScene->m_pMultiBody->InitiLInksSpatialIsolatedInertia();

		//make the body static to check the initial setup
		//pHead->MakeStatic();
		pTorso->MakeStatic();
		/*pHip->MakeStatic();
		pLeftArmTop->MakeStatic();
		pLeftArmBottom->MakeStatic();
		pRightArmTop->MakeStatic();
		pRightArmBottom->MakeStatic();
		pLeftLegTop->MakeStatic();
		pLeftLegBottom->MakeStatic();
		pRightLegTop->MakeStatic();
		pRightLegBottom->MakeStatic();*/

	}

	void C_TestPhysics::UpdateFeatherstoneRagdoll(const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		const HVec3 gravity(0.f, -9.8f, 0.f);
		U32 numJoints = m_pScene->m_pMultiBody->GetNumJoints();
		for(U32 i=0; i< numJoints; ++i)
		{
			Solver2::C_Joint* joint = m_pScene->m_pMultiBody->m_pSortedJoints[i];
			joint->m_scalarVelocity =MakeVF32(g_JointVelocity);
			joint-> m_spatialscalarForce =MakeVF32(g_JointForce);
			Solver2::C_Link* link=joint->m_pOutBoundLink;
			Solver2::C_Link* inBound=joint->m_pInBoundLink;
			SharedPhysics::C_RigidBody& inBody = inBound->m_pBody->GetBody();
			SharedPhysics::C_RigidBody& rigidB = link->m_pBody->GetBody();
			const HMat43 transformation(rigidB.m_OrientationMatrix, rigidB.m_Position);
			const HVec3 position = HVec3(joint->m_position) * transformation ;
			const Vector3 jointAxis = joint->m_jointAxis * rigidB.m_OrientationMatrix;
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(rigidB.m_Position, 0.5f, RGBA(0, 255, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(inBody.m_Position, 0.5f, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(position, 0.15f, RGBA(255, 0, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(position, HVec3(jointAxis), 2.f,  0.2f, RGBA(0, 0, 255, 255));
		}


		
		//m_MultiBody.FwdDynamic(time);


	}
};
