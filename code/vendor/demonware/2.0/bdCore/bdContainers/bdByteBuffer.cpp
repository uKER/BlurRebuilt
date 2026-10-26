// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdContainers/bdByteBuffer.h>

#define BD_LOG_LEVEL "core/bytebuffer"


bdByteBuffer::~bdByteBuffer()
{
	if(m_data)
	{
		bdDeallocate<bdUByte8>(m_data);
	}
	m_data = BD_NULL;
	m_readPtr = BD_NULL;
	m_writePtr = BD_NULL;
}

bdUInt bdByteBuffer::getDataSize() const
{
	return static_cast<bdUInt>(m_writePtr - m_data);
}

void bdByteBuffer::allocateBuffer()
{
	if(m_data)
	{
		BD_WARN(BD_LOG_LEVEL, "Buffer already allocated.");
	}
	else
	{
		m_data = bdAllocate<bdUByte8>(m_size);
		m_writePtr = m_data;
		m_readPtr = m_data;
	}
}

bdBool bdByteBuffer::write(const void *data, const bdUInt size)
{
	bdBool ok = false;
	if (m_data == BD_NULL)
	{
		m_size += size;
		return true;
	}
	bdUInt maxWriteSize = m_size - (m_writePtr - m_data);
	if(size <= maxWriteSize)
	{
		bdUInt temp;
		ok = bdBytePacker::appendBuffer(m_writePtr, maxWriteSize, 0, temp, data, size);
		if(ok)
		{
			m_writePtr += temp;
		}
	}
	else
	{
		BD_ASSERT(false, "Could not write data to buffer. Insufficient space.\n");
	}
	return ok;
}

bdBool bdByteBuffer::writeArrayStart(bdUByte8 type, bdUInt32 numElements, bdUInt32 elementSize)
{
	bdBool ok;
	type = BD_BB_ARRAY_START_TYPE + type;

	m_typeCheckedCopy = m_typeChecked;
	
	m_typeChecked = false;
	ok = writeUByte8(type);
	m_typeChecked = true;
	ok = ok && writeUInt32(numElements * elementSize);
	m_typeChecked = false;
	ok = ok && writeUInt32(numElements);

	return ok;
}

void bdByteBuffer::writeArrayEnd()
{
	m_typeChecked = m_typeCheckedCopy;
}

bdBool bdByteBuffer::readArrayStart(bdUByte8 expectedType, bdUInt32& numElements)
{
	bdBool ok;
	bdUByte8 type;

	m_typeCheckedCopy = m_typeChecked;
	m_typeChecked = false;

	ok = readUByte8(type);

	if (!ok)
	{
		BD_WARN("warn", "readArrayStart: No array %d\n");
		numElements = 0;
		return ok;
	}
	
	if (type - BD_BB_ARRAY_START_TYPE != expectedType)
	{
		BD_ERR("err", "readArrayStart: Expected type %d but read type %d\n", expectedType, type);
		ok = false;
	}

	if (ok)
	{
		m_typeChecked = false;
		bdUByte8 ignore;
		bdUInt32 ignoredArrayLength;
		ok = ok && readUByte8(ignore);
		ok = ok && readUInt32(ignoredArrayLength);
		ok = ok && readUInt32(numElements);
	}
	return ok;
}

void bdByteBuffer::readArrayEnd()
{
	m_typeChecked = m_typeCheckedCopy;
}

bdBool bdByteBuffer::writeBool(const bdBool b)
{
	bdBool ok;
	ok = writeDataType(BD_BB_BOOL_TYPE);
	ok = ok && write(b);
	return ok;
}

bdBool bdByteBuffer::writeNChar8(const bdNChar8 c)
{
	bdBool ok;
	ok = writeDataType(BD_BB_NATIVE_CHAR8_TYPE);
	ok = ok && write(c);
	return ok;
}

bdBool bdByteBuffer::writeByte8(const bdByte8 b)
{
	bdBool ok;
	ok = writeDataType(BD_BB_SIGNED_BYTE8_TYPE);
	ok = ok && write(b);
	return ok;
}

bdBool bdByteBuffer::writeUByte8(const bdUByte8 b)
{
	bdBool ok;
	ok = writeDataType(BD_BB_UNSIGNED_BYTE8_TYPE);
	ok = ok && write(b);
	return ok;
}

bdBool bdByteBuffer::writeInt16(const bdInt16 i)
{
	bdBool ok;
	ok = writeDataType(BD_BB_SIGNED_INTEGER16_TYPE);
	ok = ok && write(i);
	return ok;
}

bdBool bdByteBuffer::writeUInt16(const bdUInt16 u)
{
	bdBool ok;
	ok = writeDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
	ok = ok && write(u);
	return ok;
}

bdBool bdByteBuffer::writeInt32(const bdInt32 i)
{
	bdBool ok;
	if(i == BD_INT32_NAN)
	{
		ok = writeNAN();	
	}
	else
	{
		ok = writeDataType(BD_BB_SIGNED_INTEGER32_TYPE);
		ok = ok && write(i);
	}
	return ok;
}

bdBool bdByteBuffer::writeUInt32(const bdUInt32 u)
{
	bdBool ok;
	if(u == BD_UINT32_NAN)
	{
		ok = writeNAN();	
	}
	else
	{
		ok = writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
		ok = ok && write(u);
	}
	return ok;
}

bdBool bdByteBuffer::writeNAN()
{
	bdBool ok = false;
	if (m_typeChecked)
	{
		ok = writeDataType(BD_BB_NAN_TYPE);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL , "NaN types cannot be used on not type checked byte buffers");
	}
	return ok;
}

bdBool bdByteBuffer::writeNoType()
{
	bdBool ok = false;
	if (m_typeChecked)
	{
		ok = writeDataType(BD_BB_NO_TYPE);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL , "No type cannot be written to not type checked byte buffers");
	}
	return ok;
}

bdBool bdByteBuffer::writeInt64(const bdInt64 i)
{
	bdBool ok;
	if(i == BD_INT64_NAN)
	{
		ok = writeNAN();
	}
	else
	{
		ok = writeDataType(BD_BB_SIGNED_INTEGER64_TYPE);
		ok = ok && write(i);
	}
	return ok;
}

bdBool bdByteBuffer::writeUInt64(const bdUInt64 u)
{
	bdBool ok;
	if(u == BD_UINT64_NAN)
	{
		ok = writeNAN();
	}
	else
	{
		ok = writeDataType(BD_BB_UNSIGNED_INTEGER64_TYPE);
		ok = ok && write(u);
	}
	return ok;
}

bdBool bdByteBuffer::writeFloat32(const bdFloat32 f)
{
	bdBool ok;
	if(f == BD_FLOAT32_NAN)
	{
		ok = writeNAN();
	}
	else
	{
		ok = writeDataType(BD_BB_FLOAT32_TYPE);
		ok = ok && write(f);
	}
	return ok;
}

bdBool bdByteBuffer::writeFloat64(const bdFloat64 f)
{
	bdBool ok;
	if(f == BD_FLOAT64_NAN)
	{
		ok = writeNAN();
	}
	else
	{
		ok = writeDataType(BD_BB_FLOAT64_TYPE);
		ok = ok && write(f);
	}
	return ok;
}

bdBool bdByteBuffer::writeString(const bdNChar8 *const s,
                                 const bdUWord maxLen)
{
	bdBool ok = false;

	if (s == BD_NULL)
	{
		ok = writeNAN();
	}
	else
	{
		bdUInt lengthToWrite = 0;
		bdBool addNullTerminator = false;
		ok = writeDataType(BD_BB_NATIVE_CHAR8_STRING_TYPE);
		const bdUWord strLength = bdStrnlen(s, maxLen);
		if (strLength < maxLen)
		{
			// Since the string is less long than maxLen
			// it is already null terminated as this is how bdStrnlen
			// detects the length..
			// Also bdStrnlen does not include the null terminator in it's
			// returned length so we need to write length + 1
			lengthToWrite = static_cast<bdUInt>(strLength + 1);
			addNullTerminator = false;
		}
		else
		{
			// If the string is as long as max len [or longer] we will make
			// the last character null to guarantee null termination.
			lengthToWrite = static_cast<bdUInt>(maxLen - 1);
			addNullTerminator = true;
			BD_WARN(BD_LOG_LEVEL, "String was not null terminated. Data will be truncated.");
		}

		ok = ok && write(s, lengthToWrite);
		if(true == addNullTerminator)
		{
			// add a null terminator, as this is what readString() depends on
			bdUByte8 null = 0;
			ok = ok && write(null);
		}
	}
	return ok;
}

bdBool bdByteBuffer::writeBlob(const void *const blob,
							   const bdUInt32 length)
{
	bdBool ok;
	ok = writeDataType(BD_BB_BLOB_TYPE);
	ok = ok && writeUInt32(length);
	ok = ok && write(blob, length);
	return ok;
}

bdBool bdByteBuffer::read(void *data, bdUInt size)
{
	bdBool ok = false;
	bdUInt maxReadSize = m_size - (m_readPtr - m_data);
	if(size <= maxReadSize)
	{
		bdUInt temp;
		ok = bdBytePacker::removeBuffer(m_readPtr, maxReadSize, 0, temp, data, size);
		m_readPtr += size;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Could not read data from buffer. Insufficient data available.\n");
	}
	return ok;
}

bdBool bdByteBuffer::readBool(bdBool &b)
{
	bdBool ok = readDataType(BD_BB_BOOL_TYPE);

	if (ok)
	{
		bdUByte8 byte = 0x0;
		ok = read(&byte, 1);

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


bdBool bdByteBuffer::readNChar8(bdNChar8 &c)
{
	bdBool ok = readDataType(BD_BB_NATIVE_CHAR8_TYPE);
	ok = ok && read(c);
	return ok;
}


bdBool bdByteBuffer::readByte8(bdByte8 &b)
{
	bdBool ok = readDataType(BD_BB_SIGNED_BYTE8_TYPE);
	ok = ok && read(b);
	return ok;
}

bdBool bdByteBuffer::readUByte8(bdUByte8 &b)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_BYTE8_TYPE);
	ok = ok && read(b);
	return ok;
}

bdBool bdByteBuffer::readInt16(bdInt16 &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER16_TYPE);
	ok = ok && read(i);
	return ok;
}


bdBool bdByteBuffer::readUInt16(bdUInt16 &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
	ok = ok && read(u);
	return ok;
}


bdBool bdByteBuffer::readInt32(bdInt &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER32_TYPE);
	ok = ok && read(i);
	return ok;
}

bdBool bdByteBuffer::readUInt32(bdUInt &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
	ok = ok && read(u);
	return ok;
}

bdBool bdByteBuffer::readInt64(bdInt64 &i)
{
	bdBool ok = readDataType(BD_BB_SIGNED_INTEGER64_TYPE);
	ok = ok && read(i);
	return ok;
}

bdBool bdByteBuffer::readUInt64(bdUInt64 &u)
{
	bdBool ok = readDataType(BD_BB_UNSIGNED_INTEGER64_TYPE);
	ok = ok && read(u);
	return ok;
}

bdBool bdByteBuffer::readFloat32(bdFloat32 &f)
{
	bdBool ok = readDataType(BD_BB_FLOAT32_TYPE);
	ok = ok && read(f);
	return ok;
}

bdBool bdByteBuffer::readFloat64(bdFloat64 &f)
{
	bdBool ok = readDataType(BD_BB_FLOAT64_TYPE);
	ok = ok && read(f);
	return ok;
}

bdBool bdByteBuffer::getStringLength(bdUInt &length)
{
	// Grab the m_readPtr so we can reset our state.
	bdUByte8 *const startPos = m_readPtr;

	bdBool ok = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);

	bdUInt i = 0;
	bdNChar8 c;

	do
	{
		c = '\0';
		ok = read(&c, 1);

		if (ok && (c != '\0'))
		{
			++i;
		}
	}
	while (ok && (c != '\0'));

	m_readPtr = startPos;
	if (ok)
	{
		length = i;
	}

	return ok;
}

bdBool bdByteBuffer::readString(bdNChar8 *const s,
							   const bdUWord maxLen)
{

	bdBool ok = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
	if (ok && s != BD_NULL)
	{
		bdUInt i = 0;
		bdNChar8 c;

		// keep reading till we hit a null terminator
		// even if we don't write the whole string to the destination
		do
		{
			// read a character
			c = '\0';
			ok = read(&c, 1);

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

		// ensure null termination of the dest
		if(s != BD_NULL && (maxLen > 0))
		{
			s[maxLen-1] = '\0';
		}

	}

	return ok;
} 

bdBool bdByteBuffer::readBlob(bdUByte8 *const blob, 
							 bdUInt32& length)
{	
	bdBool ok = readDataType(BD_BB_BLOB_TYPE);

	if(ok)
	{
		bdUInt32 tempLength = 0;
		ok = readUInt32(tempLength);
		if(ok && blob)
		{

			ok = read(blob, BD_MIN(length, tempLength) );

			if(tempLength > length)
			{					
				BD_WARN(BD_LOG_LEVEL, "Reading BLOB (%u bytes) buffer too small (%u bytes).", tempLength, length);
			}
		

		}
		length = tempLength;

	}
	return ok;
}

bdBool bdByteBuffer::readNAN()
{
	bdBool ok = false;
	if (m_typeChecked)
	{
		ok = readDataType(BD_BB_NAN_TYPE);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL , "NaN types cannot be used on not type checked byte buffers");
	}
	return ok;
}

void bdByteBuffer::setTypeCheck(const bdBool flag)
{
	m_typeChecked = flag;
}

bdBitBufferDataType bdByteBuffer::readDataType()
{
	bdBitBufferDataType dataType = BD_BB_NO_TYPE;	
	bdUByte8 dataTypeTemp = BD_BB_NO_TYPE;
	const bdBool ok = read(&dataTypeTemp, 1);
	if (ok)
	{
		dataType = static_cast<bdBitBufferDataType>(dataTypeTemp);	
	}

	return dataType;
}

bdBool bdByteBuffer::writeDataType(const bdBitBufferDataType dataType)
{
	bdBool ok = true;
	bdUByte8 dataType8 = (bdUByte8)dataType;
	if (m_typeChecked)
	{
		ok = write(&dataType8, 1);
	}
	return ok;
}


bdBool bdByteBuffer::readDataType(const bdBitBufferDataType expectedDataType)
{
	bdBool ok = true;

	if (m_typeChecked)
	{
		bdUByte8 dataType8 = BD_BB_NO_TYPE;
		ok = read(&dataType8, 1);
		if (ok)		
		{
			const bdBitBufferDataType type = static_cast<bdBitBufferDataType>(dataType8);	
			ok = (type == expectedDataType);
			if(!ok)
			{		
				bdNChar8 string1[BD_BB_TYPE_MAX_STRING_LENGTH];
				bdBitBuffer::typeToString(expectedDataType, string1, sizeof(string1));
				bdNChar8 string2[BD_BB_TYPE_MAX_STRING_LENGTH];
				bdBitBuffer::typeToString(type, string2, sizeof(string2));

				BD_ERR(BD_LOG_LEVEL, "Expected: %s , read: %s ", string1, string2);
			}
		}
	}

	return ok;
}
