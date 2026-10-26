// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdBytePacker.h>

#define BD_LOG_LEVEL "byte packer"

//TODO: this method may read beyond the end of the src buffer
bdBool BD_CALL bdBytePacker::appendBuffer(void* dest,
										  const bdUInt32 destSize,
										  const bdUInt offset,
										  bdUInt &newOffset,
										  const void *const src,
										  const bdUInt writeSize)
{
	bdBool written = true;

	// always set newOffset
	newOffset = offset + writeSize;

	if(dest && src)
	{
		// check the start position is valid (use <= to allow zero sized writes)
		const bdBool validStart = offset <= destSize;
		BD_ASSERT(validStart, "Offset is past the end of the destination buffer.");

		// check the end position is valid (use <= to allow zero sized writes)
		const bdBool validEnd = (newOffset <= destSize);
		if(!validEnd)
		{
			BD_WARN(BD_LOG_LEVEL, "Not enough room left to write %u bytes.", writeSize);
		}

		written = validStart && validEnd;	
		if(written)
		{
			dest = reinterpret_cast<bdUByte8*>(dest) + offset;
			bdMemmove(dest, src, writeSize);
		}
	}

	return written;
}

bdBool BD_CALL bdBytePacker::appendEncodedUInt16(void* buffer,
												 const bdUInt bufferSize,
												 const bdUInt offset,
												 bdUInt &newOffset,
												 const bdUInt16 value)
{
	if (value > BD_BYTE8_MAX)
	{
		BD_ASSERT(value < 0x8000u, "Integer is too big.");
		// Encode the number by hand
		bdBool result = true;

		bdUByte8 firstByte = static_cast<bdUByte8>(value >> 8) | 0x80u;
		bdUByte8 secondByte = static_cast<bdUByte8>(value & 0xFFu);

		result = result && bdBytePacker::appendBasicType(buffer, bufferSize,
														 offset, newOffset,
														 firstByte);
		result = result && bdBytePacker::appendBasicType(buffer, bufferSize,
														 newOffset, newOffset,
														 secondByte);
		return result;
	}
	else
	{
		// Write as a single byte.
		return bdBytePacker::appendBasicType(buffer, bufferSize,
											 offset, newOffset,
											 static_cast<bdUByte8>(value));
	}
}

//TODO: this method may write beyond the end of the dest buffer
bdBool BD_CALL bdBytePacker::removeBuffer(const void* src,
										  const bdUInt srcSize,
										  const bdUInt offset,
										  bdUInt &newOffset,
										  void *const dest,
										  const bdUInt readSize)
{
	bdBool read = true;

	// always set newOffset
	newOffset = offset + readSize;

	if(dest && src)
	{
		// check the start position is valid (use <= to allow zero sized reads)
		const bdBool validStart = (offset <= srcSize);
		read = offset <= srcSize;
		BD_ASSERT(validStart, "Offset is past the end of the source buffer.");

		// check the end position is valid (use <= to allow zero sized reads)
		const bdBool validEnd = (newOffset <= srcSize);
		if(!validEnd)
		{
			BD_WARN(BD_LOG_LEVEL, "Not enough data left to read %u bytes.", readSize);
		}

		read = validEnd && validStart;
		if(read)
		{
			src = reinterpret_cast<const bdUByte8*>(src) + offset;
			bdMemmove(dest, src, readSize);
		}
	}

	return read;
}

bdBool BD_CALL bdBytePacker::removeEncodedUInt16(const bdUByte8* buffer,
												 const bdUInt bufferSize,
												 const bdUInt offset,
												 bdUInt &newOffset,
												 bdUInt16 &value)
{
	bdBool result = true;
	bdUByte8 firstByte = 0;

	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, newOffset, firstByte);

	if (firstByte >= 0x80u)
	{
		firstByte = firstByte & 0x7F;
		bdUByte8 secondByte = 0;
		result = result && bdBytePacker::removeBasicType(buffer, bufferSize, newOffset, newOffset, secondByte);

		value = (static_cast<bdUInt16>(firstByte) << 8) + static_cast<bdUInt16>(secondByte);
		return result;
	}
	else
	{
		value = firstByte;
		return result;
	}
}

bdBool BD_CALL bdBytePacker::skipBytes(const void *const buffer,
									   const bdUInt bufferSize,
									   const bdUInt offset,
									   bdUInt &newOffset,
									   const bdUInt bytes)
{
	// fail safe
	bdBool canSkip = false; 
	
	// always set newOffset
	newOffset = offset + bytes;
	
	if(buffer)
	{
		canSkip = newOffset <= bufferSize;
		BD_ASSERT(canSkip, "Skipping beyond the end of the buffer.");
	}

	return canSkip;
}

bdBool BD_CALL bdBytePacker::rewindBytes(const void *const buffer,
										 const bdUInt,
										 const bdUInt offset,
										 bdUInt &newOffset,
										 const bdUInt bytes)
{
	// fail safe
	bdBool canRewind = false; 

	// always set newOffset
	newOffset = offset - bytes;

	if(buffer)
	{
		canRewind = (offset >= bytes);
		BD_ASSERT(canRewind, "Rewinding past the start of the buffer.");
	}

	return canRewind;
}

