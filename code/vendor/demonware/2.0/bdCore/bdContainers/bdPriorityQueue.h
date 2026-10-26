// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


/// Purpose: A priority queue with a small number of pre-defined priorities.

#ifndef BD_PRIORITY_QUEUE_H
#define BD_PRIORITY_QUEUE_H

#include <bdCore/bdMemory/bdMemory.h>

#include <bdCore/bdContainers/bdQueue.h>

/// A priority queue with a small number of pre-defined priorities.
/// It is implemented with a number of instances of bdQueue.
/// Enqueuing and dequeuing is O(1) in all cases. The priorities are defined in
/// bdPriority.
template <typename T>
class bdPriorityQueue 
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Insert an item with a specific priority into the queue.
		/// \param priority[in]	The item's priority.
		/// \param item[in]		The item to insert.
		inline void enqueue(const bdPriority priority, 
							const T &item);

		/// Remove the item with the highest priority from the queue.
		inline void dequeue();
		
		/// Get access to the highest priority item in the queue.
		/// \return	The item with the highest priority.
		inline T& peek();

		/// Get const access to the highest priority item in the queue.
		/// \return	The item with the highest priority.
		inline const T& peek() const;

		/// Get the number of items with a specific priority in the queue.
		/// \param priority[in]	The priority of the items to check for.
		/// \return	The number of items of the given bdPriority.
		inline bdUInt queueSize(const bdPriority priority);

		/// Get the total number of items in the queue.
		/// \return The number of items in the queue.
		inline bdUInt getSize() const;

		/// Find out if the queue is empty.
		/// \return 
		/// -	True, if getSize() == 0
		/// -	False, if the queue is not empty
		inline bdBool isEmpty() const;

	protected:

		/// An array of queues that store items of specific priorities.
		bdQueue<T>	m_queues[BD_NUM_PRIORITIES];
		
};

#include <bdCore/bdContainers/bdPriorityQueue.inl>

#endif // BD_PRIORITY_QUEUE_H
