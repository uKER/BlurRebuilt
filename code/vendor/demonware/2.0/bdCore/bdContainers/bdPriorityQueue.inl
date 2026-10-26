// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS



template <typename T> inline 
void bdPriorityQueue<T>::enqueue(const bdPriority priority, 
								 const T &item)
{
	m_queues[priority].enqueue(item);
}

template <typename T> inline 
void bdPriorityQueue<T>::dequeue()
{
	BD_ASSERT(getSize() > 0, "bdPriorityQueue::dequeue, queue empty, can't dequeue.");

	for(bdInt i = BD_NUM_PRIORITIES-1; i >= 0; i--)
	{					   
		if(!m_queues[i].isEmpty())
		{
			m_queues[i].dequeue();
			return;
		}
	}
}

template <typename T> inline 
T& bdPriorityQueue<T>::peek() 
{
	BD_ASSERT(getSize() > 0, "bdPriorityQueue::peak, queue empty, can't peak.");

	for(bdInt i = BD_NUM_PRIORITIES-1; i >= 0; i--)
	{					   
		if(!m_queues[i].isEmpty())
		{
			return m_queues[i].peek();
		}		
	}

	// just to keep the compiler happy
	return m_queues[0].peek();	
}

template <typename T> inline 
const T& bdPriorityQueue<T>::peek() const 
{
	BD_ASSERT(getSize() > 0, "bdQueue::dequeue, queue empty, can't peek.");

	for(bdInt i = BD_NUM_PRIORITIES-1; i >= 0; i--)
	{					   
		if(!m_queues[i].isEmpty())
		{
			return m_queues[i].peek();
		}		
	}

	// just to keep the compiler happy
	return m_queues[0].peek();	
}

template <typename T> inline 
bdUInt bdPriorityQueue<T>::queueSize(const bdPriority priority)
{
	return m_queues[priority].getSize();
}

template <typename T> inline 
bdUInt bdPriorityQueue<T>::getSize() const
{
	bdUInt size = 0;

	for(bdInt i = BD_NUM_PRIORITIES-1; i >= 0; i--)
	{
		size += m_queues[i].getSize();
	}

	return size;
}

template <typename T> inline 
bdBool bdPriorityQueue<T>::isEmpty() const
{
	bdBool empty = true;

	for(bdInt i = BD_NUM_PRIORITIES-1; i >= 0; i--)
	{
		if(!m_queues[i].isEmpty())
		{
			return false;
		}
	}

	return empty;
}

