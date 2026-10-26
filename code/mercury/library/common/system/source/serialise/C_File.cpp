// a cross-platform default implementation for I_File
#include "SystemPch.h"
#include <system/debuglog.h>

#include <system/serialise.h>
#include <system/serialise/C_File.h>
#include <system/serialise/FsPerfLogger.h>
#include <system/serialise/FileSystemGlobals.h>
#include <core/utility/coredebug.h>
#ifdef PLATFORM_PS3
#include <fcntl.h>
#include <sys/fs_external.h>
#include <cell/fs/cell_fs_file_api.h>
#endif
#if defined(PLATFORM_PS3)
#include <system/misc/stringutil.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize ("", off) // CFG_ALLOW_OPTIMIZE_OFF
#endif

extern Debug::Logger &g_SystemIoLog;

namespace Sys
{

// constructor
C_File::C_File() :
	m_bIsOpen(false),
	m_iVersion(0),
	m_bEndianSwapFlag(false),
	m_IsCritical(true)
#if defined(PLATFORM_PC)
	,m_f(INVALID_HANDLE_VALUE)
#else
	,m_f(0)
#endif
{
}

// destructor
C_File::~C_File()
{
	if (m_bIsOpen)
		C_File::Close();
}

// open file
bool C_File::Open(const C_FilePath& fileName,
				  const U32 fileAccessAttributes,
				  bool isCritical,
				  U32 bufferSize )
{
	FSINFOSPAM(g_SystemIoLog, "C_File::Open, %s, access=%i isCritical=%s", 
		fileName.Chars(), fileAccessAttributes, isCritical ? "true" : "false");

	m_bWriting = (fileAccessAttributes == E_FileAccess_Write || fileAccessAttributes == E_FileAccess_Append);
	m_IsCritical = isCritical;
#if defined(PLATFORM_PS3)
	static const S32  ps3FileAccess[4] = { NULL, CELL_FS_O_RDONLY, CELL_FS_O_WRONLY | CELL_FS_O_CREAT , CELL_FS_O_WRONLY | CELL_FS_O_APPEND | CELL_FS_O_CREAT };

	// TODO: [Jan] implement file creations attribute checks
//	char searchStr[] = "/dev_bdvd/PS3_GAME/USRDIR/";
	char searchStr[] = "/USRDIR/";
	char *lower = (char *)strstr(fileName.Chars(), searchStr);
	if (lower)
	{
		U32 start = strlen(searchStr);
		U32 end = strlen(lower);
		for (U32 i = start ; i < end ; i++)
		{
			if (lower[i] >= 'A' && lower[i] <= 'Z') lower[i] += 0x20;
		}
	}

#endif
	m_Name = fileName.Chars();
	assert(!m_bIsOpen);

#if defined(PLATFORM_PC)

	static const S32  fileAccess[4] = { NULL, GENERIC_READ, GENERIC_WRITE , GENERIC_WRITE  };
	static const S32  disposition[4] = { NULL, OPEN_EXISTING, CREATE_ALWAYS , OPEN_ALWAYS  };
	// open file
	m_f = CreateFile(	fileName.Chars(),					// file name
						fileAccess[fileAccessAttributes],	// access mode
						FILE_SHARE_READ, // share mode
						NULL, // SD
						disposition[fileAccessAttributes], // how to create
						FILE_ATTRIBUTE_NORMAL, // file attributes
						NULL // handle to template file
						);
						
	if (m_f == INVALID_HANDLE_VALUE)	// [Blur2PC][x64] was (U32)m_f == 0xffffffff
	{
		DWORD err = GetLastError();

		FSERRORSPAM(g_SystemIoLog, "C_File::Open, %s, osErr %i", fileName.Chars(), err);

		if (err == ERROR_PATH_NOT_FOUND ||
			err == ERROR_INVALID_DRIVE ||
			err == ERROR_INVALID_NAME ||
			err == ERROR_FILE_NOT_FOUND ||

			//Access denied
			err == ERROR_ACCESS_DENIED ||

			//Media change.
			err == ERROR_IO_DEVICE ||
			err == ERROR_NO_MEDIA_IN_DRIVE ||
			err == ERROR_MEDIA_CHANGED)
			//Non fatal return.
			return false;

		//Potentially fatal return.
		Sys::FileRwError(Name().Chars(), m_bWriting, m_IsCritical, NULL);
		return false;
	}
	
	if (fileAccessAttributes==E_FileAccess_Append)
		SetFilePointer(m_f, 0, 0, FILE_END);

#elif defined(PLATFORM_PS3)

	int ret = cellFsOpen(fileName.Chars(), ps3FileAccess[fileAccessAttributes], &m_f, NULL, 0 );
	if (ret == CELL_FS_EMFILE ||
		ret == CELL_FS_EIO ||
		ret == CELL_FS_ENOMEM ||
		ret == CELL_FS_EFSSPECIFIC ||
		ret == CELL_FS_EPERM ||
		ret == CELL_FS_EACCES || 
		ret == CELL_FS_ERROR_ENOTMOUNTED)
	{
		FSERRORSPAM(g_SystemIoLog, "C_File::Open, failed to open %s, osErr %i", fileName.Chars(), ret);

		m_f = NULL;
		//printf("ERROR: Failed to open file %s Error %x\n",fileName.Chars(),ret);
		//Potentially fatal return.
		Sys::FileRwError(Name().Chars(), m_bWriting, m_IsCritical, NULL);
		return false;
	}

#else

	static const char *pszFileAccess[4] = { NULL, "rb", "wb", "rb+" };
	// open file
	m_f = fopen(fileName.Chars(), pszFileAccess[fileAccessAttributes]);
	if (!m_f)
	{
		FSERRORSPAM(g_SystemIoLog, C_File::Open, failed to open %s", fileName.Chars());
		Sys::FileRwError(Name().Chars(), m_bWriting, m_IsCritical, NULL);
		return false;
	}

#endif

	m_bIsOpen = true;

	buffer = new U8 [ bufferSize ];

	// initialize buffer
	m_pBufferEnd = buffer+bufferSize;
	if (m_bWriting)
		m_pCurBuffer = buffer;
	else
		m_pCurBuffer = m_pBufferEnd;
	m_iBufferOffset = 0;
	m_Length = 0;

	FSPERF_OPENED("CFile", m_Name, (U64)this, (E_FileAccess)fileAccessAttributes);
	return true;
}

// close file
void C_File::Close()
{
	FSINFOSPAM(g_SystemIoLog, "C_File::Closing %s", Name().Chars());

	if (m_bIsOpen)
	{
		// flush write buffer to file
		if (IsWriting())
			Overflow();

		m_bIsOpen = false;

#if defined(PLATFORM_PC)
		SetEndOfFile( m_f );
		CloseHandle( m_f );
#elif defined(PLATFORM_PS3)
		cellFsClose( m_f );
#else
		fclose(m_f);
#endif
		SAFE_DELETE_ARY( buffer );
		FSPERF_CLOSED("CFile", (U64)this);
	}
}

/// Returns true if file is currently open.
const bool C_File::IsOpen() const
{
	return m_bIsOpen;
}



/// Returns the size of the file. returs 0 if the file is not open.
const C_StreamSize C_File::Length() 
{
	if ((!IsWriting()) && (m_Length != 0))
	{
		// if we're reading and the length is set use it,
		// otherwise use the full file length seek end below
		return m_Length;
	}
	else
	{
#if defined(PLATFORM_PC)
		long iLength = GetFileSize( m_f, NULL );
#elif defined(PLATFORM_PS3)
		CellFsStat stat;
		// Get file statistics
		memset(&stat,0x00,sizeof(stat));
		int err = cellFsFstat( m_f, &stat );
		if(err != CELL_FS_SUCCEEDED) 
		{ 
			//printf("Failed to Get File Stats %s err: %x\n",Name().Chars(), err);		
		}
		long iLength = stat.st_size;
#else
		long iOrgPos = ftell( m_f );
		fseek( m_f, 0, SEEK_END );
		long iLength = ftell( m_f );
		fseek( m_f, iOrgPos, SEEK_SET );
#endif

		if (!IsWriting()) m_Length = iLength;
		return iLength;
	}
}

/// Returns the current read/write position in bytes. If the file is not open, this function returns 0.
const C_StreamPos C_File::GetPosition() const
{
	if (!IsWriting())
		return m_iBufferOffset + (m_pCurBuffer - m_pBufferEnd);
	else
		return m_iBufferOffset + (m_pCurBuffer - buffer);
}
 
/// Sets the current read / write position in the file.
void C_File::SetPosition( const C_StreamPos newPosition )
{
	// flush write buffer
	if ( IsWriting() )
		Overflow();
	else
	{
		if (m_pBufferEnd != 0)
		{
			size_t bufferSize = m_pBufferEnd - buffer;
			//Just fix up our pointers rather than seeking if we're within the bounds of our buffer.
			C_StreamPos bufferLogicalStart = m_iBufferOffset - bufferSize;
			if (newPosition >= bufferLogicalStart && newPosition < bufferLogicalStart + bufferSize)
			{
				C_StreamPos offset = newPosition - (m_iBufferOffset - bufferSize);
				m_pCurBuffer = buffer + offset;
				return;
			}
			m_pCurBuffer = m_pBufferEnd;
		}
	}

#if defined(PLATFORM_PC)

	//[Ken] Adding large file support.
	LARGE_INTEGER li;
	li.QuadPart = newPosition;
	li.LowPart = SetFilePointer (m_f, li.LowPart, &li.HighPart, FILE_BEGIN);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
		BIZ_ASSERT_FAIL("Failed to set file pos");
   }

#elif defined(PLATFORM_PS3)
	U64 position = 0;
	int ret = cellFsLseek( m_f, newPosition, CELL_FS_SEEK_SET, &position);
	if (ret != CELL_FS_SUCCEEDED)
	{
		BIZ_ASSERT_FAIL("Failed to set file pos");
	}
#else
	BIZ_ASSERT(newPosition < 2^31);
	fseek(m_f, (long)newPosition, SEEK_SET);
#endif

	m_iBufferOffset = newPosition;
}

/// Returns true if the stream is writing, or false if the stream is reading
const bool C_File::IsWriting() const
{
	return m_bWriting;
}

/// Returns the currently open filename or the filename used to attempt to open the file.
Btl::String C_File::Name() const
{
	return m_Name;
}

// write buffer to file
bool C_File::Overflow()
{
	assert(IsWriting());
	U32 n = (U32)(m_pCurBuffer - buffer);

#if defined(PLATFORM_PC)
	DWORD nrWritten = 0;
	WriteFile( m_f, buffer, n, &nrWritten, NULL );
#elif defined(PLATFORM_PS3)
	U64 nrWritten = 0;
	int ret = cellFsWrite( m_f, buffer, n, &nrWritten );
	if (ret != CELL_FS_SUCCEEDED )
	{
		//printf("Error Closing File %s err %x\n",Name().Chars(), ret );
	}
#else
	fwrite(buffer, n, 1, m_f);
#endif

	m_iBufferOffset += n;
	m_pCurBuffer = buffer;
	FSPERF_WRITE_PHYSICAL("CFile", (U64)this, nrWritten);
	return true;
}

bool C_File::DoFileRead(U8* data, U32 size, U32* nrRead)
{
	if (size > 0)
	{
#if defined(PLATFORM_PC)
	DWORD bytesRead = 0;

	if (ReadFile(m_f, data, size, &bytesRead, 0) == FALSE)
	{
		*nrRead = bytesRead;
		return false;
	}

	*nrRead = bytesRead;
	return bytesRead > 0;

#elif defined(PLATFORM_PS3)

	U64 bytesRead = 0;
	int ret = cellFsRead( m_f, data, size, &bytesRead );
	if (ret != CELL_FS_SUCCEEDED )
	{
		*nrRead = (U32)bytesRead;
		return false;
	}

	*nrRead = (U32)bytesRead;
	return bytesRead > 0;

#else

	size_t bytesRead = fread(data, size, 1, m_f);
	if (bytesRead == size)
	{
		*nrRead = bytesRead;
		return false;
	}
	
	*nrRead = bytesRead;
	return bytesRead > 0;

#endif
	}
	//When attempting to read 0 bytes we return a read of 0 and a status of true, as per windows ReadFile function.
	else if (data)
	{
		*nrRead = 0;
		return true;
	}
	
	//Trying to read 0 bytes with no data ptr. Fail.
	*nrRead = 0;
	return false;
}

// read from file into buffer
bool C_File::Underflow(UnderflowContext& ctx)
{
	assert(!IsWriting());

	U32 bytesRead = 0;
	if (ctx.readSize >= sizeof(buffer))
	{
		if (!DoFileRead(ctx.readTarget, ctx.readSize, &bytesRead))
		{
			Sys::FileRwError(Name().Chars(), m_bWriting, m_IsCritical, NULL);
			return false;
		}

		ctx.bytesRead = bytesRead;

		m_iBufferOffset += bytesRead;

		m_pCurBuffer	= 0;
		m_pBufferEnd	= 0;
	}
	else
	{
		if (!DoFileRead(buffer, sizeof(buffer), &bytesRead))
		{
			Sys::FileRwError(Name().Chars(), m_bWriting, m_IsCritical, NULL);
			return false;
		}

		m_iBufferOffset += bytesRead;

		m_pCurBuffer	= buffer;
		m_pBufferEnd	= buffer + bytesRead;
	}

	FSPERF_READ_PHYSICAL("CFile", (U64)this, GetPosition(), bytesRead, NULL);
	return true;
}

/// Returns true if the data should be endian-swapped after being read
const bool C_File::GetEndianSwapFlag() const
{
	return m_bEndianSwapFlag;
}
/// Set the endian swap flag on or off
void C_File::SetEndianSwapFlag( const bool swizzle )
{
	m_bEndianSwapFlag = swizzle;
}

/// Returns a pointer to the versioning system that is associated with this stream
/// This function will return NULL if no versioning system is attached
U32 C_File::GetVersionNumber() const
{
	return m_iVersion;
}

/// Attaches a versioning system to this stream
void C_File::SetVersionNumber(U32 iVersion)
{
	m_iVersion = iVersion;
}

/// Flush the contents of the file to disk
void C_File::Flush()
{
#if defined(PLATFORM_PC)
	FlushFileBuffers( m_f );
#elif defined(PLATFORM_PS3)
	cellFsFsync( m_f );
#else
	fflush(m_f);
#endif

}

}
