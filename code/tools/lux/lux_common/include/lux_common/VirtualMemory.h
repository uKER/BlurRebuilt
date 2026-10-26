#pragma once

#define MAX_BLOCKS 100000
#define MAX_VMEM_SIZE 2 // in Gigabytes - enough free space has to exist on the disk

// a memory block class
class CVirtualMemoryBlock
{
public:
	CVirtualMemoryBlock(int iSize);
	~CVirtualMemoryBlock();

	int			m_iSize;
	long long	m_lOffset;
	void		*m_pData;
	void		*m_pBlockData;

	void		*Lock();
	void		Release();
};

typedef Ut::UtVector< Ut::DynamicVector<CVirtualMemoryBlock*> > TBlockList;

// the memory manager
class CVirtualMemoryMgr
{
public:
	CVirtualMemoryMgr();
	~CVirtualMemoryMgr();

	char				m_pszFilename[_MAX_PATH];
	static HANDLE		m_hFile;
	static HANDLE		m_hMap;
	static int			m_iAllocGranularity;

	long long			m_lTotalSize;
	TBlockList			m_Blocks;

	static CVirtualMemoryMgr *GetVirtualMemoryMgr();
	static CVirtualMemoryMgr m_Mgr;

	CVirtualMemoryBlock	*Alloc(int iSize);
	void				Purge();
};

