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

using namespace Aftk;

C_Status C_NativeSceneLightNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneLightNode* pSceneLightNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_LIGHT_NODE);
	}
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneLightNode,
			true);
		if (!status) return status;

		*pDiskWriter << pSceneLightNode->GetColour();

		*pDiskWriter << pSceneLightNode->GetIntensity();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneLightNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneLightNode* pSceneLightNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_LIGHT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find light node.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneLightNode,
			true);
		if (!status) return status;
		{
			Vector4F32 temp;
			if (pDiskReader->GetVersion() <= 170)
			{
				U32 colour;
				*pDiskReader >> colour;
				temp = ConvertU32ToVector4F32(colour);
			}
			else
			{
				*pDiskReader >> temp;
			}
			pSceneLightNode->SetColour(temp);
		}
		{
			F32 intensity;
			*pDiskReader >> intensity;
			pSceneLightNode->SetIntensity(intensity);
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

