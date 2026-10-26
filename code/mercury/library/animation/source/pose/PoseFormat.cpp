#include "stdafx.h"
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseFieldMask.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseLayout.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/common/StringId.h>
#include <animation/cycle/Cycle.h>
#include <btl/utility/string.h>

namespace Animation
{

C_PoseFormat::C_PoseFormat():
	m_skeleton(0),
	m_totalFields(0)
{
}

C_PoseFormat::C_PoseFormat( const C_Skeleton* skeleton ):
	m_totalFields(0)
{
	SetSkeleton( skeleton );
}

C_PoseFormat::C_PoseFormat( const C_Cycle* cycle ):
	m_skeleton(0),
	m_totalFields(0)
{
	AddCycleFields( cycle );
}

void C_PoseFormat::Clear()
{
	for ( U32 i = 0; i < E_PoseFieldType_Count; ++i )
	{
		m_fieldNames[ i ].Clear();
	}
	m_totalFields = 0;
}

void C_PoseFormat::SetSkeleton( const C_Skeleton* skeleton )
{
	if ( NULL != skeleton )
	{
		const Animation::E_PoseFieldType transformFieldType = E_PoseFieldType_JointTransform;
		const U32 numJoints = skeleton->GetNumJoints();

		// Add trajectory root fields.
		AddTrajectoryTransformField();
		AddTrajectoryDeltaTransformField();
		
		// Add joint transform fields.
		FieldNameVector& fieldNames = m_fieldNames[transformFieldType];
		
		m_totalFields -= fieldNames.Size();
		fieldNames.Resize(numJoints);
		
		m_totalFields += numJoints;
				
		for ( U32 i = 0; i < numJoints; ++i )
		{
			fieldNames[i] = skeleton->GetJointName(i);
		}
	}
	
	m_skeleton = skeleton;
}

U32 C_PoseFormat::AddCustomScalarField( const CHAR8* name )
{
	return AddField( E_PoseFieldType_CustomScalar, C_StringId( name ) );
}

U32 C_PoseFormat::AddCustomVector3Field( const CHAR8* name )
{
	return AddField( E_PoseFieldType_CustomVector3, C_StringId( name ) );
}

U32 C_PoseFormat::AddCustomVector4Field( const CHAR8* name )
{
	return AddField( E_PoseFieldType_CustomVector4, C_StringId( name ) );
}

void C_PoseFormat::AddCycleFields( const C_Cycle* cycle )
{
	if ( cycle != NULL )
	{
		U32 fieldSearchCounts[Animation::E_PoseFieldType_Count];
		for (U32 j = 0; j < E_PoseFieldType_Count; ++j)
		{
			fieldSearchCounts[j] = m_fieldNames[j].Size();
		}	
		for ( U32 i = 0, count = cycle->GetNumFields(); i < count; ++i )
		{
			Animation::E_PoseFieldType fieldType = cycle->GetFieldType(i);
			AddUniqueField( fieldType, cycle->GetFieldName(i), fieldSearchCounts[fieldType]);
		}
	}
}

void C_PoseFormat::AddCycleFields( const C_Cycle* cycle, const C_PoseFieldMask& poseFieldMask )
{
	if ( cycle != NULL )
	{
		U32 fieldSearchCounts[Animation::E_PoseFieldType_Count];
		for (U32 j = 0; j < E_PoseFieldType_Count; ++j)
		{
			fieldSearchCounts[j] = m_fieldNames[j].Size();
		}
	
		for ( U32 i = 0, count = cycle->GetNumFields(); i < count; ++i )
		{
			const Animation::E_PoseFieldType cycleFieldType = cycle->GetFieldType(i);
			if (poseFieldMask.IsEnabled(cycleFieldType))
			{
				AddUniqueField(cycleFieldType, cycle->GetFieldName(i), fieldSearchCounts[cycleFieldType]);
			}
		}
	}
}

void C_PoseFormat::AddPoseFields( const C_Pose* pose, const C_PoseFieldMask& poseFieldMask )
{
	if ( pose != NULL )
	{
		for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
		{
			U32 searchCount = m_fieldNames[i].Size();
			Animation::E_PoseFieldType fieldType = static_cast<Animation::E_PoseFieldType>(i);
			if (poseFieldMask.IsEnabled(fieldType))
			{
				for (U32 j = 0, count = pose->GetFieldCount(fieldType); j < count; ++j)
				{
					Animation::C_StringId fieldName = pose->GetFieldName(fieldType, j);
					AddUniqueField(fieldType, pose->GetFieldName(fieldType, j), searchCount);
				}				
			}
		}
	}
}

void C_PoseFormat::AddPoseFieldsWithNamespace(const C_Pose* pose, const C_PoseFieldMask& poseFieldMask, const CHAR8* namespacePrefix)
{
	if (!pose)
	{
		return;
	}
	if (!namespacePrefix)
	{
		AddPoseFields(pose, poseFieldMask);
		return;
	}
	if (*namespacePrefix == 0)
	{
		AddPoseFields(pose, poseFieldMask);
		return;
	}

	Btl::String128 fieldName;
	Btl::String128 prefix = namespacePrefix;
	for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
	{
		U32 searchCount = m_fieldNames[i].Size();
		Animation::E_PoseFieldType fieldType = static_cast<Animation::E_PoseFieldType>(i);
		if (poseFieldMask.IsEnabled(fieldType))
		{
			for (U32 j = 0, count = pose->GetFieldCount(fieldType); j < count; ++j)
			{
				fieldName = pose->GetFieldName(fieldType, j).GetString();
				if (fieldName.Search(prefix) == 0)
				{
					// Strip prefix to make field name.
					fieldName = fieldName(prefix.Length(), fieldName.Length());
					AddUniqueField(fieldType, fieldName.Chars(), searchCount);
				}
			}				
		}
	}
}

void C_PoseFormat::AddTrajectoryTransformField()
{
	AddField( E_PoseFieldType_TrajectoryTransform, C_StringId( "$trajectory_root" ) );
}

void C_PoseFormat::AddTrajectoryDeltaTransformField()
{
	AddField( E_PoseFieldType_TrajectoryDeltaTransform, C_StringId( "$trajectory_root_delta" ) );
}

U32 C_PoseFormat::GetTotalFields() const
{
	return m_totalFields;
}

U32 C_PoseFormat::GetFieldCount( E_PoseFieldType fieldType ) const
{
	return m_fieldNames[ fieldType ].Size();
}

const C_StringId& C_PoseFormat::GetFieldName( E_PoseFieldType fieldType, U32 index ) const
{
	return (m_fieldNames[ fieldType ])[ index ];
}

U32 C_PoseFormat::GetFieldSize( E_PoseFieldType fieldType ) const
{
	return C_PoseLayout::GetFieldSize( fieldType );
}

U32 C_PoseFormat::AddField( E_PoseFieldType fieldType, const C_StringId& name )
{
	return AddUniqueField(fieldType, name, m_fieldNames[fieldType].Size());
}

S32 C_PoseFormat::FindFieldByName( E_PoseFieldType fieldType, const CHAR8* name ) const
{
	// If name has not been converted to a C_StringId before then it cannot have been used
	// in this (or any other) C_PoseFormat. Therefore we can early out and avoid polluting
	// the animation string table.
	Animation::C_StringId nameId(name, false);
	if (nameId.IsNull())
	{
		return -1;
	}
	
	const FieldNameVector& fieldNames = m_fieldNames[fieldType];
	
	for (U32 i = 0, count = fieldNames.Size(); i < count; ++i)
	{
		if (fieldNames[i] == nameId)
		{
			return i;
		}
	}
	return -1;
}

U32 C_PoseFormat::AddUniqueField(E_PoseFieldType fieldType, const C_StringId& name, S32 searchCount)
{
	S32 index = -1;
	Btl::Vector<Animation::C_StringId>& fieldNames = m_fieldNames[fieldType];
	for (S32 i = 0; i < searchCount; ++i)
	{
		if (fieldNames[i] == name)
		{
			LOGDEBUG(Debug::GetLogger("mercury.animation"), "Duplicate field '%s'", fieldNames[i].GetString());
			return i;
		}
	}
	++m_totalFields;
	fieldNames.PushBack( name );
	return fieldNames.Size() - 1;
}


} // end Animation
