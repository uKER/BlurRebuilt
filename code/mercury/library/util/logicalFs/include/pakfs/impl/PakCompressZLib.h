#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_ZLIB_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_ZLIB_H

#ifdef _MSC_VER
#pragma once
#endif

//Zlib
#include "Compression.h"

#define PAK_ZLIB_BUFFER_SIZE 32768

namespace Pak
{

class C_PakCompressZLib : public I_PakCompress
{
public:
	C_PakCompressZLib(U32 flags);
	virtual ~C_PakCompressZLib();
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
	bool Flush( int flush );

	C_PakIoMediator*	m_pMediator;
	Sys::C_StreamPos	m_physicalStartPos;
	z_stream			m_zStream;
	U8					m_buffer[PAK_ZLIB_BUFFER_SIZE];
	U32					m_fileAccessAttributes;
	//Physical size of compressed data.
	U32					m_dataPhysicalLength;
	//Location of physical data size value.
	U32					m_compress;
	Sys::C_StreamPos	m_physicalReadPos;
};

}

#endif
