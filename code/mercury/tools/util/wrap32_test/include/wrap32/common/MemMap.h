// MemMap.h: interface for the CMemMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMMAP_H__210B79CC_BD1F_467C_87D2_8AC6CE869577__INCLUDED_)
#define AFX_MEMMAP_H__210B79CC_BD1F_467C_87D2_8AC6CE869577__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The size of each memory block
#ifndef MMF_PAGESIZE
#define MMF_PAGESIZE		4096  // 4k  
#endif

// used to mark end of data
const UINT D_N_ = 0xFFFFFF00;
const LPVOID DOUBLE_NULL = (LPVOID)&D_N_;

// structure with counters kept in memory-mapped file
struct RW_COUNTERS
{
   int    m_nWaitingReaders; // Number of readers waiting for access
   int    m_nWaitingWriters; // Number of writers waiting for access
   int    m_nActive;         // Number of threads currently with access
};


class CMemMap
{
public:
	CMemMap					();
	virtual ~CMemMap		();

	DWORD	Create			(LPCTSTR szMappedName, DWORD dwWaitTime, ULONG ulMappedSize);
	BOOL	Close			();
	VOID    Vacuum			();

	// String functions
	BOOL	AddString		(LPCTSTR szString, UINT uId);
	BOOL	UpdateString	(LPCTSTR szString, UINT uId);
	UINT	GetString		(LPCTSTR szString, UINT uLen, UINT uId);
	UINT    GetStringLength	(UINT uId);
	LPCTSTR GetString		(UINT uId);

	// Binary data functions
	BOOL	AddBinary		(LPVOID lpBin, UINT uSize, UINT uId);
	BOOL	UpdateBinary	(LPVOID lpBin, UINT uSize, UINT uId);
	UINT	GetBinary		(LPVOID lpBin, UINT uSize, UINT uId);
	UINT    GetBinarySize	(UINT uId);
	LPVOID	GetBinary		(UINT uId);

	// Object functions
	BOOL	DeleteID		(UINT uId);
	UINT	Count			();
	UINT64	UsedSize		();

protected:
	// managment functions
	DWORD	Grow			(UINT uGrowBy);
	VOID	Shrink			(UINT uShrinkBy);
	VOID	CheckForNewPages();
	DWORD	MapFiles		(UINT uStart, UINT uEnd);
	VOID	CreateContainerNames(UINT uStart, UINT uEnd);

	// Read Write functions
	VOID	Write			(UINT *uPage, LPBYTE *lpOffset, UINT uLen, LPVOID lpIn);
	LPBYTE	Read			(UINT *uPage, LPBYTE *lpOffset, UINT uLen, LPVOID lpOut);

	// Helper functions
	VOID	NextID			(UINT *uPage, LPBYTE *lpOffset);
	BOOL	FindID			(UINT uId, UINT *uPage, LPBYTE *lpOffset);
	UINT	GetID			(UINT uPage, LPBYTE lpOffset);
	UINT	GetLength		(UINT uPage, LPBYTE lpOffset);

private:
	UINT	m_uPageCount;					// number of array members

	LPTSTR	*m_lpUniqueContainerNames;		// array of unique names
	LPTSTR	m_lpContainerName;				// copy of unappended name

	LPBYTE	m_lpReturnBuffer;				// internal buffer for returning data
	LPBYTE	m_lpDataBuffer;					// internal buffer for merging data

	HANDLE	*m_hMappedFiles;				// array of page handles
	LPVOID	*m_pMappedViews;				// array of page pointers
	

/*
*	Multi reader/Single writer Written by: Alex Farber   alexm@cmt.co.il
*/
public:

    BOOL WaitToRead			();				// Call this to gain shared read access
    BOOL WaitToWrite		();				// Call this to gain exclusive write access
    BOOL Done				();				// Call this when done accessing the resource

protected:

	BOOL InitProtect		(LPCTSTR szName,DWORD dwWait);
	BOOL EndProtect			();
    BOOL InitSyncObjects	();

    DWORD	m_dwMilliseconds;
    LPTSTR	m_sMutexName;
    LPTSTR	m_sSemReadersName;
    LPTSTR	m_sSemWritersName;
    LPTSTR	m_sMemFileName;

    HANDLE	m_hMutex;          // Permits exclusive access to other members
    HANDLE	m_hsemReaders;     // Readers wait on this if a writer has access
    HANDLE	m_hsemWriters;     // Writers wait on this if a reader has access

    // memory-mapped file used to keep counters:
    HANDLE	m_hFileMapping;
    LPVOID	m_pViewOfFile;

    int*    m_pnWaitingReaders; // Number of readers waiting for access
    int*    m_pnWaitingWriters; // Number of writers waiting for access
    int*    m_pnActive;         // Number of threads currently with access
	
};

#endif // !defined(AFX_MEMMAP_H__210B79CC_BD1F_467C_87D2_8AC6CE869577__INCLUDED_)
