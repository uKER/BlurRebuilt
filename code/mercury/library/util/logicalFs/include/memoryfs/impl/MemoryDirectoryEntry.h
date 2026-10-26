#ifndef SYSTEM_C_MEMORY_DIRECTORY_ENTRY_H
#define SYSTEM_C_MEMORY_DIRECTORY_ENTRY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <MemoryFs/Impl/MemoryFileEntry.h>

namespace LogicalFs
{
	class C_MemoryDirectoryEntry
	{
	public:
		C_MemoryDirectoryEntry(C_MemoryDirectoryEntry* pParent);

		~C_MemoryDirectoryEntry();

		const Btl::Vector<C_MemoryFileEntry*>& GetFiles() const;

		bool AddFile(const Sys::C_FileInfo& info, void* pData);

		bool AddDirectory(const Sys::C_FileInfo& info);

		C_MemoryFileEntry* GetFileEntry(const Sys::C_FilePath& file);

		bool GetFileInfo(const Sys::C_FilePath& formattedFilePath, Sys::C_FileInfo* pDirInfo);

		bool GetDirectoryInfo(Sys::C_DirectoryInfo* pDirInfo);

		C_MemoryDirectoryEntry*	GetParent();

		const C_MemoryDirectoryEntry* GetParent() const;

		bool DeleteFile(const Sys::C_FilePath& formattedFilePath);

	private:
		C_MemoryDirectoryEntry(const C_MemoryDirectoryEntry& rhs);

		const C_MemoryDirectoryEntry& operator=(const C_MemoryDirectoryEntry& rhs);

		Btl::Vector<C_MemoryFileEntry*>	m_files;
		C_MemoryDirectoryEntry*			m_pParent;
	};
}

#endif
