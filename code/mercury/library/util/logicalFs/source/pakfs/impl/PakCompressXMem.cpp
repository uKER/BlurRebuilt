#include "LogicalFsPch.h"

//XCompress
#include <xcompress.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressXMem.h>
#include <PakFs/Impl/PakSha1.h>

#define PAK_XMEM_WRITE_BUFFER_SIZE 256*1024

namespace Pak
{

bool XMemInit(C_XMemStream& stream)
{
	stream.totalLogical = 0;
	stream.totalPhysical = 0;
	stream.readRemainingPhysicalBytesFromDisk = 0;
	stream.readPhysicalDiskBytesProcessed = 0;

	HRESULT hr = E_FAIL;
	if (stream.compressing)
	{
		if (FAILED(hr = XMemCreateCompressionContext(
			XMEMCODEC_LZX, 
			0, 
			XMEMCOMPRESS_STREAM, 
			&stream.context)))
		{
			BIZ_ASSERT_FAIL("Failed to create XMem compression context!");
			return false;
		}
	}
	else
	{
		if (FAILED(hr = XMemCreateDecompressionContext(
			XMEMCODEC_LZX, 
			0, 
			XMEMCOMPRESS_STREAM, 
			&stream.context)))
		{
			BIZ_ASSERT_FAIL("Failed to create XMem decompression context!");
			return false;
		}
	}
	return true;
}

void XMemEnd(C_XMemStream& stream)
{
	if (stream.compressing)
	    XMemDestroyCompressionContext(stream.context);
	else
		XMemDestroyDecompressionContext(stream.context);
}

bool XMemRestart(C_XMemStream& stream)
{
	stream.totalLogical = 0;
	stream.totalPhysical = 0;
	stream.readRemainingPhysicalBytesFromDisk = 0;
	stream.readPhysicalDiskBytesProcessed = 0;

	HRESULT hr = E_FAIL;
	if (stream.compressing)
	{
		if (FAILED(hr = XMemResetCompressionContext(stream.context)))
		{
			LOGERROR(g_log, "Failed to reset XMem compression stream!");
			return false;
		}
	}
	else
	{
		if (FAILED(hr = XMemResetDecompressionContext(stream.context)))
		{
			LOGERROR(g_log, "Failed to reset XMem decompression stream!");
			return false;
		}
	}

	return true;
}

bool XMemInflate(C_XMemStream& stream, U8* pOutputBuf, SIZE_T* outputSize, U8* pInputBuf, SIZE_T* inputSize)
{
	HRESULT hr = E_FAIL;
    // Decompress the data from the pCompressedData buffer 
    // and copy it to the pDecompressedData buffer.
    if (FAILED(hr = XMemDecompressStream(
		stream.context, 
		pOutputBuf, 
		outputSize, 
		pInputBuf, 
		inputSize)))
	{
		LOGERROR(g_log, "Failure in XMem decompression!");
		return false;
	}

	return true;
}

bool XMemDeflate(C_XMemStream& stream, U8* pOutputBuf, SIZE_T* outputSize, U8* pInputBuf, SIZE_T* inputSize)
{
	HRESULT hr = E_FAIL;
    // Compress the data from the pData buffer 
    // and copy it to the pCompressedData buffer.
    hr = XMemCompressStream(
		stream.context,
		pOutputBuf, 
		outputSize, 
		pInputBuf, 
		inputSize);
	if(FAILED( hr ) || hr == XMCDERR_MOREDATA)
	{
		LOGERROR(g_log, "Failure in XMem compression!");
		return false;
	}

	return true;
}

C_PakCompressXMem::C_PakCompressXMem(U32 flags)
{
	m_pMediator = 0;
	m_physicalStartPos = 0;
	m_physicalReadPos = 0;

	m_fileAccessAttributes = 0;
	//Physical size of compressed data.
	m_dataPhysicalLength = 0;
}

C_PakCompressXMem::~C_PakCompressXMem()
{
}

bool C_PakCompressXMem::Open(
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
			memset(&m_xmemStream, 0, sizeof(m_xmemStream));
			if (!XMemInit(m_xmemStream))
				return false;
			return m_pMediator->Read(m_dataPhysicalLength, &m_physicalReadPos);
#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			memset(&m_xmemStream, 0, sizeof(m_xmemStream));
			m_xmemStream.compressing = true;
			// Create a compression context.
			if (!XMemInit(m_xmemStream))
				return false;

			//Write space for physical length
			m_pMediator->SetWritePosition(physicalStartPos);
			return m_pMediator->Write(m_dataPhysicalLength);
#endif
		default:
			break;
	}
		
	LOGERROR(g_log, "Failed to open XMem compressed pak!");
	return false;
}

bool C_PakCompressXMem::Restart(U32 streamToLogicalPos)
{
	HRESULT hr = E_FAIL;
	// Re-initialise the stream
	if (!XMemRestart(m_xmemStream))
		return false;

	m_physicalReadPos = m_physicalStartPos;
	{
		// Read In The Length
		if (!m_pMediator->Read(m_dataPhysicalLength, &m_physicalReadPos))
			return false;
	}
	// Stream in to logicalPosition
	const U32 bufferSize = PAK_XMEM_READ_BUFFER_SIZE;
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

U32 C_PakCompressXMem::GetPhysicalLength() const
{
	return m_dataPhysicalLength;
}

bool C_PakCompressXMem::Close()
{
	switch(m_fileAccessAttributes)
	{
		case Sys::E_FileAccess_Read:
			XMemEnd(m_xmemStream);
			return true;

#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			//inputSize of 0 terminates stream.
			Flush(NULL, 0);
			U8 zeroBuffer[PAK_SECTOR_SIZE_MAX] = {0};
			//The U32 term below is the physical length.
			U32 sectorSize = m_pMediator->GetSectorSize();
			U32 nrToWrite = sectorSize - ((m_xmemStream.totalPhysical+sizeof(U32)) % sectorSize);
			m_pMediator->Write((U8*)zeroBuffer, nrToWrite);
			m_dataPhysicalLength = m_xmemStream.totalPhysical;
			//Write physical length.
			{
				Sys::C_StreamPos pos = m_pMediator->GetWritePosition();
				m_pMediator->SetWritePosition(m_physicalStartPos);
				m_pMediator->Write(m_dataPhysicalLength);
				m_pMediator->SetWritePosition(pos);
			}
			XMemEnd(m_xmemStream);
			return true;
#endif
	}

	LOGERROR(g_log, "Failed to close XMem compressed pak");
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressXMem::Flush(U8* pSrc, SIZE_T srcSize)
{
	U8 outputBuf[PAK_XMEM_WRITE_BUFFER_SIZE];
	SIZE_T outputSize = sizeof(outputBuf);

	SIZE_T inputSize = srcSize;
	//Flush
	{
		if (!XMemDeflate(m_xmemStream, outputBuf, &outputSize, pSrc, &srcSize))
			return false;
		m_xmemStream.totalLogical += srcSize;

		//If compressed bytes were emitted.
		if (outputSize)
		{
			m_xmemStream.totalPhysical += outputSize;
			m_pMediator->Write(outputBuf, outputSize);

			if (m_pHashContext)
				m_pHashContext->UpdateHash(outputBuf, outputSize);
		}
	}

	return true;
}

bool C_PakCompressXMem::Write(U8* buffer, U32 nrBytes)
{
	return Flush(buffer, nrBytes);
}

#endif

bool C_PakCompressXMem::Read(U8* buffer, U32 logicalBytesRequested)
{
	if (logicalBytesRequested == 0)
		return true;

	SIZE_T inputSize(0);
	U32 logicalBytesServed = 0;
	while(logicalBytesServed < logicalBytesRequested)
	{
		//Potential physical input starvation...
		if (m_xmemStream.readRemainingPhysicalBytesFromDisk == 0)
		{
			U32 remainingLength = m_dataPhysicalLength - m_xmemStream.totalPhysical;
			U32 physicalBytesToRead = remainingLength > sizeof(m_readBuffer) ?  
				sizeof(m_readBuffer) : remainingLength;

			//Stream requires flush?
			if (physicalBytesToRead == 0)
			{
				U8* pOutputBuf = buffer+logicalBytesServed;
				SIZE_T outputSize = logicalBytesRequested-logicalBytesServed;
				U8* pInputBuf = m_readBuffer + inputSize;
				
				BIZ_ASSERT(outputSize > 0);
				//Complete stream.
				if (!XMemInflate(m_xmemStream, pOutputBuf, &outputSize, pInputBuf, &inputSize))
				{
					BIZ_ASSERT_FAIL("XMem Decompression Failure in PAK system.");
					return false;
				}
				BIZ_ASSERT(inputSize == 0);
				return true;
			}

			//Physical stream starvation.
			if (!m_pMediator->Read(m_readBuffer, physicalBytesToRead, &m_physicalReadPos))
			{
				LOGERROR(g_log, "Invalid read position within XMem pak!");
				return false;
			}
			m_xmemStream.readPhysicalDiskBytesProcessed		= 0;
			m_xmemStream.readRemainingPhysicalBytesFromDisk	= physicalBytesToRead;
		}

		U8* pOutputBuf = buffer+logicalBytesServed;
		SIZE_T outputSize = logicalBytesRequested-logicalBytesServed;
		inputSize = m_xmemStream.readRemainingPhysicalBytesFromDisk;
		U8* pInputBuf = m_readBuffer + m_xmemStream.readPhysicalDiskBytesProcessed;
		
		BIZ_ASSERT(outputSize > 0 && inputSize > 0);
		if (!XMemInflate(m_xmemStream, pOutputBuf, &outputSize, pInputBuf, &inputSize))
		{
			BIZ_ASSERT_FAIL("XMem Decompression Failure in PAK system.");
			return false;
		}

		if (m_pHashContext)
			m_pHashContext->UpdateHash(pInputBuf, inputSize);
		
		m_xmemStream.readRemainingPhysicalBytesFromDisk -= inputSize;
		m_xmemStream.readPhysicalDiskBytesProcessed += inputSize;
		m_xmemStream.totalPhysical += inputSize;
		m_xmemStream.totalLogical += outputSize;
		logicalBytesServed += outputSize;
	}

	return true;
}

}
