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
	Sys::I_File*			Open( const Util::C_String& fileName, const U32 fileAccessAttributes = Sys::E_FileAccess_Read );

	bool					FileExists( const Util::C_String &fileName );

	bool					FileInfo( const Util::C_String &fileName, Sys::C_Fileinfo* p_FileInfo );

	bool					FileAccess( const Util::C_String &fileName, U32 fileAccess );

	bool					FileTime( const Util::C_String& fileName, const Time::C_Time& time );

	bool					DeleteFile( const Util::C_String& fileName );

	bool					DirectoryInfo( const Util::C_String& filePath, Sys::C_DirectoryInfo* dirInfo );

	bool					CreateSingleDirectory( const Util::C_String& filePath );

	bool					SetWorkingDirectory( const Util::C_String& directoryPath );

	const Util::C_String	GetWorkingDirectory();
};

extern C_UiFileSystem g_UiFileSystem;

#endif /* UIFILESYSTEM_H */
