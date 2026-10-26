#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneLodGroupNode.h"

using namespace Aftk;

C_Status C_NativeSceneLodGroupNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneLodGroupNode* pSceneLodGroupNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SCENE_LOD_GROUP_NODE);
	}
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneLodGroupNode,
			true);
		if (!status) return status;
	}

	*pDiskWriter << (U32)pSceneLodGroupNode->GetNumLodLevels();
	for (U32 i = 0; i < pSceneLodGroupNode->GetNumLodLevels(); i++)
	{
		*pDiskWriter << (F32)pSceneLodGroupNode->GetLodLevelInfo(i).GetThreshold();
		//Serialise this lod levels scene node pointer.
		*pDiskWriter <<	pNativeFtk->GetIdFromPtr(
			pSceneLodGroupNode->GetLodLevelInfo(i).GetSceneNodePtr());
	}

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneLodGroupNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneLodGroupNode* pSceneLodGroupNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SCENE_LOD_GROUP_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find joint node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneLodGroupNode,
			true);
		if (!status) return status;
	}

	U32 numLodLevels;
	*pDiskReader >> numLodLevels;
	for (U32 i = 0; i < numLodLevels; i++)
	{
		C_GroupLodLevelInfo info;
		F32 threshold;
		*pDiskReader >> threshold;
		info.SetThreshold(threshold);

		//Disable warnings about casting from an integer to a pointer. This is fine for
		//our purposes
		#pragma warning(push)
		{
			#pragma warning(disable: 4312)
			S32 sourcePointer;
			*pDiskReader >>			sourcePointer;
			C_SceneNode* pSceneNode = 
				reinterpret_cast<C_SceneNode*>(sourcePointer);
			info.SetSceneNodePtr(pSceneNode);
		}
		#pragma warning(pop)

		pSceneLodGroupNode->AddLodLevelInfo(info);
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

bool C_NativeSceneLodGroupNode::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	C_SceneLodGroupNode* pLodGroupNode)
{
#pragma warning(push)
#pragma warning(disable: 4311)

	for (U32 iLodLevel = 0; iLodLevel < pLodGroupNode->GetNumLodLevels(); iLodLevel++)
	{
		C_GroupLodLevelInfo& info = pLodGroupNode->GetLodLevelInfo(iLodLevel);
		C_SceneNode* pLodSceneNode = 
			(C_SceneNode*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(info.GetSceneNodePtr()));
		info.SetSceneNodePtr(pLodSceneNode);
	}

#pragma warning(pop)
	return true;
}
