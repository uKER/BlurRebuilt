#include "stdafx.h"
#include "gravity/sdk/RagdollController.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/sdk/Scene.h"
#include <system/perf.h>

namespace Gravity
{


	DECLARE_DEBUG_BOOL(g_dbDrawMuscleConstraint, "Gravity.Debug.DrawRagdollMuscles", false);
	U32 C_RagdollController::m_sIndex = 0;

	const Maths::C_Quaternion C_RagdollRelativePose::GetRelativeQuaternion(const S32 index) const
	{
		return Maths::C_Quaternion(Maths::Quat(m_RelativeQuatTrans[index]));
	}

	const Maths::HVec3 C_RagdollRelativePose::GetRelativePosition(const S32 index) const
	{
		return Maths::HVec3(Maths::Trans(m_RelativeQuatTrans[index]));
	}

	//Directly sets the position and orientation of the ragdoll bodies
	void C_RagdollController::SetQuaternionPose(const C_RagdollQuaternionPose& pose)
	{
		using namespace Maths;
		const S32 numBodies = m_RagdollConfig.GetBodyCount();
		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			SharedPhysics::C_RigidBody& pRigidBody = pBody->GetBody();
			const HVec3& position = pose.GetBodyPosition(i);
			const C_Quaternion& q= pose.GetBodyQuaternion(i);

			//Update all the state in the rigid body
			pRigidBody.StoreVelocitiesAndPosition(pRigidBody.m_LinearVelocity, pRigidBody.m_AngularVelocity, HMat33(q), position);										
		}

		//m_InitialQuaternionPose = pose;
		m_TargetQuaternionPose =pose;

	}

	//// Returns the current position and orientation of the ragdoll bodies
	C_RagdollQuaternionPose C_RagdollController::GetQuaternionPose()const
	{
		using namespace Maths;

		C_RagdollQuaternionPose pose;
		const S32 numBodies = m_RagdollConfig.GetBodyCount();
		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			SharedPhysics::C_RigidBody& pRigidBody = pBody->GetBody();
	
			pose.SetBodyQuaternion(pRigidBody.m_Orientation);
			pose.SetBodyPosition(pRigidBody.m_Position);
		}

		return pose;
	}

	//void C_RagdollController::SetRelativePose(const C_RagdollRelativePose& pose)
	//{

	//	//m_InitialRelativePose = pose;
	//	m_TargetRelativePose =pose;
	//}

	void C_RagdollController::MakeRaytraceOnly(bool bRaytraceOnly)
	{
		if(bRaytraceOnly != m_RaytraceOnly)
		{
			if(bRaytraceOnly)
			{
				const S32 numBodies = m_RagdollConfig.GetBodyCount();
				for(S32 i=0; i<numBodies; ++i)
				{
					C_Body* pBody = m_RagdollConfig.GetBody(i);
					pBody->MakeRaytraceOnly();
				}
			}
			else
			{
				const S32 numBodies = m_RagdollConfig.GetBodyCount();
				for(S32 i=0; i<numBodies; ++i)
				{
					C_Body* pBody = m_RagdollConfig.GetBody(i);
					pBody->DisableRaytraceOnly();
				}
			}
			m_RaytraceOnly = bRaytraceOnly;
		}
	}


	const C_RagdollRelativePose& C_RagdollController::GetRelativePose()const
	{
		return m_TargetRelativePose;
	}

	void C_RagdollController::SetTargetQuaternionPose(const C_RagdollQuaternionPose& pose, const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 two(Constants<HF32>::Two());
		const HF32 half(Constants<HF32>::Half());
		const Maths::HF32 recipTime(time.Reciprocal());

		const S32 numBodies = m_RagdollConfig.GetBodyCount();

		const HF32 scale(Maths::Min(time * m_RecipExpectedTimeStep, one));

		for(S32 i=0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			SharedPhysics::C_RigidBody& pRigidBody = pBody->GetBody();
			
			const HVec3 p=pose.GetBodyPosition(i);
			
			const C_Weights w = m_Weights.GetBodyWeight(i);
			const HF32 animationWeight(Maths::Load<Maths::HF32>(&w.m_AnimationWeight));
			const HF32 weight(Maths::Select(animationWeight.EQ(one), animationWeight, animationWeight * scale));
			const HF32 rest = one - weight;

			const HVec3 positionDt =(p - pRigidBody.m_Position);
			HVec3 linVel = positionDt * recipTime;

			const C_Quaternion targetRot(pose.GetBodyQuaternion(i));
			const Maths::C_Quaternion newRot(pRigidBody.m_Orientation.ShortestSlerp(targetRot, weight));
			const HVec3 angVel(Maths::Imag(newRot * !pRigidBody.m_Orientation) * recipTime * two);
			
			pRigidBody.m_LinearVelocity = weight * linVel + rest * pRigidBody.m_LinearVelocity;
			pRigidBody.m_AngularVelocity = weight* angVel + rest*pRigidBody.m_AngularVelocity;

			pRigidBody.m_LinearMomentum = pRigidBody.m_LinearVelocity * pRigidBody.Mass();
			pRigidBody.m_AngularMomentum = pRigidBody.m_AngularVelocity * pRigidBody.m_InertiaTensor;
		}

		m_TargetQuaternionPose = pose;
		//Activate();
	}

	void C_RagdollConstraint::Initialise(SharedPhysics::C_Constraint* pConstraint, U32 indexA, U32 indexB,
		SharedPhysics::C_Constraint* pFixedConstraint)
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		m_pConstraint = pConstraint;
		m_IndexA = indexA;
		m_IndexB = indexB;
		if(pFixedConstraint)
		{
			m_pFixedConstraint = (Solver2::C_FixedRotationalConstraint*)pFixedConstraint;
			m_pFixedConstraint->SetRotationalScale(0.f);
		}
	}

	void C_RagdollController::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		const S32 numConstraints = m_RagdollConfig.GetConstraintCount();
		bool bDrawMuscles = g_dbDrawMuscleConstraint;
		for(S32 i=0; i< numConstraints ; ++i)
		{
			C_RagdollConstraint& con = m_RagdollConfig.GetConstraint(i);
			if(!bDrawMuscles)
			{
				con.m_pConstraint->DebugRender(pRender);
			}
			else if( con.m_pFixedConstraint)
			{
				Solver2::C_FixedRotationalConstraint* pFixed = con.m_pFixedConstraint;
				pFixed->DebugRender(pRender);
			}
 		}
	}


	//Assume body A is fix and the relative orientation is body B relative to body A
	void C_RagdollController::SetTargetRelativePose(const C_RagdollRelativePose& pose, const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		const S32 numConstraints = m_RagdollConfig.GetConstraintCount();
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());

		const HF32 scale(Maths::Min(time * m_RecipExpectedTimeStep, one));
		F32 fScale;
		Maths::Store(scale, &fScale);

		for(S32 i=0; i< numConstraints ; ++i)
		{
			C_RagdollConstraint& con = m_RagdollConfig.GetConstraint(i);
			if(con.m_pFixedConstraint)
			{
				F32 weight = m_Weights.GetConstraintWeight(i);
				weight = weight * fScale;//weight*weight;
				//weight = 0.f;
				//assert(weight >= 0.f && weight <= 1.f);
				Solver2::C_FixedRotationalConstraint* pFixed = con.m_pFixedConstraint;
				const Maths::HVec3 position = pose.GetRelativePosition(i);
				//The transform from 'A' space to 'B' space!
				const Maths::QuatTrans relTrans = pose.GetRelativeQuatTrans(i);
				const Maths::Quaternion qRot = relTrans.m_Q;

				pFixed->m_rotationalOffSet = qRot;
				pFixed->SetRotationalScale(weight);
				//con.m_pConstraint->SetRotationalScale(1.f);//Maths::Max(1.f - weight, 0.f));
				con.m_pConstraint->SetRotationalScale(Maths::Max(1.f - weight, 0.f));
			}
 		}
		m_TargetRelativePose = pose;
	}



	const C_RagdollQuaternionPose& C_RagdollController::GetTargetQuaternionPose() const
	{
		return m_TargetQuaternionPose;
	}

	void C_RagdollController::SetTargetWeights(const Gravity::C_RagdollWeights& targetWeights)
	{
		m_Weights = targetWeights;
	}

	C_RagdollWeights& C_RagdollController::GetTargetWeights()
	{
		return m_Weights;
	}

	void C_RagdollController::MakeAnimated()
	{
		const S32 numBodies = m_RagdollConfig.GetBodyCount();
		for(S32 a = 0; a < numBodies; ++a)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(a);
			if(!pBody->IsExternalKinematic())
			{
				pBody->MakeKinematic();
			}
		}
	}

	void C_RagdollController::MakeAnimated(const U32 bodyIndex)
	{
		C_Body* pBody = m_RagdollConfig.GetBody(bodyIndex);
		pBody->MakeKinematic();
	}


	void C_RagdollController::MakeActive()
	{
		const S32 numBodies = m_RagdollConfig.GetBodyCount();
		for(S32 a = 0; a < numBodies; ++a)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(a);
			if(pBody->IsExternalKinematic())
			{
				pBody->Activate();
			}
		}
	}
	void C_RagdollController::MakeActive(const U32 bodyIndex)
	{
		C_Body* pBody = m_RagdollConfig.GetBody(bodyIndex);
		pBody->Activate();
	}

	void C_RagdollController::SetTargetFrameOfReferencePose( const C_RagdollRelativePose& targetPose, const Maths::HF32 PCREF time )
	{
		m_TargetFrameOfReferencePose = targetPose;
		m_RemainingReferenceTime = time;
		m_ReciprocalFrameOfReferenceTime = time.Reciprocal();
		RecordCurrentFrameOfReferencePose();
		//Activate();
	}

	void C_RagdollController::RecordCurrentFrameOfReferencePose()
	{
		using namespace Maths;
		//Record this current pose of the body parts relative to the frame of reference object...
		if(m_pFrameOfReference)
		{
			m_InitialFrameOfReferencePose.m_RelativeQuatTrans.m_numElements = 0;
			SharedPhysics::C_RigidBody& referenceBody = m_pFrameOfReference->GetBody();
			const Maths::QuatTrans referenceTrans = Maths::MakeQuatTrans(referenceBody.m_Orientation, referenceBody.m_Position);
			const Maths::QuatTrans inverseTrans(Maths::Inverse(referenceTrans));
			const S32 numBodies = m_RagdollConfig.GetBodyCount();
			for(S32 a = 0; a < numBodies; ++a)
			{
				C_Body* pBody = m_RagdollConfig.GetBody(a);
				SharedPhysics::C_RigidBody& rigid = pBody->GetBody();
				const Maths::QuatTrans qTrans = Maths::MakeQuatTrans(rigid.m_Orientation, rigid.m_Position);
				//Now work this out in local space to the reference body...
				Maths::QuatTrans relativeTrans(qTrans * inverseTrans);
				m_InitialFrameOfReferencePose.SetQuatTrans(relativeTrans);
			}
		}
	}


	void C_RagdollController::FollowFrameOfReference(const Maths::HF32 PCREF time, C_Body** pBodies)
	{
		using namespace Maths;
		//Updated target position... update current target position. 
		if(m_pFrameOfReference)
		{
			const HF32 recipTime(time.Reciprocal());
			const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const HF32 half(Maths::Constants<Maths::HF32>::Half());
			const HF32 two(Maths::Constants<Maths::HF32>::Two());
			const HF32 one(Maths::Constants<Maths::HF32>::One());
			const HF32 timeScale(Maths::Min(time*m_RecipExpectedTimeStep, one));
			Spu::MemBaseMarker marker;
			C_Body* pFrameOfReference = (C_Body*) Spu::FetchAlloc(m_pFrameOfReference, sizeof(C_Body));
			SharedPhysics::C_RigidBody& referenceBody = pFrameOfReference->GetBody();
			//SharedPhysics::C_RigidBody& referenceBody = m_pFrameOfReference->GetBody();
			//Where the reference body actually is in world space...
			const QuatTrans referenceTrans = MakeQuatTrans(referenceBody.m_Orientation, referenceBody.m_Position);

			m_RemainingReferenceTime = Maths::Max(zero, m_RemainingReferenceTime - time);
			const HF32 lerpScale(one - (m_RemainingReferenceTime*m_ReciprocalFrameOfReferenceTime));

			const S32 numBodies = m_RagdollConfig.GetBodyCount();


			QuatTrans* initialPos = (QuatTrans*)Spu::FetchAlloc(&m_InitialFrameOfReferencePose.m_RelativeQuatTrans[0], sizeof(QuatTrans)*numBodies);
			QuatTrans* targetPos = (QuatTrans*)Spu::FetchAlloc(&m_TargetFrameOfReferencePose.m_RelativeQuatTrans[0], sizeof(QuatTrans)*numBodies);
			C_Weights* animationWeights = (C_Weights*)Spu::FetchAllocUnaligned(&m_Weights.m_BodyWeights[0], sizeof(C_Weights)*numBodies);

			//for(S32 a = 0; a < numBodies; ++a)
			//{
			//	C_Body* pBody = pBodies[a];
			//	
			//	if(pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			//	{
			//		SharedPhysics::C_RigidBody& rigid = pBody->GetBody();
			//		//What the initial relative pose was...
			//		
			//		QuatTrans& initialRelativePose = initialPos[a];
			//		QuatTrans& targetRelativePose = targetPos[a];
			//		//In local space
			//		const QuatTrans frameOfReferencePose(NearestSlerp(initialRelativePose, targetRelativePose, scale));
			//	
			//		//Transform into world space, then work out velocities accordingly...

			//		const QuatTrans finalQTrans(frameOfReferencePose * referenceTrans);

			//		const HVec3 p(Maths::Trans(finalQTrans));

			//		const HF32 weight(Maths::Load<Maths::HF32>(&animationWeight[a].m_AnimationWeight));
			//		const HF32 rest = one - weight;

			//		const HVec3 positionDt =(p - rigid.m_Position);
			//	
			//		const C_Quaternion targetRot(Maths::Quat(finalQTrans));

			//		//Project body into future position using current velocity...
			//		const C_Quaternion orientationDt(Maths::C_Quaternion((rigid.m_AngularVelocity * half) * time) * rigid.m_Orientation );
			//		const C_Quaternion futureRot((rigid.m_Orientation + orientationDt));
			//		const C_Quaternion newRot(futureRot.Normalised().ShortestSlerp(targetRot, weight));
			//		//const HF32 cosAngle(newRot | rigid.m_Orientation);
			//		//const C_Quaternion oldRot(Maths::Select(cosAngle.GE(zero), rigid.m_Orientation, -rigid.m_Orientation));
			//		////HVec3 angVel(Maths::Imag(newRot * !oldRot) * recipTime * two);

			//		//const Maths::C_Quaternion angVelQuat(Maths::NearestDifference(oldRot, newRot));

			//		///*const Maths::HVec3 normalisedImag(Maths::RotationAxis(angVelQuat));
			//		//const Maths::HF32 angle(-Maths::RotationAngle(angVelQuat));
			//		//const Maths::HVec3 angVel(normalisedImag * angle * recipTime);*/
			//		//const Maths::HVec3 angVel(Maths::RotationVector(angVelQuat) * recipTime);

			//		const HVec3 angVel(Maths::Imag(newRot * !rigid.m_Orientation) * recipTime * two);

			//		HVec3 linVel = positionDt * recipTime;

			//		
			//		rigid.m_LinearVelocity = weight * linVel + rest * rigid.m_LinearVelocity;
			//		rigid.m_AngularVelocity = angVel;
			//		rigid.m_LinearMomentum = rigid.m_LinearVelocity * rigid.m_Mass;
			//		rigid.m_AngularMomentum = rigid.m_AngularVelocity * rigid.m_InertiaTensor;

			//	}
			//}

			for(S32 a = 0; a < numBodies; ++a)
			{
				C_Body* pBody = pBodies[a];
				
				if(pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
				{
					SharedPhysics::C_RigidBody& rigid = pBody->GetBody();
					//What the initial relative pose was...
					
					const QuatTrans& initialRelativePose = initialPos[a];
					const QuatTrans& targetRelativePose = targetPos[a];
					//In local space
					const QuatTrans frameOfReferencePose(NearestSlerp(initialRelativePose, targetRelativePose, lerpScale));
				
					//Transform into world space, then work out velocities accordingly...

					const QuatTrans finalQTrans(frameOfReferencePose * referenceTrans);

					const HVec3 p(Maths::Trans(finalQTrans));

					const HF32 animationWeight(Maths::Load<Maths::HF32>(&animationWeights[a].m_AnimationWeight));
					const HF32 weight(Maths::Select(animationWeight.EQ(one), animationWeight, animationWeight * timeScale));
					const HF32 rest = one - weight;

					const HVec3 positionDt =(p - rigid.m_Position);
					const HVec3 linVel = positionDt * recipTime;
				
					const C_Quaternion targetRot(Maths::Quat(finalQTrans));
					//Project body into future position using current velocity...
					const C_Quaternion newRot(rigid.m_Orientation.ShortestSlerp(targetRot, weight));
					const HVec3 angVel(Maths::Imag(newRot * !rigid.m_Orientation) * recipTime * two);

					rigid.m_LinearVelocity = weight * linVel + rest * rigid.m_LinearVelocity;
					rigid.m_AngularVelocity = weight * angVel + rest * rigid.m_AngularVelocity;
					rigid.m_LinearMomentum = rigid.m_LinearVelocity * rigid.Mass();
					rigid.m_AngularMomentum = rigid.m_AngularVelocity * rigid.m_InertiaTensor;

				}
			}
		}

	}


	//After the solver solve all constraints, the animation system should 
	void C_RagdollController::Update(const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		PERF_EVENT("Gravity.RagdollController.Update");

		Spu::MemBaseMarker marker;

		const S32 numBodies = m_RagdollConfig.GetBodyCount();
		C_Body** pPUBodies = (C_Body**)Spu::FetchAllocUnalignedAsync(m_RagdollConfig.m_Bodies.m_pElems, sizeof(C_Body*) * numBodies, 3);
#ifdef PLATFORM_SPU
		C_Body** pBodies = (C_Body**)Spu::Alloc(sizeof(C_Body*) * numBodies);
		Spu::Wait(3);

		for(S32 i=0; i<numBodies; ++i)
		{
			pBodies[i] = (C_Body*)Spu::FetchAllocAsync(pPUBodies[i], sizeof(C_Body), 3);
		}

		Spu::Wait(3);
#else
		//C_Body** pBodies = (C_Body**)alloca(sizeof(C_Body*) * numBodies);
		C_Body** pBodies = pPUBodies;
#endif

		FollowFrameOfReference(time, pBodies);

		//const HF32 zero(Constants<HF32>::Zero());
		//const HF32 one=Constants<HF32>::One();
		
		//const HF32 recipTime(time.Reciprocal());
		//const HF32 two(Constants<HF32>::Two());
		const HF32 half(Constants<HF32>::Half());
		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = pBodies[i];
		
			if(pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			{
				SharedPhysics::C_RigidBody& pRigidBody = pBody->GetBody();
				const Maths::HVec3 newPos(pRigidBody.m_Position + pRigidBody.m_LinearVelocity*time);

				const Maths::C_Quaternion orientationDt(Maths::C_Quaternion((pRigidBody.m_AngularVelocity * half) * time) * pRigidBody.m_Orientation );
				const Maths::C_Quaternion newRot(pRigidBody.m_Orientation + orientationDt);
				pRigidBody.StoreVelocitiesAndPosition(pRigidBody.m_LinearVelocity, pRigidBody.m_AngularVelocity, newRot.Normalised(), newPos);
			}								
		}
#ifdef PLATFORM_SPU
		for(S32 i = 0; i<numBodies; ++i)
		{
			Spu::StoreAsync(&pPUBodies[i]->GetBody(), &pBodies[i]->GetBody(), sizeof(SharedPhysics::C_RigidBody), 3);
		}

		Spu::Wait(3);
#endif
	}


	void C_RagdollController::Synchronise()
	{
		const S32 numBodies = m_RagdollConfig.GetBodyCount();

		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			pBody->SynchroniseState();
			pBody->SynchronisePosRot();
		}
	}

	void C_RagdollController::Deactivate()
	{
		if(!m_Active || m_pScene == NULL)
			return;
		m_Active = false;
		if(m_pScene->m_pActiveRagdollControllers == this)
			m_pScene->m_pActiveRagdollControllers = m_pNext;
		if(m_pNext)
			m_pNext->m_pPrev = m_pPrev;
		if(m_pPrev)
			m_pPrev->m_pNext = m_pNext;
		m_pNext = m_pPrev = NULL;
	}

	void C_RagdollController::Activate()
	{
		if(m_Active || m_pScene == NULL)
			return;

		m_Active = true;
		m_pNext = m_pScene->m_pActiveRagdollControllers;
		if(m_pScene->m_pActiveRagdollControllers)
			m_pScene->m_pActiveRagdollControllers->m_pPrev = this;
		m_pPrev = NULL;
		m_pScene->m_pActiveRagdollControllers = this;
	}

	void C_RagdollController::Stop()
	{
		//Make all the kinematic parts static!!!
		const S32 numBodies = m_RagdollConfig.GetBodyCount();

		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			pBody->MakeStatic();
		}

		Deactivate();
	}

	void C_RagdollController::Start()
	{
		//Make all the kinematic parts static!!!
		const S32 numBodies = m_RagdollConfig.GetBodyCount();

		for(S32 i =0; i<numBodies; ++i)
		{
			C_Body* pBody = m_RagdollConfig.GetBody(i);
			pBody->MakeKinematic();
		}

		Activate();
	}
}

