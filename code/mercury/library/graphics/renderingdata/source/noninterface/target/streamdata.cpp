#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include "renderingdata/archive.h"
#include "renderingdata/noninterface/archive.inl"
#include <Framework/application/CoreSystems.h>
#include <System/Serialise/I_FileSystem.h>
#include "system/serialise/C_MemoryStream.h"
#include "System/Debug/I_DebugRender.h"
#include "core/maths/conversion.h"
#include "btl/containers/algorithm.h"
#include "System/Memory/MemoryTracker.h"
#include "renderingdata/debugtextures.h"
#include "world_render/pvs.h"
#include "streamingstatistics.h"

//#pragma optimize ("", off)
//#define CHUNK_DEBUG

static Debug::Logger &g_LogDebug = Debug::GetLogger("graphics.renderingdata.streamer.memory");
static Debug::Logger &g_LogActivityLoad = Debug::GetLogger("graphics.renderingdata.streamer.activity.load");
static Debug::Logger &g_LogActivityProcess = Debug::GetLogger("graphics.renderingdata.streamer.activity.process");
static Debug::Logger &g_LogActivityUnload = Debug::GetLogger("graphics.renderingdata.streamer.activity.unload");
static Debug::Logger &g_LogActivitySwap = Debug::GetLogger("graphics.renderingdata.streamer.activity.swap");

Debug::Int g_streamerMemoryLimit("Graphics.RenderingData.Streaming.MemoryLimit_Mb", 1000);
Debug::Int g_debugStreamerMemoryFreeBuffer("Graphics.RenderingData.Streaming.FreeMemoryBuffer_Mb", 10);
Debug::Int g_debugStreamerStepDownBufferCount("Graphics.RenderingData.Streaming.StepDownBuffer_Mb", 4);
Debug::Int g_highestChunkToLoad("Graphics.RenderingData.Streaming.HighestChunkToLoad", -1);
Debug::Int g_streamerCacheSize("Graphics.RenderingData.Streaming.CacheSize_Mb", 0);
Debug::Bool g_updateStreamerChunks("Graphics.RenderingData.Streaming.UpdateStreamerChunks", true);
Debug::Bool g_forceUnload("Graphics.RenderingData.Streaming.ForceUnloadIfNotRequested", false);
Debug::Int g_forceUnloadChunk("Graphics.RenderingData.Streaming.ForceUnloadChunk", -1);

Debug::Bool g_updateStreamerRequests("Graphics.RenderingData.Streaming.Requests.UpdateStreamerRequests", true);
Debug::Bool g_streamEverything("Graphics.RenderingData.Streaming.Requests.StreamEverything", false);
Debug::Int g_streamRoomIndex("Graphics.RenderingData.Streaming.Requests.RoomIndex", -1);

Debug::Int g_maxStreamerUpdateTime("Graphics.RenderingData.Streaming.MaxStreamerUpdateTime", 2000);

extern Scheduler::jqModule StreamerRequestModule;

namespace Sys {class I_File;}

namespace RenderingData {
namespace Target 
{

#if defined(PLATFORM_PS3)
	#define LARGE_HEAP Display:: E_AllocHeap_Video
#endif

StreamData::StreamData()
{
	m_pScene = 0;
	m_pFile = 0;
	m_failedCount = 0;
	m_lastUnloadFailedCount = 0;
	m_debugPVSCell = -1;
	m_requestCount = 0;
	m_updateBlocked = false;
}

StreamData::~StreamData()
{
	if (m_pFile)
	{
		FlushTasks();
		StreamManager::Instance().FlushTasks();
		CloseStreamFile();
	}

#if !PLATFORM_PC
	while (!m_stepDownBuffers.Empty())
	{
		U8* p = m_stepDownBuffers.Front();
		m_stepDownBuffers.PopFront();
		Display::Device()->FreeMemory(1024 * 1024, p, LARGE_HEAP, Display2::E_MemoryType_Texture);
	}
#endif
}

void StreamData::SetScenePtr(Scene* pScene)
{
	m_pScene = pScene;

	assert(m_cachedChunks.Size() == 0);
	m_cachedChunks.Resize(m_chunks.Size());
	m_chunkState.Resize(m_chunks.Size());
	m_requestResults.Resize(Btl::AlignUp((U32)m_chunks.Size(), 16));
	S32 maxChunkSize = StreamManager::Instance().MaxChunkSize();

	for (int i = 0; i < m_chunkState.Size(); i++)
	{
		m_chunkState[i].activity = STATE_IDLE;
		m_chunkState[i].shouldCache = false;
		m_chunkState[i].shouldLoad = false;
		m_chunkState[i].cached = false;
		m_chunkState[i].loaded = false;
		m_chunkState[i].disable = (m_chunks[i].GetCompressedSize() > maxChunkSize);
	}

#ifdef CHUNK_DEBUG
	m_chunkDebug.Resize(m_chunks.Size());

	for (int i = 0; i < m_chunkDebug.Size(); i++)
	{
		m_chunkDebug[i].loadCount = 0;
		m_chunkDebug[i].totalMBytes = 0;
		m_chunkDebug[i].framesSinceLoad = 0;
		m_chunkDebug[i].framesSinceUnload = 0;
	}
#endif

	m_chunkToCache = -1;
	m_nextChunkToLoad = -1;
	m_chunkToUnload = -1;
	m_chunkToSwapIn = -1;
	m_chunkToSwapOut = -1;
	m_loadedChunk = -1;
	m_unloadedChunk = -1;

	m_multipleChunksUpdated = false;
	m_nodesChanged = false;
	m_blocksChanged = 0;
	m_chunksChanged = false;
	m_requestCount = 0;
	m_updateBlocked = false;

	m_lastChunkProcessType = 0;
}

void StreamData::OpenStreamFile(const Btl::String& filename)
{
	StreamManager::Instance().Create();

	if (m_pFile)
	{
		assert(0 && "Stream file already open.");
		return;
	}

	Sys::C_OpenParams prms;
	prms.m_BufferSizeHint = 0;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	m_pFile = fs.Open(filename.Chars(), Sys::E_FileAccess_Read, prms);

	if (!m_pFile)
	{
		LOGERROR(g_LogDebug, "Could not open file: '%s'\n", filename.Chars());
		assert(m_pFile && "Could not open file.");
	}
}

void StreamData::CloseStreamFile()
{
	if (m_pFile)
	{
		StreamManager::Instance().FlushRequests();
		delete m_pFile;
		m_pFile = 0;
	}

	//StreamManager::Instance().Destroy();
}

void StreamData::ClearRequests()
{
	if (!g_updateStreamerRequests)
		return;

	memset(m_requestResults.First(), 0, m_requestResults.Size());
	m_streamRequestWorker.m_requests.Clear();
}

void StreamData::RequestAll()
{
	if (!g_updateStreamerRequests)
		return;

	for (int i = 0; i < m_chunkState.Size(); i++)
		m_chunkState[i].shouldLoad = true;
}

void StreamData::RequestLowest(S32 bytesToRequest, S32 groupIndex)
{
	if (!g_updateStreamerRequests)
		return;

	U64 bytesRequested = 0;

	for (int i = 0; i < m_chunks.Size(); i++)
	{
		if (groupIndex > -1 && groupIndex < m_groups[i].Size() && !m_groups[i][groupIndex])
			continue;

		bytesRequested += m_chunks[i].GetOriginalSize();

		if (bytesRequested > bytesToRequest)
			break;

		m_requestResults[i] = true;
	}
}

void StreamData::Request(const RequestInfo* pRequest, S32 nRequests)
{
	if (!g_updateStreamerRequests)
		return;

	PERF_EVENT("StreamData::Request");

	m_requestCount++;

	FlushTasks();
	ClearRequests();

	//RequestLowest(0);

	if (g_streamEverything)
	{
		RequestAll();
		return;
	}

	m_streamRequestWorker.m_requests.Resize(nRequests);
	m_streamRequestWorker.m_roomData.Clear();
	m_streamRequestWorker.m_roomData.Resize(nRequests);

	for (int i = 0; i < m_streamRequestWorker.m_requests.Size(); i++)
		static_cast<RequestInfo&>(m_streamRequestWorker.m_requests[i]) = pRequest[i];

	for (int ir = 0; ir < nRequests; ir++)
	{
		const StreamerRequestWorker::RequestInfo& request(m_streamRequestWorker.m_requests[ir]);

		if (request.directional)
		{
			// perspective projection
			F32 z_near = 0.1f;
			F32 z_far = request.drawDistance;
			F32 Q = z_far / (z_far - z_near);
			F32 h = 1.f / tan(request.fovY / 2.f);
			F32 w = h / request.aspect;

			Maths::HMat44 cameraToDevice;
			cameraToDevice.AX(Maths::HVec4(w, 0.f, 0.f, 0.f ));
			cameraToDevice.AY(Maths::HVec4(0.f, h, 0.f, 0.f ));
			cameraToDevice.AZ(Maths::HVec4(0.f, 0.f, Q, 1.f));
			cameraToDevice.AW(Maths::HVec4(0.f, 0.f, -Q * z_near, 0.f));
			Maths::HMat44 worldToDevice = request.cameraToWorld.InverseRotTrans() * cameraToDevice;

			request.view.Setup(cameraToDevice, worldToDevice, 1.f);

			// debug
			if (request.mainView)
			{
				m_cameraPosition = Maths::HVec3(request.cameraToWorld.AW());
				m_cameraDirection = Maths::HVec3(request.cameraToWorld.AZ());
			}
		}
		else
		{
			// debug
			if (request.mainView)
			{
				m_cameraPosition = request.cameraPosition;
				m_cameraDirection = Maths::Constants<Maths::HVec3>::Zero();
			}
		}

		if (request.pvsCell > -1)
		{
			request.pvsData = m_chunkPvs[request.pvsCell].Data();
			m_debugPVSCell = request.pvsCell;
		}
		else
			request.pvsData = 0;

		if (!request.visibleRooms.Empty() || g_streamRoomIndex != -1)
		{
			Btl::BitVector& roomData(m_streamRequestWorker.m_roomData[ir]);
			roomData.Resize(m_chunks.Size());
			roomData.ClearBits();

			if (g_streamRoomIndex == -1)
			{
				for (int i = 0; i < request.visibleRooms.Size(); i++)
				{
					if (!m_chunkRooms.Empty())
						roomData |= m_chunkRooms[request.visibleRooms[i]];
				}
			}
			else if (g_streamRoomIndex < m_chunkRooms.Size())
			{
				roomData |= m_chunkRooms[g_streamRoomIndex];
			}

			/*for (int chunk = 0; chunk < m_chunkRooms[0].Size(); chunk++)
			{
				if (m_chunkRooms[0][chunk])
					LOGINFO(g_LogDebug, "chunk %i", chunk);
			}*/

			request.roomData = roomData.Data();
		}
		else
			request.roomData = 0;
	}

	m_streamRequestWorker.m_nRequests = m_streamRequestWorker.m_requests.Size();
	m_streamRequestWorker.m_pRequests = m_streamRequestWorker.m_requests.First();
	m_streamRequestWorker.m_pChunkBounds = m_chunkBounds.First();
	m_streamRequestWorker.m_nChunks = m_chunkBounds.Size();
	m_streamRequestWorker.m_pResults = m_requestResults.First();
	m_streamRequestWorker.m_nResults = m_requestResults.Size();

	BeginTasks();
}

void StreamData::BeginTasks()
{
	if (m_streamRequestWorker.m_requests.Size())
	{
		void* p = &m_streamRequestWorker;
		Scheduler::AddTask(StreamerRequestModule, 0, 0, StreamManager::Instance().GetRequestGroup(), StreamManager::Instance().GetPolicy(), &p, sizeof(p));	// [Blur2PC][x64] the whole pointer (4 on x86)
	}
}

void StreamData::FlushTasks()
{
	Scheduler::Flush(StreamManager::Instance().GetRequestGroup());

	// Copy the results into the chunk state.
	{
		for (int ic = 0; ic < m_chunkState.Size(); ic++)
			m_chunkState[ic].shouldLoad = false;

		for (int ic = 0; ic < m_chunkState.Size(); ic++)
		{
			if (m_requestResults[ic])
			{
				for (int ir = 0; ir < m_streamRequestWorker.m_requests.Size(); ir++)
				{
					const StreamerRequestWorker::RequestInfo& request(m_streamRequestWorker.m_requests[ir]);

					bool loadChunk = false;

					const ChunkDefs::BitVector& bitVector = m_groups[ic];
					if(bitVector.Size() == 0)
					{
						loadChunk = true;
					}
					else
					{
						// work out if chunk needs loading from groupMask and groups bitvector
						U32 mask = request.groupMask;
						const U8* pData = bitVector.Data();
						const U32 bufferSize = bitVector.BufferSize();
						for(U32 i=0; i<bufferSize; i++)
						{
							if((pData[i] & mask) != 0)
							{
								loadChunk = true;
								break;
							}

							mask >>= 8;
						}
					}

					if(!loadChunk)
						continue;
					
					//if (request.groupIndex > -1 && request.groupIndex < m_groups[ic].Size() && !m_groups[ic][request.groupIndex])
					//	continue;					

					SetLoadChunk(ic);
					break;
				}
			}
		}
	}
}

bool StreamData::CellRequiresChunk(S32 cell, S32 chunk) const
{
	if (m_chunkPvs[cell][chunk])
		return true;

	for (int i = 0; i < m_chunks[chunk].m_dependants.Size(); i++)
	{
		if (CellRequiresChunk(cell, m_chunks[chunk].m_dependants[i]))
			return true;
	}

	return false;
}

void StreamData::SetCacheChunk(S32 chunk, bool cache)
{
	m_chunkState[chunk].shouldCache = cache;
}

void StreamData::SetLoadChunk(S32 chunk, bool load)
{
	if (m_chunkState[chunk].shouldLoad == load)
		return;

	m_chunkState[chunk].shouldLoad = load;

	if (load)
	{
		for (int i = 0; i < m_chunks[chunk].m_prerequisites.Size(); i++)
			SetLoadChunk(m_chunks[chunk].m_prerequisites[i], true);
	}
	else
	{
		for (int i = 0; i < m_chunks[chunk].m_dependants.Size(); i++)
			SetLoadChunk(m_chunks[chunk].m_dependants[i], false);
	}
}

void StreamData::SetLoadChunk(S32 chunk)
{
	if (m_chunkState[chunk].shouldLoad)
		return;

	m_chunkState[chunk].shouldLoad = true;

	for (int i = 0; i < m_chunks[chunk].m_prerequisites.Size(); i++)
		SetLoadChunk(m_chunks[chunk].m_prerequisites[i], true);
}

bool StreamData::CacheChunk(S32 chunk)
{
	for (int i = 0; i < m_chunks[chunk].m_prerequisites.Size(); i++)
	{
		S32 p = m_chunks[chunk].m_prerequisites[i];
		if (!m_chunkState[p].loaded && !m_chunkState[p].cached)
			return false;
	}

	if (StreamManager::Instance().LoadChunk(chunk, m_pFile, 
											m_chunks[chunk].GetOffset(), 
											m_chunks[chunk].GetCompressedSize()))
	{
		LOGDEBUG(g_LogActivityLoad, "Loading chunk %i", chunk);
		m_chunkToCache = chunk;
		m_chunkState[chunk].activity = STATE_ASYNCH_CACHE;
		return true;
	}
	else
	{
		LOGDEBUG(g_LogActivityLoad, "Could not allocate chunk %i", chunk);
		return false;
	}
}

void StreamData::LoadChunk(S32 chunk)
{
	for (int i = 0; i < m_chunks[chunk].m_prerequisites.Size(); i++)
	{
		S32 p = m_chunks[chunk].m_prerequisites[i];
		if (!m_chunkState[p].loaded || m_chunkState[p].activity != STATE_IDLE)
			return;
	}

	m_chunkState[chunk].activity = STATE_ASYNCH_LOAD;

	if (m_nextChunkToLoad == -1)
		m_nextChunkToLoad = chunk;

	// Move to the front of the list.
	ChunkList::Iterator it = Btl::Find(m_cachedChunksLRU, chunk);
	m_cachedChunksLRU.Erase(it);
	m_cachedChunksLRU.PushFront(chunk);
}

bool StreamData::UnloadChunk(S32 chunk)
{
	for (int i = 0; i < m_chunks[chunk].m_dependants.Size(); i++)
		if (m_chunkState[m_chunks[chunk].m_dependants[i]].loaded
			|| m_chunkState[m_chunks[chunk].m_dependants[i]].activity != STATE_IDLE)
			return false;

	//if (m_chunks[chunk].GetOriginalSize() == 0)
	//	return;

	m_chunkState[chunk].activity = STATE_ASYNCH_UNLOAD;
	m_chunkToUnload = chunk;

	return true;
}

bool StreamData::Busy() const
{
	if (StreamManager::Instance().IsLoadingChunk() || StreamManager::Instance().GetNumRequests() > 0)
		return true;

	if (m_cacheFailed)
		return true;

	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
	{
		if (m_chunkState[chunk].activity != STATE_IDLE)
			return true;
	}

	if (m_requestCount == 1)
		return true;

	if (m_updateBlocked)
		return true;

	return false;
}

void StreamData::LoadAllRequested()
{
	//MEMORYSTATUS status;
	//GlobalMemoryStatus(&status);

	if (!g_updateStreamerChunks)
		return;

	FlushTasks();

#if defined(PLATFORM_PS3)
	Display::Device()->BlockUntilIdle();
#endif

	StreamManager::Instance().SetPreLoad(true);

	bool loading = true;
	S32 failedCount = m_failedCount;

	while (loading && m_failedCount == failedCount)
	{
		UpdateMain();

		loading = false;

		if (StreamManager::Instance().IsLoadingChunk())
			loading = true;

		for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
			if (m_chunkState[chunk].activity != STATE_IDLE)
				loading = true;

		DebugInfo();
	}

	StreamManager::Instance().SetPreLoad(false);

	m_multipleChunksUpdated = true;

	//GlobalMemoryStatus(&status);
	//int breakhere = 0;
}

bool StreamData::UpdateMain()
{
	m_debugBounds.Clear();

	if (!g_updateStreamerChunks)
		return true;

	S32 failedCount = m_failedCount;

	PERF_EVENT("StreamData.UpdateMain");
	MEMORY_TRACKER_PUSH("application.streamer.updatemain");

	//UpdateChunkActivity();

	m_nodesChanged = false;
	m_blocksChanged = 0;
	m_chunksChanged = false;
	m_multipleChunksUpdated = false;
	m_updateBlocked = true;

	m_loadedChunk = -1;
	m_unloadedChunk = -1;

	LoadingStatistics::Instance().Update();

	if (!StreamManager::Instance().TasksRunning())
	{
		if (g_resourceAllocatorMutex.Wait(0))
		{
			m_updateStartTime = Time::GetPerformanceCounter();

			m_updateBlocked = false;

			DeleteBlocks();
			StreamManager::Instance().BeginGPUCopyTasks();
			SwapChunks();
			UpdateChunkActivity();
			ProcessChunks();

			g_resourceAllocatorMutex.Release();
		}
	}
	else
	{
		LoadingStatistics::Instance().WaitForTasks();
	}


#ifdef CHUNK_DEBUG
	for (int i = 0; i < m_chunkDebug.Size(); i++)
	{
		m_chunkDebug[i].framesSinceLoad++;
		m_chunkDebug[i].framesSinceUnload++;
	}
#endif

	MEMORY_TRACKER_POP("application.streamer.updatemain");
	return failedCount == m_failedCount;
}

void StreamData::DeleteBlocks()
{
	PERF_EVENT("StreamData.DeleteBlocks");

	// Delete old blocks.
	m_garbage[0].Clear();

	for (int i = 0; i < m_garbage.Size() - 1; i++)
		Swap(m_garbage[i], m_garbage[i + 1]);

	AllocateStepDownBuffer();
}

void StreamData::AllocateStepDownBuffer()
{
#if !PLATFORM_PC
	//PERF_EVENT("StreamData.AllocateStepDownBuffer");

	while (m_stepDownBuffers.Size() < g_debugStreamerStepDownBufferCount)
	{
		U8* p = 0;
		Display::Device()->AllocMemory(4096, 1024 * 1024, &p, LARGE_HEAP, Display::E_AllocType_Start, Display2::E_MemoryType_Texture);

		if (p == 0)
		{
			//m_failedCount++;
			return;
		}
		else
			m_stepDownBuffers.PushBack(p);
	}
#endif
}

U64 StreamData::GetTotalUsed() const
{
	U64 totalUsed = 0;

	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
		if (m_chunkState[chunk].loaded)
			totalUsed += m_chunks[chunk].GetOriginalSize();

	return totalUsed;
}

struct StreamerMemory
{
	S32 memoryLimit;
	S32 bytesToUnload;
	S32 totalUsed;
};

void StreamData::UpdateChunkActivity()
{
	PERF_EVENT("StreamData.UpdateChunkActivity");

	// Look for new chunks ready to process. 
	S32 loadedChunk;

	do
	{
		bool loadFailed;
		AutoChunkPointer p = StreamManager::Instance().GetChunkData(loadedChunk, loadFailed);

		if (p != 0)
		{
			assert(m_cachedChunks[loadedChunk] == 0);
			assert(m_chunkState[loadedChunk].activity == STATE_ASYNCH_CACHE);
			assert(!m_chunkState[loadedChunk].cached);

			if (!loadFailed)
			{
				m_cachedChunks[loadedChunk] = p;
				m_cachedChunksLRU.PushFront(loadedChunk);
				m_chunkState[loadedChunk].cached = true;
			}

			m_chunkState[loadedChunk].activity = STATE_IDLE;
			m_chunksChanged = true;
		}
	} while (loadedChunk > -1);

	// Work out how many chunks we can fit in the memory buffer.
	U64 bytesRequestedForLoad = 0;
	U64 bytesRequestedForCache = 0;
	S32 highestChunkToLoad = -1;
	S32 highestChunkToCache = -1;

	StreamerMemory gpuMemory;
	StreamerMemory cpuMemory;

	{
		gpuMemory.memoryLimit = Maths::Min(StreamManager::Instance().GetMemoryLimit(), (S32)g_streamerMemoryLimit * 1024 * 1024);
		cpuMemory.memoryLimit = 10 * 1024 * 1024;

#if PLATFORM_PS3
		U32 freeMemory = Display::Device()->GetVideoMemory()->GetFree();
#else
		U32 freeMemory = 1024 * 1024 * 512;
#endif

		gpuMemory.totalUsed = 0;
		cpuMemory.totalUsed = 0;

		for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
		{
			if (m_chunkState[chunk].loaded)
			{
				if (m_chunks[chunk].GetOriginalSize() > 0)
					gpuMemory.totalUsed += (S32) m_chunks[chunk].GetOriginalSize();
				else
					cpuMemory.totalUsed += (S32) m_chunks[chunk].GetCompressedSize();
			}
		}

		gpuMemory.memoryLimit = Maths::Min(gpuMemory.memoryLimit, S32(gpuMemory.totalUsed + freeMemory - (U32)(g_debugStreamerMemoryFreeBuffer * 1024 * 1024)));

		// Without this it can cycle between loading and unloading and never report that it finished.
		if (freeMemory < (U32)g_debugStreamerMemoryFreeBuffer * 1024 * 1024)
			m_failedCount++;

		for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
		{
			if (m_chunkState[chunk].disable)
			{
				m_chunkState[chunk].shouldLoad = false;
				m_chunkState[chunk].shouldCache = false;
			}

			if (m_chunkState[chunk].shouldLoad)
			{
				bytesRequestedForLoad += m_chunks[chunk].GetOriginalSize();

				if (bytesRequestedForLoad < gpuMemory.memoryLimit)
					highestChunkToLoad = chunk;
			}

			if (m_chunkState[chunk].shouldCache)
			{
				bytesRequestedForCache += m_chunks[chunk].GetOriginalSize();

				if (bytesRequestedForCache < gpuMemory.memoryLimit)
					highestChunkToCache = chunk;
			}
		}

		if (g_highestChunkToLoad > -1)
			highestChunkToLoad = Maths::Min((S32)g_highestChunkToLoad, highestChunkToLoad);

		gpuMemory.bytesToUnload = gpuMemory.totalUsed - gpuMemory.memoryLimit + g_debugStreamerMemoryFreeBuffer * 1024 * 1024;
		cpuMemory.bytesToUnload = cpuMemory.totalUsed - cpuMemory.memoryLimit;

		// No need to cache chunks that won't be loaded.
		highestChunkToCache = Maths::Min(highestChunkToCache, highestChunkToLoad);
	}

	if (StreamManager::Instance().GetNumRequests() > 1)
		LoadingStatistics::Instance().IsLoadingChunk();

	m_nextChunkToLoad = -1;
	m_chunkToCache = -1;
	//StreamManager::Instance().ClearRequests();
	m_cacheFailed = false;
	S32 nChunksLoading = 0;

	// Load chunks.
	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
	{
		if (m_chunkState[chunk].activity == STATE_IDLE)
		{
			bool shouldLoad = m_chunkState[chunk].shouldLoad && chunk <= highestChunkToLoad;
			
			if (m_chunkState[chunk].cached && !m_chunkState[chunk].loaded && shouldLoad)
				LoadChunk(chunk);
		}
	}

	DeleteChunks();

	// Load chunks.
	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
	{
		if (m_chunkState[chunk].activity == STATE_IDLE)
		{
			bool shouldLoad = m_chunkState[chunk].shouldLoad && chunk <= highestChunkToLoad;
			bool shouldCache = (m_chunkState[chunk].shouldCache && chunk <= highestChunkToCache) 
								|| (shouldLoad && !m_chunkState[chunk].loaded);

			if (!m_chunkState[chunk].cached && shouldCache)
			{
				m_cacheFailed = !CacheChunk(chunk);

				if (m_cacheFailed && nChunksLoading > 0)
					break;
				else
					nChunksLoading++;
			}
		}
	}

	if (!StreamManager::Instance().IsLoadingChunk())
		LoadingStatistics::Instance().NoRequests();

	// Unload chunks.
	if (gpuMemory.bytesToUnload > 0 
		|| cpuMemory.bytesToUnload > 0 
		|| m_failedCount > m_lastUnloadFailedCount 
		|| g_forceUnload
		|| m_stepDownBuffers.Size() < g_debugStreamerStepDownBufferCount)
	{
		for (int chunk = m_chunks.Size() - 1; chunk > -1; chunk--)
		{
			if (m_chunkState[chunk].activity == STATE_IDLE)
			{
				bool shouldLoad = m_chunkState[chunk].shouldLoad && chunk <= highestChunkToLoad;
				
				if (!shouldLoad && m_chunkState[chunk].loaded)
				{
					if (g_forceUnload)
					{
						UnloadChunk(chunk);
					}
					else if (m_chunks[chunk].GetOriginalSize() > 0)
					{
						if (gpuMemory.bytesToUnload > 0 || m_failedCount > m_lastUnloadFailedCount)
						{
							if (UnloadChunk(chunk))
							{
								m_lastUnloadFailedCount = m_failedCount;
								gpuMemory.bytesToUnload -= m_chunks[chunk].GetOriginalSize();
							}
						}
					}
					else
					{
						if (cpuMemory.bytesToUnload > 0)
						{
							if (UnloadChunk(chunk))
								cpuMemory.bytesToUnload -= m_chunks[chunk].GetCompressedSize();
						}
					}
				}
			}
		}

		// Have to unload something if failed.
		if (m_nextChunkToLoad > -1 && (m_failedCount > m_lastUnloadFailedCount || m_stepDownBuffers.Size() < g_debugStreamerStepDownBufferCount))
		{
			for (int chunk = m_chunks.Size() - 1; chunk > m_nextChunkToLoad; chunk--)
			{
				if (m_chunkState[chunk].activity == STATE_IDLE)
				{
					if (m_chunkState[chunk].loaded)
					{
						UnloadChunk(chunk);
						m_lastUnloadFailedCount = m_failedCount;
						break;
					}
				}
			}
		}
	}

	if (g_forceUnloadChunk > -1 && m_chunkState[g_forceUnloadChunk].loaded && m_chunkState[g_forceUnloadChunk].activity == STATE_IDLE)
		UnloadChunk(g_forceUnloadChunk);

	// Kick off any new chunk loading.
	RenderingData::Target::StreamManager::Instance().Update();
}

void StreamData::DeleteChunks()
{
	PERF_EVENT("StreamData.DeleteChunks");

	// Delete old chunks
	if (m_cachedChunksLRU.Size() > 0)
	{
		S32 chunksCached = m_cachedChunksLRU.Size();
		U64 cachedSize = 0;
		ChunkList::Iterator it;
		S32 nMax = 0;

		for (it = m_cachedChunksLRU.Begin(); it != m_cachedChunksLRU.End(); ++it)
		{
			cachedSize += m_chunks[*it].GetCompressedSize();

			if (cachedSize < g_streamerCacheSize * 1024 * 1024)
				nMax++;
		}

		S32 nToDelete = m_cachedChunksLRU.Size() - nMax;

		for (int i = 0; i < nToDelete; i++)
		{
			S32 chunk = m_cachedChunksLRU.Back();

			if (m_chunkState[chunk].activity == STATE_IDLE && chunk != g_forceUnloadChunk)
			{
				Free(m_cachedChunks[chunk]);
				m_chunkState[chunk].cached = false;
				m_cachedChunksLRU.PopBack();
			}
		}
	}
}

void StreamData::SwapChunks()
{
	PERF_EVENT("StreamData.SwapChunks");

	S32 count = 0;

	// Look for processed chunks ready to swap.
	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
	{

	//if (m_chunkToSwapIn > -1)
	if (m_chunkState[chunk].activity == STATE_PENDING_SWAP_IN)
	{
		SwapChunk(chunk);
		m_loadedChunk = m_chunkToSwapIn;
		m_chunkToSwapIn = -1;
		count++;
	}

	//if (m_chunkToSwapOut > -1) 
	if (m_chunkState[chunk].activity == STATE_PENDING_SWAP_OUT)
	{
		SwapChunk(chunk);
		m_unloadedChunk = m_chunkToSwapOut;
		m_chunkToSwapOut = -1;
		count++;
	}

	}

	if (count > 0)
		m_multipleChunksUpdated = true;
}

void StreamData::SwapChunk(S32 chunk)
{
	ChunkActivity a = m_chunkState[chunk].activity;

	assert(a == STATE_PENDING_SWAP_IN || a == STATE_PENDING_SWAP_OUT);

	PERF_EVENT("StreamData.Swap");
	LOGDEBUG(g_LogActivitySwap, "Swapping chunk %i", chunk);

	S32 nBlocks = m_chunks[chunk].GetNumBlocks();

	for (int i = 0; i < nBlocks; i++)
	{
		S32 b = m_chunks[chunk].GetBlockIndex(i);
		ResourceBlock& block = m_pScene->GetResourceBlock(b);

		if (block.m_resources.Empty() || block.m_back.Empty())
			m_nodesChanged = true;

		if (a == STATE_PENDING_SWAP_IN)
			AddTextures(block.m_back, chunk);
		else
			AddTextures(block.m_back, -1);

		ResourceBlock::SwapNew(block.m_resources, block.m_back);
		RemoveTextures(block.m_back);

		block.m_back.RemoveFromDevice();
		m_garbage.Back().PushBack();
		Btl::Swap(block.m_back, m_garbage.Back().Back());

		m_blocksChanged++;
	}

	if (m_chunkState[chunk].activity == STATE_PENDING_SWAP_IN)
		m_chunkState[chunk].loaded = true;
	else 
		m_chunkState[chunk].loaded = false;

	m_chunkState[chunk].activity = STATE_IDLE;

	m_nUpdatedChunks++;
}

void StreamData::ProcessLoadChunk(S32 chunk)
{
#if !PLATFORM_PC
	AllocateStepDownBuffer();

	if (m_stepDownBuffers.Size() < g_debugStreamerStepDownBufferCount)
	{
		if (m_chunkState[chunk].activity == STATE_ASYNCH_LOAD)
		{
			Discard(chunk);
			m_chunkState[chunk].activity = STATE_IDLE;
			return;
		}
	}
#endif

	MEMORY_TRACKER_PUSH("application.streamer.updateasynch.load");

	bool discard = false;

	assert(m_chunkState[chunk].loaded == false);
	assert(m_chunkState[chunk].activity == STATE_ASYNCH_LOAD);

	PERF_EVENT("StreamData.ASYNCH_LOAD");
	LOGDEBUG(g_LogActivityProcess, "Processing chunk %i", chunk);
	assert(m_cachedChunks[chunk] != 0);

	Sys::C_MemoryStream stream(&*m_cachedChunks[chunk], (U32)m_chunks[chunk].GetCompressedSize());
	Arch::BinaryInputArchive ar(stream);

	S32 nItems;
	ar >> nItems;

	assert(nItems == m_chunks[chunk].GetNumItems());

	#ifdef CHUNK_DEBUG
	m_chunkDebug[chunk].loadCount++;
	m_chunkDebug[chunk].framesSinceLoad = 0;
	m_chunkDebug[chunk].totalMBytes += m_chunks[chunk].GetCompressedSize() / (1024.f * 1024.f);
	#endif

	for (int i = 0; i < nItems; i++)
	{
		PERF_EVENT("StreamData.ASYNCH_LOAD_item");
		const ChunkInfo::Item& item = m_chunks[chunk].GetItem(i);
		ResourceBlock& block = m_pScene->GetResourceBlock(item.block);

		if (item.block == 78)
			int breakhere = 0;

		const ResourceBlock::Resources* pFront = 0;

		if (!block.m_resources.Empty())
			pFront = &block.m_resources;

		assert(pFront == 0 || item.lod > pFront->lod);

		if (block.m_back.Empty())
		{
			//PERF_EVENT("StreamData.Load block");
			block.m_back.pScene = block.m_resources.pScene;
			LodReader<ResourceBlock::Resources> reader(pFront, 0, block.m_back, item.lod);
			ar >> reader;
		}
		else
		{
			//PERF_EVENT("StreamData.Load block 2");
			ResourceBlock::Resources resources;
			resources.pScene = block.m_resources.pScene;
			LodReader<ResourceBlock::Resources> reader(pFront, &block.m_back, resources, item.lod);
			ar >> reader;
			ResourceBlock::SwapNew(block.m_back, resources);
			resources.RemoveFromDevice();
			m_garbage.Back().PushBack();
			Btl::Swap(resources, m_garbage.Back().Back());
		}

		if (block.m_back.discard)
		{
			discard = true;
			Discard(chunk);
			break;
		}

		block.SetChunk(chunk);

		// Nodes are slower to load, don't do anything else this frame.
		//if (block.m_back.nodes.Size() > 0)
		//	loadedNodes = true;
	}

	if (!discard)
	{
		m_chunkToSwapIn = chunk;
		m_chunkState[chunk].activity = STATE_PENDING_SWAP_IN;
	}
	else
	{
		m_chunkState[chunk].activity = STATE_IDLE;
		//loadChunks = false;
	}

	Time::C_Time time = Time::GetPerformanceCounter();
	Time::C_Time duration = time - m_updateStartTime;

	if (duration > g_maxStreamerUpdateTime)
	{
		m_loadChunks = false;
		m_unloadChunks = false;
	}

	m_lastChunkProcessType = 0;

	MEMORY_TRACKER_POP("application.streamer.updateasynch.load");
}

void StreamData::ProcessUnloadChunk(S32 chunk)
{
	MEMORY_TRACKER_PUSH("application.streamer.updateasynch.unload");

	bool discard = false;

	m_chunkToUnload = -1;
	assert(m_chunkState[chunk].loaded == true);
	assert(m_chunkState[chunk].activity == STATE_ASYNCH_UNLOAD);

	PERF_EVENT("StreamData.ASYNCH_UNLOAD");
	LOGDEBUG(g_LogActivityUnload, "Unloading chunk %i", chunk);

#ifdef CHUNK_DEBUG
	m_chunkDebug[chunk].framesSinceUnload = 0;
#endif

	#if !PLATFORM_PC
	for (int i = 0; i < 2; i++)
	{
		if (!m_stepDownBuffers.Empty())
		{
			U8* p = m_stepDownBuffers.Front();
			m_stepDownBuffers.PopFront();
			Display::Device()->FreeMemory(1024 * 1024, p, LARGE_HEAP, Display2::E_MemoryType_Texture);
		}
	}
	#endif

	// Unload in reverse order
	for (int i = m_chunks[chunk].GetNumItems() - 1; i > -1; i--)
	{
		const ChunkInfo::Item& item = m_chunks[chunk].GetItem(i);
		ResourceBlock& block = m_pScene->GetResourceBlock(item.block);

		if (item.block == 78)
			int breakhere = 0;

		if (block.m_back.Empty())
		{
			PERF_EVENT("StreamData.Clear block");
			ResourceBlock::ReduceLod(0, block.m_resources, block.m_back);
		}
		else
		{
			PERF_EVENT("StreamData.Decrease block LOD");
			ResourceBlock::Resources resources;
			resources.pScene = block.m_resources.pScene;
			ResourceBlock::ReduceLod(&block.m_resources, block.m_back, resources);
			ResourceBlock::SwapNew(block.m_back, resources);

			{
				PERF_EVENT("StreamData.Remove From Device");
				resources.RemoveFromDevice();
			}

			m_garbage.Back().PushBack();
			Btl::Swap(resources, m_garbage.Back().Back());
		}

		if (block.m_back.discard)
		{
			discard = true;
			Discard(chunk);
			m_loadChunks = false;
			break;
		}
	}

	if (!discard)
	{
		m_chunkToSwapOut = chunk;
		m_chunkState[chunk].activity = STATE_PENDING_SWAP_OUT;
	}

	Time::C_Time time = Time::GetPerformanceCounter();
	Time::C_Time duration = time - m_updateStartTime;

	if (duration > g_maxStreamerUpdateTime)
	{
		m_loadChunks = false;
		m_unloadChunks = false;
	}

	m_lastChunkProcessType = 1;

	MEMORY_TRACKER_POP("application.streamer.updateasynch.unload");
}

void StreamData::ProcessChunks()
{
	MEMORY_TRACKER_PUSH("application.streamer.ProcessChunks");
	PERF_EVENT("StreamData.ProcessChunks");
	RESOURCE_MEMORY_ID("Streamer");

	StreamManager::Instance().BeginCopyList();

	//bool loadedNodes = false;

	m_loadChunks = true;
	m_unloadChunks = true;

	S32 chunkToLoad = 0;
	S32 chunkToUnload = m_chunks.Size() - 1;

	while (m_loadChunks || m_unloadChunks)
	{
		if (m_loadChunks && (m_lastChunkProcessType == 1 || !m_unloadChunks))
		{
			if (m_chunkState[chunkToLoad].activity == STATE_ASYNCH_LOAD)
				ProcessLoadChunk(chunkToLoad);

			chunkToLoad++;

			if (chunkToLoad == m_chunks.Size())
				m_loadChunks = false;
		}

		if (m_unloadChunks && (m_lastChunkProcessType == 0 || !m_loadChunks))
		{
			if (m_chunkState[chunkToUnload].activity == STATE_ASYNCH_UNLOAD)
				ProcessUnloadChunk(chunkToUnload);

			chunkToUnload--;

			if (chunkToUnload == 0)
				m_unloadChunks = false;
		}
	}

	//m_memCopyWorker.Process();
	StreamManager::Instance().BeginTasks();

	MEMORY_TRACKER_POP("application.streamer.ProcessChunks");
}

void StreamData::Discard(S32 chunk)
{
	// Loading failed, delete any blocks that were created and don't swap.
	for (int i = m_chunks[chunk].GetNumItems() - 1; i > -1; i--)
	{
		const ChunkInfo::Item& item = m_chunks[chunk].GetItem(i);
		ResourceBlock& block = m_pScene->GetResourceBlock(item.block);

		if (!block.m_back.Empty())
		{
			block.m_back.RemoveFromDevice();
			m_garbage.Back().PushBack();
			Btl::Swap(block.m_back, m_garbage.Back().Back());
		}
	}

	m_chunkState[chunk].activity = STATE_IDLE;
	m_failedCount++;
}

}}
