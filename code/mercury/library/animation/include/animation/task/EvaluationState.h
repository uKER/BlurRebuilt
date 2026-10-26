#ifndef ANIMATION_EVALUATIONSTATE_H
#define ANIMATION_EVALUATIONSTATE_H

#include <animation/deferred/CommandList.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/skeleton/SkeletonInstance.h>

namespace Animation
{

	class C_Skeleton;

	/**
	Encapsulates the main structures required for evaluating an animated model.
	
	Also handles multi-threaded update.
	*/
	class C_EvaluationState
	{
	private:	
		// Intentionally private, use New() instead.
		C_EvaluationState(U32 maxCommands);
		
	public:
		~C_EvaluationState();

		/// Constructs a new C_EvaluationState object.
		static C_EvaluationState* New(U32 maxCommands);
		
	public:
		
		/// Creates the Pose, PoseMatrixCache and, if skeleton is not NULL, the SkeletonInstance.
		void Create(const C_PoseFormat& poseFormat, const C_Skeleton* skeleton = NULL);
		
		C_CommandList& GetCommandList();
		const C_CommandList& GetCommandList() const;
		
		C_Pose& GetPose();
		const C_Pose& GetPose() const;
		const C_Pose& GetPoseReadOnly() const;
		const C_Pose& GetPoseNonBlocking() const;
		
		C_PoseMatrixCache& GetPoseMatrixCache();
		const C_PoseMatrixCache& GetPoseMatrixCache() const;
		const C_PoseMatrixCache& GetPoseMatrixCacheReadOnly() const;
		const C_PoseMatrixCache& GetPoseMatrixCacheNonBlocking() const;
		
		C_SkeletonInstance& GetSkeletonInstance();
		const C_SkeletonInstance& GetSkeletonInstance() const;
		
		const C_Skeleton* GetSkeleton() const;
		
		void ScheduleEvaluationTask(bool updatePoseMatrixCache = true);
		
		/// Ensures that any pending multi-threaded processing that uses this C_EvaluationState has completed.
		void Flush() const;
		
	private:
		void FlushIfNotReadable() const;
		void FlushIfNotWriteable() const;
		bool IsPoseModifiable() const;
	
		C_CommandList				m_commandList;
		C_Pose						m_pose;
		C_PoseMatrixCache			m_poseMatrixCache;
		C_SkeletonInstance			m_skeletonInstance;
	};
	
}

#endif
