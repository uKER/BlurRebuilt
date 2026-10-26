// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

template <typename T> inline 
T* bdAllocate(const bdUWord n)
{
	return reinterpret_cast<T*>(bdMemory::allocate(n * sizeof(T)));
}

template <typename T> inline 
void bdDeallocate(T *p)
{
	bdMemory::deallocate(p);
}

template <typename T> inline 
T* bdReallocate(T *p,
				const bdUWord n)
{
	return reinterpret_cast<T*>(bdMemory::reallocate(p, n * sizeof(T)));
}

template <typename T> inline 
T* bdAlignedAllocate(const bdUWord n, 
					 const bdUWord align)
{
	return reinterpret_cast<T*>(bdMemory::alignedAllocate(n * sizeof(T), align));
}

template <typename T> inline 
void bdAlignedDeallocate(T *p)
{
	bdMemory::alignedDeallocate(p);
}


template <typename T> inline 
T* bdAlignedReallocate(T *p,
					   const bdUWord n,
					   const bdUWord align)
{
	return reinterpret_cast<T*>(bdMemory::alignedReallocate(p, n * sizeof(T), align));
}


