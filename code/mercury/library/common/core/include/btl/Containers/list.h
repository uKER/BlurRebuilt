#ifndef BTL_LIST_H
#define BTL_LIST_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Linked List
//////////////////////////////////////////////////////

#include <btl/allocators/allocator.h>
#include <btl/containers/sequence.h>
#include <core/utility/coredebug.h>

/////////////////////////////////////////////////////////

namespace Btl 
{

/// @addtogroup Containers
/// @{

////////////////////////////////////////////////////////
// ListElement: Single or double linked.

// Note: The ListElement (LE_) functions have to be public for List to access them.
// Other code does not need to call any of these functions.
/// Singly-linked list element.
/// Objects derived from this can be stored in List.
/// They should have a Destroy() member function, which is called when the element is removed from the list.
template <typename T>
class ListElementSingle
{
public:
	ListElementSingle() {}

	void ListInsert() {}
	void Destroy() {}

	// ********************************************************
	// *** DON'T USE ANY OF THE LE_ FUNCTIONS (List only) ***
	// ********************************************************
	void LE_Clear() 
	{
		LE_next = 0;
		LE_Insert();
	}

	T* LE_Next() 
	{
		return LE_next;
	}

	void LE_InsertAfter(T& item) 
	{
		item.LE_next = LE_next; 
		LE_next = &item;
		item.LE_Insert();
	}

	T* LE_PopFront() 
	{
		T* temp = LE_next; 
		LE_Remove(); 
		return temp;
	}

	void LE_EraseNext() 
	{
		T* temp = LE_next; 
		LE_next = temp->LE_next; 
		temp->LE_Remove();
	}

	void LE_PushFront(T& item) 
	{
		item.LE_next = LE_Current();
		item.LE_Insert();
	}

	void LE_PushBack(T& item) 
	{
		LE_next = &item;
		item.LE_next = 0;
		item.LE_Insert();
	}

	const T* LE_Current() const {return static_cast<const T*>(this);} // get pointer to derived class
	T* LE_Current() {return static_cast<T*>(this);} // get pointer to derived class

private:
	void LE_Insert() {LE_Current()->ListInsert();} // called on elements added to the list
	void LE_Remove() {LE_Current()->Destroy();} // called on elements removed from the list
	T* LE_next;
};

///////////////////////////////////////////////////////////

/// Double-linked list element.
/// Objects derived from this can be stored in List
/// They should have a Destroy() member function, which is called when the element is removed from the list.
/// This type allows more functionality than ListElementSingle, but has to maintain an extra pointer.
template <typename T>
class ListElementDouble
{
public:
	ListElementDouble() {}
	virtual ~ListElementDouble()
	{
	}

	void ListInsert() {}
	void Destroy() {}

	// ********************************************************
	// *** DON'T USE ANY OF THE LE_ FUNCTIONS (List only) ***
	// ********************************************************
	void LE_Clear() 
	{
		LE_next = 0;
		LE_previous = 0;
		LE_Insert();
	}

	T* LE_Next()
	{
		return LE_next;
	}

	void LE_InsertAfter(T& item)
	{
		item.LE_next = LE_next;
		item.LE_previous = LE_Current();
		if (LE_next) LE_next->LE_previous = &item;
		LE_next = &item;
		item.LE_Insert();
	}

	T* LE_PopFront()
	{
		if (LE_next) LE_next->LE_previous = 0;
		T* temp = LE_next;
		LE_Remove();
		return temp;
	}

	void LE_EraseNext()
	{
		T* temp = LE_next; 
		LE_next = temp->LE_next;
		if (LE_next) LE_next->LE_previous = LE_Current();
		temp->LE_Remove();
	}

	void LE_PushFront(T& item)
	{
		item.LE_next = LE_Current();
		item.LE_previous = 0;
		LE_previous = &item;
		item.LE_Insert();
	}

	void LE_PushBack(T& item)
	{
		item.LE_previous = LE_Current();
		item.LE_next = 0;
		LE_next = &item;
		item.LE_Insert();
	}

	// extra functions for ListElementDouble
	T* LE_Previous()
	{
		return LE_previous;
	}

	void LE_InsertBefore(T& item)
	{
		item.LE_previous = LE_previous;
		item.LE_next = LE_Current();
		if (LE_previous) LE_previous->LE_next = &item;
		LE_previous = &item;
		item.LE_Insert();
	}

	T* LE_PopBack()
	{
		if (LE_previous) LE_previous->LE_next = 0;
		T* temp = LE_previous;
		LE_Remove();
		return temp;
	}
	
	void LE_Erase()
	{
		if (LE_previous) LE_previous->LE_next = LE_next;
		if (LE_next) LE_next->LE_previous = LE_previous;
		LE_Remove();
	}

	void LE_ErasePrevious()
	{
		T* temp = LE_previous; 
		LE_previous = temp->LE_previous;
		if (LE_previous) LE_previous->LE_next = LE_Current();
		temp->LE_Remove();
	}

	// swap with another element which must not already be in the list
	void LE_Swap(ListElementDouble& other)
	{		
		other.LE_next = LE_next;
		other.LE_previous = LE_previous;

		if (LE_next) LE_next->LE_previous = other.LE_Current();
		if (LE_previous) LE_previous->LE_next = other.LE_Current();
		LE_Remove();
		other.LE_Insert();
	}

	void LE_Append(T& item)
	{
		item.LE_previous = LE_Current();
		LE_next = &item;
		item.LE_Insert();
	}

	void LE_Reverse()
	{
		Swap(LE_next, LE_previous);
	}

	void LE_SetNext(T* p)
	{
		LE_next = p;
	}

	void LE_SetPrevious(T* p)
	{
		LE_previous = p;
	}

	const T* LE_Current() const {return static_cast<const T*>(this);}
	T* LE_Current() {return static_cast<T*>(this);}

private:
	void LE_Insert() {LE_Current()->ListInsert();} // called on elements added to the list
	void LE_Remove() {LE_Current()->Destroy();}
	T* LE_next;
	T* LE_previous;
};

//////////////////////////////////////////////////////
// List Structure: single or double ended.

// Single ended list.
/// Structure policy class for List.
/// Holds a pointer to the start of the list, so methods that access the front of 
/// the list are available but not those that access the back.
template <typename T>
class SingleEndedList
{
public:
	SingleEndedList() : m_first(0) {}

	const T& Front() const {BTL_ASSERT(m_first != 0); return *m_first;} 
	T& Front() {BTL_ASSERT(m_first != 0); return *m_first;} 
	const T* First() const {return m_first;}
	T* First() {return m_first;}

	void SetFirst(T* item) {m_first = item;}
	void SetFirstIfEqual(T* item, const T* compare)  {if (m_first == compare) m_first = item;}

	// okay to call SetLast (just does nothing) but no function to get it back
	void SetLast(T* item) {} 
	void SetLastIfEqual(T* item, const T* compare) {}

	void Swap(T* i1, T* i2)
	{
		// see if pointers need to change
		if (i1 == m_first)
			m_first = i2;
		else if (i2 == m_first)
			m_first = i1;
	}

private:
	T* m_first;
};

// Double ended list.
/// Structure policy class for List.
/// Stores pointer to first and last elements.
/// This allows the list to act as a queue.
template <typename T>
class DoubleEndedList
{
public:
	DoubleEndedList() : m_first(0), m_last(0) {}

	T& Front() {BTL_ASSERT(m_first != 0); return *m_first;}
	const T& Front() const {BTL_ASSERT(m_first != 0); return *m_first;}
	T& Back() {BTL_ASSERT(m_last != 0); return *m_last;}
	const T& Back() const {BTL_ASSERT(m_last != 0); return *m_last;}

	const T* First() const {return m_first;}
	const T* Last() const {return m_last;}
	T* First() {return m_first;}
	T* Last() {return m_last;}

	void SetFirst(T* item) {m_first = item;}
	void SetFirstIfEqual(T* item, const T* compare) {if (m_first == compare) m_first = item;}

	void SetLast(T* item) {m_last = item;}
	void SetLastIfEqual(T* item, const T* compare) {if (m_last == compare) m_last = item;}

	void Swap(T* i1, T* i2)
	{
		// see if pointers need to change
		if ((i1 == m_first && i2 == m_last) || (i2 == m_first && i1 == m_last))
		{
			T* t = m_first;
			m_first = m_last;
			m_last = t;
		}
		else if (i1 == m_first)
			m_first = i2;
		else if (i2 == m_first)
			m_first = i1;
		else if (i1 == m_last)
			m_last = i2;
		else if (i2 == m_last)
			m_last = i1;
	}

	void Reverse()
	{
		Swap(m_first, m_last);
	}

private:
	T* m_first;
	T* m_last;
};

//////////////////////////////////////////////////////

template <class T, template <typename U> class ListElementType>
class ListWrapper : public ListElementType<ListWrapper<T, ListElementType> >
{
public:
	ListWrapper() {}
	explicit ListWrapper(const T& data) : m_element(data) {}
	explicit ListWrapper(T& data) : m_element(data) {}

	T* LE_Current() {return &m_element;}
	const T* LE_Current() const {return &m_element;}
	void Destroy() {}

	T m_element;
};

template <class T, class StoredType>
class ReferenceInitialiser;

class SingleThreaded;

#if !PLATFORM_SPU
template <class T = NullType, typename AllocatorType = ArrayAllocator<U8>, class ThreadingModel = SingleThreaded>
#else
template <class T = NullType, typename AllocatorType = LinearArrayAllocator<U8>, class ThreadingModel = SingleThreaded>
#endif
class DynamicPool;

template <class T, class StoredType, typename AllocatorType = DynamicPool<StoredType> >
class CopyInitialiser;

////////////////////////////////////////////////////////////////////////////////
// Parameters

template <typename T>
struct AutoInternal
{
	const static bool result = IsConvertible<T, ListElementDouble<T> >::exists || IsConvertible<T, ListElementSingle<T> >::exists;
};

template <typename T>
struct Internal
{
	const static bool result = true;
};

template <typename T>
struct External
{
	 const static bool result = false;
};

/// Parameters for linked list class.
/// @param Structure SingleEndedList or DoubleEndedList
/// @param Counter MaintainedCounter or OnDemandCounter
/// @param ListElementType ListElementSingle or ListElementDouble (ignored for internal links).
/// @param Allocator The type of allocator used by the list.
template <	template <typename U> class Structure = DoubleEndedList,
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble,
#if !PLATFORM_SPU
			typename Allocator = StandardAllocator<NullType>,
#else
			typename Allocator = LinearAllocator<NullType>,
#endif
			template <typename U> class Internal_t = AutoInternal
			>
struct ListParameters
{
	template <typename T>
	struct InternalType
	{
		const static bool result = Internal_t<T>::result;
	};

	template<typename T>
	struct StoredType
	{
		typedef typename Select<InternalType<T>::result, T, ListWrapper<T, ListElementType> >::Result Result;
	};

	template<typename T>
	struct Initialiser
	{
		typedef typename StoredType<T>::Result StoredTypeResult;
		typedef ReferenceInitialiser<T, StoredTypeResult> ReferenceInitialiserType;
		typedef typename Allocator::template Rebind<StoredTypeResult>::Other AllocatorType;
		typedef CopyInitialiser<T, StoredTypeResult, AllocatorType> CopyInitialiserType;
		typedef typename Select<InternalType<T>::result, 
								ReferenceInitialiserType, 
								CopyInitialiserType>::Result Result;
	};

	template<typename T>
	struct ListStructure
	{
		typedef Structure<typename StoredType<T>::Result> Result;
	};

	typedef Counter ListCounter;
};

/// Parameters for linked list class.
/// @param Structure SingleEndedList or DoubleEndedList
/// @param Counter MaintainedCounter or OnDemandCounter
template <	template <typename U> class Structure = DoubleEndedList,
			class Counter = MaintainedCounter
			>
struct ListParameters_Internal : public ListParameters<Structure, Counter, ListElementDouble, 
#if !PLATFORM_SPU
			StandardAllocator<NullType>,
#else
			LinearAllocator<NullType>,
#endif
			Internal>
{
};

/// Parameters for linked list class.
/// @param Structure SingleEndedList or DoubleEndedList
/// @param Counter MaintainedCounter or OnDemandCounter
/// @param ListElementType ListElementSingle or ListElementDouble
/// @param Allocator The type of allocator used by the list.
template <	template <typename U> class Structure = DoubleEndedList,
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble,
#if !PLATFORM_SPU
			typename Allocator = StandardAllocator<NullType>
#else
			typename Allocator = LinearAllocator<NullType>
#endif
			>
struct ListParameters_External : public ListParameters<Structure, Counter, ListElementType, Allocator, External>
{
};

/// Parameters for linked list class.
/// @param Structure SingleEndedList or DoubleEndedList
/// @param Counter MaintainedCounter or OnDemandCounter
/// @param ListElementType ListElementSingle or ListElementDouble
/// @remarks Uses a pool allocator.
template <	template <typename U> class Structure = DoubleEndedList,
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble
			>
struct ListParameters_Pool : public ListParameters<Structure, Counter, ListElementType, DynamicPool<NullType> >
{
};

//////////////////////////////////////////////////////
/// Linked list class.
///                                                   
/// Choice of: @n
/// @li single linked (ListElementSingle) or double linked (ListElementDouble) elements, @n
/// @li stack behaviour (SingleEndedList) or queue (DoubleEndedList), @n
/// @li list length maintained (MaintainedCounter) or calculated on demand (OnDemandCounter), @n
/// @li internal or external links. @n@n
///
/// Elements of lists with internal links should derive from ListElementSingle or ListElementDouble,
/// and define void Destroy() which is called when an element is erased from the list. 
/// Using internal links means that the object is stored outside the list, and the list changes pointers in the object. 
/// The list just links the elements; it does not copy them or allocate memory.
/// It doesn't release memory when elements are removed either; this should be done in Destroy() if necessary.
/// Items can only be in the list once. Putting the same item in the list twice or in more than one list 
/// will cause undefined behaviour.
///
/// External lists copy elements when they are added and allocate memory to store them. 
/// This type of list is created if the elements do not derive from either of the ListElement types. @n@n
///                                                  
/// Use typedefs to create custom list types, e.g. 
///
/// @code   
///
/// class MyClass : public ListElementDouble<MyClass> 
/// {                                                 
/// public:                                           
///	  void Destroy() {}                               
/// };
///
/// %List<MyClass> myList; // default is DoubleEndedList, OnDemandCounter
/// %List<int> myList2; // an external list
///
/// @endcode
///
/// @param T The type stored in the list.
/// @param Parameters See @ref ListParameters.
/// @remarks
/// For efficiency, doing something that doesn't make sense, e.g. calling PopFront
/// on an empty list, is undefined (but will assert).
/// To declare a %List of an undefined type
/// use ListParameters_Internal or ListParameters_External.
template <typename T, typename Parameters = ListParameters<> >
class List
{
public:
	class ConstIterator;
	class Iterator;

private:
	typedef List<T, Parameters> This;

	typedef T ReferenceType;
	typedef typename Parameters::template StoredType<T>::Result StoredType;
	typedef typename Parameters::template Initialiser<T>::Result Initialiser;

	typedef typename Parameters::template ListStructure<T>::Result ListStructure;
	typedef typename Parameters::ListCounter ListCounter;

	const static bool ownsItems = !Parameters::template InternalType<T>::result;

	static T* Current(void* p) {if (p == 0) return 0; else return ((StoredType*) p)->LE_Current();}
	static void* Next(void* p) {return ((StoredType*) p)->LE_Next();}
	static void* Previous(void* p) {return ((StoredType*) p)->LE_Previous();}

	static const T* Current(const void* p) {if (p == 0) return 0; else return ((StoredType*) p)->LE_Current();}
	static const void* Next(const void* p) {return ((StoredType*) p)->LE_Next();}
	static const void* Previous(const void* p) {return ((StoredType*) p)->LE_Previous();}

public:
	/// %Iterator for list elements
	class ConstIterator
	{
	public:
		// constructors
		ConstIterator() {}
		ConstIterator(const ConstIterator& c) : m_pointer(c.m_pointer) {}
		ConstIterator& operator = (const ConstIterator& c) {m_pointer = c.m_pointer; return *this;} 

		// comparison
		bool operator == (const ConstIterator& c) const {return m_pointer == c.m_pointer;} ///< @a
		bool operator == (const T* p) const {return Current(m_pointer) == p;} ///< @a
		bool operator != (const ConstIterator& c) const {return m_pointer != c.m_pointer;} ///< @a
		bool operator != (const T* p) const {return Current(m_pointer) != p;} ///< @a
		bool operator < (const ConstIterator& c) const {return m_pointer < c.m_pointer;} ///< @a

		// return T
		const T& operator*() const {BTL_ASSERT(m_pointer); return *Current(m_pointer);} ///< @a
		const T* operator->() const {BTL_ASSERT(m_pointer); return Current(m_pointer);} ///< @a

		const void* Pointer() const {return m_pointer;}
		
		// only define pre-increment as post-increment can be inefficient
		ConstIterator& operator++() {BTL_ASSERT(m_pointer); m_pointer = Next(m_pointer); return *this;} ///< @a
		ConstIterator& operator--() {BTL_ASSERT(m_pointer); m_pointer = Previous(m_pointer); return *this;}  ///< Only with ListElementDouble.
		ConstIterator operator + (int n) const {ConstIterator p(*this); for (int i = 0; i < n; i++) ++p; return p;} ///< @a
		ConstIterator operator - (int n) const {ConstIterator p(*this); for (int i = 0; i < n; i++) --p; return p;} ///< @a

		explicit ConstIterator(const void* p) {m_pointer = const_cast<void*>(p);}

	protected:
		void* m_pointer;
	};

	/// %Iterator for list elements
	class Iterator : public ConstIterator
	{
	public:
		using ConstIterator::m_pointer;

		// constructors
		Iterator() {}
		Iterator(const Iterator& c) : ConstIterator(c) {}
		Iterator& operator = (const Iterator c) {m_pointer = c.m_pointer; return *this;}

		// return T
		using ConstIterator::operator *;
		using ConstIterator::operator ->;
		T& operator*() {BTL_ASSERT(m_pointer); return *Current(m_pointer);} ///< @a
		T* operator->() {BTL_ASSERT(m_pointer); return Current(m_pointer);} ///< @a

		void* Pointer() {return m_pointer;}

		Iterator& operator++() {BTL_ASSERT(m_pointer); m_pointer = Next(m_pointer); return *this;} ///< @a
		Iterator& operator--() {BTL_ASSERT(m_pointer); m_pointer = Previous(m_pointer); return *this;} ///< Only with ListElementDouble.
		Iterator operator + (int n) const {Iterator p(*this); for (int i = 0; i < n; ++i) ++p; return p;} ///< @a
		Iterator operator - (int n) const {Iterator p(*this); for (int i = 0; i < n; ++i) --p; return p;} ///< @a

		explicit Iterator(void* p) {m_pointer = p;}
	};

	typedef ConstReverseIterator_t<T, This, void*> ConstReverseIterator; ///< Reverse Iterator.
	typedef ReverseIterator_t<T, This, void*> ReverseIterator; ///< Reverse Iterator.

	List() {} ///< Create an empty list.

public:
	~List() {Clear();} ///< Free all elements.

	/// Destroy list without freeing elements.
	/// Useful if you don't need to free the elements in the destructor.
	void Abandon() {structure.SetFirst(0); structure.SetLast(0);} 

	Iterator Begin() {return Iterator(structure.First());} ///< %Iterator pointing to the first element in the list.
	ConstIterator Begin() const {return ConstIterator(structure.First());} ///< %Iterator pointing to the first element in the list.
	Iterator End() {return Iterator(0);} ///< %Iterator representing the end of the list.
	ConstIterator End() const {return ConstIterator(0);} ///< %Iterator representing the end of the list.

	// reverse iterators
	ReverseIterator ReverseBegin() {return ReverseIterator(structure.Last());} ///< Reverse iterator pointing to the last element in the list.
	ConstReverseIterator ReverseBegin() const {return ConstReverseIterator(structure.Last());} ///< Reverse iterator pointing to the last element in the list.
	ReverseIterator ReverseEnd() {return ReverseIterator(0);} ///< Reverse iterator representing the end of the list.
	ConstReverseIterator ReverseEnd() const {return ConstReverseIterator(0);} ///< Reverse iterator representing the end of the list.

	T& Front() {return *structure.Front().LE_Current();} ///< Access the first element in the list.
	const T& Front() const {return *structure.Front().LE_Current();} ///< Access the first element in the list.
	T& Back() {return *structure.Back().LE_Current();} ///< Access the last element in the list.
	const T& Back() const {return *structure.Back().LE_Current();} ///< Access the last element in the list.

	/// Return the number of elements in the list.
	/// This may be a slow operation if using OnDemandCounter.
	/// @see Empty
	int Size() const {return counter.Count(*this);}

	/// Return true if the list is empty.
	bool Empty() const {return structure.First() == 0;}
	
	// list manipulation
	void PushFront(T& item); ///< Insert item at the beginning of the list.
	void PushFront(const T& item); ///< Insert item at the beginning of the list.
	void PushFront(); ///< Create item at the beginning of the list.
	void PushBack(T& item); ///< Insert item at the end of the list. @remarks Only with DoubleEndedList.
	void PushBack(const T& item); ///< Insert item at the end of the list. @remarks Only with DoubleEndedList.
	void PushBack(); ///< Create item at the end of the list. @remarks Only with DoubleEndedList.
	void PopFront(); ///< Erase item from the front of the list.
	void PopBack(); ///< Erase item from the end of the list. @remarks Only with DoubleEndedList and ListElementDouble.
	void Clear(); ///< Erase all elements.

	// operations using iterators
	/// Insert item after element pointed to by iterator.
	void InsertAfter(T& item, Iterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 
	void InsertAfter(const T& item, Iterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 

	/// Insert item before element pointed to by iterator. @remarks Only with ListElementDouble.
	void InsertBefore(T& item, Iterator where) {InsertBefore(item, (StoredType*) where.Pointer());} 
	void InsertBefore(const T& item, Iterator where) {InsertBefore(item, (StoredType*) where.Pointer());} 

	/// Erase element after element pointed to by iterator.
	void EraseNext(Iterator where) {EraseNext((StoredType*) where.Pointer());}
	/// Erase element pointed to by iterator, and return iterator to next item, or null if it was the last item. @remarks Only with ListElementDouble.
	Iterator Erase(Iterator where) {Iterator t = where; ++t; Erase((StoredType*) where.Pointer()); return t;}
	/// Erase element before element pointed to by iterator. @remarks Only with ListElementDouble.
	void ErasePrevious(Iterator where) {ErasePrevious((StoredType*) where.Pointer());}
	/// Erase for single linked list (slow)
	void EraseSingleLinked(Iterator where);
	/// Swap elements pointed to by iterators @remarks Only with ListElementDouble.
	void Swap(Iterator i1, Iterator i2); 

	// operations with reverse iterators
	void InsertAfter(T& item, ReverseIterator where) {InsertBefore(item, (StoredType*) where.Pointer());} 
	void InsertAfter(const T& item, ReverseIterator where) {InsertBefore(item, (StoredType*) where.Pointer());} 

	void InsertBefore(T& item, ReverseIterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 
	void InsertBefore(const T& item, ReverseIterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 

	void EraseNext(ReverseIterator where) {ErasePrevious((StoredType*) where.Pointer());}
	ReverseIterator Erase(ReverseIterator where) {ReverseIterator t = where; ++t; Erase((StoredType*) where.Pointer()); return t;}
	void ErasePrevious(ReverseIterator where) {EraseNext((StoredType*) where.Pointer());}
	void Swap(ReverseIterator i1, ReverseIterator i2) {Swap(Iterator(i1.Pointer()), Iterator(i2.Pointer()));}

	void Reverse(); ///< Reverse the order of the list. @remarks Only with DoubleEndedList and ListElementDouble.
	
	/// Sort using < operator. 
	/// @remarks Only with DoubleEndedList and ListElementDouble. 
	/// Uses in-place merge sort which is O(N log N).
	void Sort(); 

	/// Swap with another list.
	void Swap(List& other)
	{
		Btl::SwapByMemcopy(structure, other.structure);
		Btl::SwapByMemcopy(counter, other.counter);
		Btl::SwapByMemcopy(initialiser, other.initialiser);
	}

	List(const List& other)
	{
		BIZ_STATIC_ASSERT(ownsItems);
		CopyElements(other);
	}

	void operator = (const List& other)
	{
		BIZ_STATIC_ASSERT(ownsItems);

		if (this != &other)
		{
			Clear();
			CopyElements(other);
		}
	}

private:
	void InsertAfter(T& item, StoredType* where); 
	void InsertAfter(const T& item, StoredType* where); 
	void InsertBefore(T& item, StoredType* where); 
	void InsertBefore(const T& item, StoredType* where); 
	void EraseNext(StoredType* where);
	void Erase(StoredType* where); 
	void ErasePrevious(StoredType* where);

	// to do: these can't be public at the moment because they don't work
	// for lists using an internal pool

	// Move item from the beginning of another list to the end of this list.
	void MoveFrontToBack(List& other);

	// Move all items from the other list onto the end of this list.
	void Append(List& other);

	// Move items from the other list onto the end of this list.
	void Append(List& other, StoredType* first, StoredType* last, int count);

	// Merge items from another list into this list.
	// If both lists are sorted, the merged list is also sorted.
	void Merge(List& other);

	void CopyElements(const List& other)
	{
		ConstIterator end = other.End();
		for (ConstIterator it = other.Begin(); it != end; ++it)
			PushBack(*it);
	}

	ListStructure structure;
	ListCounter counter;
	Initialiser initialiser;
};

/// @} end group

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "list.inl"

/// Swap two lists
template <typename T, typename Parameters>
inline void Swap(List<T, Parameters>& left, List<T, Parameters>& right)
{
	left.Swap(right);
}

template <typename T, typename Parameters>
void Move(void* pDst, List<T, Parameters>* pSrc) 
{
	MoveByMemcopy<List<T, Parameters> >::Move(pDst, pSrc);
}

template <typename T, typename Parameters>
void MoveArray(void* pDst, List<T, Parameters>* pSrc, int n) 
{
	MoveByMemcopy<List<T, Parameters> >::MoveArray(pDst, pSrc, n);
}

////////////////////////////////////////////////////////////////////////

}

#include "btl/containers/initialiser.h"

#endif
