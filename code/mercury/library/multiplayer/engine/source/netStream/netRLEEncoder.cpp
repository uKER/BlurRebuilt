
#include "stdafx.h"
#include "netRLEEncoder.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"
#include "netStream.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

BitfieldRLE::BitfieldRLE(char* bitfield, unsigned int bitCount)
{
	// sanity check
	assert(bitCount < (2000 * 8));

	m_bitfield  = bitfield;
	m_bitCount  = bitCount;
	m_header	= 0;
}

unsigned int BitfieldRLE::byteCount() const
{
	return ((m_bitCount-1)/8)+1;
}


bool BitfieldRLE::serialise(class netStream* stream)
{
	// we can encode buffer length as a very short number.
	if(getWriteByteCount() <= HEADER_SHORT_LENGTH_MAX)
	{
		m_header |= HEADER_SHORT_LENGTH;
		m_header |= ((unsigned char)getWriteByteCount());
		stream->writeType(m_header);
	}
	// length too big. encode length as a short.
	else
	{
		stream->writeType(m_header);
		stream->writeVBLE(getWriteByteCount());
	}
	stream->writeBytes(buffer(), getWriteByteCount());
	return stream->ok();
}


bool BitfieldRLE::deserialise(class netStream* stream)
{
	// read header flags
	stream->readType(m_header);

	// we've encoded a short length
	if((m_header & HEADER_SHORT_LENGTH) != 0)
	{
		unsigned int bytes = (m_header & HEADER_SHORT_LENGTH_MAX);
		setWriteByteCount(bytes);
	}
	// length is encoded as an extra short 
	else
	{
		unsigned int bytes;
		stream->readVBLE(bytes);
		setWriteByteCount(bytes);
	}
	
	stream->readBytes(buffer(), getWriteByteCount());
	return stream->ok();
}


bool BitfieldRLE::getBit(int i)
{
	int byte = (i / 8);
	int bit  = (i & 7);
	return ((m_bitfield[byte] & (1 << bit)) != 0);
}


void BitfieldRLE::setBit(int i, bool onOff)
{
	int byte = (i / 8);
	int bit  = (i & 7);
	
	if(onOff)
	{
		m_bitfield[byte] |= (1 << bit);
	}
	else
	{
		m_bitfield[byte] &= ~(1 << bit);
	}
}
bool BitfieldRLE::encode()
{
	// number of bits to encode is very small. 
	// dont bother with conpression.
	if(byteCount() <= 4) 
		return encodeRAW();

	if(encodeRLE()) 
		return true;
	else if(encodeRAW())
		return true;
	else 
		return false;
}


bool BitfieldRLE::encodeRAW()
{
	setReadByteCount(0);
	setWriteByteCount(0);
	m_header = 0;

	unsigned short count = byteCount();
	return writeBytes(m_bitfield, count);
}

bool BitfieldRLE::encodeRLE()
{
	setReadByteCount(0);
	setWriteByteCount(0);
	m_header = (char)HEADER_RLE_ENCODED;
	
	// get fist bit value
	bool base = getBit(0);
	if(base)
		m_header |= HEADER_FIRST_BIT_SET;
	
	// run length will be encoded as VBLE.
	unsigned int runlength = 0;
	int count = 0;
	for(unsigned int i = 0; i < m_bitCount; i++)
	{
		bool bit = getBit(i);
		
		// bit changed, terminate this run.
		if(bit != base)
		{
			// buffer overflow or the RLE would actually be larger than
			// just writing the buffer as raw.
			if(!writeVBLE(runlength))
				return false;

			count += runlength;

			// we've got a new run
			runlength = 1;
			base = !base;
		}
		else
		{
			// run length increased
			runlength++;
		}
	}
	count += runlength;
	assert(count == m_bitCount);

	// write the last run.
	return writeVBLE(runlength);
}


bool BitfieldRLE::decode()
{
	if(m_header & HEADER_RLE_ENCODED)
		return decodeRLE();
	else 
		return decodeRAW();
}


bool BitfieldRLE::decodeRAW()
{
	setReadByteCount(0);
	unsigned short count = byteCount();
	return readBytes(m_bitfield, count);
}


bool BitfieldRLE::decodeRLE()
{
	setReadByteCount(0);
	
	// first bit state.
	bool base = ((m_header & HEADER_FIRST_BIT_SET) == 0)? false : true;
	
	int bit = 0;
	while(!eof())
	{
		// read the bit run length
		unsigned int length;

		if(!readVBLE(length))
		{
			assert(!"Error decoding RLE");
			return false;
		}

		// write the bit run
		for(unsigned int i = 0; i < length; i ++, bit++)
		{
			setBit(bit, base);
		}
		// flip bit
		base = !base;
	}
	return true;
}

void BitfieldRLE::debugOutput()
{
	LOGINFO(netDebug::g_Log, "RLE(%d bits, start bit(%s), %d length)", m_bitCount, (m_header & HEADER_FIRST_BIT_SET)? "on" : "off" , getWriteByteCount());
	LOGINFO(netDebug::g_Log, "bits :");

	for(unsigned int i = 0; i < m_bitCount; )
	{
		char line[256];
		char* l = line;
		unsigned int w = 50;
		unsigned int j;
		for(j = 0; j < w; j++, i++)
		{
			if(i >= m_bitCount)
				break;

			l += sprintf_s(l, (&(line[sizeof(line)])-l), "%1d",  getBit(i)? 1 : 0);
		}	
		LOGINFO(netDebug::g_Log, "%s", line);
	}

	LOGINFO(netDebug::g_Log, "");
	LOGINFO(netDebug::g_Log, "rle:");
	for(int i = 0; i < getWriteByteCount(); i++)
	{
		char line[256];
		char* l = line;
		int w = 16;
		int j;
		for(j = 0; j < w; j++, i++)
		{
			if(i >= getWriteByteCount())
				break;

			l += sprintf_s(l, (&(line[sizeof(line)])-l), "%02x ",  buffer()[i]);
		}		
		LOGINFO(netDebug::g_Log, "%s", line);
	}	
	LOGINFO(netDebug::g_Log, "");
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

