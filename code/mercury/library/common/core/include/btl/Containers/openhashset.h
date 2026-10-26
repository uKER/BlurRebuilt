#ifndef BTL_OPENHASHSET_H
#define BTL_OPENHASHSET_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Hash Set
//////////////////////////////////////////////////////

#include "btl/containers/openhashtable.h"
#include "btl/containers/setitem.h"

/////////////////////////////////////////////////////////

namespace Btl 
{

/// @addtogroup Containers
/// @{

	//OpenHashTableEntry<Key, ItemType>
	/// HashSet container class.
	/// Elements in a set are unique.
	/// @param Key The type stored in the set.
	/// @param Allocator The allocator type, e.g. StandardAllocator<Key>.
	template <
		typename Key, 
		typename HashKeyOp = HashKeyOpDefault<Key>,
		typename Probe = OpenHashTableQuadraticProbe<Key, SetItem<Key>, HashKeyOp>,
		typename Allocator = ArrayAllocator<OpenHashTableEntry<Key, SetItem<Key> > > >
	class OpenHashSet : public 
		OpenHashTable<
			Key,
			SetItem<Key>, 
			HashKeyOp,
			Probe,
			Allocator>
	{
	public:
		OpenHashSet() {} ///<

		OpenHashSet(const OpenHashSet& set) : 
			OpenHashTable<
				Key,
				SetItem<Key>, 
				HashKeyOp,
				Probe,
				Allocator> (set) {}
	};

/// @}

}

#endif
