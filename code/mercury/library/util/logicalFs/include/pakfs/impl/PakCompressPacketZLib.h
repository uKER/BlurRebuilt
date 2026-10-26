#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_PACKET_ZLIB_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_PACKET_ZLIB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler.h>
#include "PakAtomicJobList.h"
#define PAKET_ZLIB_BUFFER_SIZE (64 * 1024)

namespace Pak
{


#define MAX_NR_ZLIB_ENTRIES	32


class C_PakCompressPacketZLib : public I_PakCompress
{
	C_AtomicJobList*	mAtomicJobList;
public:
	C_PakCompressPacketZLib( U32 flags );
	virtual ~C_PakCompressPacketZLib();
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
};

}

#endif
