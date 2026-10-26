#ifndef SSS_STL_ALLOC_H
#define SSS_STL_ALLOC_H

template <typename T> class SSSStlAllocator;
template <typename T> class AftkArrayAllocator;

template <> class SSSStlAllocator<void>
{
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    // reference to void members are impossible.
    typedef void value_type;
    template <class U> 
        struct rebind { typedef SSSStlAllocator<U> other; };
};    

namespace SSSStlAlloc{
    inline void destruct(char *){}
    inline void destruct(wchar_t*){}
    template <typename T> 
        inline void destruct(T *t){t->~T();}
} // namespace
    
template <typename T>
class SSSStlAllocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template <class U> 
        struct rebind { typedef SSSStlAllocator<U> other; };
    SSSStlAllocator(){}
    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}
    pointer allocate(size_type size, SSSStlAllocator<void>::const_pointer hint = 0)
    {
        return static_cast<pointer>(
			AftkArrayAllocator<T>::Allocate(size*sizeof(T))
		);
    }
#ifndef GCC
    //for Dinkumware:
    char *_Charalloc(size_type n){return static_cast<char*>(mem_.allocate(n));}
    // end Dinkumware
#endif

    template <class U> SSSStlAllocator(const SSSStlAllocator<U>&){}
    void deallocate(pointer p, size_type n)
    {
        AftkArrayAllocator<T>::Free(p, n);
    }
    void deallocate(void *p, size_type n)
    {
        AftkArrayAllocator<T>::Free(p, n);
    }
    size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}
    void construct(pointer p, const T& val)
    {
        new(static_cast<void*>(p)) T(val);
    }
    void construct(pointer p)
    {
        new(static_cast<void*>(p)) T();
    }
    void destroy(pointer p){SSSStlAlloc::destruct(p);}
private:
};

template <typename T, typename U>
inline bool operator==(const SSSStlAllocator<T>&, const SSSStlAllocator<U>){return true;}

template <typename T, typename U>
inline bool operator!=(const SSSStlAllocator<T>&, const SSSStlAllocator<U>){return false;}


// For VC6/STLPort 4-5-3 see /stl/_alloc.h, line 464
// "If custom allocators are being used without member template classes support :
// user (on purpose) is forced to define rebind/get operations !!!"
#ifdef _WIN32
#define SSSStlAlloc_CDECL __cdecl
#else
#define SSSStlAlloc_CDECL
#endif

namespace std{
template <class _Tp1, class _Tp2>
inline SSSStlAllocator<_Tp2>& SSSStlAlloc_CDECL
__stl_alloc_rebind(SSSStlAllocator<_Tp1>& __a, const _Tp2*) 
{  
    return (SSSStlAllocator<_Tp2>&)(__a); 
}


template <class _Tp1, class _Tp2>
inline SSSStlAllocator<_Tp2> SSSStlAlloc_CDECL
__stl_alloc_create(const SSSStlAllocator<_Tp1>&, const _Tp2*) 
{ 
    return SSSStlAllocator<_Tp2>(); 
}

} // namespace std
// end STLPort

#endif

