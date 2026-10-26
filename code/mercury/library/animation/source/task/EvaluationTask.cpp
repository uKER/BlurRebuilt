#include "stdafx.h"
#include <animation/task/EvaluationTask.h>
#include <animation/task/EvaluationTaskModule.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/deferred/CommandProcessor.h>
#include <animation/deferred/CommandList.h>
#include <system/Scheduler.h>
#include <system/Perf.h>

namespace Animation
{

//
// Helper class for setting root joint to identity.
//
class C_ResetRootTransform
{
public:
	C_ResetRootTransform(C_Pose& pose, bool reset):
		m_pose(pose)
	{
		m_savedRootTransform = pose.GetJointTransform(0);
		if (reset)
		{
			pose.GetJointTransform(0).SetIdentity();
		}
	}
	
	~C_ResetRootTransform()
	{
		m_pose.GetJointTransform(0) = m_savedRootTransform;
	}
	
private:
	C_Pose&				m_pose;
	C_JointTransform	m_savedRootTransform;
};

//
// C_EvaluationTask
//

C_EvaluationTask::C_EvaluationTask():
	m_skeleton(NULL),
	m_pose(NULL),
	m_commandList(NULL)
{
}
	
bool C_EvaluationTask::Run() const
{
	return RunBatch(this, 1);
}


#ifdef PLATFORM_SPU
#define ANIM_SCRATCH_ALLOC(numBytes)	ptr = (U8*) Spu::Alloc(numBytes)
#else
#define ANIM_SCRATCH_ALLOC(numBytes)	(U8*) alloca(numBytes)
#endif

// This function allocates local store memory via poseMatrixCache->SpuAlloc(). Some memory is currently
// wasted by the command list data, which is allocated before poseMatrixCache->SpuAlloc().
bool C_EvaluationTask::Run(C_Pose* localPose, C_PoseMatrixCache* localPoseMatrixCache, const C_Skeleton* localSkeleton) const
{
#ifndef PLATFORM_SPU
	BIZ_ASSERT(localPose == m_pose);
	BIZ_ASSERT(localPoseMatrixCache == m_poseMatrixCache);
	BIZ_ASSERT(localSkeleton == m_skeleton);
#endif

	const U32 kDmaTagA = 1;
	
	bool ok = true;
	
	{
		// Fetch command list.
		C_CommandList* commandList = (C_CommandList*) Spu::FetchAlloc(m_commandList, sizeof(C_CommandList));
		commandList->SpuFetch();
	
		Animation::C_CommandProcessor cmdProcessor;
		ok = cmdProcessor.ExecuteCommandList(*commandList, *localPose, localSkeleton, localPoseMatrixCache);
	
#ifdef PLATFORM_SPU
		// Store back command list.
		Spu::StoreAsync(m_commandList, commandList, sizeof(C_CommandList), kDmaTagA);
#endif
	}
	
	
#ifdef PLATFORM_SPU
	// Store back internal pose data, but not the pose object itself as it depends on its
	// reader and writer counts being in main memory.
	localPose->SpuStoreAsync(kDmaTagA);
	Spu::Wait(kDmaTagA);
#endif		
	
	Animation::C_Pose::ReleaseWriter(m_pose);
		
	return ok;
}

bool C_EvaluationTask::RunBatch(const C_EvaluationTask* tasks, U32 numTasks)
{
	if (tasks == NULL || numTasks == 0)
	{
		return true;
	}
	
	Spu::MemBaseMarker topMemBaseMarker;
	Animation::C_CommandProcessor cmdProcessor;
	
	
	// Reserve DMA tag 0 Spu::Fetch functions
	const U32 kTagA = 1;
	const U32 kTagB = 2;
	
	bool allOk = true;
	
	for (U32 i = 0; i < numTasks; ++i)
	{
		Spu::MemBaseMarker taskMemBaseMarker;

		const C_EvaluationTask* task = &tasks[i];

		const C_Skeleton* skeleton = NULL;

		// Fetch skeleton.
		if (task->m_skeleton != NULL)
		{
			skeleton = (const C_Skeleton*) Spu::FetchAllocAsync(task->m_skeleton, sizeof(C_Skeleton), kTagA);
		}

		// Fetch pose object.
		C_Pose* pose = (C_Pose*) Spu::FetchAllocAsync(task->m_pose, sizeof(C_Pose), kTagA);
		
		// Fetch pose matrix cache object.
		C_PoseMatrixCache* poseMatrixCache = NULL;
		if (task->m_poseMatrixCache != NULL)
		{
			poseMatrixCache = (C_PoseMatrixCache*) Spu::FetchAllocAsync(task->m_poseMatrixCache, sizeof(C_PoseMatrixCache), kTagA);
		}
		
		// Wait for objects to arrive.
		Spu::Wait(kTagA);

		// Fetch skeleton data.
		if (skeleton)
		{
			skeleton->SpuFetchHierarchyAsync(kTagA);
			skeleton->SpuFetchLocalBasePoseAsync(kTagA);
		}
		
		// Fetch pose data.
		pose->SpuFetchAsync(kTagA);

		// Wait for pose and skeleton data to arrive.
		Spu::Wait(kTagA);
		
		// Run the update which will store back command list and matrix cache, but not the pose.
		bool ok = task->Run(pose, poseMatrixCache, skeleton);
		allOk = allOk && ok;
	}

	return allOk;
}

#undef ANIM_SCRATCH_ALLOC

#ifndef PLATFORM_SPU
bool C_EvaluationTask::Schedule(S32 group, S32 policy)
{
	return Schedule(this, 1, group, policy);
}

bool C_EvaluationTask::Schedule(const C_EvaluationTask* tasks, U32 numTasks, S32 group, S32 policy)
{
	if (numTasks == 0)
	{
		return false;
	}

	bool scheduled = true;
	if (Scheduler::Active())
	{
		C_EvaluationTaskParams params;
		params.tasks = tasks;
		params.numTasks = numTasks;
		
		Scheduler::AddTask(
			AnimationEvaluationTaskModule,
			0, // input ptr
			0, // output ptr
			group,	// group ID
			policy, // policy
			&params,
			sizeof(params)
		);
	}
	else
	{
		// If not using threading, then execute the task immediately.
		RunBatch(tasks, numTasks);
		scheduled = false;
	}
	return scheduled;
}
#endif

} // end Animation




