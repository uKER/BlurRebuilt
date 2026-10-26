#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeScenePointConstraintNode.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeScenePointConstraintNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_ScenePointConstraintNode* pScenePointConstraintNode)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_POINT_CONSTRAINT_NODE);
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pScenePointConstraintNode,
			true);
		if (!status) return status;
	}


	*pDiskWriter << (U32)pScenePointConstraintNode->m_inputs.Size();
	for (U32 i = 0; i < (U32)pScenePointConstraintNode->m_inputs.Size(); i++)
	{
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug);
		if (!status) return status;
		{
			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(0));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(1));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(2));
			if (!status) return status;
		}

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetWeightPlug);
		if (!status) return status;

		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetParentPlug);
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pScenePointConstraintNode->m_translateOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeStaticPlug<F32>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pScenePointConstraintNode->m_offsetPolarityPlug);
	if (!status) return status;

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pScenePointConstraintNode->m_restTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pScenePointConstraintNode->m_constraintTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	*pDiskWriter << (U8)pScenePointConstraintNode->m_enableRestTranslate;

	pDiskWriter->EndBlock();
	return C_Status::E_Status_Success;
}

C_Status C_NativeScenePointConstraintNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_ScenePointConstraintNode* pScenePointConstraintNode)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_POINT_CONSTRAINT_NODE))
		return C_Status(C_Status::E_Status_Failure, "Failed to find point constraint node.");
	pDiskReader->EnterBlock();
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pScenePointConstraintNode,
			true);
		if (!status) return status;
	}

	U32 nInputs;
	*pDiskReader >> nInputs;
	pScenePointConstraintNode->SetNumInputs(nInputs);
	for (U32 i = 0; i < nInputs; i++)
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug);
		if (!status) return status;
		{
			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(0));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(1));
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_StaticPlug<F32>*)pScenePointConstraintNode->m_inputs[i]->targetTranslatePlug.GetChildPtr(2));
			if (!status) return status;
		}

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetWeightPlug);
		if (!status) return status;

		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pScenePointConstraintNode->m_inputs[i]->targetParentPlug);
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pScenePointConstraintNode->m_translateOffsetPlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_translateOffsetPlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeStaticPlug<F32>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pScenePointConstraintNode->m_offsetPolarityPlug);
	if (!status) return status;

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pScenePointConstraintNode->m_restTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_restTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pScenePointConstraintNode->m_constraintTranslatePlug);
	if (!status) return status;
	{
		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(0));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(1));
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_StaticPlug<F32>*)pScenePointConstraintNode->m_constraintTranslatePlug.GetChildPtr(2));
		if (!status) return status;
	}

	U8 tmp;
	*pDiskReader >> tmp;
	pScenePointConstraintNode->m_enableRestTranslate = tmp == 1;

	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

