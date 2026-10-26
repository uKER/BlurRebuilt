// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdCore/bdUtilities/bdBytePacker.h>

template <typename T>
bdCircularBuffer<T>::bdCircularBuffer(const bdUInt size, 
									  T *const buf, 
									  const bdBool wrappingAllowed)
: m_canWrap(wrappingAllowed),
  m_size(size),
  m_data(buf),
  m_allocated(false)
{
	if(!m_data)
	{
		m_data = bdAllocate<T>(size);
		m_allocated = true;
	}

	m_dataEnd = m_data + size;
	m_start = m_data;
	m_end = m_data;
}

template <typename T>
bdCircularBuffer<T>::~bdCircularBuffer()
{
	if(m_allocated)
	{
		bdDeallocate<T>(m_data);
	}
}

template <typename T>
T* bdCircularBuffer<T>::getFreeBlockStart()
{
	return m_end;
}

template <typename T>
bdUInt bdCircularBuffer<T>::getFreeBlockSize()
{
	if (m_end >= m_start)
	{
		return static_cast<bdUInt>(m_dataEnd - m_end);
	}
	else
	{
		BD_ASSERT(m_canWrap, "Buffer seems wrapped, but not allowed.");
		return static_cast<bdUInt>((m_start - m_end) - 1);
	}
}

template <typename T>
void bdCircularBuffer<T>::protectBlock(bdUInt size)
{
	BD_ASSERT(size <= getFreeBlockSize(), "Tried to put more data than allowed.");
	m_end += size;
	attemptWrap();
}

template <typename T>
void bdCircularBuffer<T>::attemptWrap()
{
	// Only wrap if allowed, if no space at the end, and
	// there's space at the beginning
	if (m_canWrap && m_end == m_dataEnd && m_start > m_data)
	{
		m_end = m_data;
	}

	// Also wrap m_start, don't care about space at the beginning
	if (m_canWrap && m_start == m_dataEnd)
	{
		m_start = m_data;
	}
}

template <typename T>
T* bdCircularBuffer<T>::getUsedBlockStart()
{
	return m_start;
}

template <typename T>
bdUInt bdCircularBuffer<T>::getUsedBlockSize()
{
	if (m_end >= m_start)
	{
		return static_cast<bdUInt>(m_end - m_start);
	}
	else
	{
		BD_ASSERT(m_canWrap, "Buffer seems wrapped, but not allowed.");
		return static_cast<bdUInt>(m_dataEnd - m_start);
	}
}

template <typename T>
void bdCircularBuffer<T>::freeBlock(bdUInt size)
{
	BD_ASSERT(size <= getUsedBlockSize(), "Tried to remove more data than available.");
	m_start += size;
	attemptWrap();
}

template <typename T>
void bdCircularBuffer<T>::alignUsedBlock()
{
	BD_ASSERT(!m_canWrap, "Aligning not implemented on wrappable buffers.");
	::bdMemmove(m_data, getUsedBlockStart(), getUsedBlockSize()*sizeof(T));
	const bdUInt movedBy = static_cast<bdUInt>(m_start - m_data);
	m_start = m_data;
	m_end -= movedBy;
}

template <typename T>
bdUInt bdCircularBuffer<T>::getTotalFreeSpace()
{
	if (m_end >= m_start)
	{
		return static_cast<bdUInt>((m_start - m_data) + getFreeBlockSize());
	}
	else
	{
		return static_cast<bdUInt>(getFreeBlockSize());
	}
}

template <typename T>
bdUInt bdCircularBuffer<T>::getTotalUsedSpace()
{
	if (m_end >= m_start)
	{
		return static_cast<bdUInt>(m_end - m_start);
	}
	else
	{
		return static_cast<bdUInt>((m_dataEnd - m_start) + (m_end - m_data));
	}
}

template <typename T>
bdBool bdCircularBuffer<T>::appendBuffer(const T* const var, const bdUInt size)
{
	bdUInt offset = 0;
	bdBool result = true;
	T *const oldStart = m_start;
	T *const oldEnd = m_end;
	
	result = result && bdBytePacker::appendBuffer(getFreeBlockStart(),
													getFreeBlockSize(),
													0,
													offset,
													var,
													BD_MIN(size, getFreeBlockSize()));
	BD_ASSERT(result, "Operation should never fail.");
	protectBlock(offset);
	if (offset < size)
	{
		bdUInt newOffset = offset;
		result = result && bdBytePacker::appendBuffer(getFreeBlockStart(),
													getFreeBlockSize(),
													0,
													newOffset,
													var+offset,
													size-offset);
		if (result)
		{
			protectBlock(newOffset);
		}
	}

	// Operations above are guaranteed not to have corrupted any of the data already
	// in the buffer, so simply roll them back.
	if (!result)
	{
		m_start = oldStart;
		m_end = oldEnd;
	}

	return result;
}

template <typename T>
bdBool bdCircularBuffer<T>::removeBuffer(T *const var, const bdUInt size)
{
	bdUInt offset = 0;
	bdBool result = true;
	T *const oldStart = m_start;
	T *const oldEnd = m_end;

	result = result && bdBytePacker::removeBuffer(getUsedBlockStart(),
												getUsedBlockSize(),
												0,
												offset,
												var,
												BD_MIN(size, getUsedBlockSize()));
	BD_ASSERT(result, "Operation should never fail.");
	freeBlock(offset);
	if (offset < size)
	{
		bdUInt newOffset = offset;
		result = result && bdBytePacker::removeBuffer(getUsedBlockStart(),
												getUsedBlockSize(),
												0,
												newOffset,
												var+offset,
												size-offset);
		if (result)
		{
			freeBlock(newOffset);
		}
	}

	if (!result)
	{
		m_start = oldStart;
		m_end = oldEnd;
	}

	return result;
}
