#include "wrap32/com/stdafx.h"
#include <core/core.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wrap32/common/wrap32_common.h>
#include <vector>

#pragma warning(push)
//Disable convertion from x to y of greater size warning as we move from 32->64 bit.
#pragma warning(disable:4312)

struct C_Wrap32PageHeader
{
	UINT nPagesRemaining;
};

const UINT actualPageSize = 4096;
const UINT userPageSize = actualPageSize-sizeof(C_Wrap32PageHeader);
std::vector<HANDLE> g_sharedMemoryPages;
char str[4096];
char number[2048];

char* GetFileMappingName(UINT page)
{
	strcpy_s(str, "Local\\XDK");
	sprintf_s(number, "%i", page);
	strcat_s(str, number);
	return str;
}



UINT C_Wrap32Utility::CreateSharedMemory(UINT size)
{
	assert(g_sharedMemoryPages.size()==0);

	SECURITY_ATTRIBUTES secAttr;
	char secDesc[ SECURITY_DESCRIPTOR_MIN_LENGTH ];
	secAttr.nLength = sizeof(secAttr);
	secAttr.bInheritHandle = FALSE;
	secAttr.lpSecurityDescriptor = &secDesc;
	InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0, FALSE);

	int nPages = size / userPageSize;
	if (size % userPageSize)
		nPages++;

	HANDLE firstObject;
	HANDLE currentFile;
	UINT offset = 0;

	//Create first page.
	int iFile = 0;
	{
		const char* pageName = GetFileMappingName(iFile);

		firstObject = CreateFileMapping(
			INVALID_HANDLE_VALUE,   // use paging file
			&secAttr,                  // default security attributes
			PAGE_READWRITE,         // read/write access
			0,                      // size: high 32-bits
			actualPageSize,              // size: low 32-bits
			pageName); // name of map object
		g_sharedMemoryPages.push_back(firstObject);
		currentFile = firstObject;
	}
	iFile++;

	UINT userBytesAllocated = 0;
	for (int page = 0; page < nPages; iFile++, page++)
	{
		//Create next page if we're not the final page.
		HANDLE nextPage = 0;
		if (page < nPages-1)
		{
			const char* pageName = GetFileMappingName(iFile);

			nextPage = CreateFileMapping(
				INVALID_HANDLE_VALUE,   // use paging file
				&secAttr,                  // default security attributes
				PAGE_READWRITE,         // read/write access
				0,                      // size: high 32-bits
				actualPageSize,              // size: low 32-bits
				pageName); // name of map object
			g_sharedMemoryPages.push_back(nextPage);
		}

		// Get a pointer to the file-mapped shared memory
		LPVOID pPage = MapViewOfFile( 
			currentFile,     // object to map view of
			FILE_MAP_WRITE|FILE_MAP_READ, // read/write access
			0,              // high offset:  map from
			0,              // low offset:   beginning
			0);             // default: map entire file

		C_Wrap32PageHeader& header = *((C_Wrap32PageHeader*)pPage);
		header.nPagesRemaining = nPages-1 - page;

		BOOL success = FlushViewOfFile(pPage, 0);
		assert(success && "!FlushViewOfFile failed!");
		success = UnmapViewOfFile(pPage);
		assert(success && "!UnmapViewOfFile failed!");

		offset += actualPageSize;
		userBytesAllocated += userPageSize;
		currentFile = nextPage;
	}

	//return the number of bytes allocated.
	if(userBytesAllocated >= size)
		return size;
	else
	{
		assert(!"Wrap32 Memory Allocation Error!");
		return userBytesAllocated;
	}
}

BOOL C_Wrap32Utility::DestroySharedMemory()
{
	for (UINT i = 0; i < g_sharedMemoryPages.size(); i++)
		CloseHandle(g_sharedMemoryPages[i]);
	
	//Swap trick to free memory.
	g_sharedMemoryPages = std::vector<HANDLE>();
	return TRUE;
}

U8* ReadWriteSharedMemory(bool read, U8* pMappedMemory)
{
	BOOL success;

	UINT offset = 0;

	UINT nPagesRemaining = 0xffffffff;
	
	for (UINT iPage = 0; nPagesRemaining != 0; iPage++)
	{
		char* pFileName = GetFileMappingName(iPage);

		DWORD dwDesiredAccess = (!read) ? FILE_MAP_WRITE : FILE_MAP_READ;
		HANDLE hFile = OpenFileMapping(
			dwDesiredAccess,
			0, 
			pFileName);
		if (!hFile)
		{
			assert("OpenFileMapping failed in Wrap32!");
			return 0;
		}

		// Get a pointer to the file-mapped shared memory
		U8* pPage = (U8*)MapViewOfFile( 
			hFile,     // object to map view of
			dwDesiredAccess, // read/write access
			0,              // high offset:  map from
			0,              // low offset:   beginning
			0);             // default: map entire file
		if (!pPage)
		{
			assert("MapViewOfFile failed in Wrap32!");
			return 0;
		}

		//Get next page handle. Requires translation.
		const C_Wrap32PageHeader& header = *((C_Wrap32PageHeader*)pPage);
		nPagesRemaining = header.nPagesRemaining;

		if (read)
		{
			if (iPage == 0)
			{
				UINT size = (header.nPagesRemaining+1) * userPageSize;
				pMappedMemory = new U8[size];
			}
			memcpy(pMappedMemory+offset, pPage+sizeof(C_Wrap32PageHeader), userPageSize);
		}
		else
		{
			memcpy(pPage+sizeof(C_Wrap32PageHeader), pMappedMemory+offset, userPageSize);
		}

		success = FlushViewOfFile(pPage, 0);
		assert(success && "!FlushViewOfFile failed!");
		success = UnmapViewOfFile(pPage);
		assert(success && "!UnmapViewOfFile failed!");

		offset += userPageSize;
	}

	return pMappedMemory;
}

U8* C_Wrap32Utility::LockSharedMemory()
{
	return ReadWriteSharedMemory(true, 0);
}

BOOL C_Wrap32Utility::UnlockSharedMemory(U8* pSharedMemory)
{
	ReadWriteSharedMemory(false, pSharedMemory);
	delete[] pSharedMemory;
	return TRUE;
}

BOOL C_Wrap32Utility::DiscardSharedMemory(U8* pSharedMemory)
{
	delete[] pSharedMemory;
	return TRUE;
}

#ifdef _PSAPI_H_
bool C_Wrap32Utility::TerminateProcessesByName(const char *pName)
{
	BOOL bResult;
	DWORD aiPID[1000],iCb=1000,iNumProc,iV2000=0;
	DWORD iCbneeded,i;
	char szName[MAX_PATH],pNameUpper[MAX_PATH];
	HANDLE hProc;
	OSVERSIONINFO osvi;
	int iLen,iLenP,indx;
    HMODULE hMod;

      // ToolHelp Function Pointers.

	// Transfer Process name into "pNameUpper" and
	// convert it to upper case
	iLenP = (int)strlen(pName);
	if(iLenP<1 || iLenP>MAX_PATH)
		return false;
	for(indx=0;indx<iLenP;indx++)
		pNameUpper[indx]=toupper(pName[indx]);
	pNameUpper[iLenP]=0;

	// First check what version of Windows we're in
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    bResult=GetVersionEx(&osvi);
	if(!bResult)     // Unable to identify system version
	    return false;

    if(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
	{
		bResult=EnumProcesses(aiPID,iCb,&iCbneeded);
		if(!bResult)
		{
			// Unable to get process list, EnumProcesses failed
			return false;
		}

		// How many processes are there?
		iNumProc=iCbneeded/sizeof(DWORD);

		// Get and match the name of each process
		for(i=0;i<iNumProc;i++)
		{
			// Get the (module) name for this process

	        strcpy_s(szName,"Unknown");
			// First, get a handle to the process
	        hProc=OpenProcess(PROCESS_TERMINATE|PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,
				aiPID[i]);
	        // Now, get the process name
	        if(hProc)
			{
               if(EnumProcessModules(hProc,&hMod,sizeof(hMod),&iCbneeded) )
			   {
                  iLen=GetModuleBaseName(hProc,hMod,szName,MAX_PATH);
			   }
			}

			_strupr_s(szName);
			if(strcmp(szName,pNameUpper)==0)
				TerminateProcess(hProc, 0);

	        CloseHandle(hProc);
			// Match regardless of lower or upper case
		}
	}
	else
	{
		return false;
	}

	return true;
}
#endif

#pragma warning(pop)
