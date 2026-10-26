#ifndef __I_BINARYSTREAM_H__
#define __I_BINARYSTREAM_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core/containers/string.h>
#include <btl/utility/string.h>
#include <system/serialise/FileSystemHelperTypes.h>

namespace Sys
{

// definition of a virtual interface that specifies a way to read and write data
class I_BinaryStream
{
public:
	/// When an underflow occurs the current read attempt is passed in to allow the implementation to
	/// perform an efficient read straight into the target buffer, rather having to trigger multiple
	/// underflows for a read which is several times larger than the internal buffer. If an implementation
	/// does not support this it can ignore the context parameter completely, otherwise it should fill the
	/// bytesRead member with the number of bytes that were read into readTarget. The implementation should
	/// only do this if the readSize is large enough that doing this is worthwhile.
	struct UnderflowContext
	{
		U8* readTarget;
		U32 readSize;
		U32 bytesRead;

	public:
		UnderflowContext(U8* readTarget_ = 0, U32 readSize_ = 0) : readTarget(readTarget_), readSize(readSize_), bytesRead(0) { }
	};

public:
	// Need a virtual destructor for GNU compiler

	I_BinaryStream() : m_pCurBuffer(0), m_pBufferEnd(0) { }

	virtual ~I_BinaryStream() {};

// -- concrete facade (use these functions for reading/writing)
    /// Reads a block of data from the file. This function is implemented using Underflow/Overflow.
    /// This function will terminate with an error if the file is not open with read access.
	bool							    Read( void* pDest, U32 bytes );

    /// Write a block of data to the file. This function is implemented using Underflow/Overflow.
    /// This function will terminate with an error if the file is not open with write access.

    bool							    Write( const void* pSrc, U32 bytes );
// -- virtual interface
    /// Returns true if the stream is writing, or false if the stream is reading
    virtual const bool				    IsWriting() const = 0;

    /// Returns the currently open filename or the filename used to attempt to open the file.
    /// This is for debug purposes only, the format may differ depending on the filesystem. 
    virtual Btl::String					Name() const = 0;

	/// Returns true if the data should be endian-swapped after being read.
    virtual const bool					GetEndianSwapFlag() const = 0;
	/// Set the endian swap flag on or off.
    virtual void						SetEndianSwapFlag( const bool swizzle ) = 0;

	/// Returns the version number of the stream.
	virtual U32							GetVersionNumber() const = 0;
	/// Sets the version number of the stream.
	/// The caller should restore the original version number when it's section of the code is finished.
	virtual void						SetVersionNumber(U32 iVersion) = 0;

	/// Returns the current read/write position in bytes.
    virtual const C_StreamPos			GetPosition() const = 0;

//protected: // used internally to transfer binary data
    U8  *m_pCurBuffer;			// pointer to the current position in the buffer (increased by serializer) 
    U8  *m_pBufferEnd;          // pointer to the end of the buffer (used by serializer to detect buffer underflow/overflow)

    // refills the buffer by reading data
    virtual bool Underflow(UnderflowContext& ctx) = 0;

    // writes the output of the buffer and clears it
    virtual bool Overflow() = 0;
};

}

#include "I_BinaryStream.inl"

#endif
