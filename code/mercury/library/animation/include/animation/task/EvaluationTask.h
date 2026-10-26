#ifndef ANIMATION_EVALUATIONTASK_H
#define ANIMATION_EVALUATIONTASK_H

namespace Animation
{
	class C_Skeleton;
	class C_Pose;
	class C_CycleInstance;
	class C_CommandList;
	class C_SkeletonInstance;
	class C_PoseMatrixCache;

	/**
	An evaluation task that executes an animation command list.
	*/
	class ALIGN16 C_EvaluationTask
	{
	public:
		C_EvaluationTask();
		
		bool IsValid() const		{ return m_pose != NULL; }
		
		/// Runs this task immediately on the current thread.
		bool Run() const;
		
		/// Runs this task immediately on the current thread with local memory pose, matrix cache and skeleton objects.
		bool Run(C_Pose* localPose, C_PoseMatrixCache* localPoseMatrixCache, const C_Skeleton* localSkeleton) const;
		
		/// Runs a series of evaluation tasks immediately on the current thread.
		/// Returns true if all tasks executed successfully without pose stack overflows or other errors.
		/// On PS3 SPU it is assumed that the array of tasks is in local memory and all data pointed to
		/// in the C_EvaluationTask structures is in main memory.
		static bool RunBatch(const C_EvaluationTask* tasks, U32 numTasks);
		
		/// Schedules this task to run asynchronously on any available thread/SPU.
		/// Returns true if task was scheduled and can be waited on using Scheduler::Flush().
		bool Schedule(S32 group, S32 policy);
		
		/// Schedules a series of tasks for execution on any available thread/SPU.
		/// Returns true if tasks were scheduled and can be waited on using Scheduler::Flush().
		static bool Schedule(const C_EvaluationTask* tasks, U32 numTasks, S32 group, S32 policy);
				
		const C_Skeleton*	m_skeleton;
		C_Pose*				m_pose;
		C_PoseMatrixCache*	m_poseMatrixCache;
		C_CommandList*		m_commandList;
	};
	
	

} // end Animation

#endif
