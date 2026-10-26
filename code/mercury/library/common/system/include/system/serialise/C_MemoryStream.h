#ifndef __SYSTEM_SERIALISE_C_MemoryStream_H__
#define __SYSTEM_SERIALISE_C_MemoryStream_H__

#pragma once

namespace Sys
{

class C_MemoryStream : public I_BinaryStream
{
public:
									/// Constructor for writing to a buffer
									/// argument is the step size with which the buffer increases
									C_MemoryStream(int iBufferExpandSize);	
									/// constructor for reading from a given buffer
									C_MemoryStream(U8 *pData, U32 cData);
									/// constructor for reading where the buffer will be set later
									C_MemoryStream();

	void							SetBuffer(U8 *pData, U32 cData);

									/// Returns the size of the buffer
	virtual U64						GetBufferSize() const;

									/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
    virtual const C_StreamPos		GetPosition() const;

									/// Returns the address of the buffer
	virtual U8						*GetBufferPtr() const;

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

									/// This can be called to take ownership of the memory buffer which was 'new'ed by C_MemoryStream
	void							TakeOwnership();

									/// destructor
	virtual							~C_MemoryStream();

private:
	// local data buffer
	U8				*m_pBuffer;
	int				m_iBufferSize;
	int				m_iBufferExpandSize;

	// flags
	bool			m_bWriting;
	bool			m_bEndianSwapFlag;
	int				m_iVersion;

	Btl::String		m_Name;
};

};

#endif
