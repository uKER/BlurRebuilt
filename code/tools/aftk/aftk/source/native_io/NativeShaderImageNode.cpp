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
#include "NativeLegacyShaderConnection.h"
#include "NativeShaderNode.h"
#include "NativeShaderImageNode.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"

using namespace Aftk;

C_Status C_NativeShaderImageNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_ShaderImageNode* pShaderImageNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SHADER_IMAGE_NODE);
	}
	status = C_NativeShaderNode::Serialise(
		pDiskWriter, 
		pNativeFtk, 
		pShaderImageNode, 
		true);
	if (!status) return status;

	//Write out static plugs.
	status = C_NativeStaticPlug<std::string>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pShaderImageNode->m_filePlug);
	if (!status) return status;
	{
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(0));
		if (!status) return status;
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(1));
		if (!status) return status;
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(2));
		if (!status) return status;
		status = C_NativeNullPlug::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(3));
		if (!status) return status;
	}
	
	status = C_NativeStaticPlug<std::string>::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pShaderImageNode->m_imageTypePlug);
	if (!status) return status;

	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeShaderImageNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_ShaderImageNode* pShaderImageNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SHADER_IMAGE_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find shader node block.");

		pDiskReader->EnterBlock();
	}

	status = C_NativeShaderNode::Deserialise(
		pDiskReader, 
		pNativeFtk, 
		pShaderImageNode, 
		true);
	if (!status) return status;

	//Read static plugs.
	status = C_NativeStaticPlug<std::string>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pShaderImageNode->m_filePlug);
	if (!status) return status;
	{
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(0));
		if (!status) return status;
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(1));
		if (!status) return status;
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(2));
		if (!status) return status;
		status = C_NativeNullPlug::Deserialise(
			pDiskReader,
			pNativeFtk,
			(C_NullPlug*)pShaderImageNode->m_filePlug.GetChildPtr(3));
		if (!status) return status;
	}

	status = C_NativeStaticPlug<std::string>::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pShaderImageNode->m_imageTypePlug);
	if (!status) return status;

	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock(); // end of plug
	}
	
	return C_Status::E_Status_Success;
}



