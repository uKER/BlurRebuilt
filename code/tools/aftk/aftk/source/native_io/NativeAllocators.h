#ifndef NATIVE_ALLOCATORS_H
#define NATIVE_ALLOCATORS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	template <typename T>
	class C_NativeStdPoolAllocator
	{
	public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	template <typename U>
	struct rebind { typedef C_NativeStdPoolAllocator<U> other; };

	pointer address(reference x) const {return &x;}
	const_pointer address(const_reference x) const {return &x;}

	C_NativeStdPoolAllocator(){}
	C_NativeStdPoolAllocator(const C_NativeStdPoolAllocator&) {}
	template <typename U>
	C_NativeStdPoolAllocator (const C_NativeStdPoolAllocator<U>&) {}
	~C_NativeStdPoolAllocator(){}


	size_type max_size() const throw() {return size_t(-1) /
	sizeof(value_type);}

	pointer allocate(size_type size, const void* hint = 0)
	{
		return (T*)g_pNativeFtkMemoryPool->Allocate((U32)(sizeof(T)*size));
	}

	void construct(pointer p, const T& val)
	{
		new(static_cast<void*>(p)) T(val);
	}
	void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	void deallocate(pointer p, size_type n)
	{
		g_pNativeFtkMemoryPool->Free((U32)(sizeof(T)*n), p);	
	}
	void deallocate(void *p, size_type n)
	{
		g_pNativeFtkMemoryPool->Free((U32)(sizeof(T)*n), p);	
	}

	void deallocate(pointer p)
	{
		g_pNativeFtkMemoryPool->Free(sizeof(T), p);	
	}
	void deallocate(void *p)
	{
		g_pNativeFtkMemoryPool->Free(sizeof(T), p);	
	}

	};
}
#endif
