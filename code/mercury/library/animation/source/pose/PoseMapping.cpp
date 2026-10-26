#include "stdafx.h"
#include <animation/pose/PoseMapping.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseLayout.h>
#include <btl/utility/string.h>

/*

Notes:
	- PoseMappings could be linked with their source C_PoseLayout and shared to save memory
	- However, PoseMappings will only be used rarely, so this probably isn't worth the effort
	- Could extend (or complement) the PoseMapping class with a class that knows about differences between skeletons
	  and can calculate sensible rotations for destination joints that have no corresponding joints in the source pose
	  e.g. spine joints
	  Not sure if this would ever actually be needed, though.
*/

namespace Animation
{

C_PoseMapping::C_PoseMapping():
	m_sourcePoseLayout(NULL),
	m_destPoseLayout(NULL),
	m_numMappedFields(0)
{
}

C_PoseMapping::C_PoseMapping(const C_Pose& sourcePose, const C_Pose& destPose, const CHAR8* namespacePrefix):
	m_sourcePoseLayout(NULL),
	m_destPoseLayout(NULL),
	m_numMappedFields(0)
{
	Create(sourcePose, destPose, namespacePrefix);
}


C_PoseMapping::~C_PoseMapping()
{
	Destroy();
}

void C_PoseMapping::Create(const C_Pose& sourcePose, const C_Pose& destPose, const CHAR8* namespacePrefix)
{
	const C_PoseLayout* sourcePoseLayout = sourcePose.GetPoseLayout();
	const C_PoseLayout* destPoseLayout = destPose.GetPoseLayout();
	
	if (sourcePoseLayout != NULL && destPoseLayout != NULL)
	{
		Create(*sourcePoseLayout, *destPoseLayout, namespacePrefix);
	}
}
	
void C_PoseMapping::Create(const C_PoseLayout& sourcePoseLayout, const C_PoseLayout& destPoseLayout, const CHAR8* namespacePrefix)
{
	sourcePoseLayout.AddRef();
	destPoseLayout.AddRef();
	
	m_sourcePoseLayout = &sourcePoseLayout;
	m_destPoseLayout = &destPoseLayout;
	
	// Mapping defines a source pose field index for each destination pose field index.
	U32 destTotalFieldCount = destPoseLayout.GetTotalFieldCount();
	m_sourceFieldIndices.Resize(destTotalFieldCount);
	
	// For each field in the destination pose layout, find a corresponding field in the source one.
	// If performance becomes an issue, a temporary hash table might help here.
	
	U32 offset = 0;
	Btl::String128 prefixString;
	Btl::String128 nameWithNamespace;
	
	if (namespacePrefix != NULL)
	{
		prefixString = namespacePrefix;
	}
	
	U32 numMappedFields = 0;

	for (U32 fieldTypeIndex = 0; fieldTypeIndex < E_PoseFieldType_Count; ++fieldTypeIndex)
	{
		E_PoseFieldType fieldType = static_cast<E_PoseFieldType>(fieldTypeIndex);
		U32 destFieldCount = destPoseLayout.GetFieldCount(fieldType);
		m_fieldTypeStartIndex[fieldTypeIndex] = offset;
		for (U32 destFieldIndex = 0; destFieldIndex < destFieldCount; ++destFieldIndex)
		{
			C_StringId destFieldName = destPoseLayout.GetFieldName(fieldType, destFieldIndex);
			S32 sourceFieldIndex;
			
			if (prefixString.Empty())
			{
				sourceFieldIndex = sourcePoseLayout.FindFieldByName(fieldType, destFieldName);
			}
			else
			{
				nameWithNamespace = prefixString;
				nameWithNamespace += destFieldName.GetString();
				sourceFieldIndex = sourcePoseLayout.FindFieldByName(fieldType, nameWithNamespace.Chars());
			}
			m_sourceFieldIndices[destFieldIndex + offset] = sourceFieldIndex;
			
			if (sourceFieldIndex >= 0)
			{
				++numMappedFields;
			}
		}
		offset += destFieldCount;
	}
	BIZ_ASSERT(offset == destTotalFieldCount);
	m_numMappedFields = numMappedFields;
}

void C_PoseMapping::Destroy()
{
	if (m_sourcePoseLayout)
	{
		m_sourcePoseLayout->DecRef();
		m_sourcePoseLayout = NULL;
	}
	
	if (m_destPoseLayout)
	{
		m_destPoseLayout->DecRef();
		m_destPoseLayout = NULL;
	}
	
	m_sourceFieldIndices.Clear();
}

bool C_PoseMapping::IsEmpty() const
{
	return 0 == m_numMappedFields;
}

} // end Animation

