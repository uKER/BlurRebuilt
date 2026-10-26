// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

inline
bdUInt bdBitBuffer::getDataSize() const
{
	const bdUInt size = m_data.getSize();
	return size;
}

inline 
bdUInt bdBitBuffer::getNumBitsWritten() const
{
	bdUInt numBitsWritten = 0;
	if(m_maxWritePosition > 0)
	{
		numBitsWritten = m_maxWritePosition - BD_BB_NUM_HEADER_BITS;
	}
	return numBitsWritten;
}

inline
const bdUByte8* bdBitBuffer::getData() const
{
	const bdUByte8 *const head = m_data.begin();
	return head;
}

inline
bdUByte8* bdBitBuffer::getData()
{
	return m_data.begin();
}

inline 
bdBool bdBitBuffer::failedRead() const
{
	return m_failedRead;
}

inline 
bdUInt bdBitBuffer::getWritePosition() const
{
	return m_writePosition;
}

inline 
bdUInt bdBitBuffer::getReadPosition() const
{
	return m_readPosition;
}

inline 
void bdBitBuffer::setReadPosition(const bdUInt bitPosition)
{
	// don't allow the read position to be set inside the header
	// setting past the bits written is fine as any reads will fail
	m_readPosition = BD_MAX(bitPosition, BD_BB_NUM_HEADER_BITS);
}

inline 
void bdBitBuffer::resetReadPosition()
{
	// skip the first BD_BB_NUM_HEADER_BITS
	m_readPosition = BD_BB_NUM_HEADER_BITS;
}

inline 
void bdBitBuffer::setWritePosition(const bdUInt bitPosition)
{
	// don't allow the write position to be set inside the header
	m_writePosition = BD_MAX(bitPosition, BD_BB_NUM_HEADER_BITS);
}

inline 
void bdBitBuffer::resetWritePosition()
{
	// skip the first BD_BB_NUM_HEADER_BITS
	m_writePosition = BD_BB_NUM_HEADER_BITS;
}

inline
bdBool bdBitBuffer::writeBool(const bdBool b)
{
	writeDataType(BD_BB_BOOL_TYPE);
	if(b)
	{
		const bdUByte8 byte = 0xff;
		writeBits(&byte, 1);
	}
	else
	{
		const bdUByte8 byte = 0x0;
		writeBits(&byte, 1);
	}
	return b;
}

inline
void bdBitBuffer::writeNChar8(const bdNChar8 c)
{
	writeDataType(BD_BB_NATIVE_CHAR8_TYPE);
	writeBits(&c, 8);
}

inline
void bdBitBuffer::writeByte8(const bdByte8 b)
{
	writeDataType(BD_BB_SIGNED_BYTE8_TYPE);
	writeBits(&b, 8);
}

inline 
void bdBitBuffer::writeUByte8(const bdUByte8 b)
{
	writeDataType(BD_BB_UNSIGNED_BYTE8_TYPE);
	writeBits(&b, 8);
}

inline
void bdBitBuffer::writeInt16(const bdInt16 i)
{
	writeDataType(BD_BB_SIGNED_INTEGER16_TYPE);
	bdInt16 ni;
	endianSwap(i, ni);
	writeBits(&ni, 16);
}

inline
void bdBitBuffer::writeUInt16(const bdUInt16 u)
{
	writeDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
	bdUInt16 nu;
	endianSwap(u, nu);
	writeBits(&nu, 16);
}

inline
void bdBitBuffer::writeInt32(const bdInt32 i)
{
	writeDataType(BD_BB_SIGNED_INTEGER32_TYPE);
	bdInt ni;
	endianSwap(i, ni);
	writeBits(&ni, 32);
}

inline
void bdBitBuffer::writeUInt32(const bdUInt32 u)
{
	writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
	bdUInt nu;
	endianSwap(u, nu);
	writeBits(&nu, 32);
}

inline
void bdBitBuffer::writeInt64(const bdInt64 i)
{
	writeDataType(BD_BB_SIGNED_INTEGER64_TYPE);
	bdInt64 ni; 
	endianSwap(i, ni);
	writeBits(&ni, 64);
}

inline
void bdBitBuffer::writeUInt64(const bdUInt64 u)
{
	writeDataType(BD_BB_UNSIGNED_INTEGER64_TYPE);
	bdUInt64 nu;
	endianSwap(u, nu);
	writeBits(&nu, 64);
}

inline
void bdBitBuffer::writeFloat32(const bdFloat32 f)
{
	writeDataType(BD_BB_FLOAT32_TYPE);
	bdFloat32 nf;
	endianSwap(f, nf);
	writeBits(&nf, 32);
}

inline
void bdBitBuffer::writeFloat64(const bdFloat64 &f)
{
	writeDataType(BD_BB_FLOAT64_TYPE);
	bdFloat64 nf;
	endianSwap(f, nf);
	writeBits(&nf, 64);
}

inline
void bdBitBuffer::writeString(const bdNChar8 *const s, 
							  const bdUWord maxLen)
{
	writeDataType(BD_BB_NATIVE_CHAR8_STRING_TYPE);
	const bdUWord charsToCopy = bdStrnlen(s, maxLen);
	// copy the string data
	writeBits(s, static_cast<bdUInt>(charsToCopy << 3));
	// add a null terminator
	bdNChar8 null = '\0';
	writeBits(&null, 8);
}

inline
void bdBitBuffer::writeString(const bdString& s)
{
	writeDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
	// copy the string data and null terminator
	writeBits(s.getBuffer(), static_cast<bdUInt>((s.getLength() + 1) << 3));
}

inline
void bdBitBuffer::writeBlob(const void *const blob, 
							const bdUInt32 length)
{
	writeDataType(BD_BB_BLOB_TYPE);
	writeUInt32(length);
	writeBits(blob, length << 3);
}

inline
bdBool bdBitBuffer::readBool(bdBool &b)
{
	bdBool ok = readDataType(BD_BB_BOOL_TYPE);

	if (ok)
	{
		bdUByte8 byte = 0x0;
		ok = readBits(&byte, 1);

		if(ok)
		{
			if(byte)
			{
				b = true;
			}
			else
			{
				b = false;
			}
		}
	}

	return ok;
}


inline
bdBool bdBitBuffer::testBool()
{
	bdBool b = false;
	return readBool(b) && b;
}

inline
bdBool bdBitBuffer::readNChar8(bdNChar8 &c)
{
	bdBool ok = readDataType(BD_BB_NATIVE_CHAR8_TYPE);
	ok = ok && readBits(&c, 8);
	return ok;
}

inline
bdBool bdBitBuffer::readByte8(bdByte8 &b)
{
	bdBool ok = readDataType(BD_BB_SIGNED_BYTE8_TYPE);
	ok = ok && readBits(&b, 8);
	return ok;
}

inline 
bdBool bdBitBuffer::readUByte8(bdUByte8 &b)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_BYTE8_TYPE);
	ok = ok && readBits(&b, 8);
	return ok;
}

inline
bdBool bdBitBuffer::readInt16(bdInt16 &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER16_TYPE);
	bdInt16 ni;
	ok = ok && readBits(&ni, 16);
	if(ok)
	{
		endianSwap(ni, i);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readUInt16(bdUInt16 &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
	bdUInt16 nu;
	ok = ok && readBits(&nu, 16);
	if(ok)
	{
		endianSwap(nu, u);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readInt32(bdInt &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER32_TYPE);
	bdInt32 ni;
	ok = ok && readBits(&ni, 32);
	if(ok)
	{
		endianSwap(ni, i);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readUInt32(bdUInt &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
	bdUInt32 nu;
	ok = ok && readBits(&nu, 32);
	if(ok)
	{
		endianSwap(nu, u);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readInt64(bdInt64 &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER64_TYPE);
	bdInt64 ni;
	ok = ok && readBits(&ni, 64);
	if(ok)
	{
		endianSwap(ni, i);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readUInt64(bdUInt64 &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER64_TYPE);
	bdUInt64 nu;
	ok = ok && readBits(&nu, 64);
	if(ok)
	{
		endianSwap(nu, u);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readFloat32(bdFloat32 &f)
{
	bdBool ok = readDataType(BD_BB_FLOAT32_TYPE);
	bdFloat32 nf;
	ok = ok && readBits(&nf, 32);
	if(ok)
	{
		endianSwap(nf, f);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readFloat64(bdFloat64 &f)
{
	bdBool ok = readDataType(BD_BB_FLOAT64_TYPE);
	bdFloat64 nf;
	ok = ok && readBits(&nf, 64);
	if(ok)
	{
		endianSwap(nf, f);
	}
	return ok;
}

inline 
bdBool bdBitBuffer::getStringLength(bdUWord &length)
{
	const bdBool ok = readString(BD_NULL, 0, length);
	return ok;
}

inline 
bdBool bdBitBuffer::readString(bdNChar8 *const s, 
							   const bdUWord maxLen)
{
	bdUWord length = 0;
	const bdBool ok = readString(s, maxLen, length);

	if(ok && (length+1 > maxLen))
	{
		BD_WARN("bitbuffer", "Reading STRING (%u bytes) buffer to small (%u bytes).", length+1, maxLen);
	}
	return ok;
}

inline
bdBool bdBitBuffer::readString(bdNChar8 *const s,
							   const bdUWord maxLen,
							   bdUWord &length)
{
	const bdUInt startPosition = getReadPosition();

	bdBool ok = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
	if (ok)
	{
		bdUInt i = 0;
		bdNChar8 c;

		// keep reading till we hit a null terminator
		// even if we don't write the whole string to the destination
		do
		{
			// read a character
			c = '\0';
			ok = readBits(&c, 8);

			if((s != BD_NULL) && (i < maxLen))
			{
				s[i] = c;
			}

			if(ok && c != '\0')
			{
				i++;
			}			
		}
		while(ok && (c != '\0'));

		// ensure null ternimation of the dest
		if(s != BD_NULL && (maxLen > 0))
		{
			s[maxLen-1] = '\0';
		}

		length = i;
	}

	if(s == BD_NULL)
	{
		setReadPosition(startPosition);
	}

	return ok;
}

inline
bdBool bdBitBuffer::readString(bdString& s)
{
	bdBool ok = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
	if(ok)
	{
		bdFastArray<bdNChar8> array(32);
		bdNChar8 c;

		do
		{
			c = '\0';
			ok = readBits(&c, 8);
			array.pushBack(c);
		}
		while(ok && (c != '\0'));

		s = array.begin();
	}

	return ok;
}

inline 
bdBool bdBitBuffer::getBlobLength(bdUInt32& length)
{
	const bdUInt readPosition = getReadPosition();

	bdBool ok = readDataType(BD_BB_BLOB_TYPE);
	ok = ok && readUInt32(length);
	
	// reset the read position back to the start of the blob
	setReadPosition(readPosition);

	return ok;
}

inline
bdBool bdBitBuffer::readBlob(bdUByte8 *const blob, 
							 bdUInt32& length)
{	
	bdBool ok = readDataType(BD_BB_BLOB_TYPE);

	if(ok)
	{
		bdUInt32 tempLength = 0;
		ok = readUInt32(tempLength);
		if(ok)
		{
			const bdUInt readPosition = getReadPosition();

			if(blob != BD_NULL)
			{
				ok = readBits(blob, BD_MIN(length, tempLength) << 3);

				if(tempLength > length)
				{					
					BD_WARN("bitbuffer", "Reading BLOB (%u bytes) buffer too small (%u bytes).", tempLength, length);
				}
			}

			// skip to the end
			setReadPosition(readPosition + (tempLength << 3));
			length = tempLength;
		}
	}

	return ok;
}
