
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

template<int SIZE>
BitfieldRLE<SIZE>::BitfieldRLE(char* bitfield, int bitCount)
{
	// sanity check
	assert(bitCount < (SIZE * 8));

	m_bitfield  = bitfield;
	m_bitCount  = bitCount;

	m_limit		= byteCount(); // if we cant achieve decent compression, just send raw data.
	m_read		= 0;
	m_write		= 0;
	m_header	= 0;
}

template<int SIZE>
unsigned short BitfieldRLE<SIZE>::byteCount() const
{
	return ((m_bitCount-1)/8)+1;
}

template<int SIZE>
bool BitfieldRLE<SIZE>::serialise(class netStream* stream)
{
	// we can encode buffer length as a very short number.
	if(m_write <= HEADER_SHORT_LENGTH_MAX)
	{
		m_header |= HEADER_SHORT_LENGTH;
		m_header |= ((unsigned char)m_write);
		stream->writeType(m_header);
	}
	// length too big. encode length as a short.
	else
	{
		stream->writeType(m_header);
		stream->writeType(m_write);
	}
	stream->writeBytes(m_buffer, m_write);
	return stream->ok();
}

template<int SIZE>
bool BitfieldRLE<SIZE>::deserialise(class netStream* stream)
{
	// read header flags
	stream->readType(m_header);

	// we've encoded a short length
	if((m_header & HEADER_SHORT_LENGTH) != 0)
	{
		m_write = (unsigned short)(m_header & HEADER_SHORT_LENGTH_MAX);
	}
	// length is encoded as an extra short 
	else
	{
		stream->readType(m_write);
	}
	assert(m_write <= m_limit);

	stream->readBytes(m_buffer, m_write);
	return stream->ok();
}

template<int SIZE>
bool BitfieldRLE<SIZE>::getBit(int i)
{
	int byte = (i / 8);
	int bit  = (i & 7);
	return ((m_bitfield[byte] & (1 << bit)) != 0);
}

template<int SIZE>
void BitfieldRLE<SIZE>::setBit(int i, bool onOff)
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

template<int SIZE>
bool BitfieldRLE<SIZE>::writeBytes(const void* ptr, int len)
{
	if(m_write + len > m_limit)
	{
		//assert(!"write overflow!");
		return false;
	}

	memcpy(m_buffer + m_write, ptr, len);
	m_write += len;
	return true;
}

template<int SIZE>
bool BitfieldRLE<SIZE>::readBytes(void* ptr, int len)
{
	if(m_read + len > m_write)
	{
		assert(!"read overflow!");
		return false;
	}

	memcpy(ptr, m_buffer + m_read, len);
	m_read += len;
	return true;
}

template<int SIZE>
bool BitfieldRLE<SIZE>::encode()
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

template<int SIZE>
bool BitfieldRLE<SIZE>::encodeRAW()
{
	m_read   = 0;
	m_write  = 0;
	m_header = 0;
	unsigned short count = byteCount();
	return writeBytes(m_bitfield, count);
}

template<int SIZE>
bool BitfieldRLE<SIZE>::encodeRLE()
{
	m_read   = 0;
	m_write  = 0;
	m_header = HEADER_RLE_ENCODED;
	
	// get fist bit value
	bool base = getBit(0);
	if(base)
		m_header |= HEADER_FIRST_BIT_SET;
	
	// run length (less than 255).
	unsigned char runlength = 0;
	int count = 0;
	for(int i = 0; i < m_bitCount; i++)
	{
		bool bit = getBit(i);
		
		// bit changed, terminate this run.
		if(bit != base)
		{
			// buffer overflow or the RLE would actually be larger than
			// just writing the buffer as raw.
			if(!writeBytes(&runlength, sizeof(runlength)))
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

			// special case. We run out of length.
			if(runlength >= 255)
			{
				// write out run length
				if(!writeBytes(&runlength, sizeof(runlength)))
					return false;
				count += runlength;

				// we'll be running a 'zero length' run.
				runlength = 0;
				if(!writeBytes(&runlength, sizeof(runlength)))
					return false;
				count += runlength;

				// then we'll be carrying on.
				runlength=0;
			}
		}
	}
	count += runlength;
	assert(count == m_bitCount);

	// write the last run.
	return writeBytes(&runlength, sizeof(runlength));
}

template<int SIZE>
bool BitfieldRLE<SIZE>::decode()
{
	if(m_header & HEADER_RLE_ENCODED)
		return decodeRLE();
	else 
		return decodeRAW();
}

template<int SIZE>
bool BitfieldRLE<SIZE>::decodeRAW()
{
	m_read   = 0;
	unsigned short count = byteCount();
	return readBytes(m_bitfield, count);
}

template<int SIZE>
bool BitfieldRLE<SIZE>::decodeRLE()
{
	m_read = 0;

	// first bit state.
	bool base = ((m_header & HEADER_FIRST_BIT_SET) == 0)? false : true;
	
	int bit = 0;
	while(m_read < m_write)
	{
		// read the bit run length
		unsigned char length;
		if(!readBytes(&length, sizeof(length)))
		{
			assert(!"Error decoding RLE");
			return false;
		}

		// write the bit run
		for(int i = 0; i < length; i ++, bit++)
		{
			setBit(bit, base);
		}
		// flip bit
		base = !base;
	}
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
