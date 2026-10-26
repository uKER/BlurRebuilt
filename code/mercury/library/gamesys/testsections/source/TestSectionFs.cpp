#include "stdafx.h"
#include "TestSections/TestSectionFs.h"

#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <MemoryFs/MemoryFs.h>
#include <OverlayFs/OverlayFs.h>
#include <LogFs/LogFs.h>

namespace Framework
{

C_TestSectionFs::C_TestSectionFs(const Util::C_String& name) : C_TestSection(name)
{
	LogicalFs::C_LogFsManager::Instance().Initialise("./user/logfs");

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();
	Sys::C_FilePath workingDir = nativeFs.GetWorkingDirectory();
	{
		Sys::I_File* pFile1 = nativeFs.Open("./shaders\\billboard.fxb");
		if (!pFile1)
			return;
		SAFE_DELETE_PTR(pFile1);
	}

	int x = 1;

	LogicalFs::C_PakFs* pPakFs = LogicalFs::PakHelpers::OpenPak(nativeFs, "./shaderspc.pak");
	if (!pPakFs)
		return;
/*
	LogicalFs::C_PakFs* pPakFs2 = LogicalFs::PakHelpers::OpenPak(nativeFs, "./shadersps3.pak");
	if (!pPakFs2)
		return;
*/
	Sys::I_File* pFile = pPakFs->Open("\\billboard.fxb");
	if (!pFile)
		return;

	U32 size = (U32)pFile->Length();
	U8* pBuf = new U8[size];

	bool result = pFile->Read(pBuf, size);

	LOGFS_WRAPPER(LogicalFs::C_MemoryFs) memoryFs("memory");
	{
		memoryFs.CreateDirectory("/");
		memoryFs.CreateDirectory("/test/directory");
		Sys::C_FileInfo fileInfo;
		fileInfo.m_FileAccess = Sys::E_FileAccess_Read;
		fileInfo.m_FileSize = size;
		fileInfo.m_FileName = "/test/directory/a.file";
		fileInfo.m_FileTime = 0;
		fileInfo.m_FileType = Sys::E_FileType_File;
		if (!memoryFs.InjectFile(fileInfo, pBuf))
			return;
	}
	{
		Sys::I_File* pMemFile = memoryFs.Open("\\test/directory/a.file");
		U32 size2 = (U32)pMemFile->Length();
		U8* pBuf2 = new U8[size2];
		result = pMemFile->Read(pBuf2, size2);
		SAFE_DELETE_PTR(pMemFile);
		SAFE_DELETE_ARY(pBuf2);

		Sys::C_FileInfo info2;
		Sys::C_FileSystem::Instance().FileInfo("./shaders/additive_ld.fxb", &info2);

		Sys::C_FileInfo info;
		memoryFs.FileInfo("\\test/directory/a.file", &info);
		int x = 0;
	}

	//OverlayFS
	{	
		LOGFS_WRAPPER(LogicalFs::C_OverlayFs) overlayFs("overlay");
		overlayFs.SetWorkingDirectory(workingDir);
		LogicalFs::C_OverlayLayer::Id layer0 = 0;
		LogicalFs::C_OverlayLayer::Id layer1 = 1;

		{
			const Sys::C_FilePath kOverlayDstPath = workingDir + "/pakshaders";
			const Sys::C_FilePath kOverlaySrcPath = "/";
			overlayFs.AddLayer(
				kOverlayDstPath,
				pPakFs,
				kOverlaySrcPath,
				0,
				layer1);
		}

		{
			const Sys::C_FilePath kOverlayDstPath = workingDir + "\\mytest\\";
			const Sys::C_FilePath kOverlaySrcPath = workingDir + "\\shaders\\";
			overlayFs.InsertLayer(
				0,
				kOverlayDstPath,
				&nativeFs,
				kOverlaySrcPath,
				0,
				layer0);
		}

		Sys::I_File* pFileFromOverlay;
		{
			LOGFS_EVENT("billboard loading");

			pFileFromOverlay = overlayFs.Open(workingDir + "/mytest/billboard.fxb");
			if (!pFileFromOverlay)
				return;
			SAFE_DELETE_PTR(pFileFromOverlay);

			pFileFromOverlay = overlayFs.Open(workingDir + "/pakshaders/billboard.fxb");
			if (!pFileFromOverlay)
				return;
			SAFE_DELETE_PTR(pFileFromOverlay);
		}
	}

	SAFE_DELETE_PTR(pFile);
	SAFE_DELETE_ARY(pBuf);
	SAFE_DELETE_PTR(pPakFs);
}

C_TestSectionFs::~C_TestSectionFs()
{
}

}; // namespace Framework
