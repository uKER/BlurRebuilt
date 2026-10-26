#ifndef __ALLOCATORS_STACKALLOCATOR_H__
#define __ALLOCATORS_STACKALLOCATOR_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core/containers/vector.h>
#include <core/containers/list.h>

namespace Ut
{

/// @addtogroup Allocators
/// @{

/// Creates a buffer and allocates from it linearly.
/// The advantage of a stack allocator is that objects are allocated one after another with no padding (except for alignment) or extra data.
/// %StackAllocatorSimple has a fixed amount of space and allocation will return null pointers when the stack is full.
/// Constructors are called when allocating objects.
/// Destructors are called when freeing objects, but the memory is not released until Clear() is called or the allocator is destroyed. 
class StackAllocatorSimple
{
public:

	typedef AlignedAllocator<U8, 16> Allocator;

	StackAllocatorSimple() : m_pStackBuffer(NULL), m_bufferSize(0), m_position(0) {}

	StackAllocatorSimple(StackAllocatorSimple& other)
	{
		m_pStackBuffer = other.m_pStackBuffer;
		m_position = other.m_position;
		m_bufferSize = other.m_bufferSize;

		other.m_pStackBuffer = 0;
		other.m_position = 0;
		other.m_bufferSize = 0;
	}

	~StackAllocatorSimple()
	{
		//SysAlignedFree(m_pStackBuffer);
		Allocator::Free(m_pStackBuffer, 0);
	}

	/// Set the initial size of the buffer before allocating anything.
	void Initialise(int sizeInBytes)
	{
		assert(m_bufferSize == 0);
		
		//if (m_pStackBuffer = (U8*)SysAlignedMalloc(sysMemTrackID, sizeInBytes, 16))
		if (m_pStackBuffer = (U8*) Allocator::Allocate(sizeInBytes))
		{
			m_bufferSize	= sizeInBytes;
			m_position		= m_pStackBuffer;
		}
	}

	/// Free all the memory and reset the stack.
	void Clear() 
	{
		if (m_pStackBuffer)
		{
			//SysAlignedFree(m_pStackBuffer);
			Allocator::Free(m_pStackBuffer, 0);

			m_pStackBuffer	= NULL;
			m_bufferSize	= 0;
			m_position		= 0;
		}
	}

	/// Reset the stack without freeing memory.
	void Reset()
	{
		m_position = m_pStackBuffer;
	}
	
	/// Allocate n objects of type T.
	/// Returns zero when the stack is full.
	template <class T>
	T* Allocate(int n)
	{
		Align(ALIGNMENT_OF(T));
		T* p = (T*) m_position;
		m_position += n * sizeof(T);

		if (BytesUsed() > m_bufferSize)
			return 0;

		T* p1 = p;
		for (int i = 0; i < n; i++, p1++)
			new (p1) T;
		
		return p;
	}

	/// Allocate an object of type T.
	/// Returns zero when the stack is full.
	template <class T>
	T* Allocate()
	{
		Align(ALIGNMENT_OF(T));
		T* p = (T*) m_position;
		m_position += sizeof(T);

		if (BytesUsed() > m_bufferSize)
			return 0;

		T* p1 = p;
		new (p) T;
		
		return p;
	}

	/// Free an object of type T.
	template <class T>
	static void Free(T* p)
	{
		if (p) p->~T();
	}

	/// Free n objects of type T.
	template <class T>
	static void Free(T* p, int n)
	{
		if (p)
		{
			T* p1 = p;

			for (int i = 0; i < n; i++, p1++)
				p1->~T();
		}
	}

	// Returns the number of unused bytes remaining in the buffer.
	__forceinline int BytesRemaining() const
	{
		return m_bufferSize - BytesUsed(); 
	}

	__forceinline int BytesUsed(bool actualSpaceUsed=false) const
	{
		const int spaceUsed = (int)(m_position - m_pStackBuffer);

		return (actualSpaceUsed) ? spaceUsed : Maths::Max(spaceUsed, m_bufferSize); 
	}

private:

	__forceinline void Align(int align) 
	{
		assert((align & (align-1)) == 0);
		int s1 = (int)(m_position - m_pStackBuffer);
		int s3 = (s1 + (align - 1)) & ~(align-1);
		m_position = (U8*) m_pStackBuffer + s3;
	}

	// align to 16 bytes
	U8*	m_pStackBuffer;
	U8*	m_position;
	S32	m_bufferSize;
};

/// Stack allocator that automatically adds more stack space as required.
/// See StackAllocatorSimple.
class DynamicStackAllocator
{
public:
	DynamicStackAllocator()
	{
		m_stackAllocators.PushBack();
	}

	/// Can be called after Destroy() to begin using the allocator again.
	/// It's not necessary to call this before using the allocator for the first time.
	void Initialise()
	{
		if (m_stackAllocators.Empty())
			m_stackAllocators.PushBack();
	}

	/// Free an object of type T.
	template <class T>
	static void Free(T* p)
	{
		StackAllocatorSimple::Free(p);
	} 

	/// Free n objects of type T.
	template <class T>
	static void Free(T* p, int n)
	{
		StackAllocatorSimple::Free(p, n);
	} 

	/// Allocate an object of type T
	template <class T>
	T* Allocate(int n)
	{
		T* p = m_stackAllocators.Back().Allocate<T>(n);

		if (!p) 
		{
			Grow(sizeof(T) * n);
			p = m_stackAllocators.Back().Allocate<T>(n);
		}

		return p;
	}

	/// Free an object of type T.
	template <class T>
	T* Allocate()
	{
		T* p = m_stackAllocators.Back().Allocate<T>();

		if (!p) 
		{
			Grow();
			p = m_stackAllocators.Back().Allocate<T>();
		}

		return p;	
	}

	/// Reserve n bytes of contiguous memory.
	void Reserve(int n)
	{
		if (m_stackAllocators.Back().BytesRemaining() < n)
			Grow(n);
	}

	/// Reset the pointer back to the beginning, so we can re-use the same buffer again
	void Reset(void)
	{
		assert(m_stackAllocators.Size() == 2);

		m_stackAllocators.Back().Reset();
	}

	/// Find out how much the memory has been allocated.
	int BytesUsed(void)
	{
		int total = 0;
		for (int i = 0; i < m_stackAllocators.Size(); i++)
			total += m_stackAllocators[i].BytesUsed(true);

		return total;
	}

	/// Release all memory - allocator cannot be used again until Initialise() is called.
	void Destroy()
	{
		m_stackAllocators.Destroy();
	}

private:
	void Grow(int size = 16 * 1024)
	{
		m_stackAllocators.PushBack();
		m_stackAllocators.Back().Initialise(Maths::Max(size, 16 * 1024));
	}

	Ut::UtVector<Ut::DynamicVector<Ut::StackAllocatorSimple> > m_stackAllocators;
};

/// Singleton StackAllocatorSystem for general use.
/// Create and destory allocators.
class StackAllocatorSystem
{
public:
	static StackAllocatorSystem& Instance() {return s_instance;}

	/// Create a new allocator and start using it.
	DynamicStackAllocator* Begin() 
	{
		assert(m_currentAllocator == NULL);
		m_currentAllocator = m_allocatorPool.Allocate();
		return m_currentAllocator;
	}

	/// If we have an existing allocator, why not use it again.
	void Start(DynamicStackAllocator* pAllocator)
	{
		assert(m_currentAllocator == NULL);
		m_currentAllocator = pAllocator;
	}

	/// Stop using an allocator (but don't release it yet).
	void End(DynamicStackAllocator* pAllocator)
	{
		assert(m_currentAllocator == pAllocator);
		m_currentAllocator = 0;
	}

	/// Release all the memory used by an allocator.
	void Free(DynamicStackAllocator* pAllocator)
	{
		if (pAllocator == m_currentAllocator)
			m_currentAllocator = 0;

		if (pAllocator) m_allocatorPool.Free(pAllocator);
	}

	/// Get the last allocator that was added.
	Ut::DynamicStackAllocator& CurrentAllocator() 
	{
		assert(m_currentAllocator);
		return *m_currentAllocator;
	}

private:
	StackAllocatorSystem() {}

	Ut::DynamicStackAllocator* m_currentAllocator;
	Ut::DynamicPool<Ut::DynamicStackAllocator> m_allocatorPool;

	static StackAllocatorSystem s_instance;
};

/// Allocator with static functions (uses the current allocator in StackAllocatorSystem)
template <class T>
class StackAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	static T* Allocate() 
	{
		return StackAllocatorSystem::Instance().CurrentAllocator().Allocate<T>();
	}

	/// Free an object.
	static void Free(T* p, int n = 0)
	{
		if (p) StackAllocatorSimple::Free<T>(p);
	}

	template <typename U> struct Rebind {typedef StackAllocator<U> Other;};
};

/// Array allocator with static functions (uses the current allocator in StackAllocatorSystem)
template <class T>
class StackArrayAllocator
{
public:
	typedef T* PointerType;

	/// Allocate objects.
	static T* Allocate(int n) 
	{
		return StackAllocatorSystem::Instance().CurrentAllocator()->Allocate<T>(n);
	}

	/// Free objects.
	static void Free(T* p, int n)
	{
		if (p) StackAllocatorSimple::Free<T>(p, n);
	}

	template <typename U> struct Rebind {typedef StackArrayAllocator<U> Other;};
};

/// @} end group

}

#endif
