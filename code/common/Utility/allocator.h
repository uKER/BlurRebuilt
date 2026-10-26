#ifndef GCC
#pragma once
#endif
#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

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
	/// Allocate n objects of type T.
	static T* Allocate(int n) 
	{
		assert(n > 0);
		return new T[n];
	}

	/// Free objects.
	/// The second argument is ignored.
	static void Free(T* p, int n)
	{
		delete [] p;
	}

	template <typename U> struct Rebind {typedef ArrayAllocator<U> Other;};
};

/// Array allocator which passes an ID when calling new.
template <class T, U32 ID>
class TrackedArrayAllocator
{
public:
	/// Allocate n objects of type T.
	static T* Allocate(int n) 
	{
		assert(n > 0);
		return new (ID) T[n];
	}

	/// Free objects.
	/// The second argument is ignored.
	static void Free(T* p, int n)
	{
		delete [] p;
	}

	template <typename U> struct Rebind {typedef ArrayAllocator<U> Other;};
};

#ifndef __GNUC__

/// Array allocator with memory alignment.
/// @param T The type to be allocated. This should be a basic type such as char, not a class.
/// @param align Alignment value, which must be a power of 2.
template<class T, int align>
class AlignedAllocator
{
public:
	/// Allocate memory for n objects of type T.
	static void* Allocate(int n) 
	{
		return _aligned_malloc(n * sizeof(T), align);
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(void* p, int n)
	{
		_aligned_free(p);
	}

	template <typename U> struct Rebind {typedef AlignedAllocator<T, align> Other;};
};

/// Array allocator with memory alignment and tracking.
/// See AlignedAllocator, TrackedAllocator.
template<class T, int align, U32 ID>
class TrackedAlignedAllocator
{
public:
	/// Allocate an object.
	static void* Allocate(int n) 
	{
		return SysAlignedMalloc(ID, n * sizeof(T), align);
	}

	/// Free an object.
	/// The second argument is ignored.
	static void Free(void* p, int n)
	{
		SysAlignedFree(p);
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

/// @} end group

}

#endif // _ALLOCATOR_H_
