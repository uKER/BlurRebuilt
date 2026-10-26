#include "AudioPch.h"


Debug::Bool g_bDoCircularBufferMemcpys("Audio.DoCircularBufferMemcpys", true);


Audio::CircularBuffer::CircularBuffer(U32 u32BufferSize)
{
	Init(u32BufferSize);
}


void Audio::CircularBuffer::Init(U32 u32BufferSize)
{
	m_u32BufferSize = u32BufferSize;
	m_bEndOfStreamReached = false;
	m_bStarving = false;
	m_pu8Buffer = 0;
}


Audio::CircularBuffer::~CircularBuffer()
{
	if (m_pu8Buffer)
		delete [] m_pu8Buffer;
}


bool Audio::CircularBuffer::GetData(U8** ppData, U32 u32BytesRequired, U32* pu32BytesAvailable)
{
	U8* pSource = GetBuffer() + GetReadOffset();
	U8* pDest = *(ppData);
	U32 u32CurrBytesRequired = Maths::Min(u32BytesRequired, GetBytesAvailable());
	U32 u32FirstCopyNumBytes = Maths::Min(u32CurrBytesRequired, GetReadOffsetDistanceTilBufferEnd());

	IncrementReadOffset(u32CurrBytesRequired);

	*(pu32BytesAvailable) = u32CurrBytesRequired;

	if (g_bDoCircularBufferMemcpys)
		memcpy(pDest, pSource, u32FirstCopyNumBytes);

	pDest += u32FirstCopyNumBytes;
	u32CurrBytesRequired -= u32FirstCopyNumBytes;

	if (u32CurrBytesRequired == 0)
		return true;

	pSource = GetBuffer();
	if (g_bDoCircularBufferMemcpys)
		memcpy(pDest, pSource, u32CurrBytesRequired);

	return true;
}


bool Audio::CircularBuffer::WriteData(U8* pSourceData, U32 u32BytesAvailable, U32* pu32BytesWritten)
{
	Assert (HasSpace());

	U8* pCurrSource = pSourceData;
	U8* pCurrDest = GetBuffer() + GetWriteOffset();
	U32 u32MaxBytesToWrite =Maths::Min(u32BytesAvailable, GetSpaceInBytes());
	U32 u32FirstWriteNumBytes = Maths::Min(u32MaxBytesToWrite, GetWriteOffsetDistanceTilBufferEnd());

	IncrementWriteOffset(u32MaxBytesToWrite);

	*(pu32BytesWritten) = u32MaxBytesToWrite;

	if (g_bDoCircularBufferMemcpys)
		memcpy(pCurrDest, pCurrSource, u32FirstWriteNumBytes);

	if (u32MaxBytesToWrite == u32FirstWriteNumBytes)
		return true;

	pCurrSource += u32FirstWriteNumBytes;
	pCurrDest = GetBuffer();

	if (g_bDoCircularBufferMemcpys)
		memcpy(pCurrDest, pCurrSource, u32MaxBytesToWrite - u32FirstWriteNumBytes);

	return true;
}


U32 Audio::CircularBuffer::GetBytesAvailable()
{
	U32 u32SanityCheck = ((m_u32WriteOffset + m_u32BufferSize) - m_u32ReadOffset) % m_u32BufferSize;
	U32 u32BytesAvailable = m_u32BytesAvailable;

	Assert(u32BytesAvailable <= m_u32BufferSize);

	if (u32BytesAvailable % m_u32BufferSize)
		Assert(u32BytesAvailable == u32SanityCheck);

	return m_u32BytesAvailable;
}


bool Audio::CircularBuffer::BecomeActive()
{
	if (m_pu8Buffer)
		return false;

	m_pu8Buffer = new U8[m_u32BufferSize];

	m_bEndOfStreamReached = false;
	m_bStarving = false;
	m_u32ReadOffset = 0;
	m_u32WriteOffset = 0;
	m_u32BytesAvailable = 0;

	return true;
}


bool Audio::CircularBuffer::BecomeInactive()
{
	if (! m_pu8Buffer)
		return false;

	delete [] m_pu8Buffer;
	m_pu8Buffer = 0;

	return true;
}


void Audio::CircularBuffer::IncrementReadOffset(U32 u32NumBytes)
{
	U32 u32CurrentBytesAvailable = m_u32BytesAvailable;
	u32CurrentBytesAvailable -= u32NumBytes;
	m_u32BytesAvailable = u32CurrentBytesAvailable;

	U32 u32CurrentReadOffset = GetReadOffset();
	U32 u32FinalReadOffset = (u32CurrentReadOffset + u32NumBytes) % m_u32BufferSize;

	m_u32ReadOffset = u32FinalReadOffset;
}


void Audio::CircularBuffer::IncrementWriteOffset(U32 u32NumBytes)
{
	U32 u32CurrentWriteOffset = GetWriteOffset();
	U32 u32FinalWriteOffset = (u32CurrentWriteOffset + u32NumBytes) % m_u32BufferSize;

	m_u32WriteOffset = u32FinalWriteOffset;

	U32 u32CurrentBytesAvailable = m_u32BytesAvailable;
	u32CurrentBytesAvailable += u32NumBytes;
	m_u32BytesAvailable = u32CurrentBytesAvailable;
}
