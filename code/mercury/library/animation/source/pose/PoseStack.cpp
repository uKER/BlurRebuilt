#include "stdafx.h"
#include <animation/pose/PoseStack.h>
#include <animation/pose/PoseLayout.h>
#include "Pose.inl"

namespace Animation
{

C_PoseStack::C_PoseStack( C_Pose& destPose, U8* scratchBuffer, U32 bufferSize ):
	m_maxDepth(0),
	m_poseLayout( destPose.GetPoseLayout() ),
	m_depth(0),
	m_safeDepth(0)
{
	if ( m_poseLayout )
	{
		// Handle misaligned scratchBuffer.
		U8* alignedScratchBuffer = Btl::AlignUp(scratchBuffer, 16);
		bufferSize -= (alignedScratchBuffer - scratchBuffer);
		
		const U32 poseDataSize = destPose.GetPoseDataSize();
		m_maxDepth = poseDataSize > 0 ? bufferSize / poseDataSize : 0;
		m_maxDepth = Maths::Min( m_maxDepth, kMaxDepth );
		
		// First stack entry is destination pose.
		m_poses[0].CreateInPlace(*m_poseLayout, static_cast<U8*>(destPose.GetPoseData()));
		
		// Subsequent entries allocated from scratch buffer with copies of source pose default values.
		U8* poseDataPtr = alignedScratchBuffer;
		for ( U32 i = 0; i < m_maxDepth; ++i )
		{
			m_poses[i + 1].CreateInPlace(*m_poseLayout, poseDataPtr);
			m_poses[i + 1].CopyDefaultsFrom(destPose);
			poseDataPtr += poseDataSize;
		}
		
		SetLastBlendMode(E_BlendMode_Invalid);
		SetPoseWeight(1.f);
	}
}

C_PoseStack::~C_PoseStack()
{
	BIZ_ASSERT( m_depth == 0 );
}

bool C_PoseStack::Push()
{
	++m_depth;
	m_safeDepth = Maths::Min(m_depth, m_maxDepth);
	SetLastBlendMode( E_BlendMode_Invalid );
	SetPoseWeight( 1.f );
	return m_depth == m_safeDepth;
}

void C_PoseStack::Pop()
{
	if ( m_depth > 0 )
	{
		--m_depth;
	}
	else
	{
		BIZ_ASSERT( !"Pose stack underflow" );
	}
	m_safeDepth = Maths::Min(m_depth, m_maxDepth);
}

C_Pose& C_PoseStack::GetTop( U32 offset )
{
	S32 index = m_safeDepth - offset;
	BIZ_ASSERT( index >= 0 && "Tried to index below base of pose stack" );
	return m_poses[ index ];
}

void C_PoseStack::SetLastBlendMode( E_BlendMode blendMode )
{
	m_lastBlendMode[ m_safeDepth ] = blendMode;
}

E_BlendMode C_PoseStack::GetLastBlendMode( U32 offset ) const
{
	S32 index = m_safeDepth - offset;
	BIZ_ASSERT( index >= 0 && "Tried to index below base of pose stack" );
	return static_cast<E_BlendMode>(m_lastBlendMode[ index ]);
}

void C_PoseStack::SetPoseWeight( F32 weight )
{
	m_poseWeights[ m_safeDepth ] = weight;
}

F32 C_PoseStack::GetPoseWeight() const
{
	return m_poseWeights[ m_safeDepth ];
}

U32 C_PoseStack::GetDepth() const
{
	return m_depth;
}

U32 C_PoseStack::GetMaxDepth() const
{
	return m_maxDepth;
}

} // end Animation
