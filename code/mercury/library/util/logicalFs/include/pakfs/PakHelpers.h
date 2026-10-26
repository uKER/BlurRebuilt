#ifndef LOGICALFS_PAK_HELPERS_H
#define LOGICALFS_PAK_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <PakFs/PakFs.h>

namespace Security
{
	class I_Security;
}

namespace LogicalFs
{
namespace PakHelpers
{
	void FindPakFiles(Btl::Vector<Sys::C_FilePath>& pakFiles, Sys::I_FileSystem& fs, const Sys::C_FilePath& dir);

	C_PakFs* OpenPak(Sys::I_FileSystem& fs, const Sys::C_FilePath& filePath, const Security::I_Security* pSecurity = NULL, const Sys::C_OpenParams& params = Sys::C_OpenParams());
}
}

#endif
