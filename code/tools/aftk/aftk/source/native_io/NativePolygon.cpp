#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativePolygon.h"
#include "NativePolygonVertex.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativePolygon::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const Aftk::C_Polygon* pPolygon,
	const C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_POLYGON);
	}
	{
		//Write out polygon vertex indices.
		*pDiskWriter << (U8)pPolygon->GetNumPolygonVertices();
		for (U32 i = 0; i < pPolygon->GetNumPolygonVertices(); i++)
		{
			*pDiskWriter << (U32)pPolygon->GetPolygonVertexIndex(i);
		}

		AFTK_ASSERT(pPolygon->GetBlindDataMappingIndex() < 
			pSceneMeshNode->GetNumPolygonBlindDataMappings());
		AFTK_ASSERT(pPolygon->GetMaterialMappingIndex() < 
			pSceneMeshNode->GetNumPolygonMaterialMappings());

		//Write out blind data
		*pDiskWriter << (S16)pPolygon->GetBlindDataMappingIndex();
		//Write out material
		*pDiskWriter << (S16)pPolygon->GetMaterialMappingIndex();

		//Write out what polygon this originally came from
		*pDiskWriter << pPolygon->GetBasePolygonIndex();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativePolygon::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	Aftk::C_Polygon* pPolygon,
	C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_POLYGON))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find polygon block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in polygon vertices
		U8 numPolygonVertexIndices;
		*pDiskReader >> numPolygonVertexIndices;

		for (U32 i = 0; i < numPolygonVertexIndices; i++)
		{
			//Read polygon vertex index.
			U32 index;
			*pDiskReader >> index;
			pPolygon->AddPolygonVertexIndex(index);
		}


		//The 'hard' part of the resource processing is now handled at the mesh level, 
		//As of V143 and above. In other words the reference counting is handled by the
		//mesh.
		{
			*pDiskReader >> pPolygon->m_blindDataMappingIndex;
			AFTK_ASSERT(pPolygon->m_blindDataMappingIndex < pSceneMeshNode->GetNumPolygonBlindDataMappings());
			*pDiskReader >> pPolygon->m_materialMappingIndex;
			AFTK_ASSERT(pPolygon->m_materialMappingIndex < pSceneMeshNode->GetNumPolygonMaterialMappings());

			//These two functions used handle the reference counting. Reference counting info
			//is now saved explicitly instead in V143 and above.
		}

		//Read in what polygon this originally came from.
		S32 basePolygonIndex;
		*pDiskReader >> basePolygonIndex;
		pPolygon->SetBasePolygonIndex(basePolygonIndex);
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

