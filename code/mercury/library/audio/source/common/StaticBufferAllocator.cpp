#include "AudioPch.h"
#include "../SystemHeaders.h"


////////////////////////////////////////////////////////////////////////////////
//	constructor
////////////////////////////////////////////////////////////////////////////////
Audio::StaticBufferAllocator::StaticBufferAllocator()
{
	m_pBufferMemory = 0;
	m_pbBufferInUse = 0;
	m_s32BufferSize = 0;
	m_s32NumBuffers = 0;
	m_bInited = false;
}


////////////////////////////////////////////////////////////////////////////////
//	destructor
////////////////////////////////////////////////////////////////////////////////
Audio::StaticBufferAllocator::~StaticBufferAllocator()
{
	if (m_pBufferMemory)
		BizAlignedFree(m_pBufferMemory);

	if (m_pbBufferInUse)
		delete [] m_pbBufferInUse;
}


////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
bool Audio::StaticBufferAllocator::Init(const S32 s32BufferSize, const S32 s32MaxNumBuffers, const S32 s32Alignment)
{
	if (s32BufferSize <= 0 || s32MaxNumBuffers <= 0)
		return false;

	m_s32BufferSize = (s32BufferSize + 15) & 0xfffffff0;
	m_s32NumBuffers = s32MaxNumBuffers;

	m_pBufferMemory = (void*)(BizAlignedMalloc(s32BufferSize * s32MaxNumBuffers, s32Alignment));
	m_pbBufferInUse = new bool[s32MaxNumBuffers];

	if (m_pbBufferInUse)
		memset(m_pbBufferInUse, 0, sizeof(bool) * s32MaxNumBuffers);

	m_bInited = ((m_pBufferMemory != 0) && (m_pbBufferInUse != 0));

	return m_bInited;
}


////////////////////////////////////////////////////////////////////////////////
//	GetBuffer
////////////////////////////////////////////////////////////////////////////////
void* Audio::StaticBufferAllocator::GetBuffer(const S32 s32Size)
{
	if ((! m_bInited) || s32Size > m_s32BufferSize)
	{
		AUDIO_SYS_FATAL("Audio state buffer allocation error");
		return 0;
	}

	int s32FreeBuffer = -1;

	m_Lock.Acquire();

	for (S32 i = 0; i < m_s32NumBuffers && s32FreeBuffer == -1; i++)
	{
		if (! m_pbBufferInUse[i])
		{
			m_pbBufferInUse[i] = true;
			s32FreeBuffer = i;
		}
	}

	m_Lock.Release();

	if (s32FreeBuffer == -1)
		return 0;

	return (void*)((U8*)m_pBufferMemory + m_s32BufferSize * s32FreeBuffer);
}


////////////////////////////////////////////////////////////////////////////////
//	FreeBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::StaticBufferAllocator::FreeBuffer(const void* pBuffer)
{
	if (! m_bInited)
		return false;

	S32 s32BufferNumber = ((U8*)pBuffer - (U8*)m_pBufferMemory) / m_s32BufferSize;
	m_pbBufferInUse[s32BufferNumber] = false;

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	FreeAllBuffers
////////////////////////////////////////////////////////////////////////////////
bool Audio::StaticBufferAllocator::FreeAllBuffers()
{
	for (U32 i = 0; i < (U32)m_s32NumBuffers; i++)
		m_pbBufferInUse[i] = false;

	return true;
}
