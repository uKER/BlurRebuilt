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
#include "NativeSceneMeshRemappedInstanceNode.h"
#include <aftk/AftkUtility.h>

using namespace Aftk;

C_Status C_NativeSceneMeshRemappedInstanceNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_SceneMeshRemappedInstanceNode* pSceneMeshRemappedInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MESH_REMAPPED_INSTANCE_NODE);
	}

	//Write out the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneMeshRemappableNode::Serialise(
		pDiskWriter, 
		pNativeFtk,
		pSceneMeshRemappedInstanceNode, 
		true);
	if (!status) return status;

	//Serialise this node's integer mapped 'source' pointer.
	*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pSceneMeshRemappedInstanceNode->m_pSourceNode);

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneMeshRemappedInstanceNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneMeshRemappedInstanceNode* pSceneMeshRemappedInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MESH_REMAPPED_INSTANCE_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find remapped mesh node block.");
		}
		pDiskReader->EnterBlock();
	}
	//Read in the fragment of this node that comes from C_SceneNode
	status = C_NativeSceneMeshRemappableNode::Deserialise(
		pDiskReader, 
		pNativeFtk, 
		pSceneMeshRemappedInstanceNode, 
		true);
	if (!status) return status;

	//Disable warnings about casting from an integer to a pointer. This is fine for
	//our purposes
	#pragma warning(push)
	{
		#pragma warning(disable: 4312)
		S32 sourcePointer;
		*pDiskReader >>			sourcePointer;
		pSceneMeshRemappedInstanceNode->m_pSourceNode = 
			reinterpret_cast<C_SceneMeshNode*>(sourcePointer);
	}
	#pragma warning(pop)

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}

bool C_NativeSceneMeshRemappedInstanceNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_SceneMeshRemappedInstanceNode* pSceneMeshRemappedInstanceNode)
{
#pragma warning(push)
#pragma warning(disable: 4311)
	//Also resolve instance source pointer.
	{
		pSceneMeshRemappedInstanceNode->m_pSourceNode
			= (C_SceneMeshNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(
				pSceneMeshRemappedInstanceNode->m_pSourceNode));

		if (pSceneMeshRemappedInstanceNode->m_pSourceNode != 0)
			//Increase the reference count of the new node that we're instancing.
			pSceneMeshRemappedInstanceNode->m_pSourceNode->AddInstanceOfThisNode(
				pSceneMeshRemappedInstanceNode);
	}
#pragma warning(pop)

	return true;
}
