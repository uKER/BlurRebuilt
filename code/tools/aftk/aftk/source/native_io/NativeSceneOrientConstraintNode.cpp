#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneOrientConstraintNode.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneOrientConstraintNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneOrientConstraintNode* pSceneOrientConstraintNode)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_ORIENT_CONSTRAINT_NODE);
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneOrientConstraintNode,
			true);
		if (!status) return status;
	}

	*pDiskWriter << (U32)pSceneOrientConstraintNode->m_inputs.Size();
	for (U32 i = 0; i < (U32)pSceneOrientConstraintNode->m_inputs.Size(); i++)
	{
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug);
		if (!status) return status;
		{
			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(0));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(1));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(2));
			if (!status) return status;
		}

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetWeightPlug);
		if (!status) return status;

		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetParentPlug);
		if (!status) return status;
	}



	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_rotateOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_restRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_constraintRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	*pDiskWriter << (U8)pSceneOrientConstraintNode->m_enableRestRotate;

	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneOrientConstraintNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneOrientConstraintNode* pSceneOrientConstraintNode)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_ORIENT_CONSTRAINT_NODE))
		return C_Status(C_Status::E_Status_Failure, "Failed to find orient constraint node.");
	pDiskReader->EnterBlock();
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneOrientConstraintNode,
			true);
		if (!status) return status;
	}

	U32 nInputs;
	*pDiskReader >> nInputs;
	pSceneOrientConstraintNode->SetNumInputs(nInputs);
	for (U32 i = 0; i < nInputs; i++)
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug);
		if (!status) return status;
		{
			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(0));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(1));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_inputs[i]->targetRotatePlug.GetChildPtr(2));
			if (!status) return status;
		}

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetWeightPlug);
		if (!status) return status;

		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pSceneOrientConstraintNode->m_inputs[i]->targetParentPlug);
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_rotateOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_rotateOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_restRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_restRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pSceneOrientConstraintNode->m_constraintRotatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pSceneOrientConstraintNode->m_constraintRotatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	U8 tmp;
	*pDiskReader >> tmp;
	pSceneOrientConstraintNode->m_enableRestRotate = tmp == 1;

	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

