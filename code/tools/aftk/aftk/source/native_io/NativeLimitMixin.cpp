#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeUtility.h"
#include "NativeFtk.h"
#include "NativeLimitMixin.h"

using namespace Aftk;

C_Status C_NativeLimitMixin::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_LimitMixin* pMixin)
{
	pDiskWriter->WriteBytes(
		&(pMixin->m_limits[0]),
		sizeof(F32)*E_LimitType_Count);

	pDiskWriter->WriteBytes(
		&(pMixin->m_isLimited[0]),
		sizeof(bool)*E_LimitType_Count);

	return C_Status::E_Status_Success;
}

C_Status C_NativeLimitMixin::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_LimitMixin* pMixin)
{
	pDiskReader->ReadBytes(
		&(pMixin->m_limits[0]),
		sizeof(F32)*E_LimitType_Count);

	pDiskReader->ReadBytes(
		&(pMixin->m_isLimited[0]),
		sizeof(bool)*E_LimitType_Count);

	return C_Status::E_Status_Success;
}

