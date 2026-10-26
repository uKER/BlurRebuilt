#include "LogicalFsPch.h"

#include <Core/Threads/Atomic.h>
#include <system/serialise/FsPerfLogger.h>

#include <algorithm>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakEntryFile.h>


namespace Pak
{

C_PakFile::C_PakFile()
{
	pMediator = new C_PakIoMediator();
	pFatEntries = NULL;
	nFatEntries = 0;
	m_nOpen = 0;
	version = PAK_VERSION;			// [Blur2PC]
	pStoredSizes = NULL;
	pRawFatV2 = NULL;
	rawFatV2Size = 0;
}

C_PakFile::~C_PakFile()
{
	SAFE_DELETE_ARY(pFatEntries);
	SAFE_DELETE_ARY(pStoredSizes);	// [Blur2PC]
	SAFE_DELETE_ARY(pRawFatV2);
	SAFE_DELETE_PTR(pMediator);
}

int GetIndexByCRC(Btl::Vector<C_PakFatEntry>& fat, U32 crc)
{
	for (int x = 0 ; x < fat.Size() ; x++)
	{
		if (fat[x].filePathCrc == crc)
		{
			return x;
		}
	}
	return -1;
}

C_PakFatEntry* C_PakFile::GetFatEntry(const Sys::C_FilePath& filePath)
{
	Btl::String260 localised;
	C_PakCrc32 crc32;
	U32 crc;
	if (header.pathSeparator != Sys::GetLocalDirToken())
	{
		ConvertToPakDirToken(
			localised, 
			header.pathSeparator, 
			filePath);
	}
	else
		localised = filePath.String();

	//Remove trailing path seperator from all but root. Might need to consider a better
	//way of handling this kind of thing.
	if (localised.Length() > 1 && localised[localised.Length()-1] == header.pathSeparator)
		localised = localised(0, localised.Length()-1);

	crc = crc32.CRCFromMemory((const U8*)localised.Chars(), localised.Length());
	return GetFatEntryByCrc(crc);
}

C_PakFatEntry* C_PakFile::GetFatEntryByCrc(U32 crc)
{
	if (nFatEntries == 0)
		return NULL;
	U32 t = nFatEntries;
	U32 b = 0;
	bool found = false;
	while(1)
	{
		int x = ((t - b) / 2) + b;
		if (crc == GetFatEntryByIdx(x)->filePathCrc) 
		{ 
			return GetFatEntryByIdx(x); 
		}
		if (t-b == 1) 
		{ 
			break; 
		};
		if (crc > GetFatEntryByIdx(x)->filePathCrc)
			b = x;
		if (crc < GetFatEntryByIdx(x)->filePathCrc) 
			t = x;
	}

	return NULL;
}


C_PakFatEntry* C_PakFile::GetFatEntryByIdx(U32 idx)
{
	U32 fatEntrySize = GetFatEntrySize((E_FatEntryType)header.fatEntryType);
	return (C_PakFatEntry*)(((U8*)pFatEntries)+fatEntrySize*idx);
}

//Ideally this would get moved elsewhere, it's only here so we can grab the pak name.
void C_PakFile::OnFileOpened(C_PakEntryFile* pFile)
{
	MT::AtomicIncrement((volatile int*)&m_nOpen);
	FSPERF_OPENED(pMediator->GetLowLevelName().Chars(), pFile->Name(), (U64)pFile, Sys::E_FileAccess_Read);
}

void C_PakFile::OnFileClosed(C_PakEntryFile* pFile)
{
	MT::AtomicDecrement((volatile int*)&m_nOpen);
	FSPERF_CLOSED(pMediator->GetLowLevelName().Chars(), (U64)pFile);
}

void C_PakFile::OnFileRead(C_PakEntryFile* pFile, U64 pos, U64 bytes)
{
	FSPERF_READ_VIRTUAL(pMediator->GetLowLevelName().Chars(), (U64)pFile, pos, bytes, NULL);
}

void C_PakFile::OnFileWrite(C_PakEntryFile* pFile, U64 bytes)
{
	FSPERF_WRITE_VIRTUAL(pMediator->GetLowLevelName().Chars(), (U64)pFile, bytes);
}

}
