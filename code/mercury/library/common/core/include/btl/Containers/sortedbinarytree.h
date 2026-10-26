#ifndef BTL_SORTEDBINARYTREE_H
#define BTL_SORTEDBINARYTREE_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////////////////
// Sorted Binary Tree
/////////////////////////////////////////////////////////////////

#include "btl/utility/types.h"
#include "btl/utility/pair.h"
#include "btl/containers/initialiser.h"
#include <btl/containers/sequence.h>

namespace Btl
{
/// @addtogroup Containers
/// @{

// Sorted Binary Tree
//
// Implemented as Andersson Tree (which is self balancing and simpler than red-black).
// Data items may be unique (Set and Map), i.e. one copy of each key, or not (MultiSet, MultiMap).
// Insert, erase, find are O(log(N)).
//
// Template parameters:
//  Data is the type stored in the tree.
//  Key is the type used for comparisons, which may be the same or different to Data.
//
//  'less than' operator must be available:
//    Key < Key
//
// See Set / Map / MultiSet / MultiMap below.

/// Base class for Set, MultiSet, Map and MultiMap.
template <typename Key, typename Data, typename Allocator>
class SortedBinaryTree
{
protected:
	struct Node; // JASOND: This had to be moved up here because the intel compiler generates an error in ConstIterator otherwise.
	
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
		ConstIterator& operator --(); ///< @a 

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

/// Base class for Set, MultiSet, Map and MultiMap.
/// Inserting and deleting items is O(log(N)) where N is the number of items in the container.
/// Iterators are not invalidated by inserting or deleting other items.
template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
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

	typedef typename ElementPolicy<Iterator>::InsertionReturnType InsertionReturnType;

	/// Insert an element.
	/// Set and Map return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// MultiSet and MultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(const KeyType& item); 

	/// Insert an element with a hint to the location.
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	/// Set and Map return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// MultiSet and MultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(Iterator hint, const KeyType& item);

	// Insert multiple elements.
	template <typename InputIterator>
	void Insert(InputIterator begin, InputIterator end)
	{
		for (; begin != end; begin++)
			Insert(*begin);
	}

	/// Deletes all items matching the key and returns the number of items deleted.
	int Erase(const Key& key); 

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

	void Validate() const;

private:
	void Validate(const Node* pNode, const Node* pParent) const;
	InsertionReturnType Insert(Node** pp, Node* pParent, const KeyType& item);
};

template <typename Tree>
void CopyBinaryTreeElements(Tree& tree1, const Tree& tree2)
{
	typename Tree::ConstIterator end = tree2.End();
	for (typename Tree::ConstIterator it = tree2.Begin(); it != end; ++it)
		tree1.Insert(*it);
}

/// @}

#include "sortedbinarytree.inl"

}

#endif
