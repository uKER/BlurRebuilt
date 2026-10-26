#include "stdafx.h"
#include <System/Time/Time.h>

#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>

#include "PakCreate.h"
#include "PakCommon.h"

namespace Commands
{

bool WriteHashHeader(U8* pSecurityHash, const C_Options& options)
{
	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	Sys::C_FilePath headerFilePath = options.pakFilePath;
	headerFilePath = Sys::RemoveFileExtension(headerFilePath);
	Sys::C_FilePath headerFilePathAndExt = headerFilePath + ".hash";
	Sys::I_File* pFile = nativeFs.Open(headerFilePathAndExt, Sys::E_FileAccess_Write);
	if (!pFile)
		return false;

	pFile->Write(pSecurityHash, 20);
	SAFE_DELETE_PTR(pFile);
	LOGINFO(g_log, "Wrote security hash binary %s\n", headerFilePathAndExt.Chars());
	return true;
}

E_ReturnCode PakCreate(const C_Options& options)
{
	LOGINFO(g_log, "\n-=Pak Create=-\n");

	if (options.switches.compressType == E_CompressType_Unknown)
	{
		LOGERROR(g_log, "Unknown codec specified.\n");
		return E_ReturnCode_Fatal;
	}

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	U8 securityHash[20];
	bool result(false);

	Pak::C_PakOptions createOptions;
	createOptions.pakFilePath = options.pakFilePath;
	createOptions.pakRootPath = options.pakRootPath;
	createOptions.sectorSize = options.switches.sectorSize;
	createOptions.bigEndian = options.switches.bigEndian;
	createOptions.codec = (Pak::E_Codec)options.switches.compressType;
	createOptions.recurse = options.switches.recurseSubdirectories;
	createOptions.metadata = options.switches.metadata;
	if (options.pathSeparator != '\\' && options.pathSeparator != '/')
	{
		LOGERROR(g_log, "Please specify either \\ or / as a path separator\n");
		return E_ReturnCode_Fatal;
	}
	createOptions.pathSeparator = options.pathSeparator;

	LOGINFO(g_log, "\nCreating new pak...\n");

	{
		Sys::C_FilePath dir = Sys::ExtractDirectory(createOptions.pakFilePath);
		if (!nativeFs.FileExists(dir) && !nativeFs.CreateDirectory(dir))
		{
			LOGINFO(g_log, "Failed to create output folder %s!", dir.Chars());
			return E_ReturnCode_Fatal;
		}

		result = Pak::CreatePak(
			options.switches.secure ? securityHash : 0,		//SHA1 hash result
			nativeFs,
			nativeFs,										//Filesystem containing src data
			createOptions,									//Creation params
			options.toProcess);								//List of files and folders to pak
		if (!result)
			return E_ReturnCode_Fatal;
	}

	if (options.switches.secure)
	{
		result = WriteHashHeader(securityHash, options);
	}

	if (result)
	{
		Pak::C_PakFile* pPak = Pak::ReadPak(nativeFs, options.pakFilePath);
		if (!pPak)
		{
			LOGERROR(g_log, "Failed to read output file %s.\n", options.pakFilePath.Chars());
			return E_ReturnCode_Fatal;
		}
		OutputHeader(pPak);
		LOGINFO(g_log, "Pak %s created successfully!\n", options.pakFilePath.Chars());
		SAFE_DELETE_PTR(pPak);
		return E_ReturnCode_Success;
	}

	LOGERROR(g_log, "Failed to create pak %s!\n", options.pakFilePath.Chars());
	return E_ReturnCode_Fatal;
}

}
