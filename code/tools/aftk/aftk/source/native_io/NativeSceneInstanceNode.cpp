#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneInstanceNode.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativeSceneInstanceNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneInstanceNode* pSceneInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_INSTANCE_NODE);
	}
	{
		{
			C_SceneNode* pSceneNode = (C_SceneNode*)pSceneInstanceNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Serialise(pDiskWriter, pNativeFtk, pSceneNode, true);
			if (!status) return status;
		}		//Write out the fragment of this node that comes from C_Node

		//Serialise this node's integer mapped 'source' pointer.
		*pDiskWriter <<	pNativeFtk->GetIdFromPtr(pSceneInstanceNode->m_pSourceNode);
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneInstanceNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneInstanceNode* pSceneInstanceNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_INSTANCE_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find instance node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in the fragment of this node that comes from C_SceneNode
		{
			C_SceneNode* pSceneNode = (C_SceneNode*)pSceneInstanceNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Deserialise(pDiskReader, pNativeFtk, pSceneNode, true);
			if (!status) return status;
		}
		//Deserialise relationship pointers

		//Disable warnings about casting from an integer to a pointer. This is fine for
		//our purposes
		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			S32 sourcePointer;
			*pDiskReader >>			sourcePointer;
			pSceneInstanceNode->m_pSourceNode = reinterpret_cast<C_SceneNode*>(sourcePointer);
		}
		#pragma warning(pop)
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

bool C_NativeSceneInstanceNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_SceneInstanceNode* pSceneInstanceNode)
{
#pragma warning(push)
#pragma warning(disable: 4311)

	pSceneInstanceNode->m_pSourceNode
		= (C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(
			pSceneInstanceNode->m_pSourceNode));

	if (pSceneInstanceNode->m_pSourceNode != 0)
		//Increase the reference count of the new node that we're instancing.
		pSceneInstanceNode->m_pSourceNode->AddInstanceOfThisNode(pSceneInstanceNode);

#pragma warning(pop)
	return true;
}
