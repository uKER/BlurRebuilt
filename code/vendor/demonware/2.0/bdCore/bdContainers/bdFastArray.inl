// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

template <typename T> inline
bdFastArray<T>::bdFastArray(const bdUInt capacity)
: m_data(BD_NULL),
  m_capacity(capacity),
  m_size(0)
{
	if(m_capacity > 0)
	{
		m_data = bdAllocate<T>(m_capacity);
	}
}

template <typename T> inline
bdFastArray<T>::bdFastArray(const bdUInt capacity,
							const T& value)
: m_data(BD_NULL),
  m_capacity(capacity),
  m_size(capacity)
{
	if(m_capacity > 0)
	{
		m_data = bdAllocate<T>(m_capacity);
		copyArrayObject(m_data, value, m_capacity);
	}
}

template <typename T> inline
bdFastArray<T>::bdFastArray(const bdFastArray<T> &a)
: m_capacity(a.getCapacity()),
  m_size(a.getSize())
{
	m_data = uninitializedCopy(a);
}

template <typename T> inline
bdFastArray<T>::~bdFastArray()
{
	clear();
}

template <typename T> inline
void bdFastArray<T>::operator= (const bdFastArray<T> &a)
{
	// check for self assignment
	if (this != &a)
	{
		const bdUInt newSize = a.getSize();

		if (newSize > m_capacity)
		{
			// Too small, need to reallocate and copy.
			clear();
			m_data = uninitializedCopy(a);
			m_capacity = a.m_capacity;
			m_size = newSize;
		}
		else
		{
			copyArrayArray(m_data, a.m_data, newSize);
			m_size = newSize;
			decreaseCapacity();
		}
	}
}

template <typename T> inline
T& bdFastArray<T>::operator[] (const bdUInt i)
{
	BD_ASSERT(rangeCheck(i), "bdFastArray<T>::operator[], rangecheck failed");

	return m_data[i];
}

template <typename T> inline
const T& bdFastArray<T>::operator[] (const bdUInt i) const
{
	BD_ASSERT(rangeCheck(i), "bdFastArray<T>::operator[], rangecheck failed");

	return m_data[i];
}

template <typename T> inline
bdBool bdFastArray<T>::isEmpty() const
{
	return m_size == 0;
}

template <typename T> inline
bdUInt bdFastArray<T>::getSize() const
{
	return m_size;
}

template <typename T> inline
bdUInt bdFastArray<T>::getCapacity() const
{
	return m_capacity;
}

template <typename T> inline
void bdFastArray<T>::increaseCapacity(const bdUInt increase)
{
	// Note that the size does not change, just the capacity.
	const bdUInt newCapacity = m_capacity + ((increase>m_capacity)?increase:m_capacity);

	T* newData = BD_NULL;
	if(newCapacity > 0)
	{
		newData = bdAllocate<T>(newCapacity);
		copyArrayArray(newData, m_data, m_size);
	}

	bdDeallocate<T>(m_data);

	m_data = newData;
	m_capacity = newCapacity;
}

template <typename T> inline
void bdFastArray<T>::decreaseCapacity(const bdUInt decrease)
{
	if (m_capacity > (m_size << 2))
	{
		const bdUInt spare = m_capacity - m_size;
		const bdUInt possibleDecrease = (decrease>spare)?spare:decrease;

		m_capacity -= (possibleDecrease>(m_capacity >> 1))?possibleDecrease:(m_capacity >> 1);

		T* newData = BD_NULL;
		if(m_capacity > 0)
		{
			newData = bdReallocate<T>(m_data, m_capacity);
		}

		m_data = newData;
	}
}

template <typename T> inline
void bdFastArray<T>::ensureCapacity(const bdUInt capacity)
{
	if (m_capacity < capacity)
	{
		const bdUInt increase = capacity - m_capacity;
		increaseCapacity(increase);
	}
}


template <typename T> inline
bdBool bdFastArray<T>::get(const bdUInt i,
						   T &value) const
{
	const bdBool inRange = rangeCheck(i);
	if (inRange)
	{
		value = m_data[i];
	}

	return inRange;
}

template <typename T> inline
void bdFastArray<T>::getSection(const bdUInt begin,
								const bdUInt end,
								T *value) const
{
	bdBool inRange = rangeCheck(begin);
	inRange = inRange && (end <= m_size) && (begin < end);

	if(inRange)
	{
		const bdUInt numToCopy = end - begin;
		copyArrayArray(value, m_data+begin, numToCopy);
	}
}

template <typename T> inline
bdBool bdFastArray<T>::set(const bdUInt i,
						   const T &value)
{
	const bdBool inRange = rangeCheck(i);
	if (inRange)
	{
		m_data[i] = value;
	}

	return inRange;
}

template <typename T> inline
void bdFastArray<T>::setGrow(const bdUInt i,
							 const T &value)
{
	const bdBool inRange = rangeCheck(i);

	if(!inRange)
	{
		ensureCapacity(i+1);
		m_size = i+1;
	}

	m_data[i] = value;
}

template <typename T> inline
void bdFastArray<T>::pushBack(const T &value)
{
	if (m_size == m_capacity)
	{
		increaseCapacity(1);
	}

	copyObjectObject(m_data + m_size, value);
	m_size++;
}

template <typename T> inline
void bdFastArray<T>::pushBack(const T *value, const bdUInt n)
{
	ensureCapacity(m_size + n);
	copyArrayArray(m_data + m_size, value, n);
	m_size += n;
}

template <typename T> inline
void bdFastArray<T>::popBack()
{
	if (m_size > 0)
	{
		m_size--;
		decreaseCapacity();
	}
}

template <typename T> inline
void bdFastArray<T>::popBack(const bdUInt n)
{
	if (n < m_size)
	{
		m_size -= n;
	}
	else
	{
		m_size = 0;
	}

	decreaseCapacity();
}

template <typename T> inline
bdBool bdFastArray<T>::rangeCheck(const bdUInt i) const
{
	return i < m_size;
}

template <typename T> inline
bdBool bdFastArray<T>::findFirst(const T &value,
								 bdUInt& i) const
{
	for(bdUInt j = 0; j < m_size; j++)
	{
		if ( value == m_data[j] )
		{
			i = j;
			return true;
		}
	}

	return false;
}

template <typename T> inline
void bdFastArray<T>::removeAt(const bdUInt i)
{
	const bdBool inRange = rangeCheck(i);
	if (inRange)
	{
		m_data[i] = m_data[m_size-1];
		popBack();
	}
}

template <typename T> inline
void bdFastArray<T>::removeSection(const bdUInt begin,
								   const bdUInt end)
{
	bdBool inRange = rangeCheck(begin);
	inRange = inRange && (end <= m_size) && (begin < end);

	if(inRange)
	{
		const bdUInt numToRemove = end - begin;
		bdMemmove(m_data + begin, m_data + end, (m_size - end) * sizeof(T));
		m_size -= numToRemove;
		decreaseCapacity();
	}
}

template <typename T> inline
void bdFastArray<T>::removeAll(const T value)
{
	for(bdUInt i = 0; i < m_size; i++)
	{
		if ( value == m_data[i] )
		{
			removeAt(i);

			// Need to recheck the same slot.
			i--;
		}
	}
}

template <typename T> inline
void bdFastArray<T>::removeAllKeepOrder(const T value)
{
	for(bdUInt i = 0; i < m_size; i++)
	{
		if ( value == m_data[i] )
		{
			removeSection(i,i+1);

			// Need to recheck the same slot.
			i--;
		}
	}
}

template <typename T> inline
void bdFastArray<T>::clear()
{
	bdDeallocate<T>(m_data);
	m_data = BD_NULL;
	m_size = 0;
	m_capacity = 0;
}

template <typename T> inline
const T* bdFastArray<T>::begin() const
{
	return m_data;
}

template <typename T> inline
T* bdFastArray<T>::begin()
{
	return m_data;
}

template <typename T> inline
const T* bdFastArray<T>::end() const
{
	return m_data + m_size;
}

template <typename T> inline
T* bdFastArray<T>::end()
{
	return m_data + m_size;
}

template <typename T> inline
void bdFastArray<T>::copyArrayArray(T *dest,
									const T *src,
									const bdUInt n) const
{
	if(n > 0)
	{
		bdMemcpy(dest, src, n * sizeof(T));
	}

//  equivalent to
//	for (bdUInt i = 0; i < n; i++)
//	{
//		dest[i] = src[i];
//	}
}

template <typename T> inline
void bdFastArray<T>::copyArrayObject(T *dest,
									 const T &src,
									 const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		bdMemcpy(dest + i, &src, sizeof(T));
	}

//  equivalent to
//	for (bdUInt i = 0; i < n; i++)
//	{
//		dest[i] = T;
//	}

}

template <typename T> inline
void bdFastArray<T>::copyObjectObject(T* dest,
									  const T &src) const
{
	bdMemcpy(dest, &src, sizeof(T));

//  equivalent to
//  *dest = src;
}

template <typename T> inline
T* bdFastArray<T>::uninitializedCopy(const bdFastArray<T> &a) const
{
	T* data = BD_NULL;
	if(a.m_capacity > 0)
	{
		data = bdAllocate<T>(a.m_capacity);
		copyArrayArray(data, a.m_data, a.m_size);
	}
	return data;
}
