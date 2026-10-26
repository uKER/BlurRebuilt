#include "stdafx.h"
#include "gravity\sdk\RagdollController.h"
#include "gravity\sdk\ConstraintDescriptors.h"
#include <Core/Maths/HVec/New/IncludePhysics.h>
#include <Core/Maths/HVec/New/IncludePhysics.inl>
#include "TestPhysics.h"

namespace Samples
{
	
	Debug::Bool g_UseStoredPose( "RagdollTest.UseStoredPose", false );
	Debug::Bool g_SetStoredPose( "RagdollTest.SetStoredPose", false );
	Debug::Float g_BodyVelocityWeight( "RagdollTest.BodyVelocityWeights", 0.0f,  Debug::Range(0.0f, 1.0f) );
	Debug::Float g_BodyPositionWeight( "RagdollTest.BodyPositionWeights", 0.0f,  Debug::Range(0.0f, 1.0f) );
	Debug::Float g_ConstraintWeights( "RagdollTest.ConstraintWeights", 0.0f,  Debug::Range(0.0f, 1.0f) );
	Debug::Bool g_AnimatedRagdoll("RagdollTest.AnimatedRagdoll", false);

	void C_TestPhysics::InitialiseRagdollConfig(Collision::C_CollisionFilter* pFilter, const Maths::HVec3 PCREF position)
	{
		using namespace Maths;

		Gravity::C_RagdollConfig ragdollConfig;
		//Gravity::C_RagdollPose ragdollPose;
		Gravity::C_RagdollQuaternionPose ragdollQuaternionPose;
		Gravity::C_RagdollRelativePose ragdollRelativePose;
		HF32 eps =Constants<HF32>::Epsilon();

		const Maths::HF32 mass(1.f);

		HMat33 rot;
		rot.SetIdentity();
		C_Quaternion orientation(rot);
		Quaternion orientationNew(MakeUnitQuaternion((Mat33)rot));
		//const Maths::HVec3 extents(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());
		//const HF32 mass(Maths::Constants<HF32>::Three());

		HVec3 head(HVec3(0.f, 0.44f, 0.f) + position);
		//C_Quaternion headRot(orientation);
		Quaternion headRot(orientationNew);
		HMat43 headMat(rot, head);
		

		//Gravity::C_Body* pHead(AddBox(Maths::HVec3(0.f, 1.5f, 0.f) + position, orientation, Maths::HVec3(0.5f, 0.5f, 0.5f), Maths::HF32(2.f), pFilter));
		Gravity::C_Body* pHead(AddSphere(head, Maths::HF32(0.1f), mass, pFilter));
		//pHead->MakeStatic();
		ragdollConfig.SetBody(pHead);
		//ragdollPose.SetBodyMatrix(HMat43(rot, head));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(head);
		
		
		Maths::HVec3 torso(position);
		//C_Quaternion torsoRot(orientation);
		Quaternion torsoRot(orientationNew);
		HMat43 torsoMat(rot, position);
		
	
		
		//Gravity::C_Body* pTorso(AddBox(Maths::HVec3(0.f, 0.f, 0.f) + position, orientation, Maths::HVec3(1.f, 1.f, 1.f), Maths::HF32(5.f), pFilter ));
		Gravity::C_Body* pTorso(AddCapsule(torso, orientation, Maths::HF32(0.2f), Maths::HF32(0.2f), mass, pFilter ));
		//pTorso->MakeStatic();

		ragdollConfig.SetBody(pTorso);
//		ragdollPose.SetBodyMatrix(HMat43(rot, torso));

		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(torso);

		Maths::RotZ rot_90(Maths::Constants<Maths::HF32>::Pi()*Maths::Constants<Maths::HF32>::Half());
		const HMat33 newRot=HMat33(rot_90);
		orientation = Maths::C_Quaternion(newRot);
		orientationNew = Maths::C_Quaternion(MakeUnitQuaternion((Mat33)newRot));

		HVec3 hip(HVec3(0.f, -0.45f, 0.f) + position);
		//C_Quaternion hipRot(orientation);
		Quaternion hipRot(orientationNew);
		HMat43 hipMat(newRot, hip);
	
		
		//Gravity::C_Body* pHip(AddBox(Maths::HVec3(0.f, -2.f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f,1.f), Maths::HF32(1.f), pFilter));
		Gravity::C_Body* pHip(AddCapsule(hip, orientation, Maths::HF32(0.1f), Maths::HF32(0.08f), mass, pFilter));
		//pHip->MakeStatic();

		ragdollConfig.SetBody(pHip);
//		ragdollPose.SetBodyMatrix(HMat43(newRot, hip));

		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(hip);
		
		//Gravity::C_Body* pLeftArmTop(AddBox(Maths::HVec3(-2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		
		HVec3 leftArmTop(HVec3(-0.41f, 0.2f, 0.f) + position);
		//C_Quaternion leftArmTopRot(orientation);
		Quaternion leftArmTopRot(orientationNew);
		HMat43 leftArmTopMat(newRot, leftArmTop);
	
		
		
		Gravity::C_Body* pLeftArmTop(AddCapsule(leftArmTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pLeftArmTop->MakeStatic();

		ragdollConfig.SetBody(pLeftArmTop);
//		ragdollPose.SetBodyMatrix(HMat43(newRot, leftArmTop));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(leftArmTop);
		

		HVec3 leftArmBottom(HVec3(-0.82f, 0.15f, 0.f) + position);
		//C_Quaternion leftArmBottomRot(orientation);
		Quaternion leftArmBottomRot(orientationNew);
		HMat43 leftArmBottomMat(newRot, leftArmBottom);
	
		
		//Gravity::C_Body* pLeftArmBottom(AddBox(Maths::HVec3(-4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftArmBottom(AddCapsule(leftArmBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pLeftArmBottom->MakeStatic();

		ragdollConfig.SetBody(pLeftArmBottom);
//		ragdollPose.SetBodyMatrix(HMat43(newRot, leftArmBottom));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(leftArmBottom);


		HVec3 rightArmTop(HVec3(0.41f, 0.2f, 0.f) + position);
		//C_Quaternion rightArmTopRot(orientation);
		Quaternion rightArmTopRot(orientationNew);
		HMat43 rightArmTopMat(newRot, rightArmTop);
	

		
		//Gravity::C_Body* pRightArmTop(AddBox(Maths::HVec3(2.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightArmTop(AddCapsule(rightArmTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pRightArmTop->MakeStatic();

		ragdollConfig.SetBody(pRightArmTop);
//		ragdollPose.SetBodyMatrix(HMat43(newRot, rightArmTop));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(rightArmTop);

		HVec3 rightArmBottom(HVec3(0.82f, 0.15f, 0.f) + position);
		//C_Quaternion rightArmBottomRot(orientation);
		Quaternion rightArmBottomRot(orientationNew);
		HMat43 rightArmBottomMat(newRot, rightArmBottom);
	

		//Gravity::C_Body* pRightArmBottom(AddBox(Maths::HVec3(4.f, 0.5f, 0.f) + position, orientation, Maths::HVec3(1.f, 0.5f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightArmBottom(AddCapsule(rightArmBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pRightArmBottom->MakeStatic();

		ragdollConfig.SetBody(pRightArmBottom);
//		ragdollPose.SetBodyMatrix(HMat43(newRot, rightArmBottom));
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyPosition(rightArmBottom);

		orientation = Maths::C_Quaternion(rot);
		orientationNew = Maths::C_Quaternion(orientation);

		HVec3 leftLegTop(HVec3(-0.1f, -0.75f, 0.f) + position);
		//C_Quaternion leftLegTopRot(orientation);
		Quaternion leftLegTopRot(orientationNew);
		HMat43 leftLegTopMat(rot, leftLegTop);
	
		//Gravity::C_Body* pLeftLegTop(AddBox(Maths::HVec3(-0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftLegTop(AddCapsule(leftLegTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pLeftLegTop->MakeStatic();

		ragdollConfig.SetBody(pLeftLegTop);
//		ragdollPose.SetBodyMatrix(HMat43(rot, leftLegTop));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(leftLegTop);

		HVec3 leftLegBottom(HVec3(-0.1f, -1.15f, 0.f) + position);
		//C_Quaternion leftLegBottomRot(orientation);
		Quaternion leftLegBottomRot(orientationNew);
		HMat43 leftLegBottomMat(rot, leftLegBottom);
	

		//Gravity::C_Body* pLeftLegBottom(AddBox(Maths::HVec3(-0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pLeftLegBottom(AddCapsule(leftLegBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pLeftLegBottom->MakeStatic();

		ragdollConfig.SetBody(pLeftLegBottom);
//		ragdollPose.SetBodyMatrix(HMat43(rot, leftLegBottom));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(leftLegBottom);

		HVec3 rightLegTop(HVec3(0.1f, -0.75f, 0.f) + position);
		//C_Quaternion rightLegTopRot(orientation);
		Quaternion rightLegTopRot(orientationNew);
		HMat43 rightLegTopMat(rot, rightLegTop);
		

		//Gravity::C_Body* pRightLegTop(AddBox(Maths::HVec3(0.5f, -4.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightLegTop(AddCapsule(rightLegTop, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pRightLegTop->MakeStatic();

		ragdollConfig.SetBody(pRightLegTop);
//		ragdollPose.SetBodyMatrix(HMat43(rot, rightLegTop));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(rightLegTop);

		HVec3 rightLegBottom(HVec3(0.1f, -1.15f, 0.f) + position);
		//C_Quaternion rightLegBottomRot(orientation);
		Quaternion rightLegBottomRot(orientationNew);
		HMat43 rightLegBottomMat(rot, rightLegBottom);
		
		//Gravity::C_Body* pRightLegBottom(AddBox(Maths::HVec3(0.5f, -6.f, 0.f) + position, orientation, Maths::HVec3(0.5f, 1.f, 0.5f), Maths::HF32(1.f), pFilter ));
		Gravity::C_Body* pRightLegBottom(AddCapsule(rightLegBottom, orientation, Maths::HF32(0.2f), Maths::HF32(0.1f), mass, pFilter ));
		//pRightLegBottom->MakeStatic();

		ragdollConfig.SetBody(pRightLegBottom);
//		ragdollPose.SetBodyMatrix(HMat43(rot, rightLegBottom));
		//ragdollQuaternionPose.SetBodyQuaternion(orientation);
		ragdollQuaternionPose.SetBodyQuaternion(C_Quaternion(orientationNew));
		ragdollQuaternionPose.SetBodyPosition(rightLegBottom);

	
		Gravity::SphericalJointDescriptor desc;
		Gravity::RevoluteJointLimitDescriptor revDesc;
		//Gravity::RevoluteJointDescriptor revDesc;
		Gravity::SphericalJointDescriptor fixDesc;
		//fixDesc.SetBaumgarte(0.0000001f);

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());

		fixDesc.DefineAnchorPoints(zero, zero);
//		fixDesc.DefineJointAxis(zero, zero);

		desc.DefineAnchorPoints(Maths::HVec3(0.f, 0.32f, 0.f), Maths::HVec3(0.f, -0.12f, 0.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(0.f, 0.32f, 0.f), Maths::HVec3(0.f, -0.12f, 0.f));

		//revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		SharedPhysics::C_Constraint* pConstraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHead);
		SharedPhysics::C_Constraint* pFixedConstraint = fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHead);

		//HVec3 relativePosition = (torso - head)*(!torsoRot);
		////C_Quaternion relativeQua = headRot * (!torsoRot) ;
		//Quaternion relativeQua = headRot * (!torsoRot) ;

		HMat43 relativeMat = headMat * torsoMat.InverseRotTrans();
		QuatTrans relativeQt = MakeQuatTrans(relativeMat);

		/*ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		Gravity::C_RagdollConstraint con;
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		//con.m_pConstraint = pConstraint;
		//con.m_IndexA = ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody);
		//con.m_IndexB = ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody);
		ragdollConfig.SetConstraint(con);

		desc.DefineAnchorPoints(Maths::HVec3(-0.21f, 0.15f, 0.f), pLeftArmTop->Orientation() * Maths::HVec3(0.21f, 0.f, 0.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(-0.21f, 0.15f, 0.f), pLeftArmTop->Orientation() * Maths::HVec3(0.21f, 0.f, 0.f));
		pConstraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
		pFixedConstraint = fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pLeftArmTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		/*con.m_pConstraint = pConstraint;
		con.m_IndexA = ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody);
		con.m_IndexB = ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody);*/
		ragdollConfig.SetConstraint(con);

		/*relativePosition = (torso - leftArmTop)*(!torsoRot);
		relativeQua = leftArmTopRot * (!torsoRot);*/
		relativeMat = leftArmTopMat * torsoMat.InverseRotTrans();
		relativeQt = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);
		

		//revDesc.DefineAnchorPoints(pLeftArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f), pLeftArmBottom->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f));
		revDesc.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(180.f));
		fixDesc.DefineAnchorPoints(pLeftArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f), pLeftArmBottom->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		pConstraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);
		pFixedConstraint = fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftArmTop, pLeftArmBottom);

		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);

		/*relativePosition = (leftArmTop - leftArmBottom)*(!leftArmTopRot);
		relativeQua = leftArmBottomRot * (!leftArmTopRot) ;*/
		relativeMat = leftArmBottomMat * leftArmTopMat.InverseRotTrans();
		relativeQt = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);


		desc.DefineAnchorPoints(Maths::HVec3(0.21f, 0.15f, 0.f), pRightArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(0.21f, 0.15f, 0.f), pRightArmTop->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		pConstraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
		pFixedConstraint = fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pRightArmTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (torso - rightArmTop)*(!torsoRot);
		relativeQua =  rightArmTopRot* (!torsoRot);*/
		relativeMat = rightArmTopMat * torsoMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);


		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		revDesc.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(180.f));
//		fixDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
//		revDesc.DefineAnchorPoints(pRightArmTop->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f), pRightArmBottom->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		fixDesc.DefineAnchorPoints(pRightArmTop->Orientation()*Maths::HVec3(0.21f, 0.f, 0.f), pRightArmBottom->Orientation()*Maths::HVec3(-0.21f, 0.f, 0.f));
		pConstraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
		pFixedConstraint = fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightArmTop, pRightArmBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (rightArmTop - rightArmBottom)*(!rightArmTopRot);
		relativeQua =  rightArmBottomRot * (!rightArmTopRot);*/
		relativeMat = rightArmBottomMat * rightArmTopMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

//		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.31f, 0.f), pHip->Orientation()*Maths::HVec3(0.f, 0.085f, 0.f));
		revDesc.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(180.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.31f, 0.f), pHip->Orientation()*Maths::HVec3(0.f, 0.085f, 0.f));
		revDesc.DefineJointAxis(Maths::HVec3(0.f, 0.f, 1.f), Maths::HVec3(0.f, 0.f, 1.f));
		pConstraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
		pFixedConstraint=fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pTorso, pHip);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);;
		/*relativePosition = (torso - hip)*(!torsoRot);
		relativeQua =  hipRot*(!torsoRot);*/
		relativeMat = hipMat * torsoMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(-0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		fixDesc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(-0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		pConstraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
		pFixedConstraint=fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pLeftLegTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (hip- leftLegTop)*(!hipRot);
		relativeQua = leftLegTopRot * (!hipRot);*/
		relativeMat = leftLegTopMat * hipMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);


		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(180.f));
		//fixDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
//		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		pConstraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
		pFixedConstraint=fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pLeftLegTop, pLeftLegBottom);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (leftLegTop - leftLegBottom)*(!leftLegTopRot);
		relativeQua = leftLegBottomRot * (!leftLegTopRot);*/
		relativeMat = leftLegBottomMat * leftLegTopMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

		desc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		fixDesc.DefineAnchorPoints(pHip->Orientation()*Maths::HVec3(0.1f, -0.085f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		pConstraint=desc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
		pFixedConstraint=fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pHip, pRightLegTop);
		//m_Constraints.AddToEnd(&constraint);
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (hip - rightLegTop)*(!hipRot);
		relativeQua = rightLegTopRot * (!hipRot);*/
		relativeMat = rightLegTopMat * hipMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

		revDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
		revDesc.DefineJointLimit(Maths::HF32(0.f), Maths::HF32(180.f));
		//fixDesc.DefineJointAxis(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f));
//		revDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		fixDesc.DefineAnchorPoints(Maths::HVec3(0.f, -0.21f, 0.f), Maths::HVec3(0.f, 0.21f, 0.f));
		pConstraint=revDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
		pFixedConstraint=fixDesc.CreateConstraint(*m_pScene->m_pGraphManager, pRightLegTop, pRightLegBottom);
		
		m_pScene->AddConstraint(pConstraint);
		m_pScene->AddConstraint(pFixedConstraint);
		con.Initialise(pConstraint, ragdollConfig.GetIndex(pConstraint->pNodeA->m_pBody),
			ragdollConfig.GetIndex(pConstraint->pNodeB->m_pBody), pFixedConstraint);
		ragdollConfig.SetConstraint(con);
		/*relativePosition = (rightLegTop - rightLegBottom)*(!rightLegTopRot);
		relativeQua = rightLegBottomRot * (!rightLegTopRot);*/
		relativeMat = rightLegBottomMat * rightLegTopMat.InverseRotTrans();
		relativeQt  = MakeQuatTrans(relativeMat);
		/*ragdollRelativePose.SetRelativePosition(relativePosition);
		ragdollRelativePose.SetRelativeQuaternion(relativeQua);
		ragdollRelativePose.SetRelativeMatrix(relativeMat);*/
		ragdollRelativePose.SetQuatTrans(relativeQt);

		m_RagdollController.Initialise(ragdollConfig);
//		m_RagdollController.SetPose(ragdollPose);
		m_RagdollController.SetQuaternionPose(ragdollQuaternionPose);
		//m_RagdollController.SetRelativePose(ragdollRelativePose);

		Gravity::C_RagdollWeights weights;
		const S32 bodyCount = m_RagdollController.GetConfig().GetBodyCount();
		

		for(S32 i=0; i<bodyCount; i++)
		{
			Gravity::C_Weights weight(g_BodyPositionWeight);
			weights.SetBodyWeight( weight );
		}
		
		const S32 constraintCount = m_RagdollController.GetConfig().GetConstraintCount();
		
		for(S32 i=0; i<constraintCount; i++)
		{
			weights.SetConstraintWeight( g_ConstraintWeights );
			//weights.SetConstraintWeight( 1.f );
		}
		
		m_RagdollController.SetTargetWeights( weights );
		m_pScene->AddRagdollController(&m_RagdollController);
		
	}

	void C_TestPhysics::UpdateRagdollBodyWeight()
	{
		Gravity::C_RagdollWeights& weights = m_RagdollController.GetTargetWeights();
	
		const S32 bodyCount = weights.GetBodyCount();
		
		for(S32 i=0; i<bodyCount; i++)
		{
			Gravity::C_Weights w(g_BodyPositionWeight);
			weights.SetBodyWeight(i, w );
		}

	}

	void C_TestPhysics::UpdateRagdollConstraintWeight()
	{
		Gravity::C_RagdollWeights& weights = m_RagdollController.GetTargetWeights();

		const S32 constraintCount = weights.GetConstraintCount();
		
		for(S32 i=0; i<constraintCount; i++)
		{
			weights.SetConstraintWeight( i, g_ConstraintWeights );
		}

	}

	void C_TestPhysics::UpdateRagdollController(const Maths::HF32 PCREF time)
	{

		bool bAnimated = g_AnimatedRagdoll;

		if(bAnimated)
		{
			m_RagdollController.MakeAnimated();
		}
		else
		{
			m_RagdollController.MakeActive();
		}

	/*	Gravity::C_RagdollPose m_StoredPose = m_RagdollController.GetInitialPose();

		if(g_SetStoredPose)
		{
			m_StoredPose = m_RagdollController.GetPose();
		}
		
		if(g_UseStoredPose)
        {
            m_RagdollController.SetTargetPose(m_StoredPose, time);
		}
        else
		{
			m_RagdollController.SetTargetPose(m_RagdollController.GetInitialPose(), time);
		}*/

		/*Gravity::C_RagdollQuaternionPose& m_StoredPose = m_RagdollController.GetInitialQuaternionPose();

		if(g_SetStoredPose)
		{
			m_StoredPose = m_RagdollController.GetQuaternionPose();
		}

		if(g_UseStoredPose)
        {
            m_RagdollController.SetTargetQuaternionPose(m_StoredPose, time);
		}
        else
		{
			m_RagdollController.SetTargetQuaternionPose(m_RagdollController.GetInitialQuaternionPose(), time);
		}

		Gravity::C_RagdollRelativePose& m_StoredRelativePose = m_RagdollController.GetInitialRelativePose();

		if(g_SetStoredPose)
		{
			m_StoredRelativePose = m_RagdollController.GetRelativePose();
		}

		if(g_UseStoredPose)
		{
			m_RagdollController.SetTargetRelativePose(m_StoredRelativePose, time);
		}
		else
		{
			m_RagdollController.SetTargetRelativePose(m_RagdollController.GetInitialRelativePose(), time);
		}*/
		
	
	}

}
