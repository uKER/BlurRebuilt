#ifndef UIFILESYSTEM_H
#define UIFILESYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/I_FileSystem.h>

class C_UiFileSystem : public Sys::I_FileSystem
{
public:
	C_UiFileSystem()			{  }
	~C_UiFileSystem()			{  }

public:
	Sys::I_File*	Open( const Util::C_String& fileName, const U32 fileAccessAttributes = Sys::E_FileAccess_Read );
	bool			DirectoryInfo( const Util::C_String& filePath, Sys::C_DirectoryInfo* pDirectoryInfo );
};

extern C_UiFileSystem g_UiFileSystem;

#endif /* UIFILESYSTEM_H */
