#ifndef BTL_SCOPEDPOINTER_H
#define BTL_SCOPEDPOINTER_H
#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////
// ScopedPointer
////////////////////////////////////////////////////////////////

#include "types.h"
#include "btl/allocators/allocator.h"

namespace Btl
{

/// @addtogroup Misc
/// @{

#if !PLATFORM_SPU
template <typename T, typename AllocatorType = StandardAllocator<T> > class ScopedPointer;
#else
template <typename T, typename AllocatorType = LinearAllocator<T> > class ScopedPointer;
#endif

/// Similar to AutoPointer but ownership cannot be transferred.
/// (Which stops it happening unintentionally.)
template<typename T, typename Allocator>
class ScopedPointer
{
public:
	ScopedPointer() : m_p(0) {}
	~ScopedPointer() {Free(*this);}

	// NOTE: object must have been allocated with new
	ScopedPointer(T* p) : m_p(p) 
	{
#if !PLATFORM_SPU
		BIZ_STATIC_ASSERT((Btl::SameType<Allocator, Btl::StandardAllocator<T> >::result));
#endif
	}

	T* operator -> () {return m_p;} ///< @a
	const T* operator -> () const{return m_p;} ///< @a
	const T& operator *() const {return *m_p;} ///< @a
	T& operator *() {return *m_p;} ///< @a

	bool operator == (const T* p) const {return m_p == p;} ///< @a
	bool operator != (const T* p) const {return m_p != p;} ///< @a

	// Non-members (no named member functions)
	template <typename T2, typename Allocator2>
	friend void Allocate(ScopedPointer<T2, Allocator2>& p);

	template <typename T2, typename ActualType, typename Allocator2>
	friend void Allocate(ScopedPointer<T2, Allocator2>& p, TypeHolder<ActualType> deduceTypeFromThis);

	template <typename T2, typename Allocator2>
	friend void Initialise(ScopedPointer<T2, Allocator2>& p, T2* in);

	template <typename T2, typename Allocator2>
	friend void Free(ScopedPointer<T2, Allocator2>& p);

	template <typename T2, typename Allocator2>
	friend const T2* Pointer(const ScopedPointer<T2, Allocator2>& p);

	template <typename T2, typename Allocator2>
	friend T2* Pointer(ScopedPointer<T2, Allocator2>& p);

	template <typename T2, typename Allocator2>
	friend void Swap(ScopedPointer<T2, Allocator2>& p1, ScopedPointer<T2, Allocator2>& p2);

	typedef Allocator AllocatorType;

	template <typename U> struct Rebind {typedef ScopedPointer<U, typename Allocator::template Rebind<U>::Other > Other;};

	ScopedPointer(const ScopedPointer& p)
	{
		if (p != 0)
			m_p = p->Clone();
		else
			m_p = 0;
	}

	ScopedPointer& operator = (const ScopedPointer& p)
	{
		Free(*this);

		if (p != 0)
			m_p = p->Clone();
	}

	T* m_p;
};

/// Allocate the object held by the ScopedPointer
template <typename T, typename Allocator>
void Allocate(ScopedPointer<T, Allocator>& p) 
{
	assert(p.m_p == 0);
	p.m_p = Allocator::Allocate();
}

/// The second argument provides a way to create a type other than the one specified in the ScopedPointer template.
/// (With AutoPointer you would just transfer ownership from another AutoPointer with the required type)
template <typename T, typename ActualType, typename Allocator>
void Allocate(ScopedPointer<T, Allocator>& p, TypeHolder<ActualType> deduceTypeFromThis) 
{
	assert(p.m_p == 0);
	p.m_p = Allocator::template Rebind<ActualType>::Other::Allocate();
}

/// Initialise from a pointer (which must have been allocated with the same allocator).
template <typename T, typename Allocator>
void Initialise(ScopedPointer<T, Allocator>& p, T* in) 
{
	assert(p.m_p == 0);
	Free(p); // assert above would trigger if this does anything
	p.m_p = in;
}

/// Free the object held by the ScopedPointer.
template<typename T, typename Allocator>
void Free(ScopedPointer<T, Allocator>& p)
{
	Allocator::Free(p.m_p, 0);
	p.m_p = 0;
}

/// Get a plain pointer to the object held by the ScopedPointer.
template <typename T, typename Allocator>
const T* Pointer(const ScopedPointer<T, Allocator>& p) 
{
	return p.m_p;
}

/// Get a plain pointer to the object held by the ScopedPointer.
template <typename T, typename Allocator>
T* Pointer(ScopedPointer<T, Allocator>& p) 
{
	return p.m_p;
}

/// Swap two scoped pointers of the same type.
template <typename T, typename Allocator>
void Swap(ScopedPointer<T, Allocator>& p1, ScopedPointer<T, Allocator>& p2)
{
	Swap(p1.m_p, p2.m_p);
}

template <typename T, typename Allocator>
void Move(void* pDst, ScopedPointer<T, Allocator>* pSrc) 
{
	MoveByMemcopy<ScopedPointer<T, Allocator> >::Move(pDst, pSrc);
}

template <typename T, typename Allocator>
void MoveArray(void* pDst, ScopedPointer<T, Allocator>* pSrc, int n) 
{
	MoveByMemcopy<ScopedPointer<T, Allocator> >::MoveArray(pDst, pSrc, n);
}

/// @}

}

#endif
