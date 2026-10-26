#ifndef BTL_SET_H
#define BTL_SET_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// Set
////////////////////////////////////////////////////////////////

#include "btl/allocators/allocator.h"
#include "sortedbinarytree.h"
#include "setitem.h"

namespace Btl
{

/// @addtogroup Containers
/// @{

/// %Set container class.
/// Elements in a set are unique and sorted according to the < operator.
///
/// @code
/// typedef Set<String> MySet; // define a set type
/// MySet set; // create a set
/// set.Insert(String("test")); // insert an item
/// @endcode
///
/// @param Key The type stored in the set.
/// @param Allocator The allocator type, e.g. StandardAllocator<Key>.
template <typename Key, typename Allocator = StandardAllocator<NullType> >
class Set : public TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements>
{
public:
	typedef TreeBuilder<Key, SetItem<Key, Allocator>, Allocator, UniqueElements> Base;
	using Base::Initialise;
	using Base::Clear;
	using Base::InsertionReturnType;

	Set() {}
	Set(const Set& other) 
	{
		CopyBinaryTreeElements(*this, other);
	}

	Set& operator = (const Set& other)
	{
		if (this == &other) return *this;
		Clear();
		CopyBinaryTreeElements(*this, other);
		return *this;
	}

	// swap needs to be an exact match so it can't be in the base class
	friend void Swap(Set& left, Set& right) {left.Swap(right);} ///< @a
};

/// @} end group

}

#endif
