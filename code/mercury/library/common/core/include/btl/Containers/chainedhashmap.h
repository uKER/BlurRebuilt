#ifndef BTL_CHAINEDHASHMAP_H
#define BTL_CHAINEDHASHMAP_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Chained Hash Map
//////////////////////////////////////////////////////

#include "chainedhashtable.h"
#include "setitem.h"

/////////////////////////////////////////////////////////

namespace Btl 
{

/// @addtogroup Containers
/// @{

/// %ChainedHashMap container class.
/// Associative container with unique keys.
/// The type stored in the map is MapItem<Key, Value>.
/// @param Key The key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type.
template <typename Key, typename Value, typename HashKeyOp = HashKeyOpDefault<Key>, typename Allocator =

#if !PLATFORM_SPU
		StandardAllocator<NullType>
#else
		LinearAllocator<NullType>
#endif
	
	>
class ChainedHashMap : public ChainedHashTable<Key, MapItem<Key, Value>, 
												ChainedHashTable_Parameters<UniqueElements, HashKeyOp, Allocator> >
{
public:
	typedef ChainedHashTable<Key, MapItem<Key, Value>, ChainedHashTable_Parameters<UniqueElements, HashKeyOp, Allocator> > Base;

	/// Returns a reference to the value associated with the key, inserting a new item if necessary.
	Value& operator [] (const Key& key)
	{
		return (*Base::Insert(key).iterator).GetValue();
	}

	typename Base::InsertionReturnType Insert(const Key& key)
	{
		return Base::Insert(key);
	}

	/// Insert an element.
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(item.GetKey());
		if (r.inserted) 
			r.iterator->GetValue() = item.GetValue();

		return r;
	}

	/// Insert an element.
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(const typename Base::KeyType& key, const typename Base::DataType::ValueType& value)
	{
		typename Base::InsertionReturnType r = Base::Insert(key);
		if (r.inserted) 
			r.iterator->GetValue() = value;

		return r;
	}

	friend void Swap(ChainedHashMap& left, ChainedHashMap& right) {left.Swap(right);}  ///< @a
};

////////////////////////////////////////////////////////////////////
// Multimap

/// %ChainedHashMap container class.
/// Associative container which can store multiple elements with the same key.
/// The type stored in the map is MapItem<Key, Value>.
/// @param Key The key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type.
template <typename Key, typename Value, typename HashKeyOp = HashKeyOpDefault<Key>, typename Allocator =

#if !PLATFORM_SPU
		StandardAllocator<NullType>
#else
		LinearAllocator<NullType>
#endif

	>
class ChainedHashMultiMap : public ChainedHashTable<Key, MapItem<Key, Value>, 
													ChainedHashTable_Parameters<MultipleElements, HashKeyOp, Allocator> >
{
public:
	typedef ChainedHashTable<Key, MapItem<Key, Value>, ChainedHashTable_Parameters<MultipleElements, HashKeyOp, Allocator> > Base;

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

	friend void Swap(ChainedHashMultiMap& left, ChainedHashMultiMap& right) {left.Swap(right);} ///< @a
};

/// @}

}

#endif
