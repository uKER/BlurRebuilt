#ifndef BTL_MAP_H
#define BTL_MAP_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// Map
////////////////////////////////////////////////////////////////

#include "sortedbinarytree.h"
#include "setitem.h"
#include "btl/allocators/allocator.h"

namespace Btl
{

/// @addtogroup Containers
/// @{

/// %Map container class.
/// Elements in a map are unique and sorted according to the < operator on the keys.
/// The type stored in the map is MapItem<Key, Value>.
///
/// @code
/// typedef Map<U32, std::string> MyMap; // define a map type
/// MyMap map; // create a map
/// map.Insert(MapItem(0, std::string("test"))); // insert an item with key 0
/// @endcode
///
/// @param Key The sort key for items stored in the map.
/// @param Value The value type for items stored in the map.
/// @param Allocator The allocator type, e.g. StandardAllocator<MapItem>.
template <typename Key, typename Value, typename Allocator = StandardAllocator<NullType> >
class Map : public TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements>
{
public:
	typedef TreeBuilder<Key, MapItem<Key, Value, Allocator>, Allocator, UniqueElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	Map() {}
	Map(const Map& other) 
	{
		CopyBinaryTreeElements(*this, other);
	}

	Map& operator = (const Map& other)
	{
		if (this == &other) return *this;
		Clear();
		CopyBinaryTreeElements(*this, other);
		return *this;
	}

	/// Returns a reference to the value associated with the key, inserting a new item if necessary.
	Value& operator [] (const Key& key)
	{
		return (*Base::Insert(key).iterator).GetValue(); 
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

	typename Base::InsertionReturnType Insert(const Key& key)
	{
		return Base::Insert(key);
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
	/// If the location immediately follows the hint, the insertion can be done in constant rather than logarithmic time.@n
	/// Returns a Pair containing an iterator to the element, and a bool which is true if an insertion was made.
	typename Base::InsertionReturnType Insert(typename Base::Iterator hint, const typename Base::DataType& item)
	{
		typename Base::InsertionReturnType r = Base::Insert(hint, item.GetKey());
		if (r.inserted)
			r.iterator->GetValue() = item.GetValue();

		return r;
	}

	friend void Swap(Map& left, Map& right) {left.Swap(right);}  ///< @a
};

/// @} end group

}

#endif
