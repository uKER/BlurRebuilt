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

using namespace Aftk;

C_Status C_NativeSceneNonAmbientLightNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneNonAmbientLightNode* pSceneNonAmbientLightNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_NON_AMBIENT_LIGHT_NODE);
	}
	{
		status = C_NativeSceneLightNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneNonAmbientLightNode,
			true);
		if (!status) return status;

		*pDiskWriter << (U32)pSceneNonAmbientLightNode->GetDecayType();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneNonAmbientLightNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneNonAmbientLightNode* pSceneNonAmbientLightNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_NON_AMBIENT_LIGHT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find ambient light node.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneLightNode::Deserialise(
			pDiskReader, 
			pNativeFtk,
			pSceneNonAmbientLightNode,
			true);
		if (!status) return status;

		{
			U32 temp;
			*pDiskReader >> temp;
			E_LightDecay decayType = 
				(E_LightDecay)temp;

			pSceneNonAmbientLightNode->SetDecayType(decayType);
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}

	return C_Status::E_Status_Success;
}
