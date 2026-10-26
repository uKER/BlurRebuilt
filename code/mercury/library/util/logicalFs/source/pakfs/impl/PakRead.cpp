#include "LogicalFsPch.h"

#include <System/Serialise/BinarySerialiseAutoEndian.h>
#include <System/Serialise/C_MemoryStream.h>

#include <Misc_Shared/Uid/FourCc.h>
#include <Misc_Shared/DataStructure/StringTable.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakFatEntry.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakRead.h>

namespace Pak
{

bool ReadPreHeader(C_PakFile& pak, C_PreHeader& preHeader, Sys::C_StreamPos& versionPos, Sys::C_StreamPos* pPhysicalPos)
{
	if (!pak.pMediator->Read(preHeader.fourCc, pPhysicalPos)) return false;
	versionPos = *pPhysicalPos;
	if (!pak.pMediator->Read(preHeader.version, pPhysicalPos)) return false;
	if (!pak.pMediator->Read(preHeader.sectorSize, pPhysicalPos)) return false;

	 //PAK_FOURCC_U32;
	if (preHeader.fourCc != PAK_FOURCC_U32)
	{
		Ut::DoEndianSwap::EndianSwap(preHeader.fourCc);
		if (preHeader.fourCc == PAK_FOURCC_U32)
		{
			Ut::DoEndianSwap::EndianSwap(preHeader.version);
			Ut::DoEndianSwap::EndianSwap(preHeader.sectorSize);
			pak.pMediator->SetEndianSwapFlag(true);
		}
		else
		{
			LOGERROR(g_log, "Failed to read pak. Given file is not a pak!");
			return false;
		}
	}

	pak.pMediator->SetSectorSize(preHeader.sectorSize);
	pak.version = preHeader.version;	// [Blur2PC]
	return true;
}

bool ReadHeader(C_PakFile& pak, Sys::C_StreamPos& headerStartPos, Sys::C_StreamPos* pPhysicalPos)
{
	U32 headerSize;
	if (!pak.pMediator->Read(headerSize, pPhysicalPos)) return false;
	// [Blur2PC] version 2 header: map onto the version 1 structure, then jump to sector 1
	// where the string table and FAT live.
	if (pak.version >= 2)
	{
		if (headerSize != sizeof(C_PakHeaderV2))
		{
			LOGERROR(g_log, "Pak v2 header size mismatch!");
			return false;
		}
		headerStartPos = *pPhysicalPos;
		C_PakHeaderV2 h2;
		if (!pak.pMediator->Read(&h2, headerSize, pPhysicalPos)) return false;
		if (pak.pMediator->GetEndianSwapFlag())
		{
			Ut::DoEndianSwap::EndianSwap(h2.codecFlags);
			Ut::DoEndianSwap::EndianSwap(h2.metaStrOffset);
			Ut::DoEndianSwap::EndianSwap(h2.fileTime);
			Ut::DoEndianSwap::EndianSwap(h2.stringTableBlockSize);
			Ut::DoEndianSwap::EndianSwap(h2.fatBlockSize);
		}
		memset(pak.header.wholeFileIntegrityHash, 0, sizeof(pak.header.wholeFileIntegrityHash));
		pak.header.codec = h2.codec;
		pak.header.fatEntryType = h2.fatEntryType;
		pak.header.pathSeparator = h2.pathSeparator;
		pak.header.unused = h2.unused;
		pak.header.codecFlags = h2.codecFlags;
		pak.header.metaStrOffset = h2.metaStrOffset;
		pak.header.fileTime = h2.fileTime;
		*pPhysicalPos = pak.pMediator->GetSectorSize();
		return true;
	}
	if (headerSize != sizeof(C_PakHeader))
	{
		LOGERROR(g_log, "Pak header size mismatch!");
		return false;
	}

	headerStartPos = *pPhysicalPos;
	if (!pak.pMediator->Read(&pak.header, headerSize, pPhysicalPos)) return false;

	//In place endian fixup.
	if (pak.pMediator->GetEndianSwapFlag())
	{
		Sys::C_MemoryStream memStream((U8*)&pak.header, headerSize);
		memStream.SetEndianSwapFlag(pak.pMediator->GetEndianSwapFlag());
		Sys::BinaryDeserialiserAutoEndian ar(memStream);
		ar >> pak.header;
	}

	return true;
}

bool ReadStringTable(C_PakFile& pak, Sys::C_StreamPos* pPhysicalPos)
{
	U8 sectorBuf[PAK_SECTOR_SIZE_MAX];
	//String table.
	{
		if (!pak.pMediator->Read(pak.stringTable.dataSize, pPhysicalPos)) return false;
		pak.stringTable.pData = new U8[pak.stringTable.dataSize];
		if (!pak.pMediator->Read(pak.stringTable.pData, pak.stringTable.dataSize, pPhysicalPos)) return false;
		PakCryptDecrypt(pak.stringTable.pData, pak.stringTable.dataSize);
		U32 padding = pak.pMediator->GetSectorSize() - (*pPhysicalPos % pak.pMediator->GetSectorSize());
		//Consume sector padding.
		if (!pak.pMediator->Read(sectorBuf, padding, pPhysicalPos)) return false;
	}

	return true;
}

// [Blur2PC]
bool ReadFatV2(C_PakFile& pak, Sys::C_StreamPos& fatStartPos, Sys::C_StreamPos* pPhysicalPos, U32 filePakFatSize)
{
	const bool secure = pak.header.fatEntryType == E_FatEntryType_Secure;
	if (!secure && pak.header.fatEntryType != E_FatEntryType_Normal)
	{
		LOGERROR(g_log, "Unknown FAT entry type!");
		return false;
	}
	const U32 diskEntrySize = sizeof(C_PakFatEntryV2) + (secure ? sizeof(U32) + 20 : 0);
	if (filePakFatSize != diskEntrySize * pak.nFatEntries)
	{
		LOGERROR(g_log, "FAT size mismatch (v2)!");
		return false;
	}
	pak.pRawFatV2 = new U8[filePakFatSize];
	pak.rawFatV2Size = filePakFatSize;
	fatStartPos = *pPhysicalPos;
	if (!pak.pMediator->Read(pak.pRawFatV2, filePakFatSize, pPhysicalPos)) return false;

	const U32 memEntrySize = GetFatEntrySize((E_FatEntryType)pak.header.fatEntryType);
	pak.pFatEntries = new U8[memEntrySize * pak.nFatEntries];
	pak.pStoredSizes = new U32[pak.nFatEntries];
	const bool swap = pak.pMediator->GetEndianSwapFlag();
	for (U32 i = 0; i < pak.nFatEntries; ++i)
	{
		C_PakFatEntryV2 src;
		memcpy(&src, pak.pRawFatV2 + diskEntrySize * i, sizeof(src));
		if (swap)
		{
			Ut::DoEndianSwap::EndianSwap(src.filePathCrc);
			Ut::DoEndianSwap::EndianSwap(src.filePathStrTableOffset);
			Ut::DoEndianSwap::EndianSwap(src.parentDirectoryOffsetBytes);
			Ut::DoEndianSwap::EndianSwap(src.dataLogicalLengthBytes);
			Ut::DoEndianSwap::EndianSwap(src.fileTime);
			Ut::DoEndianSwap::EndianSwap(src.dataStartSector);
			Ut::DoEndianSwap::EndianSwap(src.unknown);
			Ut::DoEndianSwap::EndianSwap(src.dataPhysicalLengthBytes);
		}
		C_PakFatEntry& dst = *(C_PakFatEntry*)(pak.pFatEntries + memEntrySize * i);
		dst.filePathCrc = src.filePathCrc;
		dst.filePathStrTableOffset = src.filePathStrTableOffset;
		// v2 parent offsets count on-disk entry strides; rescale to the in-memory stride.
		dst.parentDirectoryOffsetBytes = src.parentDirectoryOffsetBytes == (U32)PAK_FAT_ENTRY_NO_PARENT
			? src.parentDirectoryOffsetBytes : (src.parentDirectoryOffsetBytes / diskEntrySize) * memEntrySize;
		dst.dataLogicalLengthBytes = src.dataLogicalLengthBytes;
		dst.fileTime = src.fileTime;
		dst.dataStartSector = src.dataStartSector;
		pak.pStoredSizes[i] = src.dataPhysicalLengthBytes;
		if (secure)
		{
			C_PakFatEntrySecure& dsts = (C_PakFatEntrySecure&)dst;
			dsts.padding = 0;
			memcpy(dsts.fileHash, pak.pRawFatV2 + diskEntrySize * i + sizeof(C_PakFatEntryV2) + sizeof(U32), sizeof(dsts.fileHash));
		}
	}
	return true;
}

bool ReadFat(C_PakFile& pak, Sys::C_StreamPos& fatStartPos, Sys::C_StreamPos* pPhysicalPos)
{
	pak.pMediator->Read(pak.nFatEntries, pPhysicalPos);
	U32 filePakFatSize;
	pak.pMediator->Read(filePakFatSize, pPhysicalPos);

	U32 fatEntrySize = GetFatEntrySize((E_FatEntryType)pak.header.fatEntryType);
	U32 fatSize = fatEntrySize * pak.nFatEntries;
	// [Blur2PC] version 2 FAT: 40-byte entries (+24 secure) converted to the in-memory layout.
	if (pak.version >= 2)
		return ReadFatV2(pak, fatStartPos, pPhysicalPos, filePakFatSize);
	if (filePakFatSize != fatSize)
	{
		LOGERROR(g_log, "FAT size mismatch!");
		return false;
	}

	pak.pFatEntries = new U8[filePakFatSize];
	fatStartPos = *pPhysicalPos;
	if (!pak.pMediator->Read(pak.pFatEntries, filePakFatSize, pPhysicalPos)) return false;

	//In place endian fixup.
	if (pak.pMediator->GetEndianSwapFlag())
	{
		Sys::C_MemoryStream memStream((U8*)pak.pFatEntries, filePakFatSize);
		memStream.SetEndianSwapFlag(pak.pMediator->GetEndianSwapFlag());
		Sys::BinaryDeserialiserAutoEndian ar(memStream);

		//Perform fixup for appropriate derived structure type.
		if (pak.header.fatEntryType == E_FatEntryType_Normal)
			for (U32 i = 0; i < pak.nFatEntries; ++i)
			{
				C_PakFatEntry& fatEntry = ((C_PakFatEntry*)pak.pFatEntries)[i];
				ar >> fatEntry;
			}
		else if (pak.header.fatEntryType == E_FatEntryType_Secure)
			for (U32 i = 0; i < pak.nFatEntries; ++i)
			{
				C_PakFatEntrySecure& fatEntry = ((C_PakFatEntrySecure*)pak.pFatEntries)[i];
				ar >> fatEntry;
			}
		else
		{
			LOGERROR(g_log, "Unknown FAT entry type!");
			return false;
		}
	}

	return true;
}

}
