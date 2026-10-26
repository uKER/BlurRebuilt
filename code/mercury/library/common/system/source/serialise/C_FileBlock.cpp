#include "SystemPch.h"
#include <core/core.h>
#include <core/utility/endian.h>
#include <system/debuglog.h>
#include "system/serialise.h"
#include "system/serialise/C_FileBlock.h"

namespace Sys
{
   
static Debug::Logger &g_Log = Debug::GetLogger("mercury.serialise.fileblock");

/// construct with file
C_FileBlock::C_FileBlock(I_File *pFile) :
	m_pParent(pFile),
	m_nBlocks(0)
{
	OpenBlock();
}

/// construct with parent block
C_FileBlock::C_FileBlock(C_FileBlock *pParent) :
	m_pParent(pParent),
	m_nBlocks(0)
{
	OpenBlock();

	pParent->AccessChildBlock();
}

/// construct from a file and set name
C_FileBlock::C_FileBlock(I_File *pFile, const Util::C_String name) :
	m_pParent(pFile),
	m_nBlocks(0)
{
	m_BlockHeader.m_Id = name;
	assert(name.Length() <= 7);	// block names longer than 7 characters will be truncated

	OpenBlock();
}

/// construct from a parent block and set name
C_FileBlock::C_FileBlock(C_FileBlock *pParent, const Util::C_String name) :
	m_pParent(pParent),
	m_nBlocks(0)
{
	m_BlockHeader.m_Id = name;
	assert(name.Length() <= 7);	// block names longer than 7 characters will be truncated

	OpenBlock();

	pParent->AccessChildBlock();
}

/// destructor
C_FileBlock::~C_FileBlock()
{
	CloseBlock();
}

// interface for iterating the file block
/// return the block header information
const C_BlockHeader &C_FileBlock::GetBlockHeader() const
{
	return m_BlockHeader;
}

const C_BlockHeaderId &C_FileBlock::HeaderId() const
{
	return m_BlockHeader.m_Id;
}


/// open block header from the file. called by constructor
void C_FileBlock::OpenBlock()
{
	m_iHeaderPosition = m_pParent->GetPosition();

	if (!IsWriting())
	{
		// read the header
		if (GetEndianSwapFlag())
		{
			BinaryDeserialiser<Ut::DoEndianSwap> ar(*m_pParent);
			ar >> m_BlockHeader;
		}
		else
		{
			BinaryDeserialiser<Ut::NoEndianSwap> ar(*m_pParent);
			ar >> m_BlockHeader;
		}
		m_nBlocks = m_BlockHeader.m_Count;
	}
	else
	{
		// write empty space where the header will go. will be filled in in 'CloseBlock' later on.
		m_pParent->Write(&m_BlockHeader, sizeof(m_BlockHeader));
	}

	GetPointers();
}

/// signal that one of the child blocks was read or written
void C_FileBlock::AccessChildBlock()
{
	if (!IsWriting())
	{
		// [PaulM] put this 'if' in here as we were getting a debug assert when reading a block with just data in it
		if(m_BlockHeader.m_Count!=0)
		{
			m_nBlocks--;

			// asserts here if we are trying to read another block, which doesn't exist. should never get here.
			// at this point we should have already received a warning about reading past the end of the parent block.
#ifndef PLATFORM_PS3
			assert(m_nBlocks>=0);
#endif
		}
	}
	else
	{
		m_BlockHeader.m_Count++;
	}
}

/// close block. called by destructor
void C_FileBlock::CloseBlock()
{
	SetPointers();

	if (!IsWriting())
	{
		U64 iPos = GetPosition();
		// check for block underrun
		if (iPos < m_BlockHeader.m_Size)
		{
			LOGWARN( g_Log, "Note: %d bytes unread at end of block %s.\n", m_BlockHeader.m_Size - iPos, Name().Chars());
 			SetPosition(m_BlockHeader.m_Size);
		}
		// check for block overrun
		if (iPos > m_BlockHeader.m_Size)
		{
			LOGWARN( g_Log, "The program has read %d bytes past the end of block %s.\n", iPos - m_BlockHeader.m_Size, Name().Chars());
			SetPosition(m_BlockHeader.m_Size);
		}
	}
	else
	{
		Overflow();

		// fill in the block header structure
		m_BlockHeader.m_Size = U32(GetPosition());
		m_BlockHeader.m_Type = (m_BlockHeader.m_Count>0)?E_StreamHeaderType_Header:E_StreamHeaderType_Data;

		// scan back to where header is supposed to go
		U64 iOrgFilePos = m_pParent->GetPosition();
		m_pParent->SetPosition(m_iHeaderPosition);

		// write the header
		if (GetEndianSwapFlag())
		{
			BinarySerialiser<Ut::DoEndianSwap> ar(*m_pParent);
			ar << m_BlockHeader;
		}
		else
		{
			BinarySerialiser<Ut::NoEndianSwap> ar(*m_pParent);
			ar << m_BlockHeader;
		}

		m_pParent->SetPosition(iOrgFilePos);
	}
}

// the following functions drop straight through to the parent I_File class

/// Returns true if the stream is writing, or false if the stream is reading
const bool C_FileBlock::IsWriting() const
{
	return m_pParent->IsWriting();
}

/// Returns the currently open filename or the filename used to attempt to open the file.
/// This is for debug purposes only, the format may differ depending on the filesystem. 
Btl::String C_FileBlock::Name() const
{
	if(m_pParent)
	{
		m_Name.Printf( "%s [%s]", m_pParent->Name().Chars(), m_BlockHeader.m_Id.Chars());
	}
	else
	{
		m_Name.Printf( "Unknown [%s]", m_BlockHeader.m_Id.Chars());
	}
	return m_Name;
}

/// Returns true if the data should be endian-swapped after being read.
const bool C_FileBlock::GetEndianSwapFlag() const
{
	assert(m_pParent);
	return m_pParent->GetEndianSwapFlag();
}

/// Set the endian swap flag on or off.
void C_FileBlock::SetEndianSwapFlag( const bool swizzle )
{
	assert(m_pParent);
	m_pParent->SetEndianSwapFlag( swizzle );
}

/// Returns the version number of the stream.
U32	C_FileBlock::GetVersionNumber() const
{
	assert(m_pParent);
	return m_pParent->GetVersionNumber();
}

/// Sets the version number of the stream.
/// The caller should restore the original version number when it's section of the code is finished.
void C_FileBlock::SetVersionNumber(U32 iVersion)
{
	assert(m_pParent);
	m_pParent->SetVersionNumber(iVersion);
}

// read/write functionality with modified file positioning/length

// refills the buffer by reading data
bool C_FileBlock::Underflow(UnderflowContext& ctx)
{
	SetPointers();
	bool bReturn = m_pParent->Underflow(ctx);
	GetPointers();
	return bReturn;
}

// writes the output of the buffer and clears it
bool C_FileBlock::Overflow()
{
	SetPointers();
	bool bReturn = m_pParent->Overflow();
	GetPointers();
	return bReturn;
}

/// Returns the size of the block body. returs 0 if the file is not open.
const C_StreamSize C_FileBlock::Length()
{
	return m_BlockHeader.m_Size;
}

/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
const C_StreamPos C_FileBlock::GetPosition() const
{
	((C_FileBlock*)this)->SetPointers();
	return m_pParent->GetPosition() - m_iHeaderPosition;
}

/// Sets the current read / write position in the file.
/// This function will terminate with an error if the file is not open or newPosition is out of range.
/// This function will terminate with an error if the file is locked.
void C_FileBlock::SetPosition( const C_StreamPos newPosition )
{
	SetPointers();
	m_pParent->SetPosition( newPosition + m_iHeaderPosition );
	GetPointers();
}

void C_FileBlock::Flush()
{
	assert(m_pParent);
	m_pParent->Flush();
}

}; // namespace Sys




