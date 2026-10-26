///////////////////////////////////////////////////
// List helpers
///////////////////////////////////////////////////

#pragma once

#include "utility.h"
#include "pool.h"

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
class CopyInitialiser : public AllocatorType
{
public:
	StoredType& Initialise(T& item) 
	{
		StoredType* p = AllocatorType::Allocate();
		assert(p);
		p->m_element = item;
		return *p;
	}

	StoredType& Initialise(const T& item) 
	{
		StoredType* p = AllocatorType::Allocate();
		assert(p);
		p->m_element = item;
		return *p;
	}

	void Release(StoredType* item)
	{
		AllocatorType::Free(item);
	}
};

template <class T, class StoredType, typename AllocatorType>
static void Swap(CopyInitialiser<T, StoredType, AllocatorType>& left, CopyInitialiser<T, StoredType, AllocatorType>& right) 
{
	Swap(static_cast<AllocatorType&>(left), static_cast<AllocatorType&>(right));
}

} // End UT

