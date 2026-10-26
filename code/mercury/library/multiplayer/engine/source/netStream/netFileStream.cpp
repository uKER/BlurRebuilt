
#include "stdafx.h"
#include "netFileStream.h"
#include "system\serialise.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netFileStream::netFileStream(char* filename, int access, bool endianSwap)
: netStream(1024)
, m_access(access)
, m_file(NULL)
{
	int fileaccess;
	
	switch(m_access)
	{
	default: 
		{
			setMode(0);
			setStatus(STATUS_INVALID);
			return;
		}
	case ACCESS_READ: 
		{
			fileaccess = Sys::E_FileAccess_Read; 
			setMode(MODE_READ);
			break;
		}

	case ACCESS_WRITE: 
		{
			fileaccess = Sys::E_FileAccess_Write; 
			setMode(MODE_WRITE);
			break;
		}
	case ACCESS_APPEND: 
		{
			fileaccess = Sys::E_FileAccess_Append; 
			setMode(MODE_WRITE);
			break;
		}
	}

	m_file = Sys::GetDefaultFileSystem()->Open(filename, (Sys::E_FileAccess)fileaccess);
	
	if(m_file == NULL)
	{
		assert(!"<netFileStream> failed to create file");
		setStatus(STATUS_INVALID);
	}
}

netFileStream::~netFileStream()
{
	SAFE_DELETE_PTR(m_file);
}

bool netFileStream::readBytes_internal(void* ptr, int len)
{
	if(m_file == NULL)
	{
		assert(!"<netFileStream> no file pointer");
		setStatus(STATUS_FAILED_READ);
		return false;
	}

	if(m_access != ACCESS_READ)
	{
		assert(!"<netFileStream> write only");
		setStatus(STATUS_FAILED_READ);
		return false;
	}

	if(!m_file->Read(ptr, len))
	{
		assert(!"<netFileStream> read failed");
		setStatus(STATUS_FAILED_READ);
		return false;
	}
	return true;
}

bool netFileStream::writeBytes_internal(const void* ptr, int len)
{
	if(m_file == NULL)
	{
		assert(!"<netFileStream> no file pointer");
		setStatus(STATUS_FAILED_WRITE);
		return false;
	}

	if(m_access != ACCESS_WRITE && m_access != ACCESS_APPEND)
	{
		assert(!"<netFileStream> read only");
		setStatus(STATUS_FAILED_WRITE);
		return false;
	}

	if(!m_file->Write(ptr, len))
	{
		assert(!"<netFileStream> read failed");
		setStatus(STATUS_FAILED_READ);
		return false;
	}
	return true;
}

	
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

