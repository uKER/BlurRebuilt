#ifndef __UT_SCOPEDPOINTER_H__
#define __UT_SCOPEDPOINTER_H__

#pragma once

#include "utility.h"

namespace Ut
{

// Similar to AutoPointer but ownership cannot be transferred.
// (Which stops it happening unintentionally.)
template<typename T, typename Allocator = Ut::StandardAllocator<T> >
class ScopedPointer
{
public:
	ScopedPointer() : m_p(0) {}
	~ScopedPointer() {Free(*this);}

	ScopedPointer(T* p) : m_p(p) 
	{
		BIZ_STATIC_ASSERT((Ut::SameType<Allocator, Ut::StandardAllocator<T> >::result));
	}


	T* operator -> () {return m_p;}
	const T* operator -> () const{return m_p;}
	const T& operator *() const {return *m_p;}
	T& operator *() {return *m_p;}

	bool operator == (const T* p) const {return m_p == p;}
	bool operator != (const T* p) const {return m_p != p;}

	// [Jan] template friend function definitions are different on GCC. they don't seem to work either. hacked around it for now
#ifdef COMPILER_VC
	// Non-members (no named member functions)
	template <typename T, typename Allocator>
	friend void Allocate(ScopedPointer<T, Allocator>& p);

	template <typename T, typename ActualType, typename Allocator>
	friend void Allocate(ScopedPointer<T, Allocator>& p, Ut::TypeHolder<ActualType>& deduceTypeFromThis);

	template <typename T, typename Allocator>
	friend void Free(ScopedPointer& p);

	template <typename T, typename Allocator>
	friend const T* Pointer(const ScopedPointer<T>& p);

	template <typename T, typename Allocator>
	friend T* Pointer(ScopedPointer<T>& p);

	template <typename T, typename Allocator>
	friend void Swap(ScopedPointer<T, Allocator>& p1, ScopedPointer<T, Allocator>& p2);

	typename Allocator AllocatorType;

	template <typename U> struct Rebind {typedef ScopedPointer<U, typename Allocator::Rebind<U>::Other > Other;};
#endif

private:
	// Most useful behaviour would be to clone the object, however, we don't know
	// the exact type of the object at this point, because T could be a base class.
	ScopedPointer(const ScopedPointer& other);
	ScopedPointer& operator = (const ScopedPointer& other);

#ifdef COMPILER_VC
protected:
#else
public:
#endif


	T* m_p;
};

// Allocate the ScopedPointer
template <typename T, typename Allocator>
void Allocate(ScopedPointer<T, Allocator>& p) 
{
	assert(p.m_p == 0);
	p.m_p = Allocator::Allocate();
}

// The second argument provides a way to create a type other than the one specified in the ScopedPointer template.
// (With AutoPointer you would just transfer ownership from another AutoPointer with the required type)
template <typename T, typename ActualType, typename Allocator>
void Allocate(ScopedPointer<T, Allocator>& p, TypeHolder<ActualType>& deduceTypeFromThis) 
{
	assert(p.m_p == 0);
	p.m_p = Allocator::template Rebind<ActualType>::Other::Allocate();
}

template<typename T, typename Allocator>
void Free(ScopedPointer<T, Allocator>& p)
{
	Allocator::Free(p.m_p, 0);
	p.m_p = 0;
}

template <typename T, typename Allocator>
const T* Pointer(const ScopedPointer<T, Allocator>& p) 
{
	return p.m_p;
}

template <typename T, typename Allocator>
T* Pointer(ScopedPointer<T, Allocator>& p) 
{
	return p.m_p;
}

// Can swap two scoped pointers of the same type.
template <typename T, typename Allocator>
void Swap(ScopedPointer<T, Allocator>& p1, ScopedPointer<T, Allocator>& p2)
{
	Swap(p1.m_p, p2.m_p);
}

/////////////////////////////////////////////////////
// Basic non-virtual serialisation.
// Only type T is serialised, even if p points to a type derived from T.

template <typename Archive, typename T, typename Allocator>
void Serialise(Archive &ar, const ScopedPointer<T, Allocator>& p)
{
	ar << U32(p != 0);

	if (p != 0)
		ar << *Pointer(p);
}

template <typename Archive, typename T, typename Allocator>
void Deserialise(Archive &ar, ScopedPointer<T, Allocator>& p)
{
	Free(p);

	U32 exists;
	ar >> exists;

	if (exists)
	{
		Allocate(p);
		ar >> *Pointer(p);
	}
}

}

#endif