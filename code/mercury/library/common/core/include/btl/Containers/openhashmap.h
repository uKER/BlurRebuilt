#ifndef BTL_OPENHASHMAP_H
#define BTL_OPENHASHMAP_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Hash Map
//////////////////////////////////////////////////////

#include "btl/containers/openhashtable.h"
#include "btl/containers/setitem.h"

/////////////////////////////////////////////////////////

namespace Btl 
{

/// @addtogroup Containers
/// @{

	/// %HashMap container class.
	/// Elements in a map are unique and sorted according to the < operator on the keys.
	/// The type stored in the map is MapItem<Key, Value>.
	/// @param Key The sort key for items stored in the map.
	/// @param Value The value type for items stored in the map.
	/// @param Allocator The allocator type, e.g. StandardAllocator<MapItem>.
	template <
		typename Key, 
		typename Value, 
		typename HashKeyOp = HashKeyOpDefault<Key>,
		typename Probe = OpenHashTableQuadraticProbe<Key, MapItem<Key, Value>, HashKeyOp>,
		typename Allocator = ArrayAllocator<OpenHashTableEntry<Key, MapItem<Key, Value> > > >
	class OpenHashMap : public OpenHashTable<
		Key, 
		MapItem<Key, Value>, 
		HashKeyOp, 
		Probe, 
		Allocator>
	{
	public:
		///\brief Constructor
		OpenHashMap() {}

		///\brief Constructor
		OpenHashMap(const OpenHashMap& map) : 
			OpenHashTable<
				Key,
				MapItem<Key, Value>, 
				HashKeyOp,
				Probe,
				Allocator> (map) 
		{
		}

		typedef OpenHashTable<Key, MapItem<Key, Value>, HashKeyOp, Probe, Allocator> Base;

		using Base::Insert;

		///\brief Inserts a key into the hash table.
		///
		///@param key The key to insert.
		///@param value The value of the inserted element.
		///@return Returns a pair: The first element is an iterator pointing to the key position. The second element is a boolean indicating whether a new key was inserted (true) or not (false).
		typename Base::InsertionReturnType Insert(const Key& key, const Value& value)
		{
			return Base::Insert(typename Base::DataType(key, value));
		}

		///\brief Returns a reference to the value associated with the key, inserting a new item if necessary.
		Value& operator [] (const Key& key)
		{
			return Insert(MapItem<Key, Value>(key)).iterator->GetValue();
		}
	};

/// @}

}

#endif
