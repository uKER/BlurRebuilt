#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_XMEM_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_XMEM_H

#ifdef _MSC_VER
#pragma once
#endif

#define PAK_XMEM_READ_BUFFER_SIZE 32*1024

namespace Pak
{
struct C_XMemStream
{
	bool compressing;
	void* context;

	U32 totalLogical;
	U32 totalPhysical;
	U32 readRemainingPhysicalBytesFromDisk;
	U32 readPhysicalDiskBytesProcessed;
};

class C_PakCompressXMem : public I_PakCompress
{
public:
	C_PakCompressXMem(U32 flags);
	virtual ~C_PakCompressXMem();
	virtual bool Open(
		C_PakIoMediator* m_pPakRWOptimiser,
		Sys::C_StreamPos physicalStartPos, 
		const U32 fileAccessAttributes = Sys::E_FileAccess_Read,
		Sha1::C_HashContext* pHashContext = NULL);

	virtual bool Close();

#ifdef CFG_CONVERTER
	virtual bool Write(U8* buffer, U32 nrBytes);
#endif
	virtual bool Read(U8* buffer, U32 nrBytes);

	virtual bool Restart(U32 streamToLogicalPos);

	virtual U32 GetPhysicalLength() const;

private:
#ifdef CFG_CONVERTER
	bool Flush(U8* pSrcBuf, SIZE_T srcSize);
#endif

	C_PakIoMediator*	m_pMediator;
	Sys::C_StreamPos	m_physicalStartPos;
	C_XMemStream		m_xmemStream;
	U8					m_readBuffer[PAK_XMEM_READ_BUFFER_SIZE];
	U32					m_fileAccessAttributes;
	//Physical size of compressed data.
	U32					m_dataPhysicalLength;
	//Location of physical data size value.
	Sys::C_StreamPos	m_physicalReadPos;
};

}

#endif
