#include "stdafx.h"
#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>

#include "PakExtract.h"

namespace Commands
{

E_ReturnCode PakExtract(const C_Options& options)
{
	LOGINFO(g_log, "\n-=Pak Extract=-\n");

	if (options.switches.outputDirectory == "")
	{
		LOGERROR(g_log, "No output directory specified!\n");
		return E_ReturnCode_Fatal;
	}

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	LogicalFs::C_PakFs* pPakFs = 
		LogicalFs::PakHelpers::OpenPak(
		nativeFs, 
		options.pakFilePath);
	if (!pPakFs)
	{
		LOGERROR(g_log, "Failed to open pak %s!\n", options.pakFilePath.Chars());
		return E_ReturnCode_Fatal;
	}

	bool status = Pak::Extract(
		nativeFs,
		options.switches.outputDirectory,
		*pPakFs->GetPak());

	SAFE_DELETE_PTR(pPakFs);

	if (status)
	{
		LOGINFO(g_log, "Pak %s extracted successfully!\n", options.pakFilePath.Chars());
		return E_ReturnCode_Success;
	}

	LOGERROR(g_log, "Failed to extract pak %s!\n", options.pakFilePath.Chars());
	return E_ReturnCode_Fatal;
}

}
