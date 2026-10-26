#ifndef PAK_CREATE_FILES_H
#define PAK_CREATE_FILES_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef CFG_CONVERTER

namespace Pak
{
	class C_PakFile;
	struct C_PakFatEntry;

	bool ProcessPhysicalFile(
		C_PakFile& pak,
		Sys::I_FileSystem& srcDataFs,
		C_PakFatEntry* pFatEntry, 
		const Sys::C_FilePath& filePath);

	bool ProcessMemoryFile(
		C_PakFile& pak,
		C_PakFatEntry* pFatEntry, 
		C_PakFile* pBasePak,
		const C_PakFatEntry* pBasePakFatEntry);	
}

#endif

#endif
