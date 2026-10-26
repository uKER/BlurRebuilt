#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeBlindDataNode.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeGenericAttribute.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativeBlindDataNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_BlindDataNode* pBlindDataNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;

	// Material
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_BLIND_DATA_NODE);
	}
	{
		//Write out the fragment of this material node that comes from C_Node
		{
			C_Node* pNode = (C_Node*)pBlindDataNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Serialise(pDiskWriter, pNativeFtk, pNode, false);
			if (!status) return status;
		}

		//Only write out the proporties defined within AFTK BLIND DATA NODE
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE_LIST);
		{
			//num attributes
			U32 numAttributes = (U32)pBlindDataNode->GetNumAttributes();
			//Store the number of attributes
			*pDiskWriter << numAttributes;
			//blind data attributes
			for (U32 i = 0; i < numAttributes; i++) 
			{
				const C_GenericAttribute* pGenericAttribute =
					&pBlindDataNode->GetAttribute(i);
				status = C_NativeGenericAttribute::Serialise(pDiskWriter, pNativeFtk, pGenericAttribute, true);
				if (!status) return status;
			}
		}
		pDiskWriter->EndBlock();
	}
	if (useOuterBlockHeaders)
	{
		pDiskWriter->EndBlock(); // end of blind data
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeBlindDataNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_BlindDataNode* pBlindDataNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_BLIND_DATA_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find blind data node.");
		}
		pDiskReader->EnterBlock();
	}
	// Blind data
	{
		//Write out the fragment of this blind data that comes from C_Node
		{
			C_Node* pNode = (C_Node*)pBlindDataNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeNode::Deserialise(pDiskReader, pNativeFtk, pNode, false);
			if (!status) return status;
		}

		//Read attribute list
		if (pDiskReader->GetNextBlockInfo() != NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE_LIST)
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find generic attribute list.");
		}
		pDiskReader->EnterBlock();
		{
			//num attributes
			U32 numAttributes;
			*pDiskReader >> numAttributes;
			pBlindDataNode->SetAttributeInitialCapacity(numAttributes);
			//blind data attributes
			for (U32 i = 0; i < numAttributes; i++) 
			{
				C_GenericAttribute aftkAttr;
				status = C_NativeGenericAttribute::Deserialise(pDiskReader, pNativeFtk, &aftkAttr, true);
				if (!status)
				{
					//We failed to deserialise our attribute.
					return status;
				}
				pBlindDataNode->AddAttribute(aftkAttr);
			}
		}
		pDiskReader->ExitBlock();
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock(); // end of blind data
	}

	return C_Status::E_Status_Success;
}

