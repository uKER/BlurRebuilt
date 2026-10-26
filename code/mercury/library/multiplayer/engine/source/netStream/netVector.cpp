
#include "stdafx.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"
#include "netStream/netVector.h"
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

netVector::netVector(int length)
: netBuffer(new char[length], length)
{}

netVector::~netVector()
{
	delete[] m_buffer;
}

bool netVector::deserialise(netStream* stream)
{
	clear();

	int len;
	if(!stream->readType(len)) return false;

	// four meg length. out of order!
	if(len > (4 * 1024 * 1024))
	{
		LOGERROR(netDebug::g_Log, "<netVector> stream length is over 4 megs!!!(%f megabytes)", (float) len / (1024.0f * 1024.0f));
		setStatus(netStream::STATUS_FAILED_WRITE);
		return false;
	}

	realloc(len);
	
	if(!stream->readBytes(m_buffer, len)) return false;
	m_write = len;
	return true;
}

bool netVector::realloc(int size)
{
	// no need
	if(size == m_limit) return true;

	char* buf = new char[size];

	int copyLen = (size < getWriteByteCount())? size : getWriteByteCount();
	
	if(copyLen > 0)
		bdMemcpy(buf, m_buffer, copyLen);

	delete[] m_buffer;
	m_buffer = buf;
	m_limit = size;
	return true;
}

bool netVector::writeBytes_internal(const void* ptr, int len)
{
	if(!ok()) return false;

	if(byteWriteOverflow(len))
	{
		int required = (getWriteByteCount() + len);
		int new_size = required + (getWriteByteLimit()) / 2 + 8;
		realloc(new_size);
	}

	return netBuffer::writeBytes_internal(ptr, len);
}