#ifndef LOGICALFS_PAKFS_PAK_HEADER_H
#define LOGICALFS_PAKFS_PAK_HEADER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{

struct C_PreHeader
{
	U32 fourCc;
	U32 version;
	U32 sectorSize;
};

struct C_PakHeader
{
	C_PakHeader() 
	{
		codec = 0;
		fatEntryType = 0;
		codecFlags = 0;
		pathSeparator = 0;
		unused = 0;
		metaStrOffset = 0;
		fileTime = 0;
		memset(wholeFileIntegrityHash, 0, sizeof(wholeFileIntegrityHash));
	}

    template <typename ARCHIVE>  
    void Serialise(ARCHIVE& ar)  
    {  
		ar.Write(wholeFileIntegrityHash, sizeof(wholeFileIntegrityHash));
		ar << codec;
		ar << fatEntryType;
		ar << pathSeparator;
		ar << unused;
		ar << codecFlags;
		ar << metaStrOffset;
		ar << fileTime;
    }

    template <typename ARCHIVE>  
    void Deserialise(ARCHIVE& ar)  
    {  
		ar.Read(wholeFileIntegrityHash, sizeof(wholeFileIntegrityHash));
		ar >> codec;
		ar >> fatEntryType;
		ar >> pathSeparator;
		ar >> unused;
		ar >> codecFlags;
		ar >> metaStrOffset;
		ar >> fileTime;
    }

	U8				wholeFileIntegrityHash[20];
	U8				codec;
	U8				fatEntryType;
	char			pathSeparator;
	U8				unused;
	U32				codecFlags;
	U32				metaStrOffset;
	U64				fileTime;
};

// [Blur2PC] On-disk header of a version 2 pak (32 bytes, follows the U32 header size).
// No whole-file integrity hash; the string table and FAT blocks start at sector 1 and
// their sector-aligned sizes are stored here. The last 4 bytes hold leftover characters
// ("l\0l\0", "ss\0\0", "bin\0") in every retail pak; their meaning is unknown.
#pragma pack(push, 1)	// the U64 sits at offset 12 in the file
struct C_PakHeaderV2
{
	U8				codec;
	U8				fatEntryType;
	char			pathSeparator;
	U8				unused;
	U32				codecFlags;
	U32				metaStrOffset;
	U64				fileTime;
	U32				stringTableBlockSize;
	U32				fatBlockSize;
	U32				unknown;
};
#pragma pack(pop)

// [Blur2PC] Version 2 FAT entry as stored (40 bytes; the secure variant appends the
// U32 padding + 20-byte SHA-1 of C_PakFatEntrySecure). The first 32 bytes match
// C_PakFatEntry. `unknown` is a per-file 32-bit value that is neither the path CRC
// nor a hash of the stored or logical bytes; `dataPhysicalLengthBytes` is the stored
// size, which version 1 kept as a U32 prefix in front of each payload.
struct C_PakFatEntryV2
{
	U32	filePathCrc;
	U32 filePathStrTableOffset;
	U32	parentDirectoryOffsetBytes;
	U32	dataLogicalLengthBytes;
	U64 fileTime;
	U64	dataStartSector;
	U32 unknown;
	U32 dataPhysicalLengthBytes;
};
}

#endif
