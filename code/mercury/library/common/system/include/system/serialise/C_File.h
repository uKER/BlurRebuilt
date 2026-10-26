#ifndef __SYSTEM_SERIALISE_C_FILE_H__
#define __SYSTEM_SERIALISE_C_FILE_H__

#pragma once

#if defined(PLATFORM_PC)
#include <windows.h>	// [Blur2PC] HANDLE m_f below; the header was relying on the including PCH
#endif

namespace Sys
{

#define DEFAULT_FILE_STREAM_BUFFER_SIZE 16*1024

class C_File : public I_File
{
public:
	friend class C_FileSystem;

									/// Returns the size of the file. returs 0 if the file is not open.
    virtual const C_StreamSize		Length();

									/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
    virtual const C_StreamPos		GetPosition() const;
 
									/// Sets the current read / write position in the file.
									/// This function will terminate with an error if the file is not open or newPosition is out of range.
									/// This function will terminate with an error if the file is locked.
    virtual void					SetPosition( const C_StreamPos newPosition );

									/// Returns true if the stream is writing, or false if the stream is reading
    virtual const bool              IsWriting() const;

									/// Returns the currently open filename or the filename used to attempt to open the file.
									/// This is for debug purposes only, the format may differ depending on the filesystem. 
    virtual Btl::String				Name() const;

									/// refills the buffer by reading data
    virtual bool					Underflow(UnderflowContext& ctx);

									/// writes the output of the buffer and clears it
    virtual bool					Overflow();

									/// Checks whether this file should be endian-swapped (this is not done by the I_File class)
    virtual const bool				GetEndianSwapFlag() const;

									/// Set the endian swap flag on or off
    virtual void					SetEndianSwapFlag( const bool swizzle );

	virtual U32						GetVersionNumber() const;
	virtual void					SetVersionNumber( U32 );

	virtual void					Flush();

									/// destructor
	virtual							~C_File();

protected:
	// these functions are only to be used by C_FileSystem
									/// Constructor
									C_File();

									/// open a file
	bool							Open( 
		const C_FilePath& fileName, 
		const U32 fileAccessAttributes,
		bool isCritical,
		U32 bufferSize = DEFAULT_FILE_STREAM_BUFFER_SIZE );
									/// close the file
	virtual void					Close();

									/// Returns true if file is currently open.
    virtual const bool				IsOpen() const;
 

private:
	bool DoFileRead(U8* data, U32 size, U32* nrRead);

private:
	// local data buffer
	U8*				buffer;
	// file handle
#if defined(PLATFORM_PC)
	HANDLE			m_f;
#elif defined(PLATFORM_PS3)
	int				m_f;
#else
	FILE			*m_f;
#endif

	// offset of buffer start of buffer in file
	Sys::C_StreamPos	m_iBufferOffset;
	// name
	Btl::String		m_Name;

	// flags
	bool			m_bWriting;
	bool			m_bEndianSwapFlag;
	int				m_iVersion;
	bool			m_bIsOpen;
	// Store length if file is read-only, store once
	C_StreamSize	m_Length;
	bool			m_IsCritical;
};

};

#endif
