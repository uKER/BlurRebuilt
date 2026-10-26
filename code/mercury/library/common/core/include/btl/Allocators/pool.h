#ifndef BTL_POOL_H
#define BTL_POOL_H
#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////
// Pool
///////////////////////////////////////////////////////

// Allocate fixed size objects from pool.

#include <core/utility/coredebug.h>
#include "btl/utility/threads.h"
#include "btl/containers/list.h"
#include "btl/containers/vector.h"
#include "btl/utility/types.h"
#include "btl/allocators/allocator.h"

namespace Btl
{
////////////////////////////////////////////////////////

/// @addtogroup Allocators
/// @{

/// Pre-allocated pool of objects of type T.
/// @param T The type of object in the pool.
/// @param N The number of pre-allocated objects. 
///	@remarks T must be at least the size of a pointer.
template <class T, int N>
class StaticPool
{
public:
	StaticPool();
	~StaticPool() {assert(m_list.Size() == N);}

	T* Allocate(); ///< Allocate an object.
	void Free(T* p); ///< Free an object.

	bool Empty() {return m_list.Empty();} ///< Test for empty pool.

	/// The number of allocated objects 
	/// @remarks Slow since the pool does not maintain a count.
	int Allocated() const {return N - m_list.Size();}

	/// The number of objects left in the pool.
	/// @remarks Slow since the pool does not maintain a count.
	int Remaining() const {return m_list.Size();}

	/// This gives a pool type which allocates U instead of T.
	template <typename U> struct Rebind {typedef StaticPool<U, N> Other;};

protected:

	Vector<SameSizeAndAlignment<T>, VectorParameters_Array<N> > m_pool;

	// dummy list item type
	struct ListItem : public ListElementSingle<ListItem> {void Destroy() {}};

	List<ListItem, ListParameters<SingleEndedList, OnDemandCounter> > m_list;

private:
	StaticPool(StaticPool& other); // copy protection
	void operator = (StaticPool& other); // copy protection
};

/// @}

//////////////////////////////////////////////////////////
// inline functions

template <class T, int N>
StaticPool<T, N>::StaticPool()
{
	BIZ_STATIC_ASSERT(sizeof(T) >= sizeof(ListItem)); // must be space for a ListItem inside a T

	for (int i = 0; i < N; i++)
	{
		m_list.PushFront((ListItem&) m_pool[i]);
	}
}

template <class T, int N>
inline T* StaticPool<T, N>::Allocate()
{
	assert(!m_list.Empty()); // error to call Allocate() when no items remaining

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	// Invoke default constructor on allocated object - this is essential if T has virtual functions.
	new(p) T;

	return p;
}

template <class T, int N>
inline void StaticPool<T, N>::Free(T* p)
{
	if (p == 0) return;

	assert(m_pool.Validate((SameSizeAndAlignment<T>*)p)); // check that the pointer came from the pool

	// Call object's destructor.
	p->T::~T();

	m_list.PushFront((ListItem&) *p);
}

/////////////////////////////////////////////////////////////
// DynamicPool
/////////////////////////////////////////////////////////////

/// @addtogroup Allocators
/// @{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4348)
#endif

/// Dynamic pool of objects of type T.
/// When the pool is empty a new block is added automatically.
/// @param T The type of object allocated by the pool.
/// @param AllocatorType The allocator used to create memory blocks.
/// @param ThreadingModel See @ref Threading_Models.
// todo: clean-up function to free unused blocks
// default parameters in forward definition (list.h)
template <class T, typename AllocatorType, class ThreadingModel>
class DynamicPool
{
public:
	DynamicPool();
	~DynamicPool();

	T* Allocate(); ///< Allocate an object.
	T* AllocateNoConstructor(); ///< Allocate an object without calling it's constructor.
	T* AllocatePrefetch(); ///< Allocate an object and prefetch next object.
	void Prefetch(); ///< Prefetch next object in anticipation of an Allocate.
	void Free(T* p); ///< Free an object.
	

	/// This gives a pool type which allocates U instead of T.
	template <typename U> struct Rebind {typedef DynamicPool<U, AllocatorType, ThreadingModel> Other;};

	friend void Swap(DynamicPool& left, DynamicPool& right) {SwapByMemcopy(left, right);}

	/// The total number of bytes used by the pool
	int SizeInBytes() const {return m_sizeInBytes;}

	/// The number of objects allocated by the pool.
	/// @remarks Slow since the pool does not maintain a count.
	int Allocated() const {return m_sizeInBytes / sizeof(T) - Remaining();}

	/// The number of objects that can be allocated before the size of the pool will be increased.
	/// @remarks Slow since the pool does not maintain a count.
	int Remaining() const {return m_list.Size();}

private:
	void Grow();

	int m_sizeInBytes;

	// hold pointers to blocks
	Vector<U8*> m_pool; // deque may be better if the number of blocks is large

	// dummy list item type
	struct ListItem : public ListElementSingle<ListItem> {void Destroy() {}};

	List<ListItem, ListParameters<SingleEndedList, OnDemandCounter> > m_list;

	ThreadingModel m_threadData;

	DynamicPool(DynamicPool& other); // copy protection
	void operator = (DynamicPool& other); // copy protection
};

template <class T, typename AllocatorType, class ThreadingModel>
void Swap(DynamicPool<T, AllocatorType, ThreadingModel>& left, DynamicPool<T, AllocatorType, ThreadingModel>& right) 
{
	SwapByMemcopy(left, right);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// @}

/////////////////////////////////////////////////////////////////////

// Separate class to Pool so that you can declare Pool<T> before defining T.
template <class T>
struct PoolTypes
{
	const static int N = StaticMax<4 * 1024 / (int)sizeof(T), 1>::RESULT;
	typedef Vector<SameSizeAndAlignment<T>, VectorParameters_Array<N> > Block;
};

template <class T, typename AllocatorType, class ThreadingModel>
inline DynamicPool<T, AllocatorType, ThreadingModel>::DynamicPool() : m_sizeInBytes(0)
{
	BIZ_STATIC_ASSERT(sizeof(T) >= sizeof(ListItem)); // must be space for a ListItem inside a T
}

template <class T, typename AllocatorType, class ThreadingModel>
DynamicPool<T, AllocatorType, ThreadingModel>::~DynamicPool()
{
	BIZ_ASSERT_MSG(m_list.Size() == PoolTypes<T>::N * m_pool.Size(), "m_list.Size(%d) PoolTypes<T>::N(%d) m_pool.Size(%d)", m_list.Size(), PoolTypes<T>::N, m_pool.Size());

	m_list.Abandon(); // stop list clearing

	while (!m_pool.Empty())
	{
		typedef typename PoolTypes<T>::Block U;
		((U*) m_pool.Back())->U::~U();
		AllocatorType::Free(m_pool.Back(), sizeof(U));
		m_pool.PopBack();
	}
}

template <class T, typename AllocatorType,  class ThreadingModel>
inline T* DynamicPool<T, AllocatorType, ThreadingModel>::Allocate()
{
	typename ThreadingModel::Lock lock(m_threadData);

	if (m_list.Empty())
		Grow();

	if (m_list.Empty())
		return 0;

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	// Call default constructor for new object.
	new (p) T;

	return p;
}

template <class T, typename AllocatorType,  class ThreadingModel>
inline T* DynamicPool<T, AllocatorType, ThreadingModel>::AllocateNoConstructor()
{
	typename ThreadingModel::Lock lock(m_threadData);

	if (m_list.Empty())
		Grow();

	if (m_list.Empty())
		return 0;

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	return p;
}

template <class T, typename AllocatorType,  class ThreadingModel>
inline T* DynamicPool<T, AllocatorType, ThreadingModel>::AllocatePrefetch()
{
	typename ThreadingModel::Lock lock(m_threadData);

	if (m_list.Empty())
		Grow();

	if (m_list.Empty())
		return 0;

	T* p = (T*) &m_list.Front();
	ListItem* pListItem = (ListItem*)p;
	DCBT(0, pListItem->LE_Next());
	m_list.PopFront();

	// Call default constructor for new object.
	new (p) T;

	return p;
}

template <class T, typename AllocatorType,  class ThreadingModel>
inline void DynamicPool<T, AllocatorType, ThreadingModel>::Prefetch()
{
	if (!m_list.Empty())
	{
		T* p = (T*) &m_list.Front();
		DCBT(0, p);
	}
}

template <class T, typename AllocatorType, class ThreadingModel>
inline void DynamicPool<T, AllocatorType, ThreadingModel>::Free(T* p)
{
	if (p == 0) return;

	typename ThreadingModel::Lock lock(m_threadData);

#ifndef CFG_GOLD
	U32 blockSize = PoolTypes<T>::N * sizeof(T);
	bool found = false;
	for(S32 i = 0; i < m_pool.Size(); ++i)
	{
		INTPTR pfree = (INTPTR)p;	// [Blur2PC][x64] addresses, not U32
		INTPTR memStart = (INTPTR)m_pool[i];
		if(pfree >= memStart && pfree < memStart + blockSize)
		{
			found = true;
			break;
		}
	}
	BIZ_ASSERT_SPEED(found);
#endif

	// Call object's destructor.
	p->T::~T();

	m_list.PushFront((ListItem&) *p);
}



template <class T, typename AllocatorType, class ThreadingModel>
void DynamicPool<T, AllocatorType, ThreadingModel>::Grow()
{
	U8* data = AllocatorType::Allocate(sizeof(typename PoolTypes<T>::Block));

	if (data)
	{
		typename PoolTypes<T>::Block* newVector = (typename PoolTypes<T>::Block*) data;
		new (newVector) typename PoolTypes<T>::Block;

		m_pool.PushBack(data);

		for (int i = PoolTypes<T>::N - 1; i >= 0; i--)
		{
			m_list.PushFront((ListItem&) (*newVector)[i]);
		}

		m_sizeInBytes += sizeof(typename PoolTypes<T>::Block);
	}
}

}
#endif
