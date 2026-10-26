#include "LogicalFsPch.h"

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressZLib.h>
#include <PakFs/Impl/PakSha1.h>
#ifdef PLATFORM_PS3
#include <sys/timer.h>
#endif

namespace Pak
{

C_PakCompressZLib::C_PakCompressZLib(U32 flags)
{
	m_pMediator = 0;
	m_physicalStartPos = 0;
	m_physicalReadPos = 0;

	m_fileAccessAttributes = 0;
	//Physical size of compressed data.
	m_dataPhysicalLength = 0;
	if (flags == 0)
		m_compress = Z_BEST_COMPRESSION;
	else
		m_compress = flags;
}

C_PakCompressZLib::~C_PakCompressZLib()
{
}

bool C_PakCompressZLib::Open(
	C_PakIoMediator* pMediator, 
	Sys::C_StreamPos physicalStartPos, 
	const U32 fileAccessAttributes,
	Sha1::C_HashContext* pHashContext)
{
	m_physicalStartPos = physicalStartPos;
	m_physicalReadPos = m_physicalStartPos;
	m_pMediator = pMediator;
	m_fileAccessAttributes = fileAccessAttributes;

	m_pHashContext = pHashContext;

	switch(m_fileAccessAttributes)
	{
		case Sys::E_FileAccess_Read:
			memset(&m_zStream, 0, sizeof(m_zStream));
			inflateInit( &m_zStream );
			return ReadPhysicalLength(m_pMediator, m_dataPhysicalLength, &m_physicalReadPos);	// [Blur2PC]
#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			memset(&m_zStream, 0, sizeof(m_zStream));
			m_zStream.zalloc		= NULL;
			m_zStream.zfree		= NULL;
			m_zStream.opaque		= NULL;
			m_zStream.next_in	= NULL;
			m_zStream.next_out	= m_buffer;
			m_zStream.avail_in	= 0;
			m_zStream.avail_out	= PAK_ZLIB_BUFFER_SIZE;
			deflateInit(&m_zStream, m_compress );

			//Write space for physical length
			m_pMediator->SetWritePosition(physicalStartPos);
			return m_pMediator->Write(m_dataPhysicalLength);
#endif
		default:
			break;
	}
		
	LOGERROR(g_log, "Failed to open ZLib compressed pak!");
	return false;
}

bool C_PakCompressZLib::Restart(U32 streamToLogicalPos)
{
	// This Function is used for seeking 
	// Close The zLib Stream
	inflateEnd( &m_zStream );
	// Re-initialise the zLib Stream
	m_zStream.avail_in = 0;
	inflateInit( &m_zStream );

	m_physicalReadPos = m_physicalStartPos;
	{
		// Read In The Length
		if (!ReadPhysicalLength(m_pMediator, m_dataPhysicalLength, &m_physicalReadPos))	// [Blur2PC]
			return false;
	}
	// Stream in to logicalPosition
	const U32 bufferSize = PAK_ZLIB_BUFFER_SIZE;
	U8	buffer[bufferSize];
	U32 bytesRemaining = streamToLogicalPos;
	while (bytesRemaining)
	{
		U32 nrToRead = bytesRemaining > bufferSize ? bufferSize : bytesRemaining;
		Read(buffer, nrToRead);
		bytesRemaining -= nrToRead;
	}

	return true;
}

U32 C_PakCompressZLib::GetPhysicalLength() const
{
	return m_dataPhysicalLength;
}

bool C_PakCompressZLib::Close()
{
	switch(m_fileAccessAttributes)
	{
		case Sys::E_FileAccess_Read:
			inflateEnd( &m_zStream );
			return true;

#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			m_zStream.next_in = NULL;
			m_zStream.avail_in = 0;
			Flush(Z_FINISH);
			U8 zeroBuffer[PAK_SECTOR_SIZE_MAX] = {0};
			//The U32 term below is the physical length.
			U32 sectorSize = m_pMediator->GetSectorSize();
			U32 nrToWrite = sectorSize - ((m_zStream.total_out+sizeof(U32)) % sectorSize);
			m_pMediator->Write((U8*)zeroBuffer, nrToWrite);
			m_dataPhysicalLength = m_zStream.total_out;
			//Write physical length.
			{
				Sys::C_StreamPos pos = m_pMediator->GetWritePosition();
				m_pMediator->SetWritePosition(m_physicalStartPos);
				m_pMediator->Write(m_dataPhysicalLength);
				m_pMediator->SetWritePosition(pos);
			}
			deflateEnd( &m_zStream );
			return true;
#endif
	}

	LOGERROR(g_log, "Failed to close ZLib compressed pak");
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressZLib::Flush(int flush)
{
	U32 nrToWrite = 0;
	do
	{
		// Compress
		m_zStream.next_out = m_buffer;
		m_zStream.avail_out = PAK_ZLIB_BUFFER_SIZE;

		S32 inflateResult = deflate(&m_zStream, flush);
		nrToWrite = PAK_ZLIB_BUFFER_SIZE - m_zStream.avail_out;

		// Write
		if (nrToWrite)
		{
			m_pMediator->Write(m_buffer, nrToWrite);

			if (m_pHashContext)
				m_pHashContext->UpdateHash(m_buffer, nrToWrite);
		}
		if (inflateResult == Z_STREAM_END) 
			break;
	} while (nrToWrite == PAK_ZLIB_BUFFER_SIZE);
	
	return true;
}

bool C_PakCompressZLib::Write(U8* buffer, U32 nrBytes)
{
	m_zStream.next_in = buffer;
	m_zStream.avail_in = nrBytes;

	Flush( Z_NO_FLUSH );

	return true;
}
#endif

bool C_PakCompressZLib::Read(U8* buffer, U32 nrBytes)
{
	if (nrBytes == 0)
		return true;

	S32 inflateResult = 0;
	m_zStream.next_out = buffer;
	m_zStream.avail_out = nrBytes;
	do
	{
		int flush = Z_SYNC_FLUSH;
		if (m_zStream.avail_in == 0)
		{
			// Read Data
			U32 nrToRead = PAK_ZLIB_BUFFER_SIZE;
			if (( m_dataPhysicalLength - m_zStream.total_in ) <= PAK_ZLIB_BUFFER_SIZE)
			{
				nrToRead = (U32)( m_dataPhysicalLength - m_zStream.total_in );
				flush = Z_SYNC_FLUSH;
			}
			BIZ_ASSERT(nrToRead != 0);

			if (!m_pMediator->Read(m_buffer, nrToRead, &m_physicalReadPos))
				return false;

			m_zStream.next_in = m_buffer;
			m_zStream.avail_in = nrToRead;

			if (m_pHashContext)
				m_pHashContext->UpdateHash(m_zStream.next_in, m_zStream.avail_in);
 		}

		// Uncompress Data
		inflateResult = inflate(&m_zStream, flush);
		#ifdef PLATFORM_PS3
		sys_timer_usleep( 100 );
		#endif
		if (inflateResult < 0)
		{
			BIZ_ASSERT_FAIL("ZLib Decompression Failure in PAK system.");
			return false;
		}

	} while ((m_zStream.avail_out > 0) && (inflateResult != Z_STREAM_END ));	

	return true;
}

}
