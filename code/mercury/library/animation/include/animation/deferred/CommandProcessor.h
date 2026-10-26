#ifndef ANIMATION_COMMANDPROCESSOR_H
#define ANIMATION_COMMANDPROCESSOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/BlendModes.h>

namespace Animation
{

	class C_Pose;
	class C_PoseMatrixCache;
	class C_CommandList;
	class C_Skeleton;
	class C_Command;
	class C_PoseStack;
	class C_BlendCycleCommand;
	class C_PushPoseCommand;
	class C_PopPoseCommand;
	class C_MirrorPoseCommand;
	class C_SetIkTargetWeightsCommand;
	class C_SetIkTargetFromJointCommand;
	class C_EvaluateIkChainCommand;
	class C_JointTransform;
	class C_CopyOutJointTransformsCommand;
	class C_PoseMapping;
		
	class C_CommandProcessor
	{
	public:
		C_CommandProcessor();
		~C_CommandProcessor();
		
		/// Executes the commands in command list, storing the result to @a destPose. If there is an overflow
		/// in the pose stack or any other error, then false is returned.
		bool ExecuteCommandList( const C_CommandList& commandList, C_Pose& destPose, const C_Skeleton* skeleton, C_PoseMatrixCache* destPoseMatrixCache );
	
	private:
		void BlendCycle(const C_BlendCycleCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton, U32 updateCounter, bool cycleInstanceInMainMemory, bool relativeToRoot);
		bool PushPose(const C_PushPoseCommand& cmd, C_PoseStack& poseStack);
		void PopPose(const C_PopPoseCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton);
		void MirrorPose(const C_MirrorPoseCommand& cmd, C_PoseStack& poseStack);
		void CopyInPose(C_Pose& destPose, const C_Skeleton* destSkeleton, const C_Pose* sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping* poseMapping, bool ignoreScale, bool normalised);
		void SetIkTargetWeights(const C_SetIkTargetWeightsCommand& cmd);
		void SetIkTargetFromJoint(const C_SetIkTargetFromJointCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton);
		void EvaluateIkChain(const C_EvaluateIkChainCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton, bool topPoseIsNormalised, const C_JointTransform& modelWorldTransform);
		void TransformJoint(
			S32 jointIndex,
			const C_JointTransform& transform,
			C_PoseStack& poseStack,
			const C_Skeleton* skeleton,
			bool topPoseIsNormalised
		);
		void TransformJointTranslation(
			S32 jointIndex,
			const C_JointTransform& transform,
			C_PoseStack& poseStack,
			const C_Skeleton* skeleton,
			bool topPoseIsNormalised
		);
		void CopyOutJointTransforms(
			const C_CopyOutJointTransformsCommand& cmd,
			C_PoseStack& poseStack,
			const C_Skeleton* skeleton,
			bool topPoseIsNormalised
		);	
	
		void ApplyRagdoll(const C_CommandList& commandList, C_Pose& pose, const C_Skeleton* skeleton) const;
		void ApplyConstraints(const C_CommandList& commandList, C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& worldTransform) const;
	};
}

#endif
