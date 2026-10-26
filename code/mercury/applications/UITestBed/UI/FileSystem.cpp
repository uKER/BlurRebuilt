#include "Stdafx.h"

#include "FileSystem.h"

C_UiFileSystem g_UiFileSystem;

Sys::I_File* C_UiFileSystem::Open( const Util::C_String& fileName, const U32 fileAccessAttributes )
{
	return Sys::C_FileSystem::Instance().Open( fileName, fileAccessAttributes );
}


bool C_UiFileSystem::FileExists( const Util::C_String &fileName )
{
	return Sys::C_FileSystem::Instance().FileExists( fileName );
}


bool C_UiFileSystem::FileInfo( const Util::C_String &fileName, Sys::C_Fileinfo* p_FileInfo )
{
	return Sys::C_FileSystem::Instance().FileExists( fileName );
}


bool C_UiFileSystem::FileAccess( const Util::C_String &fileName, U32 fileAccess )
{
	return Sys::C_FileSystem::Instance().FileAccess( fileName, fileAccess );
}


bool C_UiFileSystem::FileTime( const Util::C_String& fileName, const Time::C_Time& time )
{
	return Sys::C_FileSystem::Instance().FileTime( fileName, time );
}


bool C_UiFileSystem::DeleteFile( const Util::C_String& fileName )
{
	return Sys::C_FileSystem::Instance().DeleteFile( fileName );
}


bool C_UiFileSystem::DirectoryInfo( const Util::C_String& filePath, Sys::C_DirectoryInfo* dirInfo )
{
	return Sys::C_FileSystem::Instance().DirectoryInfo( filePath, dirInfo );
}


bool C_UiFileSystem::CreateSingleDirectory( const Util::C_String& filePath )
{
	return Sys::C_FileSystem::Instance().CreateSingleDirectory( filePath );
}


bool C_UiFileSystem::SetWorkingDirectory( const Util::C_String& directoryPath )
{
	return Sys::C_FileSystem::Instance().SetWorkingDirectory( directoryPath );
}


const Util::C_String C_UiFileSystem::GetWorkingDirectory()
{
	return Sys::C_FileSystem::Instance().GetWorkingDirectory();
}