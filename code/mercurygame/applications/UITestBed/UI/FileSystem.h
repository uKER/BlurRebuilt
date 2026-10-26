#ifndef UIFILESYSTEM_H
#define UIFILESYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/I_FileSystem.h>

class C_UiFileSystem : public Sys::I_FileSystem
{
public:
	C_UiFileSystem()		{  }
	~C_UiFileSystem()		{  }

public:
	Sys::I_File*			Open( const Sys::C_FilePath& filePath, const U32 fileAccessAttributes = Sys::E_FileAccess_Read );

	bool					FileExists( const Sys::C_FilePath& filePath );

	bool					FileInfo( const Sys::C_FilePath& filePath, Sys::C_FileInfo* pFileInfo );

	bool					FileAccess( const Sys::C_FilePath& filePath, U32 fileAccess );

	bool					FileTime( const Sys::C_FilePath& filePath, const Time::C_Time& time );

	bool					DeleteFile( const Sys::C_FilePath& filePath );

	bool					DirectoryInfo( const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirectoryInfo );

	bool					CreateSingleDirectory( const Sys::C_FilePath& filePath );

	bool					SetWorkingDirectory( const Sys::C_FilePath& directoryPath );

	Sys::C_FilePath			GetWorkingDirectory() const;
};

extern C_UiFileSystem g_UiFileSystem;

#endif /* UIFILESYSTEM_H */
