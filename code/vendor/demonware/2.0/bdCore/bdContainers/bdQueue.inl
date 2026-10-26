// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS



template <typename T> inline
void bdQueue<T>::enqueue(const T &item)
{	
	m_list.addTail(item);
}

template <typename T> inline
void bdQueue<T>::dequeue()
{
	BD_ASSERT(getSize() > 0, "bdQueue::dequeue, queue empty, can't dequeue.");	
	m_list.removeHead();	
}

template <typename T> inline
T& bdQueue<T>::peek()
{
	BD_ASSERT(getSize() > 0, "bdQueue::dequeue, queue empty, can't peek.");	
	return m_list.getHead();
}

template <typename T> inline
const T& bdQueue<T>::peek() const
{
	BD_ASSERT(getSize() > 0, "bdQueue::dequeue, queue empty, can't peek.");
	return m_list.getHead();
}

template <typename T> inline
bdUInt bdQueue<T>::getSize() const
{
	return m_list.getSize();
}

template <typename T> inline
bdBool bdQueue<T>::isEmpty() const
{
	return m_list.isEmpty();
}

