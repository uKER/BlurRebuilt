// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

/// Purpose: A Priority heap class (allows infinite priorities).


#ifndef BD_STATIC_PRIORITY_QUEUE_H
#define BD_STATIC_PRIORITY_QUEUE_H

#include <bdCore/bdMemory/bdMemory.h>

#include <bdCore/bdContainers/bdArray.h>
#include <bdCore/bdUtilities/bdComparisonFunctors.h>

/// A Priority heap class (allows infinite priorities). 
/// Data is stored in a bdArray which will grow when needed. Enqueuing and 
/// dequeuing are (worst case) O(Log(n)).
/// If you need a priority class with a finite number of priorities consider
/// using bdPriorityQueue, which has O(1) enqueuing and dequeuing and grows 
/// better (it uses a number of linked lists instead of an array).
template <typename T, typename LESS_THAN_FUNCTOR = bdLessThan<T> >
class bdPriorityHeap
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a new heap.
		/// \param	capacity[in]	The initial capacity. Default: 128
		inline bdPriorityHeap(bdUInt capacity = 128);

		/// Add an item to the heap. The priority is implicit in the item. The 
		/// templated less than functor will be used to compare items.
		/// \param item[in]	Item to be added to the heap.
		inline void enqueue(const T &item);

		/// Remove the item with the lowest priority from the heap.
		inline void dequeue();

		/// Get access to the lowest priority item in the heap.
		/// \return	The item with the lowest priority.
		inline T& peek();

		/// Get const access to the lowest priority item in the heap.
		/// \return	The item with the lowest priority.
		inline const T& peek() const;

		/// Get the number of elements currently in the heap.
		/// \return	The size of the heap.
		inline bdUInt getSize() const;

		/// Find out if the heap is empty.
		/// \return 
		/// -	True, if getSize() == 0
		/// -	False, if the heap is not empty
		inline bdBool isEmpty() const;

	protected:

		/// The functor used to compare the priorities of items in the heap.
		LESS_THAN_FUNCTOR m_lessThan;

		/// Sift (ie move) the 'T' at 'hole' down the heap.
		/// This is an internal helper function.
		inline void siftDown( bdUInt hole );

		/// Array to store the contents of the heap.
		bdArray<T> m_array;

		/// The number of items in the heap.
		bdUInt m_size;

		/// The current capacity of the heap.
		bdUInt m_capacity;
};

#include <bdCore/bdContainers/bdPriorityHeap.inl>

#endif // BD_STATIC_PRIORITY_QUEUE_H
