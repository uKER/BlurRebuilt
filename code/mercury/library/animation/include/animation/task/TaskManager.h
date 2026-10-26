#ifndef ANIMATION_TASKMANAGER_H
#define ANIMATION_TASKMANAGER_H

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
	class C_PoseMatrixCache;
	class C_CommandList;
	
	

	/**
	Animation evaluation task groups.
	*/
	enum E_TaskGroup
	{
		E_TaskGroup_Evaluate = 0,					///< General evaluation task group.
		E_TaskGroup_Count,
		E_TaskGroup_Immediate = E_TaskGroup_Count	///< Runs the task immediately (i.e. synchronously).
	};

	/**
	Task manager interface for running asynchronous animation update tasks.
	Tasks are grouped into batches to reduce overhead. Incomplete batches are
	scheduled during calls to Sync() which may cause Sync() to block. To avoid
	this, call ScheduleQueuedTasks() ahead of calls to Sync().
	If the task manager has not been created via a call to Create(), then all
	schedule methods will be executed synchronously.
	*/
	class C_TaskManager
	{
	private:
		C_TaskManager();
		~C_TaskManager();
	public:
	
		/// Function called when flushing tasks that depend on evaluation tasks.
		typedef void (*FlushCallback)(void* data);
			
		void Create(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy);
		void Destroy();
		
		static C_TaskManager& Instance();
		
		bool IsActive() const;

		/// Schedules an evaluation of a command list into a pose and caching of matrices
		/// into a pose matrix cache.
		/// @warning The caller must avoid modifying any of the objects passed to this function
		/// until the specified sync point has been passed. If there is a chance of any
		/// of the objects being modified or destroyed, then E_Sync_Immediate should be 
		/// used to force the task to complete before the function returns.
		/// If @a forceSchedule is true, then the task will be scheduled immediately rather
		/// than waiting until there are enough queued tasks to make up a batch. 
		void ScheduleEvaluationTask(
			E_TaskGroup group,
			C_Pose& pose,
			C_CommandList& commandList,
			const C_Skeleton* skeleton,
			C_PoseMatrixCache* poseMatrixCache,
			bool forceSchedule = false
		);
		
		/// Schedules any tasks that have been queued but not scheduled for execution.
		/// Call this some time before Sync() to avoid blocking during Sync().
		void ScheduleQueuedTasks();
		
		/// Ensures that all tasks in the specified task group has finished, blocking if necessary.
		void Flush(E_TaskGroup taskGroup);
		
		/// Ensures that all tasks are finished.
		void Flush();

		void ClearStats();

		/// Adds a scheduler group that depends on a particular task group.
		void AddDependent(E_TaskGroup taskGroup, FlushCallback callback, void* data);
		
		/// Remove a previously added group dependency.
		void RemoveDependent(E_TaskGroup taskGroup, FlushCallback callback, void* data);
		
		/// Ensures that all task groups that depend on a task group have finished.
		void FlushDependents(E_TaskGroup taskGroup);
		
		/// Returns the scheduler group handle for a particular task group.
		S32 GetTaskGroupHandle(E_TaskGroup taskGroup);

	private:
		struct C_Stats;
		struct Impl;
		Impl* m_impl;
	};

	/// Creates the task manager. Should only be called once.
	bool CreateTaskManager(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy);

	/// Destroys the task manager.
	void DestroyTaskManager();

	/// Gets the animation task manager instance (always valid regardless of whether or not it has been created).
	C_TaskManager& TaskManager();

} // end Animation

#endif
