#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneMeshRemappableNode.h"
#include <Aftk/AftkUtility.h>

using namespace Aftk;

C_Status C_NativeSceneMeshRemappableNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_SceneMeshRemappableNode* pSceneMeshRemappableNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MESH_REMAPPABLE_NODE);
	}

	//Write out the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneNode::Serialise(
		pDiskWriter, 
		pNativeFtk,
		pSceneMeshRemappableNode, 
		true);
	if (!status) return status;

	//Write out UV offset information.
	*pDiskWriter << (U8)pSceneMeshRemappableNode->GetNumTextureCoordinateSets();
	for (U8 iOffset = 0; iOffset < (U8)pSceneMeshRemappableNode->GetNumTextureCoordinateSets(); iOffset++)
	{
		*pDiskWriter << pSceneMeshRemappableNode->GetTextureCoordinateOffset(iOffset);
	}

	//Write out blind data mapping table.
	*pDiskWriter << (U16)pSceneMeshRemappableNode->GetNumPolygonBlindDataMappings();
	for (U16 iMapping = 0; iMapping < (U16)pSceneMeshRemappableNode->GetNumPolygonBlindDataMappings(); iMapping++)
	{
		const C_SharedResource<C_BlindDataNode>* pResource = 
			pSceneMeshRemappableNode->GetPolygonBlindDataMappingPtr(iMapping);

		if (pResource == 0)
		{
			*pDiskWriter << S32(-1);
		}
		else
		{
			//store the instance of the blind data resource, not its shared resource pointer
			Ut::UtMap<const C_BlindDataResource*, U32>::Iterator bdIt;
				bdIt = pNativeFtk->m_pBlindDataResToIthIndexMapping->Find(pResource);
				if (bdIt == pNativeFtk->m_pBlindDataResToIthIndexMapping->End())
				{
					LOGERROR(g_aftkMainLogger, "Corrupt blind-data iterator [%s].\n",
						pSceneMeshRemappableNode->GetName());
					AFTK_ASSERT(bdIt != pNativeFtk->m_pBlindDataResToIthIndexMapping->End());
					return C_Status::E_Status_Failure;
				}
				S32 ithIndex = bdIt->GetValue();
				*pDiskWriter << ithIndex;
		}

		*pDiskWriter << (U32)pSceneMeshRemappableNode->GetPolygonBlindDataMappingReferenceCount(iMapping);
	}

	//Write out material mapping table.
	*pDiskWriter << (U16)pSceneMeshRemappableNode->GetNumPolygonMaterialMappings();
	for (U16 iMapping = 0; iMapping < (U16)pSceneMeshRemappableNode->GetNumPolygonMaterialMappings(); iMapping++)
	{
		const C_SharedResource<C_MaterialNode>* pResource = 
			pSceneMeshRemappableNode->GetPolygonMaterialMappingPtr(iMapping);

		if (pResource == 0)
		{
			*pDiskWriter << S32(-1);
		}
		else
		{
			//store the instance of the material resource, not its shared resource pointer
			*pDiskWriter << pResource->GetIndex();
		}

		*pDiskWriter << (U32)pSceneMeshRemappableNode->GetPolygonMaterialMappingReferenceCount(iMapping);
	}

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneMeshRemappableNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneMeshRemappableNode* pSceneMeshRemappableNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MESH_REMAPPABLE_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find remappable mesh node block.");
		}
		pDiskReader->EnterBlock();
	}
	//Read in the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneNode::Deserialise(
		pDiskReader, 
		pNativeFtk, 
		pSceneMeshRemappableNode, 
		true);
	if (!status) return status;

	//Read in UV offset information.
	U8 numMappings;
	*pDiskReader >> numMappings;
//	pSceneMeshRemappableNode->SetTextureCoordinateOffsetSize(numMappings);
	for (U8 iOffset = 0; iOffset < numMappings; iOffset++)
	{
		TextureCoordinate4 offset;
		*pDiskReader >> offset;
		pSceneMeshRemappableNode->SetTextureCoordinateOffset(iOffset, offset);
	}

	//Read in blind data mapping table.
	U16 numBlindDataMappings;
	*pDiskReader >> numBlindDataMappings;
	for (U16 iMapping = 0; iMapping < numBlindDataMappings; iMapping++)
	{
		//Fetch the instance of the blind data resource, not its shared resource pointer
		S32 blindDataIndex;
		*pDiskReader >> blindDataIndex;

		//Fetch the blind data corresponding to this instance
		const C_SharedResource<C_BlindDataNode>* pBlindDataResource = 
			pNativeFtk->m_blindDataMappingArray[blindDataIndex];
		
		if (pBlindDataResource == 0)
			LOGWARN(g_aftkMainLogger, "Could not fetch blind-data resource for [/%s].\n",
				pSceneMeshRemappableNode->GetName());

		U32 refCount;
		*pDiskReader >> refCount;

		// Store our newly acquired blind data resource pointer.
		S16 tableIndex = pSceneMeshRemappableNode->AddPolygonBlindDataMappingSetCount(
			pBlindDataResource, 
			refCount);
	}

	//Read in material mapping table.
	U16 numMaterialMappings;
	*pDiskReader >> numMaterialMappings;
	for (U16 iMapping = 0; iMapping < numMaterialMappings; iMapping++)
	{
		//Fetch the instance of the material resource, not its shared resource pointer
		S32 materialIndex;
		*pDiskReader >> materialIndex;

		//A container for a material resource
		const C_SharedResource<C_MaterialNode>* pMaterialResource;
		
		//Fetch the material corresponding to this instance
		pMaterialResource = 
			pNativeFtk->GetScenePtr()->GetMaterialResourcePtr((U32)materialIndex);
		
		if (pMaterialResource == 0)
			LOGWARN(g_aftkMainLogger, "Could not fetch material resource for [/%s].\n", 
				pSceneMeshRemappableNode->GetName());

		U32 refCount;
		*pDiskReader >> refCount;

		// Store our newly acquired material resource pointer.
		S16 tableIndex = pSceneMeshRemappableNode->AddPolygonMaterialMappingSetCount(
			pMaterialResource, 
			refCount);
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}


