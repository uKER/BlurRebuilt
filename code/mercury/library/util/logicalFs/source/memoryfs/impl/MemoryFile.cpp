// a cross-platform default implementation for I_File
#include "LogicalFsPch.h"

#include <system/serialise.h>
#include <system/serialise/FsPerfLogger.h>
#include <MemoryFs/Impl/MemoryFile.h>
#include <MemoryFs/MemoryFs.h>

namespace LogicalFs
{

C_MemoryFile::C_MemoryFile()
:
m_flags(0)
{
	m_pBufferEnd = 0;
	m_pCurBuffer = 0;
	m_pData = 0;

	m_pFsToDeleteFromAfterClosed = NULL;

	// flags
	m_iVersion = 0;
	m_length = 0;
}

// destructor
C_MemoryFile::~C_MemoryFile()
{
	Sys::C_StreamPos pos = Sys::C_StreamPos((U64)m_pCurBuffer - (U64)m_pData);
	FSPERF_READ_VIRTUAL("MemoryFs", (U64)this, 0, pos, NULL);
	FSPERF_CLOSED("MemoryFs", (U64)this);

	if (m_pFsToDeleteFromAfterClosed)
	{
		m_pFsToDeleteFromAfterClosed->DeleteFile(m_Name);
		LOGINFO(g_log, "Unmounted ram file %s", m_Name.Chars());
	}
}

// open file
bool C_MemoryFile::Open(const void* pData, U32 size, const Btl::String& name, const Sys::C_OpenParams& params)
{
	m_iVersion = 0;
	m_bEndianSwapFlag = false;
	m_bWriting = false;
	m_bIsOpen = true;

	m_pData = const_cast<U8*>((U8*)pData);
	m_pCurBuffer = m_pData;
	m_pBufferEnd = m_pData + size;
	m_length = size;
	m_Name = name;

	FSPERF_OPENED("MemoryFs", m_Name, (U64)this, Sys::E_FileAccess_Read);
	return true;
}

/// Returns true if file is currently open.
const bool C_MemoryFile::IsOpen() const
{
	return m_bIsOpen;
}

void C_MemoryFile::ReleaseFromFsAfterClosed(C_MemoryFs* pMemoryFs)
{
	m_pFsToDeleteFromAfterClosed = pMemoryFs;
}

Btl::String C_MemoryFile::Name() const
{
	return m_Name;
}

/// Returns the size of the file. returs 0 if the file is not open.
const Sys::C_StreamSize C_MemoryFile::Length() 
{
	return m_length;
}

/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
const Sys::C_StreamPos C_MemoryFile::GetPosition() const
{
	BIZ_ASSERT(m_pCurBuffer);
	BIZ_ASSERT(m_pData);
	return Sys::C_StreamPos((U64)m_pCurBuffer - (U64)m_pData);
}
 
/// Sets the current read / write position in the file.
void C_MemoryFile::SetPosition( const Sys::C_StreamPos newPosition )
{
	//Unsafe cast from 64-bit - need to sort this.
	BIZ_ASSERT(m_pCurBuffer);
	BIZ_ASSERT(m_pData);
	m_pCurBuffer = m_pData + (U32)newPosition;
}

/// Returns true if the stream is writing, or false if the stream is reading
const bool C_MemoryFile::IsWriting() const
{
	return m_bWriting;
}

// write buffer to file
bool C_MemoryFile::Overflow()
{
	BIZ_ASSERT_FAIL("NOT IMPLEMENTED");
	return false;
}

// read from file into buffer
bool C_MemoryFile::Underflow(UnderflowContext& ctx)
{
	//Should not underflow.
	m_pCurBuffer = m_pBufferEnd;
	return false;
}

/// Returns true if the data should be endian-swapped after being read
const bool C_MemoryFile::GetEndianSwapFlag() const
{
	return m_bEndianSwapFlag;
}
/// Set the endian swap flag on or off
void C_MemoryFile::SetEndianSwapFlag( const bool swizzle )
{
	m_bEndianSwapFlag = swizzle;
}

/// Returns a pointer to the versioning system that is associated with this stream
/// This function will return NULL if no versioning system is attached
U32 C_MemoryFile::GetVersionNumber() const
{
	return m_iVersion;
}

/// Attaches a versioning system to this stream
void C_MemoryFile::SetVersionNumber(U32 iVersion)
{
	m_iVersion = iVersion;
}

}
