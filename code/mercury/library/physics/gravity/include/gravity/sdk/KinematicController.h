#ifndef KINEMATIC_CONTROLLER_H
#define KINEMATIC_CONTROLLER_H

#include <gravity/SharedPhysics/AllocationManager.h>

/*
 *	Kinematic controller - a simple interface to animate/move a kinematic body!
 */

namespace Gravity
{
	class C_Body;
	class C_Scene;
	class C_KinematicController : public SharedPhysics::C_BlockItem<C_KinematicController>
	{
		friend class C_Scene;

		C_KinematicController* m_pPrev;
		Gravity::C_Scene* m_pScene;
		C_Body* m_pReferenceBody;
		C_Body* m_pBody;
		Maths::QuatTrans m_TargetPose;
		Maths::QuatTrans m_TargetRelativePose;
		Maths::QuatTrans m_InitialRelativePose;
		Maths::HF32 m_TimeStep;
		Maths::HF32 m_RemainingReferenceTime;
		Maths::HF32 m_ReciprocalFrameOfReferenceTime;
		bool m_Active;

	public:

		C_KinematicController(C_Body* pBody = NULL) : m_pBody(pBody), m_pReferenceBody(NULL), m_pPrev(NULL),
			m_Active(false), m_pScene(NULL), m_TimeStep(Maths::Constants<Maths::HF32>::Zero()), 
			m_RemainingReferenceTime(Maths::Constants<Maths::HF32>::Zero()), m_ReciprocalFrameOfReferenceTime(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		void SetReferenceBody(C_Body* pBody)
		{
			m_pReferenceBody = pBody;
		}

		void SetScene(Gravity::C_Scene* pScene)
		{
			m_pScene = pScene;
		}

		void SetTargetRelativeTransformation(const Maths::HMat43 PCREF transform, const Maths::HF32 PCREF timeStep);
		void SetTargetRelativeTransformation(const Maths::C_QuatTrans PCREF qTrans, const Maths::HF32 PCREF timeStep);

		//NB - this transform comes from the element matrix, so we need to apply body offsets to it!
		void SetTargetTransformation(const Maths::HMat43 PCREF transform, const Maths::HF32 PCREF timeStep);
		void SetTargetTransformation(const Maths::C_QuatTrans& qTrans, const Maths::HF32 PCREF timeStep);
		void SetTargetTransformationAndVelocity(const Maths::HMat43 PCREF transform, 
			const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep);
		void SetTargetTransformationAndVelocity(const Maths::C_QuatTrans& qTrans, 
			const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep);

		void SetTargetTransformationAndVelocityZeroOffset(const Maths::HMat43 PCREF transform, 
			const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep);
		void SetTargetTransformationAndVelocityZeroOffset(const Maths::C_QuatTrans& qTrans, 
			const Maths::HVec3 PCREF linearVelocity, const Maths::HVec3 PCREF angularVelocity, 
			const Maths::HF32 PCREF timeStep);
		void Teleport(const Maths::HMat43 PCREF transform);
		void Stop();
		void Update(const Maths::HF32 PCREF time);
		C_Body* GetBody();
		C_Body* GetReferenceBody();

		bool operator < (const C_KinematicController& controller);
		bool operator <= (const C_KinematicController& controller);
		bool operator > (const C_KinematicController& controller);
		bool operator >= (const C_KinematicController& controller);
		bool operator == (const C_KinematicController& controller);
		bool operator != (const C_KinematicController& controller);

	private:
		void SetTargetTransformation(const Maths::HVec3 PCREF translation, const Maths::C_Quaternion PCREF rotation, const Maths::HF32 PCREF timeStep);
		void RecordCurrentFrameOfReferencePose(Gravity::C_Body* pSpuBody, Gravity::C_Body* pReferenceBody);
		void FollowFrameOfReference(const Maths::HF32 PCREF time, Gravity::C_Body* pSpuBody, Gravity::C_Body* pReferenceBody);
		void Activate();
		void Deactivate();
	};
}


#endif
