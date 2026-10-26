#include "stdafx.h"
#include <System/Time/Time.h>

#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>

#include "PakAddUpdate.h"
#include "PakCommon.h"

namespace Commands
{

E_ReturnCode PakUpdate(const C_Options& options)
{
	LOGINFO(g_log, "\n-=Pak Update=-\n");

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	Pak::C_PakOptions createOptions;
	createOptions.pakFilePath = options.pakFilePath;
	createOptions.pakRootPath = options.pakRootPath;
	createOptions.sectorSize = options.switches.sectorSize;
	createOptions.bigEndian = options.switches.bigEndian;
	createOptions.codec = (Pak::E_Codec)options.switches.compressType;
	createOptions.recurse = options.switches.recurseSubdirectories;
	createOptions.metadata = options.switches.metadata;
	createOptions.pathSeparator = options.pathSeparator;

	if (options.command == E_Command_AddUpdate)
		createOptions.updateType = Pak::E_UpdateType_AddUpdate;
	if (options.command == E_Command_CreateIncremental)
	{
		if (options.pathSeparator != '\\' && options.pathSeparator != '/')
		{
			LOGERROR(g_log, "Please specify either \\ or / as a path separator\n");
			return E_ReturnCode_Fatal;
		}
		createOptions.updateType = Pak::E_UpdateType_CreateIncremental;
	}
	if (options.command == E_Command_Delete)
		createOptions.updateType = Pak::E_UpdateType_Delete;

	bool result = Pak::UpdatePak(
		nativeFs,
		nativeFs,
		createOptions,
		options.toProcess);
	if (result)
	{
		LogicalFs::C_PakFs* pPakFs = LogicalFs::PakHelpers::OpenPak(
			nativeFs, 
			options.pakFilePath);
		if (!pPakFs)
			return E_ReturnCode_Fatal;

		OutputHeader(pPakFs->GetPak());

		SAFE_DELETE_PTR(pPakFs);

		LOGINFO(g_log, "Pak %s updated successfully!\n", options.pakFilePath.Chars());
	}
	else
	{
		LOGERROR(g_log, "Failed to update pak %s!\n", options.pakFilePath.Chars());
	}

	return result ? E_ReturnCode_Success : E_ReturnCode_Fatal;
}

}
