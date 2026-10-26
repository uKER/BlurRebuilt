#ifndef _NETSTREAM_SIZE_CALCULATOR_MEMORY_STREAM
#define _NETSTREAM_SIZE_CALCULATOR_MEMORY_STREAM

#include "netStream/netStream.h"

class netStreamSizeCalculator_NEW: public netStream
{
public:
	netStreamSizeCalculator_NEW(int limit=0, bool checkLimit=false) : netStream(limit), m_checkLimit(checkLimit) {}

	virtual bool		serialise(netStream* stream)			{	assert(false);	return false; }
	virtual bool		deserialise(netStream* stream)			{	assert(false);	return false; }
	virtual bool		readBytes_internal(void* ptr, int len)	{	assert(false);	return false; }
	virtual bool		writeBytes_internal(const void* ptr, int len);

	bool m_checkLimit;
};

class netStreamSizeCalculator: public netStream
{
public:
	enum { E_StreamSize = 10 * 1024 };
	
	netStreamSizeCalculator(int size=E_StreamSize) : netStream(E_StreamSize) {}

	virtual bool		serialise(netStream* stream)			{	assert(false);	return false; }
	virtual bool		deserialise(netStream* stream)			{	assert(false);	return false; }
	virtual bool		readBytes_internal(void* ptr, int len)	{	assert(false);	return false; }
	virtual const char* buffer	 () const						{	return m_Buffer; }
	virtual char*		buffer	 ()								{	return m_Buffer; }

	virtual bool		writeBytes_internal(const void* ptr, int len);
private:
	static char m_Buffer[E_StreamSize];
};


#endif