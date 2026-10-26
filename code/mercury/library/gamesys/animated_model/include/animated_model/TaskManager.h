#ifndef ANIMATEDMODEL_TASKMANAGER_H
#define ANIMATEDMODEL_TASKMANAGER_H

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
	class C_PoseMatrixCache;
	class C_CommandList;
	class C_EvaluationState;
}

namespace AnimatedModel
{

class C_AnimatedModelInstance;

/**
Ordered synchronisation points for animated model tasks.
*/
enum E_Sync
{
	E_Sync_PreRender = 0,					///< Task will be completed just before rendering begins.
	E_Sync_Count,						
	E_Sync_Immediate	= E_Sync_Count,	///< Runs the task immediately (i.e. synchronously).
};

/**
Task manager interface for running asynchronous animated model update tasks.
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
	
	void Create(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy);
	void Destroy();
	
	/// Returns true if task manager is active i.e. it has been created and capable of
	/// scheduling asynchronous tasks. If this returns false, then all calls to ScheduleXXX
	/// will result in immediate single-threaded execution of tasks on the current thread.
	bool IsActive() const;
	
	static C_TaskManager& Instance();

	/// Schedules an update of an animated model from a pose and its matrix cache.
	/// @warning The caller must avoid modifying any of the objects passed to this function
	/// until the specified sync point has been passed. If there is a chance of any
	/// of the objects being modified or destroyed, then E_Sync_Immediate should be 
	/// used to force the task to complete before the function returns.
	void ScheduleUpdateTask(
		E_Sync syncPoint,
		C_AnimatedModelInstance& animatedModelInstance,
		const Maths::HMat43& worldMatrix,
		//const Animation::C_EvaluationState& evaluationState,
		const Animation::C_Pose& pose,
		const Animation::C_PoseMatrixCache& poseMatrixCache,
		bool forceSchedule = false
	);
	
	void ScheduleUpdateTask(
		E_Sync syncPoint,
		C_AnimatedModelInstance& animatedModelInstance,
		const Maths::HMat43& worldMatrix,
		const Animation::C_EvaluationState& evaluationState,
		bool forceSchedule = false
	);	

	/// Schedules an animation evaluation followed by an update of the animated model.
	/// @warning The caller must avoid modifying any of the objects passed to this function
	/// until the specified sync point has been passed. If there is a chance of any
	/// of the objects being modified or destroyed, then E_Sync_Immediate should be 
	/// used to force the task to complete before the function returns.
	void ScheduleAnimateAndUpdateTask(
		E_Sync syncPoint,
		C_AnimatedModelInstance& animatedModelInstance,
		const Maths::HMat43& worldMatrix,
		Animation::C_Pose& pose,
		Animation::C_PoseMatrixCache& poseMatrixCache,
		Animation::C_CommandList& commandList,
		const Animation::C_Skeleton* skeleton,
		bool forceSchedule = false
	);
	
	
	/// Schedules any tasks that have been queued but not scheduled for execution.
	/// Call this some time before Sync() to avoid blocking during Sync().
	void ScheduleQueuedTasks();
	
	/// Ensures that all pending tasks have been completed.
	void Flush();
	
	/// Ensures that deleted C_AnimatedModelInstances never get PostUpdated.
	void OnAnimatedModelInstanceDestroyed(const C_AnimatedModelInstance* pInstance);
	
private:
	struct Impl;
	Impl* m_impl;
};

/// Creates the task manager. Should only be called once.
bool CreateTaskManager(U32 maxAsyncTasksPerFrame, S32 schedulerPolicy);

/// Destroys the task manager.
void DestroyTaskManager();

/// Gets the animation task manager instance (always valid).
C_TaskManager& TaskManager();

} // end Animation

#endif
