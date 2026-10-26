#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_PACKET_XMEM_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_PACKET_XMEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler.h>
#include "PakAtomicJobList.h"
#define PAKET_XMEM_BUFFER_SIZE (64 * 1024)

namespace Pak
{


#define MAX_NR_XMEM_ENTRIES	32


class C_PakCompressPacketXMem : public I_PakCompress
{
	C_AtomicJobList*	mAtomicJobList __attribute__ ((aligned (128))); // Static? i.e. one global job list? or local 1 job list per pak-file?

	#define				C_PAKCOMPRESSPACKETXMEM_NR_CACHES 6
	static U8			mDecompressCache[C_PAKCOMPRESSPACKETXMEM_NR_CACHES][PAKET_XMEM_BUFFER_SIZE];
	static bool			mDecompressCacheUsed[C_PAKCOMPRESSPACKETXMEM_NR_CACHES];
	static MT::Mutex	mDecompressCacheMutex;

public:
	C_PakCompressPacketXMem( U32 flags );
	virtual ~C_PakCompressPacketXMem();
	virtual bool Open(
		C_PakIoMediator* m_pPakRWOptimiser,
		Sys::C_StreamPos physicalStartPos, 
		const U32 fileAccessAttributes = Sys::E_FileAccess_Read,
		Sha1::C_HashContext* pHashContext = NULL);
	virtual bool Close();
#ifdef CFG_CONVERTER
	virtual bool Write(U8* buffer, U32 nrBytes);
#endif
	void StartInflateTasks();
	virtual bool Read(U8* buffer, U32 nrBytes);
	virtual bool Restart(U32 streamToLogicalPos);
	virtual U32 GetPhysicalLength() const;

	static void DeflateTask( Scheduler::Task* pTask );
	static void InflateTask( Scheduler::Task* pTask );
	
private:
	bool Flush( int flush );
	bool PendingJobs();
	bool PendingWrites();

	C_PakIoMediator*		m_pMediator;
	Sys::C_StreamPos		m_physicalStartPos;
	U32						m_fileAccessAttributes;
	U32						m_totalWritten;
	//Physical size of compressed data.
	U32						m_dataPhysicalLength;
	U32						m_dataPhysicalRead;
	//Location of physical data size value.
	U32						m_compress;
	Sys::C_StreamPos		m_physicalReadPos;
	U8*						m_Buffer;
	U32						m_BufferPosition;
	U32						m_BufferJobIndex;
	U32						m_PacketSize;
	int						m_NrParallelTasks;
	Scheduler::GroupHandle	m_Group;
	volatile int			m_JobRunningCounter;
	static U8*				AllocateDecompressCache( U32 *index );
	static void				FreeDecompressCache( U32 index );

};

}

#endif
