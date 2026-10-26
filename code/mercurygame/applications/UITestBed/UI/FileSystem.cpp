#include "Stdafx.h"

#include "FileSystem.h"

C_UiFileSystem g_UiFileSystem;

Sys::I_File* C_UiFileSystem::Open( const Sys::C_FilePath& filePath, const U32 fileAccessAttributes )
{
	return NULL;//Sys::C_FileSystem::Instance().Open( filePath, fileAccessAttributes );
}


bool C_UiFileSystem::FileExists( const Sys::C_FilePath& filePath )
{
	return false;//Sys::C_FileSystem::Instance().FileExists( filePath );
}


bool C_UiFileSystem::FileInfo( const Sys::C_FilePath& filePath, Sys::C_FileInfo* pFileInfo )
{
	return false;//Sys::C_FileSystem::Instance().FileInfo( filePath, pFileInfo );
}


bool C_UiFileSystem::FileAccess( const Sys::C_FilePath& filePath, U32 fileAccess )
{
	return false;//Sys::C_FileSystem::Instance().FileAccess( filePath, fileAccess );
}


bool C_UiFileSystem::FileTime( const Sys::C_FilePath& filePath, const Time::C_Time& time )
{
	return false;//Sys::C_FileSystem::Instance().FileTime( filePath, time );
}


bool C_UiFileSystem::DeleteFile( const Sys::C_FilePath& filePath )
{
	return false;//Sys::C_FileSystem::Instance().DeleteFile( filePath );
}


bool C_UiFileSystem::DirectoryInfo( const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirectoryInfo )
{
	return false;//Sys::C_FileSystem::Instance().DirectoryInfo( filePath, pDirectoryInfo );
}


bool C_UiFileSystem::CreateSingleDirectory( const Sys::C_FilePath& filePath )
{
	return false;//Sys::C_FileSystem::Instance().CreateSingleDirectory( filePath );
}


bool C_UiFileSystem::SetWorkingDirectory( const Sys::C_FilePath& directoryPath )
{
	return false;//Sys::C_FileSystem::Instance().SetWorkingDirectory( directoryPath );
}


Sys::C_FilePath C_UiFileSystem::GetWorkingDirectory() const
{
	return Sys::C_FileSystem::Instance().GetWorkingDirectory();
}
