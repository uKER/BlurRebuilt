#ifndef ANIMATION_CYCLEMAPPING_INL
#define ANIMATION_CYCLEMAPPING_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	inline U32 C_CycleMapping::GetNumCycleFields() const
	{
		return m_numCycleFields;
	}

	FORCE_INLINE S32 C_CycleMapping::GetPoseFieldIndex( U32 cycleFieldIndex ) const
	{
#ifdef CFG_DEBUG
		BIZ_ASSERT( cycleFieldIndex < GetNumCycleFields() );
#endif
		return m_poseFieldIndices[ cycleFieldIndex ];
	}
	
	inline const C_PoseLayout* C_CycleMapping::GetPoseLayout() const
	{
		return m_poseLayout;
	}
	
	inline const C_Cycle* C_CycleMapping::GetCycle() const
	{
		return m_cycle;
	}
	
	
	inline U32 C_CycleMapping::GetSize() const
	{
		return CalculateSize( m_numCycleFields );
	}

#ifndef PLATFORM_SPU
	inline void* C_CycleMapping::operator new ( size_t size, U32 numCycleFields )
	{
		return BizAlignedMalloc( CalculateSize( numCycleFields ), 16 );
	}

	inline void C_CycleMapping::operator delete ( void* ptr, U32 numCycleFields )
	{
		BizAlignedFree( ptr );
	}
	
	inline void C_CycleMapping::operator delete ( void* ptr )
	{
		BizAlignedFree( ptr );
	}
#endif

	
	/// Get next C_CycleMapping with the same C_Cycle.
	inline C_CycleMapping* C_CycleMapping::GetNextByCycle() const
	{
		return m_nextByCycle;
	}
	
	/// Get next C_CycleMapping with the same C_PoseLayout.
	inline C_CycleMapping* C_CycleMapping::GetNextByPoseLayout() const
	{
		return m_nextByPoseLayout;		
	}
}


#endif

