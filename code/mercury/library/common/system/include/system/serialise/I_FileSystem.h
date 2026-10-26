#ifndef _SYSTEM_I_FILEMSYSTEM_H__
#define _SYSTEM_I_FILEMSYSTEM_H__

#pragma once

#include "FileSystemGlobals.h"
#include "I_BinaryStream.h"
#include "I_File.h"

#ifdef PLATFORM_PC
// HACK: #include <windows.h> #defines CreateDirectory to CreateDirectoryA
// this is needed for consistency if windows.h wasn't included
#ifndef CreateDirectory
#define CreateDirectory CreateDirectoryA
#endif
#endif

#include <Btl/Containers/Vector.h>
#include "FilePath.h"

namespace Time
{
typedef U64 C_Time;
}

namespace Sys
{



/// information structure returned from Sys::I_FileSystem::FileInfo
struct C_FileInfo
{
	C_FileInfo() {}
	C_FileInfo(const C_FileInfo& rhs) { *this = rhs; }

	//We can't perform bitwise copies when using non-trivial types!
	const C_FileInfo& operator=(const C_FileInfo& rhs)
	{
		m_FileName = rhs.m_FileName;
		m_originalFilename = rhs.m_originalFilename;
		m_FileType = rhs.m_FileType;
		m_FileSize = rhs.m_FileSize;
		m_FileAccess = rhs.m_FileAccess;
		m_FileTime = rhs.m_FileTime;

		return *this;
	}

	void Clear()
	{
		m_FileName.Clear();
		m_originalFilename.Clear();
		m_FileType = E_FileType_NotPresent;
		m_FileSize = 0;
		m_FileAccess = 0;
		m_FileTime = 0;
	}

	C_FilePath					m_FileName;
	C_FilePath					m_originalFilename;
    E_FileType                  m_FileType; 
    C_StreamSize				m_FileSize; 
    U32                         m_FileAccess; 
	Time::C_Time				m_FileTime; 
};
typedef Btl::Vector<C_FileInfo> FileInfoArr;

/// information structure returned from Sys::I_FileSystem::DirectoryInfo
class C_DirectoryInfo
{
public:
	/// constructor (private use)
	C_DirectoryInfo();
	/// destructor (private use)
	~C_DirectoryInfo();

	C_DirectoryInfo(const C_DirectoryInfo& rhs);

	//We can't perform bitwise copies when using non-trivial types!
	const C_DirectoryInfo& operator=(const C_DirectoryInfo& rhs);

								/// allocates space for the file info (private use)
	void						SetNumFiles(int n);

								/// returns the number of files in the directory
    const U32                   NumFiles();

								/// retrieve the file info structure for a specified file index
    C_FileInfo					&FileInfo( const U32 fileIndex );

	FileInfoArr&				FileInfoArray();

	const FileInfoArr&			FileInfoArray() const;

								/// add file info (private use)
	void						AddFileInfo(const C_FileInfo &info);

private:
	FileInfoArr m_fileInfo;
};


struct C_OpenParams
{
	C_OpenParams(
		U32 bufferSizeHint = kBufferSizeHintDefault, //Hint at desired buffer size.
#ifdef TOOL
		bool isCritical = false, //Don't invoke critical error callback on failure for tools.
#else
		bool isCritical = true, //Invoke critical error callback on failure by default for games.
#endif
		bool assertEof = true,
		U32 numBuffers = 0,
		bool serialAccess = false
		)
		:
		m_BufferSizeHint(bufferSizeHint),
		m_IsCritical(isCritical),
		m_AssertEof(assertEof),
		m_NumBuffers(numBuffers),
		m_SerialAccess(serialAccess)
	{}

	U32 m_BufferSizeHint;
	U32 m_NumBuffers;
	bool m_IsCritical;
	bool m_AssertEof;
	bool m_SerialAccess;

public:
	static const U32 kBufferSizeHintDefault = ~(U32)0;
};


class I_FileSystem
{
public:
    // note only read access on some file systems
	virtual I_File*						Open(
		const C_FilePath& fileName, 
		const E_FileAccess fileAccess = E_FileAccess_Read,
		const C_OpenParams& params = C_OpenParams()) { return 0; }

	virtual ~I_FileSystem() {};

	/// returns true if the file exists, or otherwise false
	virtual bool						FileExists( const C_FilePath & ) { return false; }
 
	/// returns a C_FileInfo structure for a given file
	/// returns false if the file doesn't exist
	virtual bool						FileInfo( const C_FilePath &, C_FileInfo* ) { return false; }

	/// set the access to the given file
	/// returns false if we don't have the required access or the file does not exist
	virtual bool						FileAccess( const C_FilePath &, U32 ) { return false; }

	/// sets the file in C_Time time for a given file
	/// returns false if an error occurred
	virtual bool                        FileTime( const C_FilePath&, const Time::C_Time &) { return false; }
 
	/// delete a file or folder with a given name
	/// returns false if an error occurred
	virtual bool						DeleteFile( const C_FilePath& ) { return false; }

	/// moves a file or folder, including its children
	/// returns false if an error occurred
	virtual bool						MoveFile( const C_FilePath& oldFileName, const C_FilePath& newFileName ) { return false; }

	/// returns directory info in DirectoryInfo
	/// returns false if an error occurred
    virtual bool						DirectoryInfo( const C_FilePath&, C_DirectoryInfo * ) { return false; }
 
	/// create a directory with a given name/path
	/// returns false if the operation failed
    virtual bool						CreateSingleDirectory( const C_FilePath& ) { return false; }

	/// changes the current working directory
	/// subsequent use of "./filename.ext" will be relative to this directory
	/// returns false if the operation failed
	virtual bool						SetWorkingDirectory( const C_FilePath& ) { return false; }

	/// returns the path of the current working directory
	virtual C_FilePath					GetWorkingDirectory() const { return C_FilePath(); }

	/// returns the operating system filepath for the given high level path.
	virtual C_FilePath					GetSystemFilePath( const C_FilePath& filePath ) const { return filePath; } 

	/// create a directory with a given name/path
	//  any missing links in a chain of directories will be created.
	/// returns false if the operation failed
    bool								CreateDirectory( const C_FilePath& filePath );
};

} // namespace Sys

#endif
