#include "AnimatedModelPch.h"
#include <animated_model/TaskManager.h>
#include <animated_model/ManagedAnimatedModel.h>
#include <animated_model/UpdateTask.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animation/task/EvaluationState.h>
#include <animation/task/TaskList.h>
#include <animation/task/TaskManager.h>
#include <animation/pose/Pose.h>
#include <btl/containers/vector.h>
#include <system/Scheduler.h>
#include <system/debug/log/DebugLog.h>

namespace AnimatedModel
{

DECLARE_DEBUG_BOOL(gEnableThreading, "GameSys.AnimatedModel.EnableThreading", true);
DECLARE_DEBUG_INT(gTaskBatchSize, "GameSys.AnimatedModel.TaskBatchSize", 4, Debug::Range(1, 32));
DECLARE_DEBUG_BOOL(gSkipNonAnimatedTasks, "GameSys.AnimatedModel.SkipNonAnimatedTasks", true);
DECLARE_DEBUG_INT(gTaskIdealBatchCost, "GameSys.AnimatedModel.TaskIdealBatchCost", 140, Debug::Range(1, 200));

Debug::Logger& gLog = Debug::GetLogger("mercury.gamesys.animated_model");

class C_AnimatedModelUpdateTaskList: public Animation::C_TaskList<C_UpdateTask>
{
public:
	virtual void ScheduleDependenciesImpl()
	{
		// Ensure that queued animation evaluation tasks are handed over to the scheduler to guarantee
		// that model update tasks occur after any pending animation evaluation tasks.
		Animation::TaskManager().ScheduleQueuedTasks();
	}

	virtual void FlushDependenciesImpl()
	{
		// Ensure that queued animation evaluation tasks are completed.
		Animation::TaskManager().Flush();
	}
};

struct C_TaskManager::Impl
{
	Scheduler::GroupHandle			m_groups[E_Sync_Count];
	C_AnimatedModelUpdateTaskList	m_taskLists[E_Sync_Count];
	
	static void FlushCallback(void* data)
	{
		TaskManager().Flush();
	}
	
	Impl(U32 maxTasks, S32 policy)
	{
		#ifndef CFG_GOLD
		static const char* kTaskListNames[E_Sync_Count] =
		{
			"AnimModelPreRender"
		};
		#endif
		
		BIZ_ASSERT_MSG(Animation::TaskManager().IsActive(), "AnimatedModel task manager needs Animation evaluation task manager to be initialised first.");
		
		S32 animationEvaluateTaskGroupHandle = Animation::TaskManager().GetTaskGroupHandle(Animation::E_TaskGroup_Evaluate);
	
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			m_groups[i] = Scheduler::CreateGroup();
			BIZ_ASSERT_MSG(m_groups[i] != -1, "Failed to create animated model update task group");
			m_taskLists[i].Resize(maxTasks);
			m_taskLists[i].SetSchedulerGroupAndPolicy(m_groups[i], policy);
			#ifndef CFG_GOLD
			m_taskLists[i].SetName(kTaskListNames[i]);
			#endif
			
			// Add a dependency on the animation evaluate task.
			if (animationEvaluateTaskGroupHandle >= 0)
			{
				Scheduler::AddGroupDependency(m_groups[i], animationEvaluateTaskGroupHandle);
				
				// Inform animation evaluation task manager of this dependency.
				Animation::TaskManager().AddDependent(Animation::E_TaskGroup_Evaluate, &FlushCallback, (void*)i);
			}
		}
	}
	
	~Impl()
	{
		FlushAll();
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			Scheduler::DestroyGroup(m_groups[i]);
			Animation::TaskManager().RemoveDependent(Animation::E_TaskGroup_Evaluate, &FlushCallback, (void*)i);
		}
	}
	
	void ScheduleQueuedTasks()
	{
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			m_taskLists[i].ScheduleQueuedTasks();
		}	
	}
	
	void FlushAll()
	{
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			m_taskLists[i].ScheduleQueuedTasks();
		}
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			m_taskLists[i].WaitForCurrentTasksToComplete();

			for(U32 j=0, count=m_taskLists[i].GetTaskCount(); j<count; ++j)
			{
				C_UpdateTask& task = m_taskLists[i].GetTaskAtIndex(j);
				task.PostUpdate();
			}
			
			m_taskLists[i].Flush();
		}
	}
	
	void AddTask(E_Sync syncPoint, const C_UpdateTask& task, bool forceSchedule)
	{
		BIZ_ASSERT(task.pose != NULL);
	
		bool enableThreading = gEnableThreading && Scheduler::Active();
		
		if (!enableThreading || m_groups[syncPoint] < 0)
		{
			syncPoint = E_Sync_Immediate;
		}
				
		if (syncPoint != E_Sync_Immediate)
		{
			m_taskLists[syncPoint].SetIdealBatchCost(gTaskIdealBatchCost);
			S32 cost = task.animatedModelInstance->GetNumAnimatedParameters() + 1;
			m_taskLists[syncPoint].AddTask(task, cost);
			if (forceSchedule)
			{
				m_taskLists[syncPoint].ScheduleQueuedTasks();
			}
		}
		else
		{
			task.Run();
		}
	}
	
	void OnAnimatedModelInstanceDestroyed(const C_AnimatedModelInstance* pInstance)
	{
		for (U32 i = 0; i < E_Sync_Count; ++i)
		{
			for(U32 j = 0, count = m_taskLists[i].GetTaskCount(); j<count; ++j)
			{
				C_UpdateTask& task = m_taskLists[i].GetTaskAtIndex(j);
				
				if (task.animatedModelInstance == pInstance)
				{
					task.Invalidate();
				}
			}	
		}
	}
	
};

//
// C_TaskManager
//


C_TaskManager::C_TaskManager():
	m_impl(NULL)
{
}

C_TaskManager::~C_TaskManager()
{
	Destroy();
}

void C_TaskManager::Create(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy)
{
	Destroy();
	m_impl = new Impl(maxAsyncTasksPerFrame, schedulerPolicy);
}

void C_TaskManager::Destroy()
{
	delete m_impl;
	m_impl = NULL;
}

bool C_TaskManager::IsActive() const
{
	return m_impl != NULL;
}

C_TaskManager& C_TaskManager::Instance()
{
	static C_TaskManager inst;
	return inst;
}

void C_TaskManager::ScheduleUpdateTask(
	E_Sync syncPoint,
	C_AnimatedModelInstance& animatedModelInstance,
	const Maths::HMat43& worldMatrix,
	const Animation::C_Pose& pose,
	const Animation::C_PoseMatrixCache& poseMatrixCache,
	bool forceSchedule
)
{
	bool inSceneGraph = animatedModelInstance.PreUpdate();
	if (!inSceneGraph)
	{
		// Nothing to do.
		return;
	}

	C_UpdateTask task;
	task.worldMatrix = worldMatrix;
	task.pose = const_cast<Animation::C_Pose*>(&pose);
	task.poseMatrixCache = const_cast<Animation::C_PoseMatrixCache*>(&poseMatrixCache);
	task.skeleton = NULL;
	task.commandList = NULL;
	task.animatedModelInstance = &animatedModelInstance;

	AnimatedModel::C_AnimatedModelInstance::Lock(task.animatedModelInstance);
	Animation::C_Pose::AcquireReader(task.pose);
		
	// Skip creation of tasks for non-animated models since their update time is trivial.
	bool createTask = !gSkipNonAnimatedTasks || animatedModelInstance.IsAnimated();
		
	if (m_impl != NULL && createTask)
	{
		m_impl->AddTask(syncPoint, task, forceSchedule);
	}
	else
	{
		task.Run();
	}
}

void C_TaskManager::ScheduleUpdateTask(
	E_Sync syncPoint,
	C_AnimatedModelInstance& animatedModelInstance,
	const Maths::HMat43& worldMatrix,
	const Animation::C_EvaluationState& evaluationState,
	bool forceSchedule
)
{
	bool inSceneGraph = animatedModelInstance.PreUpdate();
	if (!inSceneGraph)
	{
		// Nothing to do.
		return;
	}
	
	C_UpdateTask task;
	task.worldMatrix = worldMatrix;
	task.pose = const_cast<Animation::C_Pose*>(&evaluationState.GetPoseNonBlocking());
	task.poseMatrixCache = const_cast<Animation::C_PoseMatrixCache*>(&evaluationState.GetPoseMatrixCacheNonBlocking());
	task.skeleton = NULL;
	task.commandList = NULL;
	task.animatedModelInstance = &animatedModelInstance;
	
	AnimatedModel::C_AnimatedModelInstance::Lock(task.animatedModelInstance);
	Animation::C_Pose::AcquireReader(task.pose);
		
	// Skip creation of tasks for non-animated models since their update time is trivial.
	bool createTask = !gSkipNonAnimatedTasks || animatedModelInstance.IsAnimated();
	
	if (m_impl != NULL && createTask)
	{
		m_impl->AddTask(syncPoint, task, forceSchedule);
	}
	else
	{
		task.Run();
	}
}

void C_TaskManager::ScheduleAnimateAndUpdateTask(
	E_Sync syncPoint,
	C_AnimatedModelInstance& animatedModelInstance,
	const Maths::HMat43& worldMatrix,
	Animation::C_Pose& pose,
	Animation::C_PoseMatrixCache& poseMatrixCache,
	Animation::C_CommandList& commandList,
	const Animation::C_Skeleton* skeleton,
	bool forceSchedule
)
{
	bool inSceneGraph = animatedModelInstance.PreUpdate();
	if (!inSceneGraph)
	{
		// Nothing to do.
		return;
	}
	
	C_UpdateTask task;
	task.worldMatrix = worldMatrix;
	task.pose = &pose;
	task.poseMatrixCache = &poseMatrixCache;
	task.skeleton = skeleton;
	task.commandList = &commandList;
	task.animatedModelInstance = &animatedModelInstance;
	
	AnimatedModel::C_AnimatedModelInstance::Lock(task.animatedModelInstance);
	Animation::C_Pose::AcquireReader(task.pose);
	
	if (task.commandList != NULL)
	{
		Animation::C_Pose::AcquireWriter(task.pose);
	}
	
	if (m_impl != NULL)
	{
		m_impl->AddTask(syncPoint, task, forceSchedule);
	}
	else
	{
		task.Run();
	}
}

void C_TaskManager::ScheduleQueuedTasks()
{
	if (m_impl != NULL)
	{
		m_impl->ScheduleQueuedTasks();
	}
}

void C_TaskManager::Flush()
{
	if (m_impl != NULL)
	{
		m_impl->FlushAll();
	}
}

void C_TaskManager::OnAnimatedModelInstanceDestroyed(const C_AnimatedModelInstance* pInstance)
{
	if (m_impl != NULL)
	{
		m_impl->OnAnimatedModelInstanceDestroyed(pInstance);
	}
}

/// Creates the task manager. Should only be called once.
bool CreateTaskManager(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy)
{
	C_TaskManager::Instance().Create(maxAsyncTasksPerFrame, schedulerPolicy);
	return true;
}

/// Destroys the task manager.
void DestroyTaskManager()
{
	C_TaskManager::Instance().Destroy();
}

C_TaskManager& TaskManager()
{
	return C_TaskManager::Instance();
}
	
} // end AnimatedModel
