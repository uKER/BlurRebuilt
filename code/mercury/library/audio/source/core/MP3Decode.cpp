#include "AudioPch.h"
#include "coreheaders.h"


Debug::Bool g_bDoMP3DecoderMemoryOperations("Audio.DoMP3DecoderMemoryOperations", true);
Debug::Bool g_bCallMP3LibraryFunctions("Audio.CallMP3LibraryFunctions", true);


//template <class T>
//inline T min(T a, T b) { return (a < b) ? a : b; }
namespace Audio
{
	inline U32 min(U32 a, U32 b) { return (a < b) ? a : b; }
}

Audio::Core::MP3Stream::MP3Stream() : m_UnconsumedDecodedData(SIZEOF_MP3_UNCONSUMED_DATA_BUFFERS)
{
	m_pMP3Data = 0;
	m_pDecoderStreamInstance = 0;
	m_pContext1 = 0;
	m_pContext2 = 0;
//	m_ps16UnconsumedDecodedDataL = 0;
//	m_ps16UnconsumedDecodedDataR = 0;
	m_u32NumSamplesInDecodedBuffer = 0;
	m_s16TempDestBuffer = 0;
//	m_s16TempDestBufferR = 0;
	m_pu8SrcBuffer = 0;
	m_u32EncodedDataTotalSizeBytes = 0;
	m_pManager = 0;
	m_bDecodeRequestSubmitted = false;
	m_u32SamplesStillNeeded = 0;
//	m_pFinalDestBufferL = 0;
//	m_pFinalDestBufferR = 0;
//	m_bRequestedDataReady = false;
	m_pNextUpdater = 0;
	m_pPrevUpdater = 0;
	m_pNextDataMoveItem = 0;
	m_pPrevDataMoveItem = 0;
	m_u32OriginalNumSamplesRequested = 0;
	m_bDecodingBegun = false;
	m_bPrevReadSuccessful = false;
	m_bIsInDataMoveList = false;
	m_bResourcesAllocated = false;
	m_bLoop = false;
	m_bUseable = false;
	m_bDeallocatePending = false;
	m_bInUpdateList = false;
	m_bWaveServerUpdateRequestSubmitted = false;
}


Audio::Core::MP3Stream::~MP3Stream()
{
	DeallocateResources();
}


bool Audio::Core::MP3Stream::Init(U8* pMP3Data, U32 u32DataSizeBytes, U8 u8NumChannels, bool bLoop, MP3DecodeManager* pManager)
{
	m_pMP3Data = pMP3Data;
	m_u32EncodedDataOffset = 0;
	m_u8NumChannels = u8NumChannels;
	m_u32EncodedDataTotalSizeBytes = u32DataSizeBytes;
	m_pManager = pManager;
	m_bLoop = bLoop;

	m_UnconsumedDecodedData.Init(SIZEOF_MP3_UNCONSUMED_DATA_BUFFERS * u8NumChannels);

	return true;
}


bool Audio::Core::MP3Stream::AllocateResources()
{
	if (m_bResourcesAllocated)
		return true;

	m_bDecodeRequestSubmitted = false;
	m_bWaveServerUpdateRequestSubmitted = false;
	m_u32SamplesStillNeeded = 0;
	m_pNextUpdater = 0;
	m_pPrevUpdater = 0;
	m_pNextDataMoveItem = 0;
	m_pPrevDataMoveItem = 0;
	m_u32OriginalNumSamplesRequested = 0;
	m_u32EncodedDataOffset = 0;

	m_pDecoderStreamInstance = (CellMP3DecoderStreamInstance*)BizAlignedMalloc(sizeof(CellMP3DecoderStreamInstance), 128);

	m_pContext1 = (CellMP3Context*)BizAlignedMalloc(sizeof(CellMP3Context), 128);
	m_pContext2 = (CellMP3Context*)BizAlignedMalloc(sizeof(CellMP3Context), 128);

	m_UnconsumedDecodedData.BecomeActive();

	if (! (m_pDecoderStreamInstance && m_pContext1 && m_pContext2))
		return false;

	m_pDecoderStreamInstance->pContexts[0] = m_pContext1;
	m_pDecoderStreamInstance->pContexts[1] = m_pContext2;

	m_pu8SrcBuffer = (U8*)(BizAlignedMalloc(ENCODE_BUFFER_SIZE_BYTES, 128));

	S32 s32Result = cellMP3DecoderInitStruct(m_pDecoderStreamInstance);

	if (s32Result != 0)
	{
		BizAlignedFree(m_pDecoderStreamInstance);
		BizAlignedFree(m_pContext1);
		BizAlignedFree(m_pContext2);

		return false;
	}

	m_s16TempDestBuffer = (S16*)(BizAlignedMalloc(DECODE_BUFFER_SIZE_SAMPLES * sizeof(S16) * m_u8NumChannels, 128));

	m_pDecoderStreamInstance->head.nChannels = m_u8NumChannels;
	m_pDecoderStreamInstance->head.nSkipFirstPacket = 1;
	m_pDecoderStreamInstance->head.nPcmBufferBytesWritten = 0;
	m_pDecoderStreamInstance->head.pPcmBuffer2 = 0;

	m_bResourcesAllocated = true;

	m_bUseable = true;

	return true;
}


bool Audio::Core::MP3Stream::DeallocateResources()
{
	if (! m_bResourcesAllocated)
		return true;

	m_bResourcesAllocated = false;
	m_bDeallocatePending = false;

	m_UnconsumedDecodedData.BecomeInactive();

// context size is (544 + 576) * 4 + 512 = 4992
	if (m_pContext1)
	{
		BizAlignedFree(m_pContext1);
		m_pContext1 = 0;
	}

// context size is (544 + 576) * 4 + 512 = 4992
	if (m_pContext2)
	{
		BizAlignedFree(m_pContext2);
		m_pContext2 = 0;
	}

	if (m_pDecoderStreamInstance)
	{
		BizAlignedFree(m_pDecoderStreamInstance);
		m_pDecoderStreamInstance = 0;
	}

//	DECODE_BUFFER_SIZE_SAMPLES = 1024 * 2 * sizeof(S16) = 4096
	if (m_s16TempDestBuffer)
	{
		BizAlignedFree(m_s16TempDestBuffer);
		m_s16TempDestBuffer = 0;
	}

//	ENCODE_BUFFER_SIZE_BYTES = 1024 * 2 = 2048
	if (m_pu8SrcBuffer)
	{
		BizAlignedFree(m_pu8SrcBuffer);
		m_pu8SrcBuffer = 0;
	}

	return true;
}


bool Audio::Core::MP3Stream::SendDataRequest(U32 u32SamplesRequired)
{
	if (! IsStreamUseable())
		return false;

	if (m_bDecodeRequestSubmitted)
		return false;

	m_u32SamplesStillNeeded = u32SamplesRequired;
	m_u32OriginalNumSamplesRequested = u32SamplesRequired;

	if (m_u32SamplesStillNeeded && (! HasFinished()))
		return SendRequestToDecoder();

	return true;
}


bool Audio::Core::MP3Stream::GetData(S16* ps16Data, U32 u32BytesRequired, U32* pu32BytesWritten)
{
	if (! IsStreamUseable())
		return false;

	m_UnconsumedDecodedData.GetData((U8**)(&ps16Data), u32BytesRequired, pu32BytesWritten);

	// should only be called from streamer thread..
//	WriteDecodedDataIntoCircularBuffer();

	//m_u32SamplesStillNeeded -= (*pu32BytesWritten) / 2;

	//if (m_u32SamplesStillNeeded)
	//	RequestFillBuffers();

	return true;
}


bool Audio::Core::MP3Stream::FillOutputBuffer(CircularBuffer* pDestCircularBuffer)
{
	if (! IsStreamUseable())
		return false;

	S16 s16TempBuffer[2048];

	U32 u32BytesToRequest = min(sizeof(s16TempBuffer), pDestCircularBuffer->GetSpaceInBytes());
	U32 u32BytesRetrieved = 0;
	U32 u32BytesWritten = 0;

	GetData(s16TempBuffer, u32BytesToRequest, &u32BytesRetrieved);

	if (u32BytesRetrieved)
		pDestCircularBuffer->WriteData((U8*)s16TempBuffer, u32BytesRetrieved, &u32BytesWritten);

	if (HasFinished())
	{
		// temp hack!!! set encoded data offset back to 0 if it's a loop!!!
		if (m_bLoop)
			m_u32EncodedDataOffset = 0;
		else
			pDestCircularBuffer->SetEndOfStreamReached();
	}

//	if (u32BytesWritten)
//		SendRequestToDecoder();

	return (u32BytesWritten > 0);
}


bool Audio::Core::MP3Stream::IsStreamUseable()
{
	return m_bUseable;
}


bool Audio::Core::MP3Stream::SendRequestToDecoder()
{
	if (! IsStreamUseable())
		return false;

	if (m_bDecodeRequestSubmitted)
		return false;

	Assert (m_u32EncodedDataOffset < m_u32EncodedDataTotalSizeBytes);

	if (m_u32NumSamplesInDecodedBuffer)
		return false;

	m_pDecoderStreamInstance->head.pMp3DataBuffer = m_pu8SrcBuffer;

	U32 u32SamplesDecoded = 0;
	CellMP3DecoderPacketInfo packetInfo;

	U32 u32EncodedBytesToCopy = min(ENCODE_BUFFER_SIZE_BYTES, GetEncodedBytesAvailable());
	memcpy(m_pu8SrcBuffer, m_pMP3Data + m_u32EncodedDataOffset, u32EncodedBytesToCopy);

	S32 s32Result = MP3DecoderGetPacketInfo(m_pu8SrcBuffer, &packetInfo);

	if (packetInfo.PacketSizeBytes > ENCODE_BUFFER_SIZE_BYTES)
	{
		AUDIO_CORE_ERROR("Encoded MP3 data temp buffer not big enough");
		return false;
	}

	m_pDecoderStreamInstance->head.nMp3BufferSizeBytes = packetInfo.PacketSizeBytes;
	m_pDecoderStreamInstance->head.nMp3BufferBytesRead = 0;

	m_pDecoderStreamInstance->head.pPcmBuffer = m_s16TempDestBuffer;
	m_pDecoderStreamInstance->head.nPcmBufferSizeBytes = DECODE_BUFFER_SIZE_SAMPLES * sizeof(S16);
	m_pDecoderStreamInstance->head.nPcmBufferBytesWritten = 0;

	//if (m_u8NumChannels == 2)
	//	m_pDecoderStreamInstance->head.pPcmBuffer2 = m_s16TempDestBufferR;
	//else
		m_pDecoderStreamInstance->head.pPcmBuffer2 = 0;

	if (g_bCallMP3LibraryFunctions)
		s32Result = cellMP3DecoderAddStruct(m_pManager->GetDecoderMemory(), m_pDecoderStreamInstance);
	else
		s32Result = 0;

	if (s32Result != 0)
	{
		AUDIO_CORE_ERROR("MP3 library error- cellMP3DecoderAddStruct() failed");
		return false;
	}

	m_bDecodeRequestSubmitted = true;

	return true;
}


void Audio::Core::MP3Stream::WriteDecodedDataIntoCircularBuffer()
{
	if (! IsStreamUseable())
		return;

	if (! m_UnconsumedDecodedData.HasSpace())
		return;

	U32 u32BufferBytesWritten = 0;
	m_UnconsumedDecodedData.WriteData((U8*)m_s16TempDestBuffer,  m_u32NumSamplesInDecodedBuffer * sizeof(S16) * m_u8NumChannels, &u32BufferBytesWritten);
	//if (u32BufferBytesWritten / sizeof(S16) < m_u32SamplesStillNeeded)
	//	m_u32SamplesStillNeeded -= u32BufferBytesWritten / sizeof(S16);
	//else
	//	m_u32SamplesStillNeeded = 0;

	Assert ((u32BufferBytesWritten / sizeof(S16) / m_u8NumChannels) <= m_u32NumSamplesInDecodedBuffer);

	m_u32NumSamplesInDecodedBuffer -= u32BufferBytesWritten / sizeof(S16) / m_u8NumChannels;

	if (m_u32NumSamplesInDecodedBuffer && g_bDoMP3DecoderMemoryOperations)
	{
		void* pDest = (void*)m_s16TempDestBuffer;
		void* pSrc = (void*)(m_s16TempDestBuffer + (u32BufferBytesWritten / sizeof(S16)));
		U32 u32Size = m_u32NumSamplesInDecodedBuffer * sizeof(S16) * m_u8NumChannels;
		U32 u32Diff = (U8*)pSrc - (U8*)pDest;
//printf("src=0x%x, dest=)x%x, size=%d, diff=%d, buffersize=%d\n", pSrc, pDest, u32Size, u32Diff, DECODE_BUFFER_SIZE_SAMPLES * sizeof(S16) * m_u8NumChannels);
//if (u32Diff + u32Size > DECODE_BUFFER_SIZE_SAMPLES * sizeof(S16) * m_u8NumChannels)
// DebugBreak();
		memmove(pDest, pSrc, u32Size);
	}
}


void Audio::Core::MP3Stream::Update()
{
	if (DoesDecodedDataNeedMoving() && m_UnconsumedDecodedData.HasSpace())
		WriteDecodedDataIntoCircularBuffer();

	if (! m_bDecodeRequestSubmitted)
		return;

	if (m_u32NumSamplesInDecodedBuffer)
		return;

	m_bDecodeRequestSubmitted = false;

	if (g_bCallMP3LibraryFunctions)
		m_u32EncodedDataOffset += m_pDecoderStreamInstance->head.nMp3BufferBytesRead;
	else
		m_u32EncodedDataOffset += m_pDecoderStreamInstance->head.nMp3BufferSizeBytes;

//	u32SamplesToWrite = min(u32SamplesStillNeeded, m_pDecoderStreamInstance->head.nPcmBufferBytesWritten / sizeof(S16));

//	memcpy((char*)(pDestBufferL + u32SamplesDecoded + u32UnconsumedSamplesToCopy), (char*)m_s16TempDestBufferL, u32SamplesToWrite * sizeof(S16));

	//if (m_u8NumChannels == 2)
	//	memcpy((char*)(pDestBufferR + u32SamplesDecoded + u32UnconsumedSamplesToCopy), (char*)m_s16TempDestBufferR, u32SamplesToWrite * sizeof(S16));

//	U32 u32PrevNumUnconsumedDecodedSamplesAvailable = m_u32NumSamplesInDecodedBuffer;
	U32 u32SamplesDecoded = 600;	// temp hack
	if (g_bCallMP3LibraryFunctions) 
		u32SamplesDecoded = (U32)(m_pDecoderStreamInstance->head.nPcmBufferBytesWritten / sizeof(S16) / m_u8NumChannels);
	m_u32NumSamplesInDecodedBuffer = u32SamplesDecoded;
	m_u32SamplesStillNeeded = min(0, m_u32SamplesStillNeeded - u32SamplesDecoded);

	if (u32SamplesDecoded)
//	{
		m_bDecodingBegun = true;
//		m_bPrevReadSuccessful = true;
//	}
//	else
//		m_bPrevReadSuccessful = false;
	m_bPrevReadSuccessful = true;

	//if (m_u32SamplesStillNeeded > u32SamplesDecoded)
	//	m_u32SamplesStillNeeded -= u32SamplesDecoded;
	//else
	//	m_u32SamplesStillNeeded = 0;

	Assert ((m_u32NumSamplesInDecodedBuffer * sizeof(S16)) <= SIZEOF_MP3_UNCONSUMED_DATA_BUFFERS);

	WriteDecodedDataIntoCircularBuffer();

	//if (m_u32SamplesStillNeeded)
	//	SendRequestToDecoder();

//	memcpy((char*)(m_ps16UnconsumedDecodedDataL + u32PrevNumUnconsumedDecodedSamplesAvailable), (char*)(m_s16TempDestBufferL), m_u32NumSamplesInDecodedBuffer * sizeof(S16));

	//if (m_u8NumChannels == 2)
	//	memcpy((char*)(m_ps16UnconsumedDecodedDataR + u32PrevNumUnconsumedDecodedSamplesAvailable), (char*)(m_s16TempDestBufferR), m_u32NumSamplesInDecodedBuffer * sizeof(S16));
}


//Audio::Core::MP3DecodeManager::MP3DecodeManager()
//{
//	m_pSpurs = 0;
//	m_pMP3DecoderMem = 0;
//	m_u32NumStreamsRequiringUpdate = 0;
//	m_pFirst = 0;
//	m_pLast = 0;
//}


Audio::Core::MP3DecodeManager::~MP3DecodeManager()
{
	if (m_pMP3DecoderMem)
	{
		cellMP3DecoderShutdown(m_pMP3DecoderMem);
		free(m_pMP3DecoderMem);
	}
}


bool Audio::Core::MP3DecodeManager::Init()
{
	if (! m_StreamHandles.InitHandleCollection())
		return false;

	m_pSpurs = Scheduler::jqGetSpurs();

	if (! m_pSpurs)
		return false;

	CellMP3DecoderInitParams decoderInitParams;

	uint8_t u8PriorityArray [] = {7, 7, 0, 7, 7, 7, 0, 0};

	decoderInitParams.pSpurs = m_pSpurs;
	decoderInitParams.pPriorities = u8PriorityArray;
	decoderInitParams.flags = CELL_MP3_DISABLE_SPU_PRINTF_SERVER;
	decoderInitParams.nMaxStreams = 128;

	S32 s32MP3DecoderMemSize = 0;

	S32 s32Result = cellMP3DecoderLibGetRequiredSize(&decoderInitParams, &s32MP3DecoderMemSize);

	if (s32Result != 0)
		return false;

	m_pMP3DecoderMem = (U8*)(memalign(128, s32MP3DecoderMemSize));

	U8* pTempMem = (U8*)(memalign(128, CELL_MP3_TEMP_INIT_BUFFER_SIZE));

	s32Result = cellMP3DecoderInit(m_pMP3DecoderMem, s32MP3DecoderMemSize, pTempMem, CELL_MP3_TEMP_INIT_BUFFER_SIZE, &decoderInitParams);

	if (s32Result != 0)
		return false;

	free(pTempMem);

	m_pFirstDataMoveItem = 0;
	m_pLastDataMoveItem = 0;

	return true;
}


Audio::MP3Handle Audio::Core::MP3DecodeManager::CreateMP3Stream(U8* pMP3Data, U32 u32DataSize, U8 u8NumChannels, bool bLoop)
{
	MP3Handle invalidHandle;

	MP3Stream* pNewStream = new MP3Stream();

	if (! pNewStream)
		return invalidHandle;

	if (! pNewStream->Init(pMP3Data, u32DataSize, u8NumChannels, bLoop, this))
	{
		delete pNewStream;
		return invalidHandle;
	}

	MP3Handle retHandle = m_StreamHandles.RequestUnusedHandle();

	if (retHandle.IsValid())
	{
		m_StreamHandles.MapResourceToHandle(retHandle, (HandleResource)pNewStream);	// [Blur2PC][x64]
	}

	return retHandle;
}


bool Audio::Core::MP3DecodeManager::AllocateStreamResources(MP3Handle hStream)
{
	if (! hStream.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
	{
		if (! pStream->IsDeallocatePending())
			return pStream->AllocateResources();
	}

	return false;
}


bool Audio::Core::MP3DecodeManager::DeallocateStreamResourcesImmediate(MP3Handle hStream)
{
	if (! hStream.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	Assert (! pStream->m_bDecodeRequestSubmitted);

	if (pStream)
		return pStream->DeallocateResources();

	return false;
}


bool Audio::Core::MP3DecodeManager::DeallocateStreamResources(WaveServerHandle hStream)
{
	if (! hStream.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource((MP3Handle)hStream));

	if (pStream)
	{
		pStream->SetNotUseable();
		pStream->SetDeallocatePending();
	}
	else
		return false;

	WaveStreamReadRequest req;

	req.m_hStream = hStream;
	req.m_bDeallocateResources = true;
	req.m_u8ReadRequestType = eMessageTypeMP3;

	LoadData(req);

	return true;
}


bool Audio::Core::MP3DecodeManager::AreStreamResourcesAllocated(MP3Handle hStream)
{
	if (! hStream.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
		return pStream->AreResourcesAllocated();

	return false;
}


bool Audio::Core::MP3DecodeManager::DestroyMP3Stream(MP3Handle hStream)
{
	if (! hStream.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
	{
		pStream->DeallocateResources();
		delete pStream;
		m_StreamHandles.FlagHandleForRelease(hStream);
		m_StreamHandles.ReleaseHandle(hStream);
		return true;
	}

	return false;
}


bool Audio::Core::MP3DecodeManager::SendRequestToMP3Decoder(MP3Handle hStream, U32 u32SamplesRequired)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));
	bool bRet = false;

	if (pStream)
	{
		bRet = pStream->SendDataRequest(u32SamplesRequired);

		if (bRet && pStream->GetNumSamplesStillNeeded())
			AddToUpdateList(pStream);
	}

	return bRet;
}


bool Audio::Core::MP3DecodeManager::SendDecodeRequestToWaveServerThread(WaveServerHandle hStream, U32 u32SamplesRequired)
{
	WaveStreamReadRequest readReq;

	readReq.m_bLoop = false;
	readReq.m_hStream = hStream;
	readReq.m_u32BytesToRead = u32SamplesRequired;
	readReq.m_u8ReadRequestType = eMessageTypeMP3;
	readReq.m_bDeallocateResources = false;

	LoadData(readReq);	

	return true;
}


// should only be called from Wave Server loading thread
void Audio::Core::MP3DecodeManager::Update()
{
	AUDIO_PERF_EVENT("Audio.WaveServer.MP3Update");

	MP3Stream* pNextDataMoveStream = 0;

	for (MP3Stream* pStream = m_pFirstDataMoveItem; pStream; pStream = pNextDataMoveStream)
	{
		pNextDataMoveStream = pStream->GetNextDataMoveItem();

		pStream->WriteDecodedDataIntoCircularBuffer();

		if (! pStream->DoesDecodedDataNeedMoving())
			RemoveFromDataMoveList(pStream);
	}

	while(m_u32NumStreamsRequiringUpdate)
	{
		S32 s32Result = 0;
		
		if (g_bCallMP3LibraryFunctions)
			s32Result = cellMP3DecoderProcess(m_pMP3DecoderMem);

		assert (s32Result == 0);

		MP3Stream* pNextStream = 0;

		for (MP3Stream* pStream = m_pFirst; pStream; pStream = pNextStream)
		{
			pNextStream = pStream->GetNext();

			pStream->Update();
			// were we unable to move all of the decoded data from the multistream dest buffer into our circular buffer?
			if (pStream->DoesDecodedDataNeedMoving())
				AddToDataMoveList(pStream);
			else if (pStream->IsInDataMoveList())
				RemoveFromDataMoveList(pStream);

//			pStream->CopyDecodedDataIntoBuffers();

			if ((! pStream->GetNumSamplesStillNeeded()) || (pStream->HasFinished() /*&& (! pStream->GetNumSamplesAvailable())*/))
			{
				RemoveFromUpdateList(pStream);
				pStream->UnsetWaveServerRequestState();
			}
			else
			{
				if (! pStream->HasFinished())
					pStream->SendRequestToDecoder();
			}
		}
	}
}


U32 Audio::Core::MP3DecodeManager::GetNumSamplesRequested(MP3Handle hStream)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
		return pStream->GetNumSamplesRequested();

	return 0;
}


bool Audio::Core::MP3DecodeManager::WasLastDecodeSuccessful(MP3Handle hStream)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
		return pStream->WasLastReadSuccessful();

	return false;
}


void Audio::Core::MP3DecodeManager::ReadRequestProcessed(WaveServerHandle hStream, U32 u32NumBytesRead)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource((MP3Handle)hStream));

	if (pStream)
		pStream->WriteDecodedDataIntoCircularBuffer();
}


bool Audio::Core::MP3DecodeManager::IsStreamUseable(MP3Handle hStream)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
		return pStream->IsStreamUseable();

	return false;
}


//bool Audio::Core::MP3DecodeManager::RetrieveData(MP3Handle hStream, S16** ppDestBufferL, S16** ppDestBufferR)
//{
//	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));
//
//	if (pStream)
//		return pStream->RetrieveData(ppDestBufferL, ppDestBufferR);
//
//	return false;
//}


bool Audio::Core::MP3DecodeManager::GetData(WaveServerHandle hWave, CircularBuffer* pDestCircularBuffer)
{
	bool bRet = false;

	if (! hWave.IsValid())
		return false;

	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource((MP3Handle)hWave));

	if (pStream)
	{
		bRet = pStream->FillOutputBuffer(pDestCircularBuffer);

		if (/*bRet && */pStream->NeedsData())
		{
			if (! pStream->m_bWaveServerUpdateRequestSubmitted)
			{
				Assert(pStream->m_pMP3Data && pStream->m_pDecoderStreamInstance);
				pStream->m_bWaveServerUpdateRequestSubmitted = true;

				RequestFillBuffers(hWave);
			}
		}
	}

	return bRet;
}


bool Audio::Core::MP3DecodeManager::RequestFillBuffers(WaveServerHandle hWave)
{
	return SendDecodeRequestToWaveServerThread(hWave, SIZEOF_MP3_UNCONSUMED_DATA_BUFFERS);	// TODO: work out how much space is free in the buffer, and request that number of bytes
}


bool Audio::Core::MP3DecodeManager::HasFinished(MP3Handle hStream)
{
	MP3Stream* pStream = (MP3Stream*)(m_StreamHandles.GetResource(hStream));

	if (pStream)
		return pStream->HasFinished();

	return true;
}


void Audio::Core::MP3DecodeManager::AddToUpdateList(MP3Stream* pStream)
{
	if (pStream->m_bInUpdateList)
		return;

	if (pStream->m_pNextUpdater != 0 || pStream->m_pPrevUpdater != 0)
	{
		Assert(0);
		return;
	}

	if (m_pLast == pStream || m_pFirst == pStream)
	{
		Assert(0);
		return;
	}

	pStream->m_bInUpdateList = true;

	if (! m_pLast)
	{
		m_pFirst = pStream;
		m_pLast = pStream;
	}
	else
	{
		m_pLast->SetNext(pStream);
		pStream->SetPrev(m_pLast);
		m_pLast = pStream;
	}

	m_u32NumStreamsRequiringUpdate++;

// sanity check.
//int s32Check = 0;
//for (MP3Stream* pTestStream = m_pFirst; pTestStream; pTestStream = pTestStream->m_pNextUpdater)
// s32Check++;
//
//if (s32Check != m_u32NumStreamsRequiringUpdate)
// DebugBreak();
}


void Audio::Core::MP3DecodeManager::RemoveFromUpdateList(MP3Stream* pStream)
{
	if ((! m_pLast) || (! m_pFirst))
	{
		Assert(0);
		return;
	}

	if (pStream == m_pLast)
		m_pLast = pStream->m_pPrevUpdater;

	if (pStream == m_pFirst)
		m_pFirst = pStream->m_pNextUpdater;

	if (pStream->m_pPrevUpdater)
		pStream->m_pPrevUpdater->SetNext(pStream->m_pNextUpdater);

	if (pStream->m_pNextUpdater)
		pStream->m_pNextUpdater->SetPrev(pStream->m_pPrevUpdater);

	pStream->m_pPrevUpdater = 0;
	pStream->m_pNextUpdater = 0;

	pStream->m_bInUpdateList = false;

	m_u32NumStreamsRequiringUpdate--;

// sanity check.
//int s32Check = 0;
//for (MP3Stream* pTestStream = m_pFirst; pTestStream; pTestStream = pTestStream->m_pNextUpdater)
// s32Check++;
//
//if (s32Check != m_u32NumStreamsRequiringUpdate)
// DebugBreak();
}


void Audio::Core::MP3DecodeManager::AddToDataMoveList(MP3Stream* pStream)
{
	if (pStream->m_bIsInDataMoveList)
		return;

	pStream->m_bIsInDataMoveList = true;

	if (! m_pLastDataMoveItem)
	{
		m_pFirstDataMoveItem = pStream;
		m_pLastDataMoveItem = pStream;
	}
	else
	{
		m_pLastDataMoveItem->SetNextDataMoveItem(pStream);
		pStream->SetPrevDataMoveItem(m_pLastDataMoveItem);
		m_pLastDataMoveItem = pStream;
	}
}


void Audio::Core::MP3DecodeManager::RemoveFromDataMoveList(MP3Stream* pStream)
{
	if (! pStream->m_bIsInDataMoveList)
		return;

	pStream->m_bIsInDataMoveList = false;

	if (pStream == m_pLastDataMoveItem)
		m_pLastDataMoveItem = pStream->m_pPrevDataMoveItem;

	if (pStream == m_pFirstDataMoveItem)
		m_pFirstDataMoveItem = pStream->m_pNextDataMoveItem;

	if (pStream->m_pPrevDataMoveItem)
		pStream->m_pPrevDataMoveItem->SetNextDataMoveItem(pStream->m_pNextDataMoveItem);

	if (pStream->m_pNextDataMoveItem)
		pStream->m_pNextDataMoveItem->SetPrevDataMoveItem(pStream->m_pPrevDataMoveItem);

	pStream->m_pPrevDataMoveItem = 0;
	pStream->m_pNextDataMoveItem = 0;
}
