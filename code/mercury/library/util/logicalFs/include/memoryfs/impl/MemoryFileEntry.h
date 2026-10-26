#ifndef SYSTEM_MEMORY_FILE_ENTRY_H
#define SYSTEM_MEMORY_FILE_ENTRY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace LogicalFs
{
	class C_MemoryDirectoryEntry;

	class C_MemoryFileEntry
	{
	public:
		C_MemoryFileEntry(void* pFileOrDirectory);

		C_MemoryFileEntry(const Sys::C_FileInfo& fileInfo, void* pFileOrDirectory);

		~C_MemoryFileEntry();

		const Sys::C_FileInfo& GetFileInfo() const;

		Sys::C_FileInfo& GetFileInfo();

		void* GetData();

		const void* GetData() const;

	private:
		C_MemoryFileEntry(const C_MemoryFileEntry& rhs);

		const C_MemoryFileEntry& operator=(const C_MemoryFileEntry& rhs);

		void*			m_pData;
		Sys::C_FileInfo	m_fileInfo;
	};
}

#endif
