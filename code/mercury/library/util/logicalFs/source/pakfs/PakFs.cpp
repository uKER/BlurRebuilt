#include "LogicalFsPch.h"

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakEntryFile.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Pak.h>
#include <PakFs/PakFs.h>
#include <PakFs/PakFsManager.h>

#if defined(PLATFORM_PC)
#include <wincrypt.h>
#elif defined(PLATFORM_PS3)
#include <cell/hash/libsha1.h>
#endif

#ifdef DEBUG_PAK_SYSTEM
#pragma optimize( "", off ) // DEBUG_PAK_SYSTEM
#endif

namespace LogicalFs
{

Debug::Logger& g_pakFsLog = Debug::GetLogger("mercury.pakfs");

C_PakFs::C_PakFs()
{
	m_pPak = NULL;
	m_flags.data = 0;
	if (C_PakFsManager::Exists())
		C_PakFsManager::Instance().OnPakFsCreated(this);
}

bool C_PakFs::OpenPak( 
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& filePath,
	const Security::I_Security* pSecurity,
	const Sys::C_OpenParams& params)
{
	m_pPak = NULL;
	m_flags.data = 0;

	//Read PAK header
	m_pPak = new Pak::C_PakFile();
	if (!m_pPak->pMediator->Open(pFs, filePath, Sys::E_FileAccess_Read, params))
	{
		SAFE_DELETE_PTR(m_pPak);
		return false;
	}
	// Validate Version
	if (Pak::ReadPak(*m_pPak, pSecurity))
	{
		m_flags.isOpen = true;
		m_flags.isModified = false;
		m_flags.isSecure = pSecurity ? true : false;
		return true;
	}

	SAFE_DELETE_PTR(m_pPak);
	return false;
}

bool C_PakFs::IsOpen()
{
	return m_flags.isOpen;
}

void C_PakFs::ClosePak()
{
	m_flags.isModified = false;
	SAFE_DELETE_PTR(m_pPak);
	m_flags.data = 0;
}

bool C_PakFs::CreateSingleDirectory( const Sys::C_FilePath& filePath )
{
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED!");
	return false;
}

Sys::I_File*	C_PakFs::Open(
	const Sys::C_FilePath& filePath,
	const Sys::E_FileAccess fileAccess,
	const Sys::C_OpenParams& params)
{
	//If we're not open, or trying to modify the contents of the pak, bail.
	if (fileAccess != Sys::E_FileAccess_Read)
		return 0;

	Pak::C_PakFatEntry* pFe = GetPakFatEntryAccountingForSerialAccess(filePath);
	if (!pFe || pFe->IsDirectory())
	{
		Sys::FileOpenError(filePath.Chars(), fileAccess != Sys::E_FileAccess_Read, params.m_IsCritical); 
		return 0;
	}

	// create new C_File object
	Pak::C_PakEntryFile* pPakEntryFile = new Pak::C_PakEntryFile();

	Pak::Sha1::C_HashContext* pHashContext;

	if (m_flags.isSecure)
	{
		if (fileAccess == Sys::E_FileAccess_Read)
		{
			Pak::C_PakFatEntrySecure* pFeSecure = (Pak::C_PakFatEntrySecure*)pFe;

			pHashContext = new Pak::Sha1::C_HashContext(true, pFeSecure->fileHash, sizeof(pFeSecure->fileHash));

		}
		else
		{
			pHashContext = new Pak::Sha1::C_HashContext(fileAccess == Sys::E_FileAccess_Read, NULL, 0);
		}
	}
	else
	{
		pHashContext = NULL;
	}

	// open file
	if (!pPakEntryFile->Open(
		m_pPak,
		pFe, 
		filePath, 
		fileAccess,
		params,
		pHashContext))
	{
		Sys::FileOpenError(filePath.Chars(), fileAccess != Sys::E_FileAccess_Read, params.m_IsCritical); 
		// file failed to open - delete file object and return NULL
		SAFE_DELETE_PTR(pPakEntryFile);
		return 0;
	}

	return pPakEntryFile;
}

C_PakFs::~C_PakFs()
{
	if (C_PakFsManager::Exists())
		C_PakFsManager::Instance().OnPakFsDestroy(this);

	ClosePak();
}

bool C_PakFs::FileExists( const Sys::C_FilePath &filePath )
{
	return IsOpen() && GetPakFatEntryAccountingForSerialAccess(filePath) != 0;
}

bool C_PakFs::FileInfo( 
	const Sys::C_FilePath &filePath_,
	Sys::C_FileInfo* pFileInfo)
{
	if (IsOpen())
	{
		Sys::C_FilePath filePath = filePath_.Length() > 0 ? filePath_ : Sys::GetLocalDirToken();

		Pak::C_PakFatEntry* pFe = GetPakFatEntryAccountingForSerialAccess(filePath.Chars());
		if (pFe)
		{
			pFileInfo->m_FileType = pFe->IsDirectory() ? Sys::E_FileType_Directory : Sys::E_FileType_File;
			pFileInfo->m_FileName = (const char*)(m_pPak->stringTable.pData + pFe->filePathStrTableOffset);
			pFileInfo->m_FileSize = pFe->dataLogicalLengthBytes;
			pFileInfo->m_FileAccess = Sys::E_FileAccess_Read;
			pFileInfo->m_FileTime = pFe->IsDirectory() ? m_pPak->header.fileTime : pFe->fileTime;
			return true;
		}
	}
	return false;
}

bool C_PakFs::FileAccess( const Sys::C_FilePath &fileName_, U32 fileAccess )
{  
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED!");
	return false;
}

bool C_PakFs::DeleteFile( const Sys::C_FilePath& fileName_ )
{
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED!");
	return false;
}

bool C_PakFs::DirectoryInfo( const Sys::C_FilePath& filePath_, Sys::C_DirectoryInfo* pDir )
{
	if (IsOpen())
	{
		Sys::C_FilePath filePath(filePath_);
		filePath += Sys::GetLocalDirToken();

		Sys::C_FilePath pakAbsolutePath = 
			Sys::ConvertToCurrentDirectory( GetWorkingDirectory(), filePath);

		//Prepend our folders with our pak path.
		if (pakAbsolutePath.Length() > 0)
		{
			Pak::C_PakFatEntry* pParentFatEntry = m_pPak->GetFatEntry(pakAbsolutePath);
			//Bail if we've no knowledge of this folder.
			if (!pParentFatEntry)
				return false;
			for (U32 i = 0; i < m_pPak->nFatEntries; ++i)
			{
				Pak::C_PakFatEntry* pCurFatEntry = m_pPak->GetFatEntryByIdx(i);
				if (pCurFatEntry->parentDirectoryOffsetBytes != PAK_FAT_ENTRY_NO_PARENT)
				{
					Pak::C_PakFatEntry* pCurFatEntryParent = (Pak::C_PakFatEntry*)(m_pPak->pFatEntries+pCurFatEntry->parentDirectoryOffsetBytes);
					if (pCurFatEntryParent == pParentFatEntry)
					{
						pDir->FileInfoArray().PushBack();
						Sys::C_FileInfo& fileInfo = pDir->FileInfoArray().Back();
						fileInfo.m_FileType = pCurFatEntry->IsDirectory() ? Sys::E_FileType_Directory : Sys::E_FileType_File;
						fileInfo.m_FileName = (const char*)(m_pPak->stringTable.pData + pCurFatEntry->filePathStrTableOffset);
						fileInfo.m_FileSize = pCurFatEntry->dataLogicalLengthBytes;
						fileInfo.m_FileAccess = Sys::E_FileAccess_Read;
						fileInfo.m_FileTime = pCurFatEntry->fileTime;
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool C_PakFs::SetWorkingDirectory( const Sys::C_FilePath& directoryPath )
{
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED!");
	return false;
}

Sys::C_FilePath C_PakFs::GetWorkingDirectory() const
{
	return Sys::GetLocalDirToken();
}

Sys::C_FilePath C_PakFs::GetSystemFilePath( const Sys::C_FilePath& filePath ) const
{
	return Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), filePath);
}

const Sys::C_FilePath& C_PakFs::GetPakFilePath() const
{
	return m_pPak->pMediator->GetFilePath();
}

bool C_PakFs::SwitchPakFile(
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& filePath)
{
	if (!m_pPak)
		return false;
	m_flags.isOpen = m_pPak->pMediator->SwitchFile(pFs, filePath);
	return m_flags.isOpen;
}

bool C_PakFs::Resume()
{
	m_flags.isOpen = m_pPak->pMediator->Reopen();
	return m_flags.isOpen;
}

void C_PakFs::Suspend()
{
	if (!m_pPak)
		return;
	m_pPak->pMediator->Close();
}

void C_PakFs::StopReads()
{
	if (!m_pPak)
		return;
	m_pPak->pMediator->StopReads();
}

bool C_PakFs::ReopenPakFile()
{
	if (!m_pPak)
		return false;
	return m_pPak->pMediator->Reopen();
}

Pak::C_PakFatEntry* C_PakFs::GetPakFatEntryAccountingForSerialAccess(const Sys::C_FilePath& fileName)
{
	Pak::C_PakFatEntry* pFe = m_pPak->GetFatEntry(fileName);
	if (pFe && !pFe->IsDirectory())
	{
		if (!m_pPak->pMediator)
			return NULL;

		//If we're only providing serial access, disallow reverse seeks.
		if (m_pPak->pMediator->UsingSerialAccess())
		{
			U64 dataStartInBytes = pFe->dataStartSector * m_pPak->pMediator->GetSectorSize();
			if (m_pPak->pMediator->GetReadPosition() > dataStartInBytes)
			{
				#ifdef LOGICAL_FS_DEBUG
				LOGINFO(g_log, "Ignoring random access request %s from serial pak %s", 
					fileName.Chars(), m_pPak->pMediator->GetLowLevelName().Chars());
				#endif
				return NULL;
			}
		}
	}

	return pFe;
}

bool C_PakFs::FileTime( const Sys::C_FilePath& _fileName, const Time::C_Time &_time )
{
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED!");
	return false;
}

}
