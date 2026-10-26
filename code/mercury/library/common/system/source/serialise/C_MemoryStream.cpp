// a stream that serialises to and from a memory buffer
#include "SystemPch.h"

#include <system/serialise.h>
#include <system/serialise/C_MemoryStream.h>
#include <core/utility/coredebug.h>

namespace Sys
{

// constructor for writing to a buffer
C_MemoryStream::C_MemoryStream(int iBufferExpandSize) :
	m_iVersion(0),
	m_bEndianSwapFlag(false),
	m_pBuffer(0),
	m_iBufferSize(0),
	m_iBufferExpandSize(iBufferExpandSize),
	m_bWriting(true),
	m_Name("MemoryStream")
{
	m_pCurBuffer = 0;

	C_MemoryStream::Overflow();
}

/// constructor for reading from a given buffer
C_MemoryStream::C_MemoryStream(U8 *pData, U32 cData) :
	m_iVersion(0),
	m_bEndianSwapFlag(false),
	m_bWriting(false),
	m_iBufferSize(cData),
	m_Name("MemoryStream")
{
	m_pBuffer = m_pCurBuffer = pData;
	m_pBufferEnd = m_pBuffer + m_iBufferSize;	
}

C_MemoryStream::C_MemoryStream() :
	m_iVersion(0),
	m_bEndianSwapFlag(false),
	m_pBuffer(0),
	m_iBufferSize(0),
	m_iBufferExpandSize(0),
	m_bWriting(false),
	m_Name("MemoryStream")
{
	m_pBuffer = m_pCurBuffer = 0;
	m_pBufferEnd = 0;
}

void C_MemoryStream::SetBuffer(U8 *pData, U32 cData)
{
	assert(m_pBuffer == 0);
	assert(m_bWriting == false);
	m_pBuffer = m_pCurBuffer = pData;
	m_iBufferSize = cData;
	m_pBufferEnd = m_pBuffer + m_iBufferSize;
}

// destructor
C_MemoryStream::~C_MemoryStream()
{
	if (m_bWriting)
		delete m_pBuffer;
}

/// Returns true if the stream is writing, or false if the stream is reading
const bool C_MemoryStream::IsWriting() const
{
	return m_bWriting;
}

/// Returns the currently open filename or the filename used to attempt to open the file.
Btl::String	C_MemoryStream::Name() const
{
	return m_Name;
}

// grow the write buffer by 'm_iBufferExpandSize'
bool C_MemoryStream::Overflow()
{
	assert(IsWriting());

	U64 iOffset = m_pCurBuffer - m_pBuffer;

	U8 *pNewBuffer = new U8[m_iBufferSize + m_iBufferExpandSize];
	if (!pNewBuffer)
		return false;

	if (m_pBuffer)
	{
		memcpy(pNewBuffer, m_pBuffer, m_iBufferSize);
		delete m_pBuffer;
	}
	m_iBufferSize += m_iBufferExpandSize;
	m_pBuffer = pNewBuffer;

	m_pCurBuffer = m_pBuffer + iOffset;
	m_pBufferEnd = m_pBuffer + m_iBufferSize;
	return true;
}

// should never happen
bool C_MemoryStream::Underflow(UnderflowContext& ctx)
{
	assert(!IsWriting());
	return false;
}

/// Returns true if the data should be endian-swapped after being read
const bool C_MemoryStream::GetEndianSwapFlag() const
{
	return m_bEndianSwapFlag;
}
/// Set the endian swap flag on or off
void C_MemoryStream::SetEndianSwapFlag( const bool swizzle )
{
	m_bEndianSwapFlag = swizzle;
}

/// Returns a pointer to the versioning system that is associated with this stream
/// This function will return NULL if no versioning system is attached
U32 C_MemoryStream::GetVersionNumber() const
{
	return m_iVersion;
}

/// Attaches a versioning system to this stream
void C_MemoryStream::SetVersionNumber(U32 iVersion)
{
	m_iVersion = iVersion;
}

/// Returns the size of the buffer
U64	C_MemoryStream::GetBufferSize() const
{
	return m_pCurBuffer - m_pBuffer;
}

const C_StreamPos C_MemoryStream::GetPosition() const
{
	return GetBufferSize();
}

/// Returns the address of the buffer
U8 *C_MemoryStream::GetBufferPtr() const
{
	return m_pBuffer;
}

void C_MemoryStream::TakeOwnership()
{
	// Mark the buffer as NULL because the application has taken ownership of the buffer:
	m_pBuffer = m_pCurBuffer = m_pBufferEnd = 0;
}

}
