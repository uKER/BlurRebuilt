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
bdArray<T>::bdArray(const bdUInt capacity)
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
bdArray<T>::bdArray(const bdUInt capacity,
					const T& value)
: m_data(BD_NULL),
  m_capacity(capacity),
  m_size(capacity)
{	
	if(m_capacity > 0)
	{
		m_data = bdAllocate<T>(m_capacity);
		copyConstructArrayObject(m_data, value, m_capacity);
	}
}

template <typename T> inline 
bdArray<T>::bdArray(const bdArray<T> &a)
: m_capacity(a.getCapacity()),
  m_size(a.getSize())
{	
	m_data = uninitializedCopy(a);
}

template <typename T> inline 
bdArray<T>::~bdArray()
{
	clear();
}

template <typename T> inline
void bdArray<T>::operator= (const bdArray<T> &a)
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
		else if (newSize > m_size)
		{
			// Have enough capacity, but am too small so need to
			// construct the extra.
			
			// copy over the constructed objects
			for (bdUInt i=0; i < m_size; i++)
			{
				m_data[i] = a[i];
			}

			// construct the remaining objects			
			copyConstructArrayArray(m_data + m_size, a.m_data + m_size, newSize - m_size);

			m_size = newSize;
		}
		else
		{
			// Have enough capacity, but am too big so need to
			// destroy the extra.
			for (bdUInt i=0; i < newSize; i++)
			{
				m_data[i] = a[i];
			}
		
			destruct(m_data + newSize, m_size - newSize);

			m_size = newSize;
			decreaseCapacity();
		}

	}
}

template <typename T> inline
T& bdArray<T>::operator[] (const bdUInt i)
{
	BD_ASSERT(rangeCheck(i), "bdArray<T>::operator[], rangecheck failed");

	return m_data[i];
}

template <typename T> inline 
const T& bdArray<T>::operator[] (const bdUInt i) const
{
	BD_ASSERT(rangeCheck(i), "bdArray<T>::operator[], rangecheck failed");

	return m_data[i];
}

template <typename T> inline 
bdBool bdArray<T>::isEmpty() const
{
	return m_size == 0;
}

template <typename T> inline 
bdUInt bdArray<T>::getSize() const
{	
	return m_size;
}

template <typename T> inline 
bdUInt bdArray<T>::getCapacity() const
{
	return m_capacity;
}

template <typename T> inline 
void bdArray<T>::increaseCapacity(const bdUInt increase)
{
	// Note that the size does not change, just the capacity.
	const bdUInt newCapacity = m_capacity + ((increase>m_capacity)?increase:m_capacity);

	T* newData = BD_NULL;
	if(newCapacity > 0)
	{
        newData = bdAllocate<T>(newCapacity);
		copyConstructArrayArray(newData, m_data, m_size);
	}

	destruct(m_data, m_size);
	bdDeallocate<T>(m_data);

	/*
	// This is the old fast version, unfortunately if classes contains 
	// pointers that reference themselves (eg bdSet) the object in the
	// array will become invalid.
	T* newData = bdAllocate<T>(newCapacity);	

	// copy the old data
	bdMemcpy(newData, m_data, m_size * sizeof(T));

	// delete the old data, no destructors are called
	bdDeallocate<T>(m_data);
	*/

	m_data = newData;
	m_capacity = newCapacity;
}

template <typename T> inline 
void bdArray<T>::decreaseCapacity(const bdUInt decrease)
{
	if (m_capacity > (m_size << 2))
	{
		const bdUInt spare = m_capacity - m_size;
		const bdUInt possibleDecrease = (decrease>spare)?spare:decrease;

		m_capacity -= (possibleDecrease>(m_capacity >> 1))?possibleDecrease:(m_capacity >> 1);

		/*
		// This is the old fast version, unfortunately if classes contains 
		// pointers that reference themselves (eg bdSet) the object in the
		// array will become invalid.
		m_data = bdReallocate<T>(m_data, m_capacity);	
		*/

		T* newData = BD_NULL;
		if(m_capacity > 0)
		{
			newData = bdAllocate<T>(m_capacity);
			copyConstructArrayArray(newData, m_data, m_size);
		}

		destruct(m_data, m_size);
		bdDeallocate<T>(m_data);

		m_data = newData;
	}
}

template <typename T> inline 
void bdArray<T>::ensureCapacity(const bdUInt capacity)
{
	if (m_capacity < capacity)
	{
		const bdUInt increase = capacity - m_capacity;
		increaseCapacity(increase);
	}
}

template <typename T> inline 
void bdArray<T>::clear()
{
	// call the destructors of the initialised elements
	destruct(m_data, m_size);
	bdDeallocate<T>(m_data);
	m_data = BD_NULL;
	m_size = 0;
	m_capacity = 0;
}


template <typename T> inline 
bdBool bdArray<T>::get(const bdUInt i, 
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
void bdArray<T>::getSection(const bdUInt begin,
							const bdUInt end,
							T *value) const
{
	bdBool inRange = rangeCheck(begin);
	inRange = inRange && (end <= m_size) && (begin < end);

	if(inRange)
	{
		const bdUInt numToCopy = end - begin;

		for(bdUInt i = 0; i < numToCopy; i++)
		{
			value[i] = m_data[begin + i];
		}
	}
}

template <typename T> inline 
bdBool bdArray<T>::set(const bdUInt i, 
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
void bdArray<T>::setGrow(const bdUInt i,
						 const T &value)
{
	const bdBool inRange = rangeCheck(i);

	if(inRange)
	{
		m_data[i] = value;
	}
	else
	{
		ensureCapacity(i+1);

		defaultConstructArray(m_data + m_size, i - m_size);
		copyConstructObjectObject(m_data + i, value);
		m_size = i+1;		
	}
}

template <typename T> inline 
void bdArray<T>::pushBack(const T &value)
{
	if (m_size == m_capacity)
	{
		increaseCapacity(1);
	}

	copyConstructObjectObject(m_data + m_size, value);
	m_size++;
}

template <typename T> inline 
void bdArray<T>::pushBack(const T *value, 
						  const bdUInt n)
{
	const bdUInt spare = m_capacity - m_size;
	if (n > spare)
	{
		increaseCapacity(n - spare);
	}

	copyConstructArrayArray(m_data + m_size, value, n);
	m_size += n;
}

template <typename T> inline 
void bdArray<T>::popBack()
{
	if (m_size > 0)
	{
		m_size--;
		//m_data[m_size].~T();
		destruct(m_data + m_size, 1);
		decreaseCapacity();
	}
}

template <typename T> inline 
void bdArray<T>::popBack(const bdUInt n)
{
	if (n < m_size)
	{
		destruct(m_data + m_size - n, n);
		m_size -= n;
	}
	else
	{
		destruct(m_data, m_size);
		m_size = 0;		
	}

	decreaseCapacity();
}

template <typename T> inline 
bdBool bdArray<T>::rangeCheck(const bdUInt i) const
{
	return i < m_size;
}

template <typename T> inline
bdBool bdArray<T>::findFirst(const T &value,
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
void bdArray<T>::removeAt(const bdUInt i)
{
	const bdBool inRange = rangeCheck(i);
	if (inRange)
	{
		m_data[i] = m_data[m_size-1];
		popBack();
	}
}

template <typename T> inline
void bdArray<T>::removeSection(const bdUInt begin,
							   const bdUInt end)
{
	bdBool inRange = rangeCheck(begin);
	inRange = inRange && (end <= m_size) && (begin < end);

	if(inRange)
	{
		const bdUInt numToDestroy = end - begin;	

		moveArrayArray(m_data + begin, m_data + end, m_size - end);
		popBack(numToDestroy);

		//destruct(m_data + begin, numToDestroy);
		//bdMemmove(m_data + begin, m_data + end, (m_size - end) * sizeof(T));
		//m_size -= numToDestroy;
		//decreaseCapacity();
	}
}

template <typename T> inline
void bdArray<T>::removeAll(const T value)
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
void bdArray<T>::removeAllKeepOrder(const T value)
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
const T* bdArray<T>::begin() const
{
	return m_data;
}

template <typename T> inline 
T* bdArray<T>::begin()
{
	return m_data;
}

template <typename T> inline 
const T* bdArray<T>::end() const
{
	return m_data + m_size;
}

template <typename T> inline 
T* bdArray<T>::end()
{
	return m_data + m_size;
}

template <typename T> inline 
void bdArray<T>::moveArrayArray(T *dest,
								const T *src,
								const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		dest[i] = src[i];
//		new (dest+i) T(*(src+i));
//		src[i].~T();
	}
}

template <typename T> inline 
void bdArray<T>::copyConstructArrayArray(T *dest,
										 const T *src,
										 const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		T *const pDest = dest+i;
		const T *const pSrc = src+i;
		new (pDest) T(*pSrc);	
	}
}

template <typename T> inline 
void bdArray<T>::copyConstructArrayObject(T *dest,
										  const T &src,
										  const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		new (dest+i) T(src);
	}
}

template <typename T> inline 
void bdArray<T>::defaultConstructArray(T *dest,
									   const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		new (dest+i) T;
	}
}

template <typename T> inline 
void bdArray<T>::copyConstructObjectObject(T* dest,						   
										   const T &src) const
{
	new (dest) T(src);
}

template <typename T> inline 
void bdArray<T>::destruct(T* src,
						 const bdUInt n) const
{
	for (bdUInt i = 0; i < n; i++)
	{
		src[i].~T();
	}
}

template <typename T> inline
T* bdArray<T>::uninitializedCopy(const bdArray<T> &a) const
{
	T* data = BD_NULL;
	if(a.m_capacity > 0)
	{
		data = bdAllocate<T>(a.m_capacity);
		copyConstructArrayArray(data, a.m_data, a.m_size);
	}	
	return data;
}










