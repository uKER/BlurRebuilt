#ifndef NETBUFFER_H
#define NETBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netStream.h"

BD_REFERENCE(netBuffer);
class netBuffer: public netStream
{
public:
	netBuffer(char* buffer, int bufferLen, int write=0, int read=0);

	virtual bool serialise(netStream* stream);
	virtual bool deserialise(netStream* stream);

	virtual bool readBytes_internal(void* ptr, int len);
	virtual bool writeBytes_internal(const void* ptr, int len);

	const char* buffer	 () const { return m_buffer; }
	char*		buffer	 ()		  { return m_buffer; }
	
public:
	char* m_buffer;
};

#endif //NETBUFFER_H
