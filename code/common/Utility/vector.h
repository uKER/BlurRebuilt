//////////////////////////////////////////////////////
// Vector container
//////////////////////////////////////////////////////

#ifndef __GNUC__
#pragma once
#endif
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "utility.h"
#include "sequence.h"

#ifndef __GNUC__
#ifndef NDEBUG
#include <iterator>	// cjc: defines std::random_access_iterator_tag
#endif
#endif

namespace Ut 
{

/// @addtogroup Containers
/// @{

/////////////////////////////////////////////////////////////

// ArrayVector
/// Use as a template argument for UtVector.
/// Behaves like an array, except for bounds checking in debug version.
/// Always has N elements. Therefore, inserting and erasing elements is not supported.
/// Capacity() returns a compile-time constant.
/// @param T The type stored in the vector.
/// @param N The size of the vector.
template <typename T, int N>
class ArrayVector
{
public:
	/// Number of elements.
	int Size() const {return N;}

	/// Number of elements.
	int Capacity() const {return N;}

	bool Empty() const {return false;} ///< An array vector can never be empty.

	void Resize(int n) {assert(n == N);} ///< Resize if valid only if the size is the same.

protected:
	ArrayVector() {}

	T elements[N];

	typedef T DataType;
	typedef T* PointerType;
	typedef T& ReferenceType;
	typedef const T* ConstPointerType;
	typedef const T& ConstReferenceType;
	typedef NullType Description;
	typedef NullType AllocatorType;
};

//////////////////////////////////////////////////////

// StackVector
/// Use as a template argument for UtVector.
/// A fixed-size vector that behaves as a stack.
/// Has PushBack and PopBack functions and keeps track of number of elements.
/// @param T The type stored in the vector.
/// @param N The maximum size of the vector.
template <typename T, U32 N>
class StackVector : public ArrayVector<T, N>
{
public:
	/// Access first (bottom) element.
	T& Front() {assert((m_nElements) > 0); return ArrayVector<T, N>::elements[0];}

	/// Access first (bottom) element.
	const T& Front() const {assert((m_nElements) > 0); return ArrayVector<T, N>::elements[0];}

	/// Access last (top) element.
	T& Back() {assert((m_nElements) > 0); return ArrayVector<T, N>::elements[m_nElements - 1];}

	/// Access last (top) element.
	const T& Back() const {assert((m_nElements) > 0); return ArrayVector<T, N>::elements[m_nElements - 1];}

	/// Number of elements.
	int Size() const {return m_nElements;}

	/// True if the vector is empty.
	bool Empty() const {return (m_nElements) == 0;}

	/// Push item on top of the stack by copying.
	template <typename U>
	void PushBack(const U& item) {assert((m_nElements) < N); ArrayVector<T, N>::elements[m_nElements] = item; m_nElements++;}

	/// Push item on top of the stack using default constructor.
	void PushBack() {assert((m_nElements) < N); m_nElements++;}

	// Insert item before position i
	void InsertBefore(const T& item, U32 i) 
	{
		assert((m_nElements) < N && (m_nElements) > i);

		for (U32 j=(m_nElements); j > i ; j--)
			Ut::Move(ArrayVector<T, N>::elements[j], ArrayVector<T, N>::elements[j - 1]);

		ArrayVector<T, N>::elements[i] = item;
		m_nElements++;
	}

	// Erase item at position i
	void Erase(U32 i)
	{
		assert((m_nElements) > i);
		m_nElements--;

		for (U32 j=i; j < m_nElements; j++)
			Ut::Move(ArrayVector<T, N>::elements[j], ArrayVector<T, N>::elements[j + 1]);
	}

	/// Erase item on top of the stack.
	void PopBack() {assert((m_nElements) > 0); m_nElements--;}
	void Clear() {m_nElements = 0;} ///< erase all elements

	/// Change the number of elements.
	void Resize(U32 n)
	{
		assert(n >= 0 && n <= N);
		m_nElements = n;
	}

	// copy just the active elements
	StackVector& operator = (const StackVector& other)
	{
		m_nElements = other.m_nElements;

		for (U32 i = 0; i < (m_nElements); i++)
			ArrayVector<T, N>::elements[i] = other.elements[i];

		return *this;
	}

protected:
	StackVector() : m_nElements(0) {}

private:
	U32 m_nElements;
};

/*
/// Vector with storage managed from outside.
template <typename T>
class ManagedVector
{
public:
	/// Number of elements.
	int Size() const {return m_size;}

	/// Number of elements.
	int Capacity() const {return m_size;}

	bool Empty() const {return m_size == 0;}

	/// Initialise with a pointer.
	void Initialise(T* p, int size) 
	{
		assert(elements == 0); 
		elements = p; 
		m_size = size;
	}

	void Resize(int n) {assert(n == m_size);} // Resize if valid only if the size is the same.

protected:
	ManagedVector() : elements(0), m_size(0) {}
	ManagedVector(T* p, int size) : elements(p), m_size(size) {}
	~ManagedVector() {} // Doesn't own the pointer, so don't free it.

	T* elements;
	int m_size;

	typedef T DataType;
	typedef T* PointerType;
	typedef T& ReferenceType;
	typedef const T* ConstPointerType;
	typedef const T& ConstReferenceType;
	typedef NullType Description;
	typedef NullType AllocatorType;
};
*/

///////////////////////////////////////////////////////////
// iterators for AdaptableVector

template <typename Description>
class ConstAdaptableIterator
{
public:
	typedef typename Description::ConstReferenceType ConstReferenceType;

	ConstAdaptableIterator() : m_data(0), m_description(0) {}
	ConstAdaptableIterator(const char* data, const Description* description)
		: m_data(const_cast<char*>(data)), m_description(description) {}

	// pointer operators
	ConstReferenceType operator *() const {return ConstReferenceType(m_data, *m_description);}
	bool operator >= (const ConstAdaptableIterator& p) const {return m_data >= p.m_data;}
	bool operator > (const ConstAdaptableIterator& p) const {return m_data > p.m_data;}
	bool operator <= (const ConstAdaptableIterator& p) const {return m_data <= p.m_data;}
	bool operator < (const ConstAdaptableIterator& p) const {return m_data < p.m_data;}
	bool operator == (const ConstAdaptableIterator& p) const {return m_data == p.m_data;}
	bool operator != (const ConstAdaptableIterator& p) const {return m_data != p.m_data;}
	ConstAdaptableIterator operator + (int n) const {return ConstAdaptableIterator(m_data + m_description->ObjectSize() * n, m_description);}
	ConstAdaptableIterator operator - (int n) const {return ConstAdaptableIterator(m_data - m_description->ObjectSize() * n, m_description);}
	int operator - (const ConstAdaptableIterator& p) const {return (m_data - p.m_data) / m_description->ObjectSize();}
	ConstAdaptableIterator& operator++() {m_data += m_description->ObjectSize(); return *this;}
	ConstAdaptableIterator& operator--() {m_data -= m_description->ObjectSize(); return *this;}

protected:
	char* m_data;
	const Description* m_description;
};

template <typename Description>
class AdaptableIterator : public ConstAdaptableIterator<Description>
{
public:
	typedef typename Description::ReferenceType ReferenceType;

	AdaptableIterator() {}
	AdaptableIterator(const char* data, const Description* description)
		: ConstAdaptableIterator<Description>(data, description) {}

	// pointer operations
	using ConstAdaptableIterator<Description>::operator -;
	ReferenceType operator *() const {return ReferenceType(ConstAdaptableIterator<Description>::m_data, *ConstAdaptableIterator<Description>::m_description);}
	AdaptableIterator operator + (int n) const {return AdaptableIterator(ConstAdaptableIterator<Description>::m_data + ConstAdaptableIterator<Description>::m_description->ObjectSize() * n, ConstAdaptableIterator<Description>::m_description);}
	AdaptableIterator operator - (int n) const {return AdaptableIterator(ConstAdaptableIterator<Description>::m_data - ConstAdaptableIterator<Description>::m_description->ObjectSize() * n, ConstAdaptableIterator<Description>::m_description);}
	AdaptableIterator& operator++() {ConstAdaptableIterator<Description>::m_data += ConstAdaptableIterator<Description>::m_description->ObjectSize(); return *this;}
	AdaptableIterator& operator--() {ConstAdaptableIterator<Description>::m_data -= ConstAdaptableIterator<Description>::m_description->ObjectSize(); return *this;}	
};

/////////////////////////////////////////////////////////////

// Storage for dynamic vector of T.
// This has little error checking. We rely on DynamicVector to use it correctly.
template<typename T, typename Allocator = ArrayAllocator<T> >
class DynamicArrayData
{
public:
	class Description
	{
	public:
		typedef T& ReferenceType;
		typedef const T& ConstReferenceType;
	};

	typedef T DataType;
	typedef const T* ConstPointerType;
	typedef T* PointerType;
	typedef Allocator AllocatorType;

	DynamicArrayData() : m_elements(0), m_capacity(0) {}
	~DynamicArrayData() 
	{
		Allocator::Free(m_elements, m_capacity);
	}

	DynamicArrayData(const DynamicArrayData& other, int size) : m_elements(0), m_capacity(0)
	{
		Copy(other, size);
	}

	void Initialise(int n, const Description& d) 
	{
		assert(m_capacity == 0);

		if (n > 0)
		{
			m_elements = Allocator::Allocate(n);//new T[n]; 
			assert(m_elements);
		}
		
		m_capacity = n; 
	}

	int Capacity() const {return m_capacity;}

	// n is the capacity required, size is the number of elements currently in use
	// returns true for success
	bool Reserve(int n, int size)
	{
		if (m_capacity == n) return true;
		assert(size <= n);

		if (n > 0)
		{
			if (m_capacity > n)
			{
				return true;		// Capacity is more than enough, so no need to reallocate.
			}

			T* newarray = (T*) Allocator::Allocate(n);//new T[m_capacity];
			if (!newarray)
				return false;

			//for (int i = 0; i < size; i++)
			//	Ut::Move(newarray[i], m_elements[i]);

			MoveToNewArray(&newarray[0], &m_elements[0], size);

			Allocator::Free(m_elements, m_capacity);
			m_elements = newarray;
		}
		else
		{
			assert(size == 0);
			Allocator::Free(m_elements, m_capacity);
			m_elements = 0;
		}

		m_capacity = n;

		return true;
	}

	// non-destructive copy
	void Copy(const DynamicArrayData& other, int size) // size is the number of elements required
	{
		if (m_capacity != size)
		{
			Allocator::Free(m_elements, m_capacity);
			m_capacity = size;

			if (size > 0)
			{
				m_elements = Allocator::Allocate(m_capacity);//new T[m_capacity];
				assert(m_elements);
			}
			else
			{
				m_elements = 0;
			}
		}

		for (int i = 0; i < size; i++)
			m_elements[i] = other.m_elements[i];
	}

	void Clear(int i) {Ut::Clear(m_elements[i]);}
#ifndef __GNUC__	// JAN: error: Ut::MoveArray is undefined
	void Move(int dst, int src, int n) {Ut::MoveArray(m_elements + dst, m_elements + src, n);}
#endif

	UT_FORCE_INLINE typename Description::ReferenceType operator [] (int n) {assert(n < m_capacity); return m_elements[n];}
	UT_FORCE_INLINE typename Description::ConstReferenceType operator [] (int n) const {assert(n < m_capacity); return m_elements[n];}

private:
	T* m_elements;
	int m_capacity;
};

template <typename ReferenceType, typename Allocator>
class AdaptableArrayData
{
public:
	typedef ReferenceType DataType;
	typedef typename ReferenceType::DescriptionType Description;
	typedef ConstAdaptableIterator<Description> ConstPointerType;
	typedef AdaptableIterator<Description> PointerType;

	AdaptableArrayData() : m_data(0), m_capacity(0), m_description(0) {}
	~AdaptableArrayData() {Allocator::Free(m_data, 0); Allocator::Free(m_description);}

	AdaptableArrayData(const AdaptableArrayData& other, int size) : m_data(0), m_capacity(0), m_description(0)
	{
		Copy(other, size);
	}

	void Initialise(int n, const Description& description) 
	{
		assert(m_capacity == 0);
		m_description = Allocator::template Allocate<Description>();
		new (m_description) Description(description);

		if (n > 0)
		{
			m_data = Allocator::template Allocate<char>(n * m_description->ObjectSize());
			assert(m_data);
		}

		m_capacity = n;
		assert(m_description);
	}

	bool Reserve(int n, int size) // n is the capacity required, size is the number of elements currently in use
	{
		if (m_capacity == n) return true;

		if (n > 0)
		{
			if (m_capacity > n)
			{
				return true;		// Capacity is more than enough, so no need to reallocate.
			}

			char* newarray = Allocator::template Allocate<char>(n * m_description->ObjectSize());
			assert(newarray);

			for (int i = 0; i < size * m_description->ObjectSize(); i++)
				newarray[i] = m_data[i];

			Allocator::Free(m_data, m_capacity);
			m_data = newarray;
		}
		else
		{
			assert(size == 0);
			Allocator::Free(m_data, m_capacity);
			m_data = 0;
		}

		m_capacity = n;

		return true;
	}

	// non-destructive copy
	void Copy(const AdaptableArrayData& other, int size) // size is the number of elements required
	{
		Allocator::Free(m_description);

		if (other.m_description)
		{
			m_description = Allocator::template Allocate<Description>();
			new (m_description) Description(*other.m_description);
		}

		if (m_capacity != size)
		{
			m_capacity = size;
			Allocator::Free(m_data);

			if (size > 0)
			{
				m_data = Allocator::template Allocate<char>(m_capacity * m_description->ObjectSize());
				assert(m_data);
			}
			else
			{
				m_data = 0;
			}
		}

		if (size > 0)
		{
			for (int i = 0; i < size * m_description->ObjectSize(); i++)
				m_data[i] = other.m_data[i];
		}
	}

	void Clear(int i) {}
	void Mode(int dst, int src, int n);

	int Capacity() const {return m_capacity;}
	ReferenceType operator [] (int n) {return ReferenceType(&m_data[n * m_description->ObjectSize()], *m_description);}
	typename Description::ConstReferenceType operator [] (int n) const {return Description::ConstReferenceType(&m_data[n * m_description->ObjectSize()], *m_description);}

private:
	// copy protection
	AdaptableArrayData(const AdaptableArrayData& other);

	char* m_data;
	int m_capacity;
	Description* m_description;
};

/////////////////////////////////////////////////////////////

/// Wraps a dynamic vector storage class.
template<typename Data, int N>
class DynamicVectorBase
{
public:
	typedef typename Data::DataType DataType;
	typedef typename Data::Description Description;
	typedef typename Data::ConstPointerType ConstPointerType;
	typedef typename Data::PointerType PointerType;
	typedef typename Description::ConstReferenceType ConstReferenceType;
	typedef typename Description::ReferenceType ReferenceType;

	// This creates an empty vector which must be initialised before use.
	// Only AdaptableVector calls this.
	DynamicVectorBase() : m_nElements(0) {BIZ_STATIC_ASSERT(N >= 0);}
	~DynamicVectorBase() {}

	DynamicVectorBase(const DynamicVectorBase& other) : elements(other.elements, other.m_nElements), m_nElements(other.m_nElements) {}
	
	// Initialise capacity to n.
	DynamicVectorBase(int n) : m_nElements(0)
	{
		BIZ_STATIC_ASSERT(N >= 0);
		assert(n >= 0);
		elements.Initialise(n, typename Data::Description());
	}

	DynamicVectorBase(const Description& d, int n = N) : m_nElements(0)
	{
		BIZ_STATIC_ASSERT(N >= 0);
		assert(n >= 0);
		elements.Initialise(n, d);
	}

	// This is allowed as long as the capacity is zero
	void Initialise(const Description& d, int n = N)
	{
		assert(Capacity() == 0);
		assert(n >= 0);
		elements.Initialise(n, d);
	}

	/// Access first element.
	ReferenceType Front() {assert(m_nElements > 0); return elements[0];}

	/// Access last element.
	ReferenceType Back() {assert(m_nElements > 0); return elements[m_nElements - 1];}

	/// Access first element.
	ConstReferenceType Front() const {assert(m_nElements > 0); return elements[0];}

	/// Access last element.
	ConstReferenceType Back() const {assert(m_nElements > 0); return elements[m_nElements - 1];}

	/// Number of elements.
	int Size() const {return m_nElements;}

	/// Number of elements reserved.
	int Capacity() const {return elements.Capacity();}

	/// Return true if the list is empty.
	bool Empty() const {return m_nElements == 0;}

	/// Push copy of item on the end of the vector.
	template <typename T>
	void PushBack(T& item) 
	{
		if (Size() == Capacity())
		{
			T t(item); // copy it in case it is an item already in the vector
			if (elements.Reserve(Capacity() * 2 + 1, m_nElements))
				elements[m_nElements] = t;
			else
				assert(0 && "out of memory");
		}
		else
		{
			elements[m_nElements] = item;
		}

		m_nElements++;
	}

	/// Push copy of item on the end of the vector.
	template <typename T>
	void PushBack(const T& item) 
	{
		if (Size() == Capacity())
		{
			T t(item); // copy it in case it is an item already in the vector
			if (elements.Reserve(Capacity() * 2 + 1, m_nElements))
				elements[m_nElements] = t;
			else
				assert(0 && "out of memory");
		}
		else
		{
			elements[m_nElements] = item;
		}

		m_nElements++;
	}

	/// Push item on the end of the vector using the default constructor.
	void PushBack() 
	{
		if (Size() == Capacity())
		{
			Reserve(Capacity() * 2 + 1);
		}

		m_nElements++;
	}

	// Insert item before position i
	void InsertBefore(ConstReferenceType item, int i) 
	{
		assert(m_nElements > i);

		if (Size() == Capacity())
		{
			Reserve(Capacity() * 2 + 1);
		}

		for (int j = m_nElements; j > i ; j--)
			Ut::Move(elements[j], elements[j - 1]);

		//elements.Move(i + 1, i, m_nElements - i);

		elements[i] = item;
		m_nElements++;
	}

	// Erase item at position i
	void Erase(U32 i)
	{
		assert((U32)m_nElements > i);

		if (i == m_nElements - 1)
		{
			elements.Clear(i);
		}
		else
		{
			for (int j = i; j < m_nElements - 1; j++)
				Ut::Move(elements[j], elements[j + 1]);
		}

		//elements.Move(i, i + 1, m_nElements - 1 - i);

		m_nElements--;
	}

	/// Erase last element.
	void PopBack() 
	{
		assert(m_nElements > 0);
		elements.Clear(m_nElements - 1);
		m_nElements--;
	}

	/// Erase all elements (but don't free memory).
	void Clear() 
	{
		for (int i = 0; i < m_nElements; i++)
			elements.Clear(i);

		m_nElements = 0;
	} 

	/// Grow or shrink the capacity. Must have size  not less than the number of elements.
	bool Reserve(int n, bool assertOnFailure=true) 
	{ 
		assert(n >= m_nElements); // require n large enough to hold all existing elements
		bool success = elements.Reserve(n, m_nElements);
		if (assertOnFailure)
			assert(success && "out of memory");

		return success;
	}

	/// Change the number of elements (but don't free memory).
	void Resize(int n)
	{
		if (m_nElements < n) elements.Reserve(n, m_nElements);

		for (int i = n; i < m_nElements; i++)
			elements.Clear(i);

		m_nElements = n;
	}

	// Copy a vector
	void operator = (const DynamicVectorBase& other)
	{
		if (this == &other) return;
		elements.Copy(other.elements, other.m_nElements);
		m_nElements = other.m_nElements;
	}

protected:
	Data elements;
	int m_nElements;
};

///////////////////////////////////////////////////////////

// DynamicVector
/// Use as a template argument for UtVector.
/// A vector which can change size dynamically.
/// @param T The type stored in the vector.
/// @param N The default initial size of the vector.
/// @remarks
/// A resizable vector can be quite expensive compared to a fixed-size vector.
/// Given the size of the array in advance, the compiler can often optimise out a lot of code.
/// This is even without the cost of a resize, which involves copying the entire vector.
/// Access to elements costs about the same in both cases.
template<typename T, typename Allocator = ArrayAllocator<T> >
class DynamicVector : public DynamicVectorBase<DynamicArrayData<T, Allocator>, 0>
{
	typedef DynamicVectorBase<DynamicArrayData<T, Allocator>, 0> BaseType;

protected:
	DynamicVector(int n = 0) : BaseType(n) {}
	//DynamicVector(const DynamicVector& other) : BaseType(other) {}
	~DynamicVector() {}

public:
	typedef Allocator AllocatorType;
};

template<typename T, typename Allocator = GeneralAllocator<StandardAllocator<T>, ArrayAllocator<T> > >
class AdaptableVector : public DynamicVectorBase<AdaptableArrayData<T, Allocator>, 0>
{
	typedef DynamicVectorBase<AdaptableArrayData<T, Allocator>, 0> BaseType;
	
protected:
	AdaptableVector() : BaseType() {}
	AdaptableVector(T d, int n = 0) : BaseType(d, n) {}
	AdaptableVector(const AdaptableVector& other) : BaseType(other) {}
	~AdaptableVector() {}

public:
	typedef Allocator AllocatorType;
};

/////////////////////////////////////////////////////////////
//  Vector
/////////////////////////////////////////////////////////////

/// %Vector class.
///
/// @code 
/// Vector<ArrayVector<MyClass, 100> > vector1; // vector of 100 pre-allocated MyClass objects.
/// Vector<StackVector<MyClass, 100> > vector2; // vector of up to 100 pre-allocated MyClass objects.
/// Vector<DynamicVector<MyClass> > vector3; // vector which allocates space for more objects as required.
/// @endcode
///
/// @param Structure The structure policy; ArrayVector, StackVector or DynamicVector.
template <class Structure>
class UtVector : public Structure
{
public:

	typedef Structure StructureType;
	typedef typename StructureType::DataType DataType;
	typedef typename StructureType::PointerType PointerType;
	typedef typename StructureType::ReferenceType ReferenceType;
	typedef typename StructureType::ConstPointerType ConstPointerType;
	typedef typename StructureType::ConstReferenceType ConstReferenceType;

#ifndef NDEBUG 

	class ConstCheckedIterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category ;
		typedef DataType value_type;
		typedef int difference_type;
		typedef ConstPointerType pointer;
		typedef ConstReferenceType reference;

		// constructors
		ConstCheckedIterator() {}
		ConstCheckedIterator(const ConstCheckedIterator& c) : m_pointer(c.m_pointer), m_pVector(c.m_pVector) {}
		ConstCheckedIterator& operator = (const ConstCheckedIterator& c) {m_pointer = c.m_pointer; m_pVector = c.m_pVector; return *this;}
		
		// comparison
		bool operator == (const ConstCheckedIterator& c) const {return m_pointer == c.m_pointer;} ///< @a
		bool operator == (ConstPointerType p) const {return m_pointer == p;} ///< @a
		bool operator != (const ConstCheckedIterator& c) const {return m_pointer != c.m_pointer;} ///< @a
		bool operator != (ConstPointerType p) const {return m_pointer != p;} ///< @a

		// return T
		ConstReferenceType operator*() const {m_pVector->Validate(m_pointer); return *m_pointer;} ///< @a
		ConstPointerType operator->() const {m_pVector->Validate(m_pointer); return m_pointer;} ///< @a

		// arithmetic
		ConstCheckedIterator operator + (int n) const {return ConstCheckedIterator(m_pointer + n, m_pVector);} ///< @a
		ConstCheckedIterator operator - (int n) const {return ConstCheckedIterator(m_pointer - n, m_pVector);} ///< @a
		int operator - (const ConstCheckedIterator& c) const {return m_pointer - c.m_pointer;} ///< @a

		ConstCheckedIterator& operator ++() {++m_pointer; return *this;} ///< @a
		ConstCheckedIterator& operator --() {--m_pointer; return *this;} ///< @a
		ConstCheckedIterator operator ++(int) {ConstCheckedIterator p(*this); ++m_pointer; return p;} ///< @a
		ConstCheckedIterator operator --(int) {ConstCheckedIterator p(*this); --m_pointer; return p;} ///< @a

		bool operator < (const ConstCheckedIterator& other) {assert(m_pVector == other.m_pVector); return m_pointer < other.m_pointer;} ///< @a

		explicit ConstCheckedIterator(ConstPointerType p, const UtVector* v) {m_pointer = (PointerType&)(p); m_pVector = v;}

	protected:
		PointerType m_pointer;
		const UtVector* m_pVector;
	};

	class CheckedIterator : public ConstCheckedIterator
	{
	public:
		typedef PointerType pointer;
		typedef ReferenceType reference;
		using ConstCheckedIterator::m_pointer;
		using ConstCheckedIterator::m_pVector;

		// constructors
		CheckedIterator() {}
		CheckedIterator(const CheckedIterator& c) : ConstCheckedIterator(c) {}
		CheckedIterator& operator = (const CheckedIterator c) {m_pointer = c.m_pointer; m_pVector = c.m_pVector; return *this;}
		
		// return T
		ReferenceType operator*() const {m_pVector->Validate(m_pointer); return *m_pointer;} ///< @a
		PointerType operator->() const {m_pVector->Validate(m_pointer); return m_pointer;} ///< @a

		// arithmetic
		using ConstCheckedIterator::operator -;
		CheckedIterator operator + (int n) const {return CheckedIterator(m_pointer + n, m_pVector);} ///< @a
		CheckedIterator operator - (int n) const {return CheckedIterator(m_pointer - n, m_pVector);} ///< @a

		CheckedIterator& operator++() {++m_pointer; return *this;} ///< @a
		CheckedIterator& operator--() {--m_pointer; return *this;} ///< @a
		CheckedIterator operator ++(int) {CheckedIterator p(*this); ++m_pointer; return p;} ///< @a
		CheckedIterator operator --(int) {CheckedIterator p(*this); --m_pointer; return p;} ///< @a

		explicit CheckedIterator(PointerType p, const UtVector* v) {m_pointer = p; m_pVector = v;}
	};

	typedef CheckedIterator Iterator; ///< Iterator for Vector elements.
	typedef ConstCheckedIterator ConstIterator; ///< Iterator for Vector elements.

	// reverse iterators
	typedef ConstReverseIterator_t<DataType, UtVector> ConstReverseIterator; ///< Reverse iterator for Vector elements.
	typedef ReverseIterator_t<DataType, UtVector> ReverseIterator; ///< Reverse iterator for Vector elements.

private:
	ConstIterator MakeConstIterator(ConstPointerType p) const {return ConstIterator(p, this);}
	Iterator MakeIterator(PointerType p) {return Iterator(p, this);}

	ConstReverseIterator MakeConstReverseIterator(ConstPointerType p) const {return ConstReverseIterator(p, this);}
	ReverseIterator MakeReverseIterator(PointerType p) {return ReverseIterator(p, this);}

#else

	// in release version just use pointers as iterators
	typedef PointerType Iterator;
	typedef ConstPointerType ConstIterator;

	// reverse iterators
	typedef ConstReverseIterator_t<DataType, UtVector> ConstReverseIterator;
	typedef ReverseIterator_t<DataType, UtVector> ReverseIterator;

private:
	ConstIterator MakeConstIterator(ConstPointerType p) const {return p;}
	Iterator MakeIterator(PointerType p) {return p;}

	ConstReverseIterator MakeConstReverseIterator(ConstPointerType p) const {return ConstReverseIterator(p);}
	ReverseIterator MakeReverseIterator(PointerType p) {return ReverseIterator(p);}

#endif

public:

	UtVector() {} 
	~UtVector() {}

	// will construct from anything, as long as the base class does
	template<typename T> UtVector(T& other) : StructureType(other) {}

	// check for valid pointer into array
	bool Validate(PointerType p) const {assert(p >= First() && p < First() + this->Size()); return true;}

	/// Random access to vector.
	UT_FORCE_INLINE ReferenceType At(int i) {assert(i >= 0 && i < this->Size()); return Structure::elements[i];} /// Random access to vector.
	UT_FORCE_INLINE ConstReferenceType At(int i) const {assert(i >= 0 && i < this->Size()); return Structure::elements[i];} ///< Random access to vector.
	UT_FORCE_INLINE ReferenceType operator [] (int i) {assert(i >= 0 && i < this->Size()); return Structure::elements[i];} /// Random access to vector.
	UT_FORCE_INLINE ConstReferenceType operator [] (int i) const {assert(i >= 0 && i < this->Size()); return Structure::elements[i];} ///< Random access to vector.

	// get iterators
	UT_FORCE_INLINE Iterator Begin() {return MakeIterator(First());} ///< %Iterator pointing to the first element in the vector.
	UT_FORCE_INLINE ConstIterator Begin() const {return MakeConstIterator(First());}  ///< %Iterator pointing to the first element in the vector.
	UT_FORCE_INLINE Iterator End() {return MakeIterator(First() + this->Size());} ///< %Iterator pointing one past the last element of the vector.
	UT_FORCE_INLINE ConstIterator End() const {return MakeConstIterator(First() + this->Size());} ///< %Iterator pointing one past the last element of the vector.
	
	// reverse iterators
	ReverseIterator ReverseBegin() {return MakeReverseIterator(First() + this->Size() - 1);} ///< Reverse iterator pointing to the last element in the vector.
	ConstReverseIterator ReverseBegin() const {return MakeConstReverseIterator(First() + this->Size() - 1);}  ///< Reverse iterator pointing to the last element in the vector.
	ReverseIterator ReverseEnd() {return MakeReverseIterator(First() - 1);} ///< Reverse iterator pointing one before the start of the vector.
	ConstReverseIterator ReverseEnd() const {return MakeConstReverseIterator(First() - 1);} ///< Reverse iterator pointing one before the start of the vector.

	/// Swap two elements.
	template<typename T>void Swap(Iterator i1, Iterator i2) {T temp = *i1; *i1 = *i2; *i2 = temp;} /// @a

	/// Insert item before element pointed to by iterator. (Not ArrayVector.)
	/// @remarks This is not an efficient operation.
	void InsertBefore(ConstReferenceType item, Iterator i) {StructureType::InsertBefore(item, (int) (&(*i) - First()));}


	/// Erase item pointed to by iterator, and return iterator pointing to next item, or the end of the vector if no items remain. (Not ArrayVector.)
	/// @remarks It's usually faster to swap with the last element and use PopBack.
	Iterator Erase(Iterator i)
	{
		StructureType::Erase((U32)(&(*i) - First())); 
		if (this->Size() > 0)
			return i;
		else
			return End();
	}

	void EraseByIndex(int i) {StructureType::Erase(i);}
	void Destroy()
	{
		Structure::Clear();
		Structure::Reserve(0);
	}

private:
	// get pointers
	UT_FORCE_INLINE PointerType First() {if (this->Size() > 0) return &Structure::elements[0]; else return 0;} // points to first item
	UT_FORCE_INLINE ConstPointerType First() const {if (this->Size() > 0) return &Structure::elements[0]; else return 0;}
};

//////////////////////////////////////////////////////////////////
// functions that operate on vectors

/// Remove elements that equal the input from the vector.
template <class Vector, class Element>
inline void Remove(UtVector<Vector>& s, Element element)
{
	UtVector<Vector> newVector;

	for (typename UtVector<Vector>::Iterator i = s.Begin(); i != s.End(); ++i)
		if (*i != element)
			newVector.PushBack(*i);

	Swap(newVector, s);
}

/// Faster version of remove that doesn't maintain the order of the elements.
template <class Vector, class Element>
inline void RemoveNonOrdered(UtVector<Vector>& s, Element element)
{
	typename UtVector<Vector>::Iterator i = s.Begin();

	while (i != s.End())
	{
		if (*i == element)
		{
			Swap(*i, s.Back());
			s.PopBack();
		}
		else
		{
			++i;
		}
	};
}

/// Swap the two vectors
template <class Vector>
inline void Swap(UtVector<Vector>& left, UtVector<Vector>& right)
{
	MoveSwap(left, right);
}

 
// UtDynamicVector - A default dynamic vector template
template <typename T>
class UtDynamicVector : public UtVector<DynamicVector<T> >
{
};
 
/// Swap the two UtDynamicVectors
template <class T>
inline void Swap(UtDynamicVector<T>& left, UtDynamicVector<T>& right)
{
      MoveSwap(left, right);
}

// Vector - same as above, but with abbreviated name
template <typename T>
class Vector : public UtVector<DynamicVector<T> >
{
};
 
/// Swap the two Vectors
template <class T>
inline void Swap(Vector<T>& left, Vector<T>& right)
{
      MoveSwap(left, right);
}

/// @}

}

//////////////////////////////////////////////////////////////////

#ifdef GOTHAM3

namespace Ut
{

template <typename T, int ID>
struct TrackedDynamicVector
{
	typedef Ut::UtVector<Ut::DynamicVector<T, Ut::TrackedArrayAllocator<T, ID> > > Result;
};

}

#include "stackallocator.h"

namespace Ut
{

template <typename T, int ID>
struct StackedDynamicVector
{
	typedef Ut::UtVector<Ut::DynamicVector<T, Ut::StackArrayAllocator<T> > > Result;
};

}

#else

namespace Ut
{

template <typename T, int ID>
struct TrackedDynamicVector
{
	typedef Ut::UtVector<Ut::DynamicVector<T> > Result;
};

template <typename T, int ID>
struct StackedDynamicVector
{
	typedef Ut::UtVector<Ut::DynamicVector<T> > Result;
};

}

#endif

#endif
