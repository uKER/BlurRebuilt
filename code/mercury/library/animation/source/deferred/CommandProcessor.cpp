#include "stdafx.h"
#include <animation/deferred/CommandProcessor.h>
#include <animation/deferred/CommandList.h>
#include <animation/ik/IkChain.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <animation/pose/PoseStack.h>
#include <animation/pose/PoseMapping.h>
#include <animation/pose/PoseMirror.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/WindParams.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/cycle/Cycle.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>

#include <system/Perf.h>

namespace Animation
{

//
// C_CommandProcessor
//

C_CommandProcessor::C_CommandProcessor()
{
}

C_CommandProcessor::~C_CommandProcessor()
{
}

inline bool IsAddOrLerp( E_BlendMode blendMode )
{
	return blendMode == E_BlendMode_Additive || blendMode == E_BlendMode_Lerp;
}

bool C_CommandProcessor::ExecuteCommandList(
	const C_CommandList& commandList,
	C_Pose& destPose,
	const C_Skeleton* skeleton,
	C_PoseMatrixCache* destPoseMatrixCache
)
{
	//PERF_EVENT( "Animation.ExecuteCommandList" );
	
	C_JointTransform modelWorldTransform(commandList.m_modelWorldMatrix);

	const U32 numCommands = commandList.GetNumCommands();
	
	destPose.Clear();
	
	bool noPoseStackOverflow = true;
	bool normalised = false;
	
	if ( numCommands > 0 )
	{
		Spu::MemBaseMarker blendingMarker;
	
		// Be careful as default job queue thread stack size is 32K.
		// Maybe need to pass in a scratch buffer?
		const U32 kMaxPoseStackDataSize = 26 * 1024;
		U32 poseStackDataSize = 0;
		U32 poseDataSize = destPose.GetPoseDataSize();
		if (poseDataSize <= kMaxPoseStackDataSize)
		{
			poseStackDataSize = (kMaxPoseStackDataSize / poseDataSize) * poseDataSize;
		}
		
		//U8 poseStackData[kPoseStackDataSize];
		U8* poseStackData = ANIM_POSE_STACK_SCRATCH_ALLOC(poseStackDataSize);
		C_PoseStack poseStack( destPose, poseStackData, poseStackDataSize );
		
		F32 currentPoseWeight = 1.f;
			
		for ( U32 i = 0; i < numCommands; ++i )
		{
			Spu::MemBaseMarker commandMemBaseMarker;
			
			const C_Command& cmd = commandList.GetCommand(i);
			
			switch ( cmd.type )
			{
				case E_CommandType_BlendCycle:
				{
					normalised = false;
					BlendCycle(cmd.blendCycle, poseStack, skeleton, commandList.GetUpdateCounter(), commandList.InMainMemory(), (commandList.m_flags & C_CommandList::E_Flag_RelativeToRoot) != 0);
					break;
				}
					
				case E_CommandType_PushCurrentPose:
				{
					normalised = false;
					bool noOverflow = PushPose(cmd.pushPose, poseStack);
					noPoseStackOverflow = noPoseStackOverflow && noOverflow;
					break;
				}
				
				case E_CommandType_PopAndBlendPose:
				{
					normalised = false;
					PopPose(cmd.popPose, poseStack, skeleton);
					break;
				}
				
				case E_CommandType_MirrorPose:
				{
					MirrorPose(cmd.mirrorPose, poseStack);
					break;
				}

				case E_CommandType_SetIkTargetWeights:
				{
					SetIkTargetWeights(cmd.setIkTargetWeights);
					break;
				}
				
				case E_CommandType_SetIkTargetFromJoint:
				{
					SetIkTargetFromJoint(cmd.setIkTargetFromJoint, poseStack, skeleton);
					break;
				}
								
				case E_CommandType_EvaluateIkChain:
				{
					EvaluateIkChain(cmd.evaluateIkChain, poseStack, skeleton, normalised, modelWorldTransform);
					normalised = true;
					break;
				}
				
				case E_CommandType_TransformJoint:
				{
					C_JointTransform transform;
					transform.SetRotation(Maths::MakeQuaternion(Maths::Load<Maths::Vector4>(cmd.transformJoint.rotation)));
					transform.SetTranslation(Maths::Load<Maths::Vector3>(cmd.transformJoint.translation));
					transform.SetScale(Maths::Load<Maths::Vector3>(cmd.transformJoint.scale));
					TransformJoint(cmd.transformJoint.jointIndex, transform, poseStack, skeleton, normalised);
					normalised = true;
					break;
				}

				case E_CommandType_TransformJointTranslation:
				{
					C_JointTransform transform;
					transform.SetRotation(Maths::MakeQuaternion(Maths::Load<Maths::Vector4>(cmd.transformJoint.rotation)));
					transform.SetTranslation(Maths::Load<Maths::Vector3>(cmd.transformJoint.translation));
					transform.SetScale(Maths::Load<Maths::Vector3>(cmd.transformJoint.scale));
					TransformJointTranslation(cmd.transformJoint.jointIndex, transform, poseStack, skeleton, normalised);
					normalised = true;
					break;
				}
				
				case E_CommandType_CopyOutJointTransforms:
				{
					CopyOutJointTransforms(cmd.copyOutJointTransforms, poseStack, skeleton, normalised);
					normalised = true;
					break;									
				}
				
				case E_CommandType_CopyInPose:
				{
					CopyInPose(poseStack.GetTop(), skeleton, cmd.copyInPose.sourcePose, cmd.copyInPose.sourceSkeleton, cmd.copyInPose.poseMapping, cmd.copyInPose.ignoreScale, normalised);
					normalised = true; // source data is assumed to be normalised
					break;
				}

				default:
					break;
			}
		}
	}

	if (!normalised)
	{
		destPose.Normalise( skeleton );
	}
	
	if ((commandList.m_flags & C_CommandList::E_Flag_RelativeToRoot) && destPose.GetNumJointTransforms() > 0)
	{
		destPose.GetJointTransform(0).SetIdentity();
	}
	
	if (commandList.m_flags & C_CommandList::E_Flag_ApplyRagdoll)
	{
		ApplyRagdoll(commandList, destPose, skeleton);
	}

	if (commandList.m_flags & C_CommandList::E_Flag_ApplyConstraints)
	{
		ApplyConstraints(commandList, destPose, skeleton, modelWorldTransform);
	}

	//
	// NB. at this point we could free up the local base pose transforms in the skeleton.
	//

	if (destPoseMatrixCache != NULL)
	{
		destPoseMatrixCache->Update(destPose, skeleton);
	}	
	
	return noPoseStackOverflow;
}

void C_CommandProcessor::BlendCycle(const C_BlendCycleCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton, U32 updateCounter, bool cycleInstanceInMainMemory, bool relativeToRoot)
{
	C_Pose& pose = poseStack.GetTop();
	const E_BlendMode blendMode = cmd.blendMode;

	if ( IsAddOrLerp(blendMode) && poseStack.GetLastBlendMode() == E_BlendMode_Accumulate )
	{
		// End of accumulation for this pose, so normalise it before this blend.
		// After normalisation, every single field has a weight of 1 and a sensible default value.
		pose.Normalise( skeleton );					
	}

	C_CycleInstance* cycleInstance = cmd.cycleInstance;
	
	if (cycleInstanceInMainMemory)
	{
		Spu::FetchAlloc(cycleInstance);
	}
	BIZ_ASSERT( cycleInstance->IsValid() );

	cycleInstance->SpuFetchCycleAndMapping();

	cycleInstance->m_cycle.RawPtr()->Evaluate( 
		blendMode,
		cmd.time,
		*cycleInstance,
		updateCounter,
		cmd.weight, // / poseStack.GetPoseWeight(),
		pose,
		cmd.loopMode,
		C_PoseFieldMask( cmd.fieldMask )
	);

#ifdef PLATFORM_SPU
	if (cycleInstanceInMainMemory)
	{
		Spu::Store(cmd.cycleInstance, cycleInstance, sizeof(C_CycleInstance));
	}
#endif

	if (relativeToRoot && pose.GetNumJointTransforms() > 0)
	{
		pose.GetJointTransform(0).SetIdentity();
	}
	
	poseStack.SetLastBlendMode( blendMode );
}

bool C_CommandProcessor::PushPose(const C_PushPoseCommand& cmd, C_PoseStack& poseStack)
{
	bool noOverflow = poseStack.Push();
	poseStack.GetTop().Clear();
	poseStack.SetPoseWeight( cmd.weight );
	return noOverflow;
}

void C_CommandProcessor::PopPose(const C_PopPoseCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton)
{
	BIZ_ASSERT(poseStack.GetDepth() >= 1);
	C_Pose& srcPose = poseStack.GetTop(0);
	C_Pose& dstPose = poseStack.GetTop(1);

	E_BlendMode srcLastBlendMode = poseStack.GetLastBlendMode(0);
	E_BlendMode dstLastBlendMode = poseStack.GetLastBlendMode(1);

	if ( IsAddOrLerp(cmd.blendMode) && dstLastBlendMode == E_BlendMode_Accumulate )
	{
		dstPose.Normalise( skeleton );
	}

	srcPose.BlendInto( dstPose, cmd.blendMode, poseStack.GetPoseWeight() );
	poseStack.Pop();

	poseStack.SetLastBlendMode( cmd.blendMode );
}

void C_CommandProcessor::MirrorPose(const C_MirrorPoseCommand& cmd, C_PoseStack& poseStack)
{
	C_Pose& pose = poseStack.GetTop();

	Spu::MemBaseMarker poseMirrorMemBase;

	// Apply mirroring.
	const C_PoseMirror* poseMirror = cmd.poseMirror;
	Spu::FetchAlloc(poseMirror);
	poseMirror->MirrorPose( pose );

	poseStack.SetLastBlendMode( E_BlendMode_Invalid );
}

void C_CommandProcessor::CopyInPose(C_Pose& destPose, const C_Skeleton* destSkeleton, const C_Pose* sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping* poseMapping, bool ignoreScale, bool normalised)
{
	if (!normalised)
	{
		destPose.Normalise( destSkeleton );
	}

	Spu::MemBaseMarker memBase;
	
	C_Pose* localSourcePose = (C_Pose*) Spu::FetchAlloc(sourcePose, sizeof(C_Pose));
	localSourcePose->SpuFetch();
	
	C_PoseMapping* mapping = (C_PoseMapping*) Spu::FetchAlloc(poseMapping, sizeof(C_PoseMapping));
	mapping->SpuFetch();
		
	C_Skeleton* localSourceSkeleton = NULL;
	
	if (sourceSkeleton != NULL)
	{
		localSourceSkeleton = (C_Skeleton*) Spu::FetchAlloc(sourceSkeleton, sizeof(C_Skeleton));
		localSourceSkeleton->SpuFetchHierarchy();
	}
		
	destPose.CopyFrom(destSkeleton, *localSourcePose, localSourceSkeleton, *mapping, ignoreScale);
}

void C_CommandProcessor::SetIkTargetWeights(const C_SetIkTargetWeightsCommand& cmd)
{
	// Fetch IK chains.
	Spu::MemBaseMarker memBase;

	C_IkChain* ikChain = 0;
	C_IkChain* ikChainEa = 0;

	for (U32 i = 0; i < cmd.count; ++i)
	{
		if (cmd.chains[i] != ikChainEa)
		{
			#ifdef PLATFORM_SPU			
			if (ikChain)
			{
				// Write back updated IkChain.
				Spu::Store(ikChainEa, ikChain, sizeof(C_IkChain));
			}
			#endif

			memBase.Restore();
			
			// Fetch new IkChain.
			ikChain = (C_IkChain*) Spu::FetchAlloc(cmd.chains[i], sizeof(C_IkChain));
			ikChainEa = cmd.chains[i];
		}
		
		ikChain->SetTargetWeight(cmd.targets[i], cmd.weights[i]);
	}

	#ifdef PLATFORM_SPU			
	if (ikChain)
	{
		// Write back updated IkChain.
		Spu::Store(ikChainEa, ikChain, sizeof(C_IkChain));
	}
	#endif
}

void C_CommandProcessor::SetIkTargetFromJoint(const C_SetIkTargetFromJointCommand& cmd, C_PoseStack& poseStack, const C_Skeleton* skeleton)
{
	C_Pose& pose = poseStack.GetTop();
	
	if (static_cast<U32>(cmd.jointIndex) >= pose.GetNumJointTransforms())
	{
		BIZ_ASSERT(false);
		return;
	}
	
	C_JointTransform offset;
	
	offset.SetIdentity();
	offset.SetRotation(Maths::MakeQuaternion(Maths::Load<Maths::Vector4>(cmd.rotationOffset)));
	offset.SetTranslation(Maths::Load<Maths::Vector3>(cmd.translationOffset));
	
	Spu::MemBaseMarker ikChainMemBase;
	
	// Fetch IK chain.
	C_IkChain* ikChain = (C_IkChain*) Spu::FetchAlloc(cmd.ikChain, sizeof(C_IkChain));
	
	// Set IK target.
	// TODO: need to normalise the result just in case!
	if (cmd.setPosition && cmd.setRotation)
	{
		ikChain->SetTargetTransformFromJoint(cmd.targetIndex, cmd.jointIndex, pose, skeleton, offset);
	}
	else if (cmd.setPosition)
	{
		ikChain->SetTargetPositionFromJoint(cmd.targetIndex, cmd.jointIndex, pose, skeleton, offset.GetPosition());
	}
	else if (cmd.setRotation)
	{
		ikChain->SetTargetRotationFromJoint(cmd.targetIndex, cmd.jointIndex, pose, skeleton, offset.GetRotation());
	}
	
	// NB. not storing IK chain object back to main memory since SetTargetXXX should not modify
	// any of its member variables.
}

void C_CommandProcessor::EvaluateIkChain(
	const C_EvaluateIkChainCommand& cmd,
	C_PoseStack& poseStack, const C_Skeleton* skeleton, bool topPoseIsNormalised, 
	const C_JointTransform& modelWorldTransform)
{
	PERF_EVENT("Animation.ExecuteCommandList.EvaluateIkChain");
	
	C_Pose& pose = poseStack.GetTop();
	
	if (!topPoseIsNormalised)
	{
		pose.Normalise( skeleton );
	}
	
	Spu::MemBaseMarker ikChainMemBase;
	C_IkChain* ikChain = (C_IkChain*) Spu::FetchAlloc(cmd.ikChain, sizeof(C_IkChain));
	ikChain->EvaluateAndApply(pose, skeleton, modelWorldTransform);
	#ifdef PLATFORM_SPU
	Spu::Store(cmd.ikChain, ikChain, sizeof(C_IkChain)); // Is this necessary?
	#endif
}

void C_CommandProcessor::TransformJoint(
	S32 jointIndex,
	const C_JointTransform& transform,
	C_PoseStack& poseStack,
	const C_Skeleton* skeleton,
	bool topPoseIsNormalised
)
{
	PERF_EVENT("Animation.ExecuteCommandList.TransformJoint");
	
	C_Pose& pose = poseStack.GetTop();
	
	if (!topPoseIsNormalised)
	{
		pose.Normalise( skeleton );
	}
	
	if (skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, pose);
		C_JointTransform t = skeletonPose.GetModelJointTransform(jointIndex);
		C_JointTransform newT = t * transform;
		skeletonPose.SetModelJointTransform(jointIndex, newT);
	}
}

void C_CommandProcessor::TransformJointTranslation(
	S32 jointIndex,
	const C_JointTransform& transform,
	C_PoseStack& poseStack,
	const C_Skeleton* skeleton,
	bool topPoseIsNormalised
)
{
	PERF_EVENT("Animation.ExecuteCommandList.TransformJointTranslation");
	
	C_Pose& pose = poseStack.GetTop();
	
	if (!topPoseIsNormalised)
	{
		pose.Normalise( skeleton );
	}
	
	if (skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, pose);
		C_JointTransform t = skeletonPose.GetModelJointTransform(jointIndex);
		//t.SetTranslation(Maths::MakeVector3(transform.TransformPoint(Maths::MakePoint3(t.GetTranslation()))));
		t.SetTranslation(transform.TransformPoint(t.GetTranslation()));
		skeletonPose.SetModelJointTransform(jointIndex, t);
	}
}

void C_CommandProcessor::CopyOutJointTransforms(
	const C_CopyOutJointTransformsCommand& cmd,
	C_PoseStack& poseStack,
	const C_Skeleton* skeleton,
	bool topPoseIsNormalised
)
{
	PERF_EVENT("Animation.ExecuteCommandList.CopyOutJointTransforms");
	
	C_Pose& pose = poseStack.GetTop();
	
	if (!topPoseIsNormalised)
	{
		pose.Normalise( skeleton );
	}
	
	Spu::MemBaseMarker memBaseMarker;
	
	const S16* jointIndices = cmd.localJointIndices;
	if (cmd.numJoints > C_CopyOutJointTransformsCommand::kMaxLocalJointIndices)
	{
		jointIndices = (const S16*) Spu::FetchAlloc(jointIndices, sizeof(U16) * cmd.numJoints);
	}
	
	#ifdef PLATFORM_SPU
	C_JointTransform* destJointTransforms = (C_JointTransform*) Spu::Alloc(sizeof(C_JointTransform) * cmd.numJoints);
	#else
	C_JointTransform* destJointTransforms = cmd.destJointTransforms;
	#endif
	
	if (cmd.space == E_Space_Local || skeleton == NULL)
	{
		for (U32 i = 0; i < cmd.numJoints; ++i)
		{
			destJointTransforms[i] = pose.GetJointTransform(jointIndices[i]);
		}
	}
	else if (cmd.space == E_Space_Model && skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, pose);
		for (U32 i = 0; i < cmd.numJoints; ++i)
		{
			destJointTransforms[i] = skeletonPose.GetModelJointTransform(jointIndices[i]);
		}
	}
			
	#ifdef PLATFORM_SPU
	Spu::Store(cmd.destJointTransforms, destJointTransforms, sizeof(C_JointTransform) * cmd.numJoints);
	#endif
}

void C_CommandProcessor::ApplyRagdoll(const C_CommandList& commandList, C_Pose& pose, const C_Skeleton* skeleton) const
{
	PERF_EVENT("Animation.ExecuteCommandList.ApplyRagdoll");

	if (skeleton == NULL)
	{
		return;
	}

	const C_CommandList::C_ApplyRagdollCommand& applyRagdollCmd = commandList.m_applyRagdollCommand;

	Spu::MemBaseMarker ragdollMemBaseMarker;

	const U16* ragdollPoseJointIndices = (const U16*) Spu::FetchAlloc(applyRagdollCmd.jointIndices, sizeof(S16) * applyRagdollCmd.numJoints);

	if (applyRagdollCmd.outputPose != NULL)
	{
		//
		// Save ragdoll joint transforms into output pose.
		//

		PERF_EVENT("Animation.ExecuteCommandList.SaveOutputRagdollPose");
		
		#ifdef PLATFORM_SPU
		Maths::C_QuatTrans* destRagdollPose = (Maths::C_QuatTrans*) Spu::Alloc(sizeof(Maths::C_QuatTrans) * applyRagdollCmd.numJoints);
		#else
		Maths::C_QuatTrans* destRagdollPose = applyRagdollCmd.outputPose;
		#endif
		
		C_SkeletonPose skeletonPose(*skeleton, pose);
		for (U32 i = 0, count = applyRagdollCmd.numJoints; i < count; ++i)
		{
			const S32 jointIndex = ragdollPoseJointIndices[i];
			if (jointIndex >= 0)
			{
				C_JointTransform modelJointTransform = skeletonPose.GetModelJointTransform(jointIndex);
				Maths::C_QuatTrans qt(Maths::C_Quaternion(modelJointTransform.GetRotation()), Maths::HVec3(modelJointTransform.GetTranslation()));
				destRagdollPose[i] = qt;
			}
		}	
		
		#ifdef PLATFORM_SPU
		Spu::Store(applyRagdollCmd.outputPose, destRagdollPose, applyRagdollCmd.numJoints * sizeof(Maths::C_QuatTrans));
		#endif			
	}
		
	if (applyRagdollCmd.inputPose != NULL)
	{
		PERF_EVENT("Animation.ExecuteCommandList.ApplyInputRagdollPose");
		
		//
		// Apply ragdoll joint transforms from srcRagdollPose to the current pose.
		//
		
		const Maths::C_QuatTrans* srcRagdollPose = (const Maths::C_QuatTrans*) Spu::FetchAlloc(applyRagdollCmd.inputPose, sizeof(Maths::C_QuatTrans) * applyRagdollCmd.numJoints);
		
		Animation::C_SkeletonPose skeletonPose(*skeleton, pose);
		for (U32 i = 0, count = applyRagdollCmd.numJoints; i < count; ++i)
		{
			const S32 jointIndex = ragdollPoseJointIndices[i];
			if (jointIndex >= 0)
			{
				// TODO: quaternion nearest check vs. previous model space rotation
				C_JointTransform modelJointTransform(
					srcRagdollPose[i].Quaternion(),
					srcRagdollPose[i].Translation()
				);
				skeletonPose.SetModelJointTransform(jointIndex, modelJointTransform);
			}
		}				
	}
}

void C_CommandProcessor::ApplyConstraints(const C_CommandList& commandList, C_Pose& pose, const C_Skeleton* skeleton, const C_JointTransform& worldTransform) const
{
	const C_CommandList::C_ApplyConstraintsCommand& applyConstraintsCommand = commandList.m_applyConstraintsCommand;

	PERF_EVENT("Animation.ExecuteCommandList.ApplyConstraints");

	Spu::MemBaseMarker skeletonInstanceMarker;
	C_SkeletonInstance* skeletonInstance = (C_SkeletonInstance*) Spu::FetchAlloc(applyConstraintsCommand.skeletonInstance, sizeof(C_SkeletonInstance));

	const U32 kSkelInstDmaTag = 1;
	
	// Fetch dynamic joints and other data required by SkeletonInstance.
	skeletonInstance->SpuFetchAsync(kSkelInstDmaTag);

	// Apply wind.
	if (commandList.m_flags & C_CommandList::E_Flag_ApplyWindForces)
	{
		Spu::MemBaseMarker windParamsMarker;
		const C_WindParams* windParams = (const C_WindParams*) Spu::FetchAllocAsync(applyConstraintsCommand.windParams, sizeof(Animation::C_WindParams), kSkelInstDmaTag);
		Spu::Wait(kSkelInstDmaTag);
		skeletonInstance->ApplyWindForces(*windParams, applyConstraintsCommand.timeStep);
	}
	else
	{
		Spu::Wait(kSkelInstDmaTag);
	}
		
	skeletonInstance->ApplyJointConstraints(pose, worldTransform, skeleton);

	#ifdef PLATFORM_SPU
	// Store back updated dynamic joint data.
	skeletonInstance->SpuStore();
	Spu::Store(applyConstraintsCommand.skeletonInstance, skeletonInstance, sizeof(C_SkeletonInstance));
	#endif
}

} // namespace Animation
