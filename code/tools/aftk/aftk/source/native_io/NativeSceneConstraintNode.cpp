#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneConstraintNode.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneConstraintNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneConstraintNode* pSceneConstraintNode)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_CONSTRAINT_NODE);
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneConstraintNode,
			true);
		if (!status) return status;
	}
	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneConstraintNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneConstraintNode* pSceneConstraintNode)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_CONSTRAINT_NODE))
		return C_Status(C_Status::E_Status_Failure, "Failed to find constraint node.");
	pDiskReader->EnterBlock();
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneConstraintNode,
			true);
		if (!status) return status;
	}
	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

