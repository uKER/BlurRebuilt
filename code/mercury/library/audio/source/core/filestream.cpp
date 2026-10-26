#include "AudioPch.h"
#include "coreheaders.h"
#include "../systemheaders.h"
#include "../Common/CircularBuffer.h"

#include <System/memory/MemoryTracker.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#ifdef PLATFORM_PS3
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif



Audio::Core::FileStreamFileWrapper::FileStreamFileWrapper()
{
	m_File = 0;
	m_u32HeaderSize = 0;
	m_bLoaded = false;
	m_bFlaggedForRelease = false;
	m_u32StreamRefCount = 0;
	m_strFilename[0] = 0;
}


Audio::Core::FileStreamFileWrapper::~FileStreamFileWrapper()
{
	if (m_File)
		delete m_File;
}


bool Audio::Core::FileStreamFileWrapper::Load(const char* strFilename)
{
	BIZ_ASSERT(strlen(strFilename) && (! strlen(m_strFilename)) && (! m_File));

	if (strlen(strFilename) > MAX_STREAMER_FILE_PATH_LENGTH - 1)
	{
		AUDIO_CORE_ERROR("Streaming wavebank filename too long");
		return false;
	}

	strncpy(m_strFilename, strFilename, strlen(strFilename) + 1);

	return true;
}


bool Audio::Core::FileStreamFileWrapper::Open()
{
	if (! strlen(m_strFilename))
		return false;

	Sys::C_OpenParams prms;
	prms.m_BufferSizeHint = 0;
	prms.m_AssertEof = false;

	m_File = Sys::GetDefaultFileSystem()->Open(m_strFilename, Sys::E_FileAccess_Read, prms);

	return (m_bLoaded = (m_File != 0));
}


bool Audio::Core::FileStreamFileWrapper::Close()
{
	if (m_File)
	{
		delete m_File;
		m_File = 0;
		return true;
	}
	else
	{
		return false;
	}
}


bool Audio::Core::FileStreamFileWrapper::RewindToStart()
{
	m_File->SetPosition(0);
	return true;
}


Sys::I_File* Audio::Core::FileStreamFileWrapper::GetFilePointer()
{
	return m_File;
}


bool Audio::Core::FileStreamFileWrapper::GetHeader(U32 u32HeaderSize, void* pDest)
{
	if (!m_bLoaded)
		return false;

	m_u32HeaderSize = u32HeaderSize;

	// TODO: the header read should be async
	if (!m_File->Read(pDest, u32HeaderSize))
		return false;

//	return m_File.Close();
	return true;
}


void* Audio::Core::FileStreamDataChunk::GetData(U32 u32Offset, U32& u32BytesAvailable)
{
	if (u32Offset > m_u32BytesAvailable)
	{
		AUDIO_CORE_ERROR("Audio stream data chunk: requesting more bytes than available");
		return 0;
	}

	u32BytesAvailable = m_u32BytesAvailable - u32Offset;
	m_u32PrevOffset = u32Offset;

	return (void*)((U8*)m_pDataChunk + u32Offset);
}


bool Audio::Core::FileStreamDataChunk::SetBytesConsumed(U32 u32BytesConsumed)
{
	if (m_u32PrevOffset + u32BytesConsumed > m_u32BytesAvailable)
	{
		AUDIO_CORE_ERROR("Audio stream data chunk: setting bytes consumed to value greater than that available");
		return false;
	}

//	m_u32BytesAvailable -= u32BytesConsumed;

	if (m_u32PrevOffset + u32BytesConsumed == m_u32BytesAvailable)
	{
		m_u32BytesAvailable = 0;
		m_u32PrevOffset = 0;
		m_bAllDataConsumed = true;
	}

	return true;
}


bool Audio::Core::FileStreamDataChunk::SetBytesAvailable(U32 u32BytesAvailable)
{
	if (m_u32BytesAvailable > 0)
	{
		AUDIO_CORE_ERROR("Audio stream data chunk: setting bytes available when chunk already contains data");
		return false;
	}

	m_u32BytesAvailable = u32BytesAvailable;

	if (u32BytesAvailable)
		m_bAllDataConsumed = false;

	return true;
}


Audio::Core::FileStream::FileStream()
{
	m_FileStreamDef.m_u32DataProcessMaxChunkSize = 0;
	m_FileStreamDef.m_u32FileReadChunkSize = 0;
	m_FileStreamDef.m_u32NumInitialWriteChunks = 0;
	m_FileStreamDef.m_u32BufferSizeInReadChunks = 0;
	m_FileStreamDef.m_bLoop = false;
	m_FileStreamDef.m_hFile = -1;
	m_FileStreamDef.m_bReturnBufferPhysicalMemory = false;
	m_FileStreamDef.m_u32HeaderSkipSize = 0;
	m_FileStreamDef.m_bPrecache = false;
	m_FileStreamDef.m_bTempAmaxNullFileReadHack = false;

	m_u32LastIssuedRequestId = 0;
#ifdef ENABLE_DEBUG_EXTENSIONS
	memset(m_FileStreamDef.m_pchBankFilename, 0, lengthof(m_FileStreamDef.m_pchBankFilename));
	memset(m_FileStreamDef.m_pchWaveName, 0, lengthof(m_FileStreamDef.m_pchWaveName));
#endif

	m_pManager = 0;

//	m_u32StateFlags = 0;	-done in InitState()

	m_hStream = -1;

	m_bLoadRequestSubmitted = false;

	m_bEndianSwap = false;

	InitState();

//	SetStateFlagToTrue(STREAM_DATA_NOT_READY);	-done in InitState()

	SetStateFlagToTrue(STREAM_DATA_NOT_READY);
}


Audio::Core::FileStream::~FileStream()
{
	FreeBuffers();
}


void Audio::Core::FileStream::InitState()
{
	m_FileStreamState.m_u32CurrReadChunk = 0;
	m_FileStreamState.m_u32NextWriteChunk = 0;
	m_FileStreamState.m_u32HasBeenWrittenChunk = 0;
	m_FileStreamState.m_u32CurrentChunkReadPointer = 0;

	m_u64TotalBytesWritten = 0;
	m_u64TotalBytesRead = 0;
	m_u32PrevSeekPos = 0;

	FreeBuffers();

	for (U32 i = 0; i < MAX_NUM_DATA_CHUNKS; i++)
		m_aDataChunk[i].Init();

	m_u32PrevRequestedNumBytes = 0;

	m_u32StateFlags = STREAM_DATA_NOT_READY;
}


bool Audio::Core::FileStream::Setup(FileStreamDef& streamDef)
{
	m_FileStreamDef.m_u32DataProcessMaxChunkSize = streamDef.m_u32DataProcessMaxChunkSize;
	m_FileStreamDef.m_u32FileReadChunkSize = streamDef.m_u32FileReadChunkSize;
	m_FileStreamDef.m_u32NumInitialWriteChunks = 1;//streamDef.m_u32NumInitialWriteChunks;
	m_FileStreamDef.m_u32BufferSizeInReadChunks = streamDef.m_u32BufferSizeInReadChunks;
	m_FileStreamDef.m_u32FileStartPosition = streamDef.m_u32FileStartPosition;
	m_FileStreamDef.m_u32FileEndPosition = streamDef.m_u32FileEndPosition;
	m_FileStreamDef.m_hFile = streamDef.m_hFile;
	m_FileStreamDef.m_bLoop = streamDef.m_bLoop;
	m_FileStreamDef.m_bReturnBufferPhysicalMemory = streamDef.m_bReturnBufferPhysicalMemory;
	m_FileStreamDef.m_u32Format = streamDef.m_u32Format;
	m_FileStreamDef.m_bTempAmaxNullFileReadHack = streamDef.m_bTempAmaxNullFileReadHack;

	Assert(m_FileStreamDef.m_u32BufferSizeInReadChunks <= 4);

	#ifndef CFG_SPEED
	m_FileStreamDef.m_strBankFilename = streamDef.m_strBankFilename;
	m_FileStreamDef.m_strWaveName = streamDef.m_strWaveName;
	#endif

	Assert (streamDef.m_u32BufferSizeInReadChunks >= streamDef.m_u32NumInitialWriteChunks);

	return true;
}


bool Audio::Core::FileStream::AllocBuffers()
{
	BIZ_ASSERT(!(m_u32StateFlags & STREAM_RELEASING));
	BIZ_ASSERT(!(m_u32StateFlags & STREAM_DESTROYING));

	System::Get()->PushCurrentMemoryAllocationId(6);
	MEMORY_TRACKER_PUSH("Audio.StreamerBuffers");
	
	for (U32 i = 0; i < m_FileStreamDef.m_u32BufferSizeInReadChunks; i++)
	{
		if (m_aDataChunk[i].IsBufferAllocated())
		{
			FreeBuffers();
			break;
		}
	}
		
	for (U32 i = 0; i < m_FileStreamDef.m_u32BufferSizeInReadChunks; i++)
	{
		m_aDataChunk[i].Init();

		if (i < m_FileStreamDef.m_u32NumInitialWriteChunks)
		{
			void* pMem = new U8[m_FileStreamDef.m_u32FileReadChunkSize]; // m_pManager->GetBufferAllocator()->Alloc(m_FileStreamDef.m_u32FileReadChunkSize, 16);

			if (! pMem)
			{
				BIZ_ASSERT_FAIL("Couldn't allocate audio stream buffer");
				System::Get()->PopCurrentMemoryAllocationId();
				MEMORY_TRACKER_POP("Audio.StreamerBuffers");
				return false;
			}

			m_aDataChunk[i].SetDataChunk(pMem);
		}
	}

	m_pManager->Info("Allocated buffers for stream %d", m_hStream.u32Handle);

	SetStateFlagToTrue(STREAM_BUFFERS_ALLOCATED);

	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.StreamerBuffers");

	return true;
}


void Audio::Core::FileStream::FreeBuffers()
{
	System::Get()->PushCurrentMemoryAllocationId(6);
	MEMORY_TRACKER_PUSH("Audio.StreamerBuffers");

	if (m_pManager)
		m_pManager->ExtendedInfo("Freeing buffers for stream %d", m_hStream.u32Handle);

	for (U32 i = 0; i < m_FileStreamDef.m_u32BufferSizeInReadChunks; i++)
	{
		if (m_aDataChunk[i].IsBufferAllocated())
		{
			ReleaseBuffer(m_aDataChunk[i].GetBufferAndRelease());
		}
		m_aDataChunk[i].Init();
	}

	// invalidate any pending loads..
	m_u32LastIssuedRequestId++;

	SetStateFlagToFalse(STREAM_BUFFERS_ALLOCATED);
	SetStateFlagToTrue(STREAM_DATA_NOT_READY);

	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.StreamerBuffers");
}

// asks for some data
bool Audio::Core::FileStream::Prepare()
{
	BIZ_ASSERT(m_pManager && m_hStream.IsValid());
	BIZ_ASSERT(!(m_u32StateFlags & STREAM_RELEASING));
	BIZ_ASSERT(!(m_u32StateFlags & STREAM_DESTROYING));

	if (! m_pManager)
		return false;


	SubmitLoadRequest(m_FileStreamDef.m_u32NumInitialWriteChunks);

	SetStateFlagToFalse(STREAM_RELEASED);
	SetStateFlagToFalse(STREAM_FINISHED_READING);
	SetStateFlagToFalse(STREAM_FINISHED_WRITING);
	SetStateFlagToFalse(STREAM_EOF_READ);

	m_pManager->Info("Prepare stream %d", m_hStream.u32Handle);

	return true;
}

// flags this stream as ready to be released
bool Audio::Core::FileStream::FlagForRelease()
{
	SetStateFlagToTrue(STREAM_RELEASING);

	m_bLoadRequestSubmitted = false;

	Release();
	
	m_pManager->Info("Flag for release stream %d", m_hStream.u32Handle);

	return true;
}


bool Audio::Core::FileStream::Reset()
{
	InitState();
	Prepare();

	return true;
}

// flags this stream as ready to be destroyed
bool Audio::Core::FileStream::FlagForDestruction()
{
	Assert (m_pManager && m_hStream.IsValid());

	if (! m_pManager)
		return false;

	SetStateFlagToTrue(STREAM_DESTROYING);
	
	m_pManager->Info("Flag for destruction stream %d", m_hStream.u32Handle);

	return true;
}


bool Audio::Core::FileStream::FlagForReset()
{
	Assert (m_pManager && m_hStream.IsValid());

	if (! m_pManager)
		return false;

	SetStateFlagToTrue(STREAM_RESET_REQUIRED);

	return true;
}

// marks the stream as unprepared and frees buffers
bool Audio::Core::FileStream::Release()
{
	m_pManager->ExtendedInfo("Releasing stream %d", m_hStream.u32Handle);

//	m_FileStreamState.m_u32DataProcessPosition = 0;
	m_FileStreamState.m_u32CurrReadChunk = 0;
	m_FileStreamState.m_u32NextWriteChunk = 0;
	m_FileStreamState.m_u32HasBeenWrittenChunk = 0;
	m_FileStreamState.m_u32CurrentChunkReadPointer = 0;

	m_u64TotalBytesWritten = 0;
	m_u64TotalBytesRead = 0;

	m_u32PrevSeekPos = 0;

	m_u32PrevRequestedNumBytes = 0;

	m_bLoadRequestSubmitted = false;

	FreeBuffers();

	SetStateFlagToFalse(STREAM_RELEASING);
	SetStateFlagToTrue(STREAM_RELEASED);
	SetStateFlagToTrue(STREAM_DATA_NOT_READY);
	SetStateFlagToFalse(STREAM_ACTIVE);
	// new.. jkc
	SetStateFlagToFalse(STREAM_EOF_READ);
	SetStateFlagToFalse(STREAM_FINISHED_READING);
	SetStateFlagToFalse(STREAM_FINISHED_WRITING);

	m_pManager->Info("Released stream %d", m_hStream.u32Handle);

	return true;
}


// will return a maximum of m_u32DataProcessMaxChunkSize bytes.
bool Audio::Core::FileStream::GetData(CircularBuffer* pCircularBuffer)
{
	if (! QueryStateFlag(STREAM_BUFFERS_ALLOCATED))
	{
		AUDIO_CORE_FATAL("Audio::Core::FileStream::GetData() : Buffers are not allocated");
		return false;
	}

	bool bNotReady = false;

	if (QueryStateFlag(STREAM_RELEASING) || QueryStateFlag(STREAM_DESTROYING))
		bNotReady = true;

	if (bNotReady)
	{
		SetStateFlagToTrue(STREAM_DATA_NOT_READY);
		pCircularBuffer->SetStarving(true);

		return true;
	}

	SetStateFlagToFalse(STREAM_DATA_NOT_READY);

	FileStreamDataChunk* pLastAccessedReadChunk = 0;

	// if no data available yet, bail out. (we certainly don't want to attempt to read any more data yet, will cause badness)
	if (! IsDataAvailableInCurrentReadChunk())
		return true;

	while (pCircularBuffer->HasSpace() && IsDataAvailableInCurrentReadChunk())
	{
		U32 u32SourceBytesAvailable = 0;
		U32 u32DestBytesWritten = 0;
		void* pData = GetCurrentReadChunkData(u32SourceBytesAvailable);
		pCircularBuffer->WriteData((U8*)pData, u32SourceBytesAvailable, &u32DestBytesWritten);


		pLastAccessedReadChunk = GetCurrentReadChunk();

		SetCurrentReadChunkBytesConsumed(u32DestBytesWritten);
	}

	if (pLastAccessedReadChunk && pLastAccessedReadChunk->IsEndOfStreamReached() && (! pLastAccessedReadChunk->IsDataAvailable()) && (! m_FileStreamDef.m_bLoop) && (! QueryStateFlag(STREAM_FINISHED_READING)))
	{
 		SetStateFlagToTrue(STREAM_FINISHED_READING);
		pCircularBuffer->SetEndOfStreamReached();
	}
	// check that write chunk is not going to overwrite current read chunk if we attempt to read any more data off disk
	else if (! (GetNextWriteChunkIndex() == GetCurrentReadChunkIndex() && m_FileStreamState.m_u32CurrReadChunk < m_FileStreamState.m_u32NextWriteChunk))
	{
		// submit another read if necessary
		if ((! m_bLoadRequestSubmitted) && (! QueryStateFlag(STREAM_FINISHED_WRITING)) && (! QueryStateFlag(STREAM_EOF_READ)))
			SubmitLoadRequest(1);
		//TODO: change to this?
//			SubmitLoadRequest(m_FileStreamState.m_u32NextWriteChunk - m_FileStreamState.m_u32CurrReadChunk);
	}

	return true;
}


U32 Audio::Core::FileStream::GetPosition()
{
	return (U32)m_u64TotalBytesWritten;
}


bool Audio::Core::FileStream::SetFilePosition(U32 u32Position)
{
	m_u64TotalBytesWritten = u32Position;
	m_u32PrevSeekPos = u32Position;
	return true;
}


/*
bool Audio::Core::FileStream::IsFinished()
{
	return (m_eStreamState == eStreamFinished);
}
*/

// Function doesn't support reading multiple chunks at once anymore
bool Audio::Core::FileStream::SubmitLoadRequest(U32 u32NumChunksToRead, U32 u32HeaderSkip)
{
	if (QueryStateFlag(STREAM_RELEASING) || QueryStateFlag(STREAM_DESTROYING))
		return false;

	void* pDestBuffer = GetNextWriteChunk().GetBufferForWriting();

	if (!pDestBuffer)
		return false;

	U32 u32BytesIdeallyNeeded = m_FileStreamDef.m_u32FileReadChunkSize;

	U32 u32BytesToRead = Maths::Min(u32BytesIdeallyNeeded, GetStreamTotalDataSize() - GetBytesWrittenPosInStream());

	WaveStreamReadRequest readReq;

	readReq.m_bLoop = m_FileStreamDef.m_bLoop;
	readReq.m_hStream = S32(m_hStream.GetValue());
	readReq.m_pDestBuffer = pDestBuffer;
	readReq.m_pFileWrapper = m_pManager->GetFileWrapper(m_FileStreamDef.m_hFile);
	readReq.m_u32BytesToRead = u32BytesToRead;
	readReq.m_u32SeekPosition = m_FileStreamDef.m_u32FileStartPosition + GetBytesWrittenPosInStream() + u32HeaderSkip;
	readReq.m_bTempAmaxNullFileReadHack = m_FileStreamDef.m_bTempAmaxNullFileReadHack;
	readReq.m_u8ReadRequestType = eFileStream;
	readReq.m_bDeallocateResources = false;
	readReq.m_u32RequestId = ++m_u32LastIssuedRequestId;
	m_u32PrevRequestedNumBytes = readReq.m_u32BytesToRead;

	m_pManager->ExtendedInfo("sending another load request, seek pos is %d, bytes to read is %d.", readReq.m_u32SeekPosition, readReq.m_u32BytesToRead);
	m_pManager->LoadData(readReq);

	m_FileStreamState.m_u32NextWriteChunk++;

	m_bLoadRequestSubmitted = true;

	return true;
}


void Audio::Core::FileStream::ReleaseBuffer(void* pBuffer)
{
	WaveStreamReadRequest readReq;

	readReq.m_hStream = S32(m_hStream.GetValue());
	readReq.m_pDestBuffer = pBuffer;
	readReq.m_u8ReadRequestType = eFileStream;
	readReq.m_bDeallocateResources = true;

	m_pManager->LoadData(readReq);
}


void Audio::Core::FileStream::ReadRequestProcessed(U32 u32BytesWritten, U32 u32RequestId)
{
	m_pManager->ExtendedInfo("in ReadRequestProcessed");

	// dont carry on asking for for more data if this stream is shutting down
	if (QueryStateFlag(STREAM_RELEASING) || QueryStateFlag(STREAM_DESTROYING))
	{
		m_bLoadRequestSubmitted = false;
		return;
	}

	if (u32RequestId != m_u32LastIssuedRequestId)
	{
		AUDIO_CORE_WARN("Dropping load result, request ID does not match");

		m_bLoadRequestSubmitted = false;
		return;
	}

	U32 u32BytesInChunk = 0;
	bool bReachedEnd = false;
	for (U32 u32BytesLeftToProcess = u32BytesWritten; u32BytesLeftToProcess && (! bReachedEnd); u32BytesLeftToProcess -= u32BytesInChunk)
	{
		u32BytesInChunk = Maths::Min(m_FileStreamDef.m_u32FileReadChunkSize, u32BytesLeftToProcess);

		if (! GetHasBeenWrittenChunk().IsBufferAllocated())
		{
			AUDIO_CORE_WARN("Bailing from process read request, buffers not available");
			m_bLoadRequestSubmitted = false;
			return;
		}

		GetHasBeenWrittenChunk().SetBytesAvailable(u32BytesInChunk);
		m_u64TotalBytesWritten += u32BytesInChunk;

		if (m_u64TotalBytesWritten == GetStreamTotalDataSize() && (! m_FileStreamDef.m_bLoop))
			GetHasBeenWrittenChunk().SetEndOfStreamReached();

		if ((U32)(m_u64TotalBytesWritten % (U64)(m_FileStreamDef.m_u32FileEndPosition - m_FileStreamDef.m_u32FileStartPosition)) == 0)	// we've reached the end of the stream
		{
			if (! m_FileStreamDef.m_bLoop)
			{
				SetStateFlagToTrue(STREAM_FINISHED_WRITING);
				SetCurrentWriteChunkContainsStreamEnd();
				m_bLoadRequestSubmitted = false;
				bReachedEnd = true;
			}
			//else
			//	u32HeaderSkip = m_FileStreamDef.m_u32HeaderSkipSize;

			// ..else file should automatically loop
		}

		m_FileStreamState.m_u32HasBeenWrittenChunk++;

		Assert((S32)m_FileStreamState.m_u32HasBeenWrittenChunk - (S32)m_FileStreamDef.m_u32BufferSizeInReadChunks <= (S32)m_FileStreamState.m_u32CurrReadChunk);
	}

	SetStateFlagToTrue(STREAM_ACTIVE);

	SetStateFlagToFalse(STREAM_DATA_NOT_READY);

	if (bReachedEnd)
		return;

//	U32 u32HeaderSkip = 0;

	SetStateFlagToFalse(STREAM_DATA_NOT_READY);

	if (u32BytesWritten < m_u32PrevRequestedNumBytes)	// something's gone wrong.. could mean the stream size specified took the read over the end of the file
	{
//		SetStateFlagToTrue(STREAM_ERROR);
		AUDIO_SYS_ERROR("Audio Stream error");
	}

//	if (bAttemptToReadMore)
	{
		// check if we can read in any more
		m_pManager->ExtendedInfo("attempting to read more...");

		m_pManager->ExtendedInfo("m_FileStreamState.m_u32NextWriteChunk = %d, m_FileStreamDef.m_u32BufferSizeInReadChunks = %d, m_FileStreamState.m_u32CurrReadChunk = %d", m_FileStreamState.m_u32NextWriteChunk, m_FileStreamDef.m_u32BufferSizeInReadChunks, m_FileStreamState.m_u32CurrReadChunk);

		if ((m_FileStreamState.m_u32NextWriteChunk + 1) % m_FileStreamDef.m_u32BufferSizeInReadChunks != m_FileStreamState.m_u32CurrReadChunk)
		{
			if (! DoesCurrentReadChunkContainStreamEnd())
				m_bLoadRequestSubmitted = SubmitLoadRequest(1/*, u32HeaderSkip*/);
			else if (! QueryStateFlag(STREAM_FINISHED_READING))
			{
				SetStateFlagToTrue(STREAM_FINISHED_READING);
				m_bLoadRequestSubmitted = false;
			}
		}
		else
		{
			m_pManager->ExtendedInfo("it appears we can't.");
			m_bLoadRequestSubmitted = false;
		}
	}
}


void* Audio::Core::FileStream::GetCurrentReadChunkData(U32& u32BytesAvailable)
{
	FileStreamDataChunk* pReadChunk = GetCurrentReadChunk();

	if (pReadChunk)
		return pReadChunk->GetData(m_FileStreamState.m_u32CurrentChunkReadPointer, u32BytesAvailable);

	return 0;
}


bool Audio::Core::FileStream::SetCurrentReadChunkBytesConsumed(U32 u32BytesConsumed)
{
	FileStreamDataChunk* pReadChunk = GetCurrentReadChunk();

	if (pReadChunk)
	{
		m_FileStreamState.m_u32CurrentChunkReadPointer += u32BytesConsumed;
		return pReadChunk->SetBytesConsumed(u32BytesConsumed);
	}

	AUDIO_CORE_ERROR("Couldn't set current audio stream read chunk bytes consumed");
	return false;
}


bool Audio::Core::FileStream::IsDataAvailableInCurrentReadChunk()
{
	FileStreamDataChunk* pReadChunk = GetCurrentReadChunk();

	if (! pReadChunk)
		return false;

	return pReadChunk->IsDataAvailable();
}


bool Audio::Core::FileStream::DoesCurrentReadChunkContainStreamEnd()
{
	FileStreamDataChunk* pChunk = GetCurrentReadChunk();

	if (pChunk)
		return pChunk->IsEndOfStreamReached();

	return false;
}


Audio::Core::FileStreamDataChunk* Audio::Core::FileStream::GetCurrentReadChunk()
{
	if (m_aDataChunk[GetCurrentReadChunkIndex()].IsDataAvailable())
		return &(m_aDataChunk[GetCurrentReadChunkIndex()]);

	if (m_FileStreamState.m_u32CurrReadChunk < m_FileStreamState.m_u32HasBeenWrittenChunk)
		m_FileStreamState.m_u32CurrReadChunk++;
	else
		return 0;

	m_FileStreamState.m_u32CurrentChunkReadPointer = 0;

	return &(m_aDataChunk[GetCurrentReadChunkIndex()]);
}


Audio::Core::FileStreamDataChunk& Audio::Core::FileStream::GetNextWriteChunk()
{
	FileStreamDataChunk& chunk = m_aDataChunk[GetNextWriteChunkIndex()];

	if (! chunk.IsBufferAllocated())
	{
//		void* pMem = m_pManager->GetBufferAllocator()->Alloc(m_FileStreamDef.m_u32FileReadChunkSize, 16);
		void* pMem = new U8[m_FileStreamDef.m_u32FileReadChunkSize]; // m_pManager->GetBufferAllocator()->Alloc(m_FileStreamDef.m_u32FileReadChunkSize, 16);
		chunk.SetDataChunk(pMem);
	}

	return chunk;
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
