#include "stdafx.h"
#include "../aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneMeshRemappableNode.h"
#include "NativeSceneMeshBonedInstanceNode.h"
#include "NativeSceneMeshRemappedInstanceNode.h"
#include <aftk/AftkUtility.h>

using namespace Aftk;

C_Status C_NativeSceneMeshBonedInstanceNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_SceneMeshBonedInstanceNode* pSceneMeshBonedInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MESH_BONED_INSTANCE_NODE);
	}

	//Write out the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneMeshRemappableNode::Serialise(
		pDiskWriter, 
		pNativeFtk,
		pSceneMeshBonedInstanceNode, 
		true);
	if (!status) return status;

	*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pSceneMeshBonedInstanceNode->m_pSourceNode);
	*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pSceneMeshBonedInstanceNode->GetBonePtr(0));
	*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pSceneMeshBonedInstanceNode->GetBonePtr(1));

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneMeshBonedInstanceNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneMeshBonedInstanceNode* pSceneMeshBonedInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MESH_BONED_INSTANCE_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find boned mesh instance node block.");
		}
		pDiskReader->EnterBlock();
	}
	//Read in the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneMeshRemappableNode::Deserialise(
		pDiskReader, 
		pNativeFtk, 
		pSceneMeshBonedInstanceNode, 
		true);
	if (!status) return status;

	//Disable warnings about casting from an integer to a pointer. This is fine for
	//our purposes
	#pragma warning(push)
	{
		#pragma warning(disable: 4312)
		S32 sourcePointer;
		*pDiskReader >>			sourcePointer;
		pSceneMeshBonedInstanceNode->m_pSourceNode = 
			reinterpret_cast<C_SceneMeshNode*>(sourcePointer);
	}
	#pragma warning(pop)

	#pragma warning(push)
	{
		#pragma warning(disable: 4312)
		S32 sourcePointer;
		*pDiskReader >>			sourcePointer;
		pSceneMeshBonedInstanceNode->m_pBones[0] = 
			reinterpret_cast<C_SceneNode*>(sourcePointer);
	}
	#pragma warning(pop)

	#pragma warning(push)
	{
		#pragma warning(disable: 4312)
		S32 sourcePointer;
		*pDiskReader >>			sourcePointer;
		pSceneMeshBonedInstanceNode->m_pBones[1] = 
			reinterpret_cast<C_SceneNode*>(sourcePointer);
	}
	#pragma warning(pop)

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

bool C_NativeSceneMeshBonedInstanceNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_SceneMeshBonedInstanceNode* pSceneMeshBonedInstanceNode)
{
#pragma warning(push)
#pragma warning(disable: 4311)
	for (U32 i = 0; i < 2; i++)
	{
		//Also resolve instance bone0 pointer.
		pSceneMeshBonedInstanceNode->m_pBones[i]
			= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(
				pSceneMeshBonedInstanceNode->m_pBones[i]));
		
		if (pSceneMeshBonedInstanceNode->m_pBones[i])
			pSceneMeshBonedInstanceNode->m_pBones[i]->AddInstanceOfThisNode(pSceneMeshBonedInstanceNode);
	}
#pragma warning(pop)

	return true;
}
