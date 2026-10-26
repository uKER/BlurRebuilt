#include "AudioPch.h"
#include "coreheaders.h"
#include "../systemheaders.h"

#include <System/memory/MemoryTracker.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool g_bFileStreamServiceReadRequests("Audio.System.Streamer.FileStreamerServiceReadRequests", true);
Debug::Bool g_bFileStreamerDumpStreamInfo("Audio.System.Streamer.DumpFileStreamerInfo", false);
Debug::Bool g_bFileStreamerRenderStreams("Audio.System.Streamer.RenderStreamInfo", false);
Debug::Bool g_bFileStreamerRenderOnlyActiveStreams("Audio.System.Streamer.OnlyRenderPlayingStreams", true);
Debug::Float g_bFileStreamerRenderYOffset("Audio.System.Streamer.YOffset", 200.f, Debug::Range(-2000.f, 2000.f));
MT::Event	AudioStreamingEvent;


#ifdef USER_JONC
#define JONC_STREAM_DEBUG_INFO
#endif


#ifdef ENABLE_DEBUG_EXTENSIONS
void Audio::Core::FileStreamManager::DumpStreamInfo() 
{
	AUDIO_CORE_DEBUG("=========================== Dump filestreaminfo ==================");

	for (U32 i = 0; i < MAX_NUM_AUDIO_STREAMS; i++)
	{
		if (m_apStream[i])
		{
			AUDIO_CORE_DEBUG("%d hnd = %d, '%s', '%s', state flags 0x%x [%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s]\n", i, m_apStream[i]->m_hStream.u32Handle, m_apStream[i]->m_FileStreamDef.m_pchBankFilename, m_apStream[i]->m_FileStreamDef.m_pchWaveName, m_apStream[i]->m_u16StateFlags, 
				m_apStream[i]->m_u16StateFlags&STREAM_ACTIVE?"ACTIVE":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_PREPARING?"PREPARING":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_EOF_READ?"EOF_READ":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_FINISHED?"FINISHED":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_UNDERRUN?"UNDERRUN":"", 
//				m_apStream[i]->m_u16StateFlags&STREAM_ERROR?"ERROR":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_RELEASING?"RELEASING":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_RELEASED?"RELEASED":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_DESTROYING?"DESTROYING":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_BUFFERS_ALLOCATED?"BUFFERS_ALLOCATED":"", 
				m_apStream[i]->m_u16StateFlags&STREAM_DATA_NOT_READY?"DATA_NOT_READY":"");
		}
	}
	AUDIO_CORE_DEBUG("=========================== Dump complete ==================");
}
#endif

Audio::Core::FileStreamManager::FileStreamManager(IWaveServer* iWaveServer) : WaveServerComponent(iWaveServer)
{
	for (U32 i = 0; i < MAX_NUM_STREAMING_FILES; i++)
		m_apFile[i] = 0;

	m_eDebugLevel = eWarning;
}


Audio::Core::FileStreamManager::~FileStreamManager()
{
}


void* Audio::Core::FileStreamManager::operator new(size_t sz)
{
	void* pRet = Audio::System::Alloc(sz);
	return pRet;
}

void Audio::Core::FileStreamManager::operator delete(void* ptr)
{
	Audio::System::Free(ptr);
}

// opens the file
AudioStreamFileHandle Audio::Core::FileStreamManager::PrepareFile(const char* strFilename)
{
	AudioStreamFileHandle hRet;

	Audio::System::Get()->PushCurrentMemoryAllocationId(6);
	MEMORY_TRACKER_PUSH("Audio.Streamer");

	for (U32 i = 0; i < MAX_NUM_STREAMING_FILES; i++)
	{
		if (! m_apFile[i])
		{
			m_apFile[i] = new FileStreamFileWrapper();

			if (m_apFile[i])
			{
				if (m_apFile[i]->Load(strFilename))
				{
					System::Get()->PopCurrentMemoryAllocationId();

					MEMORY_TRACKER_POP("Audio.Streamer");
					hRet = i;
					return hRet;
				}
				else
				{
					delete m_apFile[i];
					m_apFile[i] = 0;
					System::Get()->PopCurrentMemoryAllocationId();

					MEMORY_TRACKER_POP("Audio.Streamer");
					return hRet;
				}
			}
		}
	}

	Assert(0 && "Too many streaming files\n");

	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.Streamer");

	return hRet;
}


U32 g_u32NumStreams = 0;

FS_STREAM_HANDLE Audio::Core::FileStreamManager::CreateStream(FileStreamDef& streamDef)
{
	FileStream* pNewStream = new FileStream();

	if (! pNewStream)
		return PagedArrayHandle();	// return invalid handle

	FS_STREAM_HANDLE hStream = m_apStream.AddEntry(pNewStream);

	if (! hStream.IsValid())
	{
		delete pNewStream;
		return hStream;	// return invalid handle
	}

	Assert (! (hStream.GetValue() & 0xff000000));

	pNewStream->SetManager(this);

	if (pNewStream->Setup(streamDef))
	{
		pNewStream->SetStreamHandle(hStream);
		System::Get()->PopCurrentMemoryAllocationId();

		m_apFile[streamDef.m_hFile]->IncRefCount();

		g_u32NumStreams++;

		return hStream;
	}
	else
	{
		delete pNewStream;
		m_apStream.Free(hStream);
	}

	System::Get()->PopCurrentMemoryAllocationId();

	return -1;
}


// flags the stream for destruction
bool Audio::Core::FileStreamManager::DestroyStream(FS_STREAM_HANDLE hStream)
{
	m_lockObject.Acquire();

	if (GetStream(hStream))
	{
		bool bFlaggedForDestruction = GetStream(hStream)->FlagForDestruction();

		if (bFlaggedForDestruction)
			m_vStreamsToUpdate.PushBack(hStream);

		m_lockObject.Release();

		return bFlaggedForDestruction;
	}

	m_lockObject.Release();

	return false;
}

// deletes the stream object
bool Audio::Core::FileStreamManager::DoStreamDestroy(FS_STREAM_HANDLE hStream)
{
	m_lockObject.Acquire();

	if (GetStream(hStream))
	{
		System::Get()->PushCurrentMemoryAllocationId(6);
		MEMORY_TRACKER_PUSH("Audio.Streamer");

		Info("Deleting stream %d", hStream.u32Handle);

		m_apFile[GetStream(hStream)->GetFileHandle()]->DecRefCount();

		delete GetStream(hStream);
		m_apStream.Free(hStream);

		System::Get()->PopCurrentMemoryAllocationId();
		MEMORY_TRACKER_POP("Audio.Streamer");

		g_u32NumStreams--;
	}

	m_lockObject.Release();

	return false;
}

bool Audio::Core::FileStreamManager::GetHeader(AudioStreamFileHandle hFile, U32 u32HeaderSize, void* pDest)
{
	if (m_apFile[hFile])
	{
		m_apFile[hFile]->RewindToStart();
		return m_apFile[hFile]->GetHeader(u32HeaderSize, pDest);
	}

	return false;
}


bool Audio::Core::FileStreamManager::AllocStreamBuffers(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->AllocBuffers();
	}

	return false;
}


void Audio::Core::FileStreamManager::FreeStreamBuffers(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		GetStream(hStream)->FreeBuffers();
	}
}

bool Audio::Core::FileStreamManager::PrepareStream(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->Prepare();
	}

	return false;
}

bool Audio::Core::FileStreamManager::ReleaseStream(FS_STREAM_HANDLE hStream)
{
	m_lockObject.Acquire();

	if (GetStream(hStream))
	{
		bool bFlaggedForRelease = GetStream(hStream)->FlagForRelease();

		if (bFlaggedForRelease)
			m_vStreamsToUpdate.PushBack(hStream);

		m_lockObject.Release();

		return bFlaggedForRelease;
	}

	m_lockObject.Release();

	return false;
}

bool Audio::Core::FileStreamManager::ResetStream(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		if (IsStreamAwaitingDestruction(hStream))
			return false;

		if (GetStream(hStream)->LoadRequestSubmitted())
		{
			bool bFlaggedForReset = GetStream(hStream)->FlagForReset();

			if (bFlaggedForReset)
			{
				m_lockObject.Acquire();
				m_vStreamsToUpdate.PushBack(hStream);
				m_lockObject.Release();
			}

			return bFlaggedForReset;
		}

		return GetStream(hStream)->Reset();
	}

	return false;
}

bool Audio::Core::FileStreamManager::IsStreamFinishedReading(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_FINISHED_READING);
	}

	return false;
}


bool Audio::Core::FileStreamManager::IsStreamFinishedWriting(FS_STREAM_HANDLE hStream)
{
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_FINISHED_WRITING);
	}

	return false;
}


bool Audio::Core::FileStreamManager::IsStreamPrepared(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_ACTIVE);
	}

	return false;
}

bool Audio::Core::FileStreamManager::IsStreamPrecached(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->ShouldStreamPrecache();
	}

	return false;
}


bool Audio::Core::FileStreamManager::IsStreamReleasing(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_RELEASING);
	}

	return false;
}

bool Audio::Core::FileStreamManager::IsStreamResetting(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_RESET_REQUIRED);
	}

	return false;
}

bool Audio::Core::FileStreamManager::IsStreamAwaitingDestruction(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_DESTROYING);
	}

	return false;
}

bool Audio::Core::FileStreamManager::AreStreamBuffersAllocated(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->QueryStateFlag(STREAM_BUFFERS_ALLOCATED);
	}

	return false;
}


bool Audio::Core::FileStreamManager::IsStreamDataReady(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return (! GetStream(hStream)->QueryStateFlag(STREAM_DATA_NOT_READY));
	}

	return false;
}


bool Audio::Core::FileStreamManager::GetData(WaveServerHandle hStream, CircularBuffer* pCircularBuffer)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->GetData(pCircularBuffer);
	}

	return 0;
}


bool Audio::Core::FileStreamManager::SetFilePosition(FS_STREAM_HANDLE hStream, U32 u32PositionInBytes)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->SetFilePosition(u32PositionInBytes);
	}

	return false;
}


U32 Audio::Core::FileStreamManager::GetFilePosition(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->GetPosition();
	}

	return 0;
}


U32 Audio::Core::FileStreamManager::GetPrevSeekPos(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->GetPrevSeekPos();
	}

	return 0;
}

S32 Audio::Core::FileStreamManager::GetNumBytesDataReady(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
	{
		return GetStream(hStream)->GetNumBytesDataReady();
	}

	return 0;
}


bool Audio::Core::FileStreamManager::ReleaseFile(AudioStreamFileHandle hFile)
{
	m_apFile[hFile]->FlagForRelease();
	return true;
}


bool Audio::Core::FileStreamManager::DoFileRelease(AudioStreamFileHandle hFile)
{
	Assert(hFile.IsValid() && hFile < MAX_NUM_STREAMING_FILES);

	if (! (hFile.IsValid() && hFile < MAX_NUM_STREAMING_FILES))
		return false;

	System::Get()->PushCurrentMemoryAllocationId(6);
	MEMORY_TRACKER_PUSH("Audio.Streamer");

	if (m_apFile[hFile])
	{
		if (m_apFile[hFile]->m_File)
		{
			if (m_apFile[hFile]->Close())
			{
				System::Get()->PopCurrentMemoryAllocationId();
				delete m_apFile[hFile];
			}
			else
			{
				System::Get()->PopCurrentMemoryAllocationId();
				MEMORY_TRACKER_POP("Audio.Streamer");
				return false;
			}
		}
		else
		{
			delete m_apFile[hFile];
		}
	}
	else
	{
		System::Get()->PopCurrentMemoryAllocationId();
		MEMORY_TRACKER_POP("Audio.Streamer");
		return false;
	}

	m_apFile[hFile] = 0;

	MEMORY_TRACKER_POP("Audio.Streamer");

	return true;
}


void Audio::Core::FileStreamManager::Update()
{
	AUDIO_PERF_EVENT("Audio.FileStream.Update");

	m_lockObject.Acquire();

	Btl::Vector<FS_STREAM_HANDLE> m_vNewStreamsToUpdate;

	if (m_vStreamsToUpdate.Size() > 0)
		m_vNewStreamsToUpdate = m_vStreamsToUpdate;

	m_vStreamsToUpdate.Clear();

	m_lockObject.Release();

	// if any streams wants to be released or destroyed then wait until any pending loads have finished then do it
	for (int i=0; i < m_vNewStreamsToUpdate.Size(); i++)
	{
		FileStream* pStream = GetStream(m_vNewStreamsToUpdate[i]);

		if (pStream)
		{
			if (pStream->LoadRequestSubmitted() == false)
			{
				if (pStream->QueryStateFlag(STREAM_RELEASING))
					pStream->Release();

				else if (pStream->QueryStateFlag(STREAM_RESET_REQUIRED))
					pStream->Reset();

				if (pStream->QueryStateFlag(STREAM_DESTROYING))
				{
					if(!pStream->m_bLoadRequestSubmitted)
					{
						DoStreamDestroy(m_vNewStreamsToUpdate[i]);		// attention. this NULLS the stream. hence the continue
						continue;
					}
					else
					{
						BIZ_ASSERT_FAIL_SPEED("(Please Grab Lee) -- Destroying a stream that has pending load requests.");
					}
				}
			}
			else
			{
				m_lockObject.Acquire();
				m_vStreamsToUpdate.PushBack(m_vNewStreamsToUpdate[i]);
				m_lockObject.Release();
			}

		}
	}

	for (U32 i = 0; i < MAX_NUM_STREAMING_FILES; i++)
	{
		if (m_apFile[i])
		{
			if (m_apFile[i]->IsReleasing() && m_apFile[i]->IsNotInUse())
				DoFileRelease(i);
		}
	}

	#ifdef ENABLE_DEBUG_EXTENSIONS
	if (g_bFileStreamerDumpStreamInfo)
	{
		DumpStreamInfo();
		g_bFileStreamerDumpStreamInfo = false;
	}
	#endif
}


Audio::Core::FileStreamerDebugRenderer::FileStreamerDebugRenderer()
{
	xPos = 120.f;
	yPos = 250.f;
	xSize = 100.f;
	ySize = 100.f;
	f32MeterWidth = 780.f;
	f32MeterHeight = 14.f;
	f32SecondMeterYOffset = 25.f;
	f32TextYOffset = 20.f;
	f32EOF_READ = 0.f;
	f32FINISHED = 100.f;
	f32UNDERRUN = 180.f;
	f32RESET_REQUIRED = 270.f;
	f32RELEASING = 365.f;
	f32RELEASED = 455.f;
	f32DESTROYING = 540.f;
	f32BUFFERS_ALLOCATED = 645.f;
	f32RELEASE_REQUIRED = 1050.f;
	f32DATA_NOT_READY = 1180.f;
	f32YET_TO_UPDATE = 1320.f;
	f32TextSize = 20.f;

	f32XStartPos = 0.f;
	f32YStartPos = 0.f;
}


void Audio::Core::FileStreamerDebugRenderer::BeginRender()
{
	f32XStartPos = 0.f;
	f32YStartPos = 0.f;
}


void Audio::Core::FileStreamerDebugRenderer::DrawOutlineBox(F32 f32xPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, F32 f32OutlineWidth, U32 u32FillColour, U32 u32OutlineColour)
{
	// draw box outline
	Audio::System::DrawBox(f32xPos - f32OutlineWidth, f32YPos - f32OutlineWidth, f32XSize + (f32OutlineWidth * 2), f32OutlineWidth, u32OutlineColour);
	Audio::System::DrawBox(f32xPos - f32OutlineWidth, f32YPos - f32OutlineWidth, f32OutlineWidth, f32YSize + (f32OutlineWidth * 2), u32OutlineColour);
	Audio::System::DrawBox(f32xPos + f32XSize, f32YPos - f32OutlineWidth, f32OutlineWidth, f32YSize + (f32OutlineWidth * 2), u32OutlineColour);
	Audio::System::DrawBox(f32xPos - f32OutlineWidth, f32YPos + f32YSize, f32XSize + (f32OutlineWidth * 2), f32OutlineWidth, u32OutlineColour);
	// draw box background
	Audio::System::DrawBox(f32xPos, f32YPos, f32XSize, f32YSize, u32FillColour);
}


void Audio::Core::FileStreamerDebugRenderer::PrintStreamInfoString(F32 f32XOffset, bool bHighlighted, char* strText)
{
	F32 f32YOffset = 0.f;
	if (f32XOffset >= 700.f)
	{
		f32XOffset -= 700.f;
		f32YOffset = 15.f;
	}
	Audio::System::PrintAt(f32XStartPos + f32XOffset, f32YStartPos + f32YOffset, (bHighlighted) ? 0xffffffff : 0x888888ff, f32TextSize, strText);
}

#ifndef CFG_SPEED 
Btl::String GetFormatString(U32 uFormat)
{
	switch (uFormat)
	{
	case Audio::Core::uPCM16BitSignedInt: return Btl::String("Pcm");
	case Audio::Core::uADPCM4Bit: return Btl::String("Adpcm");
	case Audio::Core::uXMA16Bit: return Btl::String("Xma1");
	case Audio::Core::uXMA216Bit: return Btl::String("Xma2");
	case Audio::Core::uMP3: return Btl::String("Mp3");
	}
	return Btl::String("Dunnoes");
}
#endif
  
void Audio::Core::FileStreamerDebugRenderer::DrawStreamInfoBox(U32 u32Size, U32 u32BytesRead, U32 u32BytesWritten, U64 u64TotalBytesWritten, U64 u64TotalBytesRead, U32 u32BufferSize, U32 u32ItemIndex, U32 u32StreamFlags, const Btl::String& name, U32 format)
{
#ifndef CFG_SPEED 
	if (u32Size == 0)
		return;

	f32XStartPos = xPos;
	f32YStartPos = g_bFileStreamerRenderYOffset + (100.f * u32ItemIndex);
	F32 f32WrittenRatio = (F32)u32BytesWritten / (F32)u32Size;
	F32 f32ReadRatio = (F32)u32BytesRead / (F32)u32Size;
	F32 f32WrittenInBufferRatio = F32(u64TotalBytesWritten % u32BufferSize) / (F32)u32BufferSize;
	F32 f32ReadInBufferRatio = F32(u64TotalBytesRead % u32BufferSize) / (F32)u32BufferSize;

	System::PrintAt(f32XStartPos, f32YStartPos, 0xffffffff, f32TextSize, (char*)name.Chars());
	System::PrintAt(f32XStartPos+f32MeterWidth-f32TextSize*3.f, f32YStartPos, 0xffffffff, f32TextSize, (char*)GetFormatString(format).Chars());
	f32YStartPos += f32TextSize;

	// draw meter 
	DrawOutlineBox(f32XStartPos, f32YStartPos, f32MeterWidth, f32MeterHeight, 2, 0x88000000, 0xffffffff);
	// draw bytes written
	Audio::System::DrawBox(f32XStartPos, f32YStartPos, f32MeterWidth * f32WrittenRatio, f32MeterHeight, 0xffff8888);
	// draw bytes read
	Audio::System::DrawBox(f32XStartPos, f32YStartPos, f32MeterWidth * f32ReadRatio, f32MeterHeight, 0xff88ff88);

	f32YStartPos += f32SecondMeterYOffset;

	// draw meter 
	DrawOutlineBox(f32XStartPos, f32YStartPos, f32MeterWidth, f32MeterHeight, 2, 0x88000000, 0xffffffff);
	// draw bytes written and read in buffer
	if (f32WrittenInBufferRatio > f32ReadInBufferRatio)
		Audio::System::DrawBox(f32XStartPos + f32MeterWidth * f32ReadInBufferRatio, f32YStartPos, f32MeterWidth * (f32WrittenInBufferRatio - f32ReadInBufferRatio), f32MeterHeight, 0xffffff88);
	else
	{
		Audio::System::DrawBox(f32XStartPos, f32YStartPos, f32MeterWidth * f32WrittenInBufferRatio, f32MeterHeight, 0xffffff88);
		Audio::System::DrawBox(f32XStartPos + f32MeterWidth * f32ReadInBufferRatio, f32YStartPos, f32MeterWidth * (1.f - f32ReadInBufferRatio), f32MeterHeight, 0xffffff88);
	}
	
	f32YStartPos += f32TextYOffset;

	PrintStreamInfoString(f32EOF_READ,					(u32StreamFlags & STREAM_EOF_READ) ? true : false, "EOF read/");
//	PrintStreamInfoString(f32FINISHED,					(u32StreamFlags & STREAM_FINISHED) ? true : false, "finished/");
	PrintStreamInfoString(f32UNDERRUN,					(u32StreamFlags & STREAM_UNDERRUN) ? true : false, "underrun/");
	PrintStreamInfoString(f32RESET_REQUIRED,			(u32StreamFlags & STREAM_RESET_REQUIRED) ? true : false, "reset req/");
	PrintStreamInfoString(f32RELEASING,					(u32StreamFlags & STREAM_RELEASING ) ? true : false, "releasing/");
	PrintStreamInfoString(f32RELEASED,					(u32StreamFlags & STREAM_RELEASED) ? true : false, "released/");
	PrintStreamInfoString(f32DESTROYING,				(u32StreamFlags & STREAM_DESTROYING) ? true : false, "destroying/");
	PrintStreamInfoString(f32BUFFERS_ALLOCATED,			(u32StreamFlags & STREAM_BUFFERS_ALLOCATED) ? true : false, "buffs alloced/");
	PrintStreamInfoString(f32DATA_NOT_READY,			(u32StreamFlags & STREAM_DATA_NOT_READY) ? true : false, "data unready/");
	PrintStreamInfoString(f32YET_TO_UPDATE,				(u32StreamFlags & STREAM_YET_TO_UPDATE) ? true : false, "yet to upd");
#endif
}


void Audio::Core::FileStreamManager::Render()
{ 
#ifndef CFG_SPEED 
	if (g_bFileStreamerRenderStreams)
	{
		U32 u32Index = 0;
		for (U32 i=0; i < m_apStream.GetHighestNodeIndex(); i++)
		{
			FileStream* pStream = m_apStream.GetByIndex(i);
			if (pStream)
			{
				bool bDraw = true;

				if (g_bFileStreamerRenderOnlyActiveStreams && !pStream->QueryStateFlag(STREAM_ACTIVE))
					bDraw = false;

				if (bDraw)
				{
					U32 u32StreamSize = pStream->GetStreamTotalDataSize();
					U32 u32StreamWritePos = pStream->GetBytesWrittenPosInStream();
					U32 u32StreamReadPos = pStream->GetBytesReadPosInStream();
					U64 u64TotalBytesWritten = pStream->GetTotalBytesWritten();
					U64 u64TotalBytesRead = pStream->GetTotalBytesRead();
					U32 u32BufferSize = pStream->GetBufferSize();

					m_DebugRenderer.DrawStreamInfoBox(u32StreamSize, u32StreamReadPos, u32StreamWritePos, u64TotalBytesWritten, u64TotalBytesRead, u32BufferSize, u32Index, pStream->m_u32StateFlags, Btl::String("[") + pStream->m_FileStreamDef.m_strBankFilename + "]  " + pStream->m_FileStreamDef.m_strWaveName, pStream->m_FileStreamDef.m_u32Format);
					u32Index++;
				}
			}
		}
	}
#endif
}


U32 Audio::Core::FileStreamManager::GetStreamPosition(FS_STREAM_HANDLE hStream)
{
	if (GetStream(hStream))
		return GetStream(hStream)->GetPosition();

	return 0;
}


void Audio::Core::FileStreamManager::ReadRequestProcessed(WaveServerHandle hStream, U32 u32NumBytesRead, U32 u32RequestId)
{
	if (GetStream(hStream))
		GetStream(hStream)->ReadRequestProcessed(u32NumBytesRead, u32RequestId);
}


void Audio::Core::FileStreamManager::ExtendedInfo(const char* strInfo, ...)
{
	// TODO: check for buffer overrun

	if (m_eDebugLevel > eExtendedInfo)
		return;

	char strTemp[256] = {0};
	va_list arglist;
	va_start(arglist, strInfo);
	vsprintf(strTemp, strInfo, arglist);
	va_end(arglist);

	AUDIO_SYS_DEBUG(strTemp);
}


void Audio::Core::FileStreamManager::Info(const char* strInfo, ...)
{
	// TODO: check for buffer overrun

	if (m_eDebugLevel > eStandardInfo)
		return;

	char strTemp[256] = {0};
	va_list arglist;
	va_start(arglist, strInfo);
	vsprintf(strTemp, strInfo, arglist);
	va_end(arglist);

	AUDIO_SYS_INFO(strTemp);
}


void Audio::Core::FileStreamManager::Warning(const char* strInfo, ...)
{
	// TODO: check for buffer overrun

	if (m_eDebugLevel > eWarning)
		return;

	char strTemp[256] = {0};
	va_list arglist;
	va_start(arglist, strInfo);
	vsprintf(strTemp, strInfo, arglist);
	va_end(arglist);

	AUDIO_SYS_WARN(strTemp);
}

Audio::Core::FileStreamPlaybackBuffer* Audio::Core::FileStreamPlaybackBuffer::m_pFileStreamPlaybackBuffer = NULL;

Audio::Core::FileStreamPlaybackBuffer::FileStreamPlaybackBuffer(U32 u32MemorySize)
    : m_Entries( 0 )
	, m_MemorySize(u32MemorySize)
    , m_MemoryFree( m_MemorySize )
	
    , m_pMemoryFree( (char*)m_pMemoryBase )
{
	MEMORY_TRACKER_PUSH("Audio.StreamerBuffers");
	m_pMemoryBase = (m_MemorySize) ? System::Alloc(m_MemorySize, 128) : 0;
	MEMORY_TRACKER_POP("Audio.StreamerBuffers");

	m_u32NumBuffers = 0;
    
    for ( U32 i=0; i<m_MaxEntries; i++ )
    {
        m_Size[i] = -1;
    }
}

Audio::Core::FileStreamPlaybackBuffer::~FileStreamPlaybackBuffer()
{
	MEMORY_TRACKER_PUSH("Audio.StreamerBuffers");
	System::Free(m_pMemoryBase, System::ePhysical);
	MEMORY_TRACKER_POP("Audio.StreamerBuffers");

    m_pMemoryBase = NULL;
}


void* Audio::Core::FileStreamPlaybackBuffer::Alloc( const U32 input_size, const U32 uAlignment, const Audio::System::AllocType eType)
{
	if (m_MemorySize)
	{
		// alignment and memory type is not supported.
		assert(uAlignment == (U32)-1);
		assert(eType == -1);

		const U32 size = ( input_size + 127 ) & ~127;
	    
		for ( U32 i=0; i<m_Entries; i++ )
		{
			if ( m_Size[i] == size )
			{
				void* ptr = m_Array[i].Alloc();
				if ( ptr )
				{
					memset( ptr, 0, size );
					m_u32NumBuffers++;
					return ptr;
				}
				else break;
			}
		}
	    
		if ( m_MemoryFree >= size )
		{
			void* ptr = m_pMemoryFree;
			m_pMemoryFree += size;
			m_MemoryFree -= size;
			memset( ptr, 0, size );
			m_u32NumBuffers++;
			return ptr;
		}
	    
		AUDIO_CORE_FATAL( "FileStreamPlaybackBuffer::Alloc() : m_MemorySize needs to be increased! [m_MemorySize = %d]\n", m_MemorySize );
		return NULL;
	}
	else
	{
		m_u32NumBuffers++;
		return Audio::System::Alloc(input_size, (uAlignment==-1) ? 1 : uAlignment , (eType == -1) ? Audio::ISystem::eVirtual : eType);
	}
}

void Audio::Core::FileStreamPlaybackBuffer::Free( void* ptr, const U32 input_size, const Audio::System::AllocType eType )
{
	if (m_MemorySize)
	{
		assert(eType == -1);

		const U32 size = ( input_size + 127 ) & ~127;
	    
		for ( U32 i=0; i<m_Entries; i++ )
		{
			if ( m_Size[i] == size )
			{
				m_Array[i].Free( ptr );
				m_u32NumBuffers--;
				return;
			}
		}

		if ( m_Entries < m_MaxEntries )
		{
			const U32 index = m_Entries++;
			m_Size[index] = size;
			m_Array[index].Free( ptr );
			m_u32NumBuffers--;
			return;
		}

		AUDIO_CORE_FATAL( "FileStreamPlaybackBuffer::Free() : m_MaxEntries needs to be increased! [m_MaxEntries = %d]\n", m_MaxEntries );
	}
	else
	{
		m_u32NumBuffers--;
		return Audio::System::Free(ptr, (eType == -1) ? Audio::ISystem::eVirtual : eType);
	}
}

void Audio::Core::FileStreamPlaybackBuffer::Init(U32 u32MemorySize)
{
	if (!m_pFileStreamPlaybackBuffer)
		m_pFileStreamPlaybackBuffer = new FileStreamPlaybackBuffer(u32MemorySize);
}


#ifdef AUDIO_STREAMER_DEBUG_INFO
F32 f32XPosNumEntries = 200.f;
F32 f32XPosText = 260.f;
F32 f32XPosSize = 500.f;
void Audio::Core::FileStreamPlaybackBuffer::DebugInfo()
{
	if (Audio::System::GetPrintAtCallback())
	{
		F32 f32YPos = 150.f;

		for ( U32 i=0; i<m_Entries; i++ )
		{
			Audio::System::GetPrintAtCallback()(f32XPosNumEntries, f32YPos, "%d", m_Array[i].m_Entries);
			Audio::System::GetPrintAtCallback()(f32XPosText, f32YPos, "buffers of size");

			Audio::System::GetPrintAtCallback()(f32XPosSize, f32YPos, "%d (0x%x)", m_Size[i], m_Size[i]);
			f32YPos += 30.f;
		}
	}
}
#endif


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
