#ifndef BTL_CONTAINERS_H
#define BTL_CONTAINERS_H
#ifdef _MSC_VER
#pragma once
#endif

/// @defgroup Containers
/// Main classes are:
/// @li @ref List
/// @li @ref Vector
/// @li @ref Set
/// @li @ref Map
/// @li @ref MultiSet
/// @li @ref MultiMap
/// @li @ref OpenHashSet
/// @li @ref OpenHashMap
/// @li @ref ChainedHashSet
/// @li @ref ChainedHashMap
/// @li @ref ChainedHashMultiSet
/// @li @ref ChainedHashMultiMap
/// @li @ref BitVector
///
/// Key concepts:
/// @li Iterators, which are used to step through containers.
/// @li Sequences, which contain a pair of iterators.
/// @li Algorithms, which act on sequences.
/// @li Policies, which configure the behaviour of the containers.
/// 
/// This is an STL style container library. The naming conventions are the same
/// as far as possible, although function and class names are capitalised.
/// The containers are slightly lower level than STL and more configurable, 
/// though they also work as a direct replacement.
/// All error checking is done using assert, or with static checks where possible.

#include "list.h"
#include "vector.h"
#include "set.h"
#include "map.h"
#include "multiset.h"
#include "multimap.h"
#include "openhashset.h"
#include "openhashmap.h"
#include "chainedhashset.h"
#include "chainedhashmap.h"
#include "bitvector.h"

#endif
