#ifndef __UT_LIST_H__
#define __UT_LIST_H__

//////////////////////////////////////////////////////
// Linked List
//////////////////////////////////////////////////////

#pragma once

#include <core/allocators/allocator.h>
#include <core/containers/sequence.h>
#include <core/utility/coredebug.h>

/////////////////////////////////////////////////////////

namespace Ut 
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
	// *** DON'T USE ANY OF THE LE_ FUNCTIONS (UtList only) ***
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
	// *** DON'T USE ANY OF THE LE_ FUNCTIONS (UtList only) ***
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

	T& Front() {assert(m_first != 0); return *m_first;} 
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

	T& Front() {assert(m_first != 0); return *m_first;}
	T& Back() {assert(m_last != 0); return *m_last;}

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

	T* LE_Current() {return &m_element;}
	void Destroy() {}

	T m_element;
};

template <class T, class StoredType>
class ReferenceInitialiser;

class SingleThreaded;

template <class T>
class ConstructOnAllocate;

template <class T, typename AllocatorType = ArrayAllocator<U8>, class ThreadingModel = SingleThreaded, template <typename Type> class ConstructionType = ConstructOnAllocate>
class DynamicPool;

template <class T, class StoredType, typename AllocatorType = DynamicPool<StoredType> >
class CopyInitialiser;

/// Base class for linked list.
template <	typename T, 
			template <typename U> class Structure, 
			class Counter,
			template <typename U> class ListElementType,
			bool intrusive>
class UtListBase
{
public:
	class ConstIterator;
	class Iterator;

private:
	typedef UtListBase<T, Structure, Counter, ListElementType, intrusive> This;

	//const static bool intrusive = IsConvertible<T, ListElementDouble<T> >::exists || IsConvertible<T, ListElementSingle<T> >::exists;
	typedef typename Select<intrusive, T, ListWrapper<T, ListElementType> >::Result StoredType;
	typedef typename Select<intrusive, ReferenceInitialiser<T, StoredType>, CopyInitialiser<T, StoredType> >::Result Initialiser;

	typedef Structure<StoredType> ListStructure;
	typedef Counter ListCounter;

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

		// return T
		const T& operator*() const {assert(m_pointer); return *Current(m_pointer);} ///< @a
		const T* operator->() const {assert(m_pointer); return Current(m_pointer);} ///< @a

		const void* Pointer() const {return m_pointer;}
		
		// only define pre-increment as post-increment can be inefficient
		ConstIterator& operator++() {assert(m_pointer); m_pointer = Next(m_pointer); return *this;} ///< @a
		ConstIterator& operator--() {assert(m_pointer); m_pointer = Previous(m_pointer); return *this;}  ///< Only with ListElementDouble.
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
		T& operator*() {assert(m_pointer); return *Current(m_pointer);} ///< @a
		T* operator->() {assert(m_pointer); return Current(m_pointer);} ///< @a

		void* Pointer() {return m_pointer;}

		Iterator& operator++() {assert(m_pointer); m_pointer = Next(m_pointer); return *this;} ///< @a
		Iterator& operator--() {assert(m_pointer); m_pointer = Previous(m_pointer); return *this;} ///< Only with ListElementDouble.
		Iterator operator + (int n) const {Iterator p(*this); for (int i = 0; i < n; i++) p++; return p;} ///< @a
		Iterator operator - (int n) const {Iterator p(*this); for (int i = 0; i < n; i++) p--; return p;} ///< @a

		explicit Iterator(void* p) {m_pointer = p;}
	};

	typedef ConstReverseIterator_t<T, This, void*> ConstReverseIterator; ///< Reverse Iterator.
	typedef ReverseIterator_t<T, This, void*> ReverseIterator; ///< Reverse Iterator.

protected:
	UtListBase() {} ///< Create an empty list.

public:
	~UtListBase() {Clear();} ///< Free all elements.

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
	T& Back() {return *structure.Back().LE_Current();} ///< Access the last element in the list.

	/// Return the number of elements in the list.
	/// This may be a slow operation if using OnDemandCounter.
	/// @see Empty
	int Size() const {return counter.Count(*this);}

	/// Return true if the list is empty.
	bool Empty() const {return structure.First() == 0;}
	
	// list manipulation
	void PushFront(T& item); ///< Insert item at the beginning of the list.
	void PushFront(); ///< Create item at the beginning of the list.
	void PushBack(T& item); ///< Insert item at the end of the list. @remarks Only with DoubleEndedList.
	void PushBack(); ///< Create item at the end of the list. @remarks Only with DoubleEndedList.
	void PopFront(); ///< Erase item from the front of the list.
	void PopBack(); ///< Erase item from the end of the list. @remarks Only with DoubleEndedList and ListElementDouble.
	void Clear(); ///< Erase all elements.

	// operations using iterators
	/// Insert item after element pointed to by iterator.
	void InsertAfter(T& item, Iterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 
	/// Insert item before element pointed to by iterator. @remarks Only with ListElementDouble.
	void InsertBefore(T& item, Iterator where) {InsertBefore(item, (StoredType*) where.Pointer());}  
	/// Erase element after element pointed to by iterator.
	void EraseNext(Iterator where) {ErasePrevious((StoredType*) where.Pointer());}
	/// Erase element pointed to by iterator, and return iterator to next item, or null if it was the last item. @remarks Only with ListElementDouble.
	Iterator Erase(Iterator where) {Iterator t = where; ++t; Erase((StoredType*) where.Pointer()); return t;}
	/// Erase element before element pointed to by iterator. @remarks Only with ListElementDouble.
	void ErasePrevious(Iterator where) {EraseNext((StoredType*) where.Pointer());}
	/// Swap elements pointed to by iterators @remarks Only with ListElementDouble.
	void Swap(Iterator i1, Iterator i2); 

	// operations with reverse iterators
	void InsertAfter(T& item, ReverseIterator where) {InsertBefore(item, (StoredType*) where.Pointer());} 
	void InsertBefore(T& item, ReverseIterator where) {InsertAfter(item, (StoredType*) where.Pointer());} 
	void EraseNext(ReverseIterator where) {ErasePrevious((StoredType*) where.Pointer());}
	ReverseIterator Erase(ReverseIterator where) {ReverseIterator t = where; ++t; Erase((StoredType*) where.Pointer()); return t;}
	void ErasePrevious(ReverseIterator where) {EraseNext((StoredType*) where.Pointer());}
	void Swap(ReverseIterator i1, ReverseIterator i2) {Swap(Iterator(i1.Pointer()), Iterator(i2.Pointer()));}

private:
	void InsertAfter(T& item, StoredType* where); 
	void InsertBefore(T& item, StoredType* where); 
	void EraseNext(StoredType* where);
	void Erase(StoredType* where); 
	void ErasePrevious(StoredType* where); 

	UtListBase(This& other); // copy protection
	void operator = (This& other); // copy protection

	ListStructure structure;
	ListCounter counter;
	Initialiser initialiser;
};

/// Version of UtList which is forced to be non-intrusive.
template <	typename T, 
			template <typename U> class Structure = DoubleEndedList, 
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble>
class UtListNonIntrusive : public UtListBase<T, Structure, Counter, ListElementType, false>
{
};

/// Version of UtList which is forced to be intrusive.
template <	typename T, 
			template <typename U> class Structure = DoubleEndedList, 
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble>
class UtListIntrusive : public UtListBase<T, Structure, Counter, ListElementType, true>
{
};

//////////////////////////////////////////////////////
/// Linked list class.
///                                                   
/// Choice of: @n
/// @li single linked (ListElementSingle) or double linked (ListElementDouble) elements, @n
/// @li stack behaviour (SingleEndedList) or queue (DoubleEndedList), @n
/// @li list length maintained (MaintainedCounter) or calculated on demand (OnDemandCounter), @n
/// @li intrusive or non-intrusive list. @n@n
///
/// Elements of intrusive lists should derive from ListElementSingle or ListElementDouble,
/// and define void Destroy() which is called when an element is erased from the list. 
/// Intrusive means that the object is stored outside the list, and the list changes pointers in the object. 
/// The list just links the elements; it does not copy them or allocate memory.
/// It doesn't release memory when elements are removed either; this should be done in Destroy() if necessary.
/// Items can only be in the list once. Putting the same item in the list twice or in more than one list 
/// will cause undefined behaviour.
///
/// Non-intrusive lists copy elements when they are added and allocate memory to store them. 
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
/// typedef UtList<MyClass, SingleEndedList, MaintainedCounter> MyListType1;
/// UtList<MyClass> myList; // default is DoubleEndedList, OnDemandCounter
/// UtList<int> myList2; // a non-intrusive list
///
/// @endcode
///
/// @param T The type stored in the list.
/// @param Structure The type of list structure.
/// @param Counter The type of list counter
/// @param ListElementType Which ListElement type (ListElementSingle or ListElementDouble) to use for a non-intrusive list.
/// @remarks
/// For efficiency, doing something that doesn't make sense, e.g. calling PopFront
/// on an empty list, is undefined.
/// The only error checking is through assert.
/// You can't declare a UtList of an undefined type!
/// Use UtListNonIntrusive or UtListIntrusive instead.
template <	typename T, 
			template <typename U> class Structure = DoubleEndedList, 
			class Counter = MaintainedCounter,
			template <typename U> class ListElementType = ListElementDouble>
class UtList : public UtListBase<T, Structure, Counter, ListElementType, IsConvertible<T, ListElementDouble<T> >::exists || IsConvertible<T, ListElementSingle<T> >::exists>
{
};

/// @}

//////////////////////////////////////////////////////////////
// inline functions
//////////////////////////////////////////////////////////////
	
// add item to the front of the list
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
inline void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PushFront(T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.First() != 0)
		structure.First()->LE_PushFront(i);
	else // empty list
	{ 
		i.LE_Clear();
		structure.SetLast(&i);
	}

	structure.SetFirst(&i);

	counter.AddElement();
}

// add default item to the back of the list (needs double-ended, but not double-linked, non-intrusive only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PushFront()
{
	StoredType& i = initialiser.Initialise();

	if (structure.First() != 0)
		structure.First()->LE_PushFront(i);
	else // empty list
	{ 
		i.LE_Clear();
		structure.SetLast(&i);
	}

	structure.SetFirst(&i);

	counter.AddElement();
}

// add item to the back of the list (needs double-ended, but not double-linked)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PushBack(T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(i);
	else // empty list
	{
		i.LE_Clear();
		structure.SetFirst(&i);
	}

	structure.SetLast(&i);

	counter.AddElement();
}

// add item to the back of the list (needs double-ended, but not double-linked, non-intrusive only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PushBack()
{
	StoredType& i = initialiser.Initialise();

	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(i);
	else // empty list
	{
		i.LE_Clear();
		structure.SetFirst(&i);
	}

	structure.SetLast(&i);

	counter.AddElement();
}

// insert item after iterator pointing to list item
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::InsertAfter(T& item, StoredType* where)
{
	assert(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);
	where->LE_InsertAfter(i);

	structure.SetLastIfEqual(&i, where);

	counter.AddElement();
}

// insert item before iterator pointing to list item (double-link only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::InsertBefore(T& item, StoredType* where)
{
	assert(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);
	where->LE_InsertBefore(i);

	structure.SetFirstIfEqual(&i, where);

	counter.AddElement();
}

// erase first element in the list
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PopFront()
{
	assert(structure.First() != 0); // must not be empty list

	StoredType* i = structure.First();
	structure.SetFirst(structure.First()->LE_PopFront());

	if (structure.First() == 0)
		structure.SetLast(0);

	counter.EraseElement();
	initialiser.Release(i);
}

// erase last element in the list (double link and double ended only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::PopBack()
{
	assert(structure.Last() != 0); // must not be empty list

	StoredType* i = structure.Last();
	structure.SetLast(structure.Last()->LE_PopBack());

	if (structure.Last() == 0)
		structure.SetFirst(0);

	counter.EraseElement();
	initialiser.Release(i);
}

// erase element after iterator
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::EraseNext(StoredType* where)
{
	assert(where != 0 && where->LE_Next() != 0); // next must be valid item

	StoredType* i = where->LE_Next();
	structure.SetLastIfEqual(where, where->LE_Next());

	where->LE_EraseNext();

	counter.EraseElement();
	initialiser.Release(i);
}

// erase element before iterator (double-link only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::ErasePrevious(StoredType* where)
{
	assert(where != 0 && where->LE_Previous() != 0); // previous must be valid item

	StoredType* i = where->LE_Previous();
	structure.SetFirstIfEqual(where, where->LE_Previous());

	where->LE_ErasePrevious();

	counter.EraseElement();
	initialiser.Release(i);
}

// erase the entire list
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::Clear()
{
	while (structure.First() != 0)
		PopFront();
}

// erase element pointed to by iterator (double-link only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::Erase(StoredType* where)
{
	assert(where != 0); // must be valid element

	StoredType* i = where;
	structure.SetLastIfEqual(where->LE_Previous(), where);
	structure.SetFirstIfEqual(where->LE_Next(), where);

	where->LE_Erase();

	counter.EraseElement();
	initialiser.Release(i);
}

// swap two elements (double-link only)
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
void UtListBase<T, Structure, Counter, ListElementType, intrusive>::Swap(Iterator i1, Iterator i2)
{
	assert(i1 != 0 && i2 != 0); // must be valid elements

	structure.Swap(i1.Pointer(), i2.Pointer());

	// temporary list element
	ListElementDouble<StoredType> temp;

	i1.Pointer()->LE_Swap(temp);
	i2.Pointer()->LE_Swap(*(i1.Pointer()));
	temp.LE_Swap(*(i2.Pointer()));
}

/// @addtogroup Containers
/// @{

/// Swap the two lists
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType, bool intrusive>
inline void Swap(UtListBase<T, Structure, Counter, ListElementType, intrusive>& left, UtListBase<T, Structure, Counter, ListElementType, intrusive>& right)
{
	SwapRaw(left, right);
}

/// Swap the two lists
template <typename T, template <typename U> class Structure, typename Counter, template <typename U> class ListElementType>
inline void Swap(UtList<T, Structure, Counter, ListElementType>& left, UtList<T, Structure, Counter, ListElementType>& right)
{
	SwapRaw(left, right);
}

/// @}

/// @} end group

} // end Ut

#include <core/containers/initialiser.h>


#endif
