#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeComponentCluster.h"

using namespace Aftk;

C_Status C_NativeComponentCluster::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_ComponentCluster* pComponentCluster,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_COMPONENT_CLUSTER);
	}
	{
		*pDiskWriter << pComponentCluster->GetName();
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeComponentCluster::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_ComponentCluster* pComponentCluster,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_COMPONENT_CLUSTER))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find component cluster block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in cluster name
		std::string name;
		*pDiskReader >> name;
		pComponentCluster->SetName(name);
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

