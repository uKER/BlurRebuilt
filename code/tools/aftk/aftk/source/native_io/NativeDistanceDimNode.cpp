#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeDistanceDimNode.h"
#include "NativeUtility.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"

using namespace Aftk;

C_Status C_NativeDistanceDimNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_DistanceDimNode* pNode, 
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_DISTANCE_DIM_NODE);
	}
	{
		C_Status status;
		//Write out the fragment of this node that comes from C_Node
		{
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Serialise(
				pDiskWriter, 
				pNativeFtk,
				pNode, 
				true);
			if (!status) return status;
		}

		for (U32 i = 0; i < 2; i++)
		{
			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				pNode->m_inputs[i]);
			if (!status) return status;
		}
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeDistanceDimNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_DistanceDimNode* pNode,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_DISTANCE_DIM_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find node block.");

		pDiskReader->EnterBlock();
	}
	C_Status status;
	//Read the fragment of this node that comes from I_Object
	{
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pNode, 
			true);
		if (!status) return status;
	}

	for (U32 i = 0; i < 2; i++)
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			pNode->m_inputs[i]);
		if (!status) return status;
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

