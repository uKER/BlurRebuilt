#ifndef AFTK_ARRAY_ALLOCATORS_H
#define AFTK_ARRAY_ALLOCATORS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	template <class T>
	class C_ArrayAllocator
	{
	public:
		typedef T* PointerType;

		///\brief Allocate an object.
		///
		///@param n The number of T to allocate.
		///@return a pointer to the start of the allocated memory.
		static T* Allocate(S32 n) 
		{
			T* p = (T*)C_MemoryManager::PoolAllocate(sizeof(T)*n);
			for (S32 i = 0; i < n; i++)
			{
				new (p + i) T; // create a new one over the top
			}

			return p;
		}

		///\brief Free an object.
		///
		///@param p A pointer to the start of the memory range to be freed.
		///@param n The number of instances of T in this range.
		static void Free(T* p, S32 n)
		{
			for (S32 i = 0; i < n; i++)
			{
				p[i].~T(); // destroy the object
			}
			C_MemoryManager::PoolFree(sizeof(T)*n, p);
		}

		template <typename U> struct Rebind {typedef C_ArrayAllocator<U> Other;};
	};

	template <class T>
	class C_Allocator
	{
	public:
		
		///\brief Allocate an object.
		///
		///@param n The number of T to allocate.
		///@return a pointer to the start of the allocated memory.
		static T* Allocate() 
		{
			T* p = (T*)C_MemoryManager::PoolAllocate(sizeof(T));
			new (p) T; // create a new one over the top
			return p;
		}

		static T* Allocate(const T& rhs) 
		{
			T* p = (T*)C_MemoryManager::PoolAllocate(sizeof(T));
			new (p) T(rhs); // create a new one over the top
			return p;
		}

		///\brief Free an object.
		///
		///@param p A pointer to the start of the memory range to be freed.
		static void Free(T* p)
		{
			p[0].~T(); // destroy the object
			C_MemoryManager::PoolFree(sizeof(T), p);
		}

		static void FreeVoid(void* p)
		{
			T* t = (T*)p;
			t[0].~T(); // destroy the object
			C_MemoryManager::PoolFree(sizeof(T), t);
		}

		template <typename U> struct Rebind {typedef C_Allocator<U> Other;};
	};

	template <typename T>
	class C_StdPoolAllocator
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
	struct rebind { typedef C_StdPoolAllocator<U> other; };

	pointer address(reference x) const {return &x;}
	const_pointer address(const_reference x) const {return &x;}

	C_StdPoolAllocator(){}
	C_StdPoolAllocator(const C_StdPoolAllocator&) {}
	template <typename U>
	C_StdPoolAllocator (const C_StdPoolAllocator<U>&) {}
	~C_StdPoolAllocator(){}


	size_type max_size() const throw() {return size_t(-1) /
	sizeof(value_type);}

	pointer allocate(size_type size, const void* hint = 0)
	{
		return (T*)C_MemoryManager::PoolAllocate((U32)(sizeof(T)*size));
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
		C_MemoryManager::PoolFree((U32)(sizeof(T)*n), p);	
	}
	void deallocate(void *p, size_type n)
	{
		C_MemoryManager::PoolFree((U32)(sizeof(T)*n), p);	
	}

	void deallocate(pointer p)
	{
		C_MemoryManager::PoolFree(sizeof(T), p);	
	}
	void deallocate(void *p)
	{
		C_MemoryManager::PoolFree(sizeof(T), p);	
	}

	};
}
#endif
