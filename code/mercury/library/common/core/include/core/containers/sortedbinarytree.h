#ifndef _SORTEDBINARYTREE_H_
#define _SORTEDBINARYTREE_H_
#pragma once

#include <core/utility/utility.h>
#include <core/containers/initialiser.h>

namespace Ut
{
/// @addtogroup Containers
/// @{

// Sorted Binary Tree
//
// Implemented as Andersson Tree (which is self balancing and simpler than red-black).
// Data items may be unique (Set and Map), i.e. one copy of each key, or not (MultiSet, MultiMap).
// Insert, erase, find are O(log(N)).
// Uses Ut::Pool to store elements.
//
// Template parameters:
//  Data is the type stored in the tree.
//  Key is the type used for comparisons, which may be the same or different to Data.
//
//  'less than' operator must be available:
//    Key < Key
//
// See Set / Map / MultiSet / MultiMap below.

/// Base class for UtSet, MultiSet, UtMap and MultiMap.
template <typename Key, typename Data, typename Allocator>
class SortedBinaryTree
{
protected:
	struct Node; // JASOND: This had to be moved up here because the intel copmpiler generates an error in ConstIterator otherwise.
	
public:
	SortedBinaryTree();
	~SortedBinaryTree() {Free(m_pRoot);}

	/// The type stored in the tree.
	typedef Data DataType;
	typedef Key KeyType;

	/// %Iterator for SortedBinaryTree
	class ConstIterator
	{
	public:
		// constructors
		ConstIterator() {}
		ConstIterator(const typename SortedBinaryTree::Node* pNode, const SortedBinaryTree* pTree) {m_pTree = pTree; m_pNode = pNode;}

		// comparison
		bool operator == (const ConstIterator& i) const ///< @a
		{
			return m_pNode == i.m_pNode;
		}

		bool operator != (const ConstIterator& i) const; ///< @a

		// return T
		const DataType& operator*() const {return m_pNode->m_element;} ///< @a
		const DataType* operator->() const {return &m_pNode->m_element;} ///< @a

		// arithmetic (tree iterators only support increment and decrement)
		ConstIterator& operator ++(); ///< @a
		ConstIterator& operator --() ///< @a
		{
			if (m_pNode->pFirst != m_pTree->SentinelPointer())
			{
				m_pNode = m_pNode->pFirst;

				while (m_pNode->pSecond != m_pTree->SentinelPointer())
				{
					m_pNode = m_pNode->pSecond;
				};
			}
			else
			{
				while (m_pNode != m_pTree->SentinelPointer())
				{
					if (m_pNode == m_pNode->pParent->pSecond)
					{
						m_pNode = m_pNode->pParent;
						return *this;
					} 
					else
					{
						m_pNode = m_pNode->pParent;
					}
				};
			}

			return *this;
		}

		ConstIterator operator ++(int) {ConstIterator i = *this; ++(*this); return i;} ///< @a
		ConstIterator operator --(int) {ConstIterator i = *this; --(*this); return i;} ///< @a

	protected:
		typename SortedBinaryTree::Node* m_pNode;
		SortedBinaryTree* m_pTree;

		friend class SortedBinaryTree;
	};

	/// %Iterator for SortedBinaryTree
	class Iterator : public ConstIterator
	{
	public:
		using ConstIterator::m_pNode;
		using ConstIterator::m_pTree;

		// constructors
		Iterator() {}
		Iterator(typename SortedBinaryTree::Node* pNode, SortedBinaryTree* pTree) {m_pTree = pTree; m_pNode = pNode;}

		// return T
		DataType& operator*() const {return m_pNode->m_element;} ///< @a
		DataType* operator->() const {return &m_pNode->m_element;} ///< @a

		// arithmetic
		Iterator& operator ++() {ConstIterator::operator++(); return *this;} ///< @a
		Iterator& operator --() {ConstIterator::operator--(); return *this;} ///< @a
		Iterator operator ++(int) {Iterator i = *this; ++(*this); return i;} ///< @a
		Iterator operator --(int) {Iterator i = *this; --(*this); return i;} ///< @a

	private:
		friend class SortedBinaryTree;
	};

protected:
	//struct Node;

	struct Sentinel
	{
		Node* pParent;
		Node* pFirst;
		Node* pSecond;
		int level;
	};

	struct Node : public Sentinel
	{
		Node() {}
		explicit Node(const KeyType& key) : m_element(key) {}

		DataType m_element;
	};

	Node*& GetNode(Iterator i) {return i.m_pNode;}
	const Node*& GetNode(ConstIterator i) const {return i.m_pNode;}

public:

	// reverse iterators
	typedef ConstReverseIterator_t<DataType, SortedBinaryTree, Node*> ConstReverseIterator; ///< Reverse iterator.
	typedef ReverseIterator_t<DataType, SortedBinaryTree, Node*> ReverseIterator; ///< Reverse iterator.

	// information
	bool Empty() const {return m_pRoot == SentinelPointer();} ///< Returns true if the tree is empty.
	int Size() const {return m_counter.Count(*this);} ///< Returns the number of items in the tree.

	void Clear() {Free(m_pRoot); m_counter.Reset(); m_pRoot = SentinelPointer();} ///< Erase all elements.
	
	// make iterators
	Iterator Begin(); ///< %Iterator pointing to the first element.
	ConstIterator Begin() const {return const_cast<SortedBinaryTree*>(this)->Begin();} ///< %Iterator pointing to the first element.
	Iterator End() {return Iterator(SentinelPointer(), this);} ///< %Iterator pointing past the last element.
	ConstIterator End() const {return const_cast<SortedBinaryTree*>(this)->End();} ///< %Iterator pointing past the last element.

	// reverse iterators
	ReverseIterator ReverseBegin(); ///< Reverse iterator pointing to the last element.
	ConstReverseIterator ReverseBegin() const {return const_cast<SortedBinaryTree*>(this)->ReverseBegin();} ///< Reverse iterator pointing to the last element.
	ReverseIterator ReverseEnd() {return ReverseIterator(SentinelPointer(), this);} ///< Reverse iterator pointing one before the first element.
	ConstReverseIterator ReverseEnd() const {return const_cast<SortedBinaryTree*>(this)->ReverseEnd();} ///< Reverse iterator pointing one before the first element.

	/// Swaps elements with the other tree.
	void Swap(SortedBinaryTree& other);

	/// Clear all elements and free all resources.
	void Destroy();

	void PrintDebug() const;
	typedef CopyInitialiser<KeyType, Node, typename Allocator::template Rebind<Node>::Other > Initialiser;

	bool operator == (const SortedBinaryTree& other) const
	{
		if (Size() != other.Size())
			return false;

		ConstIterator i = Begin();
		ConstIterator j = other.Begin();

		for (; i != End(); i++, j++)
		{
			if (*i != *j)
				return false;
		}

		return true;
	}

	bool operator != (const SortedBinaryTree& other) const {return !(*this == other);}

	bool operator < (const SortedBinaryTree& other) const
	{
		if (Size() != other.Size())
			return Size() < other.Size();

		ConstIterator i = Begin();
		ConstIterator j = other.Begin();

		for (; i != End(); i++, j++)
		{
			if (*i != *j)
				return *i < *j;
		}

		return false;
	}

protected:
	friend class Iterator;

	Node* m_pRoot;
	Sentinel m_sentinel;
	MaintainedCounter m_counter; // could use OnDemandCounter, but maintaining a counter is relatively cheap.

	Initialiser m_initialiser;

	void Initialise();
	void Free(Node* pNode);
	int Skew(Node*& pParent);
	int Split(Node*& pParent);
	void RotateFirst(Node*& pParent);
	void RotateSecond(Node*& pParent);
	Node** SwapWithLastItem(Node*& pNode, Node*& pNodeToSwap);
	void BalanceInsertedItem(Node** pp);
	void BalanceDeletedItem(Node** pp);
	void EraseNode(Node** pp);
	void FixSentinelPointers(Node** pp, Node* pOldSentinel);
	Node* SentinelPointer() {return (Node*) &m_sentinel;}
	const Node* SentinelPointer() const {return (Node*) &m_sentinel;}

	SortedBinaryTree(const SortedBinaryTree& tree);

	void PrintDebug(const Node* pNode, int level) const;
};

//////////////////////////////////////////////////////////////
// Insert and Erase handled by TreeBuilder

template <typename Tree>
struct MultipleElements
{
	static const bool MULTIPLE = true;
	typedef typename Tree::Iterator InsertionReturnType;
	static InsertionReturnType MakeInsertionReturnType(typename Tree::Iterator i, bool) {return i;}
};

template <typename Tree>
struct UniqueElements
{
	static const bool MULTIPLE = false;
	typedef Pair<typename Tree::Iterator, bool> InsertionReturnType;
	static InsertionReturnType MakeInsertionReturnType(typename Tree::Iterator i, bool b) {return InsertionReturnType(i, b);}
};

/// Base class for UtSet, MultiSet, UtMap and MultiMap.
/// Inserting and deleting items is O(log(N)) where N is the number of items in the container.
/// Iterators are not invalidated by inserting or deleting other items.
template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
class TreeBuilder : public SortedBinaryTree<Key, Data, Allocator>
{
public:
	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	//Types
	typedef typename Base::ConstIterator ConstIterator;
	typedef typename Base::Iterator Iterator;
	typedef typename Base::ReverseIterator ReverseIterator;
	typedef typename Base::Node Node;

	//Functions
	using Base::Initialise;
	using Base::Clear;
	using Base::End;

	//Members
	using Base::m_pRoot;
	using Base::m_sentinel;
	using Base::m_initialiser;
	using Base::m_counter;
	using Base::SentinelPointer;

	/// The type stored in the tree.
	typedef Data DataType;
	typedef Key KeyType;

	TreeBuilder() {}
	TreeBuilder(const TreeBuilder& tree);

	TreeBuilder& operator = (const TreeBuilder& tree);

	typedef typename ElementPolicy<Base>::InsertionReturnType InsertionReturnType;
//	static const bool MULTIPLE = ElementPolicy<Base>::MULTIPLE;

	/// Insert an element.
	/// UtSet and UtMap return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// MultiSet and MultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(const KeyType& item); 

	/// Insert an element with a hint to the location.
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	/// UtSet and UtMap return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// MultiSet and MultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(Iterator hint, const KeyType& item);

	// Insert multiple elements.
	template <typename InputIterator>
	void InsertMultiple(InputIterator begin, InputIterator end)
	{
		for (; begin != end; begin++)
			Insert(*begin);
	}

	int Erase(const Key& key); ///< Deletes all items matching the key and returns the number of items deleted.

	/// Erases the item pointed to by the iterator.
	/// The iterator must point to a valid item in the tree, or the results are undefined.
	void Erase(Iterator i);
	void ReverseErase(ReverseIterator i);

	// searching
	Iterator Find(const Key& key); ///< Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.

	ConstIterator Find(const Key& key) const 
	{
		return const_cast<TreeBuilder<Key, Data, Allocator, ElementPolicy>* >(this)->Find(key);
	} ///< Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.

private:
	InsertionReturnType Insert(Node** pp, Node* pParent, const KeyType& item);
};

template <typename Tree>
void CopyElements(Tree& tree1, const Tree& tree2)
{
	typename Tree::ConstIterator end = tree2.End();
	for (typename Tree::ConstIterator it = tree2.Begin(); it != end; ++it)
		tree1.Insert(*it);
}

////////////////////////////////////////////////////////////
// Set and Map
// (which are just different interfaces to SortedBinaryTree)

/// Semi-transparent wrapper for Set elements.
/// Converts implicitly to and from the Key type, so it will usually be invisible.
/// Occasionally it may be necessary to use the functions to convert explicitly. 
template <typename Key, typename Allocator>
class SetItem
{
public:
	SetItem() {}
	SetItem(const SetItem& i) : key(i.key) {}
	SetItem(const Key& k) : key(k) {} ///< Implicit construction

	operator const Key& () const {return key;} ///< Can only get a const reference

	const Key& GetKey() const {return key;} ///< Returns a const reference to the key. The key cannot be modified after creation of the %SetItem.
	const Key& GetValue() const {return key;} ///< Return a reference to the value, which is the same as the key.

	// comparison operators
	friend bool operator < (const SetItem& a, const SetItem& b) {return a.key < b.key;}
	friend bool operator < (const Key& a, const SetItem& b) {return a < b.key;}
	friend bool operator < (const SetItem& a, const Key& b) {return a.key < b;}

	FRIEND_CLASS SortedBinaryTree<Key, SetItem, Allocator>::Initialiser;

private:
	SetItem& operator = (const SetItem& i) {key = i.key; return *this;} // only the initialiser can modify the item

	//TODO: Should be a friend of the Initialiser typedef of SortedBinaryTree.
	Key key;
};

/// %Set container class.
/// Elements in a set are unique and sorted according to the < operator.
/// See SetItem.
///
/// @code
/// typedef UtSet<std::string> MySet; // define a set type
/// MySet set; // create a set
/// set.Insert(std::string("test")); // insert an item
/// @endcode
///
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<Key>.
template <typename Key, typename Allocator = StandardAllocator<NullType> >
class UtSet : public TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements>
{
public:
	typedef TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	UtSet() {}
	UtSet(const UtSet& other) 
	{
		//Initialise();
		CopyElements(*this, other);
	}

	UtSet& operator = (const UtSet& other)
	{
		Clear();
		CopyElements(*this, other);
		return *this;
	}

	// swap needs to be an exact match so it can't be in the base class
	friend void Swap(UtSet& left, UtSet& right) {left.Swap(right);} ///< @a
};

/// The type stored in a Map or MultiMap.
/// @remarks
/// The stl uses 'pair' for this. But MapItem gives named functions and protects the key from being modified.
template <typename Key, typename Value, typename Allocator>
class MapItem
{
public:
	MapItem() {}
	explicit MapItem(const Key& k) : key(k) {} ///<
	MapItem(const Key& k, const Value& v) : key(k), value(v) {} ///<
	MapItem(const MapItem& i) : key(i.key), value(i.value) {}

	Value& GetValue() {return value;} ///< Returns a reference to the value.
	const Key& GetKey() const {return key;} ///< Returns a const reference to the key. The key cannot be modified after creation of the %MapItem.
	const Value& GetValue() const {return value;} ///< Return a reference to the value.

	// comparison operators for tree
	friend bool operator < (const MapItem& a, const MapItem& b) {return a.key < b.key;}
	friend bool operator < (const Key& a, const MapItem& b) {return a < b.key;}
	friend bool operator < (const MapItem& a, const Key& b) {return a.key < b;}

	// Equality requires both key and value to be equal.
	// Unusually, two non-equal items can have neither one less than the other.
	bool operator == (const MapItem& other) const
	{
		return (key == other.key && value == other.value);
	}

	bool operator != (const MapItem& other) const
	{
		return !(key == other.key && value == other.value);
	}

	FRIEND_CLASS SortedBinaryTree<Key, MapItem, Allocator>::Initialiser;

private:
	MapItem& operator = (const MapItem& i) 
	{
		key = i.key; 
		value = i.value;
		return *this;
	} // only the initialiser can modify the item

	MapItem& operator = (const Key& k) 
	{
		key = k; 
		return *this;
	} // only the initialiser can modify the item

	//TODO: Should be a friend of the Initialiser typedef of SortedBinaryTree.
	Key key;
	Value value;
};

/// %Map container class.
/// Elements in a map are unique and sorted according to the < operator on the keys.
/// The type stored in the map is MapItem<Key, Value>.
/// See MapItem.
///
/// @code
/// typedef UtMap<U32, std::string> MyMap; // define a map type
/// MyMap map; // create a map
/// map.Insert(MapItem(0, std::string("test"))); // insert an item with key 0
/// @endcode
///
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<MapItem>.
template <typename Key, typename Value, typename Allocator = StandardAllocator<NullType> >
class UtMap : public TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements>
{
public:
	typedef TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	UtMap() {}
	UtMap(const UtMap& other) 
	{
		//Initialise();
		CopyElements(*this, other);
	}

	UtMap& operator = (const UtMap& other)
	{
		Clear();
		CopyElements(*this, other);
		return *this;
	}

	/// Returns a reference to the value associated with the key, inserting a new item if necessary.
	Value& operator [] (const Key& key)
	{
		//return (*Insert(MapItem<Key, Value, Allocator>(key, Value())).first).GetValue(); 
		return (*Base::Insert(key).first).GetValue(); 
	}

	/// Insert an element.
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(item.GetKey());
		if (r.second) 
			r.first->GetValue() = item.GetValue();

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
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(typename Base::Iterator hint, const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(hint, item.GetKey());
		if (r.second)
			r.first->GetValue() = item.GetValue();

		return r;
	}

	friend void Swap(UtMap& left, UtMap& right) {left.Swap(right);}  ///< @a
};

/// %MultiSet container class.
/// A sorted container which, unlike UtSet, can hold duplicate items.
/// See SetItem.
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<Key>.
template <typename Key, typename Allocator = StandardAllocator<NullType> >
class MultiSet : public TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, MultipleElements>
{
public:
	typedef TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, MultipleElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	MultiSet() {}
	MultiSet(const MultiSet& other) 
	{
		//Initialise();
		CopyElements(*this, other);
	}

	MultiSet& operator = (const MultiSet& other)
	{
		Clear();
		CopyElements(*this, other);
		return *this;
	}

	friend void Swap(MultiSet& left, MultiSet& right) {left.Swap(right);}  ///< @a
};

/// %MultiMap container class.
/// A sorted container which, unlike UtMap, can hold duplicate items.
/// See SetItem.
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<MapItem>.
template <typename Key, typename Value, typename Allocator = StandardAllocator<NullType> >
class MultiMap : public TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, MultipleElements>
{
public:
	typedef TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, MultipleElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	MultiMap() {}
	MultiMap(const MultiMap& other) 
	{
		//Initialise();
		CopyElements(*this, other);
	}

	MultiMap& operator = (const MultiMap& other)
	{
		Clear();
		CopyElements(*this, other);
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

	/// Insert an element with a hint to the location.
	/// Returns an iterator to the inserted element.
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	typename Base::InsertionReturnType Insert(typename Base::Iterator hint, const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(hint, item.GetKey());
		r->GetValue() = item.GetValue();
		return r;
	}

	friend void Swap(MultiMap& left, MultiMap& right) {left.Swap(right);} ///< @a
};

/// @}

///////////////////////////////////////////////////////////////

template <typename Key, typename Data, typename Allocator>
inline SortedBinaryTree<Key, Data, Allocator>::SortedBinaryTree()
{
	Initialise();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::Initialise()
{
	// set up the sentinel
	m_sentinel.pFirst = SentinelPointer();
	m_sentinel.pSecond = SentinelPointer();
	m_sentinel.pParent = SentinelPointer();
	m_sentinel.level = 0;

	// root points to the sentinel
	m_pRoot = SentinelPointer();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::Free(Node* pNode)
{
	if (pNode != SentinelPointer())
	{
		Free(pNode->pFirst);
		Free(pNode->pSecond);
		m_initialiser.Release(pNode);
	}
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::BalanceInsertedItem(Node** pp)
{
	// walk back up the tree and rebalance
	int count = 0;

	while (*pp != m_pRoot)
	{
		if ((*pp)->pParent == m_pRoot)
			pp = &m_pRoot;
		else if ((*pp)->pParent->pParent->pFirst == (*pp)->pParent)
			pp = &((*pp)->pParent->pParent->pFirst);
		else
			pp = &((*pp)->pParent->pParent->pSecond);

		int change = Skew(*pp);
		change |= Split(*pp);

		if (change == 0) count++;
		else count = 0;

		if (count == 2) 
			return;
	}
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::EraseNode(Node** pp)
{
	Node* p = (*pp);
	Node* pParent = (*pp)->pParent;

	if ((*pp)->pFirst == SentinelPointer())
	{
		(*pp) = (*pp)->pSecond;
		(*pp)->pParent = pParent;
	}
	else if ((*pp)->pSecond == SentinelPointer())
	{
		(*pp) = (*pp)->pFirst;
		(*pp)->pParent = pParent;
	}
	else
		pp = SwapWithLastItem((*pp)->pFirst, (*pp));

	BalanceDeletedItem(pp);
	m_initialiser.Release(p);
	m_counter.EraseElement();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::BalanceDeletedItem(Node** pp)
{
	// walk back up the tree and rebalance
	int count = 0;

	while (*pp != m_pRoot)
	{
		if ((*pp)->pParent == m_pRoot)
			pp = &m_pRoot;
		else if ((*pp)->pParent->pParent->pFirst == (*pp)->pParent)
			pp = &((*pp)->pParent->pParent->pFirst);
		else
			pp = &((*pp)->pParent->pParent->pSecond);

		int change = 0;
			
		if ((*pp)->pFirst->level < (*pp)->level - 1
			|| (*pp)->pSecond->level < (*pp)->level - 1)
		{
			(*pp)->level--;

			if ((*pp)->pSecond->level > (*pp)->level)
				(*pp)->pSecond->level = (*pp)->level;

			Skew((*pp));
			Skew((*pp)->pSecond);
			Skew((*pp)->pSecond->pSecond);
			Split((*pp));
			if ((*pp)->pSecond != SentinelPointer()) Split((*pp)->pSecond);

			change = 1;
		}

		if (change == 0) count++;
		else count = 0;

		if (count == 2) 
			return;
	}
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::Node** 
	SortedBinaryTree<Key, Data, Allocator>::SwapWithLastItem(Node*& pNode, Node*& pNodeToSwap)
{
	Node** pp = &pNode;

	while ((*pp)->pSecond != SentinelPointer())
		pp = &((*pp)->pSecond);

	Node* pc = *pp;

	assert(pc != SentinelPointer());

	pc->pFirst->pParent = pc->pParent;
	*pp = pc->pFirst;
	pc->pFirst = pNodeToSwap->pFirst;
	pc->pSecond = pNodeToSwap->pSecond;
	pc->pParent = pNodeToSwap->pParent;
	pc->level = pNodeToSwap->level;
	pNodeToSwap->pFirst->pParent = pc;
	pNodeToSwap->pSecond->pParent = pc;
	pNodeToSwap = pc;

	return pp;
}

template <typename Key, typename Data, typename Allocator>
UT_FORCE_INLINE int SortedBinaryTree<Key, Data, Allocator>::Skew(Node*& pNode)
{
	if (pNode->level == pNode->pFirst->level)
	{
		RotateFirst(pNode);
		return 1;
	}

	return 0;
}

template <typename Key, typename Data, typename Allocator>
UT_FORCE_INLINE int SortedBinaryTree<Key, Data, Allocator>::Split(Node*& pNode)
{
	if (pNode->level == pNode->pSecond->pSecond->level)
	{
		assert(pNode->pSecond != SentinelPointer());
		pNode->pSecond->level++;
		RotateSecond(pNode);
		return 1;
	}

	return 0;
}

template <typename Key, typename Data, typename Allocator>
UT_FORCE_INLINE void SortedBinaryTree<Key, Data, Allocator>::RotateFirst(Node*& pNode)
{
	Node* p = pNode->pFirst;
	pNode->pFirst = p->pSecond;
	p->pSecond->pParent = pNode;
	p->pSecond = pNode;
	p->pParent = pNode->pParent;
	pNode->pParent = p;
	pNode = p;
}

template <typename Key, typename Data, typename Allocator>
UT_FORCE_INLINE void SortedBinaryTree<Key, Data, Allocator>::RotateSecond(Node*& pNode)
{	
	Node* p = pNode->pSecond;
	pNode->pSecond = p->pFirst;
	p->pFirst->pParent = pNode;
	p->pFirst = pNode;
	p->pParent = pNode->pParent;
	pNode->pParent = p;
	pNode = p;
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::Iterator SortedBinaryTree<Key, Data, Allocator>::Begin()
{
	Node* pNode = m_pRoot;

	while (pNode->pFirst != SentinelPointer())
		pNode = pNode->pFirst;

	return Iterator(pNode, this);
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::ReverseIterator SortedBinaryTree<Key, Data, Allocator>::ReverseBegin()
{
	Node* pNode = m_pRoot;

	while (pNode->pSecond != SentinelPointer())
		pNode = pNode->pSecond;

	return ReverseIterator(pNode, this);
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::FixSentinelPointers(Node** pp, Node* pOldSentinel)
{
	if ((*pp) == pOldSentinel)
	{
		*pp = SentinelPointer();
	}
	else
	{
		// would be tricky to do this non-recursively as the sentinel pointers are still wrong
		FixSentinelPointers(&(*pp)->pFirst, pOldSentinel);
		FixSentinelPointers(&(*pp)->pSecond, pOldSentinel);
	}
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::Swap(SortedBinaryTree& other)
{
	Ut::Swap(m_pRoot, other.m_pRoot);
	Ut::Swap(m_counter, other.m_counter);
	// no need to swap the sentinel
	Ut::Swap(m_initialiser, other.m_initialiser);

	m_pRoot->pParent = SentinelPointer();
	other.m_pRoot->pParent = other.SentinelPointer();

	// traverse both trees and fix up the sentinel pointers
	FixSentinelPointers(&m_pRoot, other.SentinelPointer());
	other.FixSentinelPointers(&other.m_pRoot, SentinelPointer());
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::Destroy()
{
	Clear();
	Initialiser empty;
	Ut::Swap(m_initialiser, empty);
}

///////////////////////////////////////////////////////////////////////
// debug
template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::PrintDebug() const
{
	PrintDebug(m_pRoot, 0);
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::PrintDebug(const Node* pNode, int level) const
{
	//for (int i = 0; i < level; i++) 
	//	DBPrint("\t");

	if (pNode != SentinelPointer())
	{
		//DBPrint("%i \n", pNode->level);
		PrintDebug(pNode->pFirst, level + 1);
		PrintDebug(pNode->pSecond, level + 1);
		assert(pNode->pFirst == SentinelPointer() || pNode->pFirst->pParent == pNode);
		assert(pNode->pSecond == SentinelPointer() || pNode->pSecond->pParent == pNode);
		assert(pNode->pFirst->level >= pNode->level - 1);
		assert(pNode->pSecond->level >= pNode->level - 1);
	}
	//else
	//	DBPrint("(%i) \n", pNode->level);
}

//////////////////////////////////////////////////////////////////
// tree builder

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(const KeyType& item)
{
	return Insert(&m_pRoot, SentinelPointer(), item);
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(typename SortedBinaryTree<Key, Data, Allocator>::Iterator hint, 
										const KeyType& item)
{
	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	Node* pNode = GetNode(hint);

	// if the item is before the hint, do a normal insertion
	if (pNode == SentinelPointer() || item < *hint)
		return Insert(&m_pRoot, SentinelPointer(), item);
	
	// if the item immediately follows the hint, it can be inserted without starting from the root
	if (++hint == End() || !(*hint < item))
	{
		if (item < pNode->m_element)
			return Insert(&pNode->pFirst, pNode, item);
		else if (ElementPolicy<Base>::MULTIPLE || pNode->m_element < item)
			return Insert(&pNode->pSecond, pNode, item);
		else
			return ElementPolicy<Base>::MakeInsertionReturnType(Iterator(pNode, this), false);
	}
	else
		return Insert(&m_pRoot, SentinelPointer(), item);
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(typename SortedBinaryTree<Key, Data, Allocator>::Node** pp,
										typename SortedBinaryTree<Key, Data, Allocator>::Node* pParent,
										const KeyType& item)
{
	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	while (true)
	{
		if ((*pp) == SentinelPointer())
		{
			*pp = &m_initialiser.Initialise(item);
			(*pp)->pFirst = SentinelPointer();
			(*pp)->pSecond = SentinelPointer();
			(*pp)->pParent = pParent;
			(*pp)->level = 1;

			Node* pInserted = *pp;

			BalanceInsertedItem(pp);
			
			m_counter.AddElement();

			return ElementPolicy<Base>::MakeInsertionReturnType(Iterator(pInserted, this), true);
		}
		else
		{
			pParent = *pp;

			if (item < (*pp)->m_element)
				pp = &(*pp)->pFirst;
			else if (ElementPolicy<Base>::MULTIPLE || (*pp)->m_element < item)
				pp = &(*pp)->pSecond;
			else
				return ElementPolicy<Base>::MakeInsertionReturnType(Iterator(*pp, this), false); // no insertion
		}
	}
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::Erase(typename SortedBinaryTree<Key, Data, Allocator>::Iterator i)
{
	Node** pp;

	// We have an iterator but need the link to the item.
	if (GetNode(i) == m_pRoot)
		pp = &m_pRoot;
	else if (GetNode(i) == SentinelPointer())
		return;
	else if (GetNode(i)->pParent->pFirst == GetNode(i))
		pp = &(GetNode(i)->pParent->pFirst);
	else
		pp = &(GetNode(i)->pParent->pSecond);

	EraseNode(pp);
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::ReverseErase(typename SortedBinaryTree<Key, Data, Allocator>::ReverseIterator i)
{
	Erase(i.Iterator());
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline int TreeBuilder<Key, Data, Allocator, ElementPolicy>::Erase(const Key& key)
{
	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	if (!ElementPolicy<Base>::MULTIPLE)
	{
		// There can be only one...
		// Look down the tree and delete if found.
		Node** pp = &m_pRoot;

		while (true)
		{
			if ((*pp) != SentinelPointer())
			{
				if (key < (*pp)->m_element)
					pp = &(*pp)->pFirst;
				else if ((*pp)->m_element < key)
					pp = &(*pp)->pSecond;
				else
				{
					EraseNode(pp);
					return 1;
				}
			}
			else
				return 0;
		}
	}
	else
	{
		// Find the first match, then delete all subsequent items that also match.
		// May not be the quickest way, but at least it's simple.
		Iterator i = Find(key);
		int count = 0;

		if (i != End()) 
		{
			do
			{
				Iterator previous = i;
				++i;
				count++;
				Erase(previous);

			} while (i != End() && !(key < *i));
		} 

		return count;
	}
}

/// Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.
template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::Iterator TreeBuilder<Key, Data, Allocator, ElementPolicy>::Find(const Key& key) 
{
	Node* pNode = m_pRoot;

	while (pNode != SentinelPointer())
	{
		if (key < pNode->m_element)
			pNode = pNode->pFirst;
		else if (pNode->m_element < key)
			pNode = pNode->pSecond;
		else
		{
			if (!ElementPolicy<Base>::MULTIPLE)
			{
				if (pNode->pFirst != SentinelPointer() && !(pNode->pFirst->m_element < key))
					pNode = pNode->pFirst;
				else
				{
					// to do: there's a quicker way to get the first match than iterating backwards
					Iterator i = Iterator(pNode, this);

					while (true)
					{
						Iterator p = i;
						--i;
						if (i == End() || *i < key)
							return p;
					}
				}
			}
			else
				return Iterator(pNode, this);
		}
	}

	return End();
}

/////////////////////////////////////////////////////////////////
// iterators

template <typename Key, typename Data, typename Allocator>
inline bool SortedBinaryTree<Key, Data, Allocator>::ConstIterator::operator != (const ConstIterator& i) const 
{
	return m_pNode != i.m_pNode;
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::ConstIterator& SortedBinaryTree<Key, Data, Allocator>::ConstIterator::operator ++()
{
	typename SortedBinaryTree::Node* pNode = m_pNode;
	if (pNode->pSecond != m_pTree->SentinelPointer())
	{
		pNode = pNode->pSecond;

		while (pNode->pFirst != m_pTree->SentinelPointer())
		{
			pNode = pNode->pFirst;
		};
	}
	else
	{
		while (pNode != m_pTree->SentinelPointer())
		{
			if (pNode == pNode->pParent->pFirst)
			{
				m_pNode = pNode->pParent;
				return *this;
			} 
			else
			{
				pNode = pNode->pParent;
			}
		};
	}
	m_pNode = pNode;

	return *this;
}


/// @} end group

} // Ut
#endif
