#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeMaterialNode.h"
#include "NativeGenericAttribute.h"
#include "NativeUtility.h"
#include "NativeShaderNode.h"
#include "NativeIPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeShaderNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_ShaderNode* pShaderNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SHADER_NODE);
	}
	status = C_NativeNode::Serialise(
		pDiskWriter,
		pNativeFtk,
		pShaderNode,
		true);
	if (!status) return status;

	*pDiskWriter << pShaderNode->GetNodeIndex();
	*pDiskWriter << pShaderNode->GetOriginalNodeTypeName();

	//num plugs
	U32 numPlugs = (U32)pShaderNode->m_dynamicPlugs.Size();
	//Store the number of plugs
	*pDiskWriter << numPlugs;
	//material attributes
	for (U32 i = 0; i < numPlugs; i++) 
	{
		const C_DynamicPlug* pPlug = (C_DynamicPlug*)pShaderNode->m_dynamicPlugs[i];

		status = C_NativeDynamicPlug::Serialise(
			pDiskWriter, 
			pNativeFtk,
			pPlug);
		if (!status) return status;
	}

	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeShaderNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_ShaderNode* pShaderNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SHADER_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find shader node block.");
		}
		pDiskReader->EnterBlock();
	}
	if (pDiskReader->GetVersion() < 200)
	{
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pShaderNode, 
			false);
	}
	else
	{
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pShaderNode, 
			true);
	}
	if (!status) return status;

	S32 nodeInstance;
	*pDiskReader >> nodeInstance;
	pShaderNode->SetNodeInstance(nodeInstance);

	*pDiskReader >> pShaderNode->m_originalNodeTypeName;

	if (pDiskReader->GetVersion() < 200)
	{
		//Skip static plugs.
		pDiskReader->EnterBlock();
		pDiskReader->ExitBlock();
	}

	if (pDiskReader->GetVersion() < 200)
		pDiskReader->EnterBlock();

	U32 numPlugs;
	*pDiskReader >> numPlugs;
	pShaderNode->m_dynamicPlugs.Reserve(numPlugs);

	//Recursively fetch all of the children for this plug (if any).
	for (U32 i = 0; i < numPlugs; i++)
	{
		C_DynamicPlug* pPlug = new C_DynamicPlug();
		if (!pPlug) return C_Status::E_Status_OutOfMemory;
		pShaderNode->AddPlugPtr(pPlug);

		status = C_NativeDynamicPlug::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pPlug);
		
		if (!status) return status;
	}

	if (pDiskReader->GetVersion() < 200)
		pDiskReader->ExitBlock();

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock(); // end of plug
	}

	return C_Status::E_Status_Success;
}
