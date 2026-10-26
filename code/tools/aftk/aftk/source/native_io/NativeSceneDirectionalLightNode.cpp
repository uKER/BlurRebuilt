#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneLightNode.h"
#include "NativeSceneNonAmbientLightNode.h"
#include "NativeSceneDirectionalLightNode.h"

using namespace Aftk;

C_Status C_NativeSceneDirectionalLightNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneDirectionalLightNode* pSceneDirectionalLightNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_DIRECTIONAL_LIGHT_NODE);
	}
	{
		status = C_NativeSceneNonAmbientLightNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneDirectionalLightNode,
			useOuterBlockHeaders);
		if (!status) return status;
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneDirectionalLightNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneDirectionalLightNode* pSceneDirectionalLightNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_DIRECTIONAL_LIGHT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find directional light node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneNonAmbientLightNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneDirectionalLightNode,
			useOuterBlockHeaders);
		if (!status) return status;
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}
