
#include "stdafx.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"
#include "netStream/netBuffer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netBuffer::netBuffer(char* buffer, int limit, int write, int read)
: netStream(limit, write, read)
, m_buffer(buffer)
{}

bool netBuffer::serialise(netStream* stream)
{
	stream->writeType(m_write);
	stream->writeBytes(m_buffer, m_write);
	return stream->ok();
}

bool netBuffer::deserialise(netStream* stream)
{
	clear();

	int len;
	if(!stream->readType(len)) return false;

	if(len > getWriteByteLimit()) 
	{
		LOGERROR(netDebug::g_Log, "<netBuffer::deserialise()> invalid stream length.");
		return false;
	}
	
	if(!stream->readBytes(m_buffer, len)) return false;
	m_write = len;

	return true;
}

bool netBuffer::readBytes_internal(void* ptr, int len)
{
	if(!ok()) return false;

	if(byteReadOverflow(len))
	{
		setStatus(netStream::STATUS_FAILED_READ_OVERFLOW);
		LOGERROR(netDebug::g_Log, "<netBuffer::deserialise()> read overflow.");
		return false;
	}

	if(len < 0)
	{
		LOGWARN(netDebug::g_Log, "<netBuffer::deserialise()> invlai len.");
		return false;
	}

	if(!ptr) return (len == 0);
	if(len == 0) return true;

	memcpy(ptr, m_buffer + m_read, len);
	m_read += len;
	return true;
}

bool netBuffer::writeBytes_internal(const void* ptr, int len)
{
	if(!ok()) return false;

	if(byteWriteOverflow(len))
	{
		setStatus(netStream::STATUS_FAILED_WRITE_OVERFLOW);
		LOGWARN(netDebug::g_Log, "<netBuffer::deserialise()> write overflow.");
		return false;
	}
	
	if(len < 0)
	{
		LOGERROR(netDebug::g_Log, "<netBuffer::deserialise()> invalid len.");
		return false;
	}
	
	if(!ptr) return (len == 0);
	if(len == 0) return true;

	memcpy(m_buffer + m_write, ptr, len);
	m_write += len;
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

