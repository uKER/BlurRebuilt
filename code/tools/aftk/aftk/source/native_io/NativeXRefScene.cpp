#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeXRefScene.h"

using namespace Aftk;

C_Status C_NativeXRefScene::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const ::C_XRefScene* pXRefScene)
{
	C_Status status;
	pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_XREF_SCENE);
	{
		*pDiskWriter << pXRefScene->GetFileName();
		U32 crc = pXRefScene->GetCRC();
		*pDiskWriter << crc;
	}
	pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeXRefScene::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	::C_Scene* pScene)
{
	C_Status status;
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_XREF_SCENE))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find light node.");
	}

	pDiskReader->EnterBlock();
	{
		{
			std::string fileName;
			*pDiskReader >> fileName;
			U32 crc;
			*pDiskReader >> crc;
			pScene->AddXRefScene(fileName, crc, status);
			if (!status) return status;
		}
	}
	pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}


