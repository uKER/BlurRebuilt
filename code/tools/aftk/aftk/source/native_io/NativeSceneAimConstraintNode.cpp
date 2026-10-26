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
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneAimConstraintNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneAimConstraintNode* pSceneAimConstraintNode)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_AIM_CONSTRAINT_NODE);
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneAimConstraintNode,
			true);
		if (!status) return status;
	}

	//Input
	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_targetTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_aimVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_upVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_rotationalOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	//Input
	status = C_NativeStaticPlug<U32>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_worldUpVectorTypePlug);
	if (!status) return status;

	//Input
	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_worldUpVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	//Input/output
	status = C_NativeStaticPlug<U32>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintRotateOrderPlug);
	if (!status) return status;

	//Output
	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeStaticPlug<Matrix44>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_worldUpMatrixPlug);
	if (!status) return status;

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_targetParentPlug);
	if (!status) return status;

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintParentPlug);
	if (!status) return status;


	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneAimConstraintNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneAimConstraintNode* pSceneAimConstraintNode)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_AIM_CONSTRAINT_NODE))
		return C_Status(C_Status::E_Status_Failure, "Failed to find aim constraint node.");
	pDiskReader->EnterBlock();
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneAimConstraintNode,
			true);
		if (!status) return status;
	}

	//Input
	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_targetTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_targetTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_aimVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_aimVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_upVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_upVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	//Input
	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_rotationalOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_rotationalOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	//Input
	status = C_NativeStaticPlug<U32>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_worldUpVectorTypePlug);
	if (!status) return status;

	//Input
	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_worldUpVectorPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_worldUpVectorPlug.GetChildPtr(2));
		if (!status) return status;
	}

	//Input/output
	status = C_NativeStaticPlug<U32>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintRotateOrderPlug);
	if (!status) return status;

	//Output
	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneAimConstraintNode->m_constraintRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneAimConstraintNode->m_constraintRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	if (pDiskReader->GetVersion() >= 203)
	{
		status = C_NativeStaticPlug<Matrix44>::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneAimConstraintNode->m_worldUpMatrixPlug);
	}

	if (pDiskReader->GetVersion() >= 210)
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneAimConstraintNode->m_targetParentPlug);
		if (!status) return status;

		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneAimConstraintNode->m_constraintParentPlug);
		if (!status) return status;
	}

	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

