// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdContainers/bdBitBuffer.h>

#include <bdCore/bdUtilities/bdBitOperations.h>

#define BD_LOG_LEVEL "bdCore/bitBuffer"

bdBitBuffer::bdBitBuffer(const bdUInt capacityBits,
						 const bdBool typeChecked)
: m_data(BD_NUM_BITS_TO_NUM_BYTES(capacityBits)),
  m_writePosition(0),
  m_maxWritePosition(0),
  m_readPosition(0),
  m_failedRead(false),
  m_typeChecked(typeChecked)
{
	// write whether or not the bitBuffer is typechecked
	if(m_typeChecked)
	{
		const bdUByte8 byte = 0xff;
		writeBits(&byte, 1);
	}
	else
	{
		const bdUByte8 byte = 0x0;
		writeBits(&byte, 1);
	}

	BD_ASSERT(BD_BB_NUM_HEADER_BITS == m_writePosition, "BD_BB_NUM_HEADER_BITS and written header don't match.");

	// set the read position to the point after type checked flag
	resetReadPosition();
}

bdBitBuffer::bdBitBuffer(const bdUByte8 *bits,
						 const bdUInt numBits,
						 const bdBool dataHasTypeCheckedBit)
: m_writePosition(0),
  m_maxWritePosition(0),
  m_readPosition(0),
  m_failedRead(false),
  m_typeChecked(false)
{
	if ((!dataHasTypeCheckedBit) || (numBits == 0))
	{
		// add a type checked bit, buffer cannot be type checked in this case
		const bdUByte8 byte = 0x0;
		writeBits(&byte, 1);
		writeBits(bits, numBits);

		// set the read position to the point after type checked flag
		resetReadPosition();
	}
	else
	{
		m_data.pushBack(bits, BD_NUM_BITS_TO_NUM_BYTES(numBits));		
		m_writePosition = numBits;
		m_maxWritePosition = numBits;
		
		// read whether or not the bitBuffer is type checked
		BD_ASSERT(readBits(&m_typeChecked, 1), "bdBitBuffer constructor failed: could not read bits");
	}
}

bdBitBuffer::~bdBitBuffer()
{
}

void bdBitBuffer::writeRangedUInt32(const bdUInt u,
									const bdUInt begin,
									const bdUInt end, 
									const bdBool typeChecked)
{
//	BD_ASSERT(u >= begin && u <= end, "bdBitBuffer::writeRangedUInt, u is out of range.");
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedUInt, end of range is less than the begining");

	if(typeChecked)
	{
		writeDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);

		// if this is a debug buffer, then write range data
		if (m_typeChecked)
		{
			writeUInt32(begin);
			writeUInt32(end);
		}
	}

	const bdUInt rangeSize = end - begin;
	bdUInt rangeBits  = 0;
	if (rangeSize)
	{
		rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
	}

	const bdUInt v = BD_CLAMP(u, begin, end);
	bdUInt norm = v - begin;
	bdUInt nnorm;
	endianSwap(norm, nnorm);

	writeBits(&nnorm, rangeBits);
}

void bdBitBuffer::writeRangedInt32(const bdInt i,
								   const bdInt begin,
								   const bdInt end)
{
//	BD_ASSERT(i >= begin && i <= end, "bdBitBuffer::writeRangedInt32, u is out of range.");
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedInt32, end of range is less than the begining");

	writeDataType(BD_BB_RANGED_SIGNED_INTEGER32_TYPE);

	// if this is a debug buffer, then write range data
	if (m_typeChecked)
	{
		writeInt32(begin);
		writeInt32(end);
	}

	const bdUInt rangeSize = static_cast<bdUInt>(end - begin);
	bdUInt rangeBits  = 0;
	if (rangeSize)
	{
		rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
	}

	const bdInt v = BD_CLAMP(i, begin, end);
	bdUInt norm = static_cast<bdUInt>(v - begin);
	bdUInt nnorm;
	endianSwap(norm, nnorm);

	writeBits(&nnorm, rangeBits);
}


void bdBitBuffer::writeRangedFloat32(const bdFloat32 f,
									 const bdFloat32 begin,
									 const bdFloat32 end,
									 const bdFloat32 precision)
{
//	BD_ASSERT(f >= begin && f <= end, "bdBitBuffer::writeRangedFloat32, f is out of range.");
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
	BD_ASSERT(precision > 0, "bdBitBuffer::writeRangedFloat32, precision must be positive.");
	
	writeDataType(BD_BB_RANGED_FLOAT32_TYPE);

	// if this is a debug buffer, then write range data
	if (m_typeChecked)
	{
		writeFloat32(begin);
		writeFloat32(end);
		writeFloat32(precision);
	}

	const bdFloat32 absPrecision = precision > 0 ? precision : -precision;
	const bdFloat32 range = (end - begin) / absPrecision;

	if (range > BD_UINT32_MAX)
	{
		BD_WARN(BD_LOG_LEVEL, "The numerical space defined by range/precision combination is too large. No compression performed.");

		writeFloat32(f);
	}
	else
	{
		const bdUInt rangeSize = static_cast<bdUInt>(range);

		bdUInt rangeBits = 0;
		if (rangeSize)
		{
			rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
		}

		bdFloat32 v = BD_CLAMP(f, begin, end);
		v = (v - begin) / absPrecision;

		// cast and round to the nearest integer (note: v is always positive)
		bdFloat32 newVal = v + 0.5f;
		if(newVal > range)
		{
			newVal = range;
		}
		bdUInt i = static_cast<bdUInt>(newVal);		
		bdUInt ni;
		endianSwap(i, ni);
		writeBits(&ni, rangeBits);
	}
}

void bdBitBuffer::writeBits(const void *bits,
							const bdUInt numBits)
{
	// ensure there is enough space in the buffer
	const bdUInt lastByteIndex = (m_writePosition + numBits - 1)  >> 3;	
	if (!m_data.rangeCheck(lastByteIndex))
	{
		m_data.setGrow(lastByteIndex, 0);
	}

	bdUInt bitsToWrite = numBits;
	const bdUByte8 *src = reinterpret_cast<const bdUByte8*>(bits);

	while(bitsToWrite)
	{
		// create a mask to mask out the bits to be written in the destination byte

		// number of bits already written (m_writePosition % 8)
		const bdUInt upShift = m_writePosition & 0x7;
		const bdUInt downShift = 8 - upShift;

		// how bits are we going to write to this byte
		const bdUInt bitsWritten = BD_MIN(bitsToWrite, downShift);
		
		// make the mask
		const bdUByte8 mask = static_cast<bdUByte8>((0xff << (bitsWritten + upShift)) | (0xff >> downShift));
		
		// copy the byte we are writing to and mask out the bits to be written
		const bdUInt nextDestByteIndex = m_writePosition >> 3;
		const bdUByte8 maskedDest = m_data[nextDestByteIndex] & mask;

		// create a byte from the src taking into account any bits already read

		const bdUInt srcBitsRead = numBits - bitsToWrite;
		const bdUInt srcUpShift = srcBitsRead & 0x7;	
		const bdUInt srcDownShift = 8 - srcUpShift;	

		const bdUInt currentSrcByteIndex = srcBitsRead >> 3;
		bdUByte8 currentSrcByte = src[currentSrcByteIndex];
		
		// is there a following byte in the src buffer
		bdUByte8 nextSrcByte = 0x00;
		const bdUInt lastSrcByteIndex = (numBits - 1)  >> 3;
		if(lastSrcByteIndex > currentSrcByteIndex)
		{
			nextSrcByte = src[currentSrcByteIndex+1];
		}

		currentSrcByte = static_cast<bdUByte8>((currentSrcByte >> srcUpShift) | 
											   (nextSrcByte << srcDownShift));

		// shift the bits to write to the correct position and mask off any unwanted bits
		const bdUByte8 invMask = (0xff ^ mask);
		const bdUByte8 maskedSrc = static_cast<bdUByte8>(((currentSrcByte << upShift) & invMask));

		// combine the src and dest bytes into the destination
		m_data[nextDestByteIndex] = maskedDest | maskedSrc;

		m_writePosition += bitsWritten;
		bitsToWrite -= bitsWritten;	
		m_maxWritePosition = BD_MAX(m_maxWritePosition, m_writePosition);
	}
}

bdBool bdBitBuffer::readRangedUInt32(bdUInt &u,
									 const bdUInt begin,
									 const bdUInt end,
									 const bdBool typeChecked)
{	
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedUInt, end of range is less than the begining");

	bdBool ok = true;

	if(typeChecked)
	{
		ok = readDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);

		// if this is a debug buffer, read write range data
		if (m_typeChecked)
		{
			// these values are only used to output XML from a type checked buffer
			bdUInt32 bufBegin = 0;
			bdUInt32 bufEnd = 0;
			ok = ok && readUInt32(bufBegin);
			ok = ok && readUInt32(bufEnd);

			if(ok && ((begin != bufBegin) || (end != bufEnd)))
			{
				BD_ERR(BD_LOG_LEVEL, "Range error. Expected: (%u,%u), read: (%u,%u)", begin, end, bufBegin, bufEnd);
			}
		}
	}

	if (ok)
	{
		const bdUInt rangeSize = end - begin;
		bdUInt rangeBits  = 0;
		if (rangeSize)
		{
			rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
		}
		bdUInt nu = 0;
		ok = ok && readBits(&nu, rangeBits);
		if(ok)
		{
			endianSwap(nu, u);
			u += begin;

			BD_ASSERT(u >= begin && u <= end, "bdBitBuffer::readRangedUInt32, read error u is out of range.");

			u = BD_CLAMP(u, begin, end);
		}
	}

	return ok;
}

bdBool bdBitBuffer::readRangedInt32(bdInt &i,
									const bdInt begin,
									const bdInt end)
{	
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedInt32, end of range is less than the begining");

	bdBool ok = readDataType(BD_BB_RANGED_SIGNED_INTEGER32_TYPE);

	// if this is a debug buffer, then read range data
	if (m_typeChecked)
	{
		bdInt32 bufBegin = 0;
		bdInt32 bufEnd = 0;
		ok = ok && readInt32(bufBegin);
		ok = ok && readInt32(bufEnd);

		if(ok && ((begin != bufBegin) || (end != bufEnd)))
		{
			BD_ERR(BD_LOG_LEVEL, "Range error. Expected: (%i,%i), read: (%i,%i)", begin, end, bufBegin, bufEnd);
		}
	}

	if (ok)
	{
		const bdUInt rangeSize = static_cast<bdUInt>(end - begin);
		bdUInt rangeBits  = 0;
		if (rangeSize)
		{
			rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
		}

		bdInt ni = 0;
		ok = ok && readBits(&ni, rangeBits);
		if(ok)
		{
			endianSwap(ni, i);
			i += begin;

			BD_ASSERT(i >= begin && i <= end, "bdBitBuffer::readRangedInt32, read error i is out of range.");

			i = BD_CLAMP(i, begin, end);
		}
	}

	return ok;
}

bdBool bdBitBuffer::readRangedFloat32(bdFloat32 &f,
									  const bdFloat32 begin,
									  const bdFloat32 end,
									  const bdFloat32 precision)
{
	BD_ASSERT(end >= begin, "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
	BD_ASSERT(precision > 0, "bdBitBuffer::writeRangedFloat32, precision must be positive.");
	
	bdBool ok = readDataType(BD_BB_RANGED_FLOAT32_TYPE);

	// if this is a debug buffer, then read range data
	if (m_typeChecked)
	{
		bdFloat32 bufBegin = 0;
		bdFloat32 bufEnd = 0;
		bdFloat32 bufPrecision = 0;
		ok = ok && readFloat32(bufBegin);
		ok = ok && readFloat32(bufEnd);
		ok = ok && readFloat32(bufPrecision);

		if(ok && ((begin != bufBegin) || (end != bufEnd) || (precision != bufPrecision)))
		{
			BD_ERR(BD_LOG_LEVEL, "Range error. Expected: (%f,%f,%f), read: (%f,%f,%f)", begin, end, precision, bufBegin, bufEnd, bufPrecision);
		}
	}

	if (ok)
	{
		const bdFloat32 absPrecision = precision > 0 ? precision : -precision;
		const bdFloat32 range = (end - begin) / absPrecision;
		
		if (range > BD_UINT32_MAX)
		{
			BD_WARN(BD_LOG_LEVEL,"The numerical space defined by range/precision combination is too large. No compression performed.");

			ok = readFloat32(f);
		}
		else
		{
			const bdUInt rangeSize = static_cast<bdUInt>(range);
			bdUInt rangeBits = 0;
			if (rangeSize)
			{
				rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
			}

			bdUInt ni = 0;
			ok = readBits(&ni, rangeBits);
			if(ok)
			{
				bdUInt i;
				endianSwap(ni, i);
				f = begin + (i * absPrecision);
			}
		}

		if(ok)
		{
			BD_ASSERT(f >= begin && f <= end, "bdBitBuffer::readRangedFloat32, read error f is out of range.");
			f = BD_CLAMP(f, begin ,end);
		}
	}

	return ok;
}

bdBool bdBitBuffer::readBits(void *bits,
							 bdUInt numBits)
{
	if(!numBits)
	{
		return true;
	}

	if(m_readPosition + numBits > m_maxWritePosition)
	{
		m_failedRead = true;
		return false;
	}

	bdUByte8 *dest = reinterpret_cast<bdUByte8*>(bits);

	// index to the byte we will be reading from (m_readPosition / 8)
	bdUInt nextByteIndex = m_readPosition >> 3;

	while (numBits)
	{
		if(!m_data.rangeCheck(nextByteIndex))
		{			
			m_failedRead = true;			
			return false;
		}

		const bdUInt numBitsToRead = numBits < 8 ? numBits : 8;
		const bdUByte8 byte0 = m_data[nextByteIndex++];
		const bdUInt downShift = m_readPosition & 0x7;		

		if ((m_readPosition & 0x7) + numBitsToRead > 8)
		{
			// reading spans two bytes	
			if(!m_data.rangeCheck(nextByteIndex))
			{
				m_failedRead = true;
				return false;
			}

			const bdUByte8 byte1 = m_data[nextByteIndex];
			const bdUInt upShift = 8 - downShift;

			*dest++ = static_cast<bdUByte8>(((byte0 >> downShift) | (byte1 << upShift)) & (0xff >> (8 - numBitsToRead)));			
		}
		else
		{
			// reading from a single byte
			*dest++ = static_cast<bdUByte8>((byte0 >> downShift) & (0xff >> (8 - numBitsToRead)));
		}

		m_readPosition += numBitsToRead;
		numBits -= numBitsToRead;
	}	

	return true;
}

void bdBitBuffer::setTypeCheck(const bdBool flag)
{
	m_typeChecked = flag;
}

bdBool bdBitBuffer::getTypeCheck() const
{
	return m_typeChecked;
}

void bdBitBuffer::writeDataType(const bdBitBufferDataType dataType)
{
	if (m_typeChecked)
	{
		writeRangedUInt32(dataType, BD_BB_NO_TYPE, BD_BB_MAX_TYPE-1, false);
	}
}


bdBool bdBitBuffer::readDataType(const bdBitBufferDataType expectedDataType)
{
	bdBool ok = true;

	if (m_typeChecked)
	{
		bdUInt32 dataType32 = BD_BB_NO_TYPE;
		ok = readRangedUInt32(dataType32, BD_BB_NO_TYPE, BD_BB_MAX_TYPE-1, false);
		if (ok)		
		{
			const bdBitBufferDataType type = static_cast<bdBitBufferDataType>(dataType32);	
			ok = (type == expectedDataType);
			if(!ok)
			{		
				bdNChar8 string1[BD_BB_TYPE_MAX_STRING_LENGTH];
				typeToString(expectedDataType, string1, sizeof(string1));
				bdNChar8 string2[BD_BB_TYPE_MAX_STRING_LENGTH];
				typeToString(type, string2, sizeof(string2));

				BD_ERR(BD_LOG_LEVEL, "Expected: %s , read: %s ", string1, string2);
			}
		}
	}

	return ok;
}
		
bdBitBufferDataType bdBitBuffer::readDataType()
{
	bdBitBufferDataType dataType = BD_BB_NO_TYPE;	
	bdUInt32 dataType32 = BD_BB_NO_TYPE;
	const bdBool ok = readRangedUInt32(dataType32, BD_BB_NO_TYPE, BD_BB_MAX_TYPE-1, false);
	if (ok)
	{
		dataType = static_cast<bdBitBufferDataType>(dataType32);	
	}

	return dataType;
}

bdBool bdBitBuffer::append(bdBitBuffer &other)
{
	// bitbuffers have to be of the same type
	bdBool ok = (m_typeChecked == other.m_typeChecked);

	if(ok)
	{
		const bdUInt oldReadPos = other.getReadPosition();
		other.resetReadPosition();
	
		void *const tempData = bdMemory::allocate(other.getDataSize());
		ok = (tempData != BD_NULL);
		const bdUInt numBits = other.getNumBitsWritten();
		ok = ok && other.readBits(tempData, numBits);
	
		if(ok)
		{
			writeBits(tempData, numBits);
		}
	
		bdMemory::deallocate(tempData);
		other.setReadPosition(oldReadPos);
	}
	else
	{
		const bdNChar8* thisBuffer = m_typeChecked?"is not type checked":"is type checked";
		const bdNChar8* otherBuffer = other.m_typeChecked?"is not type checked":"is type checked";
		BD_ERR(BD_LOG_LEVEL, "Attempt made to append a bdBitBuffer that %s to a bdBitBuffer that %s." ,thisBuffer, otherBuffer );
	}

	return ok;
}

void BD_CALL bdBitBuffer::typeToString(const bdBitBufferDataType type, 
									   bdNChar8 *const strBuffer,
									   const bdUWord strLength)
{
#define BD_BIT_BUFFER_TYPES_BEGIN	const bdNChar8* dataTypeDescs[] = {
#define BD_BIT_BUFFER_TYPE(x, y)	#y,
#define BD_BIT_BUFFER_TYPES_END(x)	"Unknown Type" };

#include <bdCore/bdContainers/bdBitBufferTypes.h>

#undef BD_BIT_BUFFER_TYPES_BEGIN
#undef BD_BIT_BUFFER_TYPE
#undef BD_BIT_BUFFER_TYPES_END

	const bdUInt numDesc = sizeof(dataTypeDescs) / sizeof(bdNChar8*);
	const bdBitBufferDataType minDataType = static_cast<bdBitBufferDataType>(0);
	const bdBitBufferDataType maxDataType = static_cast<bdBitBufferDataType>(numDesc-1);
	const bdBitBufferDataType clampedType = BD_CLAMP(type, minDataType, maxDataType);

	bdStrlcpy(strBuffer, dataTypeDescs[clampedType], strLength);
}

