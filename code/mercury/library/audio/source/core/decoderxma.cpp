#include "AudioPch.h"
#include "coreheaders.h"
#include "../SystemHeaders.h"

//#include "../common/file.h"
#include "..\common\lock.h"

#include <System/memory/MemoryTracker.h>


#ifdef XMA_CONTEXT_DEBUG_LIST
Btl::List <const char*> activeWavNames;
Audio::Core::Lock g_Lock;

void AddToWavNames(const char* wavName)
{
	g_Lock.Acquire();
	activeWavNames.PushBack(wavName);
	g_Lock.Release();
}

void RemoveFromWavNames(const char* wavName)
{
	g_Lock.Acquire();
	Btl::List<const char*>::Iterator i = activeWavNames.Begin();
	for (; i != activeWavNames.End(); ++i)
	{
		if (strcmp(*i, wavName) == 0)
		{
			activeWavNames.Erase(i);
			g_Lock.Release();
			return;
		}
	}
	g_Lock.Release();
}

void DumpWavNames()
{
	g_Lock.Acquire();
	Btl::List<const char*>::Iterator i = activeWavNames.Begin();
	for (; i != activeWavNames.End(); ++i)
	{
		AUDIO_CORE_WARN("Active: %s", *i);
	}
	g_Lock.Release();
}
#endif	//	#ifdef XMA_CONTEXT_DEBUG_LIST

//#include <Assert.h>

//using namespace Audio;
#define XMA_DOUBLE_BUFFER

// uses temporary buffers for submitted data which is currently being decoded.
// when it submits a packet of encoded data to the decoder it allocates a buffer and copies the encoded data
// from the waveserver into it. when the decoder has finished decoding the data we free the buffer.
// this is needed because the WaveServer moves data around in memory after we access it, which
// causes the decoder to go bang as reads the data directly from the memory we give it.
#define TEMP_XMA_BUFFERING

Debug::Bool g_bRandomlyCorruptData("Audio.System.Streamer.RandomlyCorruptXMAStream", false);
Debug::Bool g_debugAudioBlockXMA("Audio.XMALoopingFixTest", false);
Debug::Int g_StreamingXmaSubmitChunkSize("Audio.System.Streamer.StreamingXmaSubmitDataChunkSize", 2048);
Debug::Bool g_bAudioPrintXmaStreamerDebug("Audio.System.Streamer.XmaStreamingDebugPrints", false);

Audio::Core::Lock Audio::Core::XMADecoder::m_Lock; 
Audio::Core::Lock Audio::Core::XMADecoderBuffer::m_Lock;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

//#define DEFERRED_XMA
#define RELEASE_XMA_ON_SKIP
#define DESTROY_ALL_XMA_ON_UPDATE

#define XMA_PACKET_SIZE 2048
#define XMA_PACKET_HEADER_SIZE 4
#define XMA_PACKET_PAYLOAD_SIZE 2044
#define XMA_SUBFRAME_SIZE 128
#define XMA_FRAME_SIZE 512

const F32 f32SampleConv = 1.f / 32768.f;

XMA_BufferAllocator* XMA_BufferAllocator::m_pXMA_PlaybackBuffer = NULL;

U32 XMA_BufferAllocator::m_PeakMemoryUsed = 0;

XMA_BufferAllocator::XMA_BufferAllocator()
    : m_Entries( 0 )
    , m_MemoryFree( m_MemorySize )
{
	MEMORY_TRACKER_PUSH("Audio.XmaDecoderBuffers");
    m_pMemoryBase = ( Audio::System::Alloc(m_MemorySize, 0, Audio::System::ePhysical) );
    m_pMemoryFree = ( (char*)m_pMemoryBase );
	MEMORY_TRACKER_POP("Audio.XmaDecoderBuffers");

    if ( (U32)m_pMemoryBase & 127 ) DebugBreak();
    
    for ( U32 i=0; i<m_MaxEntries; i++ )
    {
        m_Size[i] = -1;
    }
}

XMA_BufferAllocator::~XMA_BufferAllocator()
{
	MEMORY_TRACKER_PUSH("Audio.XmaDecoder.XmaDecoderBuffers");
	Audio::System::Free(m_pMemoryBase, Audio::System::ePhysical);
	MEMORY_TRACKER_POP("Audio.XmaDecoder.XmaDecoderBuffers");
	
    m_pMemoryBase = NULL;
}

// TODO: allow alignment to be passed in?
void* XMA_BufferAllocator::Alloc( const U32 input_size)
{
    const U32 size = ( input_size + 127 ) & ~127;
    
	m_Lock.Acquire();

	// have we got a free block of the same size as we are asking for?
	// look through array of free blocks and see if we have a free one
    for ( U32 i=0; i<m_Entries; i++ )		
    {
        if ( m_Size[i] == size)			// is this array of slots the same size as we want?
        {
            void* ptr = m_FixedSizeFreeBlocks[i].Alloc();
            if ( ptr )
            {
                XMemSet128( ptr, 0, size );
				m_Lock.Release();

				#ifndef CFG_GOLD
				if (m_MemorySize-m_MemoryFree > m_PeakMemoryUsed) 
					m_PeakMemoryUsed = m_MemorySize-m_MemoryFree;
				#endif

                return ptr;
            }
            else break;
        }
    }
    
	m_Lock.Release();

	// if we got here there are no free blocks of the desired size so 'alloc' a new one from the contigious memory chunk
    if ( m_MemoryFree >= size )		
    {
        void* ptr = m_pMemoryFree;

        m_pMemoryFree += size;
        m_MemoryFree -= size;
        XMemSet128( ptr, 0, size );
		#ifndef CFG_GOLD
		if (m_MemorySize-m_MemoryFree > m_PeakMemoryUsed) 
			m_PeakMemoryUsed = m_MemorySize-m_MemoryFree;
		#endif

        return ptr;
    }
    


    AUDIO_CORE_FATAL( "XMA_BufferAllocator::Alloc() : m_MemorySize needs to be increased! [m_MemorySize = %d]\n", m_MemorySize );
    return NULL;
}

void XMA_BufferAllocator::Free( void* ptr, const U32 input_size)
{
    const U32 size = ( input_size + 127 ) & ~127;
    
	m_Lock.Acquire();

	for ( U32 i=0; i<m_Entries; i++ )
    {
		// if we've already got a list of this sized free blocks then add it to that list
        if ( m_Size[i] == size)
        {
            m_FixedSizeFreeBlocks[i].Free( ptr );
			m_Lock.Release();
            return;
        }
    }
    
    if ( m_Entries < m_MaxEntries )
    {
		// we haven't got a list of this sized memory block so 'make' one and add it to it
        const U32 index = m_Entries++;
        m_Size[index] = size;
        m_FixedSizeFreeBlocks[index].Free( ptr );
		m_Lock.Release();
        return;
    }
    
	m_Lock.Release();

    AUDIO_CORE_FATAL( "XMA_BufferAllocator::Free() : m_MaxEntries needs to be increased! [m_MaxEntries = %d]\n", m_MaxEntries );
}

void XMA_BufferAllocator::Init()
{
	if (!m_pXMA_PlaybackBuffer)
		m_pXMA_PlaybackBuffer = new XMA_BufferAllocator;
}

// remove from free list
void* XMA_FixedSizeBufferArray::Alloc()
{
	if ( m_Entries )
	{
		void* ptr = m_PtrArray[ --m_Entries ];
		if ( !ptr ) DebugBreak();
		return ptr;
	}
	return NULL;
}

// put into the free list
void XMA_FixedSizeBufferArray::Free( void* ptr )
{
	if ( !ptr ) DebugBreak();

	if (m_Entries < (U32)m_PtrArray.Size())
		m_PtrArray[m_Entries] = ptr;
	else
		m_PtrArray.PushBack(ptr);

	m_Entries++;
}

////////////////////////////////////////////////////////////////////////////////
//	XMADecoderBuffer
////////////////////////////////////////////////////////////////////////////////
Audio::Core::XMADecoderBuffer::XMADecoderBuffer()
{
	System::Get()->PushCurrentMemoryAllocationId(4);
	MEMORY_TRACKER_PUSH("Audio.XmaDecoderBuffers");

#ifdef XMA_DOUBLE_BUFFER
	m_asBuffer = (S16*)Audio::System::Alloc((2 * 2 * AUDIO_XMA_MAX_NUM_HW_DECODERS * AUDIO_XMA_NUM_CACHE_LINES * 128) * sizeof(S16), 128, Audio::System::eVirtual);
	ZeroMemory(m_asBuffer, 2 * 2 * AUDIO_XMA_MAX_NUM_HW_DECODERS * AUDIO_XMA_NUM_CACHE_LINES * 128 * sizeof(S16));
#else
	m_asBuffer = (S16*)Audio::System::Alloc((2 * AUDIO_XMA_MAX_NUM_HW_DECODERS * AUDIO_XMA_NUM_CACHE_LINES * 128) * sizeof(S16), 128, Audio::System::eVirtual);
	ZeroMemory(m_asBuffer, 2 * AUDIO_XMA_MAX_NUM_HW_DECODERS * AUDIO_XMA_NUM_CACHE_LINES * 128 * sizeof(S16));
#endif
	ZeroMemory(m_bUsed, sizeof(m_bUsed));

	MEMORY_TRACKER_POP("Audio.XmaDecoderBuffers");
	System::Get()->PopCurrentMemoryAllocationId();
}

Audio::Core::XMADecoderBuffer::~XMADecoderBuffer()
{
	MEMORY_TRACKER_PUSH("Audio.XmaDecoderBuffers");
	Audio::System::Free(m_asBuffer, Audio::System::eVirtual);
	MEMORY_TRACKER_POP("Audio.XmaDecoderBuffers");
}

U32 Audio::Core::XMADecoderBuffer::Allocate(S16*& ptr)
{
	MT::AutoLock lock(m_ThreadLock);

#ifdef XMA_DOUBLE_BUFFER
	for (U32 iCount=0; iCount < 2 * AUDIO_XMA_MAX_NUM_HW_DECODERS; iCount++)
#else
	for (U32 iCount=0; iCount < AUDIO_XMA_MAX_NUM_HW_DECODERS; iCount++)
#endif
	{
		if (m_bUsed[iCount] == false)
		{
			m_bUsed[iCount]	= true;
			ptr				= &m_asBuffer[2 * iCount * AUDIO_XMA_NUM_CACHE_LINES * 128];
			
			return iCount;
		}
	}
	return U32(-1);
}

void Audio::Core::XMADecoderBuffer::Free(U32 index)
{
	MT::AutoLock lock(m_ThreadLock);

#ifdef XMA_DOUBLE_BUFFER
	Assert(index < 2 * AUDIO_XMA_MAX_NUM_HW_DECODERS);
#else
	Assert(index < AUDIO_XMA_MAX_NUM_HW_DECODERS);
#endif
	m_bUsed[index] = false;
}

////////////////////////////////////////////////////////////////////////////////
//	XMADecoder
////////////////////////////////////////////////////////////////////////////////
Audio::Core::XMADecoder::XMADecoder(U32 uXmaFormat, bool bSRC, WaveData& waveData) : m_waveData(waveData)
{
    m_state.pXmaPlayback = NULL;
    m_state.bLoop = false;
    m_state.bActive = false;
    m_state.uPacket = 0;
    m_state.uStartSample = 0;
    m_state.fFraction = 0.f;
	m_state.fPrevPitch = 1.f;
    m_state.fPitch = 1.f;
    m_state.sPrevious = 0;
    m_state.bPlaying = false;
    m_state.bStarving = true;
	m_state.bResumed = false;
	m_state.bStreamPrimed = false;
    m_state.u32CallbackId = 0;
    m_state.u32StateOffset = 0;
	m_state.bUseSrc = bSRC;
	m_uWaveDataOffset = 0;

	switch(m_waveData.uMaxPitchDown)
	{
	case 0:
		m_state.uOctaveMaxPitch = 2;
		break;
	case 1:
		m_state.uOctaveMaxPitch = 4;
		break;
	case 2:
		m_state.uOctaveMaxPitch = 8;
		break;
	default:
		m_state.uOctaveMaxPitch = 4;//m_waveData.uMaxPitch
		break;
	}

	m_state.uOctaveMaxPitch = 8;

	//m_waveData.bLoop = false;
	//m_waveData.psData = NULL;
	//m_waveData.uChannels = 1;
	//m_waveData.uLength = 0;
	//m_waveData.uSampleRate = 48000;
	//m_waveData.uLoopStartOffsetBit = 0;
	//m_waveData.uLoopEndOffsetBit = 0;
	//m_waveData.uSubframeData = 0;

#ifdef DEFERRED_XMA
    XMA_PLAYBACK_INIT xmaInit;
    xmaInit.channelCount = m_waveData.uChannels;
    xmaInit.outputBufferSizeInSamples = m_state.uOctaveMaxPitch * 128;
    xmaInit.sampleRate = m_waveData.uSampleRate;
    xmaInit.subframesToDecode = 8;

	//MEMORY_TRACKER_PUSH_DYNAMIC("oldtech.audio.XMA Decoder");
    if (S_OK != XMAPlaybackCreate(1, &xmaInit, XMA_PLAYBACK_CREATE_DEFER_CONTEXT_ALLOCATION, &m_state.pXmaPlayback))
        AUDIO_CORE_ERROR("XMADecoder() cannot create XMA playback");

	//MEMORY_TRACKER_POP_DYNAMIC("oldtech.audio.XMA Decoder");

#endif

		m_state.uProgress = 0;
	//m_state.bLoop = pl.numLoops == 255 ? true : false;
	m_state.bLoop = m_waveData.bLoop;

	m_state.m_fB = Maths::ArcTan(Maths::fPi * 0.005f);
	m_state.m_fB = -(m_state.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_state.m_fB);
	m_state.m_fA = 0.5f * (1.0f - m_state.m_fB);
    
	m_state.fPrevSample = 0.f;
    
    #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_DECODER_XMA ) // JASOND
	m_state.fPrevIn = 0.f;
	m_state.fPrevOut = 0.f;
	#else
    m_state.m_vA = Maths::_Scalar( m_state.m_fA );
    m_state.m_vB = Maths::_Scalar( m_state.m_fB );
    m_state.m_vPrevIn = Maths::MakeVector4( Maths::g_VectorZero );
    m_state.m_vPrevOut = Maths::MakeVector4( Maths::g_VectorZero );
    #endif
	
	m_state.fPrevSampleL = 0.f;

	m_state.fPrevInL = 0.f;
	m_state.fPrevOutL = 0.f;

	m_state.fPrevSampleR = 0.f;

	m_state.fPrevInR = 0.f;
	m_state.fPrevOutR = 0.f;

	m_state.sIndex = 0;
	
	//  num samples. this is num samples in  temp buffer and not num samples in this sample
	m_state.sNumSamples = m_state.uOctaveMaxPitch * 128;

#ifdef XMA_DOUBLE_BUFFER
	m_uIndex[0] = 0;
	m_uIndex[1] = 0;

	m_uRead = 0;
	m_uWrite = 0;
	m_uWritten = 0;
#else
	m_uIndex = 0;
#endif

	m_uTotalXMA++;
	
    m_pPlaybackBuffer = NULL;
    m_PlaybackBufferSize = 0;

	m_eType = uXmaFormat;

	m_bCallbackFired = false;

	m_u32NumChannels = waveData.uChannels;

	// [Sam] extra debugging for bug #21976
	if (! m_waveData.hWaveData.IsValid() && m_waveData.psData==NULL)
		AUDIO_CORE_FATAL("XMADecoder::Resume : '%s' m_waveData.hWaveData is invalid and m_waveData.psData is NULL. Looks like the stream didnt get created\n", waveName ? waveName : "");
}

////////////////////////////////////////////////////////////////////////////////
//	~XMADecoder
////////////////////////////////////////////////////////////////////////////////
Audio::Core::XMADecoder::~XMADecoder()
{
#ifdef DEFERRED_XMA
    if (S_OK != XMAPlaybackDestroy(m_state.pXmaPlayback))
        AUDIO_CORE_ERROR("~XMADecoder() cannot destroy XMA playback");
#endif
	
	if ( m_pPlaybackBuffer )
	{
		if (XMA_BufferAllocator::TheBuffer())
			XMA_BufferAllocator::TheBuffer()->Free( m_pPlaybackBuffer, m_PlaybackBufferSize );
	}
    
	m_uTotalXMA--;
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::Init(const DecoderDataInfo& info)
{
    //AcquireContext();

    m_state.bActive = true;
}


////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::AcquireContext(const DecoderDataInfo& info)
{
    ReleaseContext(info);

#ifndef DEFERRED_XMA
	GetLock().Acquire();

	if (m_uActiveXMA == AUDIO_XMA_MAX_NUM_HW_DECODERS)
	{
		GetLock().Release();
		//DumpWavNames();
		BIZ_ASSERT_FAIL("XMADecoder::AcquireContext() out of XMA Contexts. Please tell the audio team!");
		return;
	}

    m_uActiveXMA++;
#ifdef XMA_CONTEXT_DEBUG_LIST
	AddToWavNames(waveName);
#endif
	GetLock().Release();

    XMA_PLAYBACK_INIT xmaInit;
	xmaInit.channelCount = m_waveData.uChannels;
    xmaInit.outputBufferSizeInSamples = m_state.uOctaveMaxPitch * 128;
	xmaInit.sampleRate = m_waveData.uSampleRate;
    xmaInit.subframesToDecode = 8;
    
    const U32 xmaPlaybackBufferSize = XMAPlaybackGetRequiredBufferSize( 1, &xmaInit );
    if ( ( m_PlaybackBufferSize != xmaPlaybackBufferSize ) || !m_pPlaybackBuffer )
    {
        if ( m_pPlaybackBuffer )
        {
            //g_XMA_PlaybackBuffer.Free( m_pPlaybackBuffer, m_PlaybackBufferSize );
            AUDIO_CORE_WARN( "XMADecoder::AcquireContext(): m_pPlaybackBuffer is not NULL!" );
        }
		if (XMA_BufferAllocator::TheBuffer())
			m_pPlaybackBuffer = XMA_BufferAllocator::TheBuffer()->Alloc( xmaPlaybackBufferSize );
        m_PlaybackBufferSize = xmaPlaybackBufferSize;
    }
    
	//HRESULT hr = XMAPlaybackCreate( 1, &xmaInit, 0L, &m_state.pXmaPlayback, (BYTE*)m_pPlaybackBuffer, m_PlaybackBufferSize );
	HRESULT hr = XMAPlaybackCreate( 1, &xmaInit, XMA_PLAYBACK_CREATE_USE_PROVIDED_MEMORY, &m_state.pXmaPlayback, (BYTE*)m_pPlaybackBuffer, m_PlaybackBufferSize );

	if (S_OK != hr)
	{
        AUDIO_CORE_WARN("XMADecoder::AcquireContext() cannot create XMA playback (%d)", hr);
		if (hr == E_OUTOFMEMORY)
			AUDIO_CORE_WARN("XMADecoder::AcquireContext() out of memory");

		if (XMA_BufferAllocator::TheBuffer())
		{
			XMA_BufferAllocator::TheBuffer()->Free(m_pPlaybackBuffer, m_PlaybackBufferSize);
			m_pPlaybackBuffer = NULL;
			m_PlaybackBufferSize = 0;
		}

		// Something went wrong - we didn't keep this context
		GetLock().Acquire();
		m_uActiveXMA--;
#ifdef XMA_CONTEXT_DEBUG_LIST
		RemoveFromWavNames(waveName);
#endif
		GetLock().Release();

		return;
	}
#endif

#ifdef DEFERRED_XMA
	//MEMORY_TRACKER_PUSH("oldtech.audio.XMA Playback");
    if (S_OK != XMAPlaybackAllocateContexts(m_state.pXmaPlayback))
        AUDIO_CORE_ERROR("XMADecoder::AcquireContext() cannot allocate XMA context");

//	MEMORY_TRACKER_POP("oldtech.audio.XMA Playback");
#endif

	#ifdef XMA_DOUBLE_BUFFER
	m_uIndex[0] = XMADecoderBuffer::Instance().Allocate(m_asDecoded[0]);
	m_uIndex[1] = XMADecoderBuffer::Instance().Allocate(m_asDecoded[1]);
#else
	m_uIndex = XMADecoderBuffer::Instance().Allocate(m_asDecoded);
#endif

	// if we've got no data to give it acquire the context but just dont submit any decoded data until we've got some
	if (m_waveData.hWaveData.IsValid())
	{
		S32 sBytesAvail = GetBytesAvailable(info);

		Assert(sBytesAvail >= 0);

		if (g_bAudioPrintXmaStreamerDebug)
			AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ [%d] XMADecoder::AcquireContext() - %s", System::Get()->GetFrameNum(), m_waveData.hWaveData, sBytesAvail >= g_StreamingXmaSubmitChunkSize ? " data is READY" : "data is NOT ready");

		if (sBytesAvail < g_StreamingXmaSubmitChunkSize)
			return;
	}

	Resume(info);
}

////////////////////////////////////////////////////////////////////////////////
//	Resume
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::Resume(const DecoderDataInfo& info)
{
	// [Sam] extra debugging for bug #21976
	if (m_state.pXmaPlayback == NULL)
		AUDIO_CORE_FATAL("XMADecoder::Resume : '%s' m_pXMAPlayback is NULL\n", waveName ? waveName : "");

	if (! m_waveData.hWaveData.IsValid() && m_waveData.psData==NULL)
		AUDIO_CORE_FATAL("XMADecoder::Resume : '%s' m_waveData.hWaveData is invalid and m_waveData.psData is NULL\n", waveName ? waveName : "");

	if (S_OK != XMAPlaybackRequestModifyLock(m_state.pXmaPlayback))
        AUDIO_CORE_ERROR("XMADecoder::Resume() XMAPlaybackRequestModifyLock failed");

	if (S_OK != XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback))
	{
		// If XMAPlaybackWaitUntilModifyLockObtained() did not return S_OK, then it detected a timeout and
		// assumed that there was an XMA hang. It then tried to reset the hardware, which if successful means
		// that we should have a valid lock.
		if (!XMAPlaybackQueryModifyLockObtained(m_state.pXmaPlayback))
		{
			AUDIO_CORE_FATAL("XMAPlaybackWaitUntilModifyLockObtained() failed in XMADecoder::Resume()\n");
		}
	}

	XMA_PLAYBACK_LOOP pl;
	pl.dwLoopStartOffset = m_waveData.uLoopStartOffsetBit;
	pl.dwLoopEndOffset = m_waveData.uLoopEndOffsetBit;
	pl.numLoops = m_waveData.bLoop ? 255 : 0;
	pl.dwLoopSubframeEnd = m_waveData.uSubframeData >> 4;
	pl.dwLoopSubframeSkip = m_waveData.uSubframeData & 0xf;
	if (S_OK != XMAPlaybackSetLoop(m_state.pXmaPlayback, 0, &pl))
        AUDIO_CORE_ERROR("XMADecoder::Resume() XMAPlaybackSetLoop failed");

 	if (S_OK != XMAPlaybackFlushData(m_state.pXmaPlayback, 0))
        AUDIO_CORE_ERROR("XMADecoder::Resume() XMAPlaybackFlushData failed");

	void* pData=0;
	U32 uDataSize = 0;
	if (! m_waveData.hWaveData.IsValid())
	{
		// if non streaming give it the whole sample
		pData = m_waveData.psData;
		uDataSize = m_waveData.uLength;
	}
	else
	{
		// if streaming then give it a little chunk
		Assert(GetBytesAvailable(info) >= (U32)g_StreamingXmaSubmitChunkSize);

		pData = (void*)(GetWaveData(info)->GetData(g_StreamingXmaSubmitChunkSize, (U32*)(info.pWaveServerData)));

		if (pData == NULL)
			AUDIO_CORE_ERROR("COCO POPS. pdata is NULL. avail is %d", GetBytesAvailable(info));

		#ifndef TEMP_XMA_BUFFERING
		pData =  (char*)pData + m_uWaveDataOffset;
		// access the data but dont consume it
		GetWaveData(info)->OverrideBytesConsumed(0);
		#endif

		uDataSize = g_StreamingXmaSubmitChunkSize;
		
		if (g_bAudioPrintXmaStreamerDebug)
		{
			AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::Resume() [%d] - submitting %d bytes to decoder. buf addr 0x%x", System::Get()->GetFrameNum(), m_waveData.hWaveData, uDataSize, pData);
			AUDIO_CORE_DEBUG("%d data : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", System::Get()->GetFrameNum(), *((U8*)pData), *((U8*)pData+1), *((U8*)pData+2), *((U8*)pData+3), *((U8*)pData+4), *((U8*)pData+5), *((U8*)pData+6), *((U8*)pData+7), *((U8*)pData+8), *((U8*)pData+9), *((U8*)pData+10), *((U8*)pData+11), *((U8*)pData+12), *((U8*)pData+13), *((U8*)pData+14), *((U8*)pData+15));			
		}

		/*void* pBufMem = XPhysicalAlloc((size_t)g_StreamingXmaSubmitChunkSize, MAXULONG_PTR, 2048, MEM_LARGE_PAGES | PAGE_READWRITE);
		memcpy(pBufMem, pData, g_StreamingXmaSubmitChunkSize);
		pData = pBufMem;*/

		#ifdef TEMP_XMA_BUFFERING
		void* pBufMem = XMA_BufferAllocator::TheBuffer()->Alloc(g_StreamingXmaSubmitChunkSize + 2048);
		void *pAlignedBufMem = Btl::AlignUp(pBufMem, 2048);
		memcpy(pAlignedBufMem, pData, g_StreamingXmaSubmitChunkSize);
		pData = pAlignedBufMem;
		#endif

		m_SubmittedDataChunksList.PushBack(pBufMem);
		
		#ifndef TEMP_XMA_BUFFERING
		m_uWaveDataOffset += g_StreamingXmaSubmitChunkSize;
		#endif

		#ifndef CFG_SPEED
		if (g_bAudioPrintXmaStreamerDebug)
		{
			Btl::List<void*>::Iterator it = m_SubmittedDataChunksList.Begin();
			while (it != m_SubmittedDataChunksList.End())
			{
				AUDIO_CORE_DEBUG("submitted chunks list 0x%x", (U32)*it);
				++it;
			}
		}
		#endif
	}

	// [Sam] extra debugging for bug #21976
	if (m_state.pXmaPlayback == NULL)
		AUDIO_CORE_FATAL("XMADecoder::Resume : '%s' m_pXMAPlayback is NULL. hStream = %d. \n", waveName ? waveName : "", m_waveData.hWaveData);

	if (pData == NULL)
		AUDIO_CORE_FATAL("XMADecoder::Resume : '%s' pData is NULL. hStream = %d\n", waveName ? waveName : "", m_waveData.hWaveData);

	if (XMAPlaybackSubmitData(m_state.pXmaPlayback, 0, pData, uDataSize) !=  S_OK)
		AUDIO_CORE_ERROR("XMADecoder::Resume() cannot Submit data");

	if (m_state.uStartSample != 0 && m_waveData.puSeekData)
	{
		U32 uStartFrame = m_state.uStartSample / XMA_FRAME_SIZE;

		if (uStartFrame < 0 || uStartFrame >= m_waveData.uFrames)
		{
			AUDIO_CORE_WARN("XMADecoder::Resume() - uStartFrame(%d) invalid - Frames(%d)", uStartFrame, m_waveData.uFrames);
			uStartFrame = 0;
		}

		U32 bitOffset = m_waveData.puSeekData[uStartFrame];
		U32 subFrame = (m_state.uStartSample / XMA_SUBFRAME_SIZE) & 0x3;

		if (uStartFrame == m_waveData.uFrames - 1 && subFrame > (U32)(m_waveData.uSubframeData >> 4))
		{
			subFrame = m_waveData.uSubframeData >> 4;
		}

		if (XMAPlaybackSetDecodePosition(m_state.pXmaPlayback, 0, bitOffset, subFrame) != S_OK)
			AUDIO_CORE_WARN("XMADecoder::Resume() cannot set decode position");
	}

	//	if not xma2, start from top
	// UNLESS it's a stream- as the streamed data pos won't get reset
	// temp fix for Amax bug 24527
	if (m_waveData.puSeekData == 0 && (! m_waveData.hWaveData.IsValid()))
		m_state.uProgress = 0;

    if (S_OK != XMAPlaybackResumePlayback(m_state.pXmaPlayback))
        AUDIO_CORE_ERROR("XMADecoder::Resume() XMAPlaybackResumePlayback failed");

    m_state.bPlaying = true;
	m_state.bResumed = true;	// do we need this flag?

#ifdef XMA_DOUBLE_BUFFER
	// pad 1st buffer with 0
	memset(m_asDecoded[0], 0, 1024 * sizeof(S16) * m_waveData.uChannels);

	m_uWrite = 1;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::Shutdown(const DecoderDataInfo& info)
{
    ReleaseContext(info);

	if (IsPlaying(info))
		SetPlaying(info, false);

    m_state.bActive = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::ReleaseContext(const DecoderDataInfo& info)
{
    if (m_state.pXmaPlayback != NULL && (XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE))
    {
        //XMAPlaybackRequestModifyLock(m_state.pXmaPlayback);
		//XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback);
		//XMAPlaybackFlushData(m_state.pXmaPlayback, 0);
		//XMAPlaybackResumePlayback(m_state.pXmaPlayback);

		//while (GetData());

#ifndef DEFERRED_XMA
#ifndef	DESTROY_ALL_XMA_ON_UPDATE
        if (S_OK != XMAPlaybackDestroy(m_state.pXmaPlayback))
            AUDIO_CORE_ERROR("XMADecoder::Init() cannot destroy XMA playback");

        m_state.pXmaPlayback = NULL;
#endif
#endif

#ifdef DESTROY_ALL_XMA_ON_UPDATE

#ifndef CFG_GOLD
		if (m_state.pXmaPlayback == NULL)
			AUDIO_CORE_FATAL("XMADecoder::ReleaseContext : '%s' m_pXMAPlayback is NULL\n", waveName);
		if (m_pPlaybackBuffer == NULL)
			AUDIO_CORE_FATAL("XMADecoder::ReleaseContext : '%s' m_pBuffer is NULL\n", waveName);
#endif
		AddDoomedContext( m_state.pXmaPlayback, m_pPlaybackBuffer, m_PlaybackBufferSize );
#ifdef XMA_CONTEXT_DEBUG_LIST
		RemoveFromWavNames(waveName);
#endif
        m_pPlaybackBuffer = NULL;
        m_PlaybackBufferSize = 0;

		m_state.pXmaPlayback = NULL;
#endif

#ifdef DEFERRED_XMA
		XMAPlaybackRequestModifyLock(m_state.pXmaPlayback);
		XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback);

		XMAPlaybackFlushData(m_state.pXmaPlayback, 0);

        if (S_OK != XMAPlaybackFreeContexts(m_state.pXmaPlayback))
            AUDIO_CORE_ERROR("XMADecoder::Init() cannot free XMA context");
#endif
	//	if(m_uActiveXMA)  //if there are xma contexts active

		//	[bug 22224] moved to cleanup
		//m_uActiveXMA--;

#ifdef XMA_DOUBLE_BUFFER
		XMADecoderBuffer::Instance().Free(m_uIndex[0]);
		XMADecoderBuffer::Instance().Free(m_uIndex[1]);
		m_uIndex[0] = (U32)-1;
		m_uIndex[1] = (U32)-1;
		m_asDecoded[0] = NULL;
		m_asDecoded[1] = NULL;
		m_uRead = 0;
		m_uWrite = 0;
		m_uWritten = 0;
#else
		XMADecoderBuffer::Instance().Free(m_uIndex);
		m_uIndex = (U32)-1;
		m_asDecoded = NULL;
#endif

		FreeSubmittedBuffers(info);
    }


    m_state.bPlaying = false;
    //m_state.uProgress = 0;
	m_state.sIndex = 0;
	m_state.bStarving = true;
	m_state.bResumed = false;
	m_state.bStreamPrimed = false;
}

////////////////////////////////////////////////////////////////////////////////
//	AddDoomedContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::AddDoomedContext( PXMAPLAYBACK pPlaybackContext, void* pBuffer, U32 bufferSize )
{
	GetLock().Acquire();

	Assert( pPlaybackContext && pBuffer );

	// nickb said this is not needed
	//if (bufferSize)
	//	AUDIO_CORE_INFO("non null");
		
    #ifdef DESTROY_ALL_XMA_ON_UPDATE
	m_XMADoomedPlaybackContexts[m_DoomedIndex].m_pXMAPlayback = pPlaybackContext;
	m_XMADoomedPlaybackContexts[m_DoomedIndex].m_pBuffer      = pBuffer;
	m_XMADoomedPlaybackContexts[m_DoomedIndex].m_BufferSize   = bufferSize;
	m_DoomedIndex++;
    #endif

	GetLock().Release();
}

////////////////////////////////////////////////////////////////////////////////
//	DestroyAllDoomedContexts
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::DumpDoomedContexts()
{ 
#ifndef CFG_GOLD

	printf("===================== Dump Doomed Contexts ====================\n");

	for(U32 i = 0; i<m_uNumDoomedContexts; i++)
	{
        printf("%d - m_pXMAPlayback = 0x%x, m_pBuffer = 0x%x, m_BufferSize = %d\n", i, m_XMADoomedPlaybackContexts[i].m_pXMAPlayback, m_XMADoomedPlaybackContexts[i].m_pBuffer, m_XMADoomedPlaybackContexts[i].m_BufferSize);
	}

	printf("\nm_uActiveXMA = %d, m_uTotalXMA = %d\n");
	printf("===================== End Dump Contexts ====================\n");

#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DestroyAllDoomedContexts
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::DestroyAllDoomedContexts()
{ 
#ifdef DESTROY_ALL_XMA_ON_UPDATE
	GetLock().Acquire();

	for(U32 i = 0;i<m_DoomedIndex;i++)
	{
		if (m_XMADoomedPlaybackContexts[i].m_pXMAPlayback == NULL)
		{
			DumpDoomedContexts();
			AUDIO_CORE_FATAL("XMADecoder::DestroyAllDoomedContexts() : m_pXMAPlayback was NULL");
		}
		else
		{
			if ( S_OK != XMAPlaybackDestroy( m_XMADoomedPlaybackContexts[i].m_pXMAPlayback ) )
			{
				DumpDoomedContexts();
				AUDIO_CORE_ERROR("XMADecoder::DestroyAllDoomedContexts() cannot destroy XMA playback");
			}
		}
		
		if (m_XMADoomedPlaybackContexts[i].m_pBuffer == NULL)
		{
			DumpDoomedContexts();
			AUDIO_CORE_FATAL("XMADecoder::DestroyAllDoomedContexts() : m_pBuffer was NULL");
		}
		else
		{
			XMA_BufferAllocator::TheBuffer()->Free( m_XMADoomedPlaybackContexts[i].m_pBuffer, m_XMADoomedPlaybackContexts[i].m_BufferSize );
		}

		//	[bug 22224] moved. stops errors when out of xmas
		m_uActiveXMA--;

		m_XMADoomedPlaybackContexts[i].m_pXMAPlayback = NULL;
		m_XMADoomedPlaybackContexts[i].m_pBuffer = NULL;
		m_XMADoomedPlaybackContexts[i].m_BufferSize = 0;
	}

	m_DoomedIndex = 0;

	GetLock().Release();
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetCallbackId
////////////////////////////////////////////////////////////////////////////////
//void Audio::Core::XMADecoder::SetCallbackId(U32 u32StateOffset, U32 u32CallbackId)
//{
//	m_state.u32CallbackId = u32CallbackId;
//    m_state.u32StateOffset = u32StateOffset;
//}

////////////////////////////////////////////////////////////////////////////////
//	UpdateFilterCoeffs
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::UpdateFilterCoeffs(F32 fPitch, const ProcessInfo& info)
{
	U32 uFrameSkip = 6;

	//	only update filter if using SRC and LP
	if (m_state.bUseSrc)
	{
		//	pitch changing across subframes or new subframe set, recalc filter coeffs
		if ((m_state.fPitch != m_state.fPrevPitch) ||
			(info.uFrameMod == 0))
		{
			F32 fCutoff;
			if (fPitch > 0.5f)
				fCutoff = 0.5f / fPitch;
			else
				fCutoff = 1.f;
			
			m_state.m_fB = Maths::ArcTan(Maths::fPi * fCutoff);
			m_state.m_fB = -(m_state.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_state.m_fB);
			m_state.m_fA = 0.5f * (1.0f - m_state.m_fB);
			
			#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_DECODER_XMA ) // JASOND
            m_state.m_vA = Maths::_Scalar( m_state.m_fA );
            m_state.m_vB = Maths::_Scalar( m_state.m_fB );
            #endif
		}
	}
}

void Audio::Core::XMADecoder::FreeSubmittedBuffers(const DecoderDataInfo& info)
{
	// if decoder has finished decoding any chunks of data then tell the wave server we have finished with the data
	if (m_waveData.hWaveData.IsValid())
	{
		if (!m_SubmittedDataChunksList.Empty())
		{
			Btl::List<void*>::Iterator it = m_SubmittedDataChunksList.Begin();
			while (it != m_SubmittedDataChunksList.End())
			{
				#ifdef TEMP_XMA_BUFFERING
				// the buffer we submitted to the decoder would have been 2k aligned
				void* pAlignedMem = Btl::AlignUp(*it, 2048);
				#endif

				// check if the xma decoder has finished with this chunk of memory.
				// if m_state.pXmaPlayback is NULL then it means we've destroyed the XmaPlayback, 
				// so in this case just release the memory regardless
				if (!m_state.pXmaPlayback || XMAPlaybackQueryInputDataPending(m_state.pXmaPlayback, 0, pAlignedMem) == false)
				{
					if (g_bAudioPrintXmaStreamerDebug)
						AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FreeSubmittedBuffers() Decoder finished with chunk 0x%x", System::Get()->GetFrameNum(), *it);

					#ifdef TEMP_XMA_BUFFERING
					XMA_BufferAllocator::TheBuffer()->Free(*it, g_StreamingXmaSubmitChunkSize + 2048);
					#else
					Assert(GetBytesAvailable(info) >= (U32)g_StreamingXmaSubmitChunkSize);

					GetWaveData(info)->OverrideBytesConsumed(g_StreamingXmaSubmitChunkSize);
					GetWaveData(info)->SetToUpdate();

					m_uWaveDataOffset -= g_StreamingXmaSubmitChunkSize;
					#endif

					it = m_SubmittedDataChunksList.Erase(it);	// this returns null if its the last in the list
				}
				else
				{
					++it;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::XMADecoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
    using namespace Audio::Maths;
    
#if defined (RELEASE_XMA_ON_SKIP)
#if defined (DEFERRED_XMA)
	if (XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) != TRUE)
		AcquireContext();
#else
	if (m_state.pXmaPlayback == NULL && m_state.bActive) //[memo for nick. won't aquire context if final frame]
	{
		AcquireContext(info.decoderDataInfo);

		//	out of contexts
		if (m_state.pXmaPlayback == NULL)
		{
			//	zero and exit
			Frame::Zero(afData, m_waveData.uChannels);
			return false;
		}
	}
#endif
#endif

	FreeSubmittedBuffers(info.decoderDataInfo);

	// if data wasnt available before when aquiring the context then resume now 
	if (m_waveData.hWaveData.IsValid())
	{
		if (!m_state.bPlaying && m_state.bActive)
		{
			if (GetBytesAvailable(info.decoderDataInfo) >= (U32)g_StreamingXmaSubmitChunkSize)
			{
				if (g_bAudioPrintXmaStreamerDebug)
					AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() [%d] - resuming stream because %d bytes of data is now available", System::Get()->GetFrameNum(), m_waveData.hWaveData, GetBytesAvailable(info.decoderDataInfo) );

				Resume(info.decoderDataInfo);
			}
			else
				GetWaveData(info.decoderDataInfo)->SetToUpdate((U32*)(info.decoderDataInfo.pWaveServerData));	// make sure it updates so the "bytes available" member subsequently gets set
		}
	}

	#ifdef XMA_DOUBLE_BUFFER
	GetData(info.decoderDataInfo);
	#endif

	//  scale pitch by sample rate ratio
    F32 fPitch = m_state.fPrevPitch;

    //  interpolate between audio subframes
	fPitch += info.fFraction * (m_state.fPitch - m_state.fPrevPitch);

	//Assert(XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE);

	Frame::Zero(afData);

	if (m_waveData.uChannels > 1)
		Frame::Zero(afData + uFrameLength);

	U32 uFrameSkip = 6;

	{
		//	then fill buffer
		bool bActive = true;

		//if (m_state.bStarving && m_state.bResumed && m_state.pXmaPlayback && (XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE))
		if (m_state.bStarving && m_state.bResumed)
		{
#ifdef XMA_DOUBLE_BUFFER
			if (PeekNextBuffer() == false)
#else
			if (GetData() == false)
#endif
			{
				//  was playing, now isn't, so fire callback
				if (m_state.bPlaying == false)
				{
					//  TODO: release xma context
					//if (m_state.u32CallbackId)
					//	Audio::System::TriggerDecoderCallback(m_state.u32CallbackId, m_state.u32StateOffset);

					if (m_state.bTriggerCallback)
						m_bCallbackFired = true;

#ifdef DEFERRED_XMA
		/*			XMADecoderBuffer::Instance().m_bUsed[m_uIndex] = false;
					m_uIndex = (U32)-1;
					m_asDecoded = NULL;*/
					ReleaseContext();
#endif
					//  release hardware
					if (!m_state.bLoop)
						Shutdown(info.decoderDataInfo);

				}
				
				if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
					AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() - GetNextBuffer() failed. m_uWritten = %d", System::Get()->GetFrameNum(), m_uWritten);

				if (! m_waveData.hWaveData.IsValid())
					return false;
			}
			else
			{
				if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
					AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() - was starving but now has context. Starving set to false. m_uWritten = %d", System::Get()->GetFrameNum(), m_uWritten);

				m_state.bStarving = false;
			}
		}

		//S32 sIndex = m_state.uStartSample;

		S32 sFraction = (S32)(m_state.fFraction * (F32)0x01000000); //  2^24
		
		XMADecoder::UpdateFilterCoeffs(fPitch, info);

	
		if (m_state.bUseSrc)
		{
			//  mono
			if (m_waveData.uChannels == 1)
			{
#ifdef XMA_DOUBLE_BUFFER
				S16 const* AUDIO_RESTRICT asDataIn = m_asDecoded[m_uRead];
#else
				S16 const* AUDIO_RESTRICT asDataIn = m_asDecoded;
#endif
				F32* AUDIO_RESTRICT afDataOut = afData;

				//	temp buffer
				//F32 afShiftBuffer[(uFrameLength + 1) * 4];	//	1 extra sample, mono, 2 oct up
				//Vector4 guard0[4] = { MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ) };
				Vector4 vec4_afShiftBuffer[ ( uFrameLength + 1 ) + 4 /* + 32 */ ]; // 16 byte aligned & padded to 16 samples [ 64 bytes ]
				//Vector4 guard1[4] = { MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ), MakeVector4( 1, 2, 3, 4 ) };
				F32* afShiftBuffer = (F32*)&vec4_afShiftBuffer[0];
                
				U32 uSamplesRequired = (U32)(fFrameLength * fPitch);
                
				afShiftBuffer[0] = m_state.fPrevSample;
				F32* afShifted = &afShiftBuffer[1];

				if ((m_state.bStarving == false)/* && m_asDecoded*/)
				{
					// If we have enough data already we can convert what we have quickly
					if (m_state.sIndex + S32(uSamplesRequired) < m_state.sNumSamples)
					{
						// Fill buffer - uses fixed point maths 8:24, max pitch +8 octaves interpolate:
						FillF32BufferFromS16(uSamplesRequired, asDataIn + m_state.sIndex, afShifted);
						m_state.sIndex += uSamplesRequired;
						m_state.uStartSample += uSamplesRequired;
					}
					else
					{
						int firstBatchSize = m_state.sNumSamples - m_state.sIndex;

						// Copy up to the point we need more data
						FillF32BufferFromS16(firstBatchSize, asDataIn + m_state.sIndex, afShifted);
						m_state.sIndex += firstBatchSize;
						m_state.uStartSample += firstBatchSize;

#ifdef XMA_DOUBLE_BUFFER
						U32 uAvailable = GetNextBuffer();
						asDataIn = m_asDecoded[m_uRead];
#else
						U32 uAvailable = GetData();
#endif
						m_state.sIndex = uSamplesRequired - firstBatchSize;
						m_state.uStartSample += uSamplesRequired - firstBatchSize;

						if (uAvailable)
						{
							//Assert(uAvailable >= uSamplesRequired - firstBatchSize);

							//m_state.sNumSamples = uAvailable;
							// Copy remaining over
							FillF32BufferFromS16(uSamplesRequired - firstBatchSize, asDataIn, afShifted + firstBatchSize);
						}
						else
						{
							m_state.bStarving = true;
							
							if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
								AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() [%d] bStarving set to TRUE as no bytes available after first batch", System::Get()->GetFrameNum(), m_waveData.hWaveData);

							// Clear the rest of the buffer if we're starving
							memset(afShifted + firstBatchSize, 0, sizeof(F32) * (uSamplesRequired - firstBatchSize));
						}
					}

					//[cian] remove using
					static bool usingFilterMono = true;
					if(usingFilterMono)
					{
                        #if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_DECODER_XMA ) // JASOND
                        
						////	1st order low pass filter
						F32 fIn = 0.f;
						F32 fOut = 0.f;
						F32 fPrevIn = m_state.fPrevIn;
						F32 fPrevOut = m_state.fPrevOut;
						U32 uCount;
						for (uCount = 0; uCount < uSamplesRequired; uCount++)
						{
							fIn  = afShiftBuffer[uCount];
							fOut = m_state.m_fA * (fIn + fPrevIn) + m_state.m_fB * fPrevOut;

							afShiftBuffer[uCount] = fOut; 

							fPrevIn = fIn;
							fPrevOut = fOut;
						}
						m_state.fPrevIn = fIn;
						m_state.fPrevOut = fOut;

						//	do extra sample
						fIn  = afShiftBuffer[uCount];
						fOut = m_state.m_fA * (fIn + fPrevIn) + m_state.m_fB * fPrevOut;

						afShiftBuffer[uCount] = fOut;
						
						#else
						
						if (uSamplesRequired > ( uFrameLength + 1 ) * 4)
						    DebugBreak();
						
						GenericDspLowPass1stOrder_x16( vec4_afShiftBuffer, m_state.m_vA, m_state.m_vB, m_state.m_vPrevIn, m_state.m_vPrevOut, uSamplesRequired );
						
						const F32 fPrevIn  = _float( W( m_state.m_vPrevIn ) );
						const F32 fPrevOut = _float( W( m_state.m_vPrevOut ) );
						
						//	do extra sample
						
						const F32 fIn  = afShiftBuffer[ uSamplesRequired ];
						const F32 fOut = m_state.m_fA * ( fIn + fPrevIn ) + m_state.m_fB * fPrevOut;
						afShiftBuffer[ uSamplesRequired ] = fOut;
						
						#endif
					}

					Resample(fPitch, uSamplesRequired, afShiftBuffer, afDataOut);

					m_state.fPrevSample = afShiftBuffer[uSamplesRequired];//get final sampe of frame

					//  interpolate end
				}
				else
				{
					//  past end of sample so zero buffer
					if (m_state.sNumSamples == uFrameLength)
						Frame::Zero(afDataOut);

					bActive = false;
				}
			}

			//  stereo
			if (m_waveData.uChannels == 2)
			{
#ifdef XMA_DOUBLE_BUFFER
				S16 const* AUDIO_RESTRICT asIn = m_asDecoded[m_uRead];
#else
				S16 const* AUDIO_RESTRICT asIn = m_asDecoded;
#endif

#ifdef XMA_DOUBLE_BUFFER
				S16 const* AUDIO_RESTRICT asInL = &m_asDecoded[m_uRead][0];
				S16 const* AUDIO_RESTRICT asInR = &m_asDecoded[m_uRead][1];
#else
				S16 const* AUDIO_RESTRICT asInL = &m_asDecoded[0];
				S16 const* AUDIO_RESTRICT asInR = &m_asDecoded[1];
#endif
			
				F32* AUDIO_RESTRICT afOutL = &afData[0];
				F32* AUDIO_RESTRICT afOutR = &afData[uFrameLength];

// TODO: [Rog] See if we can have some physical scratch memory for stuff like this
				F32 afShiftBufferL[(uFrameLength + 1) * 4];//2octaves, stereo
				F32 afShiftBufferR[(uFrameLength + 1) * 4];
				
				U32 uSamplesRequired = (U32)(fFrameLength * fPitch);

				afShiftBufferL[0] = m_state.fPrevSampleL;
				F32* afShiftedL = &afShiftBufferL[1];

				afShiftBufferR[0] = m_state.fPrevSampleR;
				F32* afShiftedR = &afShiftBufferR[1];
			
				if ((m_state.bStarving == false)/* && m_asDecoded*/)
				{
					if (m_state.sIndex + S32(uSamplesRequired) < m_state.sNumSamples)
					{
						FillF32BufferFromS16(uSamplesRequired, asIn + m_state.sIndex*2, afShiftedL, afShiftedR);
						m_state.sIndex += uSamplesRequired;
						m_state.uStartSample += uSamplesRequired;
					}
					else
					{
						int firstBatchSize = m_state.sNumSamples - m_state.sIndex;

						// Copy up to the point we need more data
						FillF32BufferFromS16(firstBatchSize, asIn + m_state.sIndex*2, afShiftedL, afShiftedR);
						m_state.sIndex += firstBatchSize;
						m_state.uStartSample += firstBatchSize;

#ifdef XMA_DOUBLE_BUFFER
						U32 uAvailable = GetNextBuffer();
						asIn = m_asDecoded[m_uRead];
						asInL = &m_asDecoded[m_uRead][0];
						asInR = &m_asDecoded[m_uRead][1];
#else
						U32 uAvailable = GetData();
#endif
						if (uAvailable)
						{
							//Assert(uAvailable >= uSamplesRequired - firstBatchSize);

							m_state.sIndex = 0;
							//m_state.sNumSamples = uAvailable;

							// Copy remaining over
							FillF32BufferFromS16(uSamplesRequired - firstBatchSize, asIn, afShiftedL + firstBatchSize, afShiftedR + firstBatchSize);
							m_state.sIndex += uSamplesRequired - firstBatchSize;
							m_state.uStartSample += uSamplesRequired - firstBatchSize;
						}
						else
						{
							m_state.bStarving = true;

							// Clear the rest of the buffer if we're starving
							memset(afShiftedL + firstBatchSize, 0, sizeof(F32) * (uSamplesRequired - firstBatchSize));
							memset(afShiftedR + firstBatchSize, 0, sizeof(F32) * (uSamplesRequired - firstBatchSize));
						}
					}
					
					if(fPitch >= 1)
					{
//CIAN TODO - fix LP for stereo
						////	1st order low pass filter
						//F32 fInL = 0.f, fOutL = 0.f, fInR = 0.f, fOutR = 0.f;
						//F32 fPrevInL = m_state.fPrevInL;
						//F32 fPrevInR = m_state.fPrevInR;
						//F32 fPrevOutL = m_state.fPrevOutL;
						//F32 fPrevOutR = m_state.fPrevOutR;
						//U32 uCount;
						//for (uCount = 0; uCount < uSamplesRequired; uCount++)
						//{
						//	fInL  = afShiftBufferL[uCount];
						//	fOutL = m_state.m_fA * (fInL + fPrevInL) + m_state.m_fB * fPrevOutL;
						//	fInR  = afShiftBufferR[uCount];
						//	fOutR = m_state.m_fA * (fInR + fPrevInR) + m_state.m_fB * fPrevOutR;

						//	afShiftBufferL[uCount] = fOutL; 
						//	afShiftBufferR[uCount] = fOutR; 

						//	fPrevInL = fInL;
						//	fPrevInR = fInR;
						//	fPrevOutL = fOutL;
						//	fPrevOutR = fOutR;
						//}
						//m_state.fPrevInL = fInL;
						//m_state.fPrevInR = fInR;
						//m_state.fPrevOutL = fOutL;
						//m_state.fPrevOutR = fOutR;

						//////	do extra sample
						//fInL  = afShiftBufferL[uCount];
						//fOutL = m_state.m_fA * (fInL + fPrevInL) + m_state.m_fB * fPrevOutL;
						//fInR  = afShiftBufferR[uCount];
						//fOutR = m_state.m_fA * (fInR + fPrevInR) + m_state.m_fB * fPrevOutR;

						//afShiftBufferL[uCount] = fOutL; 
						//afShiftBufferR[uCount] = fOutR; 

						//Frame::TimingEnd();
					}
				
					Resample(fPitch, uSamplesRequired, afShiftBufferL, afShiftBufferR, afOutL, afOutR);
		
					m_state.fPrevSampleL = afShiftBufferL[uSamplesRequired];//get final sampe of frame
					m_state.fPrevSampleR = afShiftBufferR[uSamplesRequired];//get final sampe of frame

					//  interpolate end
				}
				else
				{
					//  past end of sample so zero buffer
					if (m_state.sNumSamples == uFrameLength)
					{
						Frame::Zero(afOutL);
						Frame::Zero(afOutR);
					}

					bActive = false;
				}
			}
		}
		else	// not using SRC
		{
			//  mono
			if (m_waveData.uChannels == 1)
			{
#ifdef XMA_DOUBLE_BUFFER
				S16 const* AUDIO_RESTRICT asDataIn = m_asDecoded[m_uRead];
#else
				S16 const* AUDIO_RESTRICT asDataIn = m_asDecoded;
#endif
				F32* AUDIO_RESTRICT afDataOut = afData;
				F32 afShiftBuffer[(uFrameLength)];

				if ((m_state.bStarving == false)/* && m_asDecoded*/) // ???m_asDecoded???
				{
					//  fill buffer - uses fixed point maths 8:24, max pitch +8 octaves interpolate:
					if (m_state.sIndex + S32(uFrameLength) < m_state.sNumSamples)
					{
						FillF32BufferFromS16(uFrameLength, asDataIn + m_state.sIndex, afShiftBuffer);
						m_state.sIndex += uFrameLength;
						m_state.uStartSample += uFrameLength;
					}
					else
					{
						int firstBatchSize = m_state.sNumSamples - m_state.sIndex;

						// Copy up to the point we need more data
						FillF32BufferFromS16(firstBatchSize, asDataIn + m_state.sIndex, afShiftBuffer);
						m_state.sIndex += firstBatchSize;
						m_state.uStartSample += firstBatchSize;

#ifdef XMA_DOUBLE_BUFFER
						U32 uAvailable = GetNextBuffer();
						asDataIn = m_asDecoded[m_uRead];
#else
						U32 uAvailable = GetData();
#endif
						if (uAvailable)
						{
							Assert(uAvailable >= uFrameLength - firstBatchSize);

							m_state.sIndex = 0;
						//	m_state.sNumSamples = uAvailable;

							// Copy remaining over
							FillF32BufferFromS16(uFrameLength - firstBatchSize, asDataIn, afShiftBuffer + firstBatchSize);
							m_state.sIndex += uFrameLength - firstBatchSize;
							m_state.uStartSample += uFrameLength - firstBatchSize;
						}
						else
						{
							m_state.bStarving = true;

							// Clear the rest of the buffer if we're starving
							memset(afShiftBuffer + firstBatchSize, 0, sizeof(F32) * (uFrameLength - firstBatchSize));
						}
					}

					//	copy - TODO: optimise
					for (U32 uCount = 0; uCount < uFrameLength; uCount++)
						afDataOut[uCount] = afShiftBuffer[uCount];
				}
				else
				{
					//  past end of sample so zero buffer
					if (m_state.sNumSamples == uFrameLength)
						Frame::Zero(afDataOut);

					bActive = false;
				}
			}

			//  stereo
			if (m_waveData.uChannels == 2)
			{
#ifdef XMA_DOUBLE_BUFFER
				S16 const* AUDIO_RESTRICT asIn = m_asDecoded[m_uRead];
				S16 const* AUDIO_RESTRICT asInL = &m_asDecoded[m_uRead][0];
				S16 const* AUDIO_RESTRICT asInR = &m_asDecoded[m_uRead][1];
#else
				S16 const* AUDIO_RESTRICT asIn = m_asDecoded;
				S16 const* AUDIO_RESTRICT asInL = &m_asDecoded[0];
				S16 const* AUDIO_RESTRICT asInR = &m_asDecoded[1];
#endif
				F32* AUDIO_RESTRICT afOutL = &afData[0];
				F32* AUDIO_RESTRICT afOutR = &afData[uFrameLength];

				F32 afShiftBufferL[uFrameLength];
				F32 afShiftBufferR[uFrameLength];
				
				static U32 offset = 0;

				if ((m_state.bStarving == false)/* && m_asDecoded*/) // ???m_asDecoded???
				{
					//  fill buffer - uses fixed point maths 8:24, max pitch +8 octaves interpolate:
					if (m_state.sIndex + S32(uFrameLength) < m_state.sNumSamples)
					{
						// TODO: mini bug-ette here. if (m_state.sIndex + uFrameLength == m_state.sNumSamples) it should be dropping in here but its not.
						// this means its not using the full buffer.
						// if this condition is true then we will need to flip buffers after we've filled the buffer.

						if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
							AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() Filling buffer with %d samples of decoded data. m_state.sIndex = %d, m_state.sNumSamples = %d ", System::Get()->GetFrameNum(), uFrameLength, m_state.sIndex, m_state.sNumSamples);				

						FillF32BufferFromS16(uFrameLength, asIn + m_state.sIndex*2, afShiftBufferL, afShiftBufferR);
						m_state.sIndex += uFrameLength;
						m_state.uStartSample += uFrameLength;
					}
					else
					{
						int firstBatchSize = m_state.sNumSamples - m_state.sIndex;

						// Copy up to the point we need more data
						FillF32BufferFromS16(firstBatchSize, asIn + m_state.sIndex*2, afShiftBufferL, afShiftBufferR);
						if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
							AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() First batch: filling buffer with %d samples of decoded data.  m_state.sIndex = %d, m_state.sNumSamples = %d ", System::Get()->GetFrameNum(), firstBatchSize, uFrameLength, m_state.sIndex, m_state.sNumSamples);				

						m_state.sIndex += firstBatchSize;
						m_state.uStartSample += firstBatchSize;

#ifdef XMA_DOUBLE_BUFFER
						U32 uAvailable = GetNextBuffer();
						asIn = m_asDecoded[m_uRead];
						asInL = &m_asDecoded[m_uRead][0];
						asInR = &m_asDecoded[m_uRead][1];
#else
						U32 uAvailable = GetData();
#endif
						if (uAvailable)
						{
							//Assert(uAvailable >= uFrameLength - firstBatchSize);

							m_state.sIndex = 0;
						//	m_state.sNumSamples = uAvailable;

							// Copy remaining over
							FillF32BufferFromS16(uFrameLength - firstBatchSize, asIn, afShiftBufferL + firstBatchSize, afShiftBufferR + firstBatchSize);
							if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
								AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() NOT starving. Second batch: filling buffer with %d samples of decoded data. uAvailable=%d", System::Get()->GetFrameNum(), uFrameLength - firstBatchSize, uAvailable);				

							m_state.sIndex += uFrameLength - firstBatchSize;
							m_state.uStartSample += uFrameLength - firstBatchSize;
						}
						else
						{
							if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
								AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() Not enough data for second batch: %d bytes available. Zeroing buffer. state set to STARVING", System::Get()->GetFrameNum(), uAvailable);

							m_state.bStarving = true;

							// Clear the rest of the buffer if we're starving
							memset(afShiftBufferL + firstBatchSize, 0, sizeof(F32) * (uFrameLength - firstBatchSize));
							memset(afShiftBufferR + firstBatchSize, 0, sizeof(F32) * (uFrameLength - firstBatchSize));
						}
					}
					
					//copy
					for (U32 uCount=0; uCount < uFrameLength; uCount++)
					{
						afOutL[uCount] = afShiftBufferL[uCount];
						afOutR[uCount] = afShiftBufferR[uCount];
					}
				}
				else
				{
					if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
						AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::FillBuffer() [%d] - STARVING. Zeroing frame", System::Get()->GetFrameNum(), m_waveData.hWaveData);

					//  past end of sample so zero buffer
					if (m_state.sNumSamples == uFrameLength)
					{
						Frame::Zero(afOutL);
						Frame::Zero(afOutR);
					//	Frame::Zero(afDataOut);
					}

					bActive = false;
				}
			}
		}
	
		m_state.fFraction = sFraction / (F32)0x01000000;
		m_state.uStartSample %= m_waveData.uFrames * XMA_FRAME_SIZE;

		if (uFrameNum % uFrameSkip == uFrameSkip - 1)
			m_state.fPrevPitch = m_state.fPitch;

		return bActive;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Position
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Core::XMADecoder::Position(const DecoderDataInfo& info)
{
	if (m_waveData.uChannels)
	{
		//	TODO: this is incorrect, should use XMAPlaybackQueryCurrentPosition()?
		//S32 sNumSamples = (S32)m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));
		S32 sNumSamples = (S32)m_waveData.uFrames * 512;

		return (F32)m_state.uProgress / (F32)sNumSamples;
	}
	else
		return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	SkipBuffer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::SkipBuffer(const ProcessInfo& info)
{
	U32 uFrameSkip = 6;

#if defined (RELEASE_XMA_ON_SKIP)
	// another streamer hack here- if a stream, don't release context, as it causes uProgress to get out of sync for some reason, meaning the stream never finishes
	if (m_state.pXmaPlayback && XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE && (! m_waveData.hWaveData.IsValid()))
		ReleaseContext(info.decoderDataInfo);
#endif

	//  scale pitch by sample rate ratio
    F32 fPitch = m_state.fPrevPitch;

    //  interpolate between audio subframes
	fPitch += info.fFraction * (m_state.fPitch - m_state.fPrevPitch);

    //  skip a frame of samples
    F32 fSkip = fFrameLength * fPitch;

    U32 uSkip = (U32)fSkip;
	// yet another hack. if streaming, don't change uProgress, as it then won't correspond to how much data we've streamed
	// temp fix for Amax bug 24527
	if (! m_waveData.hWaveData.IsValid())
	{
		m_state.uProgress += uSkip;
		m_state.fFraction += fSkip - (F32)uSkip;

		if (m_state.fFraction >= 1.f)
		{
			m_state.fFraction -= 1.f;
			m_state.uProgress++;
		}
	}
	else
		return;

	S32 sNumSamples = m_waveData.uFrames * XMA_FRAME_SIZE;

    //  deal with loops
    if (m_state.uProgress >= (U32)sNumSamples)
    {
		//if (m_state.u32CallbackId)
		//	Audio::System::TriggerDecoderCallback(m_state.u32CallbackId, m_state.u32StateOffset);
		if (m_state.bTriggerCallback)
			m_bCallbackFired = true;

        if (m_state.bLoop)
            m_state.uProgress -= sNumSamples;
		else
			Shutdown(info.decoderDataInfo);
    }

    m_state.uStartSample = m_state.uProgress;

	if (uFrameNum % uFrameSkip == uFrameSkip - 1)
		m_state.fPrevPitch = m_state.fPitch;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::Reset(const DecoderDataInfo& info, U32 uOffset)
{
	ReleaseContext(info);

	if (m_waveData.hWaveData.IsValid())
	{
		GetWaveData(info)->SetSourceDataPosition(0, (U32*)(info.pWaveServerData));
		m_state.uStartSample = 0;
		m_state.uProgress = 0;
	}
	else
	{
		m_state.uStartSample = uOffset;
		m_state.uProgress = uOffset;
	}

    m_state.fFraction = 0.f;
    m_state.fPitch = 1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	SetPitch
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::XMADecoder::SetPitch(F32 fPitch)
{
	F32 fRescaled = fPitch * (F32)m_waveData.uSampleRate * fRcpSampleRate; // JASOND
    
    m_state.fPitch = Maths::Clamp( fRescaled, 0.00390625f, 4.f );
}

////////////////////////////////////////////////////////////////////////////////
//	GetBytesAvailable
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::XMADecoder::GetBytesAvailable(const DecoderDataInfo& info)
{ 
	if (m_waveData.hWaveData.IsValid()) 
	{
		#ifdef TEMP_XMA_BUFFERING
		return GetWaveData(info)->GetBytesAvailable(); 
		#else
		U32 uWaveDataAvail = GetWaveData(info)->GetBytesAvailable();
		Assert(m_uWaveDataOffset <= uWaveDataAvail);
		return uWaveDataAvail - m_uWaveDataOffset; 
		#endif
	}

	return m_state.sNumSamples; 
}

Audio::Core::WaveStreamData* Audio::Core::XMADecoder::GetWaveData(const DecoderDataInfo& info) 
{ 
	WaveStreamData* pData = Core::GetWaveData(m_waveData.hWaveData, info.pWaveServerData);
	return pData; 
}

#ifdef XMA_DOUBLE_BUFFER
////////////////////////////////////////////////////////////////////////////////
//	GetData
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::XMADecoder::GetData(const DecoderDataInfo& info)
{
	bool bResult = false;
	U32 uAvailable = 0;

	if (m_waveData.hWaveData.IsValid())
	{
		// this extra flag was put in to stop the situation where at the end of a one shot stream we set playing on the wavedata to false and the waveserver
		// picks up on this in WaveServer::UpdateStream and sets the graph to inactive which makes GetWaveData(info)->IsPrimed() return false, so it tries to 
		// prime the stream again (when it obviously shouldn't).
		if (!m_state.bStreamPrimed && ! GetWaveData(info)->IsPrimed())
		{
			GetWaveData(info)->Prime((U32*)(info.pWaveServerData));
			m_state.bStreamPrimed = true;

			return 0;
		}
	}

	// get some decoded data and put it in one of our double buffers
	if (m_state.pXmaPlayback && (XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE))
	{
		if (m_uRead != m_uWrite)
		{
			//  aquire lock
			HRESULT hr = XMAPlaybackRequestModifyLock(m_state.pXmaPlayback);
			if (hr != S_OK)
				AUDIO_CORE_FATAL("XMAPlaybackRequestModifyLock() failed in XMADecoder::GetData() with error code %d\n", hr);

			hr = XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback);
			if (hr != S_OK)
			{
				// If XMAPlaybackWaitUntilModifyLockObtained() did not return S_OK, then it detected a timeout and
				// assumed that there was an XMA hang. It then tried to reset the hardware, which if successful means
				// that we should have a valid lock.
				if (!XMAPlaybackQueryModifyLockObtained(m_state.pXmaPlayback))
				{
					AUDIO_CORE_FATAL("XMAPlaybackWaitUntilModifyLockObtained() failed in XMADecoder::GetData() with error code %d\n", hr);
				}
			}	
 
			// how much decoded data is there available?
			uAvailable = XMAPlaybackQueryAvailableData(m_state.pXmaPlayback, 0);
			U32 uSamplesRequired = m_state.uOctaveMaxPitch * 128 - m_uWritten;

			if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
				AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() - %d bytes of decoded data available from xma decoder, asking for %d", System::Get()->GetFrameNum(), uAvailable, uSamplesRequired);
							
			S16* pBuffer = m_asDecoded[m_uWrite] + m_uWritten * m_waveData.uChannels;

			// if streamer is running low on encoded data then dont ask for any more decoded data otherwise decoder will get itself in a state
			if (m_waveData.hWaveData.IsValid())
			{
				if (!GetWaveData(info)->IsFinished() && GetBytesAvailable(info) < (U32)g_StreamingXmaSubmitChunkSize)
				{
					uAvailable = 0;
					GetWaveData(info)->SetToUpdate((U32*)(info.pWaveServerData));	// make sure it updates so the "bytes available" member subsequently gets set
				}
			}

			// have we got sufficient data?
			if (uAvailable >= uSamplesRequired)
			{
				//  data ready
				U32 uRemaining = uSamplesRequired;

				//  loop until all data copied
				while (uRemaining > 0)
				{
					S16* asData = NULL;

					// get some more decoded data
					U32 uRead = XMAPlaybackConsumeDecodedData(m_state.pXmaPlayback, 0, uRemaining, (void**)&asData);
					
					if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
                		AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() [uAvailable >= uSamplesRequired] - got %d samples of decoded data", System::Get()->GetFrameNum(), uRead);

					// copy into decoder's 'back buffer'
					memcpy(pBuffer, asData, uRead * sizeof(S16) * m_waveData.uChannels);

					uRemaining -= uRead; // * uChannels
					pBuffer += uRead * m_waveData.uChannels;

					m_state.uProgress += uRead;
					if (m_state.uProgress > m_waveData.uFrames * 512)
					{
						if (m_state.bLoop)
							m_state.uProgress -= m_waveData.uFrames * 512;
						else
							m_state.uProgress = m_waveData.uFrames * 512;
					}

					m_uWritten += uRead;
				}

				bResult = true;
			}
			// we've not got quite enough as requested. end of sample?
			else if (uAvailable > 0)
			{
				//  data ready
				U32 uRemaining = uAvailable;

				//  loop until all data copied
				while (uRemaining > 0)
				{
					S16* asData = NULL;

					U32 uRead = XMAPlaybackConsumeDecodedData(m_state.pXmaPlayback, 0, uRemaining, (void**)&asData);

					if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
						AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() [uAvailable < uSamplesRequired AND uAvailable > 0] - got %d samples of decoded data", System::Get()->GetFrameNum(), uRead);

					memcpy(pBuffer, asData, uRead * sizeof(S16) * m_waveData.uChannels);

					uRemaining -= uRead; // * uChannels
					pBuffer += uRead * m_waveData.uChannels;

					m_state.uProgress += uRead;
					if (m_state.uProgress > m_waveData.uFrames * 512)
					{
						if (m_state.bLoop)
							m_state.uProgress -= m_waveData.uFrames * 512;
						else
							m_state.uProgress = m_waveData.uFrames * 512;
					}

					m_uWritten += uRead;
				}

				S32 uNewAvailable = 0;
			            
				bResult = true;
			}
			else
			// no data available
			{
				if ((! m_waveData.hWaveData.IsValid()) && uSamplesRequired == 0)
				{
					SetPlaying(info, false);
				}
			}

			if (m_waveData.hWaveData.IsValid())
			{
				// does the decoder need more encoded data?
				if (XMAPlaybackQueryReadyForMoreData( m_state.pXmaPlayback, 0 ))
				{
					if (GetBytesAvailable(info) >= (U32)g_StreamingXmaSubmitChunkSize)
					{
						// TODO: deal with if GetData doesnt get number of bytes of encoded data it needs
						void* pData = (void*)(GetWaveData(info)->GetData(g_StreamingXmaSubmitChunkSize, (U32*)(info.pWaveServerData)));

						if (g_bRandomlyCorruptData)
						{
							((U8*)pData)[(U32)((float)g_StreamingXmaSubmitChunkSize * System::Get()->GetRandom())] = 255 * System::Get()->GetRandom();
							g_bRandomlyCorruptData = false;
						}

						#ifdef TEMP_XMA_BUFFERING
						void* pBufMem = XMA_BufferAllocator::TheBuffer()->Alloc(g_StreamingXmaSubmitChunkSize + 2048);
						void *pAlignedBufMem = Btl::AlignUp(pBufMem, 2048);
						memcpy(pAlignedBufMem, pData, g_StreamingXmaSubmitChunkSize);
						pData = pAlignedBufMem;
						#else
						// access the data but dont consume it. this is to stop us overwriting the data while the xma decoder is still using it
						GetWaveData(info)->OverrideBytesConsumed(0);
						if (g_bAudioPrintXmaStreamerDebug)
							AUDIO_CORE_DEBUG("GetData returned 0x%x offsetting by %d bytes. pData is now  0x%x", pData, m_uWaveDataOffset, (char*)pData + m_uWaveDataOffset);
						pData =  (char*)pData + m_uWaveDataOffset;
						#endif
						
						/*void* pBufMem = XPhysicalAlloc((size_t)g_StreamingXmaSubmitChunkSize, MAXULONG_PTR, 2048, MEM_LARGE_PAGES | PAGE_READWRITE);
						memcpy(pBufMem, pData, g_StreamingXmaSubmitChunkSize);
						pData = pBufMem;*/

						if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
						{
							AUDIO_CORE_DEBUG("submitting data 0x%x", pData);
							AUDIO_CORE_DEBUG("%d data : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", System::Get()->GetFrameNum(), *((U8*)pData), *((U8*)pData+1), *((U8*)pData+2), *((U8*)pData+3), *((U8*)pData+4), *((U8*)pData+5), *((U8*)pData+6), *((U8*)pData+7), *((U8*)pData+8), *((U8*)pData+9), *((U8*)pData+10), *((U8*)pData+11), *((U8*)pData+12), *((U8*)pData+13), *((U8*)pData+14), *((U8*)pData+15));
						}

						if (XMAPlaybackSubmitData( m_state.pXmaPlayback, 0, pData, g_StreamingXmaSubmitChunkSize ) == S_OK)
						{
							m_SubmittedDataChunksList.PushBack(pBufMem);

							#ifndef TEMP_XMA_BUFFERING
							m_uWaveDataOffset += g_StreamingXmaSubmitChunkSize;
							#endif
						}
						else
							AUDIO_CORE_ERROR("XMADecoder::GetData() cannot Submit data");

						if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
						{
							AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() - Submitted %d bytes of encoded data to decoder. addr of buffer 0x%x", System::Get()->GetFrameNum(), (U32)g_StreamingXmaSubmitChunkSize, pData);

							if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
								AUDIO_CORE_DEBUG("m_uWaveDataOffset is %d. %d num bytes avail", m_uWaveDataOffset, GetBytesAvailable(info));

							#ifndef CFG_SPEED
							Btl::List<void*>::Iterator it = m_SubmittedDataChunksList.Begin();
							while (it != m_SubmittedDataChunksList.End())
							{
								AUDIO_CORE_DEBUG("list 0x%x", (U32)*it);
								++it;
							}
							#endif
						}
					}
					else
						if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
							AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() [%d] - XMAPlaybackQueryReadyForMoreData() returned true but we didnt have enough data", System::Get()->GetFrameNum());
				}
			}

			// "Queries whether the hardware has decoded all available data". for memory resident sounds only
			if (!m_waveData.hWaveData.IsValid() && XMAPlaybackIsIdle(m_state.pXmaPlayback, 0))
				m_state.bPlaying = false;
			else if (m_waveData.hWaveData.IsValid() && m_state.uProgress >= Length() && XMAPlaybackIsIdle(m_state.pXmaPlayback, 0))
			{
				SetPlaying(info, false);
				m_state.bStarving = true;
			}

			if (S_OK != XMAPlaybackResumePlayback(m_state.pXmaPlayback))
				AUDIO_CORE_ERROR("XMAPlaybackResumePlayback() failed\n");
		}

		if (m_uWritten == m_state.uOctaveMaxPitch * 128)
		{
			if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
				AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() - filled buffer %d. write buffer switched to buffer %d", System::Get()->GetFrameNum(), m_uWrite, 1 - m_uWrite);
			m_uWrite = 1 - m_uWrite;
			m_uWritten = 0;
		}
	}

	if (m_waveData.hWaveData.IsValid() && m_state.pXmaPlayback)
	{
		if( DWORD parseErr = XMAPlaybackGetParseError( m_state.pXmaPlayback, 0 ) )
		{
			m_bCallbackFired = true;
			Shutdown(info);
			return true;
		}
	}

	// STREAMER DEBUG
	/*
	#ifndef CFG_SPEED
	if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid() && m_state.pXmaPlayback)
	{
		if( DWORD parseErr = XMAPlaybackGetParseError( m_state.pXmaPlayback, 0 ) )
			AUDIO_CORE_ERROR( "XMA Parser error %d\n", parseErr );
		
		DWORD dwPos = XMAPlaybackGetStreamPosition(m_state.pXmaPlayback, 0);
		AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - stream pos is %d/%d (in samples)", dwPos);
		
		XMAPlaybackDebugDump(m_state.pXmaPlayback);

		DWORD dwError = XMAPlaybackGetErrorBits(m_state.pXmaPlayback, 0 );
		if (dwError & XMA_PLAYBACK_ERROR_NONE)
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_NONE");
		if (dwError & XMA_PLAYBACK_ERROR_WRITE_BUFFER_INVALID)
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_WRITE_BUFFER_INVALID");
		if (dwError & XMA_PLAYBACK_ERROR_INSUFFICIENT_WRITE_BUFFER_SPACE)
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_INSUFFICIENT_WRITE_BUFFER_SPACE");
		if (dwError & XMA_PLAYBACK_ERROR_READ_BUFFER_INVALID) 
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_READ_BUFFER_INVALID");
		if (dwError & XMA_PLAYBACK_ERROR_FRAME_CROSSES_BOUNDARY_INTO_INVALID_READ_BUFFER) 
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_FRAME_CROSSES_BOUNDARY_INTO_INVALID_READ_BUFFER");
		if (dwError & XMA_PLAYBACK_ERROR_FRAME_CROSSES_BOTH_READ_BUFFER_BOUNDARIES) 
			AUDIO_CORE_DEBUG("~~~STREAM~~~ XMADecoder::GetData() - XMAPlaybackGetErrorBits() :  XMA_PLAYBACK_ERROR_FRAME_CROSSES_BOTH_READ_BUFFER_BOUNDARIES");
	}
	#endif // #ifndef CFG_SPEED
*/
   // return bResult;


	#ifndef CFG_GOLD
	if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid() && m_state.pXmaPlayback)
	{
		DWORD dwPos = XMAPlaybackGetStreamPosition(m_state.pXmaPlayback, 0);
		DWORD totNumSamples = m_waveData.uFrames * 512;
		AUDIO_CORE_DEBUG("%d ~~~STREAM~~~ XMADecoder::GetData() - returning %d bytes. stream pos is %.2f percent: %d / %d (samples)", System::Get()->GetFrameNum(), uAvailable, (F32)dwPos / (F32)totNumSamples, dwPos, totNumSamples);
	}
	#endif

	return uAvailable;
}

////////////////////////////////////////////////////////////////////////////////
//	GetNextBuffer
//	note: if the data is not available, the sound will abort as
//	this func will fail
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::XMADecoder::GetNextBuffer()
{
	U8 uNext = 1 - m_uRead;

	if (uNext != m_uWrite)
	{
		if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
			AUDIO_CORE_DEBUG("%d XMADecoder::GetNextBuffer() - changed read buffer. read is now %d, m_uWrite is %d", System::Get()->GetFrameNum(), uNext, m_uWrite);
		m_uRead = uNext;
		return true;
	}
	else
	{
		if (g_bAudioPrintXmaStreamerDebug && m_waveData.hWaveData.IsValid())
			AUDIO_CORE_DEBUG("%d XMADecoder::GetNextBuffer() - failed. read is %d, m_uWrite is %d", System::Get()->GetFrameNum(), m_uRead, m_uWrite);
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	PeekNextBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::XMADecoder::PeekNextBuffer()
{
	U8 uNext = 1 - m_uRead;
	return (uNext != m_uWrite);
}

#else
////////////////////////////////////////////////////////////////////////////////
//	GetData
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::XMADecoder::GetData()
{
    bool bResult = false;
	U32 uAvailable = 0;

    if (m_state.pXmaPlayback && (XMAPlaybackQueryContextsAllocated(m_state.pXmaPlayback) == TRUE))
    {
        //  aquire lock
        HRESULT hr = XMAPlaybackRequestModifyLock(m_state.pXmaPlayback);
		if (hr != S_OK)
			AUDIO_CORE_FATAL("XMAPlaybackRequestModifyLock() failed in XMADecoder::GetData() with error code %d\n", hr);
  
		hr = XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback);   //  TODO: don't block
		if (hr != S_OK)
			AUDIO_CORE_FATAL("XMAPlaybackWaitUntilModifyLockObtained() failed in XMADecoder::GetData() with error code %d\n", hr);
    
        uAvailable = XMAPlaybackQueryAvailableData(m_state.pXmaPlayback, 0);
        U32 uSamplesRequired = m_state.uOctaveMaxPitch * 128;

        if (uAvailable >= uSamplesRequired)
        {
            //  data ready
            U32 uRemaining = uSamplesRequired;
#ifdef XMA_DOUBLE_BUFFER
            S16* pBuffer = m_asDecoded[m_uWrite];
#else
            S16* pBuffer = m_asDecoded;
#endif

            //  loop until all data copied
            while (uRemaining > 0)
            {
                S16* asData = NULL;

                U32 uRead = XMAPlaybackConsumeDecodedData(m_state.pXmaPlayback, 0, uRemaining, (void**)&asData);
                
				memcpy(pBuffer, asData, uRead * sizeof(S16) * m_waveData.uChannels);

                uRemaining -= uRead; // * uChannels
                pBuffer += uRead * m_waveData.uChannels;

                m_state.uProgress += uRead;
            }

            bResult = true;


        }
		else if (uAvailable > 0)
		{
            //  data ready
            U32 uRemaining = uAvailable;
#ifdef XMA_DOUBLE_BUFFER
            S16* pBuffer = m_asDecoded[m_uWrite];
#else
            S16* pBuffer = m_asDecoded;
#endif

            //  loop until all data copied
            while (uRemaining > 0)
            {
                S16* asData = NULL;

                U32 uRead = XMAPlaybackConsumeDecodedData(m_state.pXmaPlayback, 0, uRemaining, (void**)&asData);

                memcpy(pBuffer, asData, uRead * sizeof(S16) * m_waveData.uChannels);

				uRemaining -= uRead; // * uChannels
                pBuffer += uRead * m_waveData.uChannels;

                m_state.uProgress += uRead;
            }

			S32 uNewAvailable = 0;
		
			if(g_debugAudioBlockXMA)	//this prevents most looping glitches..but at a cost
			{
			//[cian] BLOCKING -TEMPORARY-...to ensure we have a continous stream of data. need to double buffer it..
				U32 uRemaining = uSamplesRequired - uAvailable;

				while(uRemaining > 0)
				{
					XMAPlaybackResumePlayback(m_state.pXmaPlayback); //resume, since we've consumed the decoded data, it can make way for new data

						if(XMAPlaybackIsIdle(m_state.pXmaPlayback,0)) //need to break out of loop if playback stops
							break;

					XMAPlaybackRequestModifyLock(m_state.pXmaPlayback); //
					XMAPlaybackWaitUntilModifyLockObtained(m_state.pXmaPlayback);   //  block so we can actually retrieve data
					uNewAvailable = XMAPlaybackQueryAvailableData(m_state.pXmaPlayback, 0); //check if there's data ready

				//	if(uNewAvailable)
				//	{
						S16* asData = NULL;
                        
						U32 uRead = XMAPlaybackConsumeDecodedData(m_state.pXmaPlayback, 0, uRemaining, (void**)&asData); //finally...get the missing data
                        
				        memcpy(pBuffer, asData, uRead * sizeof(S16) * m_waveData.uChannels);
					//}

					uRemaining -= uRead;

					pBuffer += uRead * m_waveData.uChannels;
				}
				uAvailable = uSamplesRequired;
			}
			else
			{
				// pad remaining with 0
				memset(pBuffer, 0, (uSamplesRequired - uAvailable) * sizeof(S16) * m_waveData.uChannels);
            }
            
            bResult = true;
		}
		else if ( XMAPlaybackIsIdle(m_state.pXmaPlayback, 0) )
            m_state.bPlaying = false;

        XMAPlaybackResumePlayback(m_state.pXmaPlayback);
    }

#ifdef XMA_DOUBLE_BUFFER
	m_uWrite = 1 - m_uWrite;
#endif

   // return bResult;
	return uAvailable;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
//File writer
//U8 asData[4098*4];
//
//static bool m_bWriteFile = false;
//static bool m_bRecording = false;
//
//if (m_bWriteFile)
//{
//	if (!m_bRecording)
//	{
//	#if defined(PLATFORM_XENON)
//		m_File.OpenWrite("GAME:\\test.raw");
//	#else
//		m_File.OpenWrite("C:\\Documents and Settings\\nickb\\Desktop\\test.raw");
//	#endif
//		m_bRecording = true;
//	}
//}
//else
//{
//if (m_bRecording)
//{
//	m_File.Close();
//	m_bRecording = false;
//}
//}

////if (m_bRecording)
////{
////
////for (U32 uCount = 0; uCount < (uFrameLength); uCount++)
////{
//////S16 tempo = m_asDecoded[uCount];
////U32 tempo = *(U32*)&afOutL[uCount];
////asData[uCount*4] = tempo & 0xff;
////asData[uCount*4+1] = (tempo >> 8) & 0xff;
////asData[uCount*4+2] = (tempo >> 16) & 0xff;
////asData[uCount*4+3] = tempo >> 24;
////
//////		S16 tempo = m_DecoderBuffer[uCount];
//////		asData[uCount*2] = tempo & 0xff;
//////		asData[uCount*2+1] = (tempo >> 8) & 0xff;
////////		asData[2] = (tempo >> 16) & 0xff;
////////		asData[3] = tempo >> 24;
////
////}
////
////m_File.Write(asData, uFrameLength * sizeof(F32));//
////}
