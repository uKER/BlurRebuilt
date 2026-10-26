#ifndef __SYSTEM_SERIALISE_FILEBLOCK_H__
#define __SYSTEM_SERIALISE_FILEBLOCK_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "i_file.h"

namespace Sys
{

typedef Util::C_String8 C_BlockHeaderId;

enum E_BlockHeaderType
{
    E_StreamHeaderType_Invalid = -1,
    E_StreamHeaderType_Data = 0x41,
    E_StreamHeaderType_Header = 0x43
};

/// header info for a C_FileBlock
class C_BlockHeader
{
public:
    C_BlockHeader()
    {
        m_Id = "NO_ID";
        m_Size = 0;     
        m_Count = 0;
        m_Type = E_StreamHeaderType_Invalid;
        m_CompressionType = 0;
    }

	// read blockheader
	template < class Archive >
	void Deserialise( Archive& stream )
	{
		stream >> m_Id;
		stream >> m_Size;
		stream >> m_Count;
		stream >> m_Type;
		stream >> m_CompressionType;
	}

	// save blockheader
	template < class Archive >
	void Serialise( Archive& stream )
	{
		stream << m_Id;
		stream << m_Size;
		stream << m_Count;
		stream << m_Type;
		stream << m_CompressionType;
	}
public:
    C_BlockHeaderId         m_Id;
    U32                     m_Size; // Size in bytes from start of header to end of data
    U16                     m_Count;
    S8                      m_Type; // E_StreamHeaderType
    S8                      m_CompressionType;    
};



/// structure a file using a block hierarchy. facilitates backwards and forwards compatibility
class C_FileBlock : public I_File
{
private:
									/// prevent accidental copying
									C_FileBlock( C_FileBlock& other );
public:
									/// construct from a file
									C_FileBlock(I_File *pFile);
									/// construct from a parent block
									C_FileBlock(C_FileBlock *pParent);
									/// construct from a file and set name
									C_FileBlock(I_File *pFile, const Util::C_String name);
									/// construct from a parent block and set name
									C_FileBlock(C_FileBlock *pParent, const Util::C_String name);
									/// destructor
									~C_FileBlock();

// interface for iterating the file block
									/// return the block header information
	const C_BlockHeader				&GetBlockHeader() const;

	const C_BlockHeaderId			&HeaderId() const;

// implementation of I_File
									/// Returns true if the stream is writing, or false if the stream is reading
    virtual const bool				IsWriting() const;

									/// Returns the currently open filename or the filename used to attempt to open the file.
									/// This is for debug purposes only, the format may differ depending on the filesystem. 
    virtual Btl::String				Name() const;

									/// Returns true if the data should be endian-swapped after being read.
    virtual const bool				GetEndianSwapFlag() const;
									/// Set the endian swap flag on or off.
    virtual void					SetEndianSwapFlag( const bool swizzle );

									/// Returns the version number of the stream.
	virtual U32						GetVersionNumber() const;
									/// Sets the version number of the stream.
									/// The caller should restore the original version number when it's section of the code is finished.
	virtual void					SetVersionNumber(U32 iVersion);

									// refills the buffer by reading data
    virtual bool					Underflow(UnderflowContext& ctx);

									// writes the output of the buffer and clears it
    virtual bool					Overflow();

									/// Returns the size of the file. returs 0 if the file is not open.
    virtual const C_StreamSize		Length();
 
									/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
    virtual const C_StreamPos		GetPosition() const;

	virtual void					Flush();
 
									/// Sets the current read / write position in the file.
									/// This function will terminate with an error if the file is not open or newPosition is out of range.
									/// This function will terminate with an error if the file is locked.
    virtual void					SetPosition( const C_StreamPos newPosition );

protected:
									/// open block header from the file. called by constructor
	void							OpenBlock();

									/// close block. called by destructor
	void							CloseBlock();

									/// signal that one of the child blocks was read or written
	void							AccessChildBlock();

									/// synchronize pointers with parent I_File
	inline void						GetPointers()
	{
		m_pCurBuffer = m_pParent->m_pCurBuffer;
		m_pBufferEnd = m_pParent->m_pBufferEnd;
	}

									/// synchronize pointers with parent I_File
	inline void						SetPointers()
	{
		m_pParent->m_pCurBuffer = m_pCurBuffer;
		m_pParent->m_pBufferEnd = m_pBufferEnd;
	}

private:
	I_File							*m_pParent;

	C_BlockHeader					m_BlockHeader;
	int								m_nBlocks;
	U64								m_iHeaderPosition;

	mutable Btl::String	m_Name;
};

}

#endif
