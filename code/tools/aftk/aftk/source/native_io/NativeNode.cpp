#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativeNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_Node* pNode, 
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_NODE);
	}
	{
		//Write out the fragment of this node that comes from I_Object
		{
			C_Status status;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeIObject::Serialise(
				pDiskWriter, 
				pNativeFtk, 
				pNode);
			if (!status) return status;
		}

		//Write the name of this node to disk
		*pDiskWriter << pNode->GetName();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_Node* pNode,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find node block.");
		}
		pDiskReader->EnterBlock();
	}
	if (pDiskReader->GetVersion() >= 200)
	//Read the fragment of this node that comes from I_Object
	{
		C_Status status;
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeIObject::Deserialise(
			pDiskReader,
			pNativeFtk,
			pNode);
		if (!status) return status;
	}
	{
		*pDiskReader >> pNode->m_name;
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

