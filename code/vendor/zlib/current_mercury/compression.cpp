#include <string.h>


#ifdef SPU

#include "stdint.h"
#include "stdio.h"

typedef ::std::uint64_t		U64;
typedef ::std::int64_t		S64;
typedef ::std::uint32_t		U32;
typedef ::std::int32_t		S32;
typedef unsigned	short	U16;
typedef				short	S16;
typedef unsigned	char	U8;
#define assert

#else

#include "Core\Core.h"
#include "System\DebugLog.h"

#endif
#include "Compression.h"
#include "ZLib.h"

#ifndef SPU
static Debug::Logger &g_ZlibLog = Debug::GetLogger("vendor.zlib");
#endif
#ifdef PLATFORM_XENON
#include <xtl.h>
#endif

#ifdef PLATFORM_PS3
#include "vec_types.h"

#define QUADMEMCPY( dest, source, size )			\
	{												\
		U32 nrQWords = size >> 4;					\
		qword* pSrc = (qword*)source;				\
		qword* pDest = (qword*)dest;				\
		for (U32 x = 0 ; x < nrQWords ; x++)		\
		{											\
			*pDest = *pSrc;							\
			pDest++;								\
			pSrc++;									\
		}											\
	}												\

#endif

voidpf zlibAlloc(voidpf opaque, uInt items, uInt size)
{
	return new char[items * size];
}
 
void zlibFree(voidpf opaque, voidpf address)
{
	delete [] (char*)address;
}

CCompression::CCompression(bool createDecompressionContext) : m_createdDecompressionContext(createDecompressionContext)
{
	if (createDecompressionContext)
	{
		memset(&m_CompressionStream, 0, sizeof(m_CompressionStream));

#ifndef CFG_SPEED
		m_CompressionStream.zalloc		= zlibAlloc;
		m_CompressionStream.zfree		= zlibFree;
#else
		m_CompressionStream.zalloc		= NULL;
		m_CompressionStream.zfree		= NULL;
#endif

		m_CompressionStream.opaque		= NULL;
		
		m_CompressionStream.next_out	= 0;
		m_CompressionStream.avail_out	= 0;

		int nRetCode = inflateInit(&m_CompressionStream);

		assert(nRetCode == Z_OK);
	}
}

CCompression::~CCompression()
{
	if (m_createdDecompressionContext)
		inflateEnd(&m_CompressionStream);
}

bool CCompression::Decompress_Init(U8 *pOutputBuffer, U32 dwOutputBufferSize)
{
	assert(m_createdDecompressionContext == true);

	m_CompressionStream.next_out	= pOutputBuffer;
	m_CompressionStream.avail_out	= dwOutputBufferSize;

	int nRetCode = inflateReset(&m_CompressionStream);

	assert(nRetCode == Z_OK);

	return true;
}

void CCompression::Decompress_SetOutputBuffer(U8 *pOutputBuffer, U32 dwOutputBufferSize)
{
	m_CompressionStream.next_out	= pOutputBuffer;
	m_CompressionStream.avail_out	= dwOutputBufferSize;
}

U32 CCompression::TotalOut() const
{
	return m_CompressionStream.total_out;
}

U32 CCompression::TotalIn() const
{
	return m_CompressionStream.total_in;
}

U32 CCompression::AvailOut() const
{
	return m_CompressionStream.avail_out;
}

U32 CCompression::AvailIn() const
{
	return m_CompressionStream.avail_in;
}



bool CCompression::Decompress(U8 *pCompressedData, U32 dwCompressedSize)
{
	bool useOldInflateMethod = false;
//#if defined(PLATFORM_XENON) || defined(PLATFORM_PS3)
//	useOldInflateMethod = true;
//#endif
	
	if ( useOldInflateMethod )
	{
		assert(m_createdDecompressionContext == true);

		m_CompressionStream.next_in		= pCompressedData;
		m_CompressionStream.avail_in	= dwCompressedSize;
		
		int nRetCode = inflate(&m_CompressionStream, Z_SYNC_FLUSH);

		if (nRetCode != Z_OK && nRetCode != Z_STREAM_END)
		{
			LOGFATAL( g_ZlibLog, "ZLib did -NOT- return success for decompression\n");
			return false;
		}
		return true;
	}
	else
// END OF TEMP CODE
	{
		int nRetCode = 0;
		U8	tempBuffer[ 32768 ];
		U32 outputBufferSize = sizeof( tempBuffer );
		U8  *pOut = m_CompressionStream.next_out;
		int destSize = m_CompressionStream.avail_out;
		
		int thisOut = 0;
		m_CompressionStream.next_in		= pCompressedData;
		m_CompressionStream.avail_in	= dwCompressedSize;
		do
		{
			outputBufferSize = destSize < sizeof(tempBuffer) ? destSize : sizeof(tempBuffer);
			m_CompressionStream.next_out = tempBuffer;
			m_CompressionStream.avail_out = outputBufferSize;
			nRetCode = inflate(&m_CompressionStream, Z_SYNC_FLUSH);
			int bytesToCopy = outputBufferSize - m_CompressionStream.avail_out;
			if (nRetCode < 0)
			{
#ifndef SPU
				LOGFATAL( g_ZlibLog, "ZLib did -NOT- return success for decompression!\n");
				if (m_CompressionStream.msg)
				{
					LOGFATAL( g_ZlibLog, "%s", m_CompressionStream.msg);
				}
#endif
#ifndef BUILD_CONFIG_RELEASE 
			return false;
#endif
			}
#if defined( PLATFORM_XENON )
			XMemCpyStreaming( pOut, &tempBuffer, bytesToCopy );
#elif defined (PLATFORM_PS3)
			memcpy( pOut, &tempBuffer, bytesToCopy );
#else
			memcpy( pOut, &tempBuffer, bytesToCopy );
#endif
			pOut += bytesToCopy ;
			thisOut += bytesToCopy;
			if (nRetCode == Z_STREAM_END)
			{
				m_CompressionStream.next_out = tempBuffer;
				m_CompressionStream.avail_out = sizeof(tempBuffer);
				nRetCode = inflate(&m_CompressionStream, Z_FINISH);
				break;
			}
			if ((destSize - thisOut) == 0) break;
			if (m_CompressionStream.avail_in == 0) break;
		} while (1);
		m_CompressionStream.avail_out = ( destSize - thisOut );
		m_CompressionStream.next_out = pOut;
		return true;
	}
}


#ifdef OLD_INFLATE_CODE
bool CCompression::Decompress(U8 *pCompressedData, U32 dwCompressedSize )
{
}
#endif


const char* GetZLibErrorCodeMsg(int retCode)
{
	switch (retCode)
	{
	case Z_OK:
		return "Z_OK";
	case Z_STREAM_END:
		return "Z_STREAM_END";
	case Z_NEED_DICT:
		return "Z_NEED_DICT";
	case Z_ERRNO:
		return "Z_ERRNO";
	case Z_STREAM_ERROR:
		return "Z_STREAM_ERROR";
	case Z_DATA_ERROR:
		return "Z_DATA_ERROR";
	case Z_MEM_ERROR:
		return "Z_MEM_ERROR";
	case Z_BUF_ERROR:
		return "Z_BUF_ERROR";
	case Z_VERSION_ERROR:
		return "Z_VERSION_ERROR";
	default:
		assert(false);
		return "Unknown Error Code";
	}
}

bool CCompression::Decompress(U8 *pCompressedData, U32 dwCompressedSize, U8 *pOutputBuffer, U32 dwOutputBufferSize)
{
	assert(pOutputBuffer != NULL);

	Decompress_Init(pOutputBuffer, dwOutputBufferSize);

	return Decompress(pCompressedData, dwCompressedSize);
}
/*
 * Compression
 */

// Compress the original data into a newly created buffer of the same size - but data takes less room inside of it
bool CCompression::Compress(U8 *pOriginalData, U32 dwOriginalSize, U8 **ppCompressedBuffer, U32* pdwCompressedSize)
{
	assert(m_createdDecompressionContext == false);

	int nRetCode;

	memset(&m_CompressionStream, 0, sizeof(m_CompressionStream));

#ifndef CFG_SPEED
	m_CompressionStream.zalloc		= zlibAlloc;
	m_CompressionStream.zfree		= zlibFree;
#else
	m_CompressionStream.zalloc		= NULL;
	m_CompressionStream.zfree		= NULL;
#endif

	U8 *pTempBufferOut = *ppCompressedBuffer = new U8[dwOriginalSize];

	nRetCode = deflateInit(&m_CompressionStream, Z_BEST_COMPRESSION); // Z_DEFAULT_COMPRESSION);

	assert(nRetCode == Z_OK);

	m_CompressionStream.next_in		= (Bytef*) pOriginalData;
	m_CompressionStream.avail_in	= dwOriginalSize;
	
	m_CompressionStream.next_out	= pTempBufferOut;
	m_CompressionStream.avail_out	= dwOriginalSize;

	nRetCode = deflate(&m_CompressionStream, Z_FULL_FLUSH);

	assert(nRetCode == Z_OK);

	*pdwCompressedSize = m_CompressionStream.total_out;

	deflateEnd(&m_CompressionStream);

	return true;
}

