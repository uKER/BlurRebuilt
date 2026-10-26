#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{

namespace Sha1
{
	class C_HashContext;
}

class C_PakIoMediator;

class I_PakCompress
{
protected:
	Sha1::C_HashContext* m_pHashContext;
	// [Blur2PC] version 2 paks store the payload size in the FAT and have no U32 prefix
	// in front of the payload; C_PakEntryFile tells the codec before Open.
	U32  m_knownPhysicalLength;
	bool m_bHasKnownPhysicalLength;
	U32  PrefixBytes() const { return m_bHasKnownPhysicalLength ? 0 : sizeof(U32); }
	bool ReadPhysicalLength(C_PakIoMediator* pMediator, U32& length, Sys::C_StreamPos* pPos);

public: 
	I_PakCompress() : m_pHashContext(NULL), m_knownPhysicalLength(0), m_bHasKnownPhysicalLength(false) {}
	void SetKnownPhysicalLength(U32 length) { m_knownPhysicalLength = length; m_bHasKnownPhysicalLength = true; }
	virtual ~I_PakCompress() {}

	virtual bool Open(
		C_PakIoMediator* m_pMediator,
		Sys::C_StreamPos physicalStartPos, 
		const U32 fileAccessAttributes = Sys::E_FileAccess_Read,
		Sha1::C_HashContext* pHashContext = NULL
		) = 0;

	virtual bool Close() = 0;

#ifdef CFG_CONVERTER
	virtual bool Write(U8* buffer, U32 nrBytes) = 0;
#endif
	virtual bool Read(U8* buffer, U32 nrBytes) = 0;

	virtual bool Restart(U32 streamToLogicalPos) = 0;

	virtual U32 GetPhysicalLength() const = 0;
};

}

#endif
