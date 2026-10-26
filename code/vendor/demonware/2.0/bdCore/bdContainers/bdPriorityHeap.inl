// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM        : ALL
// PRODUCT        : BIT_DEMON
// VISIBILITY    : PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


// public:

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdPriorityHeap<T, LESS_THAN_FUNCTOR>::bdPriorityHeap(bdUInt capacity) 
: m_size(0), 
  m_capacity(capacity)
{
    // the array needs to be 1 bigger than the capacity
    // because we do inplace array shuffling where 
    // we need an extra element for temporary values
	m_array.ensureCapacity(capacity+1);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
void bdPriorityHeap<T, LESS_THAN_FUNCTOR>::enqueue(const T &item)
{
	bdUInt hole = m_size++;

	for ( ; hole > 0 && m_lessThan(item, m_array[hole/2]) ; hole /= 2)
	{
		if(hole >= m_array.getSize())
		{
			const T tmp = m_array[hole/2];
			m_array.setGrow(hole, tmp);
		}
		else
		{
			m_array[hole] = m_array[hole/2];
		}
	}
	
	m_array.setGrow(hole, item);
}


template <typename T, typename LESS_THAN_FUNCTOR> inline
void bdPriorityHeap<T, LESS_THAN_FUNCTOR>::dequeue()
{
	if(isEmpty())
	{
		bdBool fail = false;
		BD_ASSERT(fail, "bdPriorityHeap::dequeue(), the heap is empty so dequeueing will cause a crash");
		return;
	}

	m_array[0] = m_array[m_size-1];
	m_size--;
	m_array.popBack();

	siftDown(0);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
T & bdPriorityHeap<T, LESS_THAN_FUNCTOR>::peek()
{
	if(isEmpty())
	{
		bdBool fail = false;
		BD_ASSERT(fail, "bdPriorityHeap::peek(), the heap is empty so peeking will cause a crash");
	}

	return m_array[0];
}


template <typename T, typename LESS_THAN_FUNCTOR> inline
const T & bdPriorityHeap<T, LESS_THAN_FUNCTOR>::peek() const
{
	if(isEmpty())
	{
		BD_ASSERT(false, "bdPriorityHeap::peek(), the heap is empty so peeking will cause a crash");
	}

	return m_array[0];
}



template <typename T, typename LESS_THAN_FUNCTOR> inline
bdUInt bdPriorityHeap<T, LESS_THAN_FUNCTOR>::getSize() const
{
    return m_size;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdBool bdPriorityHeap<T, LESS_THAN_FUNCTOR>::isEmpty() const
{
	return m_size <= 0;
}

// protected:
template <typename T, typename LESS_THAN_FUNCTOR> inline
void bdPriorityHeap<T, LESS_THAN_FUNCTOR>::siftDown( bdUInt hole )
{
	if(isEmpty())
	{
		return;
	}

	bdUInt child;
	T tmp = m_array[hole];

	const bdUInt size = m_size;

	for (; hole * 2 < size; hole = child)
	{
		child = hole * 2;

		if (child < size-1 && m_lessThan(m_array[child+1], m_array[child]))
		{
			child++;
		}
		
		if (m_lessThan(m_array[child], tmp))
		{
			m_array[hole] = m_array[child];
		}
		else 
		{
			break;
		}
	}

	m_array[hole] = tmp;
}
