#ifndef ANIMATION_ACTOR_H
#define ANIMATION_ACTOR_H

#include <btl/containers/vector.h>
#include <animation/core/CommandList.h>
#include <animation/pose/Pose.h>
#include <animation/pose/EventLogger.h>
// #include <animation/skeleton/JointConstraintList.h>
#include <animation/skeleton/SkeletonInstance.h>
//#include <animation/dynamics/SpringJoint.h>
#include "AnimationSet.h"

namespace Animation
{
	class C_Skeleton;
	class C_Cycle;
	class C_CycleSkeletonMapping;
	class C_JointConstraint;
}


class ALIGN16 C_Actor
{
public:
	C_Actor();
	~C_Actor();
	

	void Create(
		U32 index
	);
	
	void SpuFetch();
	void SpuStore();


	void Update( F32 deltaTime );
	
	void BuildCommandList( F32 deltaTime );
	void EvaluateCommandList();
	void PostUpdate();	

	
//	const Animation::C_Evaluation& GetEvaluation() const	{ return m_evaluation; }
	const Animation::C_Pose& GetPose() const { return m_pose; }
	Animation::C_Pose& GetPose() { return m_pose; }
	const Animation::C_Skeleton* GetSkeleton() const	{ return m_skeletonInstance.GetSkeleton(); }
	Animation::C_SkeletonInstance& GetSkeletonInstance() { return m_skeletonInstance; }
	Animation::C_CommandList& GetCommandList() { return m_commandList; }
	
	const Maths::HVec3 GetWorldPos() const { return Maths::HVec3( m_actorRootTransform.GetTranslation() ); }
	const Maths::C_Quaternion GetWorldRotation() const { return Maths::C_Quaternion( m_actorRootTransform.GetRotation() ); }
	
	void InitialiseDynamicJoints();
	void UpdateDynamicJoints( F32 deltaTime );
	
	//void EvaluateDynamicJoint( const Animation::C_JointConstraint& jointConstraint, Animation::C_Evaluation& evaluation );
	

//private:

	//const Animation::C_Skeleton* m_skeleton;
	//Spu::Ptr< const Animation::C_Cycle > m_cycle;
	//Spu::Ptr< const Animation::C_CycleSkeletonMapping > m_cycleSkeletonMapping;
	Maths::HVec3 m_worldPos;
	
	S32 m_gameRootJointIndex;
	
	//Btl::Vector< Animation::C_SpringJoint, Btl::VectorParameters_Static< 16 > > m_springJoints;

	Animation::C_JointTransform m_actorRootTransform;
	Animation::C_JointTransform m_prevGameRootTransform;
	Animation::C_CommandList m_commandList;	
	Animation::C_SkeletonInstance m_skeletonInstance;
	Animation::C_EventLogger m_eventLogger;
	
	Btl::Vector<Maths::Mat43> m_worldJointMatrices;
	
	Animation::C_Pose m_pose;
	C_AnimationSet m_animSet;
	
	F32 m_time;
	F32 m_cycleTimes[ 3 ];
	
	U32 m_index;
	
	F32 m_runBlend;
	F32 m_runDamping;
	F32 m_direction;
	
	F32 m_targetDirX;
	F32 m_targetDirZ;
	F32 m_dirDamping;
	F32 m_dirX;
	F32 m_dirZ;
	
	F32 m_deltaTime;
	
	bool m_isPlayerControlled;
};

#endif

