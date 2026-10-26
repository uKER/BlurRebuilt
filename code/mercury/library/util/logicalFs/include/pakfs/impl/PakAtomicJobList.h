#ifndef LOGICALFS_PAKFS_PAK_ATOMIC_JOB_LIST_H
#define LOGICALFS_PAKFS_PAK_ATOMIC_JOB_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/threads/atomicmutex.h>


namespace Pak
{

	class C_PakBuffer
	{
		U8*			m_Heap;
		U8**		m_HeapList;
		U32			m_BufferSize;
		U32			m_NrBuffers;
		U32			m_AllocationSize;
		S32			m_AllocationIndex;
		MT::Mutex	m_Mutex;
	public:
		C_PakBuffer( U32 nrBuffers, U32 bufferSize ) :
		  m_Mutex("PakAtomicJobList")
		{
			m_NrBuffers = nrBuffers;
			m_BufferSize = bufferSize;
			m_AllocationSize = m_NrBuffers * m_BufferSize ;
			m_Heap = new U8[ m_AllocationSize ];
			m_HeapList = new U8*[ m_NrBuffers ];
			memset(m_Heap,0x11,m_AllocationSize);
			U8* p = m_Heap;
			for (U32 x = 0 ; x < m_NrBuffers ; x++)
			{
				m_HeapList[x] = p;
				p += m_BufferSize;
			}
			m_AllocationIndex = m_NrBuffers;
		}
		~C_PakBuffer()
		{
			if (m_Heap) delete [] m_Heap;
			if (m_HeapList) delete [] m_HeapList;
			m_Heap = NULL;
			m_HeapList = NULL;
		}
		U8* AllocateBuffer()
		{
			m_Mutex.Wait();
			U8* retval = NULL;
			if (m_AllocationIndex > 0)
			{
				m_AllocationIndex--;
				retval = m_HeapList[ m_AllocationIndex ];
			}
			else
			{
				retval = NULL;
			}
			m_Mutex.Release();
			return retval;
		}
		void FreeBuffer( U8* ptr )
		{
			m_Mutex.Wait();
			if ((ptr >= m_Heap) && (ptr < (m_Heap+m_AllocationSize)))
			{
				// Within Range
				m_HeapList[ m_AllocationIndex ] = ptr;
				m_AllocationIndex++;
			}
			m_Mutex.Release();
		}
	};

	class C_JobEntry
	{
	public:
		enum eJobStatus
		{
			eStatusUnused,
			eStatusPendingCompress,
			eStatusCompress,
			eStatusComplete,
		};
		eJobStatus GetJobStatus()
		{
			return m_eJobStatus;
		}
		void*		m_pSourceData;
		S32			m_sSourceSize;
		eJobStatus	m_eJobStatus;
		U32			m_uJobIndex;
	};

	class C_AtomicJobList
	{
		static const U32 MAX_NR_JOBS = 64; // !power of 2
		C_JobEntry			m_JobList[MAX_NR_JOBS];
		U32					m_JobPushIndex;
		U32					m_JobPopIndex;
		U32					m_JobCompleteIndex;
		U32					m_JobCompressIndex;
		MT::AtomicLock		m_PakMutex;
		C_AtomicJobList*	m_PPUAddress;
		C_PakBuffer*		m_PakBuffer;
		U32					m_JobRunningCounter;
		U32					m_MaxPendingJobs;
	public:
		C_AtomicJobList();
		~C_AtomicJobList();
		bool Initialise( U32 nrBuffers, U32 bufferSize );
		C_PakBuffer*	GetPakBuffer();
		bool PushJob( C_JobEntry* job );
		bool CanPush();
		bool ProcessJobs();
		bool CanPop();
		bool PopJob( C_JobEntry* job );
		bool SetJobComplete( U32 jobIndex, U32 size );
		void SetEA( C_AtomicJobList* ea );
		void DumpStats();
	#ifndef PLATFORM_SPU
		bool PendingJobs();
		bool GetCompleteJob( C_JobEntry* job );
		bool SetJobUnused( U32 index );
	#endif

	};

}
#endif