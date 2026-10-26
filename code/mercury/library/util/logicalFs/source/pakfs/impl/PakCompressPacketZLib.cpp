// Test cmd lines
// Pak   -c c -f z:\models.pak -i z:\ModelsPakContents -r -p / -m PacketZLib -z z:\ModelsPakContents -b
// UNPAK -c x -f z:\models.pak -o z:\DeModelsPakContents -b

#include "LogicalFsPch.h"

#include <system/Perf.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressPacketZLib.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/PakGlobals.h>

//Zlib
#include <compression.h>
#include <tools_shared/SchedulerConfig.h>

#include <System/Debug/Debug.h>
#ifndef PLATFORM_SPU
Debug::Int g_numBuffersPacketZLib("Util.LogicalFs.PacketZLib.NumBuffers", 4, Debug::Range(1, 160), Debug::Persist());
Debug::Int g_numTasksPacketZLib("Util.LogicalFs.PacketZLib.NumTasks", 4, Debug::Range(1, 16), Debug::Persist());
#else
U32 g_numBuffersPacketZLib = 4;
U32 g_numTasksPacketZLib = 4;
#endif

#ifdef PLATFORM_PS3
#include <sys/timer.h>
#endif

// OPTIMISATIONS TODO:
// Change Xenon/PPU & PC versions such that Spu::Fetch & Spu::Store don't physically move data
// remove new & delete from Xenon/PPU inflate.
// #define USE_PPU_ONLY

#if defined( PLATFORM_SPU ) || defined( PLATFORM_PS3 ) 

#if (defined( USE_PPU_ONLY ) && (!defined(DEFLATE_MODULE)) && (!defined(INFLATE_MODULE)))
	#define INFLATE_MODULE
	#define DEFLATE_MODULE
	JQ_DECLARE_PPU_MODULE( PakDeflateModule, Pak::C_PakCompressPacketZLib::DeflateTask )
	JQ_DECLARE_PPU_MODULE( PakInflateModule, Pak::C_PakCompressPacketZLib::InflateTask )
#else
	#if defined DEFLATE_MODULE || defined( PLATFORM_PS3 ) 
	JQ_DECLARE_DUAL_MODULE( PakDeflateModule, Pak::C_PakCompressPacketZLib::DeflateTask, PakDeflate, "Util.Pak")
	#endif
	#if defined INFLATE_MODULE || defined( PLATFORM_PS3 ) 
	JQ_DECLARE_SPU_MODULE( PakInflateModule, Pak::C_PakCompressPacketZLib::InflateTask, PakInflate, "Util.Pak")
	#endif
#endif

#else
#define INFLATE_MODULE
#define DEFLATE_MODULE
JQ_DECLARE_PPU_MODULE( PakDeflateModule, Pak::C_PakCompressPacketZLib::DeflateTask )
JQ_DECLARE_PPU_MODULE( PakInflateModule, Pak::C_PakCompressPacketZLib::InflateTask )
#endif

namespace Pak
{

	struct TaskData
	{
		C_AtomicJobList*	AtomicJobList;
		volatile int*		JobRunningCounter;
	};

void C_PakCompressPacketZLib::DeflateTask( Scheduler::Task* pTask )
{
	PERF_EVENT("PakPacketZLibDeflateTask");
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
	void* pSourceBuffer	= Spu::Alloc( PAKET_ZLIB_BUFFER_SIZE );
	void* pDestBuffer	= Spu::Alloc( PAKET_ZLIB_BUFFER_SIZE );
#else
	void* pSourceBuffer	= alloca( PAKET_ZLIB_BUFFER_SIZE );
	void* pDestBuffer	= alloca( PAKET_ZLIB_BUFFER_SIZE * 2 );
#endif
	while ( pAtomicJobList->PopJob( &job ) )
	{
		Spu::Fetch( pSourceBuffer, job.m_pSourceData, job.m_sSourceSize );
	
		z_stream zstream;
		
		zstream.zalloc		= NULL;
		zstream.zfree		= NULL;
		zstream.opaque		= NULL;
		zstream.next_in		= (Bytef*)pSourceBuffer;
		zstream.avail_in	= job.m_sSourceSize;
		zstream.next_out	= (Bytef*)pDestBuffer;
		zstream.avail_out	= PAKET_ZLIB_BUFFER_SIZE * 2;

		int deflateResult = deflateInit2(&zstream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY ); 
		deflateResult = deflate(&zstream, Z_FINISH );
		deflateEnd(&zstream);

		
		if( zstream.total_out >= PAKET_ZLIB_BUFFER_SIZE )
		{
			
			Spu::Store( job.m_pSourceData, pSourceBuffer, PAKET_ZLIB_BUFFER_SIZE );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, -1 );
		}
		else
		{
			Spu::Store( job.m_pSourceData, pDestBuffer, PAKET_ZLIB_BUFFER_SIZE );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, zstream.total_out );
		}

	}
	MT::AtomicDecrement( taskData->JobRunningCounter );
}

void C_PakCompressPacketZLib::InflateTask( Scheduler::Task* pTask )
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
	
#ifdef PLATFORM_SPU
	void* pSourceBuffer	= Spu::Alloc( PAKET_ZLIB_BUFFER_SIZE );
	void* pDestBuffer	= Spu::Alloc( PAKET_ZLIB_BUFFER_SIZE );
#elif defined ( PLATFORM_PC )
	void* pSourceBuffer	= alloca( PAKET_ZLIB_BUFFER_SIZE );
	void* pDestBuffer	= alloca( PAKET_ZLIB_BUFFER_SIZE );
#else
	#define ALLOCATED_WITH_NEW
	U8* pSourceBuffer	= new U8[ PAKET_ZLIB_BUFFER_SIZE ];
	U8* pDestBuffer		= new U8[ PAKET_ZLIB_BUFFER_SIZE ];
#endif
	while ( pAtomicJobList->PopJob( &job ) )
	{
		if (job.m_sSourceSize < 0)
		{
			Spu::Fetch( pSourceBuffer, job.m_pSourceData, PAKET_ZLIB_BUFFER_SIZE );
			Spu::Store( job.m_pSourceData, pSourceBuffer, PAKET_ZLIB_BUFFER_SIZE );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, PAKET_ZLIB_BUFFER_SIZE );
		}
		else
		{
			Spu::Fetch( pSourceBuffer, job.m_pSourceData, job.m_sSourceSize );
	
			z_stream zstream;
			
			zstream.zalloc		= NULL;
			zstream.zfree		= NULL;
			zstream.opaque		= NULL;
			zstream.next_in		= (Bytef*)pSourceBuffer;
			zstream.avail_in	= job.m_sSourceSize;
			zstream.next_out	= (Bytef*)pDestBuffer;
			zstream.avail_out	= PAKET_ZLIB_BUFFER_SIZE;

			int inflateResult = inflateInit2(&zstream, -15);
			inflateResult = inflate(&zstream, Z_FINISH );
			inflateEnd(&zstream);
			Spu::Store( job.m_pSourceData, pDestBuffer, PAKET_ZLIB_BUFFER_SIZE );
			pAtomicJobList->SetJobComplete( job.m_uJobIndex, zstream.total_out );
		}
		
	}
	MT::AtomicDecrement( taskData->JobRunningCounter );

#ifdef ALLOCATED_WITH_NEW
	SAFE_DELETE_ARY(pSourceBuffer);
	SAFE_DELETE_ARY(pDestBuffer);
#endif

}

#ifndef PLATFORM_SPU

C_PakCompressPacketZLib::C_PakCompressPacketZLib(U32 flags)
{
	BIZ_ASSERT (GetPacketCompressionPolicy() + 1);

	m_pMediator = 0;
	m_physicalStartPos = 0;
	m_physicalReadPos = 0;
	m_fileAccessAttributes = 0;
	m_totalWritten = 0;
	//Physical size of compressed data.
	m_dataPhysicalLength = 0;
	if (flags == 0)
		m_compress = Z_BEST_COMPRESSION;
	else
		m_compress = flags;

	m_PacketSize = 0;
	
	m_Group	= Scheduler::CreateGroup( false );

	m_Buffer = NULL;
	m_JobRunningCounter = 0;
	
#ifdef CFG_CONVERTER
	m_NrParallelTasks = 4;
	U32 nrBuffers = 64;
#else
	m_NrParallelTasks = g_numTasksPacketZLib;
	U32 nrBuffers = g_numBuffersPacketZLib;
#endif
	mAtomicJobList = new C_AtomicJobList();
	mAtomicJobList->Initialise( nrBuffers, PAKET_ZLIB_BUFFER_SIZE );
}



bool C_PakCompressPacketZLib::PendingJobs()
{
	Scheduler::Flush( m_Group );
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressPacketZLib::PendingWrites()
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
			m_totalWritten += PAKET_ZLIB_BUFFER_SIZE;
			m_pMediator->Write(job.m_pSourceData, PAKET_ZLIB_BUFFER_SIZE);
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U8*)job.m_pSourceData, PAKET_ZLIB_BUFFER_SIZE);
			mAtomicJobList->SetJobUnused( job.m_uJobIndex );
			SAFE_DELETE_ARY(job.m_pSourceData);
		}
		else
		{
			m_totalWritten += sizeof( job.m_sSourceSize );
			m_pMediator->Write((U32)job.m_sSourceSize );
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U32)job.m_sSourceSize, m_pMediator->GetEndianSwapFlag());
			m_totalWritten += job.m_sSourceSize;
			m_pMediator->Write(job.m_pSourceData, job.m_sSourceSize);
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U8*)job.m_pSourceData, job.m_sSourceSize);
			mAtomicJobList->SetJobUnused( job.m_uJobIndex );
			SAFE_DELETE_ARY(job.m_pSourceData);
		}
	}

	return false;
}
#endif

C_PakCompressPacketZLib::~C_PakCompressPacketZLib()
{
	Scheduler::DestroyGroup( m_Group );
	
	SAFE_DELETE_PTR( mAtomicJobList );
}

bool C_PakCompressPacketZLib::Open(
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
			m_BufferPosition = PAKET_ZLIB_BUFFER_SIZE;
			ReadPhysicalLength(m_pMediator, m_dataPhysicalLength, &m_physicalReadPos);	// [Blur2PC]
			m_Buffer = NULL;
			m_dataPhysicalRead = sizeof(m_PacketSize) + PrefixBytes();
			return m_pMediator->Read(m_PacketSize, &m_physicalReadPos);
#ifdef CFG_CONVERTER
		case Sys::E_FileAccess_Write:
			//Write space for physical length
			m_Buffer = new U8[ PAKET_ZLIB_BUFFER_SIZE ];
			m_totalWritten = 0;
			m_pMediator->SetWritePosition(physicalStartPos);
			memset(m_Buffer,0x00,PAKET_ZLIB_BUFFER_SIZE);
			m_BufferPosition = 0;
			m_pMediator->Write(m_dataPhysicalLength);
			m_PacketSize = PAKET_ZLIB_BUFFER_SIZE;
			m_totalWritten += sizeof( m_PacketSize );
			if (m_pHashContext)
				m_pHashContext->UpdateHash(m_PacketSize, m_pMediator->GetEndianSwapFlag());
			return m_pMediator->Write( m_PacketSize );
#endif
		default:
			break;
	}
		
	LOGERROR(g_log, "Failed to open ZLib compressed pak!");
	return false;
}

bool C_PakCompressPacketZLib::Restart(U32 streamToLogicalPos)
{
	m_physicalReadPos = m_physicalStartPos;
	{
		// Read In The Length
		if (!ReadPhysicalLength(m_pMediator, m_dataPhysicalLength, &m_physicalReadPos))	// [Blur2PC]
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

	//Has to be at least of PAKET_ZLIB_BUFFER_SIZE and this is currently so large than it needs to be on the heap.
	//Having a fixed size buffer of say, 8k and then trying to read that would fail due to a m_buffer ptr not being allocated
	//in the read function. This needs looking at.
	U8* pDestBuffer	= new U8[ PAKET_ZLIB_BUFFER_SIZE ];
	m_BufferPosition = PAKET_ZLIB_BUFFER_SIZE;
	m_dataPhysicalRead = sizeof(m_PacketSize) + PrefixBytes();	// [Blur2PC]

	// Stream in to logicalPosition
	U32 bytesRemaining = streamToLogicalPos;
	while (bytesRemaining)
	{
		U32 nrToRead = bytesRemaining > PAKET_ZLIB_BUFFER_SIZE ? PAKET_ZLIB_BUFFER_SIZE : bytesRemaining;
		Read(pDestBuffer, nrToRead);
		bytesRemaining -= nrToRead;
	}
	SAFE_DELETE_ARY(pDestBuffer);

	return true;
}

U32 C_PakCompressPacketZLib::GetPhysicalLength() const
{
	return m_dataPhysicalLength;
}

bool C_PakCompressPacketZLib::Close()
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
			SAFE_DELETE_ARY(m_Buffer);
			return true;
#endif
	}

	LOGERROR(g_log, "Failed to close packet ZLib compressed pak");
	return false;
}

#ifdef CFG_CONVERTER
bool C_PakCompressPacketZLib::Flush(int flush)
{
	Scheduler::PolicyHandle policy = GetPacketCompressionPolicy();

	C_JobEntry job;
	job.m_pSourceData = new U8[ PAKET_ZLIB_BUFFER_SIZE ];
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
				Scheduler::AddTask(PakDeflateModule, 0, 0, m_Group, policy, &data, sizeof(data));
			}
			else
			{
				break;
			}
		}
	}

	return true;
}

bool C_PakCompressPacketZLib::Write(U8* buffer, U32 nrBytes)
{
	// Write out any pending data
	PendingWrites();

	// 

	// Copy data into buffer
	U32 nrBytesRemaining = nrBytes;
	while ( nrBytesRemaining )
	{
		U32 nrBytesToCopy;
		if ( nrBytesRemaining > (PAKET_ZLIB_BUFFER_SIZE - m_BufferPosition) )
		{
			nrBytesToCopy = (PAKET_ZLIB_BUFFER_SIZE - m_BufferPosition);
		}
		else
		{
			nrBytesToCopy = nrBytesRemaining;
		}
		memcpy( m_Buffer + m_BufferPosition, buffer, nrBytesToCopy );
		nrBytesRemaining -= nrBytesToCopy;
		BIZ_ASSERT ( nrBytesRemaining < PAKET_ZLIB_BUFFER_SIZE );
		m_BufferPosition += nrBytesToCopy;
		if (m_BufferPosition == PAKET_ZLIB_BUFFER_SIZE)
		{
			if (!Flush( Z_NO_FLUSH )) return false;
			m_BufferPosition = 0;
		}
	}
	

	return true;
}
#endif

#if !defined( PLATFORM_SPU ) 
void C_PakCompressPacketZLib::StartInflateTasks()
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
			Scheduler::AddTask(PakInflateModule, 0, 0, m_Group, policy, &data, sizeof(data));
		}
		else
		{
			break;
		}
	}
}

bool C_PakCompressPacketZLib::Read(U8* buffer, U32 nrBytes)
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

			job.m_pSourceData = mAtomicJobList->GetPakBuffer()->AllocateBuffer();// new U8[ PAKET_ZLIB_BUFFER_SIZE ];
			if  (!job.m_pSourceData) break;

			// read in the packet size
			m_pMediator->Read(packetSize, &m_physicalReadPos);

			// update the hash
			if (m_pHashContext)
				m_pHashContext->UpdateHash(packetSize, m_pMediator->GetEndianSwapFlag());

			// if the packet size is -1 then the packet was not z-lib'ed (saves 20bytes per packet)
			U32 dataSize = (packetSize == -1) ? PAKET_ZLIB_BUFFER_SIZE : packetSize;
			// Set the Job Packet Size		
			job.m_sSourceSize = packetSize;
			// read in the packet data
			m_pMediator->Read(job.m_pSourceData, dataSize, &m_physicalReadPos);

			// update the hash
			if (m_pHashContext)
				m_pHashContext->UpdateHash((U8*)job.m_pSourceData, dataSize);

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
		if (m_BufferPosition >= PAKET_ZLIB_BUFFER_SIZE)
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
		if ( nrBytes > (PAKET_ZLIB_BUFFER_SIZE - m_BufferPosition) )
		{
			nrBytesToCopy = (PAKET_ZLIB_BUFFER_SIZE - m_BufferPosition);
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
