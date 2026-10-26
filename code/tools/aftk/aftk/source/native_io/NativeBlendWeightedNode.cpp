#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeBlendWeightedNode.h"
#include "NativeUtility.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"

using namespace Aftk;

C_Status C_NativeBlendWeightedNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_BlendWeightedNode* pBlendWeightedNode, 
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_BLEND_WEIGHTED_NODE);
	}
	{
		C_Status status;
		//Write out the fragment of this node that comes from C_Node
		{
			C_Node* pNode = (C_Node*)pBlendWeightedNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Serialise(
				pDiskWriter, 
				pNativeFtk,
				pNode, 
				true);
			if (!status) return status;
		}

		//Write blend weighted inputs.
		*pDiskWriter << (U32)pBlendWeightedNode->m_inputs.Size();
		for (U32 i = 0; i < (U32)pBlendWeightedNode->m_inputs.Size(); i++)
		{
			status = C_NativeNullPlug::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pBlendWeightedNode->m_inputs[i]->inputPlug);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Serialise(
				pDiskWriter,
				pNativeFtk,
				&pBlendWeightedNode->m_inputs[i]->weightPlug);
			if (!status) return status;
		}
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeBlendWeightedNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_BlendWeightedNode* pBlendWeightedNode,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_BLEND_WEIGHTED_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find node block.");

		pDiskReader->EnterBlock();
	}
	C_Status status;
	//Read the fragment of this node that comes from I_Object
	{
		C_Node* pNode = (C_Node*)pBlendWeightedNode;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pNode, 
			true);
		if (!status) return status;
	}

	//Read blend weighted inputs.
	U32 nInputs;
	*pDiskReader >> nInputs;
	pBlendWeightedNode->SetNumInputs(nInputs);
	for (U32 i = 0; i < nInputs; i++)
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			&pBlendWeightedNode->m_inputs[i]->inputPlug);
		if (!status) return status;

		if (pDiskReader->GetVersion() >= 210)
		{
			status = C_NativeNullPlug::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pBlendWeightedNode->m_inputs[i]->weightPlug);
			if (!status) return status;
		}
	}

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

