// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A template flexible object array class.

#ifndef BD_FAST_ARRAY_H
#define BD_FAST_ARRAY_H

#include <bdCore/bdMemory/bdMemory.h>

/// A flexible array of \c T objects.
/// This array will automatically grow as objects are pushed into the array,
/// and shrink as objects are popped out.
/// The class does not make any attempt to call the constructors or destructors
/// of \c T. If you require such functionality use bdArray.
template <typename T>
class bdFastArray
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		//
		// Construction and destruction.
		//

		/// Specific capacity constructor.
		/// Create a \c bdFastArray with space to store \a capacity elements.
		/// The individual array elements will not be initialized, so the
		/// array size will be zero.
		/// \param capacity[in]	The desired capacity.
		inline bdFastArray(const bdUInt capacity = 0);

		/// Specific capacity and value constructor.
		/// Create a \c bdFastArray with space to store \a capacity elements.
		/// Each element will be initialized to \a value, the array size
		/// will be equal to capacity.
		/// \param capacity[in	The desired capacity.
		/// \param value[in]	The value with which all array element will be
		///						initialized.
		inline bdFastArray(const bdUInt capacity,
						   const T &value);

		/// Copy constructor.
		/// Initializes the contents of \a this to be an exact copy of \a a.
		/// \param a[in]	The array to copy.
		inline bdFastArray(const bdFastArray<T> &a);

		/// Destructor.
		/// Deallocates all the memory associated with the array.
		inline ~bdFastArray();

		//
		// Operators (excluding new and delete).
		//

		/// Assignment operator.
		/// Initializes the contents of \a this to be an exact copy of \a a.
		/// The capacity of \a this is only increased if necessary.
		/// \param a[in]	The array to copy.
		inline void operator= (const bdFastArray<T> &a);

		/// Get read and write access to the \a i'th element.
		/// No checks are performed to ensure \a i is less that the array size.
		/// If you require range checking use \c get(), however
		/// \c operator[] is more efficient as it returns a reference to the
		/// array element rather than a copy.
		/// \param i[in]	The index of the desired element.
		/// \return			A reference to the \a i'th element in the array.
		inline T& operator[] (const bdUInt i);

		/// Get read only access to the \a i'th element.
		/// No checks are performed to ensure \a i is less that the array size.
		/// If you require range checking use \c get(), however
		/// \c operator[] is more efficient as it returns a reference to the
		/// array element rather than a copy.
		/// \param i[in]	The index of the desired element.
		/// \return			A reference to the \a i'th element in the array.
		inline const T& operator[] (const bdUInt i) const;

		//
		// Methods.
		//

		/// Check that there are elements in the array.
		/// \return
		/// - True, if the array is empty (\a m_size == 0)
		/// - False, if the array is not empty
		inline bdBool isEmpty() const;

		/// Get the number of elements in the array.
		/// \return The number of elements currently in the array.
		inline bdUInt getSize() const;

		/// Get the capacity of the array.
		/// This is the number of \c T's that can be stored in the array before
		/// a resize is required.
		/// \return	The current capacity of the array.
		inline bdUInt getCapacity() const;

		/// Increase the array storage.
		/// Allocates new storage for at least an extra \a increase elements.
		/// More memory than is indicated may be allocated to avoid subsequent
		/// reallocations.
		/// \param increase[in]	The desired increase in capacity.
		inline void increaseCapacity(const bdUInt increase);

		/// Attempts to decrease the array storage.
		/// Decreases the array's storage capacity only if the current capacity
		/// is greater than 4 times the array size. The capacity will be
		/// decreased by a maximum of \a decrease elements or half the array
		/// capacity, whichever is greatest. In the event that the requested
		/// decrease would necessitate the destruction of some array elements,
		/// only the spare capacity of the array will be deallocated.
		/// \param decrease[in]	The desired capacity decrease.
		inline void decreaseCapacity(const bdUInt decrease = 0);

		/// Increase the array storage if required.
		/// Ensure that the array has capacity to store \a capacity elements.
		/// \param capacity[in]	The desired capacity.
		inline void ensureCapacity(const bdUInt capacity);

		/// Clear the array and free all allocated memory.
		inline void clear();

		/// Get read-only access to the \a i'th element.
		/// If \a i is within the valid range of the array, the \a i'th element
		/// is copied into \a value, otherwise \a value is left unchanged.
		/// If you want to avoid copying, use \c operator[] and check the range
		/// yourself.
		/// \param i[in]		The index of the desired element.
		/// \param value[out]	Will be set to be equal to the \a i'th element,
		///						if it exists.
		/// \return
		/// - True, if the \a i'th element was copied into \a value
		/// - False, if no element was copied
		inline bdBool get(const bdUInt i,
						  T &value) const;

		/// Copy a section of the array.
		/// Copies from \a m_array[begin] (inclusive) to \a m_array[end]
		/// (exclusive) into the array pointed to by \a value.
		/// If \a begin is out of range, greater than \a end or \a end is
		/// greater than the array size, nothing is copied.
		/// You must ensure that the array pointed to by \a value is large
		/// enough to store \a begin - \a end elements of type \c T.
		/// \param begin[in]	The index of the first element to be copied.
		/// \param end[in]		The index of the last element to be copied plus
		///						one. Must be greater than \a begin.
		/// \param value[in]	A pointer to an array of \c Ts into which the
		///						desired section will be copied.
		inline void getSection(const bdUInt begin,
							   const bdUInt end,
							   T *value) const;

		/// Set the \a i'th element.
		/// If \a i is within the valid range of the array the \a i'th element
		/// is set to \a value, otherwise it is left unchanged.
		/// \param i[in]		The index of the element to be set.
		/// \param value[in]	The value to which the \a i'th element should
		///						be set.
		/// \return
		/// - True, if the \a i'th element has been set
		/// - False, if no element has been set
		inline bdBool set(const bdUInt i,
						  const T &value);

		/// Set the \a i'th element.
		/// If the array is too small it grows to accommodate the new element.
		/// Any new elements created will not be initialized.
		/// If the \a i'th element is in range it will be set to
		/// \a value with \c operator=.
		/// \param i[in]		The index of the element to set.
		/// \param value[in]	The value to which the \a i'th element should
		///						be set.
		inline void setGrow(const bdUInt i,
							const T &value);

		/// Add an element to end of the array.
		/// Appends an element to the end of the array, increasing the size
		/// by 1.
		/// \param value[in]	The element to add to the array.
		inline void pushBack(const T &value);

		/// Add \a n elements to end of the array.
		/// Appends the first \a n elements from the array pointed to by
		/// \a value to the array.\a value must contain at least \a n elements.
		/// \param value[in]	A pointer to an array of elements to be added
		///						to the array.
		/// \param n[in]		The number of elements to be added to the
		///						array.
		inline void pushBack(const T *value, const bdUInt n);

		/// Remove the last element from the array.
		/// Attempts to remove and destroy the last element in the array.
		/// If the element exists the array size is decreased by one.
		inline void popBack();

		/// Remove the last \a n elements from the array.
		/// Attempts to remove and destroy the last \a n elements from the
		/// array. The array size is decreased by the number of elements that
		/// have been successfully removed.
		/// \param n[in]	The number of elements to remove.
		inline void popBack(const bdUInt n);

		/// Test if there is an \a i'th element in the array.
		/// \param i[in]	The index of the element to check.
		/// \return
		/// - True, if the element exists
		/// - False, if the element does not exist
		inline bdBool rangeCheck(const bdUInt i) const;

		/// Search the array.
		/// Finds the index of the first occurrence of \a value in the array.
		/// If found, the index of the element is copied into \a i.
		/// \warning This method is O(n).
		/// \param value[in]	The element to search the array for.
		/// \param i[out]		Will be set to the index of the first element
		///						matching \a value.
		/// \return
		/// - True, if \a value was found
		/// - False, if \a value was not found
		inline bdBool findFirst(const T &value,
								bdUInt& i) const;

		/// Remove the \a i'th element from the array.
		/// If \a i is in range, the last element of the array is copied into
		/// the \a i'th position.
		/// \warning This function re-orders the array. Use \c removeSection()
		/// to avoid reordering.
		/// \param i[in]	The index of the element to remove.
		inline void removeAt(const bdUInt i);

		/// Remove a section of the array.
		/// Removes from \a m_array[begin] (inclusive) to \a m_array[end]
		/// (exclusive). This method copies the remaining elements and therefore does
		/// not re-order the array.
		/// If \a begin is out of range, greater than \a end or \a end is
		/// greater than the array size nothing is removed.
		/// \note This function is O(1), rather than the naive O(n) solution.
		/// \param begin[in]	The index of the first element to be removed.
		/// \param end[in]		The index of the last element to be removed
		///						plus one. Must be greater than \a begin.
		void removeSection(const bdUInt begin,
						   const bdUInt end);

		/// Remove all occurrences of \a value from the array.
		/// If any occurrences of \a value are found they are removed using
		/// \c removeAt(), so the array may become re-ordered.
		/// \warning This method is O(n).
		/// \param value[in]	The value to match array element against.
		inline void removeAll(const T value);

		/// Remove all occurrences of \a value from the array.
		/// If any occurrences of \a value are found they are removed using
		/// \c removeSection(). Hence the array will not be re-ordered.
		/// \warning This method is O(n).
		/// \param value[in]	The value to match array element against.
		inline void removeAllKeepOrder(const T value);

		/// Get a const pointer to the first array element.
		/// The pointer returned can be used to traverse the array.
		/// No checks are performed to ensure that the first element exists.
		/// \return	A const pointer to the head of the array.
		inline const T* begin() const;

		/// Get a pointer to the first array element.
		/// The pointer returned can be used to traverse the array.
		/// No checks are performed to ensure that the first element exists.
		/// \return	A pointer to the head of the array.
		inline T* begin();

		/// Get a const pointer to the tail of the array.
		/// The pointer returned can be used to traverse the array, and points
		/// to the one past the end of the array.
		/// No checks are performed to ensure that the last element exists.
		/// \return	A const pointer to the tail of the array.
		inline const T* end() const;

		/// Get a pointer to the tail of the array.
		/// The pointer returned can be used to traverse the array, and points
		/// to the one past the end of the array.
		/// No checks are performed to ensure that the last element exists.
		/// \return	A pointer to the tail of the array.
		inline T* end();


	protected:

		/// Copy \a n elements of type \c T from \a src to \a dest.
		/// \param dest[in]	Pointer to the destination array into which the
		///					elements should be copied.
		/// \param src[in]	Pointer to the source array from which the
		///					elements should be copied.
		/// \param n[in]	The number of elements to be copied.
		inline void copyArrayArray(T *dest,
								   const T *src,
								   const bdUInt n) const;

		/// Copy \a n copies of \a src to \a dest.
		/// \param dest[in]	Pointer to the destination array into which the
		///					elements should be copied.
		/// \param src[in]	The object to be copied into the array.
		/// \param n[in]	The number of times \a src is to be copied
		///					into \a dest.
		inline void copyArrayObject(T *dest,
									const T &src,
									const bdUInt n) const;

		/// Copy \a src into \a dest.
		/// \param dest[in]	Pointer to the destination memory into which the
		///					object should be copied.
		/// \param src[in]	The object to be copied.
		inline void copyObjectObject(T *dest,
									 const T &src) const;

		/// Copy \a a into a buffer.
		/// Allocates enough memory to copy the contents of \a a and copies
		/// the elements of \a a into the buffer.
		/// \param a[in]	The array to copy
		/// \return			A pointer to the allocated and copied data.
		inline T* uninitializedCopy(const bdFastArray<T> &a) const;

	protected:

		/// The array data.
		T* m_data;

		/// The current capacity of the array.
		/// This is the number of \c T's that can be stored in the array before
		/// a resize is required.
		bdUInt m_capacity;

		/// The current number of elements in the array.
		bdUInt m_size;
};

#include <bdCore/bdContainers/bdFastArray.inl>

#endif // BD_FAST_ARRAY_H
