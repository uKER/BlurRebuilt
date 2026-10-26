#ifndef _SYSTEM_C_FILESYSTEM_H__
#define _SYSTEM_C_FILESYSTEM_H__

#pragma once

#include <Core/Utility/Singleton.h>
#include <System/Serialise.h>

#if defined(PLATFORM_PS3)
#define EMULATE_CURRENT_DIRECTORY
#endif

#ifdef PLATFORM_PS3

namespace Sys
{

#define S_IWRITE S_IWUSR
#define S_IREAD  S_IRUSR

struct _finddata_t 
{
	unsigned    attrib;
	time_t      time_create;    /* -1 for FAT file systems */
	time_t      time_access;    /* -1 for FAT file systems */
	time_t      time_write;
	_fsize_t    size;
	char        name[260];
	int fd;
	char *filespec;
};

extern intptr_t  _findfirst( const char *filespec, _finddata_t *fileinfo );
extern int _findnext( intptr_t handle, _finddata_t *fileinfo );
extern int _findclose( intptr_t handle );
}


#endif

namespace Sys
{
	class C_FileSystem;
}

extern Sys::C_FileSystem* CFileSystemCreateFunc();

namespace Sys
{

template<typename T>
struct C_FileSystemCreate
{
	static T* Create()
	{
#ifdef CFILESYSTEM_CREATE_FUNC_ENABLED
		return CFileSystemCreateFunc();
#else
		return new T();
#endif
	}

	static void Destroy(T* p)
	{
		delete p;
	}

	static const bool m_bManagedDestroy = true;
};

struct C_FileInfo;


class C_FileSystem : public I_FileSystem, public Ut::Singleton<C_FileSystem, C_FileSystemCreate>
{
public:
    // this function is deprecated. please use the version above without the E_FileCreate parameter
	virtual I_File*						Open(
		const C_FilePath& fileName,
		const E_FileAccess = E_FileAccess_Read,
		const C_OpenParams& params = C_OpenParams());

	virtual ~C_FileSystem();

										/// returns true if the file exists, or otherwise false
	virtual bool						FileExists( const C_FilePath &fileName );
 
										/// returns a C_FileInfo structure for a given file
										/// returns false if the file doesn't exist
	virtual bool						FileInfo( const C_FilePath &fileName, C_FileInfo *p_FileInfo );

										/// set the access to the given file
										/// returns false if we don't have the required access or the file does not exist
	virtual bool						FileAccess( const C_FilePath &fileName, U32 fileAccess );

										/// sets the file time for a given file
										/// returns false if an error occurred
	virtual bool                        FileTime( const C_FilePath& fileName, const Time::C_Time &);
 
										/// delete a file or folder with a given name
										/// returns false if an error occurred
	virtual bool						DeleteFile( const C_FilePath& fileName );

										/// moves a file or folder, including its children
										/// returns false if an error occurred
	virtual bool						MoveFile( const C_FilePath& oldFileName, const C_FilePath& newFileName );

										/// returns directory info in DirectoryInfo
										/// returns false if an error occurred
    virtual bool						DirectoryInfo( const C_FilePath& filePath, C_DirectoryInfo * );

										/// create a directory with a given name/path
										/// returns false if the operation failed
    virtual bool						CreateSingleDirectory( const C_FilePath& filePath );

										/// changes the current working directory
										/// subsequent use of "./filename.ext" will be relative to this directory
										/// returns false if the operation failed
	virtual bool						SetWorkingDirectory( const C_FilePath& directoryPath );

										/// returns the path of the current working directory
	virtual C_FilePath					GetWorkingDirectory() const;

	/// returns the operating system filepath for the given high level path.
	virtual C_FilePath					GetSystemFilePath( const C_FilePath& filePath ) const;

private:
	virtual C_FilePath					_GetWorkingDirectory() const;
#ifdef EMULATE_CURRENT_DIRECTORY
	C_FilePath							m_sCurrentDirectory;
#endif
};

} // namespace Sys

#endif
