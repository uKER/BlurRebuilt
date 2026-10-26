#include "stdafx.h"


#include <misc_shared/DataStructure/StringTable.h>

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/SceneDescription.h>
	

// C_DataSceneDescription

C_DataSceneDescription::C_DataSceneDescription()
{
}

C_DataSceneDescription::~C_DataSceneDescription()
{
}

const Util::C_String C_DataSceneDescription::GetName() const
{
	return Util::C_String("C_DataSceneDescription");
}

ModelData::C_ModelData& C_DataSceneDescription::GetModelData()
{
	return m_ModelData;
}

const ModelData::C_ModelData& C_DataSceneDescription::GetModelData() const
{
	return m_ModelData;
}

const bool C_DataSceneDescription::ExportModelData( Sys::I_File* pFile ) const
{
	return GetModelData().Export( pFile );
}

/*
bool C_DataSceneDescription::ExportElementData( Sys::I_File* pFile ) const
{
	const SceneDesc::C_SceneDesc& sceneDesc = GetSceneDesc();
	const SceneDesc::C_Group& elementsGroup = sceneDesc.GetGroup( E_SceneDescGroup_Elements );

	// Build String Table
	Util::C_StringTableBuilder stringTableBuilder;

	for( SceneDesc::C_GroupNodeConstIterator i( &elementsGroup ); (const SceneDesc::C_GroupNode*)i; ++i)
	{
		const SceneDesc::C_GroupNode* pCurrNode = i;

		C_SceneDescUserMeshData* pUserData = (C_SceneDescUserMeshData*)pCurrNode->GetUserData();
		pUserData->m_NameId = stringTableBuilder.AddString( pCurrNode->GetName() );
	}


	// Header Block
	{
		Sys::C_FileBlock headerBlock( pFile, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		const S32 elementCount = elementsGroup.GetNodeCount();

		ar << elementCount;
	}
	
	// String table block
	{
		Sys::C_FileBlock stringTableBlock( pFile, "StrTab" );
		Sys::BinarySerialiserAutoEndian ar( stringTableBlock );

		Util::C_StringTable stringTable( stringTableBuilder );

		ar << stringTable;
	}

	// Elements Block
	{
		Sys::C_FileBlock elementsBlock( pFile, "Element" );
		Sys::BinarySerialiserAutoEndian ar( elementsBlock );

		for( SceneDesc::C_GroupNodeConstIterator i( &elementsGroup ); (const SceneDesc::C_GroupNode*)i; ++i)
		{
			const SceneDesc::C_GroupNode* pCurrNode = i;
			C_SceneDescUserMeshData* pUserData = (C_SceneDescUserMeshData*)pCurrNode->GetUserData();

			// Export Matrix
			ar << pCurrNode->GetMatrix();

			// Export Name Id
			ar << pUserData->m_NameId;
			pUserData->m_NameId = stringTableBuilder.AddString( pCurrNode->GetName() );

			// Export Index
			ar << pCurrNode->GetIndex();

			// Export Hierarchy Information		
			ar << GetExportIndex( pCurrNode->GetParent() );
			ar << GetExportIndex( pCurrNode->GetFirstChild() );
			ar << GetExportIndex( pCurrNode->GetNextSibling() );

			// Export Render Mesh Id
			ar << pUserData->m_RenderMeshId;

			// Export Physics Shape Id
			ar << pUserData->m_PhysicsShapeId;
		}
	}	

	return true;
}

*/