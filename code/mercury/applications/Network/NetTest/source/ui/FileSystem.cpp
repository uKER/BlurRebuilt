#include "Stdafx.h"

#include "FileSystem.h"

C_UiFileSystem g_UiFileSystem;

Sys::I_File* C_UiFileSystem::Open( const Util::C_String& fileName, const U32 fileAccessAttributes )
{
	return Sys::C_FileSystem::Instance().Open( fileName, fileAccessAttributes );
}

bool C_UiFileSystem::DirectoryInfo( const Util::C_String& filePath, Sys::C_DirectoryInfo* pDirectoryInfo )
{
	return Sys::C_FileSystem::Instance().DirectoryInfo( filePath, pDirectoryInfo );
}
