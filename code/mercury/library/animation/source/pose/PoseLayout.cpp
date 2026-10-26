#include "stdafx.h"
#include <animation/pose/PoseLayout.h>
#include <animation/pose/PoseFormat.h>
#include <animation/skeleton/Skeleton.h>
#include "cycle/CycleMapping.h"
#include <animation/pose/PoseFields.h>
#include <animation/common/JointTransform.h>
#include <animation/common/ObjectTransform.h>

namespace Animation
{

static const U32 kFieldElementSize = 16;		///< Size of a single field element in the pose.

U32 C_PoseLayout::s_fieldSizes[ E_PoseFieldType_Count ] =
{
	sizeof(C_JointTransform),	// JointTransform: rotation, translation, scale.
	sizeof(C_JointTransform),	// Trajectory root delta transform: rotation + translation
	sizeof(C_Vector3Field),		// CustomVector3
	sizeof(C_ScalarField),		// CustomScalar: single value.
	sizeof(C_JointTransform),	// Trajectory root transform.
	sizeof(C_Vector4Field)		// CustomVector4.
};

const C_PoseLayout* C_PoseLayout::s_trajectoryPoseLayout = NULL;

// Fields sorted in order of decreasing size.
E_PoseFieldType C_PoseLayout::s_fieldTypesSortedBySize[ E_PoseFieldType_Count ] =
{
	E_PoseFieldType_JointTransform,
	E_PoseFieldType_TrajectoryDeltaTransform,
	E_PoseFieldType_TrajectoryTransform,
	E_PoseFieldType_CustomVector4,
	E_PoseFieldType_CustomVector3,
	E_PoseFieldType_CustomScalar
};

#ifndef PLATFORM_SPU

U32 gNextUniquePoseLayoutId = 0;

C_PoseLayout::C_PoseLayout( const C_PoseFormat& poseFormat ):
	m_refCount(0),
	m_uniqueId(gNextUniquePoseLayoutId++),
	m_totalFields( poseFormat.GetTotalFields() ),
	m_fieldNames( poseFormat.GetTotalFields() ),
	m_prev( NULL ),
	m_next( NULL ),
	m_mappings( NULL )
{
	U32 offset = 0;
	U32 baseIndex = 0;
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		const E_PoseFieldType fieldType = s_fieldTypesSortedBySize[ i ]; // static_cast<E_PoseFieldType>(i);
		const U32 fieldCount = poseFormat.GetFieldCount( fieldType );
		
		m_fieldCounts[ fieldType ] = fieldCount;
		m_fieldStartIndices[ fieldType ] = baseIndex;
		m_fieldOffsets[ fieldType ] = offset;

		for ( U32 j = 0; j < fieldCount; ++j )
		{
			m_fieldNames[ baseIndex + j ] = poseFormat.GetFieldName( fieldType, j );
		}
		
		baseIndex += fieldCount;
		offset += fieldCount * s_fieldSizes[ fieldType ];
	}
	//LOGINFO(Debug::GetLogger("mercury.animation"), "New pose layout %X\n", m_uniqueId);
}

C_PoseLayout::~C_PoseLayout()
{
	BIZ_STATIC_ASSERT((sizeof(s_fieldSizes)/sizeof(s_fieldSizes[0])) == E_PoseFieldType_Count);

	//LOGINFO(Debug::GetLogger("mercury.animation"), "Deleted pose layout %X\n", m_uniqueId);
	BIZ_ASSERT( m_refCount == 0 );
	C_CycleMapping* mapping = m_mappings;
	while ( mapping  )
	{
		C_CycleMapping* next = mapping->GetNextByPoseLayout();
		mapping->Invalidate();
		mapping = next;
	}
	BIZ_ASSERT( m_mappings == 0 ); // should have been done automatically
	m_mappings = 0;
	
	Unlink();
}

// Called NonSkeleton since in the future we could create sublists
// attached to skeletons to speed up searches and this would be
// the list for any pose formats without associated skeletons.
C_PoseLayout* gNonSkeletonPoseLayoutList = NULL;

const C_PoseLayout* C_PoseLayout::CreatePoseLayout( const C_PoseFormat& key )
{
	C_PoseLayout* layout = gNonSkeletonPoseLayoutList;
	while ( layout )
	{
		if ( *layout == key )
		{
			// Return existing layout.
			return layout;
		}
		layout = layout->GetNext();
	}	
	
	// No matching layout found - create a new one.
	layout = new C_PoseLayout( key );
	layout->Link();
	
	return layout;
}

const C_PoseLayout* C_PoseLayout::GetTrajectoryPoseLayout()
{
	if (s_trajectoryPoseLayout == NULL)
	{
		C_PoseFormat poseFormat;
		poseFormat.AddTrajectoryTransformField();
		
		s_trajectoryPoseLayout = CreatePoseLayout(poseFormat);
	}
	return s_trajectoryPoseLayout;
}


//
// Reference counting.
//
void C_PoseLayout::AddRef() const
{
	++m_refCount;
}

bool C_PoseLayout::DecRef() const
{
	BIZ_ASSERT( m_refCount > 0 );
	bool deleted = false;
	if ( 0 == --m_refCount )
	{
		delete this;
		deleted = true;
	}
	return deleted;
}

#endif

U32 C_PoseLayout::GetPoseDataSize() const
{
	U32 size = 0;
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		//size = Btl::AlignUp( size, 16 );
		size += m_fieldCounts[ i ] * s_fieldSizes[ i ];
	}
	return size;
}

U32 C_PoseLayout::GetTotalFieldCount() const
{
	U32 count = 0;
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		count += m_fieldCounts[i];
	}
	return count;
}

#ifndef PLATFORM_SPU

S32 C_PoseLayout::FindFieldByName( E_PoseFieldType fieldType, const C_StringId& name ) const
{
	S32 result = -1;
	if (name.IsValid())
	{
		U32 startIndex = m_fieldStartIndices[ fieldType ];
		for ( U32 i = 0, count = m_fieldCounts[ fieldType ]; i < count; ++i )
		{
			if ( m_fieldNames[ startIndex + i ] == name )
			{
				result = i;
				break;
			}
		}
	}
	return result;
}

S32 C_PoseLayout::FindFieldByName( E_PoseFieldType fieldType, const CHAR8* name ) const
{
	S32 result = -1;
	
	C_StringId nameId(name, false);
	
	if (nameId.IsValid())
	{
		U32 startIndex = m_fieldStartIndices[fieldType];
		for ( U32 i = 0, count = m_fieldCounts[fieldType]; i < count; ++i )
		{
			if ( m_fieldNames[startIndex + i] == name )
			{
				result = i;
				break;
			}
		}
	}
	return result;
}

void C_PoseLayout::Link()
{
	LinkTo( gNonSkeletonPoseLayoutList );
}

void C_PoseLayout::LinkTo( C_PoseLayout*& head )
{
	m_prev = NULL;
	m_next = head;
	if ( head )
	{
		head->m_prev = this;
	}
	head = this;
}

void C_PoseLayout::Unlink()
{
	UnlinkFrom( gNonSkeletonPoseLayoutList );
	
	if (s_trajectoryPoseLayout == this)
	{
		s_trajectoryPoseLayout = NULL;
	}
}

void C_PoseLayout::UnlinkFrom( C_PoseLayout*& head )
{
	if ( head == this )
	{
		head = m_next;
	}
	if ( m_next )
	{
		m_next->m_prev = m_prev;
	}
	if ( m_prev )
	{
		m_prev->m_next = m_next;
	}
}

C_PoseLayout* C_PoseLayout::GetNext() const
{
	return m_next;
}

bool C_PoseLayout::operator == ( const C_PoseLayout& rhs ) const
{
	if ( m_totalFields != rhs.m_totalFields )
	{
		return false;
	}
	
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		if ( m_fieldCounts[ i ] != rhs.m_fieldCounts[ i ] )
		{
			return false;
		}
		
		const U32 baseIndex = m_fieldStartIndices[ i ];
		
		for ( U32 j = 0, count = m_fieldCounts[ i ]; j < count; ++j )
		{
			if ( m_fieldNames[ baseIndex + j ] != rhs.m_fieldNames[ baseIndex + j ] )
			{
				return false;
			}
		}
	}
	
	return true;
}

bool C_PoseLayout::operator == ( const C_PoseFormat& rhs ) const
{
	if ( m_totalFields != rhs.GetTotalFields() )
	{
		return false;
	}
	
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		const E_PoseFieldType fieldType = static_cast<E_PoseFieldType>(i);
		if ( m_fieldCounts[ i ] != rhs.GetFieldCount( fieldType ) )
		{
			return false;
		}
		
		const U32 baseIndex = m_fieldStartIndices[ i ];
		
		for ( U32 j = 0, count = m_fieldCounts[ i ]; j < count; ++j )
		{
			if ( m_fieldNames[ baseIndex + j ] != rhs.GetFieldName( fieldType, j ) )
			{
				return false;
			}
		}
	}
	
	return true;
}

#endif

} // namespace Animation
