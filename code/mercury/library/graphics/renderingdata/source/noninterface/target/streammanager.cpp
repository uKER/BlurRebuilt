#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include <System/Serialise/I_FileSystem.h>
#include "core/maths/conversion.h"
#include "core/threads/thread.h"
#include "core/threads/event.h"
#include "streamingstatistics.h"
#include <Display/Device/DeviceMarkers.h>

//#pragma optimize ("", off)

extern Scheduler::jqModule MemCopyModule;

namespace Sys {class I_File;}

namespace RenderingData {
namespace Target 
{

Debug::Float g_maxMbPerSecond("Graphics.RenderingData.Streaming.MaxMbPerSecond", 100.f);

////////////////////////////////////////////////////////////////////////////////////////

Sys::MemoryBuffer g_chunkAllocator;
ChunkAllocator::MemoryMap ChunkAllocator::s_usedMap;
MT::Lock g_chunkAllocatorLock;

U8* ChunkAllocator::Allocate(S32 n)
{
	g_chunkAllocatorLock.Enter();

	U8* p = 0;

/*#ifndef PLATFORM_PS3
	if (n > (S32) g_chunkAllocator.GetTotalSize())
#ifndef PLATFORM_PC
		Display2::Device()->AllocMemory(n, 4096, 0, &p, Display2::E_MemoryType_Misc);
#else
		p = new U8[n];
#endif
	else
#endif*/
		p = g_chunkAllocator.Allocate(n);

	if (p) s_usedMap[p] = n;

	g_chunkAllocatorLock.Exit();

	return p;
}

void ChunkAllocator::Free(U8* p)
{
	if (p)
	{
		g_chunkAllocatorLock.Enter();

		MemoryMap::Iterator i = s_usedMap.Find(p);
		assert(i != s_usedMap.End());
		S32 n = i->GetValue();
		s_usedMap.Erase(i);

/*#ifndef PLATFORM_PS3
		if (n > (S32) g_chunkAllocator.GetTotalSize())
#ifndef PLATFORM_PC
			Display2::Device()->FreeMemory(n, p, Display2::E_MemoryType_Misc);
#else
			delete [] p;
#endif
		else
#endif*/
			g_chunkAllocator.Free(p, n);

		g_chunkAllocatorLock.Exit();
	}
}

/////////////////////////////////////////////////////////////////////////////////

class LoadingThread : public MT::Thread
{
public:

	LoadingThread() : m_startLoading("StartLoading"), m_finishedLoading("FinishedLoading"), m_active(false)
	{
	}

	struct Request
	{
		Sys::I_File* pFile;
		U64 offset;
		U64 size;
		StreamManager::AutoChunkPointer pDest;
		S32 index;
		bool loadFailed;
	};

	void ClearRequests()
	{
		m_lock.Enter();
		m_requests.Clear();
		m_lock.Exit();
	}

	void AddRequest(Request& request)
	{
		m_lock.Enter();
		m_requests.PushBack(request);
		m_startLoading.Signal();
		m_lock.Exit();
	}

	S32 GetNumRequests()
	{
		m_lock.Enter();
		S32 n = m_requests.Size();
		m_lock.Exit();
		return n;
	}

	bool GetCompleted(Request& request)
	{
		m_lock.Enter();

		bool ret = false;

		if (!m_completed.Empty())
		{
			request = m_completed.Front();
			m_completed.PopFront();
			ret = true;
		}

		m_lock.Exit();
		return ret;
	}

	void WaitUntilFinished()
	{
		m_finishedLoading.Wait(1);
		m_startLoading.Signal();
		m_finishedLoading.Wait();
	}

	bool IsActive()
	{
		return m_active;
	}

	virtual void ThreadMain()
	{

		while (true)
		{
			m_active = false;
			m_startLoading.Wait();
			m_active = true;

			Request request;

			while (PopFront(request))
			{
				Time::C_Time startTime = Time::GetPerformanceCounter();

				request.pFile->SetPosition(request.offset);

				if (!request.pFile->Read(&*request.pDest, (U32)request.size))
					request.loadFailed = true;

				Time::C_Time duration = Time::GetPerformanceCounter() - startTime;
				Time::C_Time requiredTime = (U64)((request.size * 1000000.f) / (g_maxMbPerSecond * 1024 * 1024));

				if (duration < requiredTime)
					Sleep((U32)((requiredTime - duration) / 1000));

				m_lock.Enter();
				m_completed.PushBack(request);
				m_lock.Exit();
			}

			m_finishedLoading.Signal();
		}
	}

private:

	bool PopFront(Request& request)
	{
		m_lock.Enter();

		bool ret = false;

		if (!m_requests.Empty())
		{
			request = m_requests.Front();
			m_requests.PopFront();
			ret = true;
		}

		m_lock.Exit();
		return ret;
	}
	
	MT::Event m_finishedLoading;
	MT::Event m_startLoading;
	MT::Lock m_lock;

	Btl::List<Request> m_requests;
	Btl::List<Request> m_completed;
	volatile bool m_active;
};

LoadingThread g_loadingThread;

StreamManager StreamManager::s_instance;

StreamManager::StreamManager()
{
	m_preLoad = false;
	m_Destroyed = false;
	m_memoryLimit = INT_MAX;

	g_loadingThread.Start(0xffff, 2, "LoadingThread"); // loads at 1/3 speed at normal priority

	m_JobQueuePolicy = -1;
	m_JobQueueGroupDecompression = Scheduler::JQ_GROUP_NONE;
	m_JobQueueGroupGPUCopy = Scheduler::JQ_GROUP_NONE;
	m_JobQueueGroupRequest = Scheduler::JQ_GROUP_NONE;

	MEMORY_TRACKER_PUSH("application.streamer");

	if (g_chunkAllocator.BufferBegin() == 0)
		g_chunkAllocator.Initialise(1024 * 1024 * 4, 16);	// [Blur2PC] retail PC: 4 MB (Blur.exe StreamManager ctor FUN_006cc5c0, Initialise(0x400000, 0x10); _work/ghidra/streammanager_ctor.c). The snapshot's 2 MB leaves retail PC chunks up to 2,623,488 bytes uncacheable, and a chunk that needs one as a prerequisite never loads, so the race load never finishes (usa_amboy, usa_la_longbeachdocks, uk_brighton).
	
	MEMORY_TRACKER_POP("application.streamer");
}

StreamManager::~StreamManager()
{
	if (!m_Destroyed) Destroy();
}

void StreamManager::Destroy()
{
	if (m_JobQueueGroupDecompression != Scheduler::JQ_GROUP_NONE)
		Scheduler::DestroyGroup( m_JobQueueGroupDecompression );

	if (m_JobQueueGroupGPUCopy != Scheduler::JQ_GROUP_NONE)
		Scheduler::DestroyGroup( m_JobQueueGroupGPUCopy );

	if (m_JobQueueGroupRequest != Scheduler::JQ_GROUP_NONE)
		Scheduler::DestroyGroup( m_JobQueueGroupRequest );
		
	m_Destroyed = true;
}

S32 StreamManager::MaxChunkSize() const
{
	return g_chunkAllocator.GetTotalSize();
}

bool StreamManager::LoadChunk(S32 chunkIndex, Sys::I_File* pFile, U64 offset, U64 size)
{
	assert(pFile);

	LoadingThread::Request request;
	Allocate(request.pDest, (S32) size);

	if (request.pDest != 0)
	{
		request.pFile = pFile;
		request.offset = offset;
		request.size = size;
		request.index = chunkIndex;
		request.loadFailed = false;

		g_loadingThread.AddRequest(request);

		LoadingStatistics::Instance().Read(offset, size);

		return true;
	}
	else
		return false;
}

void StreamManager::Update()
{
}

S32 StreamManager::GetNumRequests() const
{
	return g_loadingThread.GetNumRequests();
}

void StreamManager::ClearRequests()
{
	g_loadingThread.ClearRequests();
}

void StreamManager::FlushRequests()
{
	g_loadingThread.ClearRequests();
	g_loadingThread.WaitUntilFinished();

	S32 index;
	bool loadFailed;
	while (GetChunkData(index, loadFailed) != 0) {}
}

bool StreamManager::IsLoadingChunk() const
{
	return g_loadingThread.IsActive();
}

StreamManager::AutoChunkPointer StreamManager::GetChunkData(S32& index, bool& loadFailed)
{
	LoadingThread::Request request;
	
	if (g_loadingThread.GetCompleted(request))
	{
		index = request.index;
		loadFailed = request.loadFailed;
	}
	else
	{
		index = -1;
		loadFailed = false;
	}

	return request.pDest;
}

void StreamManager::BeginCopyList()
{
	m_memCopyWorker.m_cpuCopyList.Clear();
	m_memCopyWorker.m_gpuCopyList.Clear();
}

void StreamManager::SetPreLoad(bool preLoad)
{
	if(m_preLoad != preLoad)
		LoadingStatistics::Instance().SetPreLoad(preLoad);
	m_preLoad = preLoad;
}

void StreamManager::AsynchCopy(void* pDst, const void* pSrc, S32 size, AsynchCopyType type)
{
	if (!pDst)
		return;

	//(*(U32*)pDst) = 0x0f0f0f0f;
	assert(pDst != NULL);
	assert(pSrc != NULL);
	assert(size > 0);
	//assert((U32(pDst) & 0xf) == 0);
	//assert((U32(pSrc) & 0xf) == 0);

#if defined (PLATFORM_PS3)

	if (m_preLoad || type == StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU)
	{
		m_memCopyWorker.m_cpuCopyList.PushBack();
		MemCopyWorker::AsynchCopy& copy(m_memCopyWorker.m_cpuCopyList.Back());
		copy.pDst = pDst;
		copy.pSrc = pSrc;
		copy.size = Btl::AlignUp(U32(size), 16);

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		Display2::Device()->CheckProtectedMemory(copy.size, (U8*)copy.pDst);
		#endif
	}
	else
	{
		m_memCopyWorker.m_gpuCopyList.PushBack();
		MemCopyWorker::AsynchCopy& copy(m_memCopyWorker.m_gpuCopyList.Back());
		copy.pDst = pDst;
		copy.pSrc = pSrc;
		copy.size = size;

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		Display2::Device()->CheckProtectedMemory(copy.size, (U8*)copy.pDst);
		#endif
	}

#else
	// Can't multi-thread as address was acquired with D3D lock.
	memcpy(pDst, pSrc, size);
#endif
}

void StreamManager::SetJobQueuePolicy(Scheduler::PolicyHandle policyHandle)
{
	m_JobQueuePolicy = policyHandle;
}

Scheduler::PolicyHandle StreamManager::GetPolicy() const
{
	return m_JobQueuePolicy;
}

Scheduler::GroupHandle StreamManager::GetDecompressionGroup() const
{
	return m_JobQueueGroupDecompression;
}

Scheduler::GroupHandle StreamManager::GetRequestGroup() const
{
	return m_JobQueueGroupRequest;
}

void StreamManager::SetMemoryLimit(S32 bytes)
{
	m_memoryLimit = bytes;
}

S32 StreamManager::GetMemoryLimit() const
{
	return m_memoryLimit;
}

void StreamManager::Create()
{
	LoadingStatistics::Instance().Initialise();

	if (m_JobQueueGroupDecompression == Scheduler::JQ_GROUP_NONE)
	{
		m_JobQueueGroupDecompression = Scheduler::CreateGroup();
	}

	if (m_JobQueueGroupRequest == Scheduler::JQ_GROUP_NONE)
	{
		m_JobQueueGroupRequest = Scheduler::CreateGroup();
	}

	if (m_JobQueueGroupGPUCopy == Scheduler::JQ_GROUP_NONE)
	{
		m_JobQueueGroupGPUCopy = Scheduler::CreateGroup();
		Scheduler::AddGroupDependency(m_JobQueueGroupGPUCopy, m_JobQueueGroupDecompression);
	}
}

void StreamManager::BeginTasks()
{
#ifndef PLATFORM_PC
	if (!m_memCopyWorker.m_cpuCopyList.Empty())
	{
		m_memCopyWorker.m_pItems = &m_memCopyWorker.m_cpuCopyList[0];
		m_memCopyWorker.m_count = m_memCopyWorker.m_cpuCopyList.Size();

		if (m_JobQueuePolicy > -1)
		{
			void* p = &m_memCopyWorker;
			Scheduler::AddTask(MemCopyModule, 0, 0, m_JobQueueGroupGPUCopy, m_JobQueuePolicy, &p, sizeof(p));	// [Blur2PC][x64] the whole pointer (4 on x86)
		}
		else
			MemCopyTask(0);
	}
#endif
}

void StreamManager::FlushTasks()
{
#ifndef PLATFORM_PC 

	Scheduler::Flush(m_JobQueueGroupDecompression);
	Scheduler::Flush(m_JobQueueGroupGPUCopy);

#if PLATFORM_PS3
	BeginGPUCopyTasks();
	BeginCopyList();
	Display::Device()->BlockUntilIdle();
#endif

#endif
}

void StreamManager::BeginGPUCopyTasks()
{
#ifdef PLATFORM_PS3
	if (m_memCopyWorker.m_gpuCopyList.Size() > 0)
	{
		Display::Device()->AcquireThreadOwnership();
		DEVICE_MARKER("Streamer");
		for (int i = 0; i < m_memCopyWorker.m_gpuCopyList.Size(); i++)
		{
			const MemCopyWorker::AsynchCopy& copy = m_memCopyWorker.m_gpuCopyList[i];
			Display::GPUImageTransfer(copy.pDst, copy.pSrc, copy.size);
			//memcpy(copy.pDst, copy.pSrc, copy.size);
		}
		DEVICE_MARKER("None");
		Display::Device()->ReleaseThreadOwnership();
	}
#endif
}

bool StreamManager::TasksRunning() const
{
	return !Scheduler::Poll(m_JobQueueGroupDecompression) || !Scheduler::Poll(m_JobQueueGroupGPUCopy);
}

}}
