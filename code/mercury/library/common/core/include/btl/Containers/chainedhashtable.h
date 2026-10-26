#ifndef BTL_CHAINED_HASH_TABLE_H
#define BTL_CHAINED_HASH_TABLE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "vector.h"
#include "list.h"
#include "btl/utility/pair.h"
#include "hashops.h"

//////////////////////////////////////////////////////////
// Chained Hash Table
//////////////////////////////////////////////////////////

namespace Btl
{

template <	template <typename Iterator> class ElementPolicy, 
			typename HashKeyOp,
			typename Allocator>
struct ChainedHashTable_Parameters
{
	template <typename Iterator>
	struct ElementPolicyType
	{
		typedef ElementPolicy<Iterator> Result;
	};

	typedef HashKeyOp HashKeyOpType;
	typedef Allocator AllocatorType;
};

/// Base type for ChainedHashSet, ChainedHashMultiSet, ChainedHashMap and ChainedHashMultiMap.
/// 
/// Chained vs. open hash tables:
/// @li Chained hash tables are more space efficient for large items (>16 bytes). 
///		However, the system allocator will typically add another 16 bytes per item.
///		With a pool there is no extra overhead per item but but there is an overhead depending on the allocation size of the pool.
/// @li Open hash tables on average have slightly quicker insertion and look-up times.
/// @li Open hash tables can be affected by clustering (e.g. due to a poor hash function).
/// @li Chained hash tables have better performance when deleting elements.
template <typename Key, typename Data, typename Parameters>
class ChainedHashTable
{
public:

	typedef Data DataType;
	typedef Key KeyType;

	struct EntryType : public ListElementSingle<EntryType>
	{
		EntryType(const KeyType& other) : item(other) {}

		DataType item;
		U32 hash;
	};

	// simple list type giving a memory overhead of 4 bytes per item (allocator may add more)
	typedef List<EntryType, ListParameters<SingleEndedList, OnDemandCounter, ListElementSingle> > ListType;
	typedef Vector<ListType> VectorType;

	///////////////////////////////////////////////////////////
	// constructors

	ChainedHashTable() {}
	ChainedHashTable(int tableSize) {m_table.Resize(tableSize);}
	ChainedHashTable(const ChainedHashTable& other) {CopyElements(other);}

	~ChainedHashTable() {Clear();}

	ChainedHashTable& operator = (const ChainedHashTable& other) {CopyElements(other); return *this;}
	void Swap(ChainedHashTable& other);

	/////////////////////////////////////////////////////////
	// Iterators

	/// %Iterator for ChainedHashTable
	class ConstIterator
	{
	public:
		// constructors
		ConstIterator() {}
		ConstIterator(	const typename VectorType::Iterator& v,
						const typename VectorType::Iterator& vend,
						const typename ListType::Iterator& l) : m_vectorIterator(v), m_vectorEnd(vend), m_listIterator(l) {}

		// comparison
		bool operator == (const ConstIterator& i) const {return m_vectorIterator == i.m_vectorIterator && m_listIterator == i.m_listIterator;} ///< @a
		bool operator != (const ConstIterator& i) const {return m_vectorIterator != i.m_vectorIterator || m_listIterator != i.m_listIterator;} ///< @a

		// return T
		const DataType& operator*() const {return m_listIterator->item;} ///< @a
		const DataType* operator->() const {return &m_listIterator->item;} ///< @a

		// arithmetic
		ConstIterator& operator ++(); ///< @a
		ConstIterator operator ++(int) {ConstIterator i = *this; ++(*this); return i;} ///< @a

	protected:
		typename VectorType::Iterator m_vectorIterator;
		typename VectorType::Iterator m_vectorEnd;
		typename ListType::Iterator m_listIterator;

		FRIEND_CLASS ChainedHashTable;
	};

	/// %Iterator for ChainedHashTable
	class Iterator : public ConstIterator
	{
	public:
		// constructors
		Iterator() {}
		Iterator(	const typename VectorType::Iterator& v,
					const typename VectorType::Iterator& vend,
					const typename ListType::Iterator& l) : ConstIterator(v, vend, l) {}

		// return T
		DataType& operator*() {return Iterator::m_listIterator->item;} ///< @a
		DataType* operator->() {return &Iterator::m_listIterator->item;}///< @a

		// arithmetic
		Iterator& operator ++() {ConstIterator::operator++(); return *this;} ///< @a
		Iterator operator ++(int) {Iterator i = *this; ++(*this); return i;} ///< @a

		FRIEND_CLASS ChainedHashTable;
	};

	// make iterators
	Iterator Begin(); ///< %Iterator pointing to the first element.
	ConstIterator Begin() const {return const_cast<ChainedHashTable*>(this)->Begin();} ///< %Iterator pointing to the first element.
	Iterator End(); ///< %Iterator pointing past the last element.
	ConstIterator End() const {return const_cast<ChainedHashTable*>(this)->End();} ///< %Iterator pointing past the last element.

	//////////////////////////////////////////////////////////
	// information

	bool Empty() const {return Size() == 0;} ///< Returns true if the table is empty.
	int Size() const {return m_counter.Count(*this);} ///< Returns the number of items in the table.

	////////////////////////////////////////////////////////////
	// add / remove elements

	typedef typename Parameters::template ElementPolicyType<Iterator>::Result::InsertionReturnType InsertionReturnType;
	typedef typename Parameters::HashKeyOpType HashKeyOpType;
	static const bool MULTIPLE = Parameters::template ElementPolicyType<Iterator>::Result::MULTIPLE;

	/// Erase all items.
	void Clear();

	/// Deletes all items matching the key.
	/// Returns the number of items deleted.
	int Erase(const Key& key);

	/// Erases the item pointed to by the iterator.
	/// The iterator must point to a valid item in the tree, or the results are undefined.
	void Erase(Iterator i);

	/////////////////////////////////////////////////////////////////
	// searching

	/// Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.
	Iterator Find(const Key& key); 
	ConstIterator Find(const Key& key) const {return const_cast<ChainedHashTable*>(this)->Find(key);}

	/////////////////////////////////////////////////////////////

protected:
	/// Insert an element.
	/// ChainedHashSet and ChainedHashMap return a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	/// ChainedHashMultiSet and ChainedHashMultiMap return an iterator to the inserted element.
	InsertionReturnType Insert(const Key& key, bool allowResize = true);

private:
	void CopyElements(const ChainedHashTable& other);
	void ResizeTable(int newSize);
	int TableSize() const {return m_table.Size();}

	typedef CopyInitialiser<KeyType, EntryType, typename Parameters::AllocatorType> Initialiser;

	VectorType m_table;
	MaintainedCounter m_counter;
	Initialiser m_initialiser;
};

///////////////////////////////////////////////////////////////////////////

template <typename Key, typename Data, typename Parameters>
typename ChainedHashTable<Key, Data, Parameters>::Iterator ChainedHashTable<Key, Data, Parameters>::Find(const Key& key)
{
	if (m_table.Size() > 0)
	{
		U32 hash = HashKeyOpType::GetHash(key);
		U32 index = hash % TableSize();

		for (typename ListType::Iterator i = m_table[index].Begin(); i != m_table[index].End(); ++i)
			if (i->hash == hash && HashKeyOpType::Equal(i->item.GetKey(), key))
				return Iterator(m_table.MakeIterator(index), m_table.End(), i);
	}

	return End();
}

template <typename Key, typename Data, typename Parameters>
void ChainedHashTable<Key, Data, Parameters>::Erase(Iterator i)
{
	EntryType* p = &*i.m_listIterator;
	i.m_vectorIterator->EraseSingleLinked(i.m_listIterator);
	m_initialiser.Release(p);
	m_counter.EraseElement();
}

template <typename Key, typename Data, typename Parameters>
int ChainedHashTable<Key, Data, Parameters>::Erase(const Key& key)
{
	if (Empty()) return 0;

	U32 hash = HashKeyOpType::GetHash(key);
	U32 index = hash % TableSize();
	U32 count = 0;

	typename ListType::Iterator i = m_table[index].Begin();
	
	// single linked list only supports PopFront and EraseNext
	while (i != m_table[index].End() && i->hash == hash && HashKeyOpType::Equal(i->item.GetKey(), key))
	{
		EntryType* p = &m_table[index].Front();
		m_table[index].PopFront();
		m_initialiser.Release(p);
		i = m_table[index].Begin();
		count++;
	}

	if (i != m_table[index].End())
	{
		typename ListType::Iterator previous = i; ++i;

		while (i != m_table[index].End())
		{
			if (i->hash == hash && HashKeyOpType::Equal(i->item.GetKey(), key))
			{
				EntryType* p = &*i;
				m_table[index].EraseNext(previous);
				m_initialiser.Release(p);
				i = previous; ++i;
				count++;
			}
			else
			{
				previous = i; ++i;
			}
		}
	}

	m_counter.Subtract(count);
	return count;
}

template <typename Key, typename Data, typename Parameters>
void ChainedHashTable<Key, Data, Parameters>::Clear()
{
	for (typename VectorType::Iterator i = m_table.Begin(); i != m_table.End(); ++i)
	{
		while (!i->Empty())
		{
			EntryType* p = &i->Front();
			i->PopFront();
			m_initialiser.Release(p);
		}
	}

	m_table.Clear();
	m_counter.Reset();
}

template <typename Key, typename Data, typename Parameters>
void ChainedHashTable<Key, Data, Parameters>::CopyElements(const ChainedHashTable& other)
{
	if (this == &other) return;

	Clear();
	ConstIterator end = other.End();
	for (ConstIterator it = other.Begin(); it != end; ++it)
		Insert(*it);
}

template <typename Key, typename Data, typename Parameters>
typename ChainedHashTable<Key, Data, Parameters>::Iterator ChainedHashTable<Key, Data, Parameters>::Begin() 
{
	for (typename VectorType::Iterator i = m_table.Begin(); i != m_table.End(); ++i)
	{
		if (!i->Empty())
			return Iterator(i, m_table.End(), i->Begin());
	}

	return End();
}

template <typename Key, typename Data, typename Parameters>
typename ChainedHashTable<Key, Data, Parameters>::Iterator ChainedHashTable<Key, Data, Parameters>::End()
{
	return Iterator(m_table.End(), m_table.End(), typename ListType::Iterator(0));
}

template <typename Key, typename Data, typename Parameters>
void ChainedHashTable<Key, Data, Parameters>::ResizeTable(int newSize)
{
	VectorType newTable;
	newTable.Resize(newSize);

	for (typename VectorType::Iterator i = m_table.Begin(); i != m_table.End(); ++i)
	{
		while (!i->Empty())
		{
			EntryType* p = &i->Front();
			i->PopFront();
			U32 index = p->hash % newSize;
			newTable[index].PushFront(*p);
		}
	}

	SwapByMemcopy(m_table, newTable);
}

template <typename Key, typename Data, typename Parameters>
typename ChainedHashTable<Key, Data, Parameters>::InsertionReturnType ChainedHashTable<Key, Data, Parameters>::Insert(const KeyType& key, bool allowResize)
{
	// decide whether to resize the table
	if (allowResize && (Size() + 1 > TableSize() * 2))
	{
		ResizeTable(Size() + 1);
		return Insert(key, false); // safe since resize does not move list elements
	}

	BTL_ASSERT(m_table.Size() > 0);
	U32 hash = HashKeyOpType::GetHash(key);
	U32 index = hash % TableSize();

	typedef typename Parameters::template ElementPolicyType<Iterator>::Result ElementPolicyType;

	if (!MULTIPLE)
	{
		// look for a match first
		for (typename ListType::Iterator i = m_table[index].Begin(); i != m_table[index].End(); ++i)
			if (i->hash == hash && HashKeyOpType::Equal(i->item.GetKey(), key))
				return ElementPolicyType::MakeInsertionReturnType(Iterator(m_table.MakeIterator(index), m_table.End(), i), false);
	}

	m_counter.AddElement();
	EntryType& newItem = m_initialiser.Initialise(key);
	newItem.hash = hash;
	m_table[index].PushFront(newItem);
	return ElementPolicyType::MakeInsertionReturnType(Iterator(m_table.MakeIterator(index), m_table.End(), m_table[index].Begin()), true);
}

template <typename Key, typename Data, typename Parameters>
void ChainedHashTable<Key, Data, Parameters>::Swap(ChainedHashTable<Key, Data, Parameters>& other)
{
	SwapByMemcopy(m_table, other.m_table);
	SwapByMemcopy(m_counter, other.m_counter);
	SwapByMemcopy(m_initialiser, other.m_initialiser);
}

template <typename Key, typename Data, typename Parameters>
typename ChainedHashTable<Key, Data, Parameters>::ConstIterator& ChainedHashTable<Key, Data, Parameters>::ConstIterator::operator ++()
{
	++m_listIterator;

	while (m_listIterator == m_vectorIterator->End())
	{
		if (++m_vectorIterator == m_vectorEnd)
		{
			m_listIterator = typename ListType::Iterator(0);
			return *this;
		}

		m_listIterator = m_vectorIterator->Begin();
	}

	return *this;
}

}

#endif
