#ifndef __SYSTEM_I_BINARYFILE_H__
#define __SYSTEM_I_BINARYFILE_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/FileSystemHelperTypes.h>

namespace Sys
{

// definition of a virtual interface that specifies a way to read and write data from a file
class I_File : public I_BinaryStream
{
public:
                                /// Returns the size of the file. returs 0 if the file is not open.
    virtual const C_StreamSize Length() = 0;
 
                               /// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
    virtual const C_StreamPos  GetPosition() const = 0;
 
                               /// Sets the current read / write position in the file.
                               /// This function will terminate with an error if the file is not open or newPosition is out of range.
                               /// This function will terminate with an error if the file is locked.
    virtual void               SetPosition( const C_StreamPos newPosition ) = 0;
 
                               /// Checks whether this file should be endian-swapped (this is not done by the I_File class)
    virtual const bool         GetEndianSwapFlag() const = 0;
                               /// Set the endian swap flag on or off
    virtual void               SetEndianSwapFlag( const bool swizzle ) = 0;

							   /// Flush the contents of the file to disk (only valid when writing)
	virtual void			   Flush() {};

	virtual ~I_File() {};
protected:
    I_File() {};               /// The user is not allowed to construct an I_File.
};

} // namespace Sys

#endif
