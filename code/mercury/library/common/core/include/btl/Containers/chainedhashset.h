#ifndef BTL_CHAINEDHASHSET_H
#define BTL_CHAINEDHASHSET_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Chained Hash Set
//////////////////////////////////////////////////////

#include "chainedhashtable.h"
#include "setitem.h"

/////////////////////////////////////////////////////////

namespace Btl 
{

/// @addtogroup Containers
/// @{

/// %ChainedHashSet container class.
/// Elements in a set are unique.
/// @param T The type stored in the set.
/// @param HashKeyOp A class used to evaluate the hash.
/// @param Allocator The allocator type.
template <typename T, typename HashKeyOp = HashKeyOpDefault<T>, typename Allocator = StandardAllocator<NullType> >
class ChainedHashSet : public ChainedHashTable<	T, SetItem<T>, 
												ChainedHashTable_Parameters<UniqueElements, HashKeyOp, Allocator> >
{
public:
	typedef ChainedHashTable<T, SetItem<T>, ChainedHashTable_Parameters<UniqueElements, HashKeyOp, Allocator> > Base;

	typename Base::InsertionReturnType Insert(const T& item)
	{
		return Base::Insert(item);
	}

	friend void Swap(ChainedHashSet& left, ChainedHashSet& right) {left.Swap(right);}  ///< @a
};

////////////////////////////////////////////////////////////////////
// MultiSet

/// %ChainedHashMultiSet container class.
/// Can hold duplicate items.
/// @param T The type stored in the set.
/// @param HashKeyOp A class used to evaluate the hash.
/// @param Allocator The allocator type.
template <typename T, typename HashKeyOp = HashKeyOpDefault<T>, typename Allocator = StandardAllocator<NullType> >
class ChainedHashMultiSet : public ChainedHashTable<T, SetItem<T>, 
													ChainedHashTable_Parameters<MultipleElements, HashKeyOp, Allocator> >
{
public:
	typedef ChainedHashTable<T, SetItem<T>, ChainedHashTable_Parameters<MultipleElements, HashKeyOp, Allocator> > Base;

	typename Base::InsertionReturnType Insert(const T& item)
	{
		return Base::Insert(item);
	}

	friend void Swap(ChainedHashMultiSet& left, ChainedHashMultiSet& right) {left.Swap(right);}  ///< @a
};

/// @}

}

#endif
