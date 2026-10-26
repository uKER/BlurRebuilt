#include "stdafx.h"
#include "ConstraintGraphTest.h"
#include "gravity/sdk/ReferenceFrame.h"


namespace Samples
{

	
	void C_TestPhysics::SetUpFrameOfReference(Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;

		const HVec3 positionA(1.f, 5.f, 3.f);
		HMat33 orientation;
		orientation.SetIdentity();
		const HVec3 extentA(2.f, 0.5f, 0.5f);
		const HF32 massA(5.f);

		Gravity::C_Body* pBodyA(AddBox(positionA, orientation, extentA, massA, pFilter));
		pBodyA->GetShape()->m_PhysicsMaterialIndex = PLATFORM;
		pBodyA->MakeKinematic();

		m_KinematicController = Gravity::C_KinematicController(pBodyA);
		m_pScene->AddKinematicController(&m_KinematicController);
		
		//pBodyA->MakeStatic();

		const HVec3 positionB(positionA + HVec3(0.5f, 0.75f, 0.f));
		const HVec3 extentB(0.25f, 0.25f, 0.25f);
		const HF32 massB(1.f);

		Gravity::C_Body* pBodyB(AddBox(positionB, orientation, extentB, massB, pFilter ));
		//pBodyB->MakeStatic();
		
		
		Gravity::C_ReferenceFrame* frame = new Gravity::C_ReferenceFrame;
		frame->AddReferenceFrame(pBodyA);
		frame->AddBody(pBodyB);

		m_pScene->AddReferenceFrame(frame);
	}

	void C_TestPhysics::UpdateFrameOfReference(const Maths::HF32 PCREF time, const Maths::HF32 PCREF rot)
	{
	
		using namespace Maths;

		Gravity::C_Body* pBody = m_KinematicController.GetBody();
		SharedPhysics::C_RigidBody& pRigidBody = pBody->GetBody();
		const HVec3 curPos = pRigidBody.m_Position;
		static const HVec3 originalPos = curPos;
		static bool swap = false;
		
		HVec3 dir = curPos - originalPos;
		HF32 dist = dir.Length();
		
		if(swap)
		{
			dist = -dist;
		}
	
		HVec3 targetPos = curPos;

		const HF32 zero(Constants<HF32>::Zero());
		if(dist.GE(zero))
		{
			if(dist.LT(100.f))
				targetPos += HVec3(-0.01f, 0.f, 0.f);
			else
				swap = true;
			
		}
		else
		{
			if(dist.GT(zero))
				swap=false;
			else
				targetPos +=HVec3(0.01f, 0.f, 0.f);
		}

	/*	static HF32 rot=0.f;
		if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
			rot=0.f;
		rot+=0.02f;*/

		const Maths::RotX rotxa(rot);
		HMat33 rotation(rotxa);
		const HMat43 trans(rotation, targetPos);
		m_KinematicController.SetTargetTransformation(trans, time);
		/*Gravity::C_ReferenceFrame* frame =m_pScene->GetReferenceFrame(0);
		frame->UpdateAttachedBodyState();*/
	}
}
