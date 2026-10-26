#ifndef BTL_INITIALISER_H
#define BTL_INITIALISER_H
#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////
// List helpers
///////////////////////////////////////////////////

namespace Btl
{

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
	StoredType& Initialise(typename StripConst<T>::Result& item) 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		BTL_ASSERT(p);
		new (p) StoredType(item);
		return *p;
	}

	StoredType& Initialise(const T& item) 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		BTL_ASSERT(p);
		new (p) StoredType(item);
		return *p;
	}

	StoredType& Initialise() 
	{
		StoredType* p = (StoredType*) allocator.Allocate();
		BTL_ASSERT(p);
		new (p) StoredType;
		return *p;
	}

	void Release(StoredType* item)
	{
		if (item) 
		{
			item->~StoredType(); // since SameSizeAndAlignment<> has no destructor
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
