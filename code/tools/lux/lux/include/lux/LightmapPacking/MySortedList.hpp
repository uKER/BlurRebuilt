// a sorted linked list template
#pragma once

#include "utility/sequence.h"

namespace Ut
{


/// Counter policy class for list. 
/// Keeps count as elements are added and removed.
template <typename T>
class MaintainedCounter
{
public:
	MaintainedCounter() : count(0) {}
	void AddElement() {count++;}
	void EraseElement() {count--;}

	// ignores pointer
	int Count(T* first) const {return count;} 

private:
	int count;
};

// Does nothing
template <class T, class StoredType>
class ReferenceInitialiser
{
public:
	StoredType& Initialise(T& item) {return item;}
	void Release(StoredType* item) {}
};

// Copies the elements
template <class T, class StoredType>
class CopyInitialiser
{
public:
	StoredType& Initialise(T& item) 
	{
/*
		StoredType* p = Allocate();
		assert(p);
		p->m_element = item;
		return *p;
*/
		return NULL;
	}

	void Release(StoredType* item)
	{
//		Free(item);
	}
};

/// Counter policy class for list. 
/// Counts list elements when required.
template <typename T>
class OnDemandCounter
{
public:
	// do nothing
	void AddElement() {}
	void EraseElement() {}

	// get count
	int Count(const T* first) const
	{
		int count = 0;
		const T* p = first;

		while (p)
		{
			count++;
			p = const_cast<T*>(p)->LE_Next();
		};

		return count;
	}
};

template <class T, template <typename U> class ListElementType>
class ListWrapper : public ListElementType<ListWrapper<T, ListElementType> >
{
public:
	T* LE_Current() {return &m_element;}
	void Destroy() {}

	T m_element;
};

template <class T, class StoredType>
class ReferenceInitialiser;

template <class T, class StoredType>
class CopyInitialiser;

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

	// initialise if first element added to list
	void LE_Clear() 
	{
		LE_next = 0;
	}

	T* LE_Next() 
	{
		return LE_next;
	}

	void LE_InsertAfter(T& item) 
	{
		item.LE_next = LE_next; 
		LE_next = &item;
	}

	T* LE_PopFront() 
	{
		T* temp = LE_next; 
		LE_Destroy(); 
		return temp;
	}

	void LE_EraseNext() 
	{
		T* temp = LE_next; 
		LE_next = temp->LE_next; 
		temp->LE_Destroy();
	}

	void LE_PushFront(T& item) 
	{
		item.LE_next = LE_Current();
	}

	void LE_PushBack(T& item) 
	{
		LE_next = &item;
		item.LE_next = 0;
	}

	T* LE_Current() {return static_cast<T*>(this);} // get pointer to derived class

private:
	void LE_Destroy() {LE_Current()->Destroy();} // called on elements removed from the list
	T* LE_next;
};


/// Double-linked list element.
/// Objects derived from this can be stored in List
/// They should have a Destroy() member function, which is called when the element is removed from the list.
/// This type allows more functionality than ListElementSingle, but has to maintain an extra pointer.
template <typename T>
class ListElementDouble
{
public:
	ListElementDouble() {}

	// as ListElementSingle
	void LE_Clear() 
	{
		LE_next = 0;
		LE_previous = 0;
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
	}

	T* LE_PopFront()
	{
		if (LE_next) LE_next->LE_previous = 0;
		T* temp = LE_next;
		LE_Destroy();
		return temp;
	}

#ifndef GCC
// JAN: I don't think this compiles - missing TPointer()
	void LE_EraseNext()
	{
		T* temp = LE_next; 
		LE_next = temp->LE_next;
		if (LE_next) LE_next->LE_previous = TPointer();
		temp->LE_Destroy();
	}
#endif

	void LE_PushFront(T& item)
	{
		item.LE_next = LE_Current();
		item.LE_previous = 0;
		LE_previous = &item;
	}

	void LE_PushBack(T& item)
	{
		item.LE_previous = LE_Current();
		item.LE_next = 0;
		LE_next = &item;
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
	}

	T* LE_PopBack()
	{
		if (LE_previous) LE_previous->LE_next = 0;
		T* temp = LE_previous;
		LE_Destroy();
		return temp;
	}
	
	void LE_Erase()
	{
		if (LE_previous) LE_previous->LE_next = LE_next;
		if (LE_next) LE_next->LE_previous = LE_previous;
		LE_Destroy();
	}

#ifndef GCC
// JAN: I don't think this compiles - missing TPointer()
	void LE_ErasePrevious()
	{
		T* temp = LE_previous; 
		LE_previous = temp->LE_previous;
		if (LE_previous) LE_previous->LE_next = TPointer();
		temp->LE_Destroy();
	}
#endif

	// swap with another element which must not already be in the list
	void LE_Swap(ListElementDouble& other)
	{		
		other.LE_next = LE_next;
		other.LE_previous = LE_previous;

		if (LE_next) LE_next->LE_previous = other.LE_Current();
		if (LE_previous) LE_previous->LE_next = other.LE_Current();
	}

	T* LE_Current() {return static_cast<T*>(this);}

private:
	void LE_Destroy() {LE_Current()->Destroy();}
	T* LE_next;
	T* LE_previous;
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


template <	typename T, 
			template <typename U> class Structure = DoubleEndedList, 
			template <typename U> class Counter = OnDemandCounter,
			template <typename U> class ListElementType = ListElementDouble>
class UtList
{
public:
	class ConstIterator;
	class Iterator;

private:
	const static bool intrusive = IsConvertible<T, ListElementDouble<T> >::exists || IsConvertible<T, ListElementSingle<T> >::exists;
	typedef typename Select<UtList::intrusive, T, ListWrapper<T, ListElementType> >::Result StoredType;
	typedef typename Select<UtList::intrusive, ReferenceInitialiser<T, StoredType>, CopyInitialiser<T, StoredType> >::Result Initialiser;

	typedef Structure<StoredType> ListStructure;
	typedef Counter<StoredType> ListCounter;

	static T* Current(void* p) {return ((StoredType*) p)->LE_Current();}
	static void* Next(void* p) {return ((StoredType*) p)->LE_Next();}
	static void* Previous(void* p) {return ((StoredType*) p)->LE_Previous();}

	static const T* Current(const void* p) {return ((StoredType*) p)->LE_Current();}
	static const void* Next(const void* p) {return ((StoredType*) p)->LE_Next();}
	static const void* Previous(const void* p) {return ((StoredType*) p)->LE_Previous();}

#ifndef __GNUC__
	friend ConstIterator;
	friend Iterator;
#endif

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
		bool operator == (const ConstIterator& c) const {return m_pointer == c.m_pointer;}
		bool operator == (const T* p) const {return Current(m_pointer) == p;}
		bool operator != (const ConstIterator& c) const {return m_pointer != c.m_pointer;}
		bool operator != (const T* p) const {return Current(m_pointer) != p;}

		// return T
		const T& operator*() const {assert(m_pointer); return *Current(m_pointer);}
		const T* operator->() const {assert(m_pointer); return Current(m_pointer);}

		const void* Pointer() const {return m_pointer;}
		
		// only define pre-increment as post-increment can be inefficient
		ConstIterator& operator++() {assert(m_pointer); m_pointer = Next(m_pointer); return *this;}
		ConstIterator& operator--() {assert(m_pointer); m_pointer = Previous(m_pointer); return *this;} // only with ListElementDouble
		ConstIterator operator + (int n) const {ConstIterator p(*this); for (int i = 0; i < n; i++) p++; return p;}
		ConstIterator operator - (int n) const {ConstIterator p(*this); for (int i = 0; i < n; i++) p--; return p;}

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
		T& operator*() {assert(m_pointer); return *Current(m_pointer);}
		T* operator->() {assert(m_pointer); return Current(m_pointer);}

		void* Pointer() {return m_pointer;}

		Iterator& operator++() {assert(m_pointer); m_pointer = Next(m_pointer); return *this;}
		Iterator& operator--() {assert(m_pointer); m_pointer = Previous(m_pointer); return *this;} // only with ListElementDouble
		Iterator operator + (int n) const {Iterator p(*this); for (int i = 0; i < n; i++) p++; return p;}
		Iterator operator - (int n) const {Iterator p(*this); for (int i = 0; i < n; i++) p--; return p;}

		explicit Iterator(void* p) {m_pointer = p;}
	};

	typedef ConstReverseIterator_t<T, UtList<T, Structure, Counter, ListElementType>, void*> ConstReverseIterator; ///< Reverse Iterator.
	typedef ReverseIterator_t<T, UtList<T, Structure, Counter, ListElementType>, void*> ReverseIterator; ///< Reverse Iterator.

	UtList() {} ///< Create an empty list.
	~UtList() {Clear();} ///< Free all elements.

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
	int Size() const {return counter.Count(structure.First());}

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

	UtList(UtList& other); // copy protection
	void operator = (UtList& other); // copy protection

	ListStructure structure;
	ListCounter counter;
	Initialiser initialiser;
};

};

using namespace Ut;


///
/// typedef UtList<MyClass, SingleEndedList, MaintainedCounter> MyListType1;
/// UtList<MyClass> myList; // default is DoubleEndedList, OnDemandCounter
/// UtList<int> myList2; // a non-intrusive list

template< class T >
class CMySortedList
{
public:
	struct element
	{
	public:
		T data;
		int iPriority;
	};

	typedef UtList<element, SingleEndedList, MaintainedCounter> T_List;
	typedef typename UtList<element>::Iterator List_Iterator;

	UtList<element> m_c;

	void clear()			{ m_c.Clear(); }
	bool empty()			{ return m_c.Empty(); }

	// move a newly added node to the right place in the heap
	void push(T pNewNode, int iPriority)
	{
		int iTestNode;

		element newe;
		newe.data = pNewNode;
		newe.iPriority = iPriority;

		List_Iterator i;
		for (i=m_c.Begin(); i!=m_c.End(); ++i)
		{
			if (i->iPriority < iPriority)
			{
				m_c.InsertBefore(newe, i);
				return;
			}
		}
		// add it at the end
		m_c.PushBack(newe);
	}
	void erase(int i)
	{
        m_c.Erase(i);
	}
	void push_back(T pNewNode)
	{
		element newe;
		newe.data = pNewNode;
		newe.iPriority = -1;
		// add it at the end
		m_c.PushBack(newe);
	}

	List_Iterator begin()	{ return m_c.Begin(); }
	List_Iterator end()		{ return m_c.End(); }
	List_Iterator &operator++() {this++;}
	List_Iterator erase(List_Iterator l) { return m_c.Erase(l); }
        
	const List_Iterator* operator->() const { return this->LE_Current(); }

	// pop the top node from the heap
	T PopTopNode()
	{
		element temp;
		temp = m_c.Back();
		m_c.PopBack();
		return temp.data;
	}

};

