#ifndef ANIMATION_TASKLIST_H
#define ANIMATION_TASKLIST_H

#include <system/debug/log/DebugLog.h>
#include <system/Scheduler.h>
#include <btl/containers/vector.h>

namespace Animation
{

	template <typename T>
	class C_TaskList
	{
	public:
		C_TaskList(U32 taskQueueSize = 16):
			m_name(NULL),
			m_batchStartIndex(0),
			m_group(-1),
			m_policy(-1),
			m_idealCostPerBatch(0),
			m_currentBatchCost(0),
			m_maxBatchSize(32),
			m_numTasks(0),
			m_numBatches(0),
			m_pendingTasks(false)
		{
			m_tasks.Reserve(taskQueueSize);
		}
		
		virtual ~C_TaskList()
		{
			BIZ_ASSERT(!m_pendingTasks);
			Flush();
		}
		
		const T& GetTaskAtIndex(U32 iIndex) const
		{
			BIZ_ASSERT(iIndex >= 0 && iIndex < m_numTasks);
			return m_tasks[iIndex];
		}

		T& GetTaskAtIndex(U32 iIndex)
		{
			BIZ_ASSERT(iIndex >= 0 && iIndex < m_numTasks);
			return m_tasks[iIndex];
		}


		const U32 GetTaskCount() const
		{
			return static_cast<U32>(m_tasks.Size());
		}

		const U32 GetNumBatches() const
		{
			return m_numBatches;
		}

		void ClearNumBatches()
		{
			m_numBatches = 0;
		}

		void Resize(U32 newSize)
		{
			Flush();
			m_tasks.Reserve(newSize);			
		}
		
		// @a name must be a static string.
		void SetName(const CHAR8* name)
		{
			m_name = name;
		}
			
		void SetSchedulerGroupAndPolicy(S32 group, S32 policy)
		{
			m_group = group;
			m_policy = policy;
		}
		
		S32 GetSchedulerGroup() const
		{
			return m_group;
		}
			
		void AddTask(const T& task, S32 cost)
		{
			++m_numTasks;
			if (m_tasks.Size() < m_tasks.Capacity())
			{
				m_tasks.PushBack(task);
				m_currentBatchCost += cost;
				// Need some heuristic for determining number of tasks to batch up.
				// For the time being just send off every 4 tasks or so.
				if (m_currentBatchCost >= m_idealCostPerBatch || GetBatchSize() > m_maxBatchSize)
				{
					ScheduleQueuedTasks();
				}
			}
			else
			{
				// Ran out of space in task queue - 
				// Ensure that any dependencies are satisfied.
				FlushDependenciesImpl();					

				// Ran out of space in task queue, so run task immediately.
				task.Run();
			}
		}
			
		void ScheduleQueuedTasks()
		{
			if (GetBatchSize() == 0)
				return;
			
			// Make sure tasks we might depend on are scheduled first.
			ScheduleDependenciesImpl();
			
			bool scheduled = T::Schedule(&m_tasks[m_batchStartIndex], GetBatchSize(), m_group, m_policy);
			
			if (scheduled)
				m_pendingTasks = true;
			
			m_batchStartIndex = m_tasks.Size();
			m_currentBatchCost = 0;

			m_numBatches += scheduled ? 1 : 0;

			return;
		}
		
		/// Ensures any dependencies have been scheduled.
		virtual void ScheduleDependenciesImpl()
		{
		}

		/// Ensures any dependencies have been completed.
		virtual void FlushDependenciesImpl()
		{
		}
		
		/// Schedules all queued tasks and waits for them to complete.
		void WaitForCurrentTasksToComplete()
		{
			ScheduleQueuedTasks();
			if (m_pendingTasks)
			{
				Scheduler::Flush(m_group);
				m_pendingTasks = false;
			}
		}
			
		/// Waits for all pending tasks to complete and then clears the task queue.
		void Flush()
		{
			WaitForCurrentTasksToComplete();
			
			m_tasks.Clear();
			if (m_numTasks > GetCapacity())
			{
				LOGDEBUG(Debug::GetLogger("mercury.animation"), "Ran out of space in %s task queue (%d requested, %d max)", m_name != NULL ? m_name : "unnamed", m_numTasks, GetCapacity());
			}
			m_numTasks = 0;
			m_batchStartIndex = 0;
			m_currentBatchCost = 0;
		}

		U32 GetCapacity() const
		{
			return m_tasks.Capacity();
		}

		void SetIdealBatchCost(S32 cost)
		{
			m_idealCostPerBatch = cost;
		}
		
		void SetMaxBatchSize(U32 maxBatchSize)
		{
			m_maxBatchSize = maxBatchSize;
		}
		
	private:	
		U32 GetBatchSize() const
		{
			return Maths::Max(m_tasks.Size() - static_cast<S32>(m_batchStartIndex), static_cast<S32>(0));
		}
		

		const CHAR8*	m_name;
		Btl::Vector<T>	m_tasks;
		S32				m_group;
		S32				m_policy;
		U32				m_batchStartIndex;
		S32				m_idealCostPerBatch;
		S32				m_currentBatchCost;
		U32				m_maxBatchSize;
		U32				m_numTasks;			// Total tasks in this frame.
		U32				m_numBatches;
		bool			m_pendingTasks;
	};
}

#endif
