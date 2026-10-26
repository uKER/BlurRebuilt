#include "AnimatedModelPch.h"
#include <animated_model/UpdateTask.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/skeleton/Skeleton.h>
#include <system/Scheduler.h>
#include <system/Perf.h>

namespace AnimatedModel
{
	//
	// Update task scheduler module interface.
	//
	
	struct C_UpdateTaskParams
	{
		const C_UpdateTask*		tasks;
		U32						numTasks;
	};
	
	/// Scheduler module for C_EvaluationTask.
	void RunUpdateTasks(Scheduler::Task* task)
	{
		PERF_EVENT("AnimatedModelUpdateTask");
		Spu::MemBaseMarker memBaseMarker;
		const C_UpdateTaskParams* params = (const C_UpdateTaskParams*)&task->ParamData[0];
		const C_UpdateTask* tasks = (const C_UpdateTask*) Spu::FetchAlloc(params->tasks, params->numTasks * sizeof(C_UpdateTask));
		C_UpdateTask::Run(tasks, params->numTasks);
	}
}

JQ_DECLARE_MODULE(AnimatedModelUpdateTaskModule, AnimatedModel::RunUpdateTasks, animated_model_update_task);

namespace AnimatedModel
{

C_UpdateTask::C_UpdateTask():
	animatedModelInstance(NULL),
	skeleton(NULL),
	pose(NULL),
	poseMatrixCache(NULL),
	commandList(NULL)
{
}

bool C_UpdateTask::IsValid() const
{
	return animatedModelInstance != NULL && pose != NULL;
}

void C_UpdateTask::Invalidate()
{
	if (IsValid())
	{
		ReleaseLocks();
	}
	animatedModelInstance = NULL;
	pose = NULL;
}

void C_UpdateTask::Run() const
{
	Run(this, 1);	
	PostUpdate();
}

void C_UpdateTask::Run(const C_UpdateTask* tasks, U32 numTasks)
{
	for (U32 i = 0; i < numTasks; ++i)
	{
		const C_UpdateTask* task = &tasks[i];

		Spu::MemBaseMarker memMarker;
		
		// Fetch skeleton.
		const Animation::C_Skeleton* skeleton = NULL;
		if (task->skeleton != NULL)
		{
			skeleton = (const Animation::C_Skeleton*) Spu::FetchAlloc(task->skeleton, sizeof(Animation::C_Skeleton));
			U32 kDmaTag = 1;
			skeleton->SpuFetchHierarchyAsync(kDmaTag);
			skeleton->SpuFetchLocalBasePoseAsync(kDmaTag);
			skeleton->SpuFetchInverseWorldBasePoseAsync(kDmaTag);
			Spu::Wait(kDmaTag);
		}
		
		// Fetch pose.
		Animation::C_Pose* pose = (Animation::C_Pose*) Spu::FetchAlloc(task->pose, sizeof(Animation::C_Pose));
		pose->SpuFetch();
		
		// Fetch pose matrix cache.
		Animation::C_PoseMatrixCache* poseMatrixCache = (Animation::C_PoseMatrixCache*) Spu::FetchAlloc(task->poseMatrixCache, sizeof(Animation::C_PoseMatrixCache));
				
		if (task->commandList != NULL)
		{
			Animation::C_EvaluationTask evalTask;
			evalTask.m_pose = task->pose;
			evalTask.m_poseMatrixCache = task->poseMatrixCache;
			evalTask.m_commandList = task->commandList;
			evalTask.m_skeleton = task->skeleton;

			// Run animation evaluation task.
			// Pose data and matrix cache will be stored back to main memory.
			// Matrix cache matrices will remain allocated so that they can be used to update the 
			// animated model instance in the next step.
			evalTask.Run(pose, poseMatrixCache, skeleton);
		}

		// Fetch previously calculated pose matrices from main memory.
		poseMatrixCache->SpuFetch();
				
		// Update the animated model instance.
		AnimatedModel::C_AnimatedModelInstance* animatedModelInstance = (AnimatedModel::C_AnimatedModelInstance*) Spu::FetchAlloc(task->animatedModelInstance, sizeof(C_AnimatedModelInstance));
		animatedModelInstance->Update(task->worldMatrix, *pose, *poseMatrixCache);

	#ifdef PLATFORM_SPU
		animatedModelInstance->SpuStore(task->animatedModelInstance);
	#endif
	}
}

#ifndef PLATFORM_SPU

bool C_UpdateTask::Schedule(S32 group, S32 policy)
{
	return Schedule(this, 1, group, policy);
}

bool C_UpdateTask::Schedule(const C_UpdateTask* tasks, U32 numTasks, S32 group, S32 policy)
{
	if (numTasks == 0)
	{
		return false;
	}

	bool scheduled = true;
	if (Scheduler::Active())
	{
		C_UpdateTaskParams params;
		params.tasks = tasks;
		params.numTasks = numTasks;

		Scheduler::AddTask(
			AnimatedModelUpdateTaskModule,
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
		Run(tasks, numTasks);
		scheduled = false;
	}
	return scheduled;
}

void C_UpdateTask::PostUpdate() const
{
	if (IsValid())
	{
		animatedModelInstance->PostUpdate(*pose);
		ReleaseLocks();
	}
}

void C_UpdateTask::ReleaseLocks() const
{
	Animation::C_Pose::ReleaseReader(pose);
	AnimatedModel::C_AnimatedModelInstance::Unlock(animatedModelInstance);
}

#endif


} // end AnimatedModel

