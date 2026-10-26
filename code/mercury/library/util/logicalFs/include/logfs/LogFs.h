#ifndef LOGICALFS_LOG_FS_H
#define LOGICALFS_LOG_FS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Serialise/I_FileSystem.h>
#include "LogFsManager.h"
#include "LogFsEvent.h"

#ifdef LOGFS_ENABLE
	#define LOGFS_INIT(output_path) LogicalFs::C_LogFsManager::Instance().Initialise(output_path)
	#define LOGFS_WRAPPER(x, name) LogicalFs::C_LogFs<x>(name)
#else
	#define LOGFS_INIT(output_path) 
	#define LOGFS_WRAPPER(x, name) x
#endif

namespace Sys
{
class I_File;
struct C_FileInfo;
}

namespace LogicalFs
{

template <typename FILE_SYSTEM>
class C_LogFs : public FILE_SYSTEM
{
//From I_FileSystem
public:
	virtual ~C_LogFs();

	virtual Sys::I_File* Open( 
		const Sys::C_FilePath& filePath, 
		const Sys::E_FileAccess fileAccess = Sys::E_FileAccess_Read,
		const Sys::C_OpenParams& params = Sys::C_OpenParams());

	virtual bool FileExists(const Sys::C_FilePath& filePath);
 
	virtual bool FileInfo(const Sys::C_FilePath& filePath, Sys::C_FileInfo* pFileInfo);

	virtual bool FileAccess(const Sys::C_FilePath& filePath, U32 fileAccess);

	virtual bool FileTime(const Sys::C_FilePath& filePath, const Time::C_Time& fileTime);
 
	virtual bool DeleteFile(const Sys::C_FilePath& filePath);

	virtual bool DirectoryInfo(const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirInfo);
 
	virtual bool CreateSingleDirectory(const Sys::C_FilePath& path);

	virtual bool SetWorkingDirectory(const Sys::C_FilePath& path);

	virtual Sys::C_FilePath GetWorkingDirectory() const;

//From C_LogFs
public:
	C_LogFs(const char* name);

	const Btl::String& GetName() const;

//From C_LogFs
private:
	Btl::String m_name;
};

}

#include "LogFs.inl"

#endif

