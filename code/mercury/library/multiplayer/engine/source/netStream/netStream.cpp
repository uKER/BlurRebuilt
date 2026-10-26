#include "stdafx.h"
#include "./netStream.h"
#include "netDebug/netDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

bool netStream::byteReadOverflow(int bytes) 
{ 
	if(!modeValid(MODE_READ)) 
	{
		LOGFATAL(netDebug::g_Log, "<netStream> read protected.");
		return true; 
	}
	return ((m_read + bytes) > m_write); 
}

bool netStream::byteWriteOverflow(int bytes) 
{ 
	if(!modeValid(MODE_WRITE)) 
	{
		LOGFATAL(netDebug::g_Log, "<netStream> write protected.");
		return true; 
	}
	
	return ((m_write + bytes) > m_limit); 
}

netStream& netStream::operator = (const netStream& stream)
{
	LOGFATAL(netDebug::g_Log, "<netStream> hmmm cant call copy constructor on streams. fix me somehow...");
	return *this;
}
	
bool netStream::copy(netStream* stream)
{
	clear();
	
	int readptr = stream->getReadByteCount();

	write(stream, stream->getWriteByteCount());

	stream->setReadByteCount(readptr);

	return ok();
}

bool netStream::different(netStream* stream)
{
	int read0ptr = getReadByteCount();
	int read1ptr = stream->getReadByteCount();

	setReadByteCount(0);
	stream->setReadByteCount(0);

	bool result = different(stream, stream->getWriteByteCount());

	setReadByteCount(read0ptr);
	stream->setReadByteCount(read1ptr);

	return result;
}

bool netStream::different(netStream* stream, int len)
{
	while(len > 0)
	{
		char buff0[1024];
		char buff1[1024];
		int bufferlen = (len > sizeof(buff0))? sizeof(buff0) : len;
	
		if(byteReadOverflow(bufferlen) || stream->byteReadOverflow(bufferlen))
			return true;

		if(!readBytes(buff0, bufferlen))
			return true;

		if(!stream->readBytes(buff1, bufferlen))
			return true;

		int result = memcmp(buff0, buff1, bufferlen);
				
		len -= bufferlen;
	}
	return false;
}

bool netStream::read(netStream* stream, int len)
{
	while(len > 0)
	{
		char buffer[2048];
		int bufferlen = (len > sizeof(buffer))? sizeof(buffer) : len;
		
		if(!readBytes(buffer, bufferlen))
			return false;

		if(!stream->writeBytes(buffer, bufferlen))
			return false;

		len -= bufferlen;
	}
	return ok();
}

bool netStream::write(netStream* stream, int len)
{
	while(len > 0)
	{
		char buffer[2048];
		int bufferlen = (len > sizeof(buffer))? sizeof(buffer) : len;
		
		if(!stream->readBytes(buffer, bufferlen))
			return false;
		
		if(!writeBytes(buffer, bufferlen))
			return false;

		len -= bufferlen;
	}
	return ok();
}

// try to read a static string
// check what's written against the magic string
bool netStream::readMarker(const char* magic)
{
	int len = strlen(magic)+1;
	assert(len < 128);

	char str[128];
	bool ok = readBytes(str, len);

	bool match = ok && (memcmp(magic, str, len) == 0);

	if(!match)
	{
		str[len-1] = 0;
		LOGERROR(netDebug::g_Log, "Mismatch of magic marker in stream (%s instead of %s)", str, magic);
		setStatus(STATUS_FAILED_READ_VERSION);
	}
	return match;
}

// try to write a static string
bool netStream::writeMarker(const char* magic)
{
	int len = strlen(magic)+1;
	assert(len < 128);
	return writeBytes(magic, len);
}

bool netStream::readSafe(void* ptr, int len)
{
	if(!modeValid(MODE_READ)) 
	{
		LOGFATAL(netDebug::g_Log, "<netStream> read protected.");
		setStatus(STATUS_FAILED_READ_PROTECTED);
		return false; 
	}
	return readBytes_internal(ptr, len);
}

bool netStream::writeSafe(const void* ptr, int len)
{
	if(!modeValid(MODE_WRITE)) 
	{
		LOGFATAL(netDebug::g_Log, "<netStream> write protected.");
		setStatus(STATUS_FAILED_WRITE_PROTECTED);
		return false; 
	}
	return writeBytes_internal(ptr, len);
}

bool netStream::readBytes(void* ptr, int len)
{
	return readSafe(ptr, len);
}

bool netStream::writeBytes(const void* ptr, int len)
{
	return writeSafe(ptr, len);
}
	
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

