// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A templated FIFO queue class.

#ifndef BD_QUEUE_H
#define BD_QUEUE_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdLinkedList.h>

/// A templated FIFO queue class. 
template <typename T>
class bdQueue
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		/// Add an item to the end of the queue.
		/// \param item[in]	The item to add to the queue.
		inline void enqueue(const T &item);

		/// Remove an item from the front of the queue.
		inline void dequeue();

		/// Get access to the the item at the front of the queue.
		/// \return	A reference to the item at the front of the queue.
		inline T& peek();

		/// Get const access to the the item at the front of the queue.
		/// \return	A const reference to the item at the front of the queue.
		inline const T& peek() const;

		/// Get the number of items currently in the queue.
		/// \return	The current size of the queue.
		inline bdUInt getSize() const;

		/// Find out if the queue is empty.
		/// \return 
		/// -	True, if getSize() == 0
		/// -	False, if the queue is not empty
		inline bdBool isEmpty() const;
		
	protected:
	
		/// A linked list, which contains the queue's contents.
		bdLinkedList<T> m_list;
};

#include <bdCore/bdContainers/bdQueue.inl>

#endif // BD_QUEUE_H
