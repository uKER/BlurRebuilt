#include "LogicalFsPch.h"

#include <system/serialise.h>
#include <MemoryFs/Impl/MemoryFileEntry.h>
#include <MemoryFs/Impl/MemoryDirectoryEntry.h>
#include <MemoryFs/Impl/MemoryFile.h>
#include <MemoryFs/MemoryFs.h>
#include <system/misc/stringutil.h>

namespace LogicalFs
{

class C_MemoryFsImpl
{
public:
	C_MemoryFsImpl() : m_root(new C_MemoryDirectoryEntry(NULL)) 
	{
		m_root.GetFileInfo().m_FileName = "/";
		m_root.GetFileInfo().m_FileType = Sys::E_FileType_Directory;
		m_root.GetFileInfo().m_FileSize = 0;
		m_root.GetFileInfo().m_FileAccess = Sys::E_FileAccess_Read;
		m_root.GetFileInfo().m_FileTime = 0;
	}

	bool AddFile(Sys::C_FileInfo info, void* pData);

	bool AddDirectory(Sys::C_FileInfo info);

	bool AddEntry(Sys::C_FileInfo info, void* pData);

	bool DeleteFile(const Sys::C_FilePath& filePath);

	C_MemoryFileEntry*	GetFileEntry(
		Sys::C_FilePath& resultFilePath,
		const Sys::C_FilePath& absoluteFilePath);

	bool SetWorkingDirectory(const Sys::C_FilePath& path)
	{
		m_workingDirectory = path;
		return true;
	}

	const Sys::C_FilePath& GetWorkingDirectory() const
	{ 
		return m_workingDirectory;
	}

private:
	C_MemoryFileEntry* GetFileEntry(
		Sys::C_FilePath& resultFilePath,
		const Sys::C_FilePath& absoluteFilePath,
		U32 pathTruncation,
		C_MemoryDirectoryEntry* pParentDir);

	C_MemoryFileEntry		m_root;
	Sys::C_FilePath			m_workingDirectory;
};



//From Sys::I_FileSystem

Sys::I_File* C_MemoryFs::Open( 
	const Sys::C_FilePath& fileName,
	const Sys::E_FileAccess fileAccess,
	const Sys::C_OpenParams& params)
{
	if (fileAccess != Sys::E_FileAccess_Read)
	{
		BIZ_ASSERT_FAIL("NOT IMPLEMENTED");
		return 0;
	}

	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pEntry = m_pImpl->GetFileEntry(resultFilePath, fileName);

	//If we've found our entry, and it's of type 'file'...
	if (pEntry && pEntry->GetFileInfo().m_FileType == Sys::E_FileType_File)
	{
		// create new C_File object
		C_MemoryFile *pFile = new C_MemoryFile();

		// open file
		if (!pFile->Open(pEntry->GetData(), (U32)pEntry->GetFileInfo().m_FileSize, fileName.Chars(), params))
			return false;

		// file opened successfully - return
		if (pFile->IsOpen())
			return pFile;

		// file failed to open - delete file object and return NULL
		delete pFile;
	}

	Sys::FileOpenError(fileName.Chars(), fileAccess != Sys::E_FileAccess_Read, params.m_IsCritical);
	return 0;
}

C_MemoryFs::C_MemoryFs()
{
	m_pImpl = new C_MemoryFsImpl();
}

C_MemoryFs::~C_MemoryFs()
{
	SAFE_DELETE_PTR(m_pImpl);
}
	
bool C_MemoryFs::FileExists( const Sys::C_FilePath &filePath )
{
	Sys::C_FileInfo info;
	return FileInfo(filePath, &info);
}

bool C_MemoryFs::FileInfo(const Sys::C_FilePath& absoluteFilePath, Sys::C_FileInfo* pFileInfo)
{
	BIZ_ASSERT(pFileInfo);

	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pEntry = m_pImpl->GetFileEntry(resultFilePath, absoluteFilePath);
	if (pEntry)
	{
		//File exists.
		*pFileInfo = pEntry->GetFileInfo();
		pFileInfo->m_FileName = resultFilePath;
		return true;
	}

	//File does not exist.
	pFileInfo->Clear();
	pFileInfo->m_FileType = Sys::E_FileType_NotPresent;
	return false;
}

bool C_MemoryFs::FileAccess( const Sys::C_FilePath &fileName, U32 fileAccess )
{
	BIZ_ASSERT_FAIL("NOT SUPPORTED!");
	return false;
}

bool C_MemoryFs::FileTime( const Sys::C_FilePath& fileName, const Time::C_Time& time)
{	
	BIZ_ASSERT_FAIL("NOT SUPPORTED!");
	return false;
}

bool C_MemoryFs::DeleteFile( const Sys::C_FilePath& filePath)
{
	return m_pImpl->DeleteFile(filePath);
}

bool C_MemoryFs::DirectoryInfo(const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirInfo)
{
	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pEntry =	m_pImpl->GetFileEntry(resultFilePath, filePath);
	if (!pEntry || pEntry->GetFileInfo().m_FileType != Sys::E_FileType_Directory)
		return false;

	C_MemoryDirectoryEntry* pDir = (C_MemoryDirectoryEntry*)pEntry->GetData();

	if (!pDir->GetDirectoryInfo(pDirInfo))
		return false;

	Sys::C_FilePath tmp(filePath);
	U32 length = tmp.Length();
	if (length == 0 || tmp[length-1] != Sys::GetLocalDirToken())
		tmp += Sys::C_FilePath(Sys::GetLocalDirToken());
	for (U32 i = 0; i < pDirInfo->NumFiles(); ++i)
		pDirInfo->FileInfo(i).m_FileName = tmp + pDirInfo->FileInfo(i).m_FileName;

	return true;
}

bool C_MemoryFs::CreateSingleDirectory( const Sys::C_FilePath& filePath )
{
	Sys::C_FileInfo info;
	info.Clear();

	info.m_FileType = Sys::E_FileType_Directory;
	info.m_FileName = filePath;

	return m_pImpl->AddEntry(info, 0);
}

bool C_MemoryFs::SetWorkingDirectory(const Sys::C_FilePath& path)
{
	return m_pImpl->SetWorkingDirectory(path);
}

Sys::C_FilePath C_MemoryFs::GetWorkingDirectory() const
{ 
	return m_pImpl->GetWorkingDirectory();
}

Sys::C_FilePath C_MemoryFs::GetSystemFilePath( const Sys::C_FilePath& filePath ) const
{
	return Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), filePath);
}

//From C_MemoryFs

bool C_MemoryFs::InjectFile(Sys::C_FileInfo fileInfo, void* pData)
{
	if (fileInfo.m_FileType != Sys::E_FileType_File)
	{
		BIZ_ASSERT_FAIL("Invalid file injection!");
		return false;
	}
	return m_pImpl->AddEntry(fileInfo, pData);
}


C_MemoryFileEntry* C_MemoryFsImpl::GetFileEntry(
	Sys::C_FilePath& resultFilePath, 
	const Sys::C_FilePath& filePath)
{
	if (filePath.Length() == 0 || filePath == Sys::C_FilePath(Sys::GetLocalDirToken()))
	{
		resultFilePath = m_root.GetFileInfo().m_FileName;
		return &m_root;
	}

	//Start our recursive search.
	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), filePath);

	//Recurse dirs
	return GetFileEntry(
		resultFilePath, 
		absoluteFilePath, 
		0, 
		(C_MemoryDirectoryEntry*)m_root.GetData());
}

C_MemoryFileEntry* C_MemoryFsImpl::GetFileEntry(
	Sys::C_FilePath& resultFilePath,
	const Sys::C_FilePath& absoluteFilePath,
	U32 pathTruncation,
	C_MemoryDirectoryEntry* pParentDir)
{
	//Find current directory
	S32 index = absoluteFilePath.String().Search(Sys::GetLocalDirToken(), pathTruncation+2);

	Sys::C_FilePath truncatedPath;
	if (index == -1)
		truncatedPath = absoluteFilePath;
	else
	{
		++index;
		truncatedPath = absoluteFilePath.String()(0, index);
	}

	C_MemoryFileEntry* pEntry = pParentDir->GetFileEntry(
		Sys::ExtractFilename(Sys::RemoveTrailingSlash(truncatedPath)));
	if (!pEntry)
		return 0;

	if (pEntry->GetFileInfo().m_FileType == Sys::E_FileType_File)
	{
		resultFilePath = truncatedPath;
		return pEntry;
	}
	else if (pEntry->GetFileInfo().m_FileType != Sys::E_FileType_Directory)
	{
		BIZ_ASSERT_FAIL("Unknown type entry type!");
		return 0;
	}

	//BASE CASE
	//No more directories to recurse into.
	if (truncatedPath.Length() == absoluteFilePath.Length())
	{
		resultFilePath = truncatedPath;
		return pEntry;
	}

	//Continue to recurse...
	return GetFileEntry(
		resultFilePath,
		absoluteFilePath, 
		index, 
		(C_MemoryDirectoryEntry*)pEntry->GetData());
}

bool C_MemoryFsImpl::AddFile(Sys::C_FileInfo info, void* pData)
{
	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), info.m_FileName);

	//Recurse dirs
	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pFileEntry = GetFileEntry(resultFilePath, Sys::ExtractDirectory(absoluteFilePath));
	if (!pFileEntry || pFileEntry->GetFileInfo().m_FileType != Sys::E_FileType_Directory)
		return false;

	C_MemoryDirectoryEntry* pDir = (C_MemoryDirectoryEntry*)pFileEntry->GetData();
	//Remove preceding path.
	info.m_FileName = Sys::ExtractFilename(info.m_FileName);
	return pDir->AddFile(info, pData);
}

bool C_MemoryFsImpl::AddDirectory(Sys::C_FileInfo info)
{
	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), info.m_FileName);

	//Special case - no need to add root.
	if (info.m_FileName == Sys::C_FilePath(Sys::GetLocalDirToken()))
		return false;

	info.m_FileName = Sys::RemoveTrailingSlash(info.m_FileName);
	S32 prevSlash = info.m_FileName.String().RevSearch(Sys::GetLocalDirToken());
	if (prevSlash == -1)
		return false;

	Sys::C_FilePath parentDir = Sys::C_FilePath(info.m_FileName.String()(0, prevSlash+1));

	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pFileEntry = GetFileEntry(resultFilePath, parentDir);
	if (!pFileEntry || pFileEntry->GetFileInfo().m_FileType != Sys::E_FileType_Directory)
		return false;

	C_MemoryDirectoryEntry* pDir = (C_MemoryDirectoryEntry*)pFileEntry->GetData();
	//Remove preceding path.
	info.m_FileName = Sys::ExtractFilename(info.m_FileName);
	return pDir->AddDirectory(info);
}

bool C_MemoryFsImpl::AddEntry(Sys::C_FileInfo info, void* pData)
{
	if (info.m_FileType == Sys::E_FileType_Directory)
		return AddDirectory(info);
	else if (info.m_FileType == Sys::E_FileType_File)
		return AddFile(info, pData);

	return false;
}

bool C_MemoryFsImpl::DeleteFile(const Sys::C_FilePath& filePath)
{
	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), filePath);

	//Recurse dirs
	Sys::C_FilePath resultFilePath;
	C_MemoryFileEntry* pFileEntry = GetFileEntry(resultFilePath, Sys::ExtractDirectory(absoluteFilePath));
	if (!pFileEntry || pFileEntry->GetFileInfo().m_FileType != Sys::E_FileType_Directory)
		return false;

	C_MemoryDirectoryEntry* pDir = (C_MemoryDirectoryEntry*)pFileEntry->GetData();
	//Remove preceding path and delete...
	return pDir->DeleteFile(Sys::ExtractFilename(filePath));
}

}