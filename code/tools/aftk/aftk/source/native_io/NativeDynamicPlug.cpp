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
#include "NativeIObject.h"
#include "NativeIPlug.h"
#include "NativeDynamicPlug.h"
#include "NativeLegacyShaderConnection.h"

using namespace Aftk;

C_Status C_NativeDynamicPlug::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_DynamicPlug* pPlug)
{
	C_Status status;

	status = C_NativeIPlug::Serialise(
		pDiskWriter, 
		pNativeFtk, 
		pPlug);
	if (!status) return status;

	status = C_NativeGenericAttribute::Serialise(
		pDiskWriter,
		pNativeFtk,
		&pPlug->m_attribute,
		false);
	if (!status) return status;

	//Store the number of plugs
	*pDiskWriter << (U32)pPlug->GetNumChildren();
	//material attributes
	for (U32 i = 0; i < pPlug->GetNumChildren(); i++) 
	{
		const C_DynamicPlug* pChildPlug = (C_DynamicPlug*)pPlug->GetChildPtr(i);
		//Recursively write out compound attribute members
		status = Serialise(
			pDiskWriter,
			pNativeFtk, 
			pChildPlug);
		if (!status) return status;
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativeDynamicPlug::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_DynamicPlug* pPlug)
{
	C_Status status;

	status = C_NativeIPlug::Deserialise(
		pDiskReader, 
		pNativeFtk, 
		pPlug);
	if (!status) return status;

	status = C_NativeGenericAttribute::Deserialise(
		pDiskReader,
		pNativeFtk,
		&pPlug->m_attribute,
		false);
	if (!status) return status;

	//num child plugs
	U32 numChildPlugs;
	//Store the number of plugs
	*pDiskReader >> numChildPlugs;
	//material attributes
	pPlug->m_children.Resize(numChildPlugs);
	for (U32 i = 0; i < numChildPlugs; i++) 
	{
		C_DynamicPlug* pChildPlug = new C_DynamicPlug;
		if (!pChildPlug) return C_Status::E_Status_OutOfMemory;
		pChildPlug->SetLogicalIndex(i);
		pChildPlug->SetParentPtr(pPlug);
		pPlug->m_children[i] = pChildPlug;
		
		//Recursively write out compound attribute members
		status = Deserialise(
			pDiskReader,
			pNativeFtk, 
			pChildPlug);
		if (!status) return status;
	}

	return C_Status::E_Status_Success;
}
