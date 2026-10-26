#include "LogicalFsPch.h"

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakEntryFile.h>
#include <PakFs/Impl/IPakCompress.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakCompressRaw.h>
#include <PakFs/Impl/PakCompressZLib.h>
#include <PakFs/Impl/PakCompressPacketZLib.h>
#if defined(CFG_CONVERTER)
#include <PakFs/Impl/PakCompressXMem.h>
#include <PakFs/Impl/PakCompressPacketXMem.h>
#endif
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakBuildHelpers.h>
#include <PakFs/Impl/PakUtil.h>	// [Blur2PC] GetFatEntrySize

extern Debug::Logger &g_SystemIoLog;

namespace Pak
{

// constructor
C_PakEntryFile::C_PakEntryFile() :
	m_iVersion(0),
	m_flags(0),
	m_pPakCompress(NULL),
	m_pHashContext(NULL)
{
}

// destructor
C_PakEntryFile::~C_PakEntryFile()
{
	Close();
}

// open file
bool C_PakEntryFile::Open(
	C_PakFile* pPak,
	const C_PakFatEntry* pPakFatEntry,
	const Sys::C_FilePath& fileName, 
	const U32 fileAccessAttributes,
	const Sys::C_OpenParams& params,
	Sha1::C_HashContext* pHashContext)
{
	m_assertEof = params.m_AssertEof;

	if (fileAccessAttributes == Sys::E_FileAccess_Read)
		m_bWriting = false;
	else if (fileAccessAttributes == Sys::E_FileAccess_Write)
		m_bWriting = true;
	else
	{
		BIZ_ASSERT_FAIL("Unsupported file access attributes!");
		return false;
	}
	
	if (!pPak || !pPakFatEntry)
	{
		BIZ_ASSERT_FAIL("Invalid pak file!");
		return false;
	}

	m_pPak = pPak;
	m_Name = fileName;

	m_pHashContext = pHashContext;

	m_pPakEntry = pPakFatEntry;
	m_Length	= pPakFatEntry->dataLogicalLengthBytes;

	switch(pPak->header.codec)
	{
	case E_Codec_ZLib:
		m_pPakCompress = new C_PakCompressZLib(pPak->header.codecFlags);
		break;
	case E_Codec_PacketZLib:
		m_pPakCompress = new C_PakCompressPacketZLib(pPak->header.codecFlags );
		break;
	case E_Codec_Raw:
		m_pPakCompress = new C_PakCompressRaw(pPak->header.codecFlags);
		break;
#if defined(CFG_CONVERTER)
	case E_Codec_PacketXMem:
		m_pPakCompress = new C_PakCompressPacketXMem(pPak->header.codecFlags );
		break;
	case E_Codec_XMem:
		m_pPakCompress = new C_PakCompressXMem(pPak->header.codecFlags);
		break;
#endif
	case E_Codec_Unknown:
	default:
		BIZ_ASSERT_FAIL("Unknown codec!");
		return false;
	};

	Sys::C_StreamPos physicalStartPosition = m_pPakEntry->dataStartSector * (U64)pPak->pMediator->GetSectorSize();
	// [Blur2PC] version 2: stored size comes from the FAT, payload has no size prefix
	S64 knownPhysicalLength = -1;
	if (pPak->pStoredSizes)
	{
		U32 entrySize = GetFatEntrySize((E_FatEntryType)pPak->header.fatEntryType);
		U32 idx = (U32)(((const U8*)pPakFatEntry - pPak->pFatEntries) / entrySize);
		knownPhysicalLength = pPak->pStoredSizes[idx];
		m_pPakCompress->SetKnownPhysicalLength(pPak->pStoredSizes[idx]);
	}

	if (!m_pPakCompress->Open(
		pPak->pMediator,
		physicalStartPosition,
		fileAccessAttributes,
		pHashContext))
	{
		SAFE_DELETE_PTR(m_pPakCompress);
		return false;
	}

	//File is now considered open.
	m_bIsOpen = true;

	//Unbuffered initialisation
	if (params.m_BufferSizeHint == 0)
	{
		m_bUnbuffered = true;

		m_unbufIo.m_unbufferedFilePos = 0;

		//TODO: Support for security when operating unbuffered...
		//VerifyFileHash...
		BIZ_ASSERT(pHashContext == NULL);
		//TODO: Support
		BIZ_ASSERT(!m_bWriting);
	}
	//Buffered initialisation.
	else
	{
		m_bUnbuffered = false;

		//TODO: Allow user hinting at this.
		m_bufIo.m_bufferSize = PAK_IFILE_BUFFER_SIZE;

		m_bufIo.m_pBuffer = new U8[m_bufIo.m_bufferSize];
		m_pBufferEnd = m_bufIo.m_pBuffer+m_bufIo.m_bufferSize;

		if (m_bWriting)
			m_pCurBuffer = m_bufIo.m_pBuffer;
		else
			m_pCurBuffer = m_pBufferEnd;
		m_bufIo.m_iBufferOffset = 0;

		//Check for people tampering with our data if security is enabled.
		if (pHashContext && !m_bWriting)
		{
			if (!VerifyFileHash(pHashContext, pPak->pMediator, physicalStartPosition, knownPhysicalLength))	// [Blur2PC]
			{
				pPak->OnFileClosed(this);

				//Corrupt file!
				Sys::FileRwError(fileName.Chars(), false, true);
				return false;
			}
		}
	}
	pPak->OnFileOpened(this);
	return true;
}

// close file
void C_PakEntryFile::Close()
{
	if (m_bIsOpen)
	{
		// flush write buffer to file
		if (IsWriting())
		{
			Overflow();
			m_pPakCompress->Close();
			m_bWriting = false;
		}
		else
		{
			if (m_pPakCompress) 
			{
				m_pPakCompress->Close();
			}
		}
		m_bIsOpen = false;
		m_pPak->OnFileClosed(this);

		if (!m_bUnbuffered)
		{
			//Free buffer.
			SAFE_DELETE_ARY(m_bufIo.m_pBuffer);
		}
	}
	SAFE_DELETE_PTR(m_pPakCompress);
	SAFE_DELETE_PTR(m_pHashContext);
}

/// Returns true if file is currently open.
const bool C_PakEntryFile::IsOpen() const
{
	return m_bIsOpen;
}

/// Returns the size of the file. returs 0 if the file is not open.
const Sys::C_StreamSize C_PakEntryFile::Length() 
{
	if ((!IsWriting()) && (m_Length != 0))
	{
		// if we're reading and the length is set use it,
		// otherwise use the full file length seek end below
		return m_Length;
	}
	else
	{
		U64 iLength = m_pPakEntry->dataLogicalLengthBytes;
		if (!IsWriting()) m_Length = iLength;
		return iLength;
	}
}

/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
const Sys::C_StreamPos C_PakEntryFile::GetPosition() const
{
	if (m_bUnbuffered)
		return m_unbufIo.m_unbufferedFilePos;
	if (!IsWriting())
		return m_bufIo.m_iBufferOffset + (m_pCurBuffer - m_pBufferEnd);
	else
		return m_bufIo.m_iBufferOffset + (m_pCurBuffer - m_bufIo.m_pBuffer);
}
 
/// Sets the current read / write position in the file.
void C_PakEntryFile::SetPosition( const Sys::C_StreamPos newPosition )
{
	//If compressed.
	if (m_pPakCompress->GetPhysicalLength() != m_Length)
	{
		Sys::C_StreamPos currentPos = GetPosition();
		//Forward seek.
		if (newPosition >= currentPos && newPosition <= m_Length)
		{
			//let's not eat too much stack.
			#define SEEK_BUFFER_SIZE 8192
			U8 buffer[SEEK_BUFFER_SIZE];
			Sys::C_StreamPos bytesToRead = newPosition - currentPos;
			while(bytesToRead > 0)
			{
				U32 readSize;
				if (bytesToRead > SEEK_BUFFER_SIZE)
					readSize = SEEK_BUFFER_SIZE;
				else
					readSize = (U32)bytesToRead;
				if (!Read(buffer, readSize))
				{
					return;
				}
				bytesToRead -= readSize;
			}
			return;
		}
	}
	
#ifndef CFG_CONVERTER
	//If compressed...
	if (m_pPakCompress->GetPhysicalLength() != m_Length)
	{
		FSASSERT(g_SystemIoLog, "Physical seek of %s within compressed pak file, someone needs to fix this as perf will be crippling.", m_Name.Chars());
	}
#endif

	//Unbuffered
	if (m_bUnbuffered)
	{
		m_unbufIo.m_unbufferedFilePos = newPosition;
		if (newPosition > Length())
		{
			FSASSERT(g_SystemIoLog, "C_PakEntryFile::SetPosition EOF failure %s pos=%i ", Name().Chars(), (U32)newPosition);
			return;
		}
		m_pPakCompress->Restart( (U32)m_unbufIo.m_unbufferedFilePos );

		//Underflow will point m_iBufferOffset to the end of the new buffer.
		//m_iBufferOffset is the distance from the start of the file to the end the buffer that 
		//will contain our updated file pointer. 
		UnderflowContext uc;
		Underflow(uc);
	}
	//Buffered
	else
	{
		Sys::C_StreamPos bufferOffset = ((newPosition + m_bufIo.m_bufferSize)/m_bufIo.m_bufferSize) * m_bufIo.m_bufferSize;
		// flush write buffer
		if (IsWriting())
		{
			Overflow();
		}
		else
		{
			if (newPosition > Length())
			{
				FSASSERT(g_SystemIoLog, "C_PakEntryFile::SetPosition EOF failure %s pos=%i ", Name().Chars(), (U32)newPosition);
				SetPosition(Length());
				return;
			}

			//Just fix up our pointers rather than seeking if we're within the bounds of our buffer.
			Sys::C_StreamPos bufferLogicalStart = m_bufIo.m_iBufferOffset - m_bufIo.m_bufferSize;
			if (newPosition >= bufferLogicalStart && newPosition < bufferLogicalStart + m_bufIo.m_bufferSize)
			{
				Sys::C_StreamPos offset = newPosition - (m_bufIo.m_iBufferOffset - m_bufIo.m_bufferSize);
				m_pCurBuffer = m_bufIo.m_pBuffer + offset;
				return;
			}

			//Set file pointer to buffer start.
			m_pCurBuffer = m_bufIo.m_pBuffer;
			//Figure our how many logical buffers worth we're into the file.
			Sys::C_StreamPos newBufferIndex = newPosition / m_bufIo.m_bufferSize;
			//Figure out the logical starting point of our new buffer.
			Sys::C_StreamPos newBufferLogicalStart = (newBufferIndex * m_bufIo.m_bufferSize);
			//Reset ourselves to point to the start of our new buffer.
			m_bufIo.m_iBufferOffset = newBufferLogicalStart;

			//We're outside our buffer. Re-init stream.
			m_pPakCompress->Restart( (U32)newBufferLogicalStart );

			//Underflow will point m_iBufferOffset to the end of the new buffer.
			//m_iBufferOffset is the distance from the start of the file to the end the buffer that 
			//will contain our updated file pointer. 
			UnderflowContext uc;
			Underflow(uc);
			//Offset the file pointer which is currently pointing to the start of our buffer.
			Sys::C_StreamPos newBufferOffset = newPosition % m_bufIo.m_bufferSize;
			m_pCurBuffer += newBufferOffset;
		}
	}

	//Sanity check. TODO: remove this once happy.
	BIZ_ASSERT(GetPosition() == newPosition);
}

/// Returns true if the stream is writing, or false if the stream is reading
const bool C_PakEntryFile::IsWriting() const
{
	return m_bWriting;
}

/// Returns the currently open filename or the filename used to attempt to open the file.
Btl::String C_PakEntryFile::Name() const
{
	return m_Name.Chars();
}

// write buffer to file
bool C_PakEntryFile::Overflow()
{
	BIZ_ASSERT(IsWriting());
	#ifdef CFG_CONVERTER

	if (m_bUnbuffered)
	{
		BIZ_ASSERT_FAIL("TODO: Unbuffered write support.");
		return false;
	}
	else
	{
		U32 n = (U32)(m_pCurBuffer - m_bufIo.m_pBuffer);

		m_pPakCompress->Write( m_bufIo.m_pBuffer, n );
		m_bufIo.m_iBufferOffset += n;
		m_pCurBuffer = m_bufIo.m_pBuffer;
		m_pPak->OnFileWrite(this, n);
		return true;
	}
	#endif

	return false;
}

// read from file into buffer
bool C_PakEntryFile::Underflow(UnderflowContext& ctx)
{
	BIZ_ASSERT(!IsWriting());

	if (m_bUnbuffered)
	{
		//Unbuffered reads
		if ((ctx.readSize > 0) && (ctx.readTarget != 0))
		{
			U64 curPos = GetPosition();
			U64 length = m_Length;
			U64 newPos = curPos + ctx.readSize;
			U64 readSize;

			//Reading within file bounds, great!
			if (newPos <= length)
			{
				readSize = ctx.readSize;
			}
			else
			{
				if (m_assertEof)
				{
					FSASSERT(g_SystemIoLog, "C_PakEntryFile::Underflow EOF %s, ctx.readSize %i", Name().Chars(), (U32)ctx.readSize);
				}
				if (curPos < length)
				{
					//Perform partial read.
					readSize = length - curPos;
				}
				else
				{
					//Perform no read.
					return false;
				}
			}
			if (!m_pPakCompress->Read(ctx.readTarget, (U32)readSize))
				return false;

			m_unbufIo.m_unbufferedFilePos += ctx.readSize;
			ctx.bytesRead = (U32)readSize;
			m_pPak->OnFileRead(this, curPos, ctx.bytesRead);
			return true;
		}
	}
	else
	{
		U64 curPos = GetPosition();
		U64 newPos = curPos + ctx.readSize;

		//fill the the buffer with new data
		Sys::C_StreamPos bytesToRead = m_bufIo.m_bufferSize;
		//Sadly buffered io relies on reading past file end. so we can't throw an error here.
		//The block io stuff is a pain as it relies upon flying off the end of a file.
		if (curPos >= m_Length)
		{
			//Don't read as we're past the end.
			bytesToRead = 0;
		}
		else if (newPos > m_Length)
		{
			//Partial read.
			bytesToRead = m_Length - curPos;
		}

		if (!m_pPakCompress->Read(m_bufIo.m_pBuffer, (U32)bytesToRead))
			return false;

		m_bufIo.m_iBufferOffset += m_bufIo.m_bufferSize;
		m_pCurBuffer = m_bufIo.m_pBuffer;
		//Guerilla hackery to achieve sensible return values despite the way the I_binarystream etc interface
		//forces us to romp along past the end of files.
		U32 bytesRead(bytesToRead);
		if (curPos + bytesToRead > m_Length)
		{
			bytesRead = m_Length - curPos;
		}
		m_pPak->OnFileRead(this, curPos, bytesRead);
	}
	
	return true;
}

/// Returns true if the data should be endian-swapped after being read
const bool C_PakEntryFile::GetEndianSwapFlag() const
{
	return m_bEndianSwapFlag;
}
/// Set the endian swap flag on or off
void C_PakEntryFile::SetEndianSwapFlag( const bool swizzle )
{
	m_bEndianSwapFlag = swizzle;
}

/// Returns a pointer to the versioning system that is associated with this stream
/// This function will return NULL if no versioning system is attached
U32 C_PakEntryFile::GetVersionNumber() const
{
	return m_iVersion;
}

/// Attaches a versioning system to this stream
void C_PakEntryFile::SetVersionNumber(U32 iVersion)
{
	m_iVersion = iVersion;
}

/// Flush the contents of the file to disk
void C_PakEntryFile::Flush()
{
}

}
