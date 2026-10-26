#ifndef BTL_ALLOCATOR_H
#define BTL_ALLOCATOR_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/types.h"

#if !PLATFORM_SPU

#include <new>

namespace Btl
{

/// @defgroup Allocators
/// Allocators can either be used directly or as template parameters for another class. @n
/// Allocators divide into those with static allocation functions (e.g. StandardAllocator or ArrayAllocator)
/// and those with non-static functions such as StaticPool or DynamicPool. With static functions there is no need to 
/// create an allocator object. @n
///	Allocation functions may be for single objects or arrays of objects. @n
/// The allocator will call the constructor when allocating an object, and the destructor when freeing an object. @n
/// %Allocate() returns a null pointer on failure. @n
/// Freeing a null pointer has no effect. @n
/// Freeing an object not allocated with the same allocator will give unpredictable results. @n
/// @{

/// Simple allocator using new and delete.
template <class T>
class StandardAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static T* Allocate() 
	{
		T* p =  new T;
		return p;
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(T* p, int n = 0)
	{
		if (!p) return;
		delete p;
	}

	template <typename U> 
	struct Rebind 
	{
		typedef StandardAllocator<U> Other;
	};
};

/// %AbstractAllocator can't allocate, but can free objects.
/// Useful for pointers to abstract base classes.
template <class T>
class AbstractAllocator
{
public:
	typedef T* PointerType;

	/// Free an object.
	/// The second argument is ignored.
	static void Free(T* p, int n = 0)
	{
		delete p;
	}

	// Rebinds to non-abstract version.
	// Will happily delete a pointer allocated using StandardAllocator.
	template <typename U> struct Rebind {typedef StandardAllocator<U> Other;};
};

/// Array allocator.
template <class T>
class ArrayAllocator
{
public:
	typedef T* PointerType;
	
	/// Allocate n objects of type T.
	static T* Allocate(int n) 
	{
		assert(n > 0);
		const static int alignment = BTL_ALIGNMENT_OF(T);
		
		if (alignment < 16)
		{
			return new T[n];
		}
		else
		{
			T* p = (T*) BizAlignedMalloc(n * sizeof(T), alignment);

			for (int i = 0; i < n; i++)
				new (p + i) T;

			return p;
		}
	}

	/// Free objects.
	static void Free(T* p, int n)
	{
		if (!p) return;

		assert(n > 0);
		const static int alignment = BTL_ALIGNMENT_OF(T);

		if (alignment < 16)
		{
			delete [] p;
		}
		else
		{
			BizAlignedFree(p);

			for (int i = 0; i < n; i++)
				p[i].~T();
		}
	}

	template <typename U> struct Rebind {typedef ArrayAllocator<U> Other;};
};

/// Specialisation for basic types.
// No need to align or pass the number of objects to Free().
#define BASIC_ARRAY_ALLOCATOR(_type)					\
template <>												\
class ArrayAllocator<_type>								\
{														\
public:													\
	typedef _type* PointerType;							\
														\
	/* Allocate n objects of type T.*/					\
	static _type* Allocate(int n)						\
	{													\
		assert(n > 0);									\
		return new _type[n];							\
	}													\
														\
	/*Free objects. The second argument is ignored.*/	\
	static void Free(_type* p, int n = 0)				\
	{													\
		n;												\
		if (!p) return;									\
		delete [] p;									\
	}													\
														\
	template <typename U> struct Rebind {typedef ArrayAllocator<U> Other;}; \
};

BASIC_ARRAY_ALLOCATOR(U8)
BASIC_ARRAY_ALLOCATOR(U16)
BASIC_ARRAY_ALLOCATOR(U32)
BASIC_ARRAY_ALLOCATOR(S8)
BASIC_ARRAY_ALLOCATOR(S16)
BASIC_ARRAY_ALLOCATOR(S32)

/// Array allocator with forced memory alignment.
/// Note that ArrayAllocator will generally align objects correctly.
/// This should only be used for build in types as constructors are not called.
/// @param T The type to be allocated.
/// @param align Alignment value, which must be a power of 2.
template<class T, int ALIGN = 0>
class AlignedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate memory for n objects of type T.
	static T* Allocate(int n, int align = 0) 
	{
		const static int alignT = BTL_ALIGNMENT_OF(T);
		int a = ALIGN;
		if (alignT > a) a = alignT;
		if (align > a) a = align;

		return (T*) BizAlignedMalloc(n * sizeof(T), a);
	}

	/// Free objects.
	/// The second argument is ignored.
	static void Free(T* p, int n = 0)
	{
		if (!p) return;
		BizAlignedFree(p);
	}

	template <typename U> struct Rebind {typedef AlignedAllocator<T, ALIGN> Other;};
};

/// Use to stop a pointer being deleted twice.
/// Clears pointers to 0 after freeing.
template <class T, template <typename U> class BaseAllocator = StandardAllocator>
class ProtectedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static T* Allocate() 
	{
		return BaseAllocator<T>::Allocate();
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(T*& p, int n = 0)
	{
		if (!p) return;
		BaseAllocator<T>::Free(p);
		p = 0;
	}

	template <typename U> struct Rebind {typedef ProtectedAllocator<U, BaseAllocator> Other;};
};

/// Class that will allocate either arrays or single items.
template<class Single, class Array>
class GeneralAllocator
{
public:
	/// Allocate memory for n objects of type T.
	template <typename T>
	static T* Allocate(int n)
	{
		return Array::template Rebind<T>::Other::Allocate(n);
	}

	/// Allocate an object.
	template <typename T>
	static T* Allocate()
	{
		return Single::template Rebind<T>::Other::Allocate();
	}

	/// Free objects.
	template <typename T>
	static void Free(T* p, int n)
	{
		if (!p) return;
		Array::template Rebind<T>::Other::Free(p, n);
	}

	/// Free objects.
	template <typename T>
	static void Free(T* p)
	{
		if (!p) return;
		Single::template Rebind<T>::Other::Free(p);
	}
};

/// With this allocator, you can call %Free() for an incomplete (i.e. forward-declared) type.
template <class T>
class IncompleteAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static T* Allocate()
	{
		// Store a pointer to the delete function.
		// The object must be defined in order to allocate it, so the destructor will be available.
		// Store the pointer to use later in the Free function.
		HoldFnPointer(Delete);
		return new T;
	}

	/// Free an object.
	static void Free(T* p, int n = 0)
	{
		// Object may not yet be defined when this function is defined.
		// Therefore, can't call delete directly as the destructor is not available.
		// However, if p points to a valid object, Allocate() must have been called.
		// In which case HoldFnPointer returns a pointer to the Delete function.
		if (!p) return;
		DeleteFn fn = HoldFnPointer(0);
		fn(p);
	}

	template <typename U> 
	struct Rebind 
	{
		typedef IncompleteAllocator<U> Other;
	};

private:
	typedef void (*DeleteFn)(T* p);

	// The function that actually deletes the object.
	static void Delete(T* p)
	{
		if (!p) return;
		delete p;
	}

	// Holds a pointer to the Delete function.
	static DeleteFn HoldFnPointer(DeleteFn in)
	{
		static DeleteFn fn;
		if (in) fn = in;
		return fn;
	}
};

/// @}
}

#else

namespace Spu {void* Alloc(unsigned size);}

namespace Btl
{

/// Uses the SPU linear allocator.
template <class T>
class LinearAllocator
{
public:
	typedef T* PointerType;
	typedef T U;
	
	/// Allocate an object of type T.
	static T* Allocate()
	{
		U32 size = sizeof(T);
		size = AlignUp(size, 16);
		T* p = (T*) Spu::Alloc(size);
		new (p) T;
		return p;
	}

	/// Free an object.
	/// The memory cannot be freed, but we still call the destructor.
	static void Free(T* p, int n = 0)
	{
		if (p) p->~T();
	}

	template <typename U> struct Rebind {typedef LinearAllocator<U> Other;};
};

/// Array allocator.
template <class T>
class LinearArrayAllocator
{
public:
	typedef T* PointerType;
	typedef T U;
	
	/// Allocate n objects of type T.
	static T* Allocate(int n)
	{
		U32 size = sizeof(T) * n;
		size = AlignUp(size, 16);
		T* p = (T*) Spu::Alloc(size);

		for (int i = 0; i < n; i++)
			new (p + i) T;

		return p;
	}

	/// Free objects.
	/// The memory cannot be freed, but we still call the destructor.
	static void Free(T* p, int n) ALWAYS_INLINE
	{
		if (p)
		{
			assert(n > 0);

			for (int i = 0; i < n; i++)
				p[i].~T();
		}
	}

	template <typename U> struct Rebind {typedef LinearArrayAllocator<U> Other;};
};

/// Aligned allocator for basic types only.
template <class T, int ALIGN = 0>
class LinearAlignedAllocator
{
public:
	typedef T* PointerType;
	typedef T U;
	
	/// Allocate n objects of type T.
	static T* Allocate(int n, int align = 0)
	{
		U32 size = sizeof(T) * n;
		size = AlignUp(size, 16);

		const static int alignT = BTL_ALIGNMENT_OF(T);
		int a = ALIGN;
		if (alignT > a) a = alignT;
		if (align > a) a = align;
		assert(a <= 16); // Spu::Alloc always aligns to 16 bytes.

		T* p = (T*) Spu::Alloc(size);

		for (int i = 0; i < n; i++)
			new (p + i) T;

		return p;
	}

	/// Free objects.
	/// Does nothing.
	static void Free(T* p, int n = 0)
	{
	}

	template <typename U> struct Rebind {typedef LinearArrayAllocator<U> Other;};
};

}

#endif
#endif
