#include "LogicalFsPch.h"

#include <MemoryFs/Impl/MemoryFileEntry.h>
#include <MemoryFs/Impl/MemoryDirectoryEntry.h>

namespace LogicalFs
{

C_MemoryDirectoryEntry::C_MemoryDirectoryEntry(C_MemoryDirectoryEntry* pParent)
{
	m_pParent = pParent;
}

C_MemoryDirectoryEntry::~C_MemoryDirectoryEntry()
{
	for (S32 i = 0; i < m_files.Size(); ++i)
		SAFE_DELETE_PTR(m_files[i]);
}

const Btl::Vector<C_MemoryFileEntry*>& C_MemoryDirectoryEntry::GetFiles() const
{
	return m_files;
}

bool C_MemoryDirectoryEntry::AddFile(const Sys::C_FileInfo& info, void* pData)
{
	//Search for a duplicate record.
	for (S32 i = 0; i < m_files.Size(); ++i)
	{
		if (m_files[i]->GetFileInfo().m_FileName == info.m_FileName)
			//Duplicate found.
			return false;
	}

	//No duplicate, insert.
	m_files.PushBack(new C_MemoryFileEntry(info, pData));
	return true;
}

bool C_MemoryDirectoryEntry::AddDirectory(const Sys::C_FileInfo& info)
{
	if (info.m_FileType != Sys::E_FileType_Directory)
		return false;

	//Search for a duplicate record.
	for (S32 i = 0; i < m_files.Size(); ++i)
	{
		if (m_files[i]->GetFileInfo().m_FileName == info.m_FileName)
			//Duplicate found.
			return false;
	}

	//No duplicate, insert.
	//The memory we allocate here is freed for us in the C_MemoryFileEntry destructor.
	C_MemoryDirectoryEntry* pNewDir = new C_MemoryDirectoryEntry(this);
	m_files.PushBack(new C_MemoryFileEntry(info, pNewDir));

	return true;
}

C_MemoryFileEntry* C_MemoryDirectoryEntry::GetFileEntry(const Sys::C_FilePath& file)
{
	for (S32 i = 0; i < m_files.Size(); ++i)
	{
		C_MemoryFileEntry* pEntry = m_files[i];
		const Sys::C_FileInfo& info = pEntry->GetFileInfo();
		if (info.m_FileName == file)
			return pEntry;
	}

	return 0;
}

bool C_MemoryDirectoryEntry::GetFileInfo(const Sys::C_FilePath& formattedFilePath, Sys::C_FileInfo* pFileInfo)
{
	C_MemoryFileEntry* pEntry = GetFileEntry(formattedFilePath);
	if (pEntry)
	{
		*pFileInfo = pEntry->GetFileInfo();
		return true;
	}
	return false;
}

bool C_MemoryDirectoryEntry::GetDirectoryInfo(Sys::C_DirectoryInfo* pDirInfo)
{
	for (U32 i = 0; i < (U32)m_files.Size(); ++i)
	{
		C_MemoryFileEntry* pEntry = m_files[i];
		pDirInfo->AddFileInfo(pEntry->GetFileInfo());
	}

	return true;
}

bool C_MemoryDirectoryEntry::DeleteFile(const Sys::C_FilePath& formattedFilePath)
{
	//Search for record.
	for (S32 i = 0; i < m_files.Size(); ++i)
	{
		if (m_files[i]->GetFileInfo().m_FileName == formattedFilePath)
		{
			SAFE_DELETE_PTR(m_files[i]);
			m_files.EraseByIndex(i);
			return true;
		}
	}

	//Could not find file to delete.
	return false;
}

C_MemoryDirectoryEntry*	C_MemoryDirectoryEntry::GetParent()
{
	return m_pParent;
}

const C_MemoryDirectoryEntry* C_MemoryDirectoryEntry::GetParent() const
{
	return m_pParent;
}

}
