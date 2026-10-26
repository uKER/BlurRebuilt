#ifndef LOGICALFS_PAKFS_PAK_FILE_H
#define LOGICALFS_PAKFS_PAK_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "PakHeader.h"
#include "PakFatEntry.h"

namespace Pak
{

class C_PakIoMediator;
class C_PakEntryFile;

struct C_PakStringTable
{
	C_PakStringTable() : pData(0), dataSize(0) {}
	~C_PakStringTable() { SAFE_DELETE_ARY(pData); }

	U8* pData;
	U32 dataSize;

private:
	C_PakStringTable(const C_PakStringTable& rhs);
	const C_PakStringTable& operator=(const C_PakStringTable& rhs);
};

class C_PakFile
{
public:
	C_PakFile();
	~C_PakFile();
	
	C_PakFatEntry*		GetFatEntry(const Sys::C_FilePath& filePath);

	C_PakFatEntry*		GetFatEntryByCrc(U32 crc);

	C_PakFatEntry*		GetFatEntryByIdx(U32 idx);

	//State
	C_PakIoMediator*		pMediator;

	//Persistent data
	C_PakHeader				header;
	C_PakStringTable		stringTable;
	U8*						pFatEntries;
	U32						nFatEntries;
	// [Blur2PC] version 2 support: entries are converted to the in-memory C_PakFatEntry
	// layout; the stored size of each entry and the raw on-disk FAT (for the signature
	// check) are kept here. NULL for version 1 paks.
	U32						version;
	U32*					pStoredSizes;
	U8*						pRawFatV2;
	U32						rawFatV2Size;
	S32						m_nOpen;

private:
	C_PakFile(const C_PakFile& rhs);
	const C_PakFile& operator=(const C_PakFile& rhs);

	void					OnFileOpened(C_PakEntryFile* pFile);
	void					OnFileClosed(C_PakEntryFile* pFile);
	void					OnFileRead(C_PakEntryFile* pFile, U64 pos, U64 bytes);
	void					OnFileWrite(C_PakEntryFile* pFile, U64 bytes);

	FRIEND_CLASS			C_PakEntryFile;
};

}

#endif 
