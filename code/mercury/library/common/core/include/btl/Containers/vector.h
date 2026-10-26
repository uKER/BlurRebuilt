#ifndef BTL_VECTOR_H
#define BTL_VECTOR_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Vector container
//////////////////////////////////////////////////////

#include <btl/allocators/allocator.h>
#include <btl/containers/sequence.h>
#include <core/utility/coredebug.h>

#ifndef CFG_SPEED
#include <iterator> // to allow vector iterators to be used in stl algorithms
#endif

namespace Btl 
{

/// @addtogroup Containers
/// @{

/////////////////////////////////////////////////////////////

// Stop the compiler calling empty constructors
// (but makes it harder to debug)
#if defined(CFG_SPEED)
#define VECTOR_ARRAY_EXPLICIT_CONSTRUCTION
#endif

/// Base type for an array vector.
template <typename T, int N>
class VectorStructure_Array
{
public:
	/// Number of elements.
	FORCE_INLINE int Size() const {return N;}

	/// Number of elements.
	FORCE_INLINE int Capacity() const {return N;}

	/// An array vector can never be empty.
	FORCE_INLINE bool Empty() const {return false;} 

	/// Resize will assert if the size is not the current size.
	FORCE_INLINE void Resize(int n) {BTL_ASSERT(n == N);} 

	/// Reserve will assert if the size is greater than the current size.
	FORCE_INLINE void Reserve(int n) {BTL_ASSERT(n <= N);} 

	/// Access first element.
	T& Front() {return Element(0);}

	/// Access first element.
	FORCE_INLINE const T& Front() const {return Element(0);}

	/// Access last element.
	FORCE_INLINE T& Back() {return Element(N - 1);}

	/// Access last element.
	FORCE_INLINE const T& Back() const {return Element(N - 1);}

protected:

#ifdef VECTOR_ARRAY_EXPLICIT_CONSTRUCTION 
	FORCE_INLINE VectorStructure_Array() ALWAYS_INLINE
	{
		for (int i = 0; i < N; i++)
			new (&Element(i)) T;
	}

	FORCE_INLINE ~VectorStructure_Array() ALWAYS_INLINE
	{
		for (int i = 0; i < N; i++)
			Element(i).~T();
	}

	VectorStructure_Array(const VectorStructure_Array& other)
	{
		for (U32 i = 0; i < N; i++)
			new (&Element(i)) T(other.Element(i));
	}

	VectorStructure_Array& operator = (const VectorStructure_Array& other)
	{
		for (U32 i = 0; i < N; i++)
			Element(i) = other.Element(i);

		return *this;
	}
#endif

protected:

#ifdef VECTOR_ARRAY_EXPLICIT_CONSTRUCTION
	FORCE_INLINE const T& Element(int i) const ALWAYS_INLINE {return *(((const T*) &storage) + i);}
	FORCE_INLINE T& Element(int i) ALWAYS_INLINE {return *(((T*) &storage) + i);}
#else
	FORCE_INLINE const T& Element(int i) const ALWAYS_INLINE {return *(const T*) (void*) &storage[i];}
	FORCE_INLINE T& Element(int i) ALWAYS_INLINE {return *(T*) (void*) &storage[i];}
#endif

	typedef T DataType;
	typedef T* PointerType;
	typedef T& ReferenceType;
	typedef const T* ConstPointerType;
	typedef const T& ConstReferenceType;
	typedef NullType Description;
	typedef NullType AllocatorType;

private:

#ifdef VECTOR_ARRAY_EXPLICIT_CONSTRUCTION
	SameSizeAndAlignment<T, N> storage;
#else
	T storage[N];
#endif
};

//////////////////////////////////////////////////////

/// Base type for a static vector.
template <typename T, int N>
class VectorStructure_Static : public VectorStructure_Array<T, N>
{
public:
	/// Access first (bottom) element.
	T& Front() {BTL_ASSERT((m_nElements) > 0); return VectorStructure_Array<T, N>::Element(0);}

	/// Access first (bottom) element.
	FORCE_INLINE const T& Front() const {BTL_ASSERT((m_nElements) > 0); return VectorStructure_Array<T, N>::Element(0);}

	/// Access last (top) element.
	FORCE_INLINE T& Back() {BTL_ASSERT((m_nElements) > 0); return VectorStructure_Array<T, N>::Element(m_nElements - 1);}

	/// Access last (top) element.
	FORCE_INLINE const T& Back() const {BTL_ASSERT((m_nElements) > 0); return VectorStructure_Array<T, N>::Element(m_nElements - 1);}

	/// Number of elements.
	FORCE_INLINE int Size() const {return m_nElements;}

	/// True if the vector is empty.
	FORCE_INLINE bool Empty() const {return (m_nElements) == 0;}

	/// Push item on top of the stack by copying.
	template <typename U>
	void PushBack(const U& item) {BTL_ASSERT((m_nElements) < N); VectorStructure_Array<T, N>::Element(m_nElements) = item; m_nElements++;}

	/// Push item on top of the stack using default constructor.
	void PushBack() {BTL_ASSERT((m_nElements) < N); m_nElements++;}

	// Insert item before position i
	void InsertBefore(const T& item, U32 i) 
	{
		BTL_ASSERT((m_nElements) < N && (m_nElements) >= i);

		for (U32 j=(m_nElements); j > i ; j--)
			Move(VectorStructure_Array<T, N>::Element(j), VectorStructure_Array<T, N>::Element(j - 1));

		VectorStructure_Array<T, N>::Element(i) = item;
		m_nElements++;
	}

	// Erase item at position i
	void EraseByIndex(U32 i)
	{
		BTL_ASSERT((m_nElements) > i);

		if (i == m_nElements - 1)
			Clear(i);
		else
		{
			for (U32 j = i; j < m_nElements - 1; j++)
				Move(VectorStructure_Array<T, N>::Element(j), VectorStructure_Array<T, N>::Element(j + 1));
		}

		m_nElements--;
	}

	// Erase item at position i by swapping it with the last item and then popping the last item
	void EraseByIndexNonOrdered(U32 i)
	{
		BTL_ASSERT((U32)m_nElements > i);

		if (i == m_nElements - 1)
			Clear(i);
		else
			Move(VectorStructure_Array<T, N>::Element(i), VectorStructure_Array<T, N>::Element(m_nElements-1));

		// decrease the count by 1
		m_nElements--;
	}

	/// Erase item on top of the stack.
	FORCE_INLINE void PopBack() {BTL_ASSERT((m_nElements) > 0); m_nElements--;}
	FORCE_INLINE void Clear() {Resize(0);} ///< erase all elements

	/// Change the number of elements.
	void Resize(U32 n)
	{
		BTL_ASSERT(n >= 0 && n <= N);

		for (U32 i = n; i < m_nElements; i++)
			Clear(i);

		m_nElements = n;
	}

	// copy just the active elements
	VectorStructure_Static& operator = (const VectorStructure_Static& other)
	{
		Resize(other.m_nElements);

		for (U32 i = 0; i < (m_nElements); i++)
			VectorStructure_Array<T, N>::Element(i) = other.Element(i);

		return *this;
	}

	void Destroy()
	{
		Resize(0);
	}

protected:
	VectorStructure_Static() : m_nElements(0) {}

private:
	void Clear(U32 i) 
	{
		BTL_ASSERT(i >= 0 && i < N && i < m_nElements);
		VectorStructure_Array<T, N>::Element(i).~T();
		new (&VectorStructure_Array<T, N>::Element(i)) T;
	}

	void Move(T& dst, T& src)
	{
		dst.~T();
		Btl::Move(&dst, &src);
		new (&src) T;
	}

	U32 m_nElements;
};

/////////////////////////////////////////////////////////////

// Storage for dynamic vector of T.
// This has little error checking. We rely on DynamicVector to use it correctly.
template<typename T, typename Allocator>
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
	~DynamicArrayData() ALWAYS_INLINE
	{
		FreeElements();
	}

	DynamicArrayData(const DynamicArrayData& other, int size)
	{
		T* newElements = 0;

		if (size > 0)
		{
			newElements = AllocateElements(size);
			BTL_ASSERT(newElements);
		}

		for (int i = 0; i < size; i++)
			Create(newElements[i], other.m_elements[i]);

		m_elements = newElements;
		m_capacity = size;
	}

	void Initialise(int n, const Description& d) 
	{
		BTL_ASSERT(m_capacity == 0);

		if (n > 0)
		{
			m_elements = AllocateElements(n);
			BTL_ASSERT(m_elements);
		}
		
		m_capacity = n; 
	}

	int Capacity() const {return m_capacity;}

	// n is the capacity required, size is the number of elements currently in use
	// returns true for success
	bool Reserve(int n, int size)
	{
		if (m_capacity >= n) return true;
		BTL_ASSERT(size <= n);

		T* newarray = AllocateElements(n);
		if (!newarray)
			return false;

		if (size)
			MoveArray(&newarray[0], &m_elements[0], size);

		FreeElements();
		m_elements = newarray;
		m_capacity = n;

		return true;
	}

	// Same as reserve but size is guaranteed to be zero.
	bool InitialiseCapacity(int n)
	{
		if (m_capacity >= n) return true;

		T* newarray = AllocateElements(n);
		if (!newarray)
			return false;

		FreeElements();
		m_elements = newarray;

		m_capacity = n;

		return true;
	}

	// non-destructive copy
	void Copy(const DynamicArrayData& other, int currentSize, int newSize, int newCapacity) // size is the number of elements required
	{
		BTL_ASSERT(newCapacity >= newSize);

		for (int i = 0; i < currentSize; i++)
			Destroy(i);

		T* newElements = m_elements;

		if (m_capacity < newCapacity)
		{
			FreeElements();
			m_capacity = newCapacity;

			if (newCapacity > 0)
			{
				newElements = AllocateElements(m_capacity);
				BTL_ASSERT(newElements);
			}
			else
			{
				newElements = 0;
			}
		}

		for (int i = 0; i < newSize; i++)
			Create(newElements[i], other.m_elements[i]);

		m_elements = newElements;
	}

	void Destroy()
	{
		FreeElements();
		m_capacity = 0;
		m_elements = 0;
	}

	FORCE_INLINE typename Description::ReferenceType operator [] (int n) {BTL_ASSERT(n < m_capacity); return m_elements[n];}
	FORCE_INLINE typename Description::ConstReferenceType operator [] (int n) const {BTL_ASSERT(n < m_capacity); return m_elements[n];}

	// create/destroy/move elements
	void Create(T& dest) {new (&dest) T;}
	template <typename U> void Create(T& dest, const U& element) {new (&dest) T(element);}
	template <typename U> void Create(T& dest, U& element) {new (&dest) T(element);}
	void Destroy(int i) {m_elements[i].~T();}

	void OffsetLocation(S32 offset) 
	{
		if (m_elements)
		{
			U8* p = ((U8*)m_elements) + offset;
			m_elements = (T*) p;
		}
	}

private:
	void FreeElements() {Allocator::Free((typename Allocator::PointerType) m_elements, m_capacity);}
	T* AllocateElements(int n) ALWAYS_INLINE {return (T*) Allocator::Allocate(n);}
	T* m_elements;
	int m_capacity;
};

/////////////////////////////////////////////////////////////

/// Base type for a dynamic vector.
template<typename Data>
class DynamicVectorBase
{
public:
	typedef typename Data::DataType DataType;
	typedef typename Data::Description Description;
	typedef typename Data::ConstPointerType ConstPointerType;
	typedef typename Data::PointerType PointerType;
	typedef typename Description::ConstReferenceType ConstReferenceType;
	typedef typename Description::ReferenceType ReferenceType;

	DynamicVectorBase() : m_nElements(0) {}
	~DynamicVectorBase() ALWAYS_INLINE
	{
		for (int i = 0; i < m_nElements; i++)
			elements.Destroy(i);
	}

	DynamicVectorBase(const DynamicVectorBase& other) : elements(other.elements, other.m_nElements), m_nElements(other.m_nElements) {}

	/// Access first element.
	FORCE_INLINE ReferenceType Front() {BTL_ASSERT(m_nElements > 0); return elements[0];}

	/// Access last element.
	FORCE_INLINE ReferenceType Back() {BTL_ASSERT(m_nElements > 0); return elements[m_nElements - 1];}

	/// Access first element.
	FORCE_INLINE ConstReferenceType Front() const {BTL_ASSERT(m_nElements > 0); return elements[0];}

	/// Access last element.
	FORCE_INLINE ConstReferenceType Back() const {BTL_ASSERT(m_nElements > 0); return elements[m_nElements - 1];}

	/// Number of elements.
	FORCE_INLINE int Size() const {return m_nElements;}

	/// Number of elements reserved.
	FORCE_INLINE int Capacity() const {return elements.Capacity();}

	/// Return true if the list is empty.
	FORCE_INLINE bool Empty() const {return m_nElements == 0;}

	/// Push copy of item on the end of the vector.
	template <typename T>
	__forceinline void PushBack(T& item) 
	{
		if (Size() == Capacity())
		{
			T t(item); // copy it in case it is an item already in the vector
			if (elements.Reserve((Capacity() * 3) / 2 + 1, m_nElements))
				elements.Create(elements[m_nElements], t);
			else
				BTL_ASSERT(0 && "out of memory");
		}
		else
		{
			elements.Create(elements[m_nElements], item);
		}

		m_nElements++;
	}

	/// Push copy of item on the end of the vector.
	template <typename T>
	__forceinline void PushBack(const T& item) 
	{
		if (Size() == Capacity())
		{
			T t(item); // copy it in case it is an item already in the vector
			if (elements.Reserve((Capacity() * 3) / 2 + 1, m_nElements))
				elements.Create(elements[m_nElements], t);
			else
				BTL_ASSERT_FAIL("out of memory");
		}
		else
		{
			elements.Create(elements[m_nElements], item);
		}

		m_nElements++;
	}

	/// Push item on the end of the vector using the default constructor.
	__forceinline void PushBack() 
	{
		if (Size() == Capacity())
		{
			Reserve((Capacity() * 3 / 2) + 1);
		}

		elements.Create(elements[m_nElements]);
		m_nElements++;
	}

	// Insert item before position i
	void InsertBefore(ConstReferenceType item, int i) 
	{
		BTL_ASSERT(m_nElements >= i);

		if (Size() == Capacity())
		{
			Reserve((Capacity() * 3) / 2 + 1);
		}

		for (int j = m_nElements; j > i ; j--)
			Move(&elements[j], &elements[j - 1]);

		elements.Create(elements[i], item);
		m_nElements++;
	}

	// Erase item at position i
	void EraseByIndex(U32 i)
	{
		BTL_ASSERT((U32)m_nElements > i);

		elements.Destroy(i);

		for (int j = i; j < m_nElements - 1; j++)
			Move(&elements[j], &elements[j + 1]);

		m_nElements--;
	}

	// Erase item at position i by swapping it with the last item and then popping the last item
	void EraseByIndexNonOrdered(U32 i)
	{
		BTL_ASSERT((U32)m_nElements > i);

		elements.Destroy(i);

		// move the last element onto the _i_th element
		if (i != m_nElements-1)
			Move(&Element(i), &Element(m_nElements-1));

		// decrease the count by 1
		m_nElements--;
	}

	/// Erase last element.
	void PopBack() 
	{
		BTL_ASSERT(m_nElements > 0);
		elements.Destroy(m_nElements - 1);
		m_nElements--;
	}

	/// Erase all elements (but don't free memory).
	void Clear() ALWAYS_INLINE
	{
		for (int i = 0; i < m_nElements; i++)
			elements.Destroy(i);

		m_nElements = 0;
	} 

	/// Equivalent to Resize, except that the current size must be zero.
	void InitialiseSize(int n, bool assertOnFailure=true)
	{
		if (Size() == 0)
		{
			bool success = elements.InitialiseCapacity(n);

			if (assertOnFailure)
				BTL_ASSERT_MSG(success, "out of memory(%d)", n);

			if (success)
			{
				for (int i = 0; i < n; i++)
					elements.Create(elements[i]);

				m_nElements = n;
			}
		}
		else
			BTL_ASSERT_FAIL("Cannot initialise non zero-size vector.");
	}

	/// Reserve enough space for n elements.
	/// Returns true for success.
	bool Reserve(int n, bool assertOnFailure=true) 
	{ 
		if (n > m_nElements)
		{
			bool success = elements.Reserve(n, m_nElements);
			if (assertOnFailure)
			{
				BTL_ASSERT_MSG(success, "out of memory(%d)", n);
			}
			return success;
		}

		return true;
	}

	/// Reserve enough space for n elements, where the current size must be zero.
	bool InitialiseCapacity(int n, bool assertOnFailure=true) 
	{ 
		BTL_ASSERT(m_nElements == 0);
		bool success = elements.InitialiseCapacity(n);
		if (assertOnFailure)
		{
			BTL_ASSERT(success && "out of memory");
		}

		return success;
	}

	/// Change the number of elements (but don't free memory).
	/// Returns true for success.
	bool Resize(int n, bool assertOnFailure = true)
	{
		BTL_ASSERT(n >= 0);
		if (m_nElements < n)
		{
			bool success = Reserve(n, assertOnFailure);
			if (!success)
				return false;
		}

		for (int i = m_nElements; i < n; i++)
			elements.Create(elements[i]);

		for (int i = n; i < m_nElements; i++)
			elements.Destroy(i);

		m_nElements = n;

		return true;
	}

	/// Reduce the capacity of the vector to match its size.
	void TrimCapacity()
	{
		if (Capacity() > Size())
		{
			DynamicVectorBase b(*this);
			SwapByMemcopy(*this, b);
		}
	}

	/// %Set the capacity of the vector.
	void SetCapacity(int n)
	{
		BTL_ASSERT_MSG(n >= Size(), "n(%d) Size(%d)", n, Size());

		if (n > Size())
		{
			DynamicVectorBase newVector;
			newVector.elements.Copy(elements, 0, Size(), n);
			newVector.m_nElements = Size();
            SwapByMemcopy(*this, newVector);
		}
	}

	// Copy a vector
	void operator = (const DynamicVectorBase& other)
	{
		if (this == &other) return;
		elements.Copy(other.elements, m_nElements, other.m_nElements,  other.m_nElements);
		m_nElements = other.m_nElements;
	}

	void OffsetLocation(S32 offset) ALWAYS_INLINE {elements.OffsetLocation(offset);}

	/// Erase all elements and free memory.
	void Destroy()
	{
		for (int i = 0; i < m_nElements; i++)
			elements.Destroy(i);

		m_nElements = 0;

		elements.Destroy();
	} 

protected:
	FORCE_INLINE typename Description::ConstReferenceType Element(int i) const ALWAYS_INLINE {return elements[i];}
	FORCE_INLINE typename Description::ReferenceType Element(int i) ALWAYS_INLINE {return elements[i];}

private:
	Data elements;
	int m_nElements;
};

///////////////////////////////////////////////////////////

/// Vector for which size = capacity.
/// This makes it smaller than a fully dynamic vector; 8 bytes instead of 12.
template<typename Data>
class FixedSizeVectorBase
{
public:
	typedef typename Data::DataType DataType;
	typedef typename Data::Description Description;
	typedef typename Data::ConstPointerType ConstPointerType;
	typedef typename Data::PointerType PointerType;
	typedef typename Description::ConstReferenceType ConstReferenceType;
	typedef typename Description::ReferenceType ReferenceType;

	FixedSizeVectorBase() ALWAYS_INLINE {}
	~FixedSizeVectorBase() ALWAYS_INLINE
	{
		for (int i = 0; i < Size(); i++)
			elements.Destroy(i);
	}

	FixedSizeVectorBase(const FixedSizeVectorBase& other) : elements(other.elements, other.Size()) {}

	/// Access first element.
	FORCE_INLINE ReferenceType Front() {BTL_ASSERT(Size() > 0); return elements[0];}

	/// Access last element.
	FORCE_INLINE ReferenceType Back() {BTL_ASSERT(Size() > 0); return elements[Size() - 1];}

	/// Access first element.
	FORCE_INLINE ConstReferenceType Front() const {BTL_ASSERT(Size() > 0); return elements[0];}

	/// Access last element.
	FORCE_INLINE ConstReferenceType Back() const {BTL_ASSERT(Size() > 0); return elements[Size() - 1];}

	/// Number of elements.
	FORCE_INLINE int Size() const {return elements.Capacity();}

	/// Number of elements reserved.
	FORCE_INLINE int Capacity() const {return elements.Capacity();}

	/// Return true if the list is empty.
	FORCE_INLINE bool Empty() const {return Size() == 0;}

	/// Equivalent to Resize, except that the current size must be zero.
	void InitialiseSize(int n, bool assertOnFailure=true)
	{
		if (Size() == 0)
		{
			bool success = elements.InitialiseCapacity(n);

			if (assertOnFailure)
			{
				BTL_ASSERT_MSG(success, "out of memory(%d)", n);
			}

			if (success)
			{
				for (int i = 0; i < n; i++)
					elements.Create(elements[i]);
			}
		}
		else
		{
			BTL_ASSERT_FAIL("Cannot initialise non zero-size vector.");
		}
	}

	/// Erase all elements and free memory.
	void Destroy() 
	{
		for (int i = 0; i < Size(); i++)
			elements.Destroy(i);

		elements.Destroy();
	} 

	// Copy a vector
	void operator = (const FixedSizeVectorBase& other)
	{
		if (this == &other) return;
		elements.Copy(other.elements, Size(), other.Size(), other.Size());
	}

	void OffsetLocation(S32 offset) ALWAYS_INLINE {elements.OffsetLocation(offset);}

protected:
	FORCE_INLINE typename Description::ConstReferenceType Element(int i) const ALWAYS_INLINE {return elements[i];}
	FORCE_INLINE typename Description::ReferenceType Element(int i) ALWAYS_INLINE {return elements[i];}

private:
	Data elements;
};

///////////////////////////////////////////////////////////

#define DYNAMIC_VECTOR_ALLOCATOR typename Allocator::template Rebind<SameSizeAndAlignment<T> >::Other

/// Structure type for dynamic vector.
template<typename T, typename Allocator>
class VectorStructure_Dynamic : public DynamicVectorBase<DynamicArrayData<T, DYNAMIC_VECTOR_ALLOCATOR> >
{
public:
	typedef DYNAMIC_VECTOR_ALLOCATOR AllocatorType;
private:
	typedef DynamicVectorBase<DynamicArrayData<T, AllocatorType> > BaseType;
protected:

	VectorStructure_Dynamic() : BaseType() {}
	~VectorStructure_Dynamic() ALWAYS_INLINE {}
};

/// Structure type for dynamic vector.
template<typename T, typename Allocator>
class VectorStructure_FixedSize : public FixedSizeVectorBase<DynamicArrayData<T, DYNAMIC_VECTOR_ALLOCATOR> >
{
public:
	typedef DYNAMIC_VECTOR_ALLOCATOR AllocatorType;
private:
	typedef FixedSizeVectorBase<DynamicArrayData<T, AllocatorType> > BaseType;
protected:

	VectorStructure_FixedSize() : BaseType() {}
	~VectorStructure_FixedSize() ALWAYS_INLINE {}
};

/////////////////////////////////////////////////////////////

/// Use as a template argument for Vector.
/// Creates a vector using VectorStructure_Array.
/// Behaves like an array, except for bounds checking in debug version.
/// Always has N elements. Therefore, inserting and erasing elements is not supported.
/// Capacity() returns a compile-time constant.
/// @param N The size of the vector.
template<int N>
struct VectorParameters_Array
{
	template<typename T>
	struct Structure
	{
		typedef VectorStructure_Array<T, N> Result;
	};
};

/// Use as a template argument for Vector.
/// Creates a vector using VectorStructure_Static.
/// A fixed-capacity vector that behaves as a stack.
/// Has PushBack and PopBack functions and keeps track of number of elements.
/// @param N The maximum size of the vector.
template<int N>
struct VectorParameters_Static
{
	template<typename T>
	struct Structure
	{
		typedef VectorStructure_Static<T, N> Result;
	};
};

/// Use as a template argument for Vector.
/// Creates a vector using VectorStructure_Dynamic.
/// A vector which can change size dynamically.
/// @param Allocator The type used to allocate memory for the vector.
#if !PLATFORM_SPU
template <typename Allocator = ArrayAllocator<NullType> >
#else
template <typename Allocator = LinearArrayAllocator<NullType> >
#endif
struct VectorParameters_Dynamic
{
	template<typename T>
	struct Structure
	{
		typedef typename Allocator::template Rebind<T>::Other AllocatorType;
		typedef VectorStructure_Dynamic<T, AllocatorType> Result;
	};
};

/// Use as a template argument for Vector.
/// Creates a vector using VectorStructure_FixedSize.
/// A vector which is created with a fixed size.
/// @param Allocator The type used to allocate memory for the vector.
#if !PLATFORM_SPU
template <typename Allocator = ArrayAllocator<NullType> >
#else
template <typename Allocator = LinearArrayAllocator<NullType> >
#endif
struct VectorParameters_FixedSize
{
	template<typename T>
	struct Structure
	{
		typedef typename Allocator::template Rebind<T>::Other AllocatorType;
		typedef VectorStructure_FixedSize<T, AllocatorType> Result;
	};
};

/////////////////////////////////////////////////////////////
//  Vector
/////////////////////////////////////////////////////////////

/// %Vector class.
///
/// @code
/// Vector<MyClass, VectorParameters_Array<100> > vector1; // vector of 100 pre-allocated MyClass objects.
/// Vector<MyClass, VectorParameters_Static<100> > vector2; // vector of up to 100 pre-allocated MyClass objects.
/// Vector<MyClass, VectorParameters_Dynamic<> > vector3; // vector which allocates space for more objects as required.
/// Vector<MyClass> // default is VectorStructure_Dynamic.
/// @endcode
///
/// @param Parameters The structure policy; VectorParameters_Array, VectorParameters_Static or VectorParameters_Dynamic.@n
template <typename T, class Parameters = VectorParameters_Dynamic<> >
class Vector : public Parameters::template Structure<T>::Result
{
public:
	typedef typename Parameters::template Structure<T>::Result StructureType;
	typedef typename StructureType::DataType DataType;
	typedef typename StructureType::PointerType PointerType;
	typedef typename StructureType::ReferenceType ReferenceType;
	typedef typename StructureType::ConstPointerType ConstPointerType;
	typedef typename StructureType::ConstReferenceType ConstReferenceType;

#ifndef CFG_SPEED

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

		ConstCheckedIterator& operator += (int n) {m_pointer += n; return *this;} ///< @a
		ConstCheckedIterator& operator -= (int n) {m_pointer -= n; return *this;} ///< @a

		ConstCheckedIterator& operator ++() {++m_pointer; return *this;} ///< @a
		ConstCheckedIterator& operator --() {--m_pointer; return *this;} ///< @a
		ConstCheckedIterator operator ++(int) {ConstCheckedIterator p(*this); ++m_pointer; return p;} ///< @a
		ConstCheckedIterator operator --(int) {ConstCheckedIterator p(*this); --m_pointer; return p;} ///< @a

		bool operator < (const ConstCheckedIterator& other) {BTL_ASSERT(m_pVector == other.m_pVector); return m_pointer < other.m_pointer;} ///< @a

		explicit ConstCheckedIterator(ConstPointerType p, const Vector* v) {m_pointer = (PointerType&)(p); m_pVector = v;}

	protected:
		PointerType m_pointer;
		const Vector* m_pVector;
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

		CheckedIterator& operator += (int n) {m_pointer += n; return *this;} ///< @a
		CheckedIterator& operator -= (int n) {m_pointer -= n; return *this;} ///< @a

		CheckedIterator& operator++() {++m_pointer; return *this;} ///< @a
		CheckedIterator& operator--() {--m_pointer; return *this;} ///< @a
		CheckedIterator operator ++(int) {CheckedIterator p(*this); ++m_pointer; return p;} ///< @a
		CheckedIterator operator --(int) {CheckedIterator p(*this); --m_pointer; return p;} ///< @a

		explicit CheckedIterator(PointerType p, const Vector* v) {m_pointer = p; m_pVector = v;}
	};

	typedef CheckedIterator Iterator; ///< Iterator for Vector elements.
	typedef ConstCheckedIterator ConstIterator; ///< Iterator for Vector elements.

	// reverse iterators
	typedef ConstReverseIterator_t<DataType, Vector> ConstReverseIterator; ///< Reverse iterator for Vector elements.
	typedef ReverseIterator_t<DataType, Vector> ReverseIterator; ///< Reverse iterator for Vector elements.

	Iterator MakeIterator(int index) {return Iterator(&StructureType::Element(index), this);}
	ConstIterator MakeConstIterator(int index) {return ConstIterator(&StructureType::Element(index), this);}

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
	typedef ConstReverseIterator_t<DataType, Vector> ConstReverseIterator;
	typedef ReverseIterator_t<DataType, Vector> ReverseIterator;

	Iterator MakeIterator(int index) {return &StructureType::Element(index);}
	ConstIterator MakeConstIterator(int index) {return &StructureType::Element(index);}

private:
	ConstIterator MakeConstIterator(ConstPointerType p) const {return p;}
	Iterator MakeIterator(PointerType p) {return p;}

	ConstReverseIterator MakeConstReverseIterator(ConstPointerType p) const {return ConstReverseIterator(p);}
	ReverseIterator MakeReverseIterator(PointerType p) {return ReverseIterator(p);}

#endif

public:

	Vector() {} 
	~Vector() ALWAYS_INLINE {}

	// will construct from anything, as long as the base class does
	template<typename U> Vector(U& other) : StructureType(other) {}

	// check for valid pointer into array
	bool Validate(PointerType p) const {BTL_ASSERT(p >= First() && p < First() + this->Size()); return true;}

	/// Random access to vector.
	FORCE_INLINE ReferenceType At(int i) ALWAYS_INLINE {BTL_ASSERT_MSG(i >= 0 && i < this->Size(), "i(%d) Size(%d)", i, this->Size()); return StructureType::Element(i);} ///< Random access to vector.
	FORCE_INLINE ConstReferenceType At(int i) const ALWAYS_INLINE {BTL_ASSERT_MSG(i >= 0 && i < this->Size(), "i(%d) Size(%d)", i, this->Size()); return StructureType::Element(i);} ///< Random access to vector.
	FORCE_INLINE ReferenceType operator [] (int i) ALWAYS_INLINE {BTL_ASSERT_MSG(i >= 0 && i < this->Size(), "i(%d) Size(%d)", i, this->Size()); return StructureType::Element(i);} ///< Random access to vector.
	FORCE_INLINE ConstReferenceType operator [] (int i) const ALWAYS_INLINE {BTL_ASSERT_MSG(i >= 0 && i < this->Size(), "i(%d) Size(%d)", i, this->Size()); return StructureType::Element(i);} ///< Random access to vector.

	// get iterators
	FORCE_INLINE Iterator Begin() {return MakeIterator(First());} ///< %Iterator pointing to the first element in the vector.
	FORCE_INLINE ConstIterator Begin() const {return MakeConstIterator(First());}  ///< %Iterator pointing to the first element in the vector.
	FORCE_INLINE Iterator End() {return MakeIterator(First() + this->Size());} ///< %Iterator pointing one past the last element of the vector.
	FORCE_INLINE ConstIterator End() const {return MakeConstIterator(First() + this->Size());} ///< %Iterator pointing one past the last element of the vector.
	
	// reverse iterators
	ReverseIterator ReverseBegin() {return MakeReverseIterator(First() + this->Size() - 1);} ///< Reverse iterator pointing to the last element in the vector.
	ConstReverseIterator ReverseBegin() const {return MakeConstReverseIterator(First() + this->Size() - 1);}  ///< Reverse iterator pointing to the last element in the vector.
	ReverseIterator ReverseEnd() {return MakeReverseIterator(First() - 1);} ///< Reverse iterator pointing one before the start of the vector.
	ConstReverseIterator ReverseEnd() const {return MakeConstReverseIterator(First() - 1);} ///< Reverse iterator pointing one before the start of the vector.

	/// Swap two elements.
	template<typename U> void Swap(Iterator i1, Iterator i2) {U temp = *i1; *i1 = *i2; *i2 = temp;} /// @a 

	/// Insert item before element pointed to by iterator. (Not ArrayVector.)
	/// @remarks This is not an efficient operation.
	void InsertBefore(ConstReferenceType item, Iterator i) {StructureType::InsertBefore(item, (int) (&(*i) - First()));}

	/// Erase item pointed to by iterator, and return iterator pointing to next item, or the end of the vector if no items remain. (Not ArrayVector.)
	/// @remarks It's usually faster to swap with the last element and use PopBack.
	Iterator Erase(Iterator i)
	{
		StructureType::EraseByIndex((U32)(&(*i) - First())); 
		if (this->Size() > 0)
			return i;
		else
			return End();
	}

	// get pointers
	FORCE_INLINE PointerType First() {if (this->Size() > 0) return &StructureType::Element(0); else return 0;} // points to first item
	FORCE_INLINE ConstPointerType First() const {if (this->Size() > 0) return &StructureType::Element(0); else return 0;}
};

//////////////////////////////////////////////////////////////////
// functions that operate on vectors

/// Remove elements that equal the input from the vector.
template <class T, class P, class E>
inline void Remove(Vector<T, P>& s, E element)
{
	Vector<T, P> newVector;

	T copy = element; // may be a reference to an item in the vector

	for (typename Vector<T, P>::Iterator i = s.Begin(); i != s.End(); ++i)
		if (*i != copy)
			newVector.PushBack(*i);

	Swap(newVector, s);
}

/// Faster version of remove that doesn't maintain the order of the elements.
template <class T, class P, class E>
inline void RemoveNonOrdered(Vector<T, P>& s, E element)
{
	typename Vector<T, P>::Iterator i = s.Begin();

	E copy = element; // may be a reference to an item in the vector

	while (i != s.End())
	{
		if (*i == copy)
		{
			if (s.Size() > 1)
			{
				Swap(*i, s.Back());
				s.PopBack();
			}
			else
			{
				s.Clear();
				break;
			}
		}
		else
		{
			++i;
		}
	};
}

/// Returns the index of the first item in the vector equal to e 
/// or the size of the vector if no items were equal to e.
template <class T, class P, class E>
inline int FindIndex(Vector<T, P>& v, const E& e)
{
	for (int i = 0; i < v.Size(); i++)
	{
		if (v[i] == e)
			return i;
	}

	return v.Size();
}

///////////////////////////////////////////////////////////////////

/// Swap the two vectors.
template <class T, class P>
inline void Swap(Vector<T, P>& left, Vector<T, P>& right)
{
	SwapByMemcopy(left, right);
}

template <class T, class P>
void Move(void* pDst, Vector<T, P>* pSrc) 
{
	MoveByMemcopy<Vector<T, P> >::Move(pDst, pSrc);
}

template <class T, class P>
void MoveArray(void* pDst, Vector<T, P>* pSrc, int n) 
{
	MoveByMemcopy<Vector<T, P> >::MoveArray(pDst, pSrc, n);
}

template <class T, class P>
inline bool operator < (const Vector<T, P>& left, const Vector<T, P>& right)
{
	if (left.Size() < right.Size()) return true;
	if (left.Size() > right.Size()) return false;

	for (int i = 0; i < left.Size(); i++)
	{
		if (left[i] < right[i]) return true;
		if (right[i] < left[i]) return false;
	}

	return false;
}

/// @}

}

#if PLATFORM_SPU

namespace Spu {void* Fetch(void* dest, const void* src, unsigned size);}
namespace Spu {void* FetchAsync(void* dest, const void* src, unsigned size, unsigned tag);}

namespace Btl
{

template <typename T, typename P>
inline void SpuFetch(Btl::Vector<T, P>& v)
{
	if (!v.Empty())
	{
		U32 size = v.Size();
		U32 sizeInBytes = size * sizeof(T);
		void* pData = v.First();

		new (&v) Btl::Vector<T>();
		v.InitialiseSize(size);

		Spu::Fetch(v.First(), pData, sizeInBytes);
	}
}

template <typename T, typename P>
inline void SpuFetchAsync(Btl::Vector<T, P>& v, unsigned tag)
{
	if (!v.Empty())
	{
		U32 size = v.Size();
		U32 sizeInBytes = size * sizeof(T);
		void* pData = v.First();

		new (&v) Btl::Vector<T>();
		v.InitialiseSize(size);

		Spu::FetchAsync(v.First(), pData, sizeInBytes, tag);
	}
}

}

#else

namespace Btl
{

template<typename T>
inline void SpuFetch(Btl::Vector<T> &v) {}

}

#endif

#endif
