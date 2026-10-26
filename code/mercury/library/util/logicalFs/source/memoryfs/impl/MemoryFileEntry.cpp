#include "LogicalFsPch.h"
#include <system/serialise.h>
#include <MemoryFs/Impl/MemoryFileEntry.h>
#include <MemoryFs/Impl/MemoryDirectoryEntry.h>

namespace LogicalFs
{

C_MemoryFileEntry::C_MemoryFileEntry(void* pFileOrDirectory)
{
	m_pData = pFileOrDirectory;
}

C_MemoryFileEntry::C_MemoryFileEntry(const Sys::C_FileInfo& fileInfo, void* pFileOrDirectory)
{
	m_fileInfo = fileInfo;
	m_pData = pFileOrDirectory;
}

C_MemoryFileEntry::~C_MemoryFileEntry()
{
	if (m_fileInfo.m_FileType == Sys::E_FileType_Directory && m_pData)
		delete (C_MemoryDirectoryEntry*)m_pData;
}

const Sys::C_FileInfo& C_MemoryFileEntry::GetFileInfo() const
{
	return m_fileInfo;
}

Sys::C_FileInfo& C_MemoryFileEntry::GetFileInfo()
{
	return m_fileInfo;
}

void* C_MemoryFileEntry::GetData()
{
	return m_pData;
}

const void* C_MemoryFileEntry::GetData() const
{
	return m_pData;
}

}
