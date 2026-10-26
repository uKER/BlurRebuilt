#include "stdafx.h"
#include <animation/deferred/CommandList.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/pose/PoseMapping.h>
#include <system/scheduler/jobqueue.h>
#include <btl/utility/types.h>

namespace Animation
{

//
// C_CommandList
//


C_CommandList::C_CommandList( U32 maxCommands, U32 initialUpdateCounter ):
	m_flags( 0 ),
	m_maxCommands( Maths::Min(maxCommands, kMaxCommands) ),
	m_numCommands( 0 ),
	m_enablePushPopElimination( 1 ),
	m_updateCounter( initialUpdateCounter ),
#if defined(PLATFORM_SPU)
	m_inMainMemory(0)
#else
	m_inMainMemory(1)
#endif
{
	// Layout command buffer memory.
	C_MemoryBlockSizer sizer;
	sizer.Allocate<C_Command>(m_commands, m_maxCommands);
	m_commandData.Create(sizer);
}

C_CommandList::~C_CommandList()
{
}

void C_CommandList::SpuFetchAsync( U32 tag )
{
	// TODO: could alter to just fetch the used part, assuming that command list is read only.
	m_commandData.SpuFetchAsync(tag);
}

void C_CommandList::SpuFetch()
{
	SpuFetchAsync(0);
	Spu::Wait(0);
}

void C_CommandList::Clear()
{
#ifdef PLATFORM_SPU
	m_inMainMemory = 0;
#else
	m_inMainMemory = 1;
#endif

	m_numCommands = 0;
	m_flags = 0;
	++m_updateCounter;

	m_modelWorldMatrix = Maths::Const<Maths::Mat43>::Identity();
}

void C_CommandList::BlendCycle( E_BlendMode blendMode, C_CycleInstance* cycleInstance, F32 time, F32 weight, E_LoopMode loopMode, const C_PoseFieldMask& fieldMask )
{
	if ( !cycleInstance )
	{
		return;
	}
	
	if (time != time)
	{
		BIZ_ASSERT(!"Time is NaN");
		return;
	}
	
	if ( !cycleInstance->IsValid() )
	{
		return;
	}
	
	const F32 kEpsilon = 1.f / 1024.f;
	if ( weight < kEpsilon )
	{
		return;
	}
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_BlendCycle;
		cmd->blendCycle.cycleInstance = cycleInstance;
		cmd->blendCycle.time = time;
		cmd->blendCycle.weight = weight;
		cmd->blendCycle.blendMode = blendMode;
		cmd->blendCycle.loopMode = loopMode;
		cmd->blendCycle.fieldMask = fieldMask.AsU16();
	}
}

void C_CommandList::PushCurrentPose( F32 newPoseWeight )
{
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_PushCurrentPose;
		cmd->pushPose.weight = newPoseWeight;
	}
}

void C_CommandList::PopAndBlendPose( E_BlendMode blendMode )
{
	C_Command* pushCmd = FindMatchingPush();
	
	if ( NULL == pushCmd )
	{
		BIZ_ASSERT(!"Pop without push!");
		return;
	}
	
	bool addPopCmd = true;
	
	C_Command* lastCmd = GetLastCommand();
	BIZ_ASSERT(lastCmd != NULL);

	// Number of commands between push and pop.
	const U32 numInnerCmds = lastCmd->index - pushCmd->index;
	
	if (numInnerCmds == 0)
	{
		// Remove empty push/pop.
		addPopCmd = false;
		RemoveCommand(pushCmd->index);
	}
	else if (m_enablePushPopElimination)
	{
		U32 numBlendCycles = 0;
		
		// Count number of matching blend cycle commands that use the same blend mode.
		const U32 firstInnerCmdIndex = pushCmd->index + 1;
		E_BlendMode innerBlendMode = E_BlendMode_Invalid;
		U32 numMatchingBlends = 0;
		U32 numBlends = 0;
		
		if (GetCommand(firstInnerCmdIndex).type == E_CommandType_BlendCycle)
		{
			innerBlendMode = GetCommand(firstInnerCmdIndex).blendCycle.blendMode;
			numMatchingBlends = 1;
			numBlends = 1;
		
			for (U32 i = 1; i < numInnerCmds && numMatchingBlends == i; ++i)
			{
				const C_Command& innerCmd = GetCommand(firstInnerCmdIndex + i);
				if (innerCmd.type == E_CommandType_BlendCycle)
				{
					++numBlends;
					if (innerCmd.blendCycle.blendMode == innerBlendMode)
					{
						++numMatchingBlends;
					}				
				}
			}
		}
		
		//
		// We can flatten a series of nested ACC blends into a series of ADD blends, but not vice versa.
		//
		bool canFlatten = (innerBlendMode == E_BlendMode_Accumulate && blendMode == E_BlendMode_Additive)
							|| (innerBlendMode == E_BlendMode_Accumulate && blendMode == E_BlendMode_Lerp && numInnerCmds == 1)
							|| (innerBlendMode == blendMode);		
		
		// Is push/pop block entirely made up of cycle evaluations with the same blend mode?
		if ( numInnerCmds == numMatchingBlends && canFlatten)
		{
			// Make blends use same mode as the pop and blend op.
			// Note that a sequence of additive blends will not make sense anyway and should
			// be converted to accumulate blends.
			addPopCmd = false;

			// Make all blends use the same mode as this pop operation and apply the enclosing block's weight.
			F32 invWeight = pushCmd->pushPose.weight;
			for (U32 i = 0; i < numInnerCmds; ++i)
			{
				C_Command& innerCmd = GetCommand(firstInnerCmdIndex + i);
				if (innerCmd.type == E_CommandType_BlendCycle)
				{
					innerCmd.blendCycle.blendMode = blendMode;
					innerCmd.blendCycle.weight *= invWeight;
				}
			}
			RemoveCommand(pushCmd->index);
		}
		else if (pushCmd->pushPose.weight == 1.f && pushCmd->index == 0)
		{
			addPopCmd = false;
			RemoveCommand(pushCmd->index);
		}
		else if (pushCmd->index > 0)
		{
			const C_Command& cmdBeforePush = GetCommand(pushCmd->index - 1);
			if (cmdBeforePush.type == E_CommandType_PushCurrentPose
				&& pushCmd->pushPose.weight == cmdBeforePush.pushPose.weight)
			{
				addPopCmd = false;
				RemoveCommand(pushCmd->index);
			}
		}
	}
	
	if ( addPopCmd )
	{
		// Add the pop.
		C_Command* cmd = NewCommand();
		if (cmd != NULL)
		{
			cmd->type = E_CommandType_PopAndBlendPose;
			cmd->popPose.weight = pushCmd->pushPose.weight;
			cmd->popPose.blendMode = blendMode;
		}
	}
	
	//LOGINFO(Debug::GetLogger("mercury.animation"), "*** After:");
	//Dump(*this);	
}

void C_CommandList::EvaluateBasePose()
{
}

void C_CommandList::MirrorPose( const C_PoseMirror& poseMirror )
{
	const C_Command* lastCmd = GetLastCommand();
	if (lastCmd != NULL)
	{
		if (lastCmd->type == E_CommandType_PushCurrentPose)
		{
			// Mirroring a blank pose has no effect, so remove this command.
			return;
		}
	}
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_MirrorPose;
		cmd->mirrorPose.poseMirror = &poseMirror;
	}
}

void C_CommandList::CopyInPose( const C_Pose& sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping& poseMapping, bool ignoreScale )
{
	BIZ_ASSERT_MSG(C_Pose::IsReadable(&sourcePose), "Attempt to copy data from a pose being modified on another thread");
	BIZ_ASSERT(poseMapping.IsValidSourcePose(sourcePose));
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_CopyInPose;
		cmd->copyInPose.sourcePose = &sourcePose;
		cmd->copyInPose.sourceSkeleton = sourceSkeleton;
		cmd->copyInPose.poseMapping = &poseMapping;
		cmd->copyInPose.ignoreScale = ignoreScale;
	}
}

void C_CommandList::TransformJointInModelSpace( S32 jointIndex, const C_JointTransform& modelSpaceTransform )
{
	if (jointIndex < 0) return;
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_TransformJoint;
		cmd->transformJoint.jointIndex = jointIndex;
		Maths::Store(modelSpaceTransform.GetRotation(), cmd->transformJoint.rotation);
		Maths::Store(modelSpaceTransform.GetTranslation(), cmd->transformJoint.translation);
		Maths::Store(modelSpaceTransform.GetScale(), cmd->transformJoint.scale);
	}
}

void C_CommandList::RotateJointInModelSpace( S32 jointIndex, const Maths::Quaternion& modelSpaceRotation )
{
	TransformJointInModelSpace( jointIndex, C_JointTransform(modelSpaceRotation, Maths::Const<Maths::Vector3>::Zero()) );
}

void C_CommandList::TranslateJointInModelSpace( S32 jointIndex, const Maths::Vector3& modelSpaceTranslation )
{
	TransformJointInModelSpace( jointIndex, C_JointTransform(Maths::Const<Maths::Quaternion>::Identity(), modelSpaceTranslation) );
}

void C_CommandList::TransformJointTranslationInModelSpace( S32 jointIndex, const C_JointTransform& modelSpaceTransform )
{
	if (jointIndex < 0) return;

	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_TransformJointTranslation;
		cmd->transformJoint.jointIndex = jointIndex;
		Maths::Store(modelSpaceTransform.GetRotation(), cmd->transformJoint.rotation);
		Maths::Store(modelSpaceTransform.GetTranslation(), cmd->transformJoint.translation);
		Maths::Store(modelSpaceTransform.GetScale(), cmd->transformJoint.scale);
	}
}

void C_CommandList::CopyOutLocalSpaceJointTransforms( const S16* jointIndices, C_JointTransform* destJointTransforms, U32 numJoints )
{
	if ( 0 == numJoints )
		return;

	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_CopyOutJointTransforms;
		cmd->copyOutJointTransforms.space = E_Space_Local;
		cmd->copyOutJointTransforms.jointIndices = jointIndices;
		cmd->copyOutJointTransforms.destJointTransforms = destJointTransforms;
		cmd->copyOutJointTransforms.numJoints = numJoints;
		
		if (numJoints <= C_CopyOutJointTransformsCommand::kMaxLocalJointIndices)
		{
			memcpy(cmd->copyOutJointTransforms.localJointIndices, jointIndices, sizeof(S16) * numJoints);
		}
	}
}

void C_CommandList::CopyOutModelSpaceJointTransforms( const S16* jointIndices, C_JointTransform* destJointTransforms, U32 numJoints )
{
	if ( 0 == numJoints )
		return;

	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_CopyOutJointTransforms;
		cmd->copyOutJointTransforms.space = E_Space_Model;
		cmd->copyOutJointTransforms.jointIndices = jointIndices;
		cmd->copyOutJointTransforms.destJointTransforms = destJointTransforms;
		cmd->copyOutJointTransforms.numJoints = numJoints;
		
		if (numJoints <= C_CopyOutJointTransformsCommand::kMaxLocalJointIndices)
		{
			memcpy(cmd->copyOutJointTransforms.localJointIndices, jointIndices, sizeof(S16) * numJoints);
		}
	}
}

void C_CommandList::SetIkTargetWeight( C_IkChain& ikChain, U32 targetIndex, F32 weight )
{
	C_Command* lastCmd = GetLastCommand();
	C_Command* cmd = NULL;

	U32 count = 1;

	if (lastCmd)
	{
		if (lastCmd->type == E_CommandType_SetIkTargetWeights && lastCmd->setIkTargetWeights.count < C_SetIkTargetWeightsCommand::kMaxCount)
		{
			count = lastCmd->setIkTargetWeights.count + 1;
			cmd = lastCmd;
		}
	}

	if (!cmd)
	{
		cmd = NewCommand();
		cmd->type = E_CommandType_SetIkTargetWeights;
		if (!cmd)
		{
			return;
		}
	}

	cmd->setIkTargetWeights.count = count;
	BIZ_ASSERT(count > 0);
	U32 index = count - 1;

	cmd->setIkTargetWeights.chains[index] = &ikChain;
	cmd->setIkTargetWeights.targets[index] = targetIndex;
	cmd->setIkTargetWeights.weights[index] = weight;
}

void C_CommandList::SetIkTargetTransformFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Quaternion& rotationOffset, const Maths::Point3& positionOffset )
{
	if (jointIndex < 0) return;
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_SetIkTargetFromJoint;
		cmd->setIkTargetFromJoint.ikChain = &ikChain;
		cmd->setIkTargetFromJoint.targetIndex = targetIndex;
		cmd->setIkTargetFromJoint.setPosition = 1;
		cmd->setIkTargetFromJoint.setRotation = 1;
		cmd->setIkTargetFromJoint.setPoleVector = 0;
		cmd->setIkTargetFromJoint.jointIndex = jointIndex;
		
		Maths::Store(rotationOffset, cmd->setIkTargetFromJoint.rotationOffset);
		Maths::Store(Maths::MakeVector3(positionOffset), cmd->setIkTargetFromJoint.translationOffset);
	}
}

void C_CommandList::SetIkTargetRotationFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Quaternion& offset )
{
	if (jointIndex < 0) return;

	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_SetIkTargetFromJoint;
		cmd->setIkTargetFromJoint.ikChain = &ikChain;
		cmd->setIkTargetFromJoint.targetIndex = targetIndex;
		cmd->setIkTargetFromJoint.setPosition = 0;
		cmd->setIkTargetFromJoint.setRotation = 1;
		cmd->setIkTargetFromJoint.setPoleVector = 0;
		cmd->setIkTargetFromJoint.jointIndex = jointIndex;
		
		Maths::Store(offset, cmd->setIkTargetFromJoint.rotationOffset);
	}
}

void C_CommandList::SetIkTargetPositionFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Point3& offset )
{
	if (jointIndex < 0) return;
	
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_SetIkTargetFromJoint;
		cmd->setIkTargetFromJoint.ikChain = &ikChain;
		cmd->setIkTargetFromJoint.targetIndex = targetIndex;
		cmd->setIkTargetFromJoint.setPosition = 1;
		cmd->setIkTargetFromJoint.setRotation = 0;
		cmd->setIkTargetFromJoint.setPoleVector = 0;
		cmd->setIkTargetFromJoint.jointIndex = jointIndex;
		Maths::Store(Maths::MakeVector3(offset), cmd->setIkTargetFromJoint.translationOffset);
	}
}

void C_CommandList::EvaluateIkChain( C_IkChain& ikChain )
{
	C_Command* cmd = NewCommand();
	if (cmd != NULL)
	{
		cmd->type = E_CommandType_EvaluateIkChain;
		cmd->evaluateIkChain.ikChain = &ikChain;
	}
}	

void C_CommandList::ApplyRagdoll( U32 numRagdollJoints, const S16* ragdollJointIndices, Maths::C_QuatTrans* outputRagdollPose, const Maths::C_QuatTrans* inputRagdollPose )
{
	m_flags |= E_Flag_ApplyRagdoll;

	C_ApplyRagdollCommand& ragdollCmd = m_applyRagdollCommand;
	
	ragdollCmd.numJoints = numRagdollJoints;
	ragdollCmd.jointIndices = ragdollJointIndices;
	ragdollCmd.inputPose = inputRagdollPose;
	ragdollCmd.outputPose = outputRagdollPose;
}
		
void C_CommandList::ApplyConstraints( C_SkeletonInstance& skeletonInstance, const Maths::Mat43& worldMatrix )
{
	if (skeletonInstance.HasConstraints())
	{
		m_flags |= E_Flag_ApplyConstraints;
	
		C_ApplyConstraintsCommand& constraintsCmd = m_applyConstraintsCommand;
	
		constraintsCmd.skeletonInstance = &skeletonInstance;
	}
	
	SetModelWorldMatrix(worldMatrix);
}

void C_CommandList::SetRootToIdentity()
{
	m_flags |= E_Flag_RelativeToRoot;
}

void C_CommandList::SetModelWorldMatrix(const Maths::Mat43& modelWorldMatrix)
{
	m_modelWorldMatrix = modelWorldMatrix;
}

void C_CommandList::ApplyWindForces(const C_WindParams& windParams, F32 timeStep)
{
	m_flags |= E_Flag_ApplyWindForces;
	m_applyConstraintsCommand.windParams = &windParams;
	m_applyConstraintsCommand.timeStep = timeStep;
}


#ifndef PLATFORM_SPU
void C_CommandList::BindToPose( const C_Pose& pose )
{
	for ( U32 i = 0, count = GetNumCommands(); i < count; ++i )
	{
		C_Command& cmd = GetCommand(i);
	
		if ( cmd.type == E_CommandType_BlendCycle )
		{
			cmd.blendCycle.cycleInstance->BindToPose( pose );
		}
	}
}
#endif

U32 C_CommandList::GetNumCommands() const
{
	return m_numCommands;
}

const C_Command& C_CommandList::GetCommand( U32 index ) const
{
	BIZ_ASSERT( index <= GetNumCommands() ); // Allow plus 1.
	return m_commands(m_commandData)[index];
}

C_Command& C_CommandList::GetCommand( U32 index )
{
	BIZ_ASSERT( index <= GetNumCommands() ); // Allow plus 1.
	return m_commands(m_commandData)[index];
}

U32 C_CommandList::GetUpdateCounter() const
{
	return m_updateCounter;
}

void C_CommandList::SetUpdateCounter( U32 value )
{
	m_updateCounter = value;
}

C_Command* C_CommandList::FindMatchingPush()
{
	C_Command* cmd = NULL;
	S32 depth = 0;
	for ( U32 i = m_numCommands; i > 0 && cmd == NULL; --i )
	{
		C_Command& c = GetCommand(i - 1);
		
		switch ( c.type )
		{
			case E_CommandType_PopAndBlendPose:
				++depth;
				break;
				
			case E_CommandType_PushCurrentPose:
				if ( 0 == depth )
				{
					cmd = &c;
				}
				else
				{
					--depth;
				}
				break;
				
			default:
				break;
		}
	}
	return cmd;
}

C_Command* C_CommandList::GetLastCommand()
{
	C_Command* cmd = NULL;
	if (m_numCommands > 0)
	{
		return &GetCommand( m_numCommands - 1);
	}
	return cmd;
}

void C_CommandList::RemoveCommand( U32 index )
{
	// Shift commands down and update indices.
	C_Command* commands = m_commands(m_commandData);
	for (U32 i = index + 1; i < m_numCommands; ++i)
	{
		commands[i - 1] = commands[i];
		commands[i - 1].index = i - 1;
	}
	--m_numCommands;
}

C_Command* C_CommandList::NewCommand()
{
	if (m_numCommands < m_maxCommands)
	{
		C_Command* cmd = &(m_commands(m_commandData)[m_numCommands]);
		cmd->index = m_numCommands;
		++m_numCommands;
		return cmd;
	}
	return NULL;
}



} // namespace Animation
