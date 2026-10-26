#ifndef ANIMATION_POSESTACK_H
#define ANIMATION_POSESTACK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/Pose.h>
#include <animation/common/BlendModes.h>

#ifdef PLATFORM_SPU
#include <system/Scheduler.h>	// For Spu::Alloc
#endif

namespace Animation
{


#ifdef PLATFORM_SPU
	#define ANIM_POSE_STACK_SCRATCH_ALLOC(numBytes)		(U8*) Spu::Alloc(numBytes)
#else
	#define ANIM_POSE_STACK_SCRATCH_ALLOC(numBytes)		(U8*) alloca(numBytes)
#endif

	class C_Pose;
	class C_PoseLayout;

	/**
		A stack of C_Poses for layered blending.
	*/
	class C_PoseStack
	{
	public:
		
		C_PoseStack( C_Pose& destPose, U8* scratchBuffer, U32 bufferSize );
		
		~C_PoseStack();
		
		bool Push();
				
		void Pop();
		
		C_Pose& GetTop( U32 offset = 0 );
		
		void SetLastBlendMode( E_BlendMode blendMode );
		E_BlendMode GetLastBlendMode( U32 offset = 0 ) const;
		
		void SetPoseWeight( F32 weight );
		F32 GetPoseWeight() const;
				
		U32 GetDepth() const;
		
		U32 GetMaxDepth() const;
				
	private:
		static const U32 kMaxDepth = 3;
		
		U32						m_maxDepth;					// Maximum depth, not including destination pose.
		C_Pose					m_poses[ kMaxDepth + 1 ];
		const C_PoseLayout*		m_poseLayout;
		U32						m_depth;
		U32						m_safeDepth;
		F32						m_poseWeights[ kMaxDepth + 1 ];
		U8						m_lastBlendMode[ kMaxDepth + 1 ];
	};




}

#endif
