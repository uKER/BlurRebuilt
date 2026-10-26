// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

template <typename T> inline 
T* bdAllocate2(const bdUWord n)
{
	return reinterpret_cast<T*>(bdMemory::allocate2(n * sizeof(T)));
}

template <typename T> inline 
void bdDeallocate2(T *p)
{
	bdMemory::deallocate2(p);
}

template <typename T> inline 
T* bdReallocate2(T *p,
				const bdUWord n)
{
	return reinterpret_cast<T*>(bdMemory::reallocate2(p, n * sizeof(T)));
}

template <typename T> inline 
T* bdAlignedAllocate2(const bdUWord n, 
					 const bdUWord align)
{
	return reinterpret_cast<T*>(bdMemory::alignedAllocate2(n * sizeof(T), align));
}

template <typename T> inline 
void bdAlignedDeallocate2(T *p)
{
	bdMemory::alignedDeallocate2(p);
}


template <typename T> inline 
T* bdAlignedReallocate2(T *p,
					   const bdUWord n,
					   const bdUWord align)
{
	return reinterpret_cast<T*>(bdMemory::alignedReallocate2(p, n * sizeof(T), align));
}


