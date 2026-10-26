#ifndef LOGICALFS_PAKWRITE_H
#define LOGICALFS_PAKWRITE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef CFG_CONVERTER

namespace Pak
{
	class C_PakFile;

	bool WritePreHeader(C_PakFile& pak, Sys::C_StreamPos& versionPos);

	bool WriteHeader(C_PakFile& pak, Sys::C_StreamPos headerStartPos);

	bool WriteStringTable(C_PakFile& pak);

	bool WriteFat(C_PakFile& pak, Sys::C_StreamPos fatStartPos, U8* pSecurityHash);
}

#endif

#endif
