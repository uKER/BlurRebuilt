#ifndef BTL_SKIPLIST_H
#define BTL_SKIPLIST_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////////////////
// Skip List
/////////////////////////////////////////////////////////////////

#include "btl/utility/types.h"
#include "btl/utility/pair.h"
#include "btl/containers/initialiser.h"
#include "btl/containers/sequence.h"
#include "btl/containers/list.h"
#include "btl/containers/setitem.h"

namespace Btl
{

/// Base class for SkipListSet, SkipListMultiSet, SkipListMap and SkipListMultiMap.
template <typename Key, typename Data, typename Allocator>
class SkipList
{
protected:
	struct Node; 

public:
	SkipList();
	~SkipList();

	typedef Data DataType;
	typedef Key KeyType;

	class ConstIterator
	{
	public:
		// constructors
		ConstIterator() {}
		ConstIterator(const typename SkipList::Node* pNode, const SkipList* pList, S32 level) {m_pList = pList; m_pNode = pNode; m_level = level;}

		// comparison
		bool operator == (const ConstIterator& i) const {return m_pNode == i.m_pNode;} ///< @a 
		bool operator != (const ConstIterator& i) const {return m_pNode != i.m_pNode;} ///< @a

		// return T
		const DataType& operator*() const {return m_pNode->m_element;} ///< @a 
		const DataType* operator->() const {return &m_pNode->m_element;} ///< @a 

		ConstIterator& operator ++(); ///< @a 
		ConstIterator operator ++(int) {ConstIterator i = *this; ++(*this); return i;} ///< @a 
		ConstIterator& operator --(); ///< @a 
		ConstIterator operator --(int) {ConstIterator i = *this; --(*this); return i;} ///< @a 

	protected:
		typename SkipList::Node* m_pNode;
		SkipList* m_pList;
		S32 m_level;

		friend class SkipList;
	};

	class Iterator : public ConstIterator
	{
	public:
		using ConstIterator::m_pNode;
		using ConstIterator::m_pList;
		using ConstIterator::m_level;

		// constructors
		Iterator() {}
		Iterator(typename SkipList::Node* pNode, SkipList* pList, S32 level) {m_pList = pList; m_pNode = pNode; m_level = level;}

		// return T
		DataType& operator*() const {return m_pNode->m_element;} ///< @a 
		DataType* operator->() const {return &m_pNode->m_element;} ///< @a 

		// arithmetic
		Iterator& operator ++() {ConstIterator::operator++(); return *this;} ///< @a 
		Iterator operator ++(int) {Iterator i = *this; ++(*this); return i;} ///< @a 
		Iterator& operator --() {ConstIterator::operator--(); return *this;} ///< @a 
		Iterator operator --(int) {Iterator i = *this; --(*this); return i;} ///< @a 

	private:
		friend class SkipList;
	};

	void Clear();
	void PopFront();

	// information
	bool Empty() const;
	int Size() const;
	
	// make iterators
	Iterator Begin(S32 level = 0) {level = Min(level, MAX_LINKS - 1); return Iterator(m_head.GetLink(level), this, level);} ///< %Iterator pointing to the first element.
	ConstIterator Begin() const {return const_cast<SkipList*>(this)->Begin();} ///< %Iterator pointing to the first element.
	Iterator End() {return Iterator(0, this, 0);} ///< %Iterator pointing past the last element.
	ConstIterator End() const {return const_cast<SkipList*>(this)->End();} ///< %Iterator pointing past the last element.

	/// Clear all elements and free all resources.
	void Destroy();

	/// Swaps elements with the other list.
	void Swap(SkipList& other);

protected:
	// Random number generator for skip lists.
	class Random
	{
	public:
		Random() : jsr(123456789) {}

		U32 GetRandom() {return (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5));}

	private:
		U32 jsr;
	};

	const static int MAX_LINKS = 16;

	struct Links
	{
		Links() {for (int i = 0; i < MAX_LINKS; i++) links[i] = 0;}
		Node* links[MAX_LINKS];
	};

	struct Node
	{
		Node() {Initialise();}
		explicit Node(const KeyType& key) : m_element(key) {Initialise();}
		~Node() {}

		DataType m_element;

		void Extend(Links* pExtend)
		{
			table = (Node**) pExtend;
		}

		Links* Extend()
		{
			if (table == links) return 0;
			else return (Links*) table;
		}

		void SetLink(S32 level, Node* p) 
		{
			assert(level >= 0 && level < MAX_LINKS);
			assert(level < 3 || table != links);

			table[level] = p;
		}

		Node* GetLink(S32 level) const
		{
			assert(level >= 0 && level < MAX_LINKS);
			assert(level < 3 || table != links);

			return table[level];
		}

		void SetPrevious(Node* p) 
		{
			previous = p;
		}

		Node* GetPrevious() const
		{
			return previous;
		}

	private:
		void Initialise()
		{
			for (int i = 0; i < 3; i++)
				links[i] = 0;

			table = links;
			previous = 0;
		}

		Node** table;
		Node* links[3];
		Node* previous;
	};

	static S32 Min(S32 a, S32 b) {return a < b ? a : b;}
	static S32 Max(S32 a, S32 b) {return a < b ? b : a;}

	typedef CopyInitialiser<KeyType, Node, typename Allocator::template Rebind<Node>::Other > Initialiser;

	Initialiser m_initialiser;
	MaintainedCounter m_counter;
	Node m_head;
	Links m_headLinks;
	Random m_random;
	typedef typename Allocator::template Rebind<Links>::Other LinkAllocator;
	LinkAllocator m_linkAllocator;
	S32 m_maxLevel;
};

// SkipListBuilder handles insertion and deletion.
template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
class SkipListBuilder : public SkipList<Key, Data, Allocator>
{
public:
	typedef SkipList<Key, Data, Allocator> Base;
	typedef typename Base::Node Node;
	using Base::Clear;
	using Base::End;
	using Base::m_maxLevel;
	using Base::m_random;
	using Base::m_head;
	using Base::m_initialiser;
	using Base::m_linkAllocator;
	using Base::m_counter;

	typedef typename Base::ConstIterator ConstIterator;
	typedef typename Base::Iterator Iterator;

	typedef typename ElementPolicy<Iterator>::InsertionReturnType InsertionReturnType;

	/// Insert an element.
	/// Set and Map return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// MultiSet and MultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(const Key& item, S32 level = 0); 

	/// Deletes all items matching the key and returns the number of items deleted.
	int Erase(const Key& key); 

	/// Erases the item pointed to by the iterator.
	/// The iterator must point to a valid item in the tree, or the results are undefined.
	void Erase(Iterator i);

	// searching
	Iterator Find(const Key& key); ///< Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.

	ConstIterator Find(const Key& key) const;
};

/////////////////////////////////////////////////////////////////////////////

/// %SkipListSet container class.
/// Elements in a set are unique and sorted according to the < operator.
///
/// @code
/// typedef SkipListSet<String> MySet; // define a set type
/// MySet set; // create a set
/// set.Insert(String("test")); // insert an item
/// @endcode
///
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. StandardAllocator<Key>.
template <typename Key, typename Allocator = StandardAllocator<NullType> >
class SkipListSet : public SkipListBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements>
{
public:
	typedef SkipListBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements> Base;
	using Base::InsertionReturnType;
	using Base::Clear;

	SkipListSet() {}
	SkipListSet(const SkipListSet& other) 
	{
		 CopySkipListElements(*this, other);
	}

	SkipListSet& operator = (const SkipListSet& other)
	{
		if (this == &other) return *this;
		Clear();
		CopySkipListElements(*this, other);
		return *this;
	}

	// swap needs to be an exact match so it can't be in the base class
	friend void Swap(SkipListSet& left, SkipListSet& right) {left.Swap(right);} ///< @a
};

///////////////////////////////////////////////////////////////////////////////

/// %SkipListMap container class.
/// Elements in a map are unique and sorted according to the < operator on the keys.
/// The type stored in the map is MapItem<Key, Value>.
///
/// @code
/// typedef SkipListMap<U32, std::string> MyMap; // define a map type
/// MyMap map; // create a map
/// map.Insert(MapItem(0, std::string("test"))); // insert an item with key 0
/// @endcode
///
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. StandardAllocator<MapItem>.
template <typename Key, typename Value, typename Allocator = StandardAllocator<NullType> >
class SkipListMap : public SkipListBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements>
{
public:
	typedef SkipListBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements> Base;
	using Base::Clear;

	SkipListMap() {}
	SkipListMap(const SkipListMap& other) 
	{
		CopySkipListElements(*this, other);
	}

	SkipListMap& operator = (const SkipListMap& other)
	{
		if (this == &other) return *this;
		Clear();
		CopySkipListElements(*this, other);
		return *this;
	}

	/// Returns a reference to the value associated with the key, inserting a new item if necessary.
	Value& operator [] (const Key& key)
	{
		return (*Base::Insert(key).iterator).GetValue(); 
	}

	/// Insert an element.
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(const typename Base::DataType& item, S32 level = 0)
	{
		typename Base::InsertionReturnType r = Base::Insert(item.GetKey(), level);
		if (r.inserted) 
			r.iterator->GetValue() = item.GetValue();

		return r;
	}

	typename Base::InsertionReturnType Insert(const Key& key, S32 level = 0)
	{
		return Base::Insert(key, level);
	}

	typename Base::InsertionReturnType Insert(const Key& key, const Value& value)
	{
		typename Base::InsertionReturnType r = Base::Insert(key);
		if (r.inserted) 
			r.iterator->GetValue() = value;

		return r;
	}

	// Insert multiple elements.
	template <typename InputIterator>
	void InsertMultiple(InputIterator begin, InputIterator end)
	{
		for (; begin != end; begin++)
			Insert(*begin);
	}

	/// Insert an element with a hint to the location.
	typename Base::InsertionReturnType Insert(typename Base::Iterator hint, const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(/*hint,*/item.GetKey());
		if (r.inserted)
			r.iterator->GetValue() = item.GetValue();

		return r;
	}

	friend void Swap(SkipListMap& left, SkipListMap& right) {left.Swap(right);}  ///< @a
};

//////////////////////////////////////////////////////////

/// %MultiSet container class.
/// A sorted container which, unlike Set, can hold duplicate items.
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. StandardAllocator<Key>.
template <typename Key, typename Allocator = StandardAllocator<NullType> >
class SkipListMultiSet : public SkipListBuilder<Key, SetItem<Key, Allocator>, Allocator, MultipleElements>
{
public:
	typedef SkipListBuilder<Key, SetItem<Key, Allocator>, Allocator, MultipleElements> Base;
	using Base::Clear;
	
	SkipListMultiSet() {}
	SkipListMultiSet(const SkipListMultiSet& other) 
	{
		CopyBinaryTreeElements(*this, other);
	}

	SkipListMultiSet& operator = (const SkipListMultiSet& other)
	{
		if (this == &other) return *this;
		Clear();
		CopySkipListElements(*this, other);
		return *this;
	}

	friend void Swap(SkipListMultiSet& left, SkipListMultiSet& right) {left.Swap(right);}  ///< @a
};

//////////////////////////////////////////////////////////

/// %SkipListMultiMap container class.
/// A sorted container which, unlike Map, can hold duplicate items.
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. StandardAllocator<MapItem>.
template <typename Key, typename Value, typename Allocator = StandardAllocator<NullType> >
class SkipListMultiMap : public SkipListBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, MultipleElements>
{
public:
	typedef SkipListBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, MultipleElements> Base;
	using Base::Clear;

	SkipListMultiMap() {}
	SkipListMultiMap(const SkipListMultiMap& other) 
	{
		CopyBinaryTreeElements(*this, other);
	}

	SkipListMultiMap& operator = (const SkipListMultiMap& other)
	{
		if (this == &other) return *this;
		Clear();
		CopySkipListElements(*this, other);
		return *this;
	}

	/// Insert an element.
	/// Returns an iterator to the inserted element.
	typename Base::InsertionReturnType Insert(const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(item.GetKey());
		r->GetValue() = item.GetValue();
		return r;
	}

	/// Insert an element.
	/// Returns an iterator to the inserted element.
	typename Base::InsertionReturnType Insert(const typename Base::KeyType& key, const typename Base::DataType::ValueType& value)
	{
		typename Base::InsertionReturnType r = Base::Insert(key);
		r->GetValue() = value;
		return r;
	}

	/// Insert an element with a hint to the location.
	/// Returns an iterator to the inserted element.
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	typename Base::InsertionReturnType Insert(typename Base::Iterator hint, const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(hint, item.GetKey());
		r->GetValue() = item.GetValue();
		return r;
	}

	friend void Swap(SkipListMultiMap& left, SkipListMultiMap& right) {left.Swap(right);} ///< @a
};

//////////////////////////////////////////////////////////

#include "skiplist.inl"

}

#endif
