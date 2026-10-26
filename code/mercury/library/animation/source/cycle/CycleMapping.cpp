#include "stdafx.h"
#include "CycleMapping.h"
#include <animation/cycle/Cycle.h>
#include <animation/common/StringUtil.h>
#include <animation/common/Stats.h>
#include <animation/pose/PoseLayout.h>
#include <system/scheduler.h>
#include <btl/utility/types.h>

namespace Animation
{

#ifndef PLATFORM_SPU

C_CycleMapping::C_CycleMapping( const C_Cycle& cycle, const C_PoseLayout& poseLayout ):
	m_cycle( &cycle ),
	m_poseLayout( &poseLayout ),
	m_prevByCycle( NULL ),
	m_nextByCycle( NULL ),
	m_prevByPoseLayout( NULL ),
	m_nextByPoseLayout( NULL ),
	m_prevInCache(0),
	m_nextInCache(0),
	m_refCount(1)
{
	const U32 numCycleFields = cycle.GetNumFields();
	BIZ_ASSERT( numCycleFields < 65536 );
	m_numCycleFields = numCycleFields;

	for ( U32 i = 0; i < numCycleFields; ++i )
	{
		const E_PoseFieldType fieldType = cycle.GetFieldType( i );
		const C_StringId& fieldName = cycle.GetFieldName( i );
		m_poseFieldIndices[ i ] = poseLayout.FindFieldByName( fieldType, fieldName );
	}

	// Link into cycle and pose layout mapping lists.
	Link();

#ifdef USER_CCOOKSON
	LOGINFO( Debug::GetLogger("mercury.animation"), "Created cycle mapping (%d bytes): %X\n", CalculateSize( m_numCycleFields ), this );
#endif

	ANIMATION_STATS_UPDATE(numActiveCycleMappings++);
	ANIMATION_STATS_UPDATE(memoryUsedByCycleMappings += GetSize());
}

C_CycleMapping::~C_CycleMapping()
{
	BIZ_ASSERT(m_refCount == 0);

	if (m_cycle)
	{
		// Remove from cycle and pose layout mapping lists.
		Unlink();
	}

	if (!IsValid())
	{
		ANIMATION_STATS_UPDATE(numInvalidCycleMappings--);
	}
	ANIMATION_STATS_UPDATE(memoryUsedByCycleMappings -= GetSize());

#if USER_CCOOKSON
	LOGINFO( Debug::GetLogger("mercury.animation"), "Destroyed cycle mapping (%d bytes): %X\n", CalculateSize( m_numCycleFields ), this );
#endif
}

void C_CycleMapping::AddRef() const
{
	if (0 == m_refCount)
	{
		s_cache.Remove(this);

		ANIMATION_STATS_UPDATE(numActiveCycleMappings++);
	}
	++m_refCount;
}

void C_CycleMapping::DecRef() const
{
	BIZ_ASSERT(m_refCount > 0);
	--m_refCount;
	if (m_refCount == 0)
	{
		ANIMATION_STATS_UPDATE(numActiveCycleMappings--);
		if (IsValid())
		{
			s_cache.Insert(this);
		}
		else
		{
			// Never keep invalid mappings around.
			delete this;
		}
	}
}

void C_CycleMapping::Invalidate()
{
	Unlink();

	m_cycle = 0;
	m_poseLayout = 0;

	ANIMATION_STATS_UPDATE(numInvalidCycleMappings++);

	if (m_refCount == 0)
	{
		s_cache.Remove(this);
		delete this;
	}
}

void C_CycleMapping::Unlink()
{
	if (!m_cycle) return;

	if ( m_cycle->GetMappingListHead() == this )
	{
		m_cycle->SetMappingListHead( m_nextByCycle );
	}
	
	if ( m_nextByCycle )
	{
		m_nextByCycle->m_prevByCycle = m_prevByCycle;
	}
	
	if ( m_prevByCycle )
	{
		m_prevByCycle->m_nextByCycle = m_nextByCycle;
	}

	m_prevByCycle = NULL;
	m_nextByCycle = NULL;
	
	if ( m_poseLayout->GetMappingListHead() == this )
	{
		m_poseLayout->SetMappingListHead( m_nextByPoseLayout );
	}
	
	if ( m_nextByPoseLayout )
	{
		m_nextByPoseLayout->m_prevByPoseLayout = m_prevByPoseLayout;
	}
	
	if ( m_prevByPoseLayout )
	{
		m_prevByPoseLayout->m_nextByPoseLayout = m_nextByPoseLayout;
	}
	
	m_prevByPoseLayout = NULL;
	m_nextByPoseLayout = NULL;
}

void C_CycleMapping::Link()
{
	C_CycleMapping* cycleRoot = m_cycle->GetMappingListHead();
	m_cycle->SetMappingListHead( this );
	m_nextByCycle = cycleRoot;
	m_prevByCycle = NULL;
	if ( cycleRoot )
	{
		cycleRoot->m_prevByCycle = this;
	}
	
	C_CycleMapping* layoutRoot = m_poseLayout->GetMappingListHead();
	m_poseLayout->SetMappingListHead( this );
	m_nextByPoseLayout = layoutRoot;
	m_prevByPoseLayout = NULL;
	if ( layoutRoot )
	{
		layoutRoot->m_prevByPoseLayout = this;
	}
}

#endif

U32 C_CycleMapping::CalculateSize( U32 numCycleFields )
{
	U32 clampedNumCycleFields = Maths::Max( numCycleFields, 1U );

	return Btl::AlignUp( U32( sizeof( C_CycleMapping ) + ( clampedNumCycleFields - 1 ) * sizeof( S16 ) ), 16u ); // [Blur2PC][x64] size_t is 64-bit
}

//
// C_CycleMapping::C_Cache
//
// A cache storing recently created, but no longer referenced, C_CycleMappings. 
//
#ifndef PLATFORM_SPU
C_CycleMapping::C_Cache C_CycleMapping::s_cache(8192);

C_CycleMapping::C_Cache::C_Cache(U32 memoryLimit):
	m_memoryUsed(0),
	m_memoryLimit(memoryLimit),
	m_numEntries(0),
	m_front(0),
	m_back(0)
{
	ANIMATION_STATS_UPDATE(cycleMappingCacheMemoryLimit = memoryLimit);
}

C_CycleMapping::C_Cache::~C_Cache()
{
	while (!IsEmpty())
	{
		PurgeFront();
	}
}

void C_CycleMapping::C_Cache::Insert(const C_CycleMapping* mapping)
{
	if (!mapping) return;

	U32 mappingSize = mapping->GetSize();

	if (mappingSize > m_memoryLimit)
	{
		// Delete mappings too large to be considered for caching.
		delete mapping;
		return;
	}

	MakeSpace(mappingSize);

	if (IsEmpty())
	{
		m_front = m_back = mapping;
		mapping->m_nextInCache = mapping->m_prevInCache = 0;
	}
	else
	{
		// Add to back.
		m_back->m_nextInCache = mapping;
		mapping->m_nextInCache = 0;
		mapping->m_prevInCache = m_back;
		m_back = mapping;
	}
	++m_numEntries;
	m_memoryUsed += mappingSize;
	ANIMATION_STATS_UPDATE(numCachedCycleMappings = m_numEntries);
	ANIMATION_STATS_UPDATE(cycleMappingCacheMemoryUsed = m_memoryUsed);
}

// Remove item located at any point in list.
void C_CycleMapping::C_Cache::Remove(const C_CycleMapping* mapping)
{
	BIZ_ASSERT(!IsEmpty());

	// Update front and back pointers.
	if (mapping == m_back)
	{
		m_back = mapping->m_prevInCache;
	}
	if (mapping == m_front)
	{
		m_front = mapping->m_nextInCache;
	}

	// Update neighbours.
	if (mapping->m_prevInCache)
	{
		mapping->m_prevInCache->m_nextInCache = mapping->m_nextInCache;
	}
	if (mapping->m_nextInCache)
	{
		mapping->m_nextInCache->m_prevInCache = mapping->m_prevInCache;
	}

	mapping->m_nextInCache = mapping->m_prevInCache = 0;

	BIZ_ASSERT(mapping->GetSize() <= m_memoryUsed);
	m_memoryUsed -= mapping->GetSize();
	--m_numEntries;
	ANIMATION_STATS_UPDATE(numCachedCycleMappings = m_numEntries);
	ANIMATION_STATS_UPDATE(cycleMappingCacheMemoryUsed = m_memoryUsed);
}

void C_CycleMapping::C_Cache::MakeSpace(U32 requiredSpace)
{
	while (m_memoryUsed + requiredSpace > m_memoryLimit)
	{
		PurgeFront();
	}
}

bool C_CycleMapping::C_Cache::IsEmpty() const
{
	return !m_front;
}

void C_CycleMapping::C_Cache::PurgeFront()
{
	const C_CycleMapping* mapping = PopFront();
	BIZ_ASSERT(mapping->GetSize() <= m_memoryUsed);
	m_memoryUsed -= mapping->GetSize();
	delete mapping;

	ANIMATION_STATS_UPDATE(cycleMappingCacheMemoryUsed = m_memoryUsed);
}

const C_CycleMapping* C_CycleMapping::C_Cache::PopFront()
{
	BIZ_ASSERT(!IsEmpty());
	const C_CycleMapping* m = m_front;
	m_front = m->m_nextInCache;
	m_front->m_prevInCache = 0;

	m->m_prevInCache = 0;
	m->m_nextInCache = 0;
	--m_numEntries;

	ANIMATION_STATS_UPDATE(numCachedCycleMappings = m_numEntries);

	return m;
}

#endif

} // namespace Animation
