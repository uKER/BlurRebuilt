// Test cmd lines
// Pak   -c c -f z:\models.pak -i z:\ModelsPakContents -r -p / -m PacketXMem -z z:\ModelsPakContents -b
// UNPAK -c x -f z:\models.pak -o z:\DeModelsPakContents -b

#include "LogicalFsPch.h"

#include <system/Perf.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressPacketXMem.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/PakGlobals.h>
#include "Xcompress.h"
#include <tools_shared/SchedulerConfig.h>
#define USE_XMEM_COMPRESSION
#include <misc_shared/compression/xmemcompress.h>

#include <System/Debug/Debug.h>
Debug::Int g_numBuffersPacketXMem("Util.LogicalFs.PacketXMem.NumBuffers", 4, Debug::Range(1, 160), Debug::Persist());
Debug::Int g_numTasksPacketXMem("Util.LogicalFs.PacketXMem.NumTasks", 4, Debug::Range(1, 16), Debug::Persist());

// OPTIMISATIONS TODO:
// Change Spu::Fetch & Spu::Store don't physically move data
// remove new & delete from Xenon/PPU inflate.

JQ_DECLARE_PPU_MODULE( XPakDeflateModule, Pak::C_PakCompressPacketXMem::DeflateTask )
JQ_DECLARE_PPU_MODULE( XPakInflateModule, Pak::C_PakCompressPacketXMem::InflateTask )
#define Z_FINISH 4
#define Z_NO_FLUSH 0
#define JQ_DEFAULT_WORKER_NUMBER_XENON 6

namespace Pak
{
	struct TaskData
	{
		C_AtomicJobList*	AtomicJobList;
		volatile int*		JobRunningCounter;
	};

U8			C_PakCompressPacketXMem::mDecompressCache[C_PAKCOMPRESSPACKETXMEM_NR_CACHES][PAKET_XMEM_BUFFER_SIZE];
bool		C_PakCompressPacketXMem::mDecompressCacheUsed[C_PAKCOMPRESSPACKETXMEM_NR_CACHES] = { 0 };
MT::Mutex	C_PakCompressPacketXMem::mDecompressCacheMutex;

void C_PakCompressPacketXMem::DeflateTask( Scheduler::Task* pTask )
{
	PERF_EVENT("PakPacketXMemDeflateTask");
	TaskData* taskData				= (TaskData*)pTask->ParamData;
	C_AtomicJobList* pJobList		= (C_AtomicJobList*)taskData->AtomicJobList;
#ifdef PLATFORM_SPU
	C_AtomicJobList* pAtomicJobList	= (C_AtomicJobList*)Spu::Alloc( sizeof( C_AtomicJobList ) );
	Spu::Fetch( pAtomicJobList, pJobList, sizeof( C_AtomicJobList ) );
#else
	C_AtomicJobList* pAtomicJobList	= pJobList;
#endif

	Pak::C_JobEntry job;
	

#ifdef PLATFORM_SPU
	void* pSourceBuffer	= Spu::Alloc( PAKET_XMEM_BUFFER_SIZE );
	void* pDestBuffer	= Spu::Alloc( PAKET_XMEM_BUFFER_SIZE );
#elif defined ( PLATFORM_PC )
	void* pDestBuffer	= alloca( PAKET_XMEM_BUFFER_SIZE * 2);
#else
	#define DEFLATE_ALLOCATED_WITH_NEW
	U8* pDestBuffer		= new U8[ PAKET_XMEM_BUFFER_SIZE ];
#endif
	while ( pAtomicJobList->PopJob( &job ) )
	{
		void* pSourceBuffer	= job.m_pSourceData;
		SIZE_T destSize = PAKET_XMEM_BUFFER_SIZE * 2;
		
#ifdef PLATFORM_SPU
		Spu::Fetch( pSourceBuffer, job.m_pSourceData, job.m_sSourceSize );
#endif

		HRESULT hr = E_FAIL;
		XMEMCOMPRESSION_CONTEXT context = NULL;
		S32 id = Scheduler::GetWorkerID() == 6 ? 0 : Scheduler::GetWorkerID();
		context = Compression::XMemCompressGetContext( id );
		XMemResetCompressionContext( context );
		hr = ::XMemCompress( context, pDestBuffer, &destSize, pSourceBuffer, job.m_sSourceSize );
	
		if( destSize >= PAKET_XMEM_BUFFER_SIZE )
		{
#ifdef PLATFORM_SPU
			Spu::Store( job.m_pSourceData, pSourceBuffer, PAKET_XMEM_BUFFER_SIZE );
#endif
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, -1 );
		}
		else
		{
			Spu::Store( job.m_pSourceData, pDestBuffer, PAKET_XMEM_BUFFER_SIZE );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, destSize );
		}

	}
	MT::AtomicDecrement( taskData->JobRunningCounter );
#ifdef DEFLATE_ALLOCATED_WITH_NEW
	delete [] pDestBuffer;
#endif
}

U8* C_PakCompressPacketXMem::AllocateDecompressCache( U32 *index )
{
	mDecompressCacheMutex.Wait();
	while(1)
	{
		for (U32 x = 0 ; x < C_PAKCOMPRESSPACKETXMEM_NR_CACHES ; x++)
		{
			if (mDecompressCacheUsed[x] == false)
			{
				mDecompressCacheUsed[x] = true;
				mDecompressCacheMutex.Release();
				*index = x;
				return &mDecompressCache[x][0];
			}
		}
		LOGINFO(g_log, "All Buffers Allocated");
		SwitchToThread();
	}
	return NULL; // Crash & Burn
}

void C_PakCompressPacketXMem::FreeDecompressCache( U32 index )
{
	mDecompressCacheMutex.Wait();
	BIZ_ASSERT(mDecompressCacheUsed[index]);
	mDecompressCacheUsed[index] = false;
	mDecompressCacheMutex.Release();
}

void C_PakCompressPacketXMem::InflateTask( Scheduler::Task* pTask )
{
	TaskData* taskData				= (TaskData*)pTask->ParamData;
	C_AtomicJobList* pJobList		= (C_AtomicJobList*)taskData->AtomicJobList;
#ifdef PLATFORM_SPU
	C_AtomicJobList* pAtomicJobList	= (C_AtomicJobList*)Spu::Alloc( sizeof( C_AtomicJobList ) );
	Spu::Fetch( pAtomicJobList, pJobList, sizeof( C_AtomicJobList ) );
#else
	C_AtomicJobList* pAtomicJobList	= pJobList;
#endif

	Pak::C_JobEntry job;
	
	U8* pDestBuffer		= NULL;

	while ( pAtomicJobList->PopJob( &job ) )
	{
		void* pSourceBuffer = job.m_pSourceData;
		if (job.m_sSourceSize < 0)
		{
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, PAKET_XMEM_BUFFER_SIZE );
		}
		else
		{
			SIZE_T destSize = PAKET_XMEM_BUFFER_SIZE;
			XMEMDECOMPRESSION_CONTEXT context = NULL;
	
			HRESULT hr = E_FAIL;
			S32 id = Scheduler::GetWorkerID();
			if (id == JQ_DEFAULT_WORKER_NUMBER_XENON)
			{
				// If this is executed from the calling thread, via a Flush from C_PakCompressPacketXMem::Read
				// it is necessary to have a unique context for the decompression. In the case
				// of simultaneous calls to the Inflate function.
				hr = XMemCreateDecompressionContext(XMEMCODEC_DEFAULT, NULL, 0, &context);
				BIZ_ASSERT(hr == S_OK);
			}
			else
			{
				context = Compression::XMemDecompressGetContext( id );
			}
			// Decompress the data from the pCompressedData buffer 
			// and copy it to the pDecompressedData buffer.
			U32 alloc_index;
			pDestBuffer = AllocateDecompressCache(&alloc_index);
			hr = XMemDecompress(context, pDestBuffer, &destSize, pSourceBuffer, job.m_sSourceSize);
			BIZ_ASSERT(hr == S_OK);
			// Destroy the decompression context.
			if (id == JQ_DEFAULT_WORKER_NUMBER_XENON)
			{
				XMemDestroyDecompressionContext( context );
			}
			Spu::Store( job.m_pSourceData, pDestBuffer, PAKET_XMEM_BUFFER_SIZE );
			FreeDecompressCache( alloc_index );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, destSize );
		}
		
	}
	MT::AtomicDecrement( taskData->JobRunningCounter );

#ifdef INFLATE_ALLOCATED_WITH_NEW
	delete [] pDestBuffer;
#endif

}

#ifndef PLATFORM_SPU

C_PakCompressPacketXMem::C_PakCompressPacketXMem(U32 flags)
{
	BIZ_ASSERT (GetPacketCompressionPolicy() + 1);

	m_pMediator = 0;
	m_physicalStartPos = 0;
	m_physicalReadPos = 0;
	m_fileAccessAttributes = 0;
	m_totalWritten = 0;
	//Physical size of compressed data.
	m_dataPhysicalLength = 0;

	m_PacketSize = 0;
	
	m_Group	= Scheduler::CreateGroup( false );

	m_Buffer = NULL;
	m_JobRunningCounter = 0;
	
#ifdef CFG_CONVERTER
	m_NrParallelTasks = 4;
	U32 nrBuffers = 64;
#else
	m_NrParallelTasks = g_numTasksPacketXMem;
	U32 nrBuffers = g_numBuffersPacketXMem;
#endif
	mAtomicJobList = new C_AtomicJobList();
	mAtomicJobList->Initialise( nrBuffers, PAKET_XMEM_BUFFER_SIZE );
}



bool C_PakCompressPacketXMem::PendingJobs()
{
	Scheduler::Flush( m_Group );
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressPacketXMem::PendingWrites()
{
	// TODO
	C_JobEntry job;
	while (mAtomicJobList->GetCompleteJob( &job ))
	{
		if (job.m_sSourceSize < 0)
		{
			m_totalWritten += sizeof( job.m_sSourceSize );
			m_pMediator->Write((U32)job.m_sSourceSize );
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U32)job.m_sSourceSize, m_pMediator->GetEndianSwapFlag());
			m_totalWritten += PAKET_XMEM_BUFFER_SIZE;
			m_pMediator->Write(job.m_pSourceData, PAKET_XMEM_BUFFER_SIZE);
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U8*)job.m_pSourceData, PAKET_XMEM_BUFFER_SIZE);
			mAtomicJobList->SetJobUnused( job.m_uJobIndex );
			delete [] job.m_pSourceData;
		}
		else
		{
			m_totalWritten += sizeof( job.m_sSourceSize );
			m_pMediator->Write( (U32)job.m_sSourceSize );
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U32)job.m_sSourceSize, m_pMediator->GetEndianSwapFlag());
			m_totalWritten += job.m_sSourceSize;
			m_pMediator->Write(job.m_pSourceData, job.m_sSourceSize);
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U8*)job.m_pSourceData, job.m_sSourceSize);
			mAtomicJobList->SetJobUnused( job.m_uJobIndex );
			delete [] job.m_pSourceData;
		}
	}

	return false;
}
#endif

C_PakCompressPacketXMem::~C_PakCompressPacketXMem()
{
	Scheduler::DestroyGroup( m_Group );
	
	SAFE_DELETE_PTR( mAtomicJobList );
}

bool C_PakCompressPacketXMem::Open(
	C_PakIoMediator* pMediator, 
	Sys::C_StreamPos physicalStartPos, 
	const U32 fileAccessAttributes,
	Sha1::C_HashContext* pHashContext)
{
	m_physicalStartPos = physicalStartPos;
	m_physicalReadPos = m_physicalStartPos;
	m_pMediator = pMediator;
	m_fileAccessAttributes = fileAccessAttributes;

	m_pHashContext = pHashContext;

	switch(m_fileAccessAttributes)
	{
		case Sys::E_FileAccess_Read:
			m_BufferPosition = PAKET_XMEM_BUFFER_SIZE;
			m_pMediator->Read(m_dataPhysicalLength, &m_physicalReadPos);
			m_Buffer = NULL;
			m_dataPhysicalRead = sizeof(m_PacketSize) + sizeof(m_dataPhysicalLength);
			return m_pMediator->Read(m_PacketSize, &m_physicalReadPos);
#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			//Write space for physical length
			m_Buffer = new U8[ PAKET_XMEM_BUFFER_SIZE ];
			m_totalWritten = 0;
			m_pMediator->SetWritePosition(physicalStartPos);
			memset(m_Buffer,0x00,PAKET_XMEM_BUFFER_SIZE);
			m_BufferPosition = 0;
			m_pMediator->Write(m_dataPhysicalLength);
			m_PacketSize = PAKET_XMEM_BUFFER_SIZE;
			m_totalWritten += sizeof( m_PacketSize );
			if (m_pHashContext)
				m_pHashContext->UpdateHash(m_PacketSize, m_pMediator->GetEndianSwapFlag());
			return m_pMediator->Write( m_PacketSize );
#endif
		default:
			break;
	}
		
	LOGERROR(g_log, "Failed to open XMem compressed pak!");
	return false;
}

bool C_PakCompressPacketXMem::Restart(U32 streamToLogicalPos)
{
	m_physicalReadPos = m_physicalStartPos;
	{
		// Read In The Length
		if (!m_pMediator->Read(m_dataPhysicalLength, &m_physicalReadPos))
			return false;
		if (!m_pMediator->Read(m_PacketSize, &m_physicalReadPos))
			return false;
	}

	if (m_Buffer)
	{
		mAtomicJobList->SetJobUnused( m_BufferJobIndex );
		mAtomicJobList->GetPakBuffer()->FreeBuffer( m_Buffer );
		m_Buffer = NULL;
	}

	//Has to be at least of PAKET_XMEM_BUFFER_SIZE and this is currently so large than it needs to be on the heap.
	//Having a fixed size buffer of say, 8k and then trying to read that would fail due to a m_buffer ptr not being allocated
	//in the read function. This needs looking at.
	U8* pDestBuffer	= new U8[ PAKET_XMEM_BUFFER_SIZE ];
	m_BufferPosition = PAKET_XMEM_BUFFER_SIZE;
	m_dataPhysicalRead = sizeof(m_PacketSize) + sizeof(m_dataPhysicalLength);
	// Stream in to logicalPosition
	U32 bytesRemaining = streamToLogicalPos;
	while (bytesRemaining)
	{
		U32 nrToRead = bytesRemaining > PAKET_XMEM_BUFFER_SIZE ? PAKET_XMEM_BUFFER_SIZE : bytesRemaining;
		Read(pDestBuffer, nrToRead);
		bytesRemaining -= nrToRead;
	}
	SAFE_DELETE_ARY(pDestBuffer);

	return true;
}

U32 C_PakCompressPacketXMem::GetPhysicalLength() const
{
	return m_dataPhysicalLength;
}

bool C_PakCompressPacketXMem::Close()
{
	switch(m_fileAccessAttributes)
	{
		case Sys::E_FileAccess_Read:
			if (m_Buffer)
			{
				mAtomicJobList->SetJobUnused( m_BufferJobIndex );
				mAtomicJobList->GetPakBuffer()->FreeBuffer( m_Buffer );
				m_Buffer = NULL;
			}
			return true;

#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			
			// Flush all Jobs
			Flush(Z_FINISH);
			// Wait for pending jobs
			PendingJobs();
			// Write all data
			PendingWrites();
			// 
			U8 zeroBuffer[PAK_SECTOR_SIZE_MAX] = {0};
			//The U32 term below is the physical length.
			U32 sectorSize = m_pMediator->GetSectorSize();
			//Calculate the sector padding including initial 4 bytes used for the compressed data size
			U32 nrToWrite = sectorSize - ((m_totalWritten + sizeof(m_dataPhysicalLength)) % sectorSize);
			m_pMediator->Write((U8*)zeroBuffer, nrToWrite);
			//m_totalWritten += nrToWrite;
			m_dataPhysicalLength = m_totalWritten;
			//Write physical length.
			{
				Sys::C_StreamPos pos = m_pMediator->GetWritePosition();
				m_pMediator->SetWritePosition(m_physicalStartPos);
				m_pMediator->Write(m_dataPhysicalLength);
				m_pMediator->SetWritePosition(pos);
			}
			delete [] m_Buffer;
			return true;
#endif
	}

	LOGERROR(g_log, "Failed to close packet XMem compressed pak");
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressPacketXMem::Flush(int flush)
{
	Scheduler::PolicyHandle policy = GetPacketCompressionPolicy();

	C_JobEntry job;
	job.m_pSourceData = new U8[ PAKET_XMEM_BUFFER_SIZE ];
	if (!job.m_pSourceData) return false;
	job.m_sSourceSize = m_BufferPosition;
	memcpy( job.m_pSourceData, m_Buffer, m_BufferPosition );
	
	while ( !mAtomicJobList->PushJob( &job ) )
	{
//		Scheduler::Flush( m_Group );
		Sleep(1);
		PendingWrites();
	}
	if ((mAtomicJobList->ProcessJobs()) || flush )
	{
		TaskData data;	
		data.AtomicJobList	= &mAtomicJobList;
		data.JobRunningCounter = &m_JobRunningCounter;
		for (int x = 0 ; x < m_NrParallelTasks ; x++)
		{
			int nrJobsRunning = MT::AtomicGet( &m_JobRunningCounter );
			if (nrJobsRunning < m_NrParallelTasks )
			{
				MT::AtomicIncrement( &m_JobRunningCounter );
				Scheduler::AddTask(XPakDeflateModule, 0, 0, m_Group, policy, &data, sizeof(data));
			}
			else
			{
				break;
			}
		}
	}

	return true;
}

bool C_PakCompressPacketXMem::Write(U8* buffer, U32 nrBytes)
{
	// Write out any pending data
	PendingWrites();

	// 

	// Copy data into buffer
	U32 nrBytesRemaining = nrBytes;
	while ( nrBytesRemaining )
	{
		U32 nrBytesToCopy;
		if ( nrBytesRemaining > (PAKET_XMEM_BUFFER_SIZE - m_BufferPosition) )
		{
			nrBytesToCopy = (PAKET_XMEM_BUFFER_SIZE - m_BufferPosition);
		}
		else
		{
			nrBytesToCopy = nrBytesRemaining;
		}
		memcpy( m_Buffer + m_BufferPosition, buffer, nrBytesToCopy );
		nrBytesRemaining -= nrBytesToCopy;
		BIZ_ASSERT ( nrBytesRemaining < PAKET_XMEM_BUFFER_SIZE );
		m_BufferPosition += nrBytesToCopy;
		if (m_BufferPosition == PAKET_XMEM_BUFFER_SIZE)
		{
			if (!Flush( Z_NO_FLUSH )) return false;
			m_BufferPosition = 0;
		}
	}
	

	return true;
}
#endif

#if !defined( PLATFORM_SPU ) 
void C_PakCompressPacketXMem::StartInflateTasks()
{
	Scheduler::PolicyHandle policy = GetPacketCompressionPolicy();

	// Start Processes 
	TaskData data;	
	data.AtomicJobList	= mAtomicJobList;
	data.JobRunningCounter = &m_JobRunningCounter;
	for (int x = 0 ; x < m_NrParallelTasks ; x++)
	{
		int nrJobsRunning = MT::AtomicGet( &m_JobRunningCounter );
		if (nrJobsRunning < m_NrParallelTasks )
		{
			MT::AtomicIncrement( &m_JobRunningCounter );
			Scheduler::AddTask(XPakInflateModule, 0, 0, m_Group, policy, &data, sizeof(data));
		}
		else
		{
			break;
		}
	}
}

bool C_PakCompressPacketXMem::Read(U8* buffer, U32 nrBytes)
{
	if (nrBytes == 0)
		return true;

	bool endOfStream = m_dataPhysicalRead >= m_dataPhysicalLength;


	// Get some data to return			
	U32 nrBytesRemaining = nrBytes;
	while (nrBytesRemaining)
	{
		// Read Ahead & Store
		while (mAtomicJobList->CanPush() && (!endOfStream))
		{
			C_JobEntry job;
			U32 packetSize = 0;

			job.m_pSourceData = mAtomicJobList->GetPakBuffer()->AllocateBuffer();// new U8[ PAKET_XMEM_BUFFER_SIZE ];
			if  (!job.m_pSourceData) break;

			// read in the packet size
			m_pMediator->Read(packetSize, &m_physicalReadPos);
			// if the packet size is -1 then the packet was not z-lib'ed (saves 20bytes per packet)
			U32 dataSize = (packetSize == -1) ? PAKET_XMEM_BUFFER_SIZE : packetSize;
			// Set the Job Packet Size		
			job.m_sSourceSize = packetSize;
			// read in the packet data
			m_pMediator->Read(job.m_pSourceData, dataSize, &m_physicalReadPos);
			// Update the number of Physical Read Bytes
			m_dataPhysicalRead += sizeof(packetSize)+dataSize;
			// Push the Job onto the Job Queue
			mAtomicJobList->PushJob( &job );
			// Check for end of the stream
			if (m_dataPhysicalRead >= m_dataPhysicalLength)
			{
				endOfStream = true;
				break;
			}
		}

		if (mAtomicJobList->ProcessJobs() || endOfStream)
		{
			StartInflateTasks();
		}
					
		U32 nrBytesToCopy;
		if (m_BufferPosition >= PAKET_XMEM_BUFFER_SIZE)
		{
			C_JobEntry job;
			if (m_Buffer)
			{
				mAtomicJobList->SetJobUnused( m_BufferJobIndex );
				mAtomicJobList->GetPakBuffer()->FreeBuffer( m_Buffer );
				m_Buffer = NULL;
			}
			while (mAtomicJobList->PendingJobs())
			{
				StartInflateTasks();
				Scheduler::Flush( m_Group );
			}
			if (mAtomicJobList->GetCompleteJob( &job ))
			{
				m_Buffer = (U8*)job.m_pSourceData;
				m_BufferPosition = 0;
				m_BufferJobIndex = job.m_uJobIndex;
			}
			else
			{
				if (endOfStream) return true;
			}
		}
	
		// Copy into dest buffer
		if ( nrBytes > (PAKET_XMEM_BUFFER_SIZE - m_BufferPosition) )
		{
			nrBytesToCopy = (PAKET_XMEM_BUFFER_SIZE - m_BufferPosition);
		}
		else
		{
			nrBytesToCopy = nrBytes;
		}
		memcpy( buffer, m_Buffer + m_BufferPosition, nrBytesToCopy );
		m_BufferPosition	+= nrBytesToCopy;
		nrBytesRemaining	-= nrBytesToCopy;
	}
	
//	mAtomicJobList->DumpStats();

	return true;
}
#endif
#endif

}
