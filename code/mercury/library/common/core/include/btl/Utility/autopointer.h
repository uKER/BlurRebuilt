#ifndef BTL_AUTOPOINTER_H
#define BTL_AUTOPOINTER_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////////
// AutoPointer
/////////////////////////////////////////////////////////

#include "btl/allocators/allocator.h"

namespace Btl
{

/// @addtogroup Misc
/// @{

#if !PLATFORM_SPU
template <typename T, typename AllocatorType = StandardAllocator<T> > class AutoPointer;
#else
template <typename T, typename AllocatorType = LinearAllocator<T> > class AutoPointer;
#endif

// Temporory (compiler created) object to transfers a pointer from one AutoPointer to another.
// Should never be used explicitly.
template <typename T, typename AllocatorType>
struct AutoPointer_Temp
{
	AutoPointer_Temp(T* p) : m_p(p) {}
	T* m_p;
};

/// A pointer that frees the object pointed to automatically in the destructor, 
/// or if it is assigned to another object.
template <typename T, typename AllocatorType>
class AutoPointer
{
public:
	AutoPointer() : m_p(0) {}
	~AutoPointer() {AllocatorType::Free(m_p);}

#if !PLATFORM_SPU
	// NOTE: object must have been allocated with new (or Btl::StandardAllocator).
	explicit AutoPointer(T* p) : m_p(p) 
	{
		BIZ_STATIC_ASSERT((Btl::SameType<AllocatorType, Btl::StandardAllocator<T> >::result));
	}
#endif

	// Copying from another autopointer transfers the pointer.
	AutoPointer(AutoPointer& other) : m_p(Pointer(other)) {other.m_p = 0;}

	// Can copy from a different type if the pointers and allocators are compatible.
	template <typename U>
	AutoPointer(AutoPointer<U, typename AllocatorType::template Rebind<U>::Other>& other) : m_p(Pointer(other))  {other.m_p = 0;}

	// Allow copying from a temporary object.
	AutoPointer(const AutoPointer_Temp<T, AllocatorType>& temp) : m_p(temp.m_p) {}

	////////////////////////////////////////////////////

	AutoPointer& operator = (AutoPointer& other) 
	{
		AllocatorType::Free(m_p); 
		m_p = Pointer(other); 
		other.m_p = 0; 
		return *this;
	}

	template <typename U>
	AutoPointer& operator = (AutoPointer<U, typename AllocatorType::template Rebind<U>::Other>& other) 
	{
		AllocatorType::Free(m_p); 
		m_p = Pointer(other); 
		other.m_p = 0; 
		return *this;
	}	

	AutoPointer& operator = (const AutoPointer_Temp<T, AllocatorType>& temp) 
	{
		AllocatorType::Free(m_p); 
		m_p = temp.m_p; 
		return *this;
	}

	///////////////////////////////////

	// Creation of a temporary object for copying.
	template <typename U>
	operator AutoPointer_Temp<U, typename AllocatorType::template Rebind<U>::Other> ()
	{
		AutoPointer_Temp<U, typename AllocatorType::template Rebind<U>::Other> temp(m_p);
		m_p = 0;
		return temp;
	}

	/////////////////////////////////

	const T* operator -> () const {return m_p;} ///< @a 
	const T& operator *() const {return *m_p;} ///< @a 
	T* operator -> () {return m_p;} ///< @a 
	T& operator *() {return *m_p;} ///< @a 

	// array access (not range-checked though)
	__forceinline T& operator [] (int i) {return m_p[i];} ///< @a 
	__forceinline const T& operator [] (int i) const {return m_p[i];} ///< @a 

	bool operator == (const T* p) const {return m_p == p;} ///< @a 
	bool operator != (const T* p) const {return m_p != p;} ///< @a 

	// Can actually only be equal if both pointers are null.
	bool operator == (const AutoPointer& p) const {return m_p == p.m_p;} ///< @a 
	bool operator != (const AutoPointer& p) const {return m_p != p.m_p;} ///< @a 

	// Non-members (no named member functions)
	template <typename T2, typename AllocatorType2>
	friend void Allocate(AutoPointer<T2, AllocatorType2>& p);

	template <typename T2, typename AllocatorType2>
	friend void Allocate(AutoPointer<T2, AllocatorType2>& p, int n);

	template <typename T2, typename AllocatorType2>
	friend void Allocate(AutoPointer<T2, AllocatorType2>& p, int n, int align);

	template <typename T2, typename AllocatorType2>
	friend void Free(AutoPointer<T2, AllocatorType2>& p);

	template <typename T2, typename Allocator2>
	friend void Initialise(AutoPointer<T2, Allocator2>& p, T2* in);

	template <typename T2, typename AllocatorType2>
	friend const T2* Pointer(const AutoPointer<T2, AllocatorType2>& p);

	template <typename T2, typename AllocatorType2>
	friend T2* Pointer(AutoPointer<T2, AllocatorType2>& p);

	template <typename T2, typename AllocatorType2>
	friend void OffsetLocation(AutoPointer<T2, AllocatorType2>& p, S32 offset);

	template <typename U> struct Rebind {typedef AutoPointer<U, typename AllocatorType::template Rebind<U>::Other > Other;};

	template <typename T2, typename AllocatorType2>
	friend class AutoPointer;

private:
	template <typename U>
	operator AutoPointer_Temp<U, typename AllocatorType::template Rebind<U>::Other> () const;

	T* m_p;
};

/// Allocate an object and point the auto-pointer at it.
template <typename T, typename AllocatorType>
inline void Allocate(AutoPointer<T, AllocatorType>& p) 
{
	AllocatorType::Free(p.m_p); 
	p.m_p = AllocatorType::Allocate();
}

/// Allocate an array and point the auto-pointer at it.
template <typename T, typename AllocatorType>
inline void Allocate(AutoPointer<T, AllocatorType>& p, int n) 
{
	AllocatorType::Free(p.m_p); 
	p.m_p = (T*) AllocatorType::Allocate(n);
}

/// Allocate an array and point the auto-pointer at it.
template <typename T, typename AllocatorType>
inline void Allocate(AutoPointer<T, AllocatorType>& p, int n, int align) 
{
	AllocatorType::Free(p.m_p); 
	p.m_p = (T*) AllocatorType::Allocate(n, align);
}

/// Free the object pointed to by the auto-pointer.
template <typename T, typename AllocatorType>
inline void Free(AutoPointer<T, AllocatorType>& p) 
{
	AllocatorType::Free(p.m_p); 
	p.m_p = 0;
}

/// Initialise from a pointer (which must have been allocated with the same allocator).
template <typename T, typename Allocator>
inline void Initialise(AutoPointer<T, Allocator>& p, T* in) 
{
	Free(p);
	p.m_p = in;
}

/// Get plain pointer from AutoPointer (can also use &* except when null)
template <typename T, typename AllocatorType>
inline const T* Pointer(const AutoPointer<T, AllocatorType>& p) 
{
	return p.m_p;
}

/// Get plain pointer from AutoPointer (can also use &* except when null)
template <typename T, typename AllocatorType>
inline T* Pointer(AutoPointer<T, AllocatorType>& p) 
{
	return p.m_p;
}

template <typename T, typename AllocatorType>
inline AutoPointer<T, AllocatorType> Clone(const AutoPointer<T, AllocatorType>& src)
{
	if (src != 0)
		return AutoPointer<T, AllocatorType>(src->Clone());
	else
		return AutoPointer<T, AllocatorType>();
}

template <typename T, typename AllocatorType>
inline void OffsetLocation(AutoPointer<T, AllocatorType>& p, S32 offset)
{
	if (p.m_p)
	{
		U8* pNew = ((U8*)p.m_p) + offset;
		p.m_p = (T*) pNew;
	}
}

/// @}

}

#endif
