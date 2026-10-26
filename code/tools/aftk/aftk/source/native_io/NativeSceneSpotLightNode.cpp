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
#include "NativeSceneSpotLightNode.h"

using namespace Aftk;

C_Status C_NativeSceneSpotLightNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneSpotLightNode* pSceneSpotLightNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SPOT_LIGHT_NODE);
	}
	{
		status = C_NativeSceneNonAmbientLightNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneSpotLightNode,
			true);
		if (!status) return status;

		*pDiskWriter << pSceneSpotLightNode->GetConeAngle();
		*pDiskWriter << pSceneSpotLightNode->GetPenumbraAngle();
		*pDiskWriter << pSceneSpotLightNode->GetDropOff();
		*pDiskWriter << pSceneSpotLightNode->GetCutOffRadius();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneSpotLightNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneSpotLightNode* pSceneSpotLightNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SPOT_LIGHT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find spot light node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneNonAmbientLightNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneSpotLightNode,
			true);
		if (!status) return status;
		{
			*pDiskReader >> pSceneSpotLightNode->m_coneAngle;
			*pDiskReader >> pSceneSpotLightNode->m_penumbraAngle;
			*pDiskReader >> pSceneSpotLightNode->m_dropOff;
		}

		if (pDiskReader->GetVersion() > 144)
		{
			*pDiskReader >> pSceneSpotLightNode->m_cutOffRadius;
		}

		pSceneSpotLightNode->m_coneAngle = 
			pSceneSpotLightNode->m_coneAngle;
		pSceneSpotLightNode->m_penumbraAngle = 
			pSceneSpotLightNode->m_penumbraAngle;
	}

	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}
