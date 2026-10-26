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
#include "NativeLegacyShaderConnection.h"

using namespace Aftk;

//#pragma optimize("", off)

C_Status C_NativeIPlug::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const I_Plug* pPlug)
{
	C_Status status;

	status = C_NativeIObject::Serialise(
		pDiskWriter, 
		pNativeFtk, 
		pPlug);
	if (!status) return status;

	AFTK_ASSERT(pPlug->m_logicalIndex != -1);
	*pDiskWriter << pPlug->m_logicalIndex;

	//Convert connectivity information to a signed integer and write to disk.
	*pDiskWriter << pNativeFtk->GetIdFromPtr(pPlug->GetSourceConnectionPtr());

	return C_Status::E_Status_Success;
}

C_Status C_NativeIPlug::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	I_Plug* pPlug)
{
	C_Status status;
	status = C_NativeIObject::Deserialise(
		pDiskReader, 
		pNativeFtk,
		pPlug);
	if (!status) return status;

	{
		U32 index = pPlug->m_logicalIndex;
		*pDiskReader >> index;
		//Index seems to be being rendered invalid, ignore it for the moment. use the one we've already been
		//assigned.
	}

	//Store an integer alias for our connection point. This must be converted to a true
	//pointer as a post process after disk-reading.
	#pragma warning(push)
	{
		#pragma warning(disable: 4312)
		S32 connectionInt;
		*pDiskReader >>	connectionInt;
		pPlug->m_pSourceConnection = reinterpret_cast<I_Object*>(connectionInt);
	}
	#pragma warning(pop)

	return C_Status::E_Status_Success;
}

bool C_NativeIPlug::ResolveUnmanagedDgNodePointers(
	U32 version,
	C_NativeFtk* pNativeFtk,
	I_Plug* pPlug)
{
	#pragma warning(push)
	#pragma warning(disable: 4311)
	//Resolve connectivity for this plug.
	I_Object* pSourceConnection = 
		(I_Object*)pNativeFtk->GetPtrFromId(reinterpret_cast<S32>(pPlug->m_pSourceConnection));
	
	//Set plug connectivity, by doing this manually we avoid triggering evaluation of the plug.
	//It is not safe to evaluate plugs at this point while we're reading in from disk.

	//Assign new connection.
	pPlug->m_pSourceConnection = pSourceConnection;
//	XSI's crazy connectivity triggers this frequently.
//	AFTK_ASSERT(pPlug->m_pSourceConnection != pPlug->m_pParent);

	if (pPlug->m_pSourceConnection)
		pPlug->m_pSourceConnection->AddConnectedPlugPtr(pPlug);
	#pragma warning(pop)
	return true;
}
