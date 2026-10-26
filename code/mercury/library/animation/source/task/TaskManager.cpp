#include "stdafx.h"
#include <animation/task/TaskManager.h>
#include <animation/task/EvaluationTask.h>
#include <animation/task/TaskList.h>
#include <animation/common/Stats.h>
#include <animation/pose/Pose.h>
#include <system/Scheduler.h>

namespace Animation
{

DECLARE_DEBUG_BOOL(gEnableThreading, "Animation.EnableThreading", true);
//Debug::Int gTaskBatchSize("Animation.TaskBatchSize", 4, Debug::Range(1, 32));
DECLARE_DEBUG_INT(gTaskIdealBatchCost, "Animation.IdealBatchCost", 500, Debug::Range(1, 500));

struct C_TaskManager::C_Stats
{
	C_Stats();
		
	U32 numImmediateTasks;
	U32 numThreadedTasks;
	U32 numFlushes;
	U32 numBatches;
};

struct C_TaskManager::Impl
{
	Scheduler::GroupHandle					m_groups[E_TaskGroup_Count];
	Animation::C_TaskList<C_EvaluationTask>	m_taskLists[E_TaskGroup_Count];
	C_TaskManager::C_Stats					m_stats;
		
	class C_Dependent
	{
	public:
		C_Dependent():
			flush(NULL),
			data(NULL)
		{
		}
		
		C_Dependent(C_TaskManager::FlushCallback callback, void* inData):
			flush(callback),
			data(inData)
		{
		}
		
		bool operator == (const C_Dependent& rhs) const
		{
			return flush == rhs.flush && data == rhs.data;
		}
		
		C_TaskManager::FlushCallback	flush;
		void*							data;
	};
	
	Btl::Vector<C_Dependent>				m_dependents[E_TaskGroup_Count];
	
	Impl(U32 maxTasks, S32 policy)
	{
		#ifndef CFG_GOLD
		static const CHAR8* kNames[E_TaskGroup_Count] =
		{
			"AnimEvaluate"
		};
		#endif
	
		for (U32 i = 0; i < E_TaskGroup_Count; ++i)
		{
			m_groups[i] = Scheduler::CreateGroup();
			BIZ_ASSERT_MSG(m_groups[i] != -1, "Failed to create animation evaluation task group");
			m_taskLists[i].Resize(maxTasks);
			m_taskLists[i].SetSchedulerGroupAndPolicy(m_groups[i], policy);
			#ifndef CFG_GOLD
			m_taskLists[i].SetName(kNames[i]);
			#endif
		}
	}
	
	~Impl()
	{
		for (U32 i = 0; i < E_TaskGroup_Count; ++i)
		{
			// Flush this group and tasks that depend on this group
			FlushDependents((E_TaskGroup)i);
			Scheduler::DestroyGroup(m_groups[i]);
		}
	}
	
	void ScheduleQueuedTasks()
	{
		for (U32 i = 0; i < E_TaskGroup_Count; ++i)
		{
			m_taskLists[i].ScheduleQueuedTasks();
		}
	}
	
	void Flush(E_TaskGroup taskGroup)
	{
		if (taskGroup < E_TaskGroup_Count)
		{
			m_stats.numFlushes++;
			m_taskLists[taskGroup].Flush();
		}
	}
	
	void FlushAll()
	{
		++m_stats.numFlushes;
		for (U32 i = 0; i < E_TaskGroup_Count; ++i)
		{
			m_taskLists[i].Flush();
			m_stats.numBatches += m_taskLists[i].GetNumBatches();
			m_taskLists[i].ClearNumBatches();
		}

		ANIMATION_STATS_UPDATE(numFlushes = m_stats.numFlushes);
		ANIMATION_STATS_UPDATE(numImmediateTasks = m_stats.numImmediateTasks);
		ANIMATION_STATS_UPDATE(numThreadedTasks= m_stats.numThreadedTasks);

		ANIMATION_STATS_UPDATE(maxThreadedTasks = m_taskLists[0].GetCapacity());
		ANIMATION_STATS_UPDATE(numBatches = m_stats.numBatches);
		ANIMATION_STATS_UPDATE(memoryUsedByTaskManager = m_taskLists[0].GetCapacity() * sizeof(C_EvaluationTask));
	}

	void ClearStats()
	{
		m_stats = C_Stats();
	}
	
	void AddDependent(E_TaskGroup taskGroup, C_TaskManager::FlushCallback callback, void* data)
	{
		if (taskGroup < E_TaskGroup_Count)
		{
			m_dependents[taskGroup].PushBack(C_Dependent(callback, data));
		}
	}
	
	void RemoveDependent(E_TaskGroup taskGroup, C_TaskManager::FlushCallback callback, void* data)
	{
		if (taskGroup < E_TaskGroup_Count)
		{
			Btl::RemoveNonOrdered(m_dependents[taskGroup], C_Dependent(callback, data));
		}
	}
	
	void FlushDependents(E_TaskGroup taskGroup)
	{
		if (taskGroup < E_TaskGroup_Count)
		{
			++m_stats.numFlushes;
			m_taskLists[taskGroup].Flush();
			for (U32 i = 0, count = m_dependents[taskGroup].Size(); i < count; ++i)
			{
				(*m_dependents[taskGroup][i].flush)(m_dependents[taskGroup][i].data);
			}
		}
	}

	void AddTask(E_TaskGroup taskGroup, const C_EvaluationTask& task, bool forceSchedule )
	{
		bool enableThreading = gEnableThreading && Scheduler::Active();
		
		if (!enableThreading || m_groups[taskGroup] < 0)
		{
			taskGroup = E_TaskGroup_Immediate;
		}
		
		if (taskGroup != E_TaskGroup_Immediate)
		{
			//m_taskLists[taskGroup].SetMaxBatchSize(gTaskBatchSize);
			m_taskLists[taskGroup].SetIdealBatchCost(gTaskIdealBatchCost);
			
			S32 cost = task.m_pose->GetNumJointTransforms() + 1;
			
			m_taskLists[taskGroup].AddTask(task, cost);
			if (forceSchedule)
			{
				m_taskLists[taskGroup].ScheduleQueuedTasks();
			}
			++m_stats.numThreadedTasks;
		}
		else
		{
			++m_stats.numImmediateTasks;
			task.Run();
		}
	}
	
	Scheduler::GroupHandle GetTaskGroupHandle(E_TaskGroup taskGroup)
	{
		if (taskGroup < E_TaskGroup_Count)
		{
			return m_taskLists[taskGroup].GetSchedulerGroup();
		}
		else
		{
			return -1;
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

bool C_TaskManager::IsActive() const
{
	return m_impl != NULL;
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

C_TaskManager& C_TaskManager::Instance()
{
	static C_TaskManager inst;
	return inst;
}

void C_TaskManager::ScheduleEvaluationTask(
			E_TaskGroup taskGroup,
			C_Pose& pose,
			C_CommandList& commandList,
			const C_Skeleton* skeleton,
			C_PoseMatrixCache* poseMatrixCache,
			bool forceSchedule
		)
{
	C_EvaluationTask task;
	task.m_skeleton = skeleton;
	task.m_pose = &pose;
	task.m_poseMatrixCache = poseMatrixCache;
	task.m_commandList = &commandList;

	Animation::C_Pose::AcquireWriter(task.m_pose);
	
	if (m_impl != NULL)
	{
		m_impl->AddTask(taskGroup, task, forceSchedule);
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

void C_TaskManager::Flush(E_TaskGroup taskGroup)
{
	if (m_impl != NULL)
	{
		m_impl->Flush(taskGroup);
	}
}

void C_TaskManager::Flush()
{
	if (m_impl != NULL)
	{
		m_impl->FlushAll();
	}
}

void C_TaskManager::ClearStats()
{
	if (m_impl != NULL)
	{
		m_impl->ClearStats();
	}
}

void C_TaskManager::AddDependent(E_TaskGroup taskGroup, FlushCallback callback, void* data)
{
	if (m_impl != NULL)
	{
		m_impl->AddDependent(taskGroup, callback, data);
	}
}

void C_TaskManager::RemoveDependent(E_TaskGroup taskGroup, FlushCallback callback, void* data)
{
	if (m_impl != NULL)
	{
		m_impl->RemoveDependent(taskGroup, callback, data);
	}
}

void C_TaskManager::FlushDependents(E_TaskGroup taskGroup)
{
	if (m_impl != NULL)
	{
		m_impl->FlushDependents(taskGroup);
	}
}

S32 C_TaskManager::GetTaskGroupHandle(E_TaskGroup taskGroup)
{
	S32 groupHandle = -1;
	if (m_impl != NULL)
	{
		groupHandle = m_impl->GetTaskGroupHandle(taskGroup);
	}
	return groupHandle;
}

C_TaskManager::C_Stats::C_Stats():
	numImmediateTasks(0),
	numThreadedTasks(0),
	numFlushes(0),
	numBatches(0)
{
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
