#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#ifdef _MSC_VER
#pragma once
#endif

#include "core/core.h"
#include <core/utility/utility.h>
#include <core/utility/coredebug.h>
#include <core/allocators/corememoryfunctions.h>

#ifdef PLATFORM_PS3
#define snPause() __asm__ volatile ( "tw 31,1,1" )
#endif

#if !defined( COMPILER_VC )
#if defined( PLATFORM_PS3)
#if CELL_SDK_VERSION >= 0x240001
#include <sys/memory.h>
#else
#include <memory.h>
#endif
#elif defined(PLATFORM_SPU)
#include <stdlib.h>
#else
#include <malloc.h>	// for memalign
#include <stdlib.h>	// for memalign
#endif
#endif

namespace Ut
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
	static void Free(T* p, int n = 0)
	{
		delete p;
	}

	template <typename U> 
	struct Rebind 
	{
		typedef StandardAllocator<U> Other;
	};
};

/// Allocator which passes an ID when calling new.
template <class T, U32 ID>
class TrackedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static T* Allocate() 
	{
		return new (ID) T;
	}

	/// Free an object.
	static void Free(T* p, int n = 0)
	{
		delete p;
	}

	template <typename U> struct Rebind {typedef TrackedAllocator<U, ID> Other;};
};

/// %AbstractAllocator can't allocate, but can free objects.
/// Useful for pointers to abstract base classes.
template <class T>
class AbstractAllocator
{
public:
	typedef T* PointerType;

	/// Free an object.
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
	typedef T U;
	
	/// Allocate n objects of type T.
	static T* Allocate(int n) 
	{
		assert(n > 0);
		const static int alignment = ALIGNMENT_OF(T);
		
		if (alignment < 16)
			return new T[n];
		else
			return (T*) BizAlignedMalloc(n * sizeof(T), alignment);
	}

#pragma warning(push)
#pragma warning(disable:4100)  
	/// Free objects.
	/// The second argument is ignored.
	static void Free(T* p, int n)
	{
		const static int alignment = ALIGNMENT_OF(T);

		if (alignment < 16)
			delete [] p;
		else
			BizAlignedFree(p);
	}
#pragma warning(pop)
	template <typename U> struct Rebind {typedef ArrayAllocator<U> Other;};
};

/// Array allocator with memory alignment.
/// @param T The type to be allocated. This should be a basic type such as char, not a class.
/// @param align Alignment value, which must be a power of 2.
template<class T, int align>
class AlignedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate memory for n objects of type T.
	static void* Allocate(int n) 
	{
		return BizAlignedMalloc(n * sizeof(T), align);
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(void* p, int n)
	{
		BizAlignedFree(p);
	}

	template <typename U> struct Rebind {typedef AlignedAllocator<T, align> Other;};
};


/// Array allocator with forced memory alignment.This is for the case when the compiler makes
/// a mess of the alignment and we need to do it ourselves. Different from the aligned allocator
/// in that the Allocate() member returns a T* not void. 
/// @param T The type to be allocated. This should be a basic type such as char, not a class.
/// @param align Alignment value, which must be a power of 2.
template<class T, int align>
class ForceAlignedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate memory for n objects of type T.
	static T* Allocate(int n) 
	{
		return (T*)BizAlignedMalloc(n * sizeof(T), align);
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(void* p, int n)
	{
		BizAlignedFree(p);
	}

	template <typename U> struct Rebind {typedef ForceAlignedAllocator<T, align> Other;};
};

#ifndef __GNUC__

/// Array allocator with memory alignment and tracking.
/// See AlignedAllocator, TrackedAllocator.
template<class T, int align, U32 ID>
class TrackedAlignedAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static void* Allocate(int n) 
	{
		return BizAlignedMalloc(ID, n * sizeof(T), align);
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(void* p, int n)
	{
		BizAlignedFree(p);
	}

	template <typename U> struct Rebind {typedef AlignedAllocator<T, align> Other;};
};

#endif

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
	static void Free(T*& p, int n = 0)
	{
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
	template <typename T>
	static T* Allocate(int n)
	{
		return Array::template Rebind<T>::Other::Allocate(n);
	}

	template <typename T>
	static T* Allocate()
	{
		return Single::template Rebind<T>::Other::Allocate();
	}

	template <typename T>
	static void Free(T* p, int n)
	{
		Array::template Rebind<T>::Other::Free(p, n);
	}

	template <typename T>
	static void Free(T* p)
	{
		Single::template Rebind<T>::Other::Free(p);
	}
};

/// With this allocator, you can call Free() for an incomplete (i.e. forward-declared) type.
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

/// @} end group

}

#endif // _ALLOCATOR_H_
