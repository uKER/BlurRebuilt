#ifndef LOGICAL_FS_MEMORY_FS_H
#define LOGICAL_FS_MEMORY_FS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/I_FileSystem.h>

namespace LogicalFs
{
	class C_MemoryFsImpl;

	class C_MemoryFs : public Sys::I_FileSystem
	{
	//From Sys::I_FileSystem
	public:
		virtual					~C_MemoryFs();

		virtual Sys::I_File*	Open( 
			const Sys::C_FilePath& fileName, 
			const Sys::E_FileAccess fileAccess = Sys::E_FileAccess_Read,
			const Sys::C_OpenParams& params = Sys::C_OpenParams());

		virtual bool			FileExists( const Sys::C_FilePath &fileName );
	 	virtual bool			FileInfo( const Sys::C_FilePath &fileName, Sys::C_FileInfo* p_FileInfo );
		virtual bool			FileAccess( const Sys::C_FilePath &fileName, U32 fileAccess );
		virtual bool            FileTime( const Sys::C_FilePath& fileName, const Time::C_Time& fileTime);
	 	virtual bool			DeleteFile( const Sys::C_FilePath& fileName );
		virtual bool			DirectoryInfo(const Sys::C_FilePath& path, Sys::C_DirectoryInfo *pDirInfo);
		virtual bool			CreateSingleDirectory(const Sys::C_FilePath& path);
		virtual bool			SetWorkingDirectory(const Sys::C_FilePath& path);
		virtual Sys::C_FilePath	GetWorkingDirectory() const;
		virtual Sys::C_FilePath	GetSystemFilePath( const Sys::C_FilePath& filePath ) const;

	//From C_MemoryFileSystem
	public:
		C_MemoryFs();
		bool					InjectFile(Sys::C_FileInfo fileInfo, void* pData);
		void					SetMountPoint(const Sys::C_FilePath& mount);

	//From C_MemoryFileSystem
	private:
		C_MemoryFsImpl*	m_pImpl;
	};
} // namespace Sys

#endif
