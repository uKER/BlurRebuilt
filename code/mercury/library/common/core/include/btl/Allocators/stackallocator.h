#ifndef BTL_STACKALLOCATOR_H
#define BTL_STACKALLOCATOR_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////
// Stack Allocator
/////////////////////////////////////////////////////

#include <btl/containers/vector.h>
#include <btl/allocators/pool.h>
#include <btl/containers/list.h>
#include "core/Threads/Mutex.h"
#include "core/Threads/threadlocalstorage.h"

namespace Btl
{

/// @addtogroup Allocators
/// @{

/// Creates a buffer and allocates from it linearly.
/// The advantage of a stack allocator is that objects are allocated one after another with no padding (except for alignment) or extra data.
/// %StackAllocatorSimple has a fixed amount of space and allocation will return null pointers when the stack is full.
/// Constructors are called when allocating objects.
/// Destructors are called when freeing objects, but the memory is not released until Clear() is called or the allocator is destroyed. 
template <
#if !PLATFORM_SPU
	typename Allocator = AlignedAllocator<U8, 16> >
#else
	typename Allocator = LinearAlignedAllocator<U8> >
#endif
class StackAllocatorSimple
{
public:

	//typedef AlignedAllocator<U8, 16> Allocator;

	StackAllocatorSimple() : m_pStackBuffer(NULL), m_position(0), m_bufferSize(0) {}

	~StackAllocatorSimple()
	{
		Allocator::Free(m_pStackBuffer, 0);
	}

	/// Set the initial size of the buffer before allocating anything.
	void Initialise(int sizeInBytes)
	{
		assert(m_bufferSize == 0);
		
		if ((m_pStackBuffer = (U8*) Allocator::Allocate(sizeInBytes)))
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
	T* Allocate(int n, int align = 0)
	{
		Align(Max(BTL_ALIGNMENT_OF(T), align));
		T* p = (T*) m_position;

		if (m_position + n * sizeof(T) - m_pStackBuffer > m_bufferSize)
			return 0;

		m_position += n * sizeof(T);

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
		Align(BTL_ALIGNMENT_OF(T));
		T* p = (T*) m_position;

		if (m_position + sizeof(T) - m_pStackBuffer > m_bufferSize)
			return 0;

		m_position += sizeof(T);

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
	FORCE_INLINE int BytesRemaining() const
	{
		return m_bufferSize - BytesUsed(); 
	}

	int Max(const int& lhs, const int& rhs) const {return (lhs > rhs) ? lhs : rhs;}

	FORCE_INLINE int BytesUsed() const
	{
		return m_position - m_pStackBuffer;
	}
	
	/// Get pointer to the buffer.
	const U8* GetBuffer() const
	{
		return m_pStackBuffer;
	}

	S32 BufferSize() const {return m_bufferSize;}

private:

	StackAllocatorSimple(StackAllocatorSimple& other);
	StackAllocatorSimple& operator = (const StackAllocatorSimple& other);

	FORCE_INLINE void Align(int align) 
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

template <typename Allocator>
inline void Move(void* pDst, StackAllocatorSimple<Allocator>* pSrc) 
{
	MoveByMemcopy<StackAllocatorSimple<Allocator> >::Move(pDst, pSrc);
}

template <typename Allocator>
inline void MoveArray(void* pDst, StackAllocatorSimple<Allocator>* pSrc, int n) 
{
	MoveByMemcopy<StackAllocatorSimple<Allocator> >::MoveArray(pDst, pSrc, n);
}

/// Stack allocator that automatically adds more stack space as required.
/// See StackAllocatorSimple.
class DynamicStackAllocator
{
public:
	DynamicStackAllocator()
	{
		m_stackAllocators.PushBack();
		m_growSize = 1024;
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
		StackAllocatorSimple<>::Free(p);
	} 

	/// Free n objects of type T.
	template <class T>
	static void Free(T* p, int n)
	{
		StackAllocatorSimple<>::Free(p, n);
	} 

	/// Allocate n objects of type T
	template <class T>
	FORCE_INLINE T* Allocate(int n, int align = 0)
	{
		T* p = m_stackAllocators.Back().Allocate<T>(n, align);

		if (p) return p;
		else return Allocate_Sub<T>(n, align);
	}

	/// Allocate n objects of type T
	template <class T>
	T* Allocate_Sub(int n, int align = 0)
	{
		T* p = 0;
		
		for (int i = 0; i < m_stackAllocators.Size(); i++)
		{
			p = m_stackAllocators[i].Allocate<T>(n, align);
			if (p) break;
		}

		if (!p) 
		{
			Grow(sizeof(T) * n);
			p = m_stackAllocators.Back().Allocate<T>(n, align);
		}

		return p;
	}

	/// Free an object of type T.
	template <class T>
	T* Allocate()
	{
		T* p = 0;
		
		for (int i = 0; i < m_stackAllocators.Size(); i++)
		{
			p = m_stackAllocators[i].Allocate<T>();
			if (p) break;
		}

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

	// Setting the size to zero will cause an assert when growing the stack.
	void SetGrowSize(int n)
	{
		assert(n >= 0);
		m_growSize = n;
	}

	/// Reset the pointer back to the beginning, so we can re-use the same buffer again
	void Reset(void)
	{
		assert(m_stackAllocators.Size() == 2);

		m_stackAllocators.Back().Reset();
	}

	/// Find out how much memory is in use.
	int BytesUsed() const
	{
		int total = 0;
		for (int i = 0; i < m_stackAllocators.Size(); i++)
			total += m_stackAllocators[i].BytesUsed();

		return total;
	}

	/// Release all memory - allocator cannot be used again until Initialise() is called.
	void Destroy()
	{
		m_stackAllocators.Destroy();
	}

	int GetNumAllocators() const {return m_stackAllocators.Size();}
	const StackAllocatorSimple<>& GetAllocator(int i) const {return m_stackAllocators[i];}

private:
	int Max(const int& lhs, const int& rhs) const {return (lhs > rhs) ? lhs : rhs;}

	void Grow(S32 size = 0)
	{
		assert(m_growSize > 0);

		if (m_stackAllocators.Back().BufferSize() != 0)
			m_stackAllocators.PushBack();
		else
			int breakhere = 0;

		m_stackAllocators.Back().Initialise(Max(size, m_growSize));
	}

	Vector<StackAllocatorSimple<> > m_stackAllocators;
	int m_growSize;
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
		m_poolMutex.Wait();
		PushBack(m_allocatorPool.Allocate());
		m_poolMutex.Release();

		return Back();
	}

	/// If we have an existing allocator, why not use it again.
	void Start(DynamicStackAllocator* pAllocator)
	{
		PushBack(pAllocator);
	}

	/// Stop using an allocator (but don't release it yet).
	void End(DynamicStackAllocator* pAllocator)
	{
		assert(Back() == pAllocator);
		PopBack();
	}

	/// Release all the memory used by an allocator.
	void Free(DynamicStackAllocator* pAllocator)
	{
		if (pAllocator)
		{
			m_poolMutex.Wait();
			m_allocatorPool.Free(pAllocator);
			m_poolMutex.Release();
		}
	}

	/// Get the last allocator that was added.
	DynamicStackAllocator* CurrentAllocator() 
	{
		return Back();
	}

private:
	StackAllocatorSystem() {}

	void PushBack(DynamicStackAllocator* pAllocator)
	{
		if (m_localStorage.GetData() == 0)
			m_localStorage.SetData(new Btl::Vector<DynamicStackAllocator*>);

		((Btl::Vector<DynamicStackAllocator*>*) m_localStorage.GetData())->PushBack(pAllocator);
	}

	void PopBack()
	{
		Btl::Vector<DynamicStackAllocator*>* pVector = ((Btl::Vector<DynamicStackAllocator*>*) m_localStorage.GetData());
		assert(pVector);
		pVector->PopBack();

		if (pVector->Empty())
		{
			delete pVector;
			m_localStorage.SetData(0);
		}
	}

	DynamicStackAllocator* Back()
	{
		Btl::Vector<DynamicStackAllocator*>* pVector = ((Btl::Vector<DynamicStackAllocator*>*) m_localStorage.GetData());

		if (pVector && !pVector->Empty())
			return pVector->Back();
		else 
			return 0;
	}

	DynamicPool<DynamicStackAllocator> m_allocatorPool;
	MT::Mutex m_poolMutex;
	MT::ThreadLocalStorage m_localStorage;

	static StackAllocatorSystem s_instance;
};

/// Allocator with static functions (uses the current allocator in StackAllocatorSystem)
template <class T>
class StackAllocator
{
public:
	typedef T* PointerType;

	/// Allocate an object.
	FORCE_INLINE static T* Allocate() 
	{
		return StackAllocatorSystem::Instance().CurrentAllocator()->Allocate<T>();
	}

	/// Free an object.
	FORCE_INLINE static void Free(T* p, int n = 0)
	{
		if (p) StackAllocatorSimple<>::Free<T>(p);
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
	FORCE_INLINE static T* Allocate(int n, int align = 0) 
	{
		return StackAllocatorSystem::Instance().CurrentAllocator()->Allocate<T>(n, align);
	}

	/// Free objects.
	FORCE_INLINE static void Free(T* p, int n)
	{
		if (p) StackAllocatorSimple<>::Free<T>(p, n);
	}

	template <typename U> struct Rebind {typedef StackArrayAllocator<U> Other;};
};

/// Specialisation for basic types
#define STACK_ARRAY_ALLOCATOR(_type)															\
template <>																						\
class StackArrayAllocator<_type>																\
{																								\
public:																							\
	typedef _type* PointerType;																	\
																								\
	/* Allocate objects.*/																		\
	FORCE_INLINE static _type* Allocate(int n, int align = 0)												\
	{																							\
		return StackAllocatorSystem::Instance().CurrentAllocator()->Allocate<_type>(n, align);	\
	}																							\
																								\
	/* No destructor to call.*/																	\
	FORCE_INLINE static void Free(_type* p, int n = 0)														\
	{																							\
	}																							\
																								\
	template <typename U> struct Rebind {typedef StackArrayAllocator<U> Other;};				\
};

STACK_ARRAY_ALLOCATOR(U8)
STACK_ARRAY_ALLOCATOR(U16)
STACK_ARRAY_ALLOCATOR(U32)
STACK_ARRAY_ALLOCATOR(S8)
STACK_ARRAY_ALLOCATOR(S16)
STACK_ARRAY_ALLOCATOR(S32)
	
/// @} end group

}

#endif
