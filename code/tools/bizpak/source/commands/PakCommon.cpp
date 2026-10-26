#include "stdafx.h"
#include <System/Time/Time.h>

#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/Pak.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakIoMediator.h>

#include "PakCommon.h"

namespace Commands
{

void OutputHeader(const Pak::C_PakFile* pPak)
{
	LOGINFO(g_log,"\n-=Pak Header=-\n");
	U32 fatEntrySize = Pak::GetFatEntrySize(
		(Pak::E_FatEntryType)pPak->header.fatEntryType);
	U32 fatSize = fatEntrySize * pPak->nFatEntries;

	const char* codecName = GetCompressTypeStr((E_CompressType)pPak->header.codec);
	LOGINFO(g_log, "Codec: %s flags: %u sector size: %u endianness: %s\n", 
		codecName, 
		pPak->header.codecFlags, 
		pPak->pMediator->GetSectorSize(),
		pPak->pMediator->GetEndianSwapFlag() ? "BigEndian" : "LittleEndian");

	Time::C_DecomposedTime decomposedTime(pPak->header.fileTime);
	Util::C_String formattedTime = Time::FormatDate_TimeString(decomposedTime);
	Btl::String pathSeparator = pPak->header.pathSeparator;
	LOGINFO(g_log,"Path separator: %s\n", pathSeparator.Chars());
	LOGINFO(g_log,"Internal timestamp: %s\n", formattedTime.Chars());
	LOGINFO(g_log,"# FAT entries: %u FAT Secure: %s\n", 
		pPak->nFatEntries, 
		pPak->header.fatEntryType == Pak::E_FatEntryType_Secure ? "true" : "false");
	LOGINFO(g_log,"Metadata: %s\n", pPak->stringTable.pData + pPak->header.metaStrOffset);
	LOGINFO(g_log,"\n-=Size Breakdown=-\n");
	LOGINFO(g_log,"FAT size            : %u bytes\n", fatSize);
	LOGINFO(g_log,"String table size   : %u bytes\n", pPak->stringTable.dataSize);

	U32 logicalSizeTotal = 0;
	U32 physicalSizeTotal = 0;
	for (U32 i = 0; i < pPak->nFatEntries; i++)
	{
		const Pak::C_PakFatEntry& fatEntry = 
			*(const Pak::C_PakFatEntry*)(((U8*)pPak->pFatEntries)+fatEntrySize*i);

		U32 physicalLength = 0;
		if (fatEntry.dataStartSector != PAK_DATA_ISDIRECTORY)
		{
			Sys::C_StreamPos dataStartPos = 
				fatEntry.dataStartSector * pPak->pMediator->GetSectorSize();
			pPak->pMediator->Read(physicalLength, &dataStartPos);
		}
		physicalSizeTotal += physicalLength;
		logicalSizeTotal += fatEntry.dataLogicalLengthBytes;
	}
	F32 compressionTotal = 0;
	if (physicalSizeTotal < logicalSizeTotal)
		compressionTotal = (1.f-((F32)physicalSizeTotal / (F32)logicalSizeTotal))*100.f;
	LOGINFO(g_log,"Total logical size  : %u bytes\n", logicalSizeTotal);
	LOGINFO(g_log,"Total physical size : %u bytes\n", physicalSizeTotal);
	LOGINFO(g_log,"Total compression   : %u%%\n", (U32)compressionTotal);
}

}
