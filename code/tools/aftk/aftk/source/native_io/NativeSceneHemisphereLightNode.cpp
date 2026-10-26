#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneLightNode.h"
#include "NativeSceneHemisphereLightNode.h"

using namespace Aftk;

C_Status C_NativeSceneHemisphereLightNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneHemisphereLightNode* pSceneHemisphereLightNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_HEMISPHERE_LIGHT_NODE);
	}
	{
		status = C_NativeSceneLightNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneHemisphereLightNode,
			true);
		if (!status) return status;

	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneHemisphereLightNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneHemisphereLightNode* pSceneHemisphereLightNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_HEMISPHERE_LIGHT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find hemisphere light node.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneLightNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneHemisphereLightNode,
			true);
		if (!status) return status;

	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}
