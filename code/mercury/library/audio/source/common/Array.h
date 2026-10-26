#ifndef _COREARRAY_H_
#define _COREARRAY_H_

#include "Util.h"

namespace Audio
{
	namespace Core
	{
//------------------------------------------------------------------------------
//	Array
//==============================================================================
		template <class Type>
		class Array
		{
		public:
			Array() : m_pData(0), m_uAllocated(0), m_uUsed(0), m_bFreeWhenDestroyed(true), m_bIsSorted(true) {}

			//! Constructs an array and allocates an initial chunk of memory.
			//! \param uStartCount: Amount of elements to allocate.
			Array(U32 uStartCount) : m_pData(0), m_uUsed(0), m_uAllocated(0), m_bFreeWhenDestroyed(true), m_bIsSorted(true)
			{
				Reallocate(uStartCount);
			}

			//! Copy constructor
			Array(const Array<Type>& tOther) : m_pData(0)
			{
				*this = tOther;
			}

			//! Destructor. Frees allocated memory, if SetFreeWhenDestroyed
			//! was not set to false by the user before.
			~Array()
			{
				if (m_bFreeWhenDestroyed)
					delete [] m_pData;
			}

			//! Reallocates the Array, make it bigger or smaller.
			//! \param uNewSize: New uSize of Array.
			void Reallocate(U32 uNewSize)
			{
				Type* ptOldData = m_pData;

				m_pData = new Type[uNewSize];
				m_uAllocated = uNewSize;
				
				S32 end = m_uUsed < uNewSize ? m_uUsed : uNewSize;
				for (S32 i = 0; i < end; ++i)
					m_pData[i] = ptOldData[i];

				if (m_uAllocated < m_uUsed)
					m_uUsed = m_uAllocated;
				
				delete [] ptOldData;
			}

			//! Adds an element at back of array. If the Array is to small to 
			//! add this new element, the array is made bigger.
			//! \param tElement: Element to add at the back of the array.
			void PushBack(const Type& tElement)
			{
				if (m_uUsed + 1 > m_uAllocated)
				{
					// Reallocate(m_uUsed * 2 +1);
					// this doesn't work if the element is in the same array. So
					// we'll copy the element first to be sure we'll get no data
					// corruption

					Type e;
					e = tElement;					// copy element
					Reallocate(m_uUsed * 2 + 1);	// increase data block
					m_pData[m_uUsed++] = e;			// push back
					m_bIsSorted = false; 
					return;
				}

				m_pData[m_uUsed++] = tElement;
				m_bIsSorted = false;
			}

			//! Adds an element at the front of the array. If the array is to small to 
			//! add this new element, the array is made bigger. Please note that this
			//! is slow, because the whole array needs to be copied for this.
			//! \param tElement: Element to add at the back of the array.
			void PushFront(const Type& tElement)
			{
				if (m_uUsed + 1 > m_uAllocated)
					Reallocate(m_uUsed * 2 + 1);

				for (int i = (int)m_uUsed; i > 0; --i)
					m_pData[i] = m_pData[i - 1];

				m_pData[0] = tElement;
				m_bIsSorted = false;
				++m_uUsed;
			}

			//! Insert item into array at specified position. Please use this
			//! only if you know what you are doing (possible performance loss). 
			//! The preferred method of adding elements should be PushBack().
			//! \param tElement: Element to be inserted
			//! \param uIndex: Where position to Insert the new element.
			void Insert(const Type& tElement, U32 uIndex=0) 
			{
				Assert(uIndex <= m_uUsed);

				if (m_uUsed + 1 > m_uAllocated)
					Reallocate(m_uUsed * 2 +1);

				for (U32 i = m_uUsed++; i > uIndex; i--) 
					m_pData[i] = m_pData[i - 1];

				m_pData[uIndex] = tElement;
				m_bIsSorted = false;
			}

			//! Clears the array and deletes all allocated memory.
			void Clear()
			{
				delete [] m_pData;
				m_pData = 0;
				m_uUsed = 0;
				m_uAllocated = 0;
				m_bIsSorted = true;
			}

			//! Sets pointer to new array, using this as new workspace.
			//! \param ptArray: Pointer to new array of elements.
			//! \param uSize: Size of the new array.
			void SetPointer(Type* ptArray, U32 uSize)
			{
				delete [] m_pData;
				m_pData = ptArray;
				m_uAllocated = uSize;
				m_uUsed = uSize;
				m_bIsSorted = false;
			}

			//! Sets if the array should delete the memory it used.
			//! \param f: If true, the array frees the allocated memory in its
			//! destructor, otherwise not. The default is true.
			void SetFreeWhenDestroyed(bool f)
			{
				m_bFreeWhenDestroyed = f;
			}

			//! Sets the size of the array.
			//! \param uUsedNow: Amount of elements now esed.
			void SetUsed(U32 uUsedNow)
			{
				if (m_uAllocated < uUsedNow)
					Reallocate(uUsedNow);

				m_uUsed = uUsedNow;
			}

			//! Assignement operator
			void operator = (const Array<Type>& tOther)
			{
				if (m_pData)
					delete [] m_pData;

				//if (m_uAllocated < tOther.m_uAllocated)
				if (tOther.m_uAllocated == 0)
					m_pData = 0;
				else
					m_pData = new Type[tOther.m_uAllocated];

				m_uUsed = tOther.m_uUsed;
				m_bFreeWhenDestroyed = tOther.m_bFreeWhenDestroyed;
				m_bIsSorted = tOther.m_bIsSorted;
				m_uAllocated = tOther.m_uAllocated;

				for (U32 i=0; i<tOther.m_uUsed; ++i)
					m_pData[i] = tOther.m_pData[i];
			}

			//! Direct access operator
			inline Type& operator [](U32 uIndex)
			{
				Assert(uIndex < m_uUsed);

				return m_pData[uIndex];
			}

			//! Direct access operator
			inline const Type& operator [](U32 uIndex) const
			{
				Assert(uIndex < m_uUsed);

				return m_pData[uIndex];
			}

			//! Gets last frame
			const Type& GetLast() const
			{
				Assert(m_uUsed);

				return m_pData[m_uUsed - 1];
			}

			//! Gets last frame
			Type& GetLast()
			{
				Assert(m_uUsed != 0);

				return m_pData[m_uUsed - 1];
			}
		    
			//! Returns a pointer to the array.
			//! \return Pointer to the array.
			Type* Pointer()
			{
				return m_pData;
			}

			//! Returns a const pointer to the array.
			//! \return Pointer to the array.
			const Type* ConstPointer() const
			{
				return m_pData;
			}

			//! Returns uSize of used array.
			//! \return Size of elements in the array.
			U32 Size() const
			{
				return m_uUsed;
			}

			//! Returns amount memory allocated.
			//! \return Returns amount of memory allocated. The amount of bytes
			//! allocated would  be AllocatedSize() * sizeof(ElementsUsed);
			U32 AllocatedSize() const
			{
				return m_uAllocated;
			}

			//! Returns true if array is empty
			//! \return True if the array is empty, false if not.
			bool IsEmpty() const
			{
				return m_uUsed == 0;
			}

			//! Sorts the array using heapsort. There is no additional memory waste and
			//! the algorithm performs (O) n log n in worst case.
			void Sort()
			{
				if (m_bIsSorted || m_uUsed<2)
					return;

				heapsort(m_pData, m_uUsed);
				m_bIsSorted = true;
			}

			//! Performs a binary search for an element, returns -1 if not found.
			//! The array will be sorted before the binary search if it is not
			//! already sorted.
			//! \param tElement: Element to search for.
			//! \return Returns position of the searched element if it was found,
			//! otherwise -1 is returned.
			S32 BinarySearch(const Type& tElement)
			{
				return BinarySearch(tElement, 0, m_uUsed-1);
			}

			//! Performs a binary search for an element, returns -1 if not found.
			//! The array will be sorted before the binary search if it is not
			//! already sorted.
			//! \param tElement: Element to search for.
			//! \param sLeft: First left index
			//! \param sRight: Last right index.
			//! \return Returns position of the searched element if it was found,
			//! otherwise -1 is returned.
			S32 BinarySearch(const Type& tElement, S32 sLeft, S32 sRight)
			{
				if (!m_uUsed)
					return -1;

				Sort();

				S32 m;

				do
				{
					m = (sLeft + sRight) >> 1;

					if (tElement < m_pData[m])
						sRight = m - 1;
					else
						sLeft = m + 1;

				} while ((tElement < m_pData[m] || m_pData[m] < tElement) && sLeft <= sRight);

				// this last line equals to:
				// " while((tElement != Array[m]) && sLeft<=sRight);"
				// but we only want to use the '<' operator.
				// the same in next line, it is "(tElement == Array[m])"

				if (!(tElement < m_pData[m]) && !(m_pData[m] < tElement))
					return m;

				return -1;
			}

			//! Finds an element in linear time, which is very slow. Use
			//! BinarySearch for faster finding. Only works if == operator is implemented.
			//! \param tElement: Element to search for.
			//! \return Returns position of the searched tElement if it was found,
			//! otherwise -1 is returned.
			S32 LinearSearch(Type& tElement)
			{
				for (U32 i = 0; i < m_uUsed; ++i)
					if (!(tElement < m_pData[i]) && !(m_pData[i] < tElement))
						return (S32)i;

				return -1;
			}

			//! Finds an tElement in linear time, which is very slow. Use
			//! BinarySearch for faster finding. Only works if == operator is implemented.
			//! \param tElement: Element to search for.
			//! \return Returns position of the searched element if it was found,
			//! otherwise -1 is returned.
			S32 LinearReverseSearch(Type& tElement)
			{
				for (S32 i = m_uUsed-1; i >= 0; --i)
					if (m_pData[i] == tElement)
						return (S32)i;

				return -1;
			}

			//! Erases an element from the array. May be slow, because all elements 
			//! following after the erased element have to be copied.
			//! \param uIndex: Index of element to be erased.
			void Erase(U32 uIndex)
			{
				Assert(uIndex < m_uUsed);
				//Assert(uIndex >= 0); // JASOND: Intel compiler: "warning #186: pointless comparison of unsigned integer with zero"

				for (U32 i = uIndex + 1; i < m_uUsed; ++i)
					m_pData[i - 1] = m_pData[i];

				--m_uUsed;
			}

			//! Erases some elements from the Array. may be slow, because all elements 
			//! following after the erased tElement have to be copied.
			//! \param uIndex: Index of the first tElement to be erased.
			//! \param sCount: Amount of elements to be erased.
			void Erase(U32 uIndex, S32 sCount)
			{
				Assert(uIndex < m_uUsed);
				Assert(uIndex >= 0);
				Assert(sCount >= 1);
				Assert(uIndex + sCount <= m_uUsed);

				for (U32 i = uIndex + sCount; i < m_uUsed; ++i)
					m_pData[i - sCount] = m_pData[i];

				m_uUsed -= sCount;
			}

			//! Sets if the Array is sorted
			void SetSorted(bool bIsSorted)
			{
				m_bIsSorted = bIsSorted;
			}

		private:
			Type* m_pData;
			U32 m_uAllocated;
			U32 m_uUsed;
			bool m_bFreeWhenDestroyed;
			bool m_bIsSorted;
		};
	}
}

#endif // _COREARRAY_H_
