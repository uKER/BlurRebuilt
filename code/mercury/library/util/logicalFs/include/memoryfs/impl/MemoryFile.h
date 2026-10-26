#ifndef SYSTEM_MEMORY_FILE_H
#define SYSTEM_MEMORY_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace LogicalFs
{

class C_MemoryFs;

class C_MemoryFile : public Sys::I_File
{
public:
	friend class C_FileSystem;

									/// Returns the size of the file. returs 0 if the file is not open.
    virtual const Sys::C_StreamSize		Length();

									/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
    virtual const Sys::C_StreamPos		GetPosition() const;
 
									/// Sets the current read / write position in the file.
									/// This function will terminate with an error if the file is not open or newPosition is out of range.
									/// This function will terminate with an error if the file is locked.
    virtual void					SetPosition( const Sys::C_StreamPos newPosition );

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

									/// destructor
	virtual							~C_MemoryFile();

									/// Constructor
									C_MemoryFile();

									/// open a file
	virtual bool					Open(
										const void* pData, U32 size,
										const Btl::String& filename,
										const Sys::C_OpenParams& params);

									/// Returns true if file is currently open.
    virtual const bool				IsOpen() const;

	void							ReleaseFromFsAfterClosed(C_MemoryFs* pMemoryFs);
 
private:
	U8*					m_pData;
	U32					m_length;
	S32					m_iVersion;
	Btl::String			m_Name;
	C_MemoryFs*			m_pFsToDeleteFromAfterClosed;

	//Flags
	union
	{
		struct
		{
			bool m_bWriting : 1;
			bool m_bIsOpen : 1;
			bool m_bEndianSwapFlag : 1;
		};
		U32 m_flags;
	};
};

};

#endif
