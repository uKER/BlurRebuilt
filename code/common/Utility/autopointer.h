#pragma once

#include "utility.h"

namespace Ut
{

/// @addtogroup Misc
/// @{

/// A pointer that frees the object pointed to automatically in the destructor, 
/// or if it is assigned to another object.
template <typename T, typename AllocatorType = StandardAllocator<T> >
class AutoPointer
{
public:
	AutoPointer() : m_p(0) {}
	//AutoPointer(bool allocate) {if (allocate) m_p = AllocatorType::Allocate(); else m_p = 0;}
	~AutoPointer() {AllocatorType::Free(m_p, 0);}

	/// Constructs or copies from any autopointer with the same allocator as long as the pointers can be converted.
	template <typename Other>
	AutoPointer(AutoPointer<Other, typename AllocatorType::Rebind<Other>::Other>& other) : m_p(other.Pointer()) {other.ClearPrivatePointer();}

	template <typename Other>
	AutoPointer& operator = (AutoPointer<Other, typename AllocatorType::Rebind<Other>::Other>& other) 
	{
		AllocatorType::Free(m_p, 0); 
		m_p = other.Pointer(); 
		other.ClearPrivatePointer(); 
		return *this;
	}

	// stop the compiler generating its own copy constructor and '=' operator
	AutoPointer(AutoPointer& other) : m_p(other.Pointer()) {other.ClearPrivatePointer();}

	/// Destructive copy.
	AutoPointer& operator = (AutoPointer& other) 
	{
		AllocatorType::Free(m_p, 0); 
		m_p = other.Pointer(); 
		other.ClearPrivatePointer(); 
		return *this;
	}

	const T* operator -> () const {return m_p;} ///< @a
	const T& operator *() const {return *m_p;} ///< @a
	T* operator -> () {return m_p;} ///< @a
	T& operator *() {return *m_p;} ///< @a
	T* Pointer() {return m_p;} ///< @a
	const T* Pointer() const {return m_p;} ///< @a

	// array access (not range-checked though)
	__forceinline T& operator [] (int i) {return m_p[i];} ///< @a
	__forceinline const T& operator [] (int i) const {return m_p[i];} ///< @a

	bool operator == (const T* p) const {return m_p == p;} ///< @a
	bool operator != (const T* p) const {return m_p != p;} ///< @a

	template <typename T, typename AllocatorType>
	friend void Allocate(AutoPointer<T, AllocatorType>& p);

	template <typename T, typename AllocatorType>
	friend void Allocate(AutoPointer<T, AllocatorType>& p, int n);

	template <typename T, typename AllocatorType>
	friend void Free(AutoPointer<T, AllocatorType>& p);

	template <typename U> struct Rebind {typedef AutoPointer<U, typename AllocatorType::Rebind<U>::Other > Other;};

	void ClearPrivatePointer() {m_p = 0;} // For internal use only.

private:
	T* m_p;
};

/// Allocate an object and point the auto-pointer at it.
template <typename T, typename AllocatorType>
void Allocate(AutoPointer<T, AllocatorType>& p) 
{
	AllocatorType::Free(p.m_p, 0); 
	p.m_p = AllocatorType::Allocate();
}

/// Allocate an array and point the auto-pointer at it.
template <typename T, typename AllocatorType>
void Allocate(AutoPointer<T, AllocatorType>& p, int n) 
{
	AllocatorType::Free(p.m_p, 0); 
	p.m_p = (T*) AllocatorType::Allocate(n);
}

/// Free the object pointed to by the auto-pointer.
template <typename T, typename AllocatorType>
void Free(AutoPointer<T, AllocatorType>& p) 
{
	AllocatorType::Free(p.m_p, 0); 
	p.m_p = 0;
}

/// @}

#ifdef GOTHAM3

#include "stackallocator.h"

template <typename T, int ID>
struct TrackedAutoPointer
{
	typedef AutoPointer<T, TrackedAllocator<T, ID> > Result;
};

template <typename T, int ID>
struct StackedAutoPointer
{
	typedef AutoPointer<T, StackAllocator<T> > Result;
};

#else

template <typename T, int ID>
struct TrackedAutoPointer
{
	typedef AutoPointer<T> Result;
};

template <typename T, int ID>
struct StackedAutoPointer
{
	typedef AutoPointer<T> Result;
};

#endif

}