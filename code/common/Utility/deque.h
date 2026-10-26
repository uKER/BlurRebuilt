//////////////////////////////////////////////////////////
// Deque: Double Ended Queue
//////////////////////////////////////////////////////////

#pragma once

#include "utility.h"
#include "sequence.h"
#include "vector.h"

namespace Ut {

/// @addtogroup Containers
/// @{

/// Use with UtDeque as a template argument.
/// A double-ended vector with a fixed size.
/// @param T The type stored in the vector.
/// @param N The maximum size of the vector.
template <typename T, int N>
class StaticDevec
{
public:
	StaticDevec() : m_nElements(0), m_firstIndex(0) {}

	/// Number of elements.
	const int Capacity() const {return N;}
	const int Size() const {return m_nElements;} ///< Number of elements.
	T& At(int i){assert(Validate(i)); return elements[BaseIndex(i)];} ///< Random access to elements.
	const T& At(int i) const {return const_cast<StaticDevec*>(this)->At(i);} ///< Random access to elements.

	// manipulation
	void PushBack(const T& item); ///< Insert item at the end of the vector using copy constructor.
	void PushBack(); ///< Insert item at the end of the vector using default constructor.
	void PopBack(); ///< Erase item from the end of the vector.

	void PushFront(const T& item); ///< Insert item at the begining of the vector using copy constructor.
	void PushFront(); ///< Insert item at the beginning of the vector using default constructor.
	void PopFront(); ///< Erase item from the beginning of the vector.

	///< Erase all elements.
	void Clear() {m_nElements = 0; m_firstIndex = 0;}

private:
	int BaseIndex(int i)
	{
		int index = m_firstIndex + i;
		if (index >= Capacity()) index -= Capacity();
		return index;
	}

	bool Validate(int i) const {assert(i >= 0 && i < Size()); return true;}
	//bool Validate(const T* p) const {int i = p - elements[0]; if (i < 0) i += Capacity(); Validate(i);}

	T elements[N];
	int m_nElements;
	int m_firstIndex;
};

/// @}

///////////////////////////////////////////////////////////

template <typename T, int N>
inline void StaticDevec<T, N>::PushBack(const T& item)
{
	assert(m_nElements < Capacity());
	elements[BaseIndex(m_nElements)] = item;
	m_nElements++;
}

template <typename T, int N>
inline void StaticDevec<T, N>::PushBack()
{
	assert(m_nElements < Capacity());
	m_nElements++;
}

template <typename T, int N>
inline void StaticDevec<T, N>::PopBack()
{
	assert(m_nElements > 0);
	m_nElements--;
}

template <typename T, int N>
inline void StaticDevec<T, N>::PushFront(const T& item)
{
	assert(m_nElements < Capacity());
	m_firstIndex -= 1;
	if (m_firstIndex < 0) m_firstIndex = Capacity() - 1;
	elements[m_firstIndex] = item;
	m_nElements++;
}

template <typename T, int N>
inline void StaticDevec<T, N>::PushFront()
{
	assert(m_nElements < Capacity());
	m_firstIndex -= 1;
	if (m_firstIndex < 0) m_firstIndex = Capacity() - 1;
	m_nElements++;
}

template <typename T, int N>
inline void StaticDevec<T, N>::PopFront()
{
	assert(m_nElements > 0);
	m_nElements--;
	m_firstIndex += 1;
	if (m_firstIndex == Capacity()) m_firstIndex = 0;
}

/////////////////////////////////////////////////////////////

/// @addtogroup Containers
/// @{

/// Use with UtDeque as a template argument.
/// A double-ended vector which can change size dynamically.
/// @param T The type stored in the vector.
/// @param N The default initial size of the vector.
/// @see DynamicDeque
template <typename T, int N>
class DynamicDevec
{
public:

	const static int DEFAULT_SIZE = 1;

	/// @param n The initial size of the vector (must be greater than zero).
	DynamicDevec(int n = N) : m_nElements(0), m_capacity(n), m_firstIndex(0)
	{
		BIZ_STATIC_ASSERT(N > 0);
		assert(n > 0); // don't allow zero size to save having special cases to deal with it

		elements = new T[n]; 
		assert(elements);
	}

	~DynamicDevec() {delete [] elements;}

	/// Number of elements allocated.
	const int Capacity() const {return m_capacity;}
	int Size() const {return m_nElements;} ///< Current number of elements.
	T& At(int i){assert(Validate(i)); return elements[BaseIndex(i)];} ///< Random access to elements.
	const T& At(int i) const {return const_cast<DynamicDevec>(this)->At(i);} ///< Random access to elements.

	void PushBack(const T& item); ///< Insert item at the end of the vector using copy constructor.
	void PushBack(); ///< Insert item at the end of the vector using default constructor.
	void PopBack(); ///< Erase item from the end of the vector.

	void PushFront(const T& item); ///< Insert item at the begining of the vector using copy constructor.
	void PushFront(); ///< Insert item at the beginning of the vector using default constructor.
	void PopFront(); ///< Erase item from the beginning of the vector.

	/// Erase all elements (but don't free memory).
	void Clear() {m_nElements = 0; m_firstIndex = m_lastIndex = 0;}

	/// Grow or shrink the capacity. Must have size non-zero and not less than the number of elements.
	void Reserve(int n);

private:
	int BaseIndex(int i)
	{
		int index = m_firstIndex + i;
		if (index >= Capacity()) index -= Capacity();
		return index;
	}

	bool Validate(int i) const {assert(i >= 0 && i < Size()); return true;}
	//bool Validate(const T* p) const {int i = p - elements[0]; if (i < 0) i += Capacity(); Validate(i);}

	T* elements;
	int m_nElements;
	int m_capacity;
	int m_firstIndex;

private:
	DynamicDevec(DynamicDevec& other); // copy protection
	void operator = (DynamicDevec& other); // copy protection
};

/// @}

///////////////////////////////////////////////////////////

template <typename T, int N>
void DynamicDevec<T, N>::Reserve(int n)
{
	assert(n > 0);
	assert(n >= m_nElements); // require n large enough to hold all existing elements
	T* newarray = new T[n];
	assert(newarray);

	for (int i = 0; i < m_nElements; i++)
		newarray[i] = elements[BaseIndex(i)];

	m_capacity = n;
	m_firstIndex = 0;

	delete [] elements;
	elements = newarray;
}

template <typename T, int N>
void DynamicDevec<T, N>::PushBack(const T& item)
{
	if (m_nElements == m_capacity) 
		Reserve(m_capacity * 2);

	elements[BaseIndex(m_nElements)] = item;
	m_nElements++;
}

template <typename T, int N>
void DynamicDevec<T, N>::PushBack()
{
	if (m_nElements == m_capacity) 
		Reserve(m_capacity * 2);

	m_nElements++;
}

template <typename T, int N>
inline void DynamicDevec<T, N>::PopBack()
{
	assert(m_nElements > 0);
	m_nElements--;
}

template <typename T, int N>
void DynamicDevec<T, N>::PushFront(const T& item)
{
	if (m_nElements == m_capacity) 
		Reserve(m_capacity * 2);

	m_firstIndex -= 1;
	if (m_firstIndex < 0) m_firstIndex = Capacity() - 1;
	elements[m_firstIndex] = item;
	m_nElements++;
}

template <typename T, int N>
void DynamicDevec<T, N>::PushFront()
{
	if (m_nElements == m_capacity) 
		Reserve(m_capacity * 2);

	m_firstIndex -= 1;
	if (m_firstIndex < 0) m_firstIndex = Capacity() - 1;
	m_nElements++;
}

template <typename T, int N>
inline void DynamicDevec<T, N>::PopFront()
{
	assert(m_nElements > 0);
	m_nElements--;
	m_firstIndex += 1;
	if (m_firstIndex == Capacity()) m_firstIndex = 0;
}

/////////////////////////////////////////////////////////////

/// @addtogroup Containers
/// @{

/// Double-ended queue.
/// Random access with insertion at both ends.
/// The main disadvantage compared to Vector is slower access to elements though iterators.
/// @param T The type stored in the deque.
/// @param Structure The structure policy; StaticDevec, DynamicDevec or DynamicDeque.
/// @param N The size of the vector (dependent on Structure).
/// @code 
/// UtDeque<MyClass, 100, StaticDevec> vector1; // vector of up to 100 MyClass objects.
/// UtDeque<MyClass> vector2; // default is DynamicDeque
/// @endcode
template <	typename T, 
			template <typename U, int M> class Structure = DynamicDeque, 
			int N = Structure<T, 1>::DEFAULT_SIZE> 
class UtDeque : public Structure<T, N>
{
public:
	/// %Iterator for deque elements
	class ConstIterator
	{
	public:
		// constructors
		ConstIterator() {}
		ConstIterator(const ConstIterator& c) : m_index(c.m_index), m_pDevec(c.m_pDevec) {}
		ConstIterator& operator = (const ConstIterator& c) {m_index = c.m_index; m_pDevec = c.m_pDevec; return *this;}

		// comparison
		bool operator == (const ConstIterator& c) const {return m_index == c.m_index;}
		bool operator == (const T* p) const {return Pointer() == p;}
		bool operator != (const ConstIterator& c) const {return m_index != c.m_index;}
		bool operator != (const T* p) const {return Pointer() != p;}

		// return T
		const T& operator*() const {return (*m_pDevec)[m_index];}
		const T* operator->() const {return &(*m_pDevec)[m_index];}
		const T* Pointer() const {return &(*m_pDevec)[m_index];}
		int Index() const {return m_index;}

		// only define pre-increment as post-increment can be inefficient
		ConstIterator& operator++() {m_index++; return *this;}
		ConstIterator& operator--() {m_index--; return *this;}

		explicit ConstIterator(int i, const UtDeque* v) {m_pDevec = const_cast<UtDeque*>(v); m_index = i;}

	protected:
		int m_index;
		UtDeque* m_pDevec;
	};

	/// %Iterator for deque elements
	class Iterator : public ConstIterator
	{
	public:
		// constructors
		Iterator() {}
		Iterator(const Iterator& c) : ConstIterator(c) {}
		Iterator& operator = (const Iterator c) {m_pointer = c.m_pointer; m_pDevec = c.m_pDevec; return *this;}

		// return T
		T& operator*() const {return (*m_pDevec)[m_index];}
		T* operator->() const {return &(*m_pDevec)[m_index];}
		T* Pointer() {return &(*m_pDevec)[m_index];}

		Iterator& operator++() {m_index++; return *this;}
		Iterator& operator--() {m_index--; return *this;}

		explicit Iterator(int i, UtDeque* v) {m_index = i; m_pDevec = v;}
	};

	// reverse iterators
	typedef ConstReverseIterator_t<T, UtDeque<T, Structure, N>, int> ConstReverseIterator; 
	typedef ReverseIterator_t<T, UtDeque<T, Structure, N>, int> ReverseIterator; 

	// number of elements
	bool Empty() const {return Size() == 0;} ///< Return true if the vector is empty.

	// access first and last elements
	T& Front() {assert(Size() > 0); return At(0);} ///< Access the first element in the vector.
	T& Back() {assert(Size() > 0); return At(Size() - 1);} ///< Access the last element in the vector.
	const T& Front() const {assert(Size() > 0); return At(0);} ///< Access the first element in the vector.
	const T& Back() const {assert(Size() > 0); return At(Size() - 1);} ///< Access the last element in the vector.

	// random access
	__forceinline T& operator [] (int i) {return At(i);} ///< Random access to elements.
	__forceinline const T& operator [] (int i) const {return At(i);} ///< Random access to elements.

	// iterators
	Iterator Begin() {return Iterator(0, this);} ///< %Iterator pointing to the first element in the vector.
	ConstIterator Begin() const {return ConstIterator(0, this);} ///< %Iterator pointing to the first element in the vector.
	Iterator End() {return Iterator(Size(), this);} ///< %Iterator representing the end of the vector.
	ConstIterator End() const {return Iterator(Size(), this);} ///< %Iterator representing the end of the vector.

	// reverse iterators
	ReverseIterator ReverseBegin() {return ReverseIterator(Size() - 1, this);} ///< Reverse iterator pointing to the first element in the vector.
	ConstReverseIterator ReverseBegin() const {return ConstReverseIterator(Size() - 1, this);} ///< Reverse iterator pointing to the first element in the vector.
	ReverseIterator ReverseEnd() {return ReverseIterator(-1, this);} ///< Reverse iterator representing the end of the vector.
	ConstReverseIterator ReverseEnd() const {return ConstReverseIterator(-1, this);} ///< Reverse iterator representing the end of the vector.

	/// Insert item before element pointed to by iterator.
	/// @remarks This is not an efficient operation.
	void InsertBefore(const T& item, Iterator i)
	{
		if (i.Index() > Size() / 2)
		{
			PushBack();
			ReverseIterator r1 = ReverseBegin();

			do 
			{
				ReverseIterator r2 = r1;
				++r1;
				*r2 = *r1;
			} while (&(*r1) != &(*i));

			*i = item;
		}
		else
		{
			PushFront();
			Iterator r1 = Begin();

			while (&(*r1) != &(*i)) 
			{
				Iterator r2 = r1;
				++r1;
				*r2 = *r1;
			};

			*i = item;
		}
	}

	/// Erase item pointed to by iterator, and return iterator pointing to next item, or the end of the vector if no items remain.
	/// @remarks It's usually faster to swap with the last element and use PopBack.
	Iterator Erase(Iterator i)
	{
		if (i.Index() > Size() / 2)
		{
			Iterator fi(i.Index(), this);
			Iterator fj = fi;

			while (++fj != End())
			{
				*fi = *fj;
			};

			PopBack();
			return i;
		}
		else
		{
			ReverseIterator ri(i.Index(), this);
			ReverseIterator rj = ri;

			while (++rj != ReverseEnd())
			{
				*ri = *rj;
			};

			PopFront();
			return ++i;
		}
	}
};

// DynamicDeque
/// Use with UtDeque as a template argument.
/// A double-ended queue which can change size dynamically.
/// Unlike DynamicDevec, the elements are stored in blocks.
/// This means that the deque can grow without moving the existing elements, which is more efficient and pointers remain valid.
/// And it can shrink automatically as elements are removed.
/// Access through iterators is slower than DynamicDevec (and much slower than any kind of Vector).
/// @param T The type stored in the queue.
/// @param N The number of items in each block (defaults to 4k block size).
template <typename T, int N>
class DynamicDeque
{
public:

	const static int DEFAULT_SIZE = UtMaths::Max((1024 * 4 - 8) / sizeof(T), 1);

	DynamicDeque() : m_nElements(0), m_firstIndex(0) {}
	~DynamicDeque();

	int Size() const {return m_nElements;} ///< Current number of elements.
	T& At(int i); ///< Random access to elements.
	const T& At(int i) const {return const_cast<DynamicDeque>(this)->At(i);} ///< Random access to elements.

	void PushBack(const T& item); ///< Insert item at the end of the vector using copy constructor.
	void PushBack(); ///< Insert item at the end of the vector using default constructor.
	void PopBack(); ///< Erase item from the end of the vector.

	void PushFront(const T& item); ///< Insert item at the begining of the vector using copy constructor.
	void PushFront(); ///< Insert item at the beginning of the vector using default constructor.
	void PopFront(); ///< Erase item from the beginning of the vector.
	void Clear(); ///< Erase all elements

private:

	bool Validate(int i) const {assert(i >= 0 && i < Size()); return true;}
	int BaseIndex(int i) const {return m_firstIndex + i;}

	typedef UtVector<ArrayVector<T, N> > Block;

	// keep deque of pointers to blocks
	typedef UtDeque<Block*, DynamicDevec> BlockStorage;
	BlockStorage m_blockPointers;

	int m_nElements;
	int m_firstIndex; // index of first element in first block.

private:
	DynamicDeque(DynamicDeque& other); // copy protection
	void operator = (DynamicDeque& other); // copy protection
};

/// @}

//////////////////////////////////////////////////////////////////

template <typename T, int N>
DynamicDeque<T, N>::~DynamicDeque()
{
	Clear();
}

template <typename T, int N>
T& DynamicDeque<T, N>::At(int i)
{
	assert(Validate(i));
	int baseIndex = BaseIndex(i);
	int block = baseIndex / N;
	int index = baseIndex % N;

	return m_blockPointers[block]->At(index);
}

template <typename T, int N>
void DynamicDeque<T, N>::PushBack(const T& item)
{
	if (BaseIndex(m_nElements) % N == 0)
		m_blockPointers.PushBack(new Block);

	m_nElements++;
	At(m_nElements - 1) = item;
}

template <typename T, int N>
void DynamicDeque<T, N>::PushBack()
{
	if (BaseIndex(m_nElements) % N == 0)
		m_blockPointers.PushBack(new Block);

	m_nElements++;
}

template <typename T, int N>
void DynamicDeque<T, N>::PopBack()
{
	m_nElements--;

	if (BaseIndex(m_nElements) % N == 0)
	{
		delete m_blockPointers.Back();
		m_blockPointers.PopBack();
	}
}

template <typename T, int N>
void DynamicDeque<T, N>::PushFront(const T& item)
{
	if (m_firstIndex == 0)
	{
		m_blockPointers.PushFront(new Block);
		m_firstIndex = N - 1;
	}
	else
		m_firstIndex--;

	m_nElements++;
	At(0) = item;
}

template <typename T, int N>
void DynamicDeque<T, N>::PushFront()
{
	if (m_firstIndex == 0)
	{
		m_blockPointers.PushFront(new Block);
		m_firstIndex = N - 1;
	}
	else
		m_firstIndex--;

	m_nElements++;
}

template <typename T, int N>
void DynamicDeque<T, N>::PopFront()
{
	m_nElements--;
	m_firstIndex++;

	if (m_firstIndex == N)
	{
		delete m_blockPointers.Front();
		m_blockPointers.PopFront();
		m_firstIndex = 0;
	}
}

template <typename T, int N>
void DynamicDeque<T, N>::Clear()
{
	for (BlockStorage::Iterator i = m_blockPointers.Begin(); i != m_blockPointers.End(); ++i)
		delete *i;

	m_nElements = 0;
	m_firstIndex = 0;
}

} // end Ut