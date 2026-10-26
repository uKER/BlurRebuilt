#ifdef WIN32
#define NULL 0
#endif
#include "memory.h"
#include "Compression.h"
#include "ZLib.h"
#include <malloc.h>
#ifdef __GNUC__
#include <alloca.h>
#endif

#include <string.h>
#include <system\debuglog.h>
#include "network\debug.h"

bool CCompression::Decompress_Init(char *pOutputBuffer, int dwOutputBufferSize)
{
	int nRetCode;

	memset(&m_CompressionStream, 0, sizeof(m_CompressionStream));

	m_CompressionStream.zalloc		= NULL;
	m_CompressionStream.zfree		= NULL;
	m_CompressionStream.opaque		= NULL;
	
	m_CompressionStream.next_out	= (Bytef*)pOutputBuffer;
	m_CompressionStream.avail_out	= dwOutputBufferSize;

	nRetCode = inflateInit(&m_CompressionStream);

	return true;
}

int CCompression::Decompress_Term(void)
{
	inflateEnd(&m_CompressionStream);

	return m_CompressionStream.total_out;
}

bool CCompression::Decompress(char *pCompressedData, int dwCompressedSize)
{
	m_CompressionStream.next_in		= (Bytef*)pCompressedData;
	m_CompressionStream.avail_in	= dwCompressedSize;
	
	inflate(&m_CompressionStream, Z_FULL_FLUSH);

	return true;
}

void NetLog(char*,...);

bool CCompression::Decompress(char *pCompressedData, int dwCompressedSize, char *pOutputBuffer, int dwOutputBufferSize)
{
	int nRetCode;

	Decompress_Init(pOutputBuffer, dwOutputBufferSize);

	m_CompressionStream.next_in		= (Bytef*)pCompressedData;
	m_CompressionStream.avail_in	= dwCompressedSize;
	
	nRetCode = inflate(&m_CompressionStream, Z_FULL_FLUSH);
	if (nRetCode<0) // return code meanings are in zlib.h
	{
		LOGERROR( g_NetLog, "NETWORK DECOMPRESSION ERROR %d: %s\n",nRetCode, m_CompressionStream.msg);
	}

	Decompress_Term();

	return true;
}

/*
	Compression
*/

int ZEXPORT deflateEndX OF((z_streamp strm));

// Compress the original data into a newly created buffer of the same size - but data takes less room inside of it
bool CCompression::Compress(char *pOriginalData, int dwOriginalSize, char **ppCompressedBuffer, int* pdwCompressedSize)
{
	int nRetCode;

	memset(&m_CompressionStream, 0, sizeof(m_CompressionStream));

	char *pTempBufferOut = *ppCompressedBuffer;

	void *pBuf[5];

	pBuf[0] = alloca(64*1024);
	pBuf[1] = alloca(64*1024);
	pBuf[2] = alloca(64*1024);
	pBuf[3] = alloca(64*1024);
	pBuf[4] = alloca(6000);

	if (dwOriginalSize > 1024*1024)
		nRetCode = deflateInit(&m_CompressionStream, Z_DEFAULT_COMPRESSION, pBuf);
	else
		nRetCode = deflateInit(&m_CompressionStream, Z_BEST_SPEED, pBuf);

	m_CompressionStream.next_in		= (Bytef*) pOriginalData;
	m_CompressionStream.avail_in	= dwOriginalSize;
	
	m_CompressionStream.next_out	= (Bytef*)pTempBufferOut;
	m_CompressionStream.avail_out	= dwOriginalSize;

	nRetCode = deflate(&m_CompressionStream, Z_FULL_FLUSH);
	if (nRetCode<0)
	{
		LOGERROR( g_NetLog, "NETWORK COMPRESSION ERROR %d: %s\n",nRetCode, m_CompressionStream.msg);
	}

	*pdwCompressedSize = m_CompressionStream.total_out;

	deflateEnd(&m_CompressionStream);

	return true;
}
