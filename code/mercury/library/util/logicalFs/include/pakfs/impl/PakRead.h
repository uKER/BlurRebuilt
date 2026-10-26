#ifndef LOGICALFS_PAKREAD_H
#define LOGICALFS_PAKREAD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{
	class C_PakFile;
	struct C_PreHeader;

	bool ReadPreHeader(C_PakFile& pak, C_PreHeader& preHeader, Sys::C_StreamPos& versionPos, Sys::C_StreamPos* pPhysicalPos);

	bool ReadHeader(C_PakFile& pak, Sys::C_StreamPos& headerStartPos, Sys::C_StreamPos* pPhysicalPos);

	bool ReadStringTable(C_PakFile& pak, Sys::C_StreamPos* pPhysicalPos);
	
	bool ReadFat(C_PakFile& pak, Sys::C_StreamPos& fatStartPos, Sys::C_StreamPos* pPhysicalPos);
}

#endif
