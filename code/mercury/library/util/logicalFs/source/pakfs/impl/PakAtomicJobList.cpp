#include "LogicalFsPch.h"

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressZLib.h>
#include <PakFs/Impl/PakAtomicJobList.h>
#include "tools_shared/SchedulerConfig.h"

 
namespace Pak
{
	C_AtomicJobList::C_AtomicJobList()
	{
	}
	
	bool C_AtomicJobList::Initialise( U32 nrBuffers, U32 bufferSize )
	{
		if (!(MAX_NR_JOBS & (~MAX_NR_JOBS+1)) == MAX_NR_JOBS) return false;
	
		m_PakMutex.Wait();
		for (U32 x = 0 ; x < MAX_NR_JOBS ; x++)
		{
			m_JobList[x].m_pSourceData = NULL;
			m_JobList[x].m_sSourceSize = 0;
			m_JobList[x].m_eJobStatus = C_JobEntry::eStatusUnused;
			m_JobList[x].m_uJobIndex = x;
					
		}
		m_JobPushIndex = 0;
		m_JobPopIndex = 0;
		m_JobCompleteIndex = 0;
		m_JobCompressIndex = 0;
		m_PPUAddress = this;
		m_PakBuffer = new C_PakBuffer( nrBuffers, bufferSize );
		m_MaxPendingJobs = nrBuffers > MAX_NR_JOBS ? MAX_NR_JOBS-1 : nrBuffers-1;
		m_PakMutex.Release();
		return true;
	}
	C_AtomicJobList::~C_AtomicJobList()
	{
		if (m_PakBuffer) delete m_PakBuffer;
		m_PakBuffer = NULL;
	}

	C_PakBuffer* C_AtomicJobList::GetPakBuffer()
	{
		return m_PakBuffer;
	}

	bool C_AtomicJobList::PushJob( C_JobEntry* job )
	{
		bool retval = false;
		m_PakMutex.Wait();
		U32 index = m_JobPushIndex & (MAX_NR_JOBS-1);
		if (m_JobList[ index ].m_eJobStatus == C_JobEntry::eStatusUnused)
		{
			m_JobList[ index ].m_pSourceData = job->m_pSourceData;
			m_JobList[ index ].m_sSourceSize = job->m_sSourceSize;
			m_JobList[ index ].m_eJobStatus = C_JobEntry::eStatusPendingCompress;
			m_JobPushIndex++;
			retval = true;
		}
		else
		{
			retval = false;
		}
		m_PakMutex.Release();
		return retval;
	}
	bool C_AtomicJobList::CanPush()
	{
		bool retval = false;
		m_PakMutex.Wait();
		U32 index = m_JobPushIndex & (MAX_NR_JOBS-1);
		if (m_JobList[ index ].m_eJobStatus == C_JobEntry::eStatusUnused)
		{
			retval = true;
		}
		m_PakMutex.Release();
		return retval;
	}

	bool C_AtomicJobList::ProcessJobs()
	{
		if ( (m_JobPushIndex - m_JobPopIndex) >= m_MaxPendingJobs) return true;
		return false;
	}
	bool C_AtomicJobList::CanPop()
	{
		m_PakMutex.Wait();
		bool retval = false;
		if (m_JobPopIndex < m_JobPushIndex)
		{
			retval = true;
		}
		else
		{
			retval = false;
		}
		m_PakMutex.Release();
		return retval;
	}
	bool C_AtomicJobList::PopJob( C_JobEntry* job )
	{
		m_PakMutex.Wait();
		bool retval = false;
#ifdef PLATFORM_SPU
		Spu::Fetch( this, m_PPUAddress, sizeof(*this));
#endif
		if (m_JobPopIndex < m_JobPushIndex)
		{
			U32 index = m_JobPopIndex & (MAX_NR_JOBS-1);
			job->m_pSourceData = m_JobList[ index ].m_pSourceData;
			job->m_sSourceSize = m_JobList[ index ].m_sSourceSize;
			job->m_eJobStatus  = m_JobList[ index ].m_eJobStatus;
			job->m_uJobIndex   = m_JobList[ index ].m_uJobIndex;
			m_JobList[ index ].m_eJobStatus = C_JobEntry::eStatusCompress;
			m_JobPopIndex++;
#ifdef PLATFORM_SPU
			Spu::Store( m_PPUAddress, this, sizeof(*this));
#endif
			retval = true;
		}
		else
		{
			retval = false;
		}
		m_PakMutex.Release();
		return retval;
	}

	bool C_AtomicJobList::SetJobComplete( U32 jobIndex, U32 size )
	{
		m_PakMutex.Wait();
#ifdef PLATFORM_SPU
		Spu::Fetch( this, m_PPUAddress, sizeof(*this));
#endif
		m_JobList[ jobIndex ].m_eJobStatus = C_JobEntry::eStatusComplete;
		m_JobList[ jobIndex ].m_sSourceSize = size;
		m_JobCompressIndex++;
#ifdef PLATFORM_SPU
		Spu::Store( m_PPUAddress, this, sizeof(*this));
#endif
		m_PakMutex.Release();
		return true;
	}

#ifndef PLATFORM_SPU
	void C_AtomicJobList::SetEA( C_AtomicJobList* ea )
	{
		m_PPUAddress = ea;
	}

	void C_AtomicJobList::DumpStats()
	{
		m_PakMutex.Wait();
		for (U32 index = 0 ; index < MAX_NR_JOBS ; index++)
		{
			switch( m_JobList[ index ].GetJobStatus() )
			{
				case C_JobEntry::eStatusUnused:
					printf("U");
					break;
				case C_JobEntry::eStatusPendingCompress:
					printf("P");
					break;
				case C_JobEntry::eStatusCompress:
					printf("Z");
					break;
				case C_JobEntry::eStatusComplete:
					printf("C");
					break;
			}
		}
		printf("\n");
		m_PakMutex.Release();
	}

	bool C_AtomicJobList::PendingJobs()
	{
		return m_JobPushIndex > m_JobCompressIndex;
	}

	bool C_AtomicJobList::GetCompleteJob( C_JobEntry* job )
	{
		m_PakMutex.Wait();
		bool retval = false;
		if (m_JobCompleteIndex < m_JobPopIndex)
		{
			U32 index = m_JobCompleteIndex & (MAX_NR_JOBS-1);
			if (m_JobList[ index ].GetJobStatus() == C_JobEntry::eStatusComplete)
			{
				job->m_pSourceData = m_JobList[ index ].m_pSourceData;
				job->m_sSourceSize = m_JobList[ index ].m_sSourceSize;
				job->m_eJobStatus  = m_JobList[ index ].m_eJobStatus;
				job->m_uJobIndex   = m_JobList[ index ].m_uJobIndex;
				m_JobCompleteIndex++;
				retval = true;
			}
		}
		else
		{
			retval = false;
		}
		m_PakMutex.Release();
		return retval;
	}

	bool C_AtomicJobList::SetJobUnused( U32 index )
	{
		m_PakMutex.Wait();
		bool retval = false;
		if (m_JobList[ index ].GetJobStatus() == C_JobEntry::eStatusComplete)
		{
			m_JobList[ index ].m_eJobStatus = C_JobEntry::eStatusUnused;
			retval = true;
		}
		else
		{
			retval = false;
		}
		m_PakMutex.Release();
		return retval;
	}
#endif
}
