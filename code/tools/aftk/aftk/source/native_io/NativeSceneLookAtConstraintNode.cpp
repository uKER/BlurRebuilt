#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneAimConstraintNode.h"
#include "NativeSceneLookAtConstraintNode.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneLookAtConstraintNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneLookAtConstraintNode* pSceneLookAtConstraintNode)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_LOOK_AT_CONSTRAINT_NODE);
	{
		status = C_NativeSceneAimConstraintNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneLookAtConstraintNode);
		if (!status) return status;
	}

	//Input
	status = C_NativeStaticPlug<F32>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneLookAtConstraintNode->m_rollPlug);
	if (!status) return status;

	//Output
	status = C_NativeStaticPlug<F32>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneLookAtConstraintNode->m_distanceBetweenPlug);
	if (!status) return status;

	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneLookAtConstraintNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneLookAtConstraintNode* pSceneLookAtConstraintNode)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_LOOK_AT_CONSTRAINT_NODE))
		return C_Status(C_Status::E_Status_Failure, "Failed to find look at constraint node.");

	pDiskReader->EnterBlock();
	{
		status = C_NativeSceneAimConstraintNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneLookAtConstraintNode);
		if (!status) return status;
	}

	//Input
	status = C_NativeStaticPlug<F32>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneLookAtConstraintNode->m_rollPlug);
	if (!status) return status;

	//Output
	status = C_NativeStaticPlug<F32>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneLookAtConstraintNode->m_distanceBetweenPlug);
	if (!status) return status;

	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

