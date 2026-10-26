#include "StdAfx.h"
#include "gravity/sdk/KinematicController.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/Scene.h"
#include "gravity/narrowphase/Shape.h"
#include <system/perf.h>

namespace Gravity
{
	void C_KinematicController::SetTargetRelativeTransformation(const Maths::HMat43 PCREF transform, const Maths::HF32 PCREF timeStep)
	{
		if(m_pBody->IsExternalStatic())
			m_pBody->MakeKinematic();
		Activate();
		BIZ_ASSERT(transform.Rotation().Finite());
		BIZ_ASSERT(transform.Translation().Finite());
		Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos((pShape->m_ElementOffset)*transform);
		this->m_TargetRelativePose = Maths::MakeQuatTrans(Maths::HMat43(transform.Rotation(), newPos));
		m_RemainingReferenceTime = timeStep;
		m_ReciprocalFrameOfReferenceTime = timeStep.Reciprocal();
	}
	void C_KinematicController::SetTargetRelativeTransformation(const Maths::C_QuatTrans PCREF qTrans, const Maths::HF32 PCREF timeStep)
	{
		if(m_pBody->IsExternalStatic())
			m_pBody->MakeKinematic();
		Activate();
		BIZ_ASSERT(qTrans.Quaternion().Finite());
		BIZ_ASSERT(qTrans.Translation().Finite());
		Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos(pShape->m_ElementOffset * qTrans);
		m_TargetRelativePose = Maths::MakeQuatTrans(qTrans.Quaternion(), newPos);
		m_RemainingReferenceTime = timeStep;
		m_ReciprocalFrameOfReferenceTime = timeStep.Reciprocal();
	}

	void C_KinematicController::FollowFrameOfReference(const Maths::HF32 PCREF time, Gravity::C_Body* pSpuBody, Gravity::C_Body* pReferenceBody)
	{
		PERF_EVENT("C_KinematicController::FollowFrameOfReference");
		using namespace Maths;
		//Updated target position... update current target position. 
		//if(m_pReferenceBody)
		{
			//Gravity::C_Body* pReferenceBody = (Gravity::C_Body*)Spu::FetchAlloc(m_pReferenceBody, sizeof(Gravity::C_Body));
			RecordCurrentFrameOfReferencePose(pSpuBody, pReferenceBody);
			const Maths::HF32 recipTime(time.Reciprocal());
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			SharedPhysics::C_RigidBody& referenceBody = m_pReferenceBody->GetBody();
			//Where the reference body actually is in world space...
			const Maths::QuatTrans referenceTrans = MakeQuatTrans(referenceBody.m_Orientation, referenceBody.m_Position);

			m_RemainingReferenceTime = Maths::Max(zero, m_RemainingReferenceTime - time);
			const Maths::HF32 scale(one - (m_RemainingReferenceTime*m_ReciprocalFrameOfReferenceTime));

			//if(m_pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			{
				SharedPhysics::C_RigidBody& rigid = pSpuBody->GetBody();
				BIZ_ASSERT(rigid.ValidState());
				//What the initial relative pose was...
				//In local space
				const Maths::QuatTrans frameOfReferencePose(Maths::NearestSlerp(m_InitialRelativePose, m_TargetRelativePose, scale));
			
				//Transform into world space, then work out velocities accordingly...

				const Maths::QuatTrans finalQTrans(frameOfReferencePose * referenceTrans);

				const Maths::HVec3 p(Maths::Trans(finalQTrans));

				const HVec3 positionDt =(p - rigid.m_Position);
					
				const C_Quaternion targetRot(Maths::Quat(finalQTrans));

				//Project body into future position using current velocity...
				const Maths::C_Quaternion newRot(targetRot);
				const HF32 cosAngle(newRot | rigid.m_Orientation);
				const C_Quaternion oldRot(Maths::Select(cosAngle.GE(zero), rigid.m_Orientation, -rigid.m_Orientation));

				const Maths::C_Quaternion angVelQuat(Maths::NearestDifference(oldRot, newRot));

				//HVec3 angVel(Maths::Imag(newRot * !oldRot) * recipTime * two);

				/*const Maths::HVec3 normalisedImag(Maths::RotationAxis(angVelQuat));
				const Maths::HF32 angle(-Maths::RotationAngle(angVelQuat));*/
				
				const Maths::HVec3 angVel(Maths::RotationVector(angVelQuat) * recipTime);

				HVec3 linVel = positionDt * recipTime;

				rigid.m_LinearVelocity = linVel;
				rigid.m_AngularVelocity = angVel;
				rigid.m_LinearMomentum = rigid.m_LinearVelocity * rigid.Mass();
				rigid.m_AngularMomentum = rigid.m_AngularVelocity * rigid.m_InertiaTensor;
				BIZ_ASSERT(rigid.ValidState());
			}

		}
	}


	void C_KinematicController::RecordCurrentFrameOfReferencePose(Gravity::C_Body* pSpuBody, Gravity::C_Body* pReferenceBody)
	{
		using namespace Maths;
		//Record this current pose of the body parts relative to the frame of reference object...
		//if(m_pReferenceBody)
		{
			SharedPhysics::C_RigidBody& referenceBody = pReferenceBody->GetBody();
			const Maths::QuatTrans referenceTrans = Maths::MakeQuatTrans(referenceBody.m_Orientation, referenceBody.m_Position);
			const Maths::QuatTrans inverseTrans(Maths::Inverse(referenceTrans));
			SharedPhysics::C_RigidBody& rigid = pSpuBody->GetBody();
			const Maths::QuatTrans qTrans = Maths::MakeQuatTrans(rigid.m_Orientation, rigid.m_Position);
			//Now work this out in local space to the reference body...
			m_InitialRelativePose = qTrans * inverseTrans;
		}
	}


	void C_KinematicController::SetTargetTransformation(const Maths::HMat43 PCREF transform, const Maths::HF32 PCREF timeStep)
	{
		BIZ_ASSERT(transform.Rotation().Finite());
		BIZ_ASSERT(transform.Translation().Finite());
		Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos((pShape->m_ElementOffset)*transform);
		const Maths::C_Quaternion quat(transform.Rotation());
		
		SetTargetTransformation(newPos, quat, timeStep);
		
	}
	
	void C_KinematicController::SetTargetTransformation(const Maths::C_QuatTrans& qTrans, const Maths::HF32 PCREF timeStep)
	{
		BIZ_ASSERT(qTrans.Quaternion().Finite());
		BIZ_ASSERT(qTrans.Translation().Finite());
		Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos(pShape->m_ElementOffset * qTrans);
		const Maths::C_Quaternion quat(qTrans.Quaternion());
		SetTargetTransformation(newPos, quat, timeStep);
		
	}

	void C_KinematicController::SetTargetTransformationAndVelocity(const Maths::HMat43 PCREF transform, 
		const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep)
	{
		BIZ_ASSERT(transform.Rotation().Finite());
		BIZ_ASSERT(transform.Translation().Finite());
		BIZ_ASSERT(linearVelocity.Finite());
		BIZ_ASSERT(angularVelocity.Finite());
		SetTargetTransformationAndVelocity(Maths::C_QuatTrans(transform), linearVelocity, angularVelocity, timeStep);
	}

	void C_KinematicController::SetTargetTransformationAndVelocityZeroOffset(const Maths::HMat43 PCREF transform, 
		const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep)
	{
		BIZ_ASSERT(transform.Rotation().Finite());
		BIZ_ASSERT(transform.Translation().Finite());
		BIZ_ASSERT(linearVelocity.Finite());
		BIZ_ASSERT(angularVelocity.Finite());
		SetTargetTransformationAndVelocityZeroOffset(Maths::C_QuatTrans(transform), linearVelocity, angularVelocity, timeStep);
	}

	
	void C_KinematicController::SetTargetTransformationAndVelocityZeroOffset(const Maths::C_QuatTrans& qTrans, const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep)
	{
		if(m_pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			m_pBody->MakeKinematic();
		Activate();
		BIZ_ASSERT(qTrans.Quaternion().Finite());
		BIZ_ASSERT(qTrans.Translation().Finite());
		BIZ_ASSERT(linearVelocity.Finite());
		BIZ_ASSERT(angularVelocity.Finite());
		m_TargetPose = Maths::MakeQuatTrans(qTrans.Quaternion(), qTrans.Translation());
		SharedPhysics::C_RigidBody& pRigid = m_pBody->GetBody();
		pRigid.m_LinearVelocity = linearVelocity;
		pRigid.m_AngularVelocity = angularVelocity;	
		m_TimeStep = timeStep;
	}

	
	void C_KinematicController::SetTargetTransformationAndVelocity(const Maths::C_QuatTrans& qTrans, const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep)
	{
		if(m_pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
			m_pBody->MakeKinematic();
		Activate();
		BIZ_ASSERT(qTrans.Quaternion().Finite());
		BIZ_ASSERT(qTrans.Translation().Finite());
		BIZ_ASSERT(linearVelocity.Finite());
		BIZ_ASSERT(angularVelocity.Finite());

		Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos((pShape->m_ElementOffset)*qTrans);
		const Maths::C_Quaternion quat(qTrans.Quaternion());

		m_TargetPose = Maths::MakeQuatTrans(quat, newPos);

		SharedPhysics::C_RigidBody& pRigid = m_pBody->GetBody();
		pRigid.m_LinearVelocity = linearVelocity;
		pRigid.m_AngularVelocity = angularVelocity;	
		
		m_TimeStep = timeStep;
	}
	
	void C_KinematicController::SetTargetTransformation(const Maths::HVec3 PCREF translation, const Maths::C_Quaternion PCREF rotation, const Maths::HF32 PCREF timeStep)
	{
	
		if(m_pBody->IsExternalStatic())
			m_pBody->MakeKinematic();
		Activate();
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 recipTime(timeStep.Reciprocal());

		SharedPhysics::C_RigidBody& pRigid = m_pBody->GetBody();
		BIZ_ASSERT(pRigid.ValidState());
		const Maths::HVec3 trans(translation - pRigid.m_Position);
		const Maths::HVec3 linVel(trans * recipTime);
		
		/*const Maths::HF32 cosAngle(pRigid.m_Orientation | rotation);
		
		const Maths::C_Quaternion oldRot(Maths::Select(cosAngle.GE(zero), pRigid.m_Orientation, -pRigid.m_Orientation));
		
		const Maths::C_Quaternion conjOldRot(!oldRot);*/
		/*const Maths::C_Quaternion conjOldRot(!pRigid.m_Orientation);
		
		const Maths::C_Quaternion angVelQuat(rotation * conjOldRot);
		const Maths::HVec3 angVel(Maths::Select(Maths::Real(angVelQuat).W().GE(zero), Maths::Imag( angVelQuat ), -Maths::Imag( angVelQuat )) * two * recipTime);*/

		const Maths::C_Quaternion angVelQuat(Maths::NearestDifference(pRigid.m_Orientation, rotation));
		
		//const Maths::HVec3 angVel(Maths::Imag(angVelQuat) * two * recipTime);
		/*const Maths::HVec3 normalisedImag(Maths::RotationAxis(angVelQuat));
		const Maths::HF32 angle(-Maths::RotationAngle(angVelQuat));*/
		const Maths::HVec3 angVel(RotationVector(angVelQuat) * recipTime);

		//const Maths::HVec3 angVel(-Maths::Imag( Maths::NearestDifference(pRigid.m_Orientation, rotation)) * two * recipTime);
		m_TimeStep = timeStep;

		/*pRigid.StoreVelocitiesAndPosition(linVel, angVel, rotation, translation);
		m_pBody->SynchroniseState();
		m_pBody->SynchronisePosRot();*/
		m_TargetPose = Maths::MakeQuatTrans(rotation, translation);
		pRigid.m_LinearVelocity = linVel;
		pRigid.m_AngularVelocity = angVel;
		
		BIZ_ASSERT(pRigid.ValidState());
	}

	void C_KinematicController::Teleport(const Maths::HMat43 PCREF transform)
	{
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//SharedPhysics::C_RigidBody& pRigid = m_pBody->GetBody();
		/*Collision::C_Shape* pShape = m_pBody->GetShape();
		const Maths::HVec3 newPos((pShape->m_ElementOffset)*transform);*/
		/*pRigid.StoreVelocitiesAndPosition(zero, zero, transform.Rotation(), newPos);
		m_pBody->SynchroniseState();
		m_pBody->SynchronisePosRot();*/
		m_pBody->ElementMatrix(transform);
		m_pBody->LinearVelocity(zero);
		m_pBody->AngularVelocity(zero);
	}

	void C_KinematicController::Stop()
	{
		SharedPhysics::C_RigidBody& pRigid = m_pBody->GetBody();
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		pRigid.m_LinearVelocity = pRigid.m_AngularVelocity = pRigid.m_LinearMomentum = pRigid.m_AngularMomentum = zero;
		if(m_pBody->IsExternalKinematic())
		{
			m_pBody->MakeStatic();
		}
		Deactivate();
	}

	void C_KinematicController::Update(const Maths::HF32 PCREF time)
	{
		//PERF_EVENT("Gravity.Kinematic.Update");

		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());;
		Spu::MemBaseMarker marker;
		Gravity::C_Body* pSpuBody = (Gravity::C_Body*)Spu::FetchAllocAsync(m_pBody, sizeof(Gravity::C_Body), 2);
		if(m_pReferenceBody)
		{
			Gravity::C_Body* pReferenceBody = (Gravity::C_Body*)Spu::FetchAllocAsync(m_pReferenceBody, sizeof(Gravity::C_Body), 2);
			Spu::Wait(2);
			FollowFrameOfReference(time, pSpuBody, pReferenceBody);
		}
		else
			Spu::Wait(2);

		if((!pSpuBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject)) || 
			m_TimeStep <= zero)
			return;

		
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

		const Maths::HF32 remainingTime(Maths::Min(m_TimeStep, time));
		m_TimeStep -= time;
		
		SharedPhysics::C_RigidBody& pRigid = pSpuBody->GetBody();
		const Maths::HVec3 newPos(pRigid.m_Position + pRigid.m_LinearVelocity*remainingTime);

		const Maths::C_Quaternion orientationDt(Maths::C_Quaternion((pRigid.m_AngularVelocity * half) * remainingTime) * pRigid.m_Orientation );
		const Maths::C_Quaternion newRot(pRigid.m_Orientation + orientationDt);
		
		pRigid.StoreVelocitiesAndPosition(pRigid.m_LinearVelocity, pRigid.m_AngularVelocity, newRot.Normalised(), newPos);
		

#ifdef PLATFORM_SPU
		Spu::Store(m_pBody, pSpuBody, sizeof(C_Body));
#endif

		/*m_pBody->SynchronisePosRot();
		m_pBody->SynchroniseState();*/
		
		//If timeStep == 0, then trigger stopping kinematic + send off message?
	}

	C_Body* C_KinematicController::GetBody()
	{
		return m_pBody;
	}

	C_Body* C_KinematicController::GetReferenceBody()
	{
		return m_pReferenceBody;
	}

	void C_KinematicController::Activate()
	{
		if(m_Active || m_pScene == NULL)
			return;

		m_Active = true;
		m_pNext = m_pScene->m_pActiveControllers;
		if(m_pScene->m_pActiveControllers)
			m_pScene->m_pActiveControllers->m_pPrev = this;
		m_pPrev = NULL;
		m_pScene->m_pActiveControllers = this;

	}
	void C_KinematicController::Deactivate()
	{
		if(!m_Active || m_pScene == NULL)
			return;
		m_Active = false;
		if(m_pScene->m_pActiveControllers == this)
			m_pScene->m_pActiveControllers = m_pNext;
		if(m_pNext)
			m_pNext->m_pPrev = m_pPrev;
		if(m_pPrev)
			m_pPrev->m_pNext = m_pNext;
		m_pNext = m_pPrev = NULL;
	}


	bool C_KinematicController::operator < (const C_KinematicController& controller)
	{
		return *m_pBody < *controller.m_pBody;
	}
	bool C_KinematicController::operator <= (const C_KinematicController& controller)
	{
		return *m_pBody <= *controller.m_pBody;
	}
	bool C_KinematicController::operator > (const C_KinematicController& controller)
	{
		return *m_pBody > *controller.m_pBody;
	}
	bool C_KinematicController::operator >= (const C_KinematicController& controller)
	{
		return *m_pBody >= *controller.m_pBody;
	}
	bool C_KinematicController::operator == (const C_KinematicController& controller)
	{
		return *m_pBody == *controller.m_pBody;
	}
	bool C_KinematicController::operator != (const C_KinematicController& controller)
	{
		return *m_pBody != *controller.m_pBody;
	}

}
