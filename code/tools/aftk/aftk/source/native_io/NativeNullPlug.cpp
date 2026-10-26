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
#include "NativeNullPlug.h"
#include "NativeLegacyShaderConnection.h"

using namespace Aftk;

C_Status C_NativeNullPlug::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_NullPlug* pPlug)
{
	C_Status status;

	status = C_NativeIPlug::Serialise(
		pDiskWriter,
		pNativeFtk,
		pPlug);
	if (!status) return status;

	//Note that children must be read/written explicitly - not here.

	return C_Status::E_Status_Success;
}

C_Status C_NativeNullPlug::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_NullPlug* pPlug)
{
	C_Status status;

	status = C_NativeIPlug::Deserialise(
		pDiskReader,
		pNativeFtk,
		pPlug);
	if (!status) return status;

	//Note that children must be read/written explicitly - not here.

	return C_Status::E_Status_Success;
}
