#ifndef ANIMATION_COMMANDLIST_H
#define ANIMATION_COMMANDLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>
#include <animation/common/BlendModes.h>
#include <animation/common/MemoryBlock.h>
#include <animation/pose/PoseFieldMask.h>
//#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

namespace Animation
{

	class C_CycleInstance;
	class C_Pose;
	class C_PoseMirror;
	class C_PoseMapping;
	class C_SkeletonInstance;
	class C_IkChain;
	class C_JointTransform;
	class C_Skeleton;
	class C_WindParams;

	class C_Command;
	
	/**	
	 \brief A list of animation commands for evaluating and blending cycles into a C_Pose, applying
	 constraints (via update of a C_SkeletonInstance) and updating a pose matrix cache.
	 
	 Stores animation evaluation functions into a command buffer for deferred evaluation.
	
	 A command list will typically be built during blend tree evaluation and then
	 processed asynchronously via a thread or SPU task.
	 
	 Layering of blended animations can be achieved using the PushCurrentPose and PopAndBlendPose
	 functions.
	*/
	
	class ALIGN16 C_CommandList
	{
	public:
	
		friend class C_CommandProcessor;
	
		C_CommandList( U32 maxCommands, U32 initialUpdateCounter = 0 );
		
		~C_CommandList();
		
		void SpuFetchAsync( U32 tag );
		void SpuFetch();
		
		void Clear();
		
		//
		// Ordered commands.
		//
	
		// Blend current pose with a cycle instance, which should be in local memory.
		void BlendCycle( E_BlendMode blendMode, C_CycleInstance* cycleInstance, F32 time, F32 weight, E_LoopMode loopMode = E_LoopMode_Continuous, const C_PoseFieldMask& fieldMask = C_PoseFieldMask() );

		// Push current pose and start a new pose with the specified weight.
		void PushCurrentPose( F32 newPoseWeight );
		
		// Pop pose on top of stack and blend into current pose using the weight specified for
		// the corresponding push.
		void PopAndBlendPose( E_BlendMode blendMode );

		/// Fill in base pose for joint transform components with zero accumulated weight.
		void EvaluateBasePose();
		
		/// Mirror the current pose using the specified C_PoseMirror.
		void MirrorPose( const C_PoseMirror& poseMirror );
		
		/// Copies mapped fields from a source pose using the speified pose mapping.
		void CopyInPose( const C_Pose& sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping& poseMapping, bool ignoreScale );
		
		//
		// Joint transformation.
		//
		
		void TransformJointInModelSpace( S32 jointIndex, const C_JointTransform& modelSpaceTransform );
		
		void RotateJointInModelSpace( S32 jointIndex, const Maths::Quaternion& modelSpaceRotation );
		
		void TranslateJointInModelSpace( S32 jointIndex, const Maths::Vector3& modelSpaceTranslation );

		void TransformJointTranslationInModelSpace( S32 jointIndex, const C_JointTransform& modelSpaceTransform );
		
		//
		// Transform extraction.
		//
		
		/// Copies out local space joint transforms from the currently active pose.
		void CopyOutLocalSpaceJointTransforms( const S16* jointIndices, C_JointTransform* destJointTransforms, U32 numJoints );
		
		/// Copies out model space joint transforms from the currently active pose.
		void CopyOutModelSpaceJointTransforms( const S16* jointIndices, C_JointTransform* destJointTransforms, U32 numJoints );
		
		//
		// IK.
		//

		void SetIkTargetWeight( C_IkChain& ikChain, U32 targetIndex, F32 weight );
				
		/// Sets an IK target to be the transform of a joint in the current pose.
		/// @a offset is applied as an offset in the joint's local space.
		void SetIkTargetTransformFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Quaternion& rotationOffset = Maths::Const<Maths::Quaternion>::Identity(), const Maths::Point3& positionOffset = Maths::Const<Maths::Point3>::Zero());

		/// Sets an IK target to the rotation of a joint in the current pose.
		/// @a offset is applied as a rotational offset in the joint's local space.
		void SetIkTargetRotationFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Quaternion& offset = Maths::Const<Maths::Quaternion>::Identity());

		/// Sets an IK target to the position of a joint in the current pose.
		/// @a offset is applied as a translational offset in the joint's local space.
		void SetIkTargetPositionFromJoint( C_IkChain& ikChain, U32 targetIndex, S32 jointIndex, const Maths::Point3& offset = Maths::Const<Maths::Point3>::Zero());
		
		/// Evaluates an IK chain, applying it to the current pose. Note that the current pose will be normalised.
		void EvaluateIkChain( C_IkChain& ikChain );
		
		//
		// Commands applied after animation blending.
		//
				
		/// After blending, saves output ragdoll pose and applies input ragdoll pose.
		// TODO: split into two separate methods to apply a ragdoll/custom pose and to save out joint transforms into a buffer.
		void ApplyRagdoll( U32 numRagdollJoints, const S16* ragdollJointIndices, Maths::C_QuatTrans* outputRagdollJoints, const Maths::C_QuatTrans* inputRagdollJoints );
		
		/// Updates the specified skeleton instance and applies its constraints to the pose after cycle blending and ragdoll.
		/// Note that C_SkeletonInstance::UpdateDynamicJoints() should be called prior to execution of this command list.
		void ApplyConstraints( C_SkeletonInstance& skeletonInstance, const Maths::Mat43& worldMatrix );
		
		/// Set root joint transform to identity so that the final pose is relative to the root.
		/// @note The original root joint transform is still available via C_Pose::GetTrajectoryTransform().
		void SetRootToIdentity();
		
		/// Sets the model world matrix used for constraints and IK chains.
		void SetModelWorldMatrix(const Maths::Mat43& modelWorldMatrix);

		/// Applies wind forces to dynamic joints.
		void ApplyWindForces(const C_WindParams& windParams, F32 timeStep);

					
#ifndef PLATFORM_SPU
		/// Bind cycles to a pose layout. This must be called before the command list is executed.
		/// PS3 PPU only.
		void BindToPose( const C_Pose& pose );
#endif		

#ifndef CFG_GOLD		
		void DebugPrint();
#endif
			
		/// Get number of commands in list.
		U32 GetNumCommands() const;
		
		/// Get command.
		const C_Command& GetCommand( U32 index ) const;
		
		C_Command& GetCommand( U32 index );		
		
		/// Returns current value of the update counter, which is incremented every time the command list
		/// is cleared.
		U32 GetUpdateCounter() const;
		
		/// Set the current value of the update counter.
		void SetUpdateCounter( U32 value );
		
		/// Enables or disables elimination of redundant push/pops.
		void EnablePushPopElimination( bool enabled = true )		{ m_enablePushPopElimination = enabled ? 1 : 0; }
		
		bool InMainMemory() const
		{
#if defined(PLATFORM_PS3) || defined(PLATFORM_SPU)
			return m_inMainMemory;
#else
			return false;
#endif
		}

		bool IsEmpty() const
		{
			return 0 == m_numCommands;
		}
				
	private:
	
		bool IsQueueFull() const
		{
			BIZ_ASSERT( m_numCommands < m_maxCommands && "Command queue full" );
			return m_numCommands >= m_maxCommands;
		}
		
		C_Command* FindMatchingPush();
		C_Command* GetLastCommand();
		void RemoveCommand( U32 index );
		
		C_Command* NewCommand();
	
		static const U32 kMaxCommands = 255;
		
		enum E_Flag
		{
			E_Flag_ApplyRagdoll		= 1 << 1,
			E_Flag_ApplyConstraints	= 1 << 2,
			E_Flag_RelativeToRoot	= 1 << 3,
			E_Flag_ApplyWindForces	= 1 << 4
		};
		
		struct C_ApplyRagdollCommand
		{
			Maths::C_QuatTrans*			outputPose;		// Output ragdoll pose taken from evaluated pose.
			const Maths::C_QuatTrans*	inputPose;		// Pose from ragdoll physics, applied after command list evaluation.
			const S16*					jointIndices;	// A list of joint indices for ragdoll pose.
			U32							numJoints;		// Number of joints in ragdoll pose.
		};
		
		struct C_ApplyConstraintsCommand
		{
			C_SkeletonInstance*	skeletonInstance;
			const C_WindParams*	windParams;
			F32					timeStep;
		};

		Maths::Mat43				m_modelWorldMatrix;
		C_ApplyRagdollCommand		m_applyRagdollCommand;
		C_ApplyConstraintsCommand	m_applyConstraintsCommand;
		U32							m_flags							: 8;
		U32							m_maxCommands					: 8;
		U32							m_numCommands					: 8;
		U32							m_enablePushPopElimination		: 1;
		U32							m_inMainMemory					: 1; // True if data pointed to by commands is in main memory and not SPU local memory.
		
		C_MemoryBlock				m_commandData;
		C_RelativePtr<C_Command>	m_commands;
		U32							m_updateCounter;
	};

	enum E_CommandType
	{
		E_CommandType_BlendCycle = 0,
		E_CommandType_PushCurrentPose,
		E_CommandType_PopAndBlendPose,
		E_CommandType_MirrorPose,
		E_CommandType_SetIkTargetWeights,
		E_CommandType_SetIkTargetFromJoint,
		E_CommandType_EvaluateIkChain,
		E_CommandType_TransformJoint,
		E_CommandType_TransformJointTranslation,
		E_CommandType_CopyOutJointTransforms,
		E_CommandType_CopyInPose,
		
		E_CommandType_ApplyRagdoll,
		
		E_CommandType_Count,
		E_CommandType_Invalid
	};
	
	class C_BlendCycleCommand
	{
	public:
		C_CycleInstance*	cycleInstance;
		F32					time;
		F32					weight;
		U16					fieldMask;
		E_BlendMode			blendMode;
		E_LoopMode			loopMode;
	};

	class C_PushPoseCommand
	{
	public:
		F32					weight;
	};

	class C_PopPoseCommand
	{
	public:
		F32					weight;
		E_BlendMode			blendMode;
	};

	class C_MirrorPoseCommand
	{
	public:
		const C_PoseMirror*		poseMirror;
	};
	
	class C_TransformJointCommand
	{
	public:
		F32		rotation[4];
		F32		translation[3];
		F32		scale[3];
		S16		jointIndex;
	};

	class C_SetIkTargetFromJointCommand
	{
	public:
		C_IkChain*	ikChain;
		U32			targetIndex;
		S16			jointIndex;
		U16			setRotation		: 1;
		U16			setPosition		: 1;
		U16			setPoleVector	: 1;
		F32			rotationOffset[4];
		F32			translationOffset[3];
	};

	class C_SetIkTargetWeightsCommand
	{
	public:
		static const U32 kMaxCount = 4;

		C_IkChain*	chains[kMaxCount];
		U32			targets[kMaxCount];
		F32			weights[kMaxCount];
		U16			count;
	};

	class C_EvaluateIkChainCommand
	{
	public:
		C_IkChain*	ikChain;
	};
	
	enum E_Space
	{
		E_Space_Local	= 0,
		E_Space_Model,
		E_Space_World
	};	
	
	class C_CopyOutJointTransformsCommand
	{
	public:
		static const U32 kMaxLocalJointIndices	= 14;
		
		C_JointTransform*	destJointTransforms;
		const S16*			jointIndices;	// Ptr to source joint indices if numJoints > kMaxLocalJointIndices
		U16					numJoints;
		S16					localJointIndices[kMaxLocalJointIndices];
		E_Space				space;
	};
	
	class C_CopyInPoseCommand
	{
	public:
		const C_Pose*			sourcePose;
		const C_Skeleton*		sourceSkeleton;
		const C_PoseMapping*	poseMapping;
		bool					ignoreScale;
	};
	
	class ALIGN16 C_Command
	{
	public:
		U16		type;
		U16		index;
		union
		{
			C_BlendCycleCommand				blendCycle;
			C_PushPoseCommand				pushPose;
			C_PopPoseCommand				popPose;
			C_MirrorPoseCommand				mirrorPose;
			C_SetIkTargetWeightsCommand		setIkTargetWeights;
			C_SetIkTargetFromJointCommand	setIkTargetFromJoint;
			C_EvaluateIkChainCommand		evaluateIkChain;
			C_TransformJointCommand			transformJoint;
			C_CopyOutJointTransformsCommand	copyOutJointTransforms;
			C_CopyInPoseCommand				copyInPose;
		};	
	};
	
	
}

#endif

