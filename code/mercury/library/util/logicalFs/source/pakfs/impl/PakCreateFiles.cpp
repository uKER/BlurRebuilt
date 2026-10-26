#include "LogicalFsPch.h"

#include <System/Serialise/BinarySerialiseAutoEndian.h>
#include <System/Serialise/C_MemoryStream.h>

#include <Misc_Shared/Uid/FourCc.h>
#include <Misc_Shared/DataStructure/StringTable.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakCreateFiles.h>
#include <PakFs/Impl/PakEntryFile.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Pak.h>

#ifdef CFG_CONVERTER

namespace Pak
{

bool ProcessMemoryFile(
	C_PakFile& pak,
	C_PakFatEntry* pFatEntry, 
	C_PakFile* pBasePak,
	const C_PakFatEntry*	pBasePakFatEntry)
{
	if (!pak.pMediator->IsSectorAligned())
		return false;

	U32 fatEntrySize1 = GetFatEntrySize(
		(Pak::E_FatEntryType)pak.header.fatEntryType);
	U32 fatEntrySize2 = GetFatEntrySize(
		(Pak::E_FatEntryType)pBasePak->header.fatEntryType);
	if (fatEntrySize1 != fatEntrySize2)
	{
		LOGERROR(g_log, "FAT entry size mismatch!");
		return false;
	}

	memcpy(pFatEntry, pBasePakFatEntry, fatEntrySize1);

	Sys::C_StreamPos basePakDataStartPos = 
		pBasePak->pMediator->GetSectorSize()*pFatEntry->dataStartSector;

	Sys::C_StreamPos basePakPhysicalPos = basePakDataStartPos;
	U32 physicalSize;
	pBasePak->pMediator->Read(physicalSize, &basePakPhysicalPos);
	Sys::C_StreamPos dataStartPos = basePakPhysicalPos;

	pFatEntry->dataStartSector =
		((U32)pak.pMediator->GetWritePosition() + (pak.pMediator->GetSectorSize()-1)) / 
		pak.pMediator->GetSectorSize();

	U32 remainingBytes = physicalSize;
	//Write physical length.
	Sys::C_StreamPos writePos = pak.pMediator->GetWritePosition();
	if (!pak.pMediator->Write(remainingBytes))	return false;
	while (remainingBytes)
	{
		U8 buffer[PAK_IFILE_BUFFER_SIZE];
		U32 readSize = remainingBytes > PAK_IFILE_BUFFER_SIZE ? 
			PAK_IFILE_BUFFER_SIZE : (U32)remainingBytes;
		
		if (!pBasePak->pMediator->Read(buffer, readSize, &basePakPhysicalPos)) return false;
		if (!pak.pMediator->Write(buffer, readSize)) return false;

		remainingBytes = physicalSize - (U32)(basePakPhysicalPos-dataStartPos);
	}

	Sys::C_StreamPos pos = pak.pMediator->GetWritePosition();
	U32 padding = pak.pMediator->GetSectorSize() - (pos % pak.pMediator->GetSectorSize());
	if (!WritePadding(*pak.pMediator, padding)) return false;

	return true;
}

bool ProcessPhysicalFile(
	C_PakFile& pak,
	Sys::I_FileSystem& srcDataFs,
	C_PakFatEntry* pFatEntry, 
	const Sys::C_FilePath& filePath)
{
	if (!pak.pMediator->IsSectorAligned())
	{
		LOGERROR(g_log, "Data not sector aligned!");
		return false;
	}

	Sys::C_FileInfo fileInfo;
	if (!srcDataFs.FileInfo(filePath, &fileInfo))
	{
		LOGERROR(g_log, "Failed to query file info for %s!", filePath.Chars());
		return false;
	}

	if (fileInfo.m_FileSize > 0xffffffff)
		return false;

	Sys::I_File* pSrcFile = srcDataFs.Open(filePath);
	if (!pSrcFile)
	{
		LOGERROR(g_log, "Failed to open file %s!", filePath.Chars());
		return false;
	}

	pFatEntry->dataLogicalLengthBytes = (U32)pSrcFile->Length();
	pFatEntry->dataStartSector =
		((U32)pak.pMediator->GetWritePosition() + (pak.pMediator->GetSectorSize()-1)) / pak.pMediator->GetSectorSize();
	pFatEntry->fileTime = fileInfo.m_FileTime;

	//Store hash result.
	U8 sha1Result[20];

	C_PakEntryFile* pDstFile = new C_PakEntryFile();
	if (!pDstFile->Open(
		&pak,
		pFatEntry, 
		filePath, 
		Sys::E_FileAccess_Write,
		Sys::C_OpenParams(),
		new Sha1::C_HashContext(false, sha1Result, sizeof(sha1Result))))
	{
		LOGERROR(g_log, "Failed to open pak entry file %s!", filePath.Chars());
		return false;
	}

	if (pSrcFile->GetPosition() != 0)
		return false;

	U32 remainingBytes = pFatEntry->dataLogicalLengthBytes;
	U8 fileHash[20];
	memset(fileHash, 0, sizeof(fileHash));
	while (remainingBytes)
	{
		U8 buffer[PAK_IFILE_BUFFER_SIZE];
		U32 readSize = remainingBytes > PAK_IFILE_BUFFER_SIZE ? 
			PAK_IFILE_BUFFER_SIZE : (U32)remainingBytes;
		
		if (!pSrcFile->Read(buffer, readSize)) return false;
		if (!pDstFile->Write(buffer, readSize))	return false;

		remainingBytes = pFatEntry->dataLogicalLengthBytes - (U32)pSrcFile->GetPosition();
	}

	SAFE_DELETE_PTR(pDstFile);
	SAFE_DELETE_PTR(pSrcFile);

	for (U32 i = 0; i < sizeof(sha1Result); ++i)
		pak.header.wholeFileIntegrityHash[i] += sha1Result[i];

	if (pak.header.fatEntryType == E_FatEntryType_Secure)
	{
		C_PakFatEntrySecure* pFatEntrySecure = (C_PakFatEntrySecure*)pFatEntry;
		memcpy(pFatEntrySecure->fileHash, sha1Result, sizeof(sha1Result));
	}

	if (!pak.pMediator->IsSectorAligned())
	{
		LOGERROR(g_log, "Data not sector aligned!");
		return false;
	}
	return true;
}
}

#endif
