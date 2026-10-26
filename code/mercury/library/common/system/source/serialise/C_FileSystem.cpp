// a cross-platform default implementation for I_FileSystem
#include "SystemPch.h"

#include <system/serialise.h>
#include <system/serialise/C_FileSystem.h>
#include <system/serialise/C_File.h>
#include <system/misc/stringutil.h>
#include <system/time/time.h>
#include <System/Debug/Debug.h>

#if defined(COMPILER_VC)
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#else
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef PLATFORM_PS3
#include <sys/stat.h>
#include <sys/paths.h>
#include <cell/cell_fs.h>
#endif

#ifndef EMULATE_CURRENT_DIRECTORY
#define ConvertFilenameToCurrentDirectory(Y, X) (X)
#endif

#include <System/Io/Async.h>
#include <system\debug\Debug.h>

Debug::Logger &g_SystemIoLog = Debug::GetLogger("mercury.system.io");

Debug::Bool gSystemIoDisableAsyncFile("System.Io.DisableAsyncFile", false);

namespace Sys
{

I_File*	C_FileSystem::Open( const C_FilePath& fileName,
						    const E_FileAccess fileAccess,
							const C_OpenParams& params)
{
#ifndef TOOL
	if (!gSystemIoDisableAsyncFile)
	{
		if (fileAccess == E_FileAccess_Read)
		{
			AsyncFile* pFile = new AsyncFile;

			if (!pFile)
				return 0;

			const U32 kDefaultBufferSize = 128 * 1024;

			if (!pFile->Open(GetSystemFilePath(fileName).Chars(), 
					(params.m_BufferSizeHint == C_OpenParams::kBufferSizeHintDefault) ? kDefaultBufferSize : params.m_BufferSizeHint,
					params.m_IsCritical,
					params.m_NumBuffers
				))
			{
				delete pFile;
				return 0;
			}

			return pFile;
		}
	}
#endif

	// create new C_File object
	C_File *pFile = new C_File();
	const U32 bufferSize = (params.m_BufferSizeHint == C_OpenParams::kBufferSizeHintDefault) ? DEFAULT_FILE_STREAM_BUFFER_SIZE : params.m_BufferSizeHint;
	// open file
#ifdef PLATFORM_PS3
	C_FilePath ps3FileName = GetSystemFilePath(fileName);
	pFile->Open(ps3FileName, fileAccess, params.m_IsCritical, bufferSize);
#else
	pFile->Open(GetSystemFilePath(fileName), fileAccess, params.m_IsCritical, bufferSize);
#endif

	// file opened successfully - return
	if (pFile->IsOpen()) return pFile;

	// file failed to open - delete file object and return NULL
	Sys::FileOpenError(fileName.Chars(), fileAccess != E_FileAccess_Read, params.m_IsCritical); 
	delete pFile;
	return NULL;
}

C_FileSystem::~C_FileSystem()
{
}

#ifdef PLATFORM_PS3



char *MakePS3Filename( const char *filename )
{
	int fileNameLen = strlen(filename);
	char *temp = new char[ fileNameLen + 1 ];
	temp[ fileNameLen ] = '\0';
	sprintf(temp,"%s",filename);
	char searchStr[] = "/USRDIR/";
	char *lower = (char *)strstr(temp, searchStr);
	if (lower)
	{
		U32 start = strlen(searchStr);
		U32 end = strlen(lower);
		for (U32 i = start ; i < end ; i++)
		{
			if (lower[i] >= 'A' && lower[i] <= 'Z') lower[i] += 0x20;
		}
	}
	for (int i = 0 ; i < fileNameLen ; i++)
	{
		if (temp[i] == '\\') temp[i] = '/';
	}
	return temp;
}

char *MakePS3Dirname( const char *dirname )
{
	char *temp = MakePS3Filename( dirname );
	int tempLen = strlen(temp);
	if (tempLen)
	{
		int i = tempLen-1;
		while (!((temp[i] > 'A' && temp[i] < 'Z') || (temp[i] > 'a' && temp[i] < 'z') || (temp[i] > '0' && temp[i] < '9')))
		{
			i--;
		}
		if (i < tempLen-1) temp[i+1] = '\0';
		return temp;
	}
	return NULL;
}

#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */

// PS3 functions that emulate windows behavior
intptr_t  _findfirst( const char *filespec, _finddata_t *fileinfo )
{
	int fd;
	int ret;
	U64	readSize;
	CellFsDirent  dent;

	char *temp = MakePS3Dirname( filespec );
	ret = cellFsOpendir (temp, &fd);
	fileinfo->fd = fd;
	fileinfo->filespec = new char[strlen(temp)+2];
	strcpy(fileinfo->filespec, temp );
//		if (fileinfo->filespec[strlen(temp)-1] != '/') strcat(fileinfo->filespec,"/");
	delete [] temp;
	if (ret == CELL_FS_SUCCEEDED)
	{
		ret = cellFsReaddir(fd, &dent, &readSize);
		if (ret) 
		{
//			printf("cellFsReaddir(%s) returned %d\n", filespec, ret);
			cellFsClosedir (fd);
			return -1;
		}
		fileinfo->attrib = 0;
		if (dent.d_type & CELL_FS_TYPE_DIRECTORY) fileinfo->attrib |= _A_SUBDIR;
		if (dent.d_type & CELL_FS_TYPE_REGULAR)   fileinfo->attrib |= _A_NORMAL;
		memset(fileinfo->name, 0x00, sizeof(fileinfo->name));
		memcpy(fileinfo->name, dent.d_name, dent.d_namlen );
		CellFsStat stat;
		char fullPath[CELL_FS_MAX_FS_PATH_LENGTH+1];
		sprintf(fullPath,"%s/%s",fileinfo->filespec,fileinfo->name);

		if (fileinfo->name[strlen(fileinfo->name)-1] == '/') fileinfo->name[strlen(fileinfo->name)-1] = '\0';

		int result = cellFsStat( fullPath, &stat );
		if (result == CELL_FS_SUCCEEDED)
		{
			fileinfo->size = stat.st_size;
			fileinfo->time_access = stat.st_atime;
			fileinfo->time_create = stat.st_ctime;
			fileinfo->time_write = stat.st_mtime;
		}
	}
	else
	{
//		printf("cellFsOpendir(%s) returned %d\n", filespec, ret);
		return  -1;
	}
	return fd;
}

int _findnext( intptr_t handle, _finddata_t *fileinfo )
{
	int ret;
	U64 readSize;
	CellFsDirent  dent;
	
	ret = cellFsReaddir (handle, &dent, &readSize);
	if (ret) 
	{
		cellFsClosedir (handle);
		return  -1;
	}
	if (readSize == 0) { return -1; }
	
	fileinfo->attrib = 0;
	if (dent.d_type & CELL_FS_TYPE_DIRECTORY) fileinfo->attrib |= _A_SUBDIR;
	if (dent.d_type & CELL_FS_TYPE_REGULAR)   fileinfo->attrib |= _A_NORMAL;
	memset(fileinfo->name, 0x00, sizeof(fileinfo->name));
	memcpy(fileinfo->name, dent.d_name, dent.d_namlen );
	CellFsStat stat;
	memset(&stat,0x00,sizeof(stat));
	char fullPath[CELL_FS_MAX_FS_PATH_LENGTH+1];
	sprintf(fullPath,"%s/%s",fileinfo->filespec,fileinfo->name);


	int result = cellFsStat( fullPath, &stat );
	if (dent.d_type & CELL_FS_TYPE_DIRECTORY)
	{
		if (fileinfo->name[strlen(fileinfo->name)-1] == '/') fileinfo->name[strlen(fileinfo->name)-1] = '\0';
		result = CELL_FS_SUCCEEDED;
	}
	if (result == CELL_FS_SUCCEEDED)
	{
		fileinfo->size = stat.st_size;
		fileinfo->time_access = stat.st_atime;
		fileinfo->time_create = stat.st_ctime;
		fileinfo->time_write = stat.st_mtime;
		return 0;
	}
	return -1;
}

int _findclose( intptr_t handle )
{
	int ret;
	ret = cellFsClosedir (handle);
	
	if (ret) 
	{
//		printf("cellFsClosedir returned %d\n", ret);
		return  -1;
	}
	return 0;
}
#endif


bool C_FileSystem::FileExists( const C_FilePath &_fileName )
{
	using namespace Util;

	enum E_FileAccessProperty
    {
	    E_FileAccessProperty_Exists         = 00,
	    E_FileAccessProperty_Read           = 02,
	    E_FileAccessProperty_Write          = 04,
	    E_FileAccessProperty_ReadWrite      = 06
    };

	C_FilePath fileName = GetSystemFilePath(_fileName);

#if defined( COMPILER_VC )
	if(_access(fileName.Chars(), E_FileAccessProperty_Exists) == 0)
		return true;
#elif defined( PLATFORM_PS3 )
	struct stat s;
	if(stat(fileName.Chars(), &s) == 0)
		return true;
#endif
    return false;
}

bool C_FileSystem::FileInfo( const C_FilePath& _fileName, C_FileInfo *p_FileInfo )
{
	using namespace Util;

	C_FilePath fileName = GetSystemFilePath(_fileName);

    struct stat statResult;

    p_FileInfo->m_FileName = (char*)_fileName.Chars();
    p_FileInfo->m_FileType = E_FileType_NotPresent;
    
	if( stat( fileName.Chars(), &statResult ) == -1)
    {
        return false;
    }

#ifndef PLATFORM_PS3
	// Get actual filename
	{
		_finddata_t fd;
		intptr_t handle = _findfirst(fileName.Chars(), &fd);
		if(handle != -1) 
		{
			p_FileInfo->m_originalFilename = fd.name;
			_findclose(handle);
		}
	}
#endif
    
    p_FileInfo->m_FileSize = statResult.st_size;

    if(statResult.st_mode & S_IFDIR)
    {   
        p_FileInfo->m_FileType = E_FileType_Directory;
    }
    else
    if(statResult.st_mode & S_IFREG)
    {
        p_FileInfo->m_FileType = E_FileType_File;
    }

    p_FileInfo->m_FileAccess = 0;
    if(statResult.st_mode & S_IWRITE)
    {
        p_FileInfo->m_FileAccess |= E_FileAccess_Write;
    }

    if(statResult.st_mode & S_IREAD)
    {
        p_FileInfo->m_FileAccess |= E_FileAccess_Read;
    }

	p_FileInfo->m_FileTime = U64(statResult.st_mtime) * 1000000L;

	return true;
};

bool C_FileSystem::FileAccess( const C_FilePath &_fileName, U32 fileAccess )
{  
	using namespace Util;

	C_FilePath fileName = GetSystemFilePath(_fileName);

    U32 newAccess = 0;

#if defined( COMPILER_VC )
    if( fileAccess& E_FileAccess_Read ) 
        newAccess |= _S_IREAD;

    if( fileAccess& E_FileAccess_Write ) 
        newAccess |= _S_IWRITE;

    int result = _chmod( fileName.Chars(), newAccess );
#else
    if( fileAccess& E_FileAccess_Read ) 
        newAccess |= S_IREAD;

    if( fileAccess& E_FileAccess_Write ) 
        newAccess |= S_IWRITE;

    int result = chmod( fileName.Chars(), newAccess );
#endif

    if( result == -1 )
		return false;
	else
		return true;
}
bool C_FileSystem::DeleteFile( const C_FilePath& _fileName )
{
	C_FilePath fileName = GetSystemFilePath(_fileName);

    int result;

    result = remove(fileName.Chars());

	return (result!=-1);
}

bool C_FileSystem::MoveFile( const C_FilePath& oldFileName_, const C_FilePath& newFileName_)
{
	C_FilePath oldFileName = GetSystemFilePath(oldFileName_);
	C_FilePath newFileName = GetSystemFilePath(newFileName_);
	return rename(oldFileName.Chars(), newFileName.Chars()) == 0;
}

#if defined( COMPILER_VC ) || defined( PLATFORM_PS3 )
bool C_FileSystem::DirectoryInfo( const C_FilePath& _filePath, C_DirectoryInfo *pDirInfo )
{
	C_FilePath filePath = GetSystemFilePath(_filePath);

	{
		U32 length = filePath.Length();
		if (length == 0)
			return false;

		if (filePath[length-1] != '/' && (filePath[length-1] != '\\'))
			filePath += '/';
	}

	C_FilePath			strSearchString(filePath + C_FilePath("*.*"));
	_finddata_t				fd;
	intptr_t				handle;
	S32						iResult;
	U32						iValidFileCount;

	// Count valid files
	handle = _findfirst(strSearchString.Chars(), &fd);
	if(handle == -1)
		return false;

	iValidFileCount = 0;
	for(iResult = 0; iResult != -1; iResult = _findnext(handle, &fd))
		if(strcmp(fd.name,"..")!=0 && strcmp(fd.name, ".")!=0)
			iValidFileCount++;

	_findclose(handle);

	// Reread directory and add info for each file
	handle = _findfirst(strSearchString.Chars(), &fd);
	if(handle == -1)
		return false;

	// Add files
	pDirInfo->SetNumFiles(iValidFileCount);

	U32 iCount = 0;
	for(iResult = 0; (iResult != -1) && (iCount < iValidFileCount); iResult = _findnext(handle, &fd))
		if(strcmp(fd.name,"..")!=0 && strcmp(fd.name, ".")!=0)
		{
			C_FileInfo		info;
			Btl::String		fName( filePath.Chars() );
			C_FilePath		fileName;
			fName += fd.name;
			fileName.SetFormatted( fName );// Read from the disc, so the file should already be formatted

			if(	FileInfo( fileName, &info ) )
			{
				pDirInfo->AddFileInfo( info );
				iCount++;
			}
		}

	_findclose(handle);
	return true;
}

bool C_FileSystem::CreateSingleDirectory( const C_FilePath& _filePath )
{
	C_FilePath filePath = GetSystemFilePath(_filePath);

#if defined( COMPILER_VC )
	return ::CreateDirectory( filePath.Chars(), NULL ) == TRUE;
#else
	int result = cellFsMkdir( filePath.Chars(), 0 );
	if (result == CELL_FS_SUCCEEDED) return true;
	return false;
#endif
}

#else
#include <dirent.h>

bool C_FileSystem::CreateSingleDirectory( const C_FilePath& _filePath )
{
	return false;
}

bool C_FileSystem::DirectoryInfo( const C_FilePath& _filePath, C_DirectoryInfo *pDirInfo )
{
	C_FilePath filePath = ConvertFilenameToCurrentDirectory(_GetWorkingDirectory(),_filePath);

	C_FilePath strSearchString(filePath );
	DIR *dp;
	struct dirent *ep;

//	printf("opendir = %s\n", strSearchString.Chars());

	dp = opendir(strSearchString.Chars());
	if (dp == NULL)
		return false;

	// count files
	int nFiles = 0;
	while (ep = readdir(dp))
	{
//		printf("%s\n", ep->d_name);

		nFiles++;
	}

	closedir(dp);

	// read files
	dp = opendir(strSearchString.Chars());
	if (dp == NULL)
		return false;

	pDirInfo->SetNumFiles(nFiles);

	while (ep = readdir(dp))
	{
		if (strcmp(ep->d_name,"..")!=0 && strcmp(ep->d_name, ".")!=0)
		{
			C_FileInfo		info;
			C_FilePath	fileName(ep->d_name);

			if(	FileInfo( C_FilePath(filePath) + fileName, &info ) )
				pDirInfo->AddFileInfo( info );
		}
	}

	closedir(dp);

	return false;
}

#endif

const U32 C_DirectoryInfo::NumFiles()
{
	return (U32)m_fileInfo.Size();
}

C_FileInfo &C_DirectoryInfo::FileInfo( const U32 fileIndex )
{
	return m_fileInfo[fileIndex];
}

C_DirectoryInfo::~C_DirectoryInfo()
{
}

C_DirectoryInfo::C_DirectoryInfo()
{
}

C_DirectoryInfo::C_DirectoryInfo(const C_DirectoryInfo& rhs)
{ 
	*this = rhs; 
}

//We can't perform bitwise copies when using non-trivial types!
const C_DirectoryInfo& C_DirectoryInfo::operator=(const C_DirectoryInfo& rhs)
{
	m_fileInfo = rhs.m_fileInfo;
	return *this;
}

void C_DirectoryInfo::SetNumFiles(int n)
{
	m_fileInfo.Reserve(n);
}

void C_DirectoryInfo::AddFileInfo(const C_FileInfo &info)
{
	m_fileInfo.PushBack(info);
}

FileInfoArr& C_DirectoryInfo::FileInfoArray()
{
	return m_fileInfo;
}

const FileInfoArr& C_DirectoryInfo::FileInfoArray() const
{
	return m_fileInfo;
}

C_FilePath C_FileSystem::GetWorkingDirectory() const
{
	//remove internal calls to virtual func to remove log spam.
	return _GetWorkingDirectory();
}

C_FilePath C_FileSystem::GetSystemFilePath( const C_FilePath& filePath ) const
{
	return ConvertToCurrentDirectory(_GetWorkingDirectory(), filePath);
}

#ifdef EMULATE_CURRENT_DIRECTORY
bool C_FileSystem::SetWorkingDirectory( const C_FilePath& directoryPath )
{
	m_sCurrentDirectory = directoryPath;
	return true;
}

C_FilePath C_FileSystem::_GetWorkingDirectory() const
{
	return m_sCurrentDirectory;
}

#else
bool C_FileSystem::SetWorkingDirectory( const C_FilePath& directoryPath )
{
	return (chdir(directoryPath.Chars()) == 0);
}

C_FilePath C_FileSystem::_GetWorkingDirectory() const
{
#ifdef PLATFORM_PC
	char buf[MAX_PATH];
	char *pCwd = _getcwd(buf, MAX_PATH);
	C_FilePath rtn(pCwd);
	return rtn;
#else
	char buf[256];
	char *pCwd = getcwd(buf, sizeof(buf));
	C_FilePath rtn(pCwd);
	return rtn;
#endif
}
#endif

#if defined( COMPILER_VC )
bool C_FileSystem::FileTime( const C_FilePath& _fileName, const Time::C_Time &_time )
{
	C_FilePath fileName = GetSystemFilePath(_fileName);

	HANDLE		hFile;
	BOOL		bResult;

	hFile = CreateFile( fileName.Chars(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( hFile == INVALID_HANDLE_VALUE )
        return false;

	Time::C_Time time = _time;

	time += SYS_DELTA_EPOCH_IN_USEC;
	time *= 10;

	bResult = SetFileTime(hFile, (LPFILETIME)NULL, (LPFILETIME)&time, (LPFILETIME)&time);

	CloseHandle( hFile );
	return bResult == TRUE;
}
#else
bool C_FileSystem::FileTime( const C_FilePath& fileName, const Time::C_Time &time )
{
	return false;
}
#endif

}
