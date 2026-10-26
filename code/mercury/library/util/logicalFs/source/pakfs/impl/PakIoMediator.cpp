#include "LogicalFsPch.h"
#include <System/Debug/Debug.h>

#include <system/serialise/FsPerfLogger.h>

#include <System/Serialise/BinarySerialiseAutoEndian.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakUtil.h>

namespace Pak
{

#ifdef LOGICAL_FS_DEBUG
Debug::Bool g_breakOnPakRewind("Util.LogicalFs.BreakOnPakRewind", false, Debug::Persist());
Debug::String g_breakOnPakRewindPakName("Util.LogicalFs.BreakOnPakRewindPakName", "game:\\cache\\boot.pak", Debug::Persist());
Debug::Int g_breakOnPos("Util.LogicalFs.BreakOnPos", -1, Debug::Persist());
#endif

C_PakIoMediator::C_PakIoMediator()
{
	m_sectorSize = PAK_SECTOR_SIZE_DEFAULT;
	m_pFs = NULL;
	m_pFile = NULL;
	m_usingSerialAccess = false;
}

C_PakIoMediator::~C_PakIoMediator()
{
	Close();
}

bool C_PakIoMediator::Open(
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& filePath,
	Sys::E_FileAccess access,
	const Sys::C_OpenParams& params)
{
	m_pFs = pFs;
	m_filePath = filePath;
	m_pFile = pFs->Open(m_filePath, access, params);
	if (!m_pFile)
	{
		LOGERROR(g_log, "C_PakIoMediator failed to open file %s!", filePath.Chars());
		return false;
	}
	m_lowLevelName = m_pFile->Name();
	m_usingSerialAccess = params.m_SerialAccess;
	return true;
}

bool C_PakIoMediator::Read(void* pDest, U32 bytes, Sys::C_StreamPos* newPosition)
{
	m_readMutex.Wait();

	U64 curPos = m_pFile->GetPosition();
	if (*newPosition != curPos)
	{
#ifdef LOGICAL_FS_DEBUG
		if (g_breakOnPakRewind && *newPosition < curPos && !stricmp((const char*)g_breakOnPakRewindPakName, m_pFile->Name().Chars()))
			LOGWARN(g_log, "C_PakIoMediator::Read pak rewind from %u to %u seek by %u", (U32)curPos, (U32)(*newPosition), (U32)(curPos - *newPosition));
#endif
		m_pFile->SetPosition(*newPosition);
	}
	// Handle the case where we ask for data that could potentially go off the end of the file
	if (bytes > (m_pFile->Length() - m_pFile->GetPosition()))
		bytes = (U32)(m_pFile->Length() - m_pFile->GetPosition());
	bool status = m_pFile->Read(pDest, bytes);
	if (!status)
	{
		LOGERROR(g_log, "C_PakIoMediator read failure!");
	}
	*newPosition = m_pFile->GetPosition();
#ifdef LOGICAL_FS_DEBUG
	if (g_breakOnPos == (U32)*newPosition)
	{
		LOGINFO(g_log, "BreakOnPos");
	}
#endif
	m_readMutex.Release();
	return status;
}

bool C_PakIoMediator::Read(U32& value, Sys::C_StreamPos* newPosition)
{
	m_readMutex.Wait();
	U64 curPos = m_pFile->GetPosition();
	if (*newPosition != curPos)
	{
#ifdef LOGICAL_FS_DEBUG
		if (g_breakOnPakRewind && *newPosition < curPos && !stricmp((const char*)g_breakOnPakRewindPakName, m_pFile->Name().Chars()))
			LOGWARN(g_log, "C_PakIoMediator::Read pak rewind from %u to %u seek by %u", (U32)curPos, (U32)(*newPosition), (U32)(curPos - *newPosition));
#endif
		m_pFile->SetPosition(*newPosition);
	}
	bool status = m_pFile->Read(&value, sizeof(value));
	if (status)
	{
		if(m_pFile->GetEndianSwapFlag())
			Ut::DoEndianSwap::EndianSwap(value);
	}
	else
	{
		LOGERROR(g_log, "C_PakIoMediator read failure!");
	}

	*newPosition = m_pFile->GetPosition();
#ifdef LOGICAL_FS_DEBUG
	if (g_breakOnPos == (U32)*newPosition)
	{
		LOGINFO(g_log, "BreakOnPos");
	}
#endif
	m_readMutex.Release();
	return status;
}

bool C_PakIoMediator::Read(U64& value, Sys::C_StreamPos* newPosition)
{
	m_readMutex.Wait();
	U64 curPos = m_pFile->GetPosition();
	if (*newPosition != curPos)
	{
#ifdef LOGICAL_FS_DEBUG
		if (g_breakOnPakRewind && *newPosition < curPos && !stricmp((const char*)g_breakOnPakRewindPakName, m_pFile->Name().Chars()))
			LOGWARN(g_log, "C_PakIoMediator::Read pak rewind from %u to %u seek by %u", (U32)curPos, (U32)(*newPosition), (U32)(curPos - *newPosition));
#endif
		m_pFile->SetPosition(*newPosition);
	}
	bool status = m_pFile->Read(&value, sizeof(value));
	if (status)
	{
		if(m_pFile->GetEndianSwapFlag())
			Ut::DoEndianSwap::EndianSwap(value);
	}
	else
	{
		LOGERROR(g_log, "C_PakIoMediator read failure!");
	}

	*newPosition = m_pFile->GetPosition();
#ifdef LOGICAL_FS_DEBUG
	if (g_breakOnPos == (U32)*newPosition)
	{
		LOGINFO(g_log, "BreakOnPos");
	}
#endif
	m_readMutex.Release();
	return status;
}

void C_PakIoMediator::SetEndianSwapFlag(bool swap)
{
	m_pFile->SetEndianSwapFlag(swap);
}

bool C_PakIoMediator::GetEndianSwapFlag() const
{
	return m_pFile->GetEndianSwapFlag();
}

bool C_PakIoMediator::SwitchFile(
	Sys::I_FileSystem* pFs, const Sys::C_FilePath& filePath, Sys::E_FileAccess access)
{
	MT::C_MutexLock lock(&m_readMutex);

	bool result;

	//Allow reopening of the same file on 360 incase transparent caching doesn't allow newly cached files
	if (m_filePath == filePath)
	{
		LOGINFO(g_log, "C_PakIoMediator no need to switch from %s!", filePath.Chars());
		return true;
	}

	LOGINFO(g_log, "C_PakIoMediator switching from %s to %s!", m_pFile->Name().Chars(), filePath.Chars());

	Sys::I_File* pNewFile = pFs->Open(filePath, access);
	if (pNewFile)
	{
		FSPERF_RENAME_FS(m_lowLevelName.Chars(), pNewFile->Name())
		//Switch if successful
		SAFE_DELETE_PTR(m_pFile);

		m_pFs = pFs;
		m_pFile = pNewFile;
		m_lowLevelName = m_pFile->Name();
		result = true;
	}
	else
	{
		//Cannot switch. Will continue to use original file.
		LOGERROR(g_log, "C_PakIoMediator failed to switch to file %s!", filePath.Chars());
		result = false;
	}

	return result;
}

bool C_PakIoMediator::Reopen()
{
	return SwitchFile(m_pFs, m_filePath);
}

void C_PakIoMediator::StopReads()
{
	m_readMutex.Wait();
}

void C_PakIoMediator::Close()
{
	m_readMutex.Wait();
	if (m_pFile)
	{
		SAFE_DELETE_PTR(m_pFile);
	}
	else
	{
		m_pFile = 0;
	}
	m_lowLevelName = "";
	m_readMutex.Release();
}

void C_PakIoMediator::SetSectorSize(U32 sectorSize)
{
	m_sectorSize = sectorSize;
}

U32 C_PakIoMediator::GetSectorSize() const
{
	return m_sectorSize;
}

Sys::C_StreamPos C_PakIoMediator::GetReadPosition() const
{
	if (m_pFile)
		return m_pFile->GetPosition();
	return 0;
}

bool C_PakIoMediator::UsingSerialAccess() const
{
	return m_usingSerialAccess;
}

Sys::I_FileSystem* C_PakIoMediator::GetFileSystem() const
{
	return m_pFs;
}

const Btl::String& C_PakIoMediator::GetLowLevelName() const
{
	return m_lowLevelName;
}

const Sys::C_FilePath& C_PakIoMediator::GetFilePath() const
{
	return m_filePath;
}

#ifdef CFG_CONVERTER
Sys::C_StreamPos C_PakIoMediator::GetWritePosition() const
{
	return (U32)m_pFile->GetPosition();
}

void C_PakIoMediator::SetWritePosition(Sys::C_StreamPos newPosition)
{
	m_pFile->SetPosition(newPosition);
}

void C_PakIoMediator::SetWriteSector(U64 sector)
{
	SetWritePosition(sector*m_sectorSize);
}

bool C_PakIoMediator::Write(const void* pSrc, U32 bytes)
{
	bool status = m_pFile->Write(pSrc, bytes);
	if (!status)
		LOGERROR(g_log, "C_PakIoMediator write failure!");
	return status;
}

bool C_PakIoMediator::Write(U32 value)
{
	if(m_pFile->GetEndianSwapFlag())
		Ut::DoEndianSwap::EndianSwap(value);

	bool status = m_pFile->Write(&value, sizeof(value));
	if (!status)
		LOGERROR(g_log, "C_PakIoMediator write failure!");
	return status;
}

bool C_PakIoMediator::Write(U64 value)
{
	if(m_pFile->GetEndianSwapFlag())
		Ut::DoEndianSwap::EndianSwap(value);

	bool status = m_pFile->Write(&value, sizeof(value));
	if (!status)
		LOGERROR(g_log, "C_PakIoMediator write failure!");
	return status;
}

bool C_PakIoMediator::IsSectorAligned()
{
	BIZ_ASSERT(m_pFile);
	return IsFilePosSectorAligned(*m_pFile, m_sectorSize);
}

S64 C_PakIoMediator::Length() const
{
	return m_pFile ? m_pFile->Length() : 0;
}

#endif

}
