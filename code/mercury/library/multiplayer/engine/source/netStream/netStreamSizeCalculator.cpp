#include "stdafx.h"
#include "netStreamSizeCalculator.h"
#include "netEnums/netEnums.h"

char netStreamSizeCalculator::m_Buffer[E_StreamSize] = {0};

bool netStreamSizeCalculator::writeBytes_internal(const void* ptr, int len)
{
	if(!ok()) return false;
	assert(len >= 0);
	if(!ptr) return (len == 0);
	if(len == 0) return true;
	m_write += len;
	//m_limit += len;
	return true;
}



bool netStreamSizeCalculator_NEW::writeBytes_internal(const void* ptr, int len)
{
	assert(len >= 0);

	if(!ok()) 
		return false;

	if(!ptr) 
		return (len == 0);

	if(len == 0) 
		return true;

	if(m_checkLimit && byteWriteOverflow(len))
	{
		setStatus(netStream::STATUS_FAILED_WRITE_OVERFLOW);
		//netDebug::logWarn("<netBuffer::deserialise()> read overflow.");
		return false;
	}
	
	if(!m_checkLimit)
	{
		m_limit += len;
	}
	m_write += len;
	return true;
}