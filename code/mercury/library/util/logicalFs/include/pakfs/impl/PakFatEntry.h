#ifndef LOGICALFS_PAKFS_PAK_FAT_ENTRY_H
#define LOGICALFS_PAKFS_PAK_FAT_ENTRY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "PakDefinitions.h"

namespace Pak
{

//32 bytes.
struct C_PakFatEntry
{
	C_PakFatEntry();
	C_PakFatEntry(const C_PakFatEntry& rhs)
	{
		*this = rhs;
	}

    const bool operator > (const C_PakFatEntry& rhs) const
    {
        return filePathCrc > rhs.filePathCrc;
    }

    const bool operator < (const C_PakFatEntry& rhs) const
    {
        return filePathCrc < rhs.filePathCrc;
    }

	const C_PakFatEntry& operator = (const C_PakFatEntry& rhs)
	{
		if (&rhs == this)
			return *this;

		CopyFrom(rhs);
		
		return *this;
	}

	bool IsDirectory() const
	{
		return dataStartSector == PAK_DATA_ISDIRECTORY;
	}

    template <typename ARCHIVE>  
    void Serialise(ARCHIVE& ar)  
    {  
		ar << filePathCrc;
		ar << filePathStrTableOffset;
        ar << parentDirectoryOffsetBytes;
		ar << dataLogicalLengthBytes;
		ar << fileTime;
		ar << dataStartSector;
    }

    template <typename ARCHIVE>  
    void Deserialise(ARCHIVE& ar)  
    {  
		ar >> filePathCrc;
		ar >> filePathStrTableOffset;
        ar >> parentDirectoryOffsetBytes;
		ar >> dataLogicalLengthBytes;
		ar >> fileTime;
		ar >> dataStartSector;
    }

	U32	filePathCrc;				//CRC used to hash pak relative filepath.
	U32 filePathStrTableOffset;		//String table offset.
	U32	parentDirectoryOffsetBytes;	//Distance from start of FAT in bytes.
	U32	dataLogicalLengthBytes;		//Logical length of file data in bytes. Data may be compressed and hence the physical size may differ.
	U64 fileTime;					//File time-stamp
	U64	dataStartSector;			//Sector for the start of our file data.

protected:
	void CopyFrom(const C_PakFatEntry& rhs)
	{
		filePathCrc					= rhs.filePathCrc;
		filePathStrTableOffset		= rhs.filePathStrTableOffset;
		parentDirectoryOffsetBytes	= rhs.parentDirectoryOffsetBytes;
		dataStartSector				= rhs.dataStartSector;
		fileTime					= rhs.fileTime;
		dataLogicalLengthBytes		= rhs.dataLogicalLengthBytes;
	}
};

//56 bytes.
struct C_PakFatEntrySecure : C_PakFatEntry
{
	C_PakFatEntrySecure() : C_PakFatEntry(), padding(0) {}
	C_PakFatEntrySecure(const C_PakFatEntrySecure& rhs)
	{
		*this = rhs;
	}
	
	const C_PakFatEntrySecure& operator=(const C_PakFatEntrySecure& rhs)
	{
		if (&rhs == this)
			return *this;

		CopyFrom(rhs);
		
		return *this;
	}

    template <typename ARCHIVE>  
    void Serialise(ARCHIVE& ar)  
    {  
		C_PakFatEntry::Serialise(ar);
		ar << padding;
		ar.Write(fileHash, sizeof(fileHash));
    }

    template <typename ARCHIVE>  
    void Deserialise(ARCHIVE& ar)  
    {  
		C_PakFatEntry::Deserialise(ar);
		ar >> padding;
		ar.Read(fileHash, sizeof(fileHash));
    }

	U32 padding;
	U8 fileHash[20]; //160-bit SHA-1 hash

protected:
	void CopyFrom(const C_PakFatEntrySecure& rhs)
	{
		C_PakFatEntry::CopyFrom(rhs);

		memcpy(&fileHash, &rhs.fileHash, sizeof(fileHash));
	}

};

}

#endif
