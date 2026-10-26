#include "AudioPch.h"

// on 360 include a couple of files that allow us to check the DVD layer

#include "coreheaders.h"


#ifdef PLATFORM_PS3
inline U32 min(U32 a, U32 b) { return (a < b) ? a : b; }
#endif


Debug::Bool g_bDoAudioGraphMemMoves("Audio.WaveServer.DoAudioGraphMemMoves", true);
Debug::Bool g_bAudioGraphsAllowDeactivation("Audio.WaveServer.AudioGraphsAllowDeactivation", true);
Debug::Bool g_bAllocateStreamProcessResources("Audio.WaveServer.AllocateStreamProcessResources", true);
Debug::Bool g_bCreateStreamNodes("Audio.WaveServer.CreateStreamNodes", true);
Debug::Bool g_bCreateStreams("Audio.WaveServer.CreateStreams", true);
Debug::Bool g_bRegisterStreamSources("Audio.WaveServer.RegisterStreamSources", true);
Debug::Bool g_bProcessFileStreamRequests("Audio.WaveServer.ProcessFileStreamRequests", true);

bool Audio::Core::WaveServerComponent::LoadData(WaveStreamReadRequest& readRequest)
{
	return GetWaveServer()->SendRequest(readRequest);
}


Audio::Core::WaveStreamLoadingThread::WaveStreamLoadingThread(U32 u32StackSize, WaveServer* pWaveServer) : Thread(u32StackSize)
{
	m_s32RequestQueueReadPointer = 0;
	m_s32RequestQueueWritePointer = 0;
	m_s32ResultQueueReadPointer = 0;
	m_s32ResultQueueWritePointer = 0;

	for (U32 i = 0; i < AUDIO_FILE_STREAMER_QUEUE_SIZE; i++)
	{
		m_aStreamReadRequest[i].m_pFileWrapper = 0;
		m_aStreamReadRequest[i].m_u32BytesToRead = 0;
		m_aStreamReadRequest[i].m_pDestBuffer = 0;
		m_aStreamReadRequest[i].m_bLoop = false;
	}

	for (U32 i = 0; i < AUDIO_FILE_STREAMER_QUEUE_SIZE; i++)
	{
		m_aStreamReadResult[i].m_u32NumBytesRead = 0;
		m_aStreamReadResult[i].m_bLastReadSuccessful = 0;
		m_aStreamReadResult[i].m_hStream = -1;
	}

	m_pWaveServer = pWaveServer;

	SetPriority(eBelowNormal);

#ifdef PLATFORM_PS3
	//m_pWaveServer->m_MP3Manager.Init();
#endif
}


void Audio::Core::WaveStreamLoadingThread::Execute(void* pVoid)
{
	extern MT::Event AudioStreamingEvent;
	AudioStreamingEvent.Wait();
	while (1)
	{
		AUDIO_PERF_BEGIN_EVENT("Audio.WaveServer.WaveServerThreadUpdate");

//#ifdef PLATFORM_PS3
//		MP3Handle ahMP3sDecoding[MAX_MP3_REQUESTS_TO_PROCESS];
//		U32 u32NumMP3sDecoding = 0;
//#endif

		for (bool bResultQueueFull = false; m_s32RequestQueueReadPointer != m_s32RequestQueueWritePointer && (! bResultQueueFull); bResultQueueFull = ((m_s32ResultQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE == m_s32ResultQueueReadPointer))
		{
			WaveStreamReadRequest currentRequest = m_aStreamReadRequest[m_s32RequestQueueReadPointer];
			U32 u32BytesRead = 0;
			bool bSuccessful = false;

			if (currentRequest.m_u8ReadRequestType == eMessageTypeMP3)
			{
//#ifndef PLATFORM_PS3
				AUDIO_CORE_FATAL("Received an MP3 decode request on a platform that doesn't support it!");
				return;
//#else
///*				if (currentRequest.m_bDeallocateResources)
//				{
//					m_pWaveServer->m_MP3Manager.DeallocateStreamResourcesImmediate(MP3Handle(currentRequest.m_hStream));
//				}
//				else */if (u32NumMP3sDecoding < MAX_MP3_REQUESTS_TO_PROCESS)
//				{
//					m_pWaveServer->m_MP3Manager.SendRequestToMP3Decoder(MP3Handle(currentRequest.m_hStream), currentRequest.m_u32BytesToRead);
//					ahMP3sDecoding[u32NumMP3sDecoding] = (MP3Handle)currentRequest.m_hStream;
//					u32NumMP3sDecoding++;
//				}
//				else
//					break;
//#endif
			}
			else
			{
			//pause the main file system, except for this file
				if (g_bProcessFileStreamRequests)
				{
					if (currentRequest.m_bDeallocateResources)
					{
						delete [] (U8*)(currentRequest.m_pDestBuffer);

						// results
						m_ResultLock.Acquire();

						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u8ResultType = eMessageTypeDeallocateResource;

						m_s32ResultQueueWritePointer = (m_s32ResultQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;

						m_ResultLock.Release();
					}
					else
					{
						// if on 360, check if the current DVD layer is the same as the one last used for this file.
						// if not, close the file. (will then get re-opened and hence be looking at the correct layer)


						if (! currentRequest.m_pFileWrapper->IsOpen())
						{
							bool bSuccess = currentRequest.m_pFileWrapper->Open();
							BIZ_ASSERT(bSuccess);

							// if on 360, set the current layer in the file stream wrapper
						}

						Sys::I_File* pFile = currentRequest.m_pFileWrapper->GetFilePointer();

						pFile->SetPosition(currentRequest.m_u32SeekPosition);
						if (pFile->GetPosition() == currentRequest.m_u32SeekPosition)
						{
							Sys::I_BinaryStream::UnderflowContext uc;

							U32 u32BytesNeeded = (currentRequest.m_u32BytesToRead + ((2 * 1024) - 1)) & ~((2 * 1024) - 1);

							uc.readTarget = (U8*)(currentRequest.m_pDestBuffer);
							uc.readSize = u32BytesNeeded;
							uc.bytesRead = 0;

							if (! currentRequest.m_bTempAmaxNullFileReadHack)
								bSuccessful = pFile->Underflow(uc);
							else
							{
								bSuccessful = true;
								uc.bytesRead = currentRequest.m_u32BytesToRead;
								memset(currentRequest.m_pDestBuffer, 0, u32BytesNeeded);
							}

							u32BytesRead = currentRequest.m_u32BytesToRead;
						}
						else
							bSuccessful = false;

						m_ResultLock.Acquire();

						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_bLastReadSuccessful = bSuccessful;
						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_hStream = currentRequest.m_hStream;
						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u32NumBytesRead = u32BytesRead;
						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u8ResultType = eMessageTypeFileStream;
						m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u32RequestId = currentRequest.m_u32RequestId;


						m_s32ResultQueueWritePointer = (m_s32ResultQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;

						m_ResultLock.Release();
					}
				}
				else
				{
				// Debug: simulate successful stream reads
					bSuccessful = true;
					u32BytesRead = 2048;
				}
			}

			m_RequestLock.Acquire();
			m_s32RequestQueueReadPointer = (m_s32RequestQueueReadPointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;
			m_RequestLock.Release();
		}
#ifdef PLATFORM_PS3
		//m_pWaveServer->m_MP3Manager.Update();

//		m_ResultLock.Acquire();
//		for (U32 i = 0; i < u32NumMP3sDecoding; i++)
//		{
////			m_pWaveServer->m_MP3Manager.RetrieveData(ahMP3sDecoding[i]);
//
//			m_aStreamReadResult[m_s32ResultQueueWritePointer].m_bLastReadSuccessful = m_pWaveServer->m_MP3Manager.WasLastDecodeSuccessful(ahMP3sDecoding[i]);
//			m_aStreamReadResult[m_s32ResultQueueWritePointer].m_hStream = ahMP3sDecoding[i].Value();
//			m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u32NumBytesRead = m_pWaveServer->m_MP3Manager.GetNumSamplesRequested(ahMP3sDecoding[i]);
//			m_aStreamReadResult[m_s32ResultQueueWritePointer].m_u8ResultType = eMessageTypeMP3;
//
//			m_s32ResultQueueWritePointer = (m_s32ResultQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;
//		}
//
//		m_ResultLock.Release();
#endif

		AUDIO_PERF_END_EVENT();

		SleepThread(1);

		if (Audio::g_bQuitAllTasks) break;
	}

}


bool Audio::Core::WaveStreamLoadingThread::SendRequest(WaveStreamReadRequest& request)
{
	bool bQueueFull = false;

	m_RequestLock.Acquire();
	bQueueFull = ((m_s32RequestQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE == m_s32RequestQueueReadPointer);
	m_RequestLock.Release();

	if (bQueueFull)
	{
		AUDIO_CORE_ERROR("WaveServer load request queue full");
		return false;
	}

	m_aStreamReadRequest[m_s32RequestQueueWritePointer] = request;

	m_RequestLock.Acquire();
	m_s32RequestQueueWritePointer = (m_s32RequestQueueWritePointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;
	m_RequestLock.Release();

	return true;
}


bool Audio::Core::WaveStreamLoadingThread::GetResult(WaveStreamReadResult& result)
{
	bool bResultsAvailable = false;

	m_ResultLock.Acquire();
	bResultsAvailable = (m_s32ResultQueueReadPointer != m_s32ResultQueueWritePointer);
	m_ResultLock.Release();

	if (! bResultsAvailable)
		return false;

	result = m_aStreamReadResult[m_s32ResultQueueReadPointer];

	m_ResultLock.Acquire();
	m_s32ResultQueueReadPointer = (m_s32ResultQueueReadPointer + 1) % AUDIO_FILE_STREAMER_QUEUE_SIZE;
	m_ResultLock.Release();

	return true;
}


void Audio::Core::WaveStreamNode::Init()
{
	m_hProcess.SetInvalid();
	m_pOutputCircularBuffer = 0;
//	m_StreamConsumptionInfo.Init();
	m_eState = eDormant;
}


void Audio::Core::WaveStreamNode::Update()
{
	Assert(m_pOutputCircularBuffer);

	if (m_pOutputCircularBuffer->HasReachedEndOfStream())
		m_eState = eStreamFinished;
	else if (m_pOutputCircularBuffer->GetBytesAvailable() < m_pOutputCircularBuffer->GetBufferSize() && g_bAllocateStreamProcessResources)
		Audio::System::Get()->GetWaveServer()->GetData(m_hProcess, m_pOutputCircularBuffer);
}


bool Audio::Core::WaveStreamNode::BecomeActive()
{
	if (! m_pOutputCircularBuffer)
		CreateBuffer();

	bool bRet = m_pOutputCircularBuffer->BecomeActive();
	if (bRet)
	{
		Audio::System::Get()->GetWaveServer()->AllocateStreamProcessResources(m_hProcess);
		m_eState = eActive;
	}

	return bRet;
}


bool Audio::Core::WaveStreamNode::BecomeInactive()
{
//	Assert(m_pOutputCircularBuffer);

	bool bRet = false;
	if (m_pOutputCircularBuffer) 
		bRet = m_pOutputCircularBuffer->BecomeInactive();
	if (bRet)
	{
		SAFE_DELETE_PTR(m_pOutputCircularBuffer);

		Audio::System::Get()->GetWaveServer()->DeallocateStreamProcessResources(m_hProcess);
		m_eState = eDormant;
	}

	return bRet;
}


bool Audio::Core::WaveStreamNode::Destroy()
{
	SAFE_DELETE_PTR(m_pOutputCircularBuffer);
	return Audio::System::Get()->GetWaveServer()->DestroyWaveServerProcess(m_hProcess);
}


Audio::Core::AudioGraph::AudioGraph()
{
	m_bPhysicalMemoryBuffer = false;
	m_bActive = false;
	m_pData = 0;
	m_u32BufferSize = 0;
	m_u32BytesAvailable = 0;
	m_u32TotalBytesConsumed = 0;
	m_pNextActive = 0;
	m_pPrevActive = 0;
}


bool Audio::Core::AudioGraph::Create(WaveServerStreamCreateParams& params)
{
	m_u32BufferSize = params.m_u32MaxReturnBufferSize;
	m_bPhysicalMemoryBuffer = params.m_bReturnBufferPhysicalMemory;
	m_u32BufferAlignment = params.m_u32ReturnBufferAlignment;

	// we used to create the output buffer here, but now it gets created when the graph becomes active

	return true;
}


bool Audio::Core::AudioGraph::Destroy()
{
	for (U32 i = 0; i < (U32)m_StreamNodes.Size(); i++)
		m_StreamNodes[i].Destroy();

	DestroyOutputBuffer();

	return true;
}


bool Audio::Core::AudioGraph::CreateOutputBuffer()
{
	if (m_pData)
		return false;

	// create output buffer
		m_pData = (U8*)(BizAlignedMalloc(m_u32BufferSize, m_u32BufferAlignment));

	return true;
}


bool Audio::Core::AudioGraph::DestroyOutputBuffer()
{
	if (m_pData)
	{
			BizAlignedFree((void*)m_pData);

		m_pData = 0;

		return true;
	}

	return false;
}


U8* Audio::Core::AudioGraph::GetBuffer()
{
	if (! g_bCreateStreamNodes)
		return m_pData;

	Assert(m_StreamNodes.Size() > 0);

	CircularBuffer* pCircularBuffer = m_StreamNodes[0].GetBuffer();


	// query circular buffer, copy frame's worth of data to buffer in m_WaveStreamData if available & advance read pointer
	if (pCircularBuffer->IsActive())
	{
		Assert(m_pData);

		U32 u32BytesRetrieved = 0;
		U8* pData = m_pData + m_u32BytesAvailable;
		pCircularBuffer->GetData(&pData, m_u32BufferSize - m_u32BytesAvailable, &u32BytesRetrieved);

		m_u32BytesAvailable += u32BytesRetrieved;
	}

	return m_pData;
}


void Audio::Core::AudioGraph::ConsumeData(U32 u32BytesConsumed)
{
	Assert(u32BytesConsumed <= m_u32BytesAvailable);

	if (m_pData)
	{
		if ((u32BytesConsumed > 0) && (u32BytesConsumed < m_u32BufferSize) && g_bDoAudioGraphMemMoves)
		{
			memmove(m_pData, m_pData + u32BytesConsumed, m_u32BufferSize - u32BytesConsumed);
		}
	}
	
	m_u32BytesAvailable -= u32BytesConsumed;
	m_u32TotalBytesConsumed += u32BytesConsumed;
}


bool Audio::Core::AudioGraph::BecomeActive()
{
	if (m_bActive) return false;

	for (U32 i = 0; i < (U32)m_StreamNodes.Size(); i++)
		m_StreamNodes.At(i).BecomeActive();

	m_bActive = true;

	m_u32TotalBytesConsumed = 0;

	m_u32BytesAvailable = 0;
	
	CreateOutputBuffer();

	return true;
}


bool Audio::Core::AudioGraph::BecomeInactive()
{
	if (! g_bAudioGraphsAllowDeactivation)
		return true;

	for (U32 i = 0; i < (U32)m_StreamNodes.Size(); i++)
		m_StreamNodes.At(i).BecomeInactive();

	m_bActive = false;

	DestroyOutputBuffer();

	m_u32TotalBytesConsumed = 0;
	m_u32BytesAvailable = 0;

	return true;
}


void Audio::Core::AudioGraph::Update()
{
//	if ((! HasFinished()) && (GetBytesAvailable() < GetBufferSize()) && m_bActive)
	if ((! HasFinished()) && m_StreamNodes[0].GetBuffer()->HasSpace() && m_bActive)
	{
		for (S32 i = 0; i < m_StreamNodes.Size(); i++)
			m_StreamNodes[i].Update();
	}
}


//#ifdef PLATFORM_PS3
//Audio::Core::WaveServer::WaveServer() : m_LoadThread(64, this), m_MP3Manager(this), m_FileStreamManager(this)
//#else
Audio::Core::WaveServer::WaveServer() : m_LoadThread(64, this), m_FileStreamManager(this)
//#endif
{
	m_u32StreamsInUse = 0;
	m_u32SourcesInUse = 0;
	m_pFirstActive = 0;
	m_pLastActive = 0;

	m_LoadThread.Start(0);
}


bool Audio::Core::WaveServer::CreateStream(StreamHandle& hStream, WaveServerStreamCreateParams& params)
{
	if (! g_bCreateStreams)
		return false;

	if (hStream.IsValid())
		return false;

	AudioGraph* pNewGraph = new AudioGraph();

	if (! pNewGraph)
		return false;

	pNewGraph->Create(params);

	hStream = m_apGraph.AddEntry(pNewGraph);

	if (! hStream.IsValid())
	{
		delete pNewGraph;
		return false;
	}

	if (params.m_u32StreamEncodedDataFormat == Audio::Core::uMP3)
	{
//#ifdef PLATFORM_PS3
//		MP3Handle hMP3 = m_MP3Manager.CreateMP3Stream((U8*)(params.m_pMemoryResidentData), params.m_u32FileEndPosition, params.m_u8NumChannels, params.m_bLoop);
//
//		WaveServerHandle hWave = GetWaveServerHandle(hMP3.Value(), eMP3);
//
//		S32 s32StreamNodeListSize = pNewGraph->m_StreamNodes.Size();
//
//		pNewGraph->m_StreamNodes.Resize(s32StreamNodeListSize + 1);
//
//		pNewGraph->m_StreamNodes[s32StreamNodeListSize].Init();
//		pNewGraph->m_StreamNodes[s32StreamNodeListSize].m_hProcess = hWave;
//#else
		AUDIO_CORE_ERROR("Trying to set up an MP3");
		delete pNewGraph;
		return false;
//#endif
	}

	if (params.m_hSourceHandle.IsValid())
	{
		StreamSource* pSource = GetSource(params.m_hSourceHandle);

		S32 s32StreamNodeListSize = pNewGraph->m_StreamNodes.Size();

		FileStreamDef streamDef;

		streamDef.m_u32DataProcessMaxChunkSize = params.m_u32DataProcessMaxChunkSize;
		streamDef.m_u32FileReadChunkSize = params.m_u32FileReadChunkSize;
		streamDef.m_u32NumInitialWriteChunks = params.m_u32NumInitialWriteChunks;
		streamDef.m_u32BufferSizeInReadChunks = params.m_u32BufferSizeInReadChunks;
		streamDef.m_u32FileStartPosition = params.m_u32FileStartPosition;
		streamDef.m_u32FileEndPosition = params.m_u32FileEndPosition;
		streamDef.m_bLoop = params.m_bLoop;
		streamDef.m_bPrecache = params.m_bPrecache;
		streamDef.m_hFile = pSource->m_fsFileHandle;
		streamDef.m_u32HeaderSkipSize = params.m_u32HeaderSkipSize;
		streamDef.m_u32Format = params.m_u32StreamEncodedDataFormat;
		streamDef.m_bTempAmaxNullFileReadHack = params.m_bTempAmaxNullFileReadHack;

		#ifndef CFG_SPEED
		streamDef.m_strBankFilename = params.m_strBankFilename;
		streamDef.m_strWaveName = params.m_strWaveName;
		#endif

		FS_STREAM_HANDLE hFileStream = m_FileStreamManager.CreateStream(streamDef);

		WaveServerHandle hWave = GetWaveServerHandle(hFileStream.GetValue(), eFileStream);

		if (g_bCreateStreamNodes)
		{
			pNewGraph->m_StreamNodes.Resize(s32StreamNodeListSize + 1);

			pNewGraph->m_StreamNodes[s32StreamNodeListSize].Init();
			pNewGraph->m_StreamNodes[s32StreamNodeListSize].m_hProcess = hWave;
		}
	}

	return true;
}


bool Audio::Core::WaveServer::DestroyStream(StreamHandle hStream)
{
	if (! hStream.IsValid())
		return false;

	AudioGraph* pGraph = m_apGraph.GetByHandle(hStream);

	if (! pGraph)
		return false;

	if (pGraph->IsPrimed())
		BecomeInactive(pGraph);

	if (pGraph->Destroy())
	{
		m_u32StreamsInUse--;
		m_apGraph.Free(hStream);
		delete pGraph;
		return true;
	}

	return false;
}


bool Audio::Core::WaveServer::BecomeActive(AudioGraph* pGraph)
{
	bool bActive = pGraph->BecomeActive();

	if (bActive)
	{
		AddToActiveList(pGraph);
		return true;
	}

	return false;
}


bool Audio::Core::WaveServer::BecomeInactive(AudioGraph* pGraph)
{
	bool bInactive = pGraph->BecomeInactive();

	if (bInactive)
	{
		RemoveFromActiveList(pGraph);
		return true;
	}

	return false;
}


bool Audio::Core::WaveServer::RegisterStreamSource(StreamSourceHandle& hSource, WaveServerRegisterSourceParams& params)
{
	if (! g_bRegisterStreamSources)
		return false;

	StreamSource* pNewSource = new StreamSource();

	if (! pNewSource)
		return false;

	pNewSource->m_fsFileHandle = m_FileStreamManager.PrepareFile((const char*)params.m_pSourceData);

	if (! pNewSource->m_fsFileHandle.IsValid())
	{
		delete pNewSource;
		return false;
	}

	if (m_u32SourcesInUse == m_apSource.Size())
	{
		hSource = m_apSource.Size();
		m_apSource.PushBack(pNewSource);
		m_u32SourcesInUse++;

		return true;
	}
	else
	{
		for (U32 i = 0; i < (U32)m_apSource.Size(); i++)
		{
			if (! m_apSource.At(i))
			{
				m_apSource.At(i) = pNewSource;
				hSource = i;
				m_u32SourcesInUse++;

				return true;
			}
		}
	}

	delete pNewSource;
	return false;
}


bool Audio::Core::WaveServer::DeregisterStreamSource(StreamSourceHandle hSource)
{
	StreamSource* pSource = m_apSource.At(hSource);

	if (! pSource)
		return false;

	m_FileStreamManager.ReleaseFile(pSource->m_fsFileHandle);

	m_apSource.At(hSource) = 0;
	m_u32SourcesInUse--;
	delete pSource;

	return true;
}


void Audio::Core::WaveServer::Update()
{
	AUDIO_PERF_EVENT("Audio.WaveServer.Update");

	// tell any stream object that's had a load request processed if it's last load was successful
	WaveStreamReadResult readResult;

	while (m_LoadThread.GetResult(readResult))
	{
		if (readResult.m_bLastReadSuccessful)
		{
			switch( readResult.m_u8ResultType  )
			{
				case eMessageTypeFileStream:
				{
					m_FileStreamManager.ReadRequestProcessed(readResult.m_hStream, readResult.m_u32NumBytesRead, readResult.m_u32RequestId);
					break;				
				}
				case eMessageTypeDeallocateResource:
				{
					break;
				}
				default:
				{
					Assert(0 && "Unknown wave server message type!");
					break;
				}
			}
		}
		//else
		//{
		//	Assert(0 && "Last Load Failed!");
		//}
	}

	for (AudioGraph* pCurrActive = m_pFirstActive; pCurrActive; pCurrActive = pCurrActive->GetNextActive())
		pCurrActive->Update();

	m_FileStreamManager.Update();
}


bool Audio::Core::WaveServer::UpdateStream(WaveStreamData& waveStreamData, U32* pu32WavesNeededUpdate)
{
	AUDIO_PERF_EVENT("Audio.ProcessConnections.UpdateWaveServer");
	
	AudioGraph* pGraph = GetGraph(waveStreamData.hWaveData);

	if (! pGraph)
		return false;

	pGraph->ConsumeData(waveStreamData.u32BytesConsumed);
	waveStreamData.u32BytesConsumed = 0;

	if (waveStreamData.bPlaying && (! pGraph->IsPrimed()))
	{
		BecomeActive(pGraph);
	}
	else if ((! waveStreamData.bPlaying) && pGraph->IsPrimed())
		BecomeInactive(pGraph);

	waveStreamData.pData = pGraph->GetBuffer();
	waveStreamData.u32BytesAvailable = pGraph->GetBytesAvailable();

	waveStreamData.bFinished = pGraph->HasFinished();
	waveStreamData.u32SourceDataPosition = pGraph->GetTotalBytesConsumed();
	waveStreamData.bResourcesAllocated = pGraph->IsPrimed();
	waveStreamData.bRequiresUpdate = (waveStreamData.u32BytesAvailable == 0 && waveStreamData.bPlaying);	// set to update again next frame as we still need data
	waveStreamData.u32LocalOffset = 0;
	waveStreamData.u32BytesConsumed = 0;

	if (! waveStreamData.bRequiresUpdate)
		(*(pu32WavesNeededUpdate))--;

	return true;
}


bool Audio::Core::WaveServer::GetData(WaveServerHandle hWave, CircularBuffer* pDestCircularBuffer)
{
	U32 u32SystemHandle = hWave & 0x00ffffff;
	U32 u32SystemId = hWave >> 24;

	switch (u32SystemId)
	{
		case eFileStream:
		{
			return m_FileStreamManager.GetData(u32SystemHandle, pDestCircularBuffer);
		}
		break;

#ifdef PLATFORM_PS3
		//case eMP3:
		//{
		//	return m_MP3Manager.GetData(u32SystemHandle, pDestCircularBuffer);
		//}
		//break;
#endif

		default:
		{
			AUDIO_CORE_ERROR("Unhandled System Id in WaveServer");
			return false;
		}
	}

	return false;
}


Audio::Core::WaveServerHandle Audio::Core::WaveServer::GetWaveServerHandle(U32 u32SystemHandle, U32 u32SystemId)
{
	Assert(u32SystemId < 255);
	Assert(! (u32SystemHandle & 0xff000000));

	return WaveServerHandle((u32SystemId << 24) | u32SystemHandle);
}


void Audio::Core::WaveServer::AddToActiveList(AudioGraph* pGraph)
{
	if (! m_pFirstActive)
	{
		m_pFirstActive = pGraph;
		m_pLastActive = pGraph;
	}
	else
	{
		m_pLastActive->SetNextActive(pGraph);
		pGraph->SetPrevActive(m_pLastActive);
		m_pLastActive = pGraph;
	}
}


void Audio::Core::WaveServer::RemoveFromActiveList(AudioGraph* pGraph)
{
	if (m_pFirstActive == pGraph)
		m_pFirstActive = pGraph->GetNextActive();

	if (m_pLastActive == pGraph)
		m_pLastActive = pGraph->GetPrevActive();

	pGraph->RemoveFromActiveList();
}


bool Audio::Core::WaveServer::AllocateStreamProcessResources(WaveServerHandle hWave)
{
	if (! g_bAllocateStreamProcessResources)
		return true;

	U32 u32SystemHandle = hWave & 0x00ffffff;
	U32 u32SystemId = hWave >> 24;

	switch (u32SystemId)
	{
		case eFileStream:
		{
			if (m_FileStreamManager.AllocStreamBuffers(u32SystemHandle))
				return m_FileStreamManager.PrepareStream(u32SystemHandle);

			return false;
		}
		break;

//#ifdef PLATFORM_PS3
//		case eMP3:
//		{
//			return m_MP3Manager.AllocateStreamResources(u32SystemHandle);
//		}
//		break;
//#endif

		default:
		{
			AUDIO_CORE_ERROR("Unhandled System Id in WaveServer");
			return false;
		}
	}

	return false;
}


bool Audio::Core::WaveServer::DeallocateStreamProcessResources(WaveServerHandle hWave)
{
	U32 u32SystemHandle = hWave & 0x00ffffff;
	U32 u32SystemId = hWave >> 24;

	switch (u32SystemId)
	{
		case eFileStream:
		{
			return m_FileStreamManager.ReleaseStream(u32SystemHandle);
		}
		break;

//#ifdef PLATFORM_PS3
//		case eMP3:
//		{
//			return m_MP3Manager.DeallocateStreamResources(hWave);
//		}
//		break;
//#endif

		default:
		{
			AUDIO_CORE_ERROR("Unhandled System Id in WaveServer");
			return false;
		}
	}

	return false;
}


bool Audio::Core::WaveServer::DestroyWaveServerProcess(WaveServerHandle hWave)
{
	U32 u32SystemHandle = hWave & 0x00ffffff;
	U32 u32SystemId = hWave >> 24;

	switch (u32SystemId)
	{
		case eFileStream:
		{
			return m_FileStreamManager.DestroyStream(u32SystemHandle);
		}
		break;

#ifdef PLATFORM_PS3
		//case eMP3:
		//{
		//	return m_MP3Manager.DestroyMP3Stream(u32SystemHandle);
		//}
		//break;
#endif

		default:
		{
			AUDIO_CORE_ERROR("Unhandled System Id in DestroyWaveServerProcess");
			return false;
		}
	}

	return false;
}
