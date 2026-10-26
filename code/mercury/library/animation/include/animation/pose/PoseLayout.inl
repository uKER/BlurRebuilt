#ifndef ANIMATION_POSELAYOUT_INL
#define ANIMATION_POSELAYOUT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{

	FORCE_INLINE U32 C_PoseLayout::GetFieldOffset( E_PoseFieldType fieldType, U32 index ) const
	{
#ifdef CFG_DEBUG
		BIZ_ASSERT( index <= m_fieldCounts[ fieldType ] ); // allow off by one
#endif
		return m_fieldOffsets[ fieldType ] + index * s_fieldSizes[ fieldType ];
	}

	FORCE_INLINE U32 C_PoseLayout::GetFirstFieldOffset( E_PoseFieldType fieldType ) const
	{
		return m_fieldOffsets[ fieldType ];
	}
	
	inline U32 C_PoseLayout::GetFieldCount( E_PoseFieldType fieldType ) const
	{
		return m_fieldCounts[ fieldType ];
	}
	
	inline U32 C_PoseLayout::GetFieldSize( E_PoseFieldType fieldType )
	{
		return s_fieldSizes[ fieldType ];
	}
	
	inline U32 C_PoseLayout::GetUniqueId() const
	{
		return m_uniqueId;
	}
	
#ifndef PLATFORM_SPU
	
	inline const C_StringId& C_PoseLayout::GetFieldName( E_PoseFieldType fieldType, U32 index ) const
	{
		BIZ_ASSERT( index < m_fieldCounts[ fieldType ] );
		return m_fieldNames[ m_fieldStartIndices[ fieldType ] + index ];
	}
	
#endif
}

#endif
