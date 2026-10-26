#ifndef BTL_MULTISET_H
#define BTL_MULTISET_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// MultiSet
////////////////////////////////////////////////////////////////

#include "sortedbinarytree.h"

namespace Btl
{

/// @addtogroup Containers
/// @{

/// %MultiSet container class.
/// A sorted container which, unlike Set, can hold duplicate items.
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. StandardAllocator<Key>.
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
		CopyBinaryTreeElements(*this, other);
	}

	MultiSet& operator = (const MultiSet& other)
	{
		if (this == &other) return *this;
		Clear();
		CopyBinaryTreeElements(*this, other);
		return *this;
	}

	friend void Swap(MultiSet& left, MultiSet& right) {left.Swap(right);}  ///< @a
};

/// @} end group

}

#endif
