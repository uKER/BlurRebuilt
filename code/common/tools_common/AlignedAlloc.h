// Memory Allocation Functions
inline void* AllocAligned(size_t size)
{
#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif
#ifdef GCC
      void *p = memalign(L1_CACHE_LINE_SIZE, size);
#else
      void *p = _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#endif
      if (!p)
            throw std::bad_alloc();

      return p;
}
inline void FreeAligned(void *ptr)
{
#ifdef GCC
      free(ptr);
#else
      _aligned_free(ptr);
#endif
}

template<class T>
T* Align16New(size_t size) 
{
	T* p = AllocAligned(size);
	return new (p)T; 
}

template<class T>
void Align16Delete(T* p)
{	
	p->~();
	FreeAligned(p);	
}

