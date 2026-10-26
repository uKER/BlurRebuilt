#include "stdafx.h"
#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>

#include "PakTest.h"

namespace Commands
{

E_ReturnCode PakTest(const C_Options& options)
{
	LOGINFO(g_log, "\n-=Pak Test=-\n");
	LOGINFO(g_log, "Verifying pak file...\n");
	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	LogicalFs::C_PakFs* pPakFs = 
		LogicalFs::PakHelpers::OpenPak(
		nativeFs, 
		options.pakFilePath);
	if (!pPakFs)
		return E_ReturnCode_Fatal;

	bool result = Pak::Verify(*pPakFs->GetPak());
	if (result)
		LOGINFO(g_log, "Pak file verified ok!\n");
	else
		LOGINFO(g_log, "Verifcation failed, pak file corrupt!\n");

	SAFE_DELETE_PTR(pPakFs);
	return result ? E_ReturnCode_Success : E_ReturnCode_Fatal;
}

}
