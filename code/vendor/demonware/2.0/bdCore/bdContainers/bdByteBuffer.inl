// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdCore/bdUtilities/bdBytePacker.h>

inline
bdByteBuffer::bdByteBuffer(const bdUInt size, bdBool isTypeChecked)
: m_size(size),
  m_data(BD_NULL),
  m_typeChecked(isTypeChecked),
  m_typeCheckedCopy(m_typeChecked)
{
	if (size > 0)
	{
		m_data = bdAllocate<bdUByte8>(size);
	}
	m_writePtr = m_data;
	m_readPtr = m_data;
}

inline
bdByteBuffer::bdByteBuffer(void *bytes, const bdUInt size, bdBool isTypeChecked)
: m_size(size),
  m_typeChecked(isTypeChecked),
  m_typeCheckedCopy(m_typeChecked)
{
	m_data = bdAllocate<bdUByte8>(size);
	m_writePtr = m_data;
	m_readPtr = m_data;
	bdMemcpy(m_data, bytes, size);
}


inline
bdUByte8& bdByteBuffer::operator[] (const bdUInt i)
{
	BD_ASSERT(i < m_size, "bdByteBuffer::operator[], "
		"i out of range.");

	return m_data[i];
}

inline
bdUByte8 bdByteBuffer::operator[] (const bdUInt i) const
{
	BD_ASSERT(i < m_size, "bdByteBuffer::operator[], "
		"i out of range.");

	return m_data[i];
}

inline
bdUInt bdByteBuffer::getSize() const
{
	return m_size;
}

inline
bdUByte8* bdByteBuffer::getData()
{
	return m_data;
}

inline
const bdUByte8* bdByteBuffer::getData() const
{
	return m_data;
}

bdUInt bdByteBuffer::getMaxWriteSize() const
{
	return m_size - (m_writePtr - m_data);
}

bdUInt bdByteBuffer::getMaxReadSize() const
{
	return m_size - (m_readPtr - m_data);
}

inline
void bdByteBuffer::reset()
{
	m_writePtr = m_data;
	m_readPtr = m_data;
}
