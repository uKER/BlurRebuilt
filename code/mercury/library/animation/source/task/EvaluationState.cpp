#include "stdafx.h"
#include <animation/task/EvaluationState.h>
#include <animation/task/TaskManager.h>
#include <system/Perf.h>

namespace Animation
{

C_EvaluationState::C_EvaluationState(U32 maxCommands):
	m_commandList(maxCommands)
{
}

C_EvaluationState::~C_EvaluationState()
{
	// Flush all task groups that are potentially accessing this pose.
	FlushIfNotWriteable();
}

C_EvaluationState* C_EvaluationState::New(U32 maxCommands)
{
	return new C_EvaluationState(maxCommands);
}

void C_EvaluationState::Create(const C_PoseFormat& poseFormat, const C_Skeleton* skeleton)
{
	Flush();
	
	m_pose.Create(poseFormat);
	m_poseMatrixCache.Create(m_pose);
	
	if (skeleton != NULL)
	{
		m_skeletonInstance.Create(*skeleton);
	}
	else
	{
		m_skeletonInstance.Destroy();
	}
}



C_CommandList& C_EvaluationState::GetCommandList()
{
	FlushIfNotWriteable();
	return m_commandList;
}

const C_CommandList& C_EvaluationState::GetCommandList() const
{
	FlushIfNotReadable();
	return m_commandList;
}

C_Pose& C_EvaluationState::GetPose()
{
	FlushIfNotWriteable();
	return m_pose;
}

const C_Pose& C_EvaluationState::GetPose() const
{
	FlushIfNotReadable();
	return m_pose;
}

const C_Pose& C_EvaluationState::GetPoseReadOnly() const
{
	FlushIfNotReadable();
	return m_pose;
}

const C_Pose& C_EvaluationState::GetPoseNonBlocking() const
{
	return m_pose;
}

C_PoseMatrixCache& C_EvaluationState::GetPoseMatrixCache()
{
	FlushIfNotWriteable();
	return m_poseMatrixCache;
}

const C_PoseMatrixCache& C_EvaluationState::GetPoseMatrixCache() const
{
	FlushIfNotReadable();
	return m_poseMatrixCache;
}

const C_PoseMatrixCache& C_EvaluationState::GetPoseMatrixCacheReadOnly() const
{
	FlushIfNotReadable();
	return m_poseMatrixCache;
}

const C_PoseMatrixCache& C_EvaluationState::GetPoseMatrixCacheNonBlocking() const
{
	return m_poseMatrixCache;
}

C_SkeletonInstance& C_EvaluationState::GetSkeletonInstance()
{
	return m_skeletonInstance;
}

const C_SkeletonInstance& C_EvaluationState::GetSkeletonInstance() const
{
	return m_skeletonInstance;
}

const C_Skeleton* C_EvaluationState::GetSkeleton() const
{
	return m_skeletonInstance.GetSkeleton();
}

void C_EvaluationState::ScheduleEvaluationTask(
	bool updatePoseMatrixCache
)
{
	Flush();

	Animation::TaskManager().ScheduleEvaluationTask(
		Animation::E_TaskGroup_Evaluate,
		m_pose,
		m_commandList,
		m_skeletonInstance.GetSkeleton(),
		updatePoseMatrixCache ? &m_poseMatrixCache : NULL
	);
}

void C_EvaluationState::Flush() const
{
	FlushIfNotWriteable();
}

void C_EvaluationState::FlushIfNotReadable() const
{
	if (!C_Pose::IsReadable(&m_pose))
	{
		// Only evaluation tasks write to poses.
		Animation::TaskManager().Flush(Animation::E_TaskGroup_Evaluate);
		BIZ_ASSERT(C_Pose::IsReadable(&m_pose));
	}
}

void C_EvaluationState::FlushIfNotWriteable() const
{
	if (!C_Pose::IsWriteable(&m_pose))
	{
		// Flush all other tasks that depend on evaluation tasks so they get a chance to the data before
		// it's modified.
		PERF_EVENT("Animation.EvaluationState.FlushIfNotWriteable");
		Animation::TaskManager().FlushDependents(Animation::E_TaskGroup_Evaluate);
		BIZ_ASSERT(C_Pose::IsWriteable(&m_pose));
	}
}

} // end Animation

