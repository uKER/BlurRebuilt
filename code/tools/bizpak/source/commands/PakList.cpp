#include "stdafx.h"
#include <System/Time/Time.h>

#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakIoMediator.h>

#include "PakList.h"
#include "PakCommon.h"

namespace Commands
{

int FatEntryCompareBySector(const void *lhs, const void *rhs)
{
	const Pak::C_PakFatEntry& a(*(const Pak::C_PakFatEntry*)lhs);
	const Pak::C_PakFatEntry& b(*(const Pak::C_PakFatEntry*)rhs);

	if (a.dataStartSector < b.dataStartSector )
		return -1;
	if (a.dataStartSector  > b.dataStartSector )
		return 1;
	return 0;
}

void OutputFat(const Pak::C_PakFile* pPak)
{
	U32 fatEntrySize = Pak::GetFatEntrySize(
		(Pak::E_FatEntryType)pPak->header.fatEntryType);
	U32 fatSize = fatEntrySize * pPak->nFatEntries;

	//Copy fat
	U8* pSortedFat = new U8[fatSize];
	memcpy(pSortedFat, pPak->pFatEntries, fatSize);

	//Sort by data sector.
	qsort(pSortedFat, pPak->nFatEntries, fatEntrySize, &FatEntryCompareBySector);

	//Display.
	LOGINFO(g_log,"\n-=FAT Contents=-\n");
	LOGINFO(g_log,"%s %-16.16s   %-10.10s %-10.10s %s %s        %s\n", 
		"Type",
		"Data Sector", 
		"Logic Size",
		"Physi Size",
		"Ratio",
		"Time Stamp",
		"Filepath");
	for (U32 i = 0; i < pPak->nFatEntries; i++)
	{
		const Pak::C_PakFatEntry* pFatEntry = 
			(const Pak::C_PakFatEntry*)(pSortedFat + fatEntrySize*i);
		const char* pFilePath = 
			(const char*)(pPak->stringTable.pData + pFatEntry->filePathStrTableOffset);
		char* type = pFatEntry->dataStartSector == PAK_DATA_ISDIRECTORY ?
			"Dir " : "File";
		U32 physicalLength = 0;
		if (pFatEntry->dataStartSector != PAK_DATA_ISDIRECTORY)
		{
			Sys::C_StreamPos dataStartPos = 
				pFatEntry->dataStartSector * pPak->pMediator->GetSectorSize();
			pPak->pMediator->Read(physicalLength, &dataStartPos);
		}
	
		F32 compression = 0;
		if (physicalLength < pFatEntry->dataLogicalLengthBytes)
			compression = (1.f-((F32)physicalLength / (F32)pFatEntry->dataLogicalLengthBytes))*100.f;

		Time::C_DecomposedTime decomposedTime(pFatEntry->fileTime);
		Util::C_String formattedTime = Time::FormatDate_TimeString(decomposedTime);

		const char* pParentFilePath;
#ifndef BUILD_CONFIG_DEBUG
		pParentFilePath = "";
#else
		if (pFatEntry->parentDirectoryOffsetBytes == PAK_FAT_ENTRY_NO_PARENT)
			pParentFilePath = "";
		else
		{
			const Pak::C_PakFatEntry* pParentFatEntry = (const Pak::C_PakFatEntry*)(pPak->pFatEntries + pFatEntry->parentDirectoryOffsetBytes);
				(const Pak::C_PakFatEntry*)(pSortedFat + fatEntrySize*i);
			pParentFilePath = 
				(const char*)(pPak->stringTable.pData + pParentFatEntry->filePathStrTableOffset);
		}
#endif
		LOGINFO(g_log,"%s 0x%-16.16I64x %-10.10u %-10.10u  %-3.3u%% %s %s\t%s\n", 
			type,
			pFatEntry->dataStartSector, 
			pFatEntry->dataLogicalLengthBytes,
			physicalLength,
			(U32)compression,
			formattedTime.Chars(),
			pFilePath,
			pParentFilePath);
	}
}

E_ReturnCode PakList(const C_Options& options)
{
	LOGINFO(g_log, "\n-=Pak List=-\n");

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	LogicalFs::C_PakFs* pPakFs = LogicalFs::PakHelpers::OpenPak(
		nativeFs, 
		options.pakFilePath);
	if (!pPakFs)
		return E_ReturnCode_Fatal;

	OutputHeader(pPakFs->GetPak());
	OutputFat(pPakFs->GetPak());

	SAFE_DELETE_PTR(pPakFs);
	return E_ReturnCode_Success;
}

}
