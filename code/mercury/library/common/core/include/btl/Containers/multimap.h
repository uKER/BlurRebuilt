#ifndef BTL_MULTIMAP_H
#define BTL_MULTIMAP_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// MultiMap
////////////////////////////////////////////////////////////////

#include "sortedbinarytree.h"

namespace Btl
{

/// @addtogroup Containers
/// @{

/// %MultiMap container class.
/// A sorted container which, unlike Map, can hold duplicate items.
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. StandardAllocator<MapItem>.
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
		CopyBinaryTreeElements(*this, other);
	}

	MultiMap& operator = (const MultiMap& other)
	{
		if (this == &other) return *this;
		Clear();
		CopyBinaryTreeElements(*this, other);
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

	friend void Swap(MultiMap& left, MultiMap& right) {left.Swap(right);} ///< @a
};

/// @} end group

}

#endif
