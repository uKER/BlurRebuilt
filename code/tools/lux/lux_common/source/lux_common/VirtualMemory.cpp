#include "stdafx.h"
#include "lux_common/VirtualMemory.h"

// virtual memory manager
// allocate a large memory-mapped file
// any allocations of memory mapped blocks will be stuck sequentially into the file and then
// paged in and out when needed
CVirtualMemoryMgr	CVirtualMemoryMgr::m_Mgr;
HANDLE				CVirtualMemoryMgr::m_hFile;
HANDLE				CVirtualMemoryMgr::m_hMap;
int					CVirtualMemoryMgr::m_iAllocGranularity;

// get pointer to singleton
CVirtualMemoryMgr *CVirtualMemoryMgr::GetVirtualMemoryMgr()
{
	return &m_Mgr;
}

// initialize a temporary file which will contain all our memory blocks
CVirtualMemoryMgr::CVirtualMemoryMgr()
{
	char	szTmpFile[_MAX_PATH];

	// Create temporary	file for mapping
	GetTempPath	(256, szTmpFile);
	GetTempFileName	(szTmpFile,
		"LightMapper\\",
		0,
		m_pszFilename);

	// If file created,	continue to	map	file
	m_hFile =
		CreateFile (m_pszFilename,
		GENERIC_WRITE |	GENERIC_READ,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);

	// create initial mapping of one byte
	m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 1, NULL);

	m_lTotalSize = 0;

	// get allocation granularity
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	m_iAllocGranularity = sysInfo.dwAllocationGranularity;
}

// remove all allocated memory blocks
void CVirtualMemoryMgr::Purge()
{
	m_lTotalSize = 0;

	for (int i=0; i<m_Blocks.Size(); i++)
		delete m_Blocks[i];
}

// delete all memory
CVirtualMemoryMgr::~CVirtualMemoryMgr()
{
	Purge();

	CloseHandle(m_hMap);
	CloseHandle(m_hFile);
}

// allocate a new block of data - returns index of the block
CVirtualMemoryBlock *CVirtualMemoryMgr::Alloc(int iSize)
{
	return NULL; // DISABLE VMM FOR NOW

	// create a new memory block and add it to thelist
	CVirtualMemoryBlock *pNewBlock = new CVirtualMemoryBlock(iSize);
	pNewBlock->m_lOffset = m_lTotalSize;
	pNewBlock->m_iSize	 = iSize;
	m_Blocks.PushBack(pNewBlock);

	// expand our virtual memory file
	m_lTotalSize += iSize;
	CloseHandle(m_hMap);
	m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, m_lTotalSize>>32, m_lTotalSize&0xffffffff, NULL);

	return pNewBlock;
}


// class for one block of paged memory
CVirtualMemoryBlock::CVirtualMemoryBlock(int iSize)
{
	m_iSize = iSize;
	m_pData = NULL;
}

// release
CVirtualMemoryBlock::~CVirtualMemoryBlock()
{
	Release();
}

// Map a view of this file into memory
// we have to do some extra work to make the mapped view aligned with the vmm alloc granularity
void *CVirtualMemoryBlock::Lock()
{
	if (!m_pData)
	{
		int iLoOffset = m_lOffset&0xffffffff;
		int iDiff = iLoOffset & (CVirtualMemoryMgr::m_iAllocGranularity-1);
		int iAlignedSize = ((m_iSize-1) | (CVirtualMemoryMgr::m_iAllocGranularity-1))+1;

		m_pBlockData = MapViewOfFile (CVirtualMemoryMgr::m_hMap, FILE_MAP_WRITE, m_lOffset>>32, iLoOffset-iDiff, m_iSize+iDiff);
		m_pData = (char*)m_pBlockData+iDiff;
	}

	return m_pData; // return mapped address
}

// release mapped memory
void CVirtualMemoryBlock::Release()
{
	if (m_pData)
	{
		UnmapViewOfFile(m_pBlockData);
		m_pData = NULL;
		m_pBlockData = NULL;
	}
}
