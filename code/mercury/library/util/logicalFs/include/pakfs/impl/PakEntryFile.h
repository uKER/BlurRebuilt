#ifndef LOGICALFS_PAKFS_PAK_ENTRY_FILE_H
#define LOGICALFS_PAKFS_PAK_ENTRY_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <PakFs/Impl/PakFatEntry.h>

namespace Pak
{
class C_PakFile;
class I_PakCompress;
union C_PakFlags;

namespace Sha1
{
	class C_HashContext;
}

class C_PakEntryFile : public Sys::I_File
{
//From Sys::I_File
public:
	virtual const Sys::C_StreamSize	Length();

    virtual const Sys::C_StreamPos	GetPosition() const;
 
    virtual void					SetPosition( const Sys::C_StreamPos newPosition );

    virtual const bool              IsWriting() const;

    virtual Btl::String				Name() const;

    virtual bool					Underflow(UnderflowContext& ctx);

    virtual bool					Overflow();

    virtual const bool				GetEndianSwapFlag() const;

    virtual void					SetEndianSwapFlag( const bool swizzle );

	virtual U32						GetVersionNumber() const;
	virtual void					SetVersionNumber( U32 );

	virtual void					Flush();

	virtual							~C_PakEntryFile();

//From C_PakEntryFile
public:
	C_PakEntryFile();

	///Open a file.
	bool Open(
		C_PakFile* pPak,
		const C_PakFatEntry* pPakFatEntry,
		const Sys::C_FilePath& fileName, 
		const U32 fileAccessAttributes,
		const Sys::C_OpenParams& params = Sys::C_OpenParams(),
		Sha1::C_HashContext* pHashContext = NULL);

	///Close the file.
	virtual void Close();

	/// Returns true if file is currently open.
    virtual const bool IsOpen() const;

private:
	//Data specific to buffered io
	struct C_BufferedIo
	{
		// local data buffer
		U8*					m_pBuffer;
		U32					m_bufferSize;
		// offset of buffer start of buffer in file
		Sys::C_StreamSize	m_iBufferOffset;
	};

	//Data specific to unbuffered io
	struct C_UnbufferedIo
	{
		Sys::C_StreamSize	m_unbufferedFilePos;
	};


	//Common data//

	const C_PakFatEntry*	m_pPakEntry;
	C_PakFile*				m_pPak;
	I_PakCompress*			m_pPakCompress;

	Sha1::C_HashContext*	m_pHashContext;

	// name
	Sys::C_FilePath			m_Name;

	// file handle
#ifndef PLATFORM_PS3
	FILE*					m_f;
#else // PLATFORM_PS3
	int						m_f;
#endif

	U32						m_iVersion;
	// Store length if file is read-only, store once
	Sys::C_StreamSize		m_Length;

	//Flags
	union
	{
		struct
		{
			bool m_bUnbuffered : 1;
			bool m_bWriting : 1;
			bool m_bIsOpen : 1;
			bool m_bEndianSwapFlag : 1;
			bool m_assertEof : 1;
		};
		U32 m_flags;
	};


	//Access mode specific data//
	union
	{
		C_UnbufferedIo	m_unbufIo;
		C_BufferedIo	m_bufIo;
	};
};

}

#endif
