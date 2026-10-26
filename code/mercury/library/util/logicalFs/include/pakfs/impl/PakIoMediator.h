#ifndef LOGICALFS_PAKFS_PAK_IO_MEDIATOR_H
#define LOGICALFS_PAKFS_PAK_IO_MEDIATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Threads/Mutex.h>

namespace Sys
{
	class BinarySerialiserAutoEndian;
}

namespace Pak
{

//Each read call must accept a position. We only have one read mediator per physical pak file. Multiple
//logical pak files can be accessed simultaniously by multiple threads, so the actual physical file
//pointer can be bouncing all over the place. So at each read we may potentially need to do a seek within
//the physical pak file.

class C_PakIoMediator
{
public:
	C_PakIoMediator();

	~C_PakIoMediator();

	bool Open(
		Sys::I_FileSystem* pFs,
		const Sys::C_FilePath& filePath,
		Sys::E_FileAccess access,
		const Sys::C_OpenParams& params);

    bool Read(void* pDest, U32 bytes, Sys::C_StreamPos* newPosition);

	bool Read(U32& value, Sys::C_StreamPos* newPosition);

	bool Read(U64& value, Sys::C_StreamPos* newPosition);

	void SetEndianSwapFlag(bool swap);

	bool GetEndianSwapFlag() const;

	void Close();

	bool SwitchFile(Sys::I_FileSystem* pFs, const Sys::C_FilePath& filePath, Sys::E_FileAccess access = Sys::E_FileAccess_Read);

	bool Reopen();
	
	void StopReads();

	void SetSectorSize(U32 sectorSize);

	U32 GetSectorSize() const;

	Sys::C_StreamPos GetReadPosition() const;

	bool UsingSerialAccess() const;

	Sys::I_FileSystem* GetFileSystem() const;

	const Btl::String& GetLowLevelName() const;

	const Sys::C_FilePath& GetFilePath() const;

#ifdef CFG_CONVERTER
	Sys::C_StreamPos GetWritePosition() const;

	void SetWritePosition(Sys::C_StreamPos newPosition);

	void SetWriteSector(U64 sector);

	bool Write(const void* pSrc, U32 bytes);

	bool Write(U32 value);

	bool Write(U64 value);

	bool IsSectorAligned();

	S64 Length() const;
#endif

private:
	U32					m_sectorSize;
	Sys::I_FileSystem*	m_pFs;
	Sys::C_FilePath		m_filePath;
	Btl::String			m_lowLevelName;
	Sys::I_File*		m_pFile;
	bool				m_usingSerialAccess;

	//Mutex to stop multiple threads reading from the underlying pak data at the same time.
	MT::Mutex			m_readMutex;
};

}

#endif
