///////////////////////////////////////////////////
// List helpers
///////////////////////////////////////////////////

#ifndef __UT_INITIALISER_H__
#define __UT_INITIALISER_H__

#if defined( COMPILER_VC )
#pragma once
#endif

#include <core/utility/utility.h>
#include <core/allocators/pool.h>

namespace Ut {

// Does nothing
template <class T, class StoredType>
class ReferenceInitialiser
{
public:
	StoredType& Initialise(T& item) {return item;}
	void Release(StoredType* item) {}
};

// Copies the elements
// default parameters in forward definition
//template <class T, class StoredType, typename AllocatorType = DynamicPool<StoredType> >
template <class T, class StoredType, typename AllocatorType>
class CopyInitialiser
{
private:
	typedef typename AllocatorType::template Rebind<SameSizeAndAlignment<StoredType> >::Other Allocator;
	Allocator allocator;

public:
	StoredType& Initialise(Ut::StripConst<T>& item) 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		assert(p);
		new (p) StoredType(item);
		return *p;
	}

	StoredType& Initialise(const T& item) 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		assert(p);
		new (p) StoredType(item);
		return *p;
	}

	StoredType& Initialise() 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		assert(p);
		new (p) StoredType;
		return *p;
	}

	void Release(StoredType* item)
	{
		if (item) 
		{
			item->~StoredType();
			allocator.Free((SameSizeAndAlignment<StoredType>*) item);
		}
	}

	void Swap2(CopyInitialiser& other)
	{
		Swap(allocator, other.allocator);
	}
};

template <class T, class StoredType, typename AllocatorType>
static void Swap(CopyInitialiser<T, StoredType, AllocatorType>& left, CopyInitialiser<T, StoredType, AllocatorType>& right) 
{
	left.Swap2(right);
}

} // End UT

#endif
