///////////////////////////////////////////////////////
// Pool
///////////////////////////////////////////////////////

// Allocate fixed size objects from pool.

#pragma once

#include "utility.h"
#include "vector.h"
#include "list.h"
#include "threads.h"

namespace Ut {

////////////////////////////////////////////////////////

template <typename T>
class SameSizeObject
{
	U8 data[sizeof(T)];
};

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

	// for debug (slow since list is not counted)
	int Allocated() const {return N - m_list.Size();}
	int Remaining() const {return m_list.Size();}

protected:

#ifndef __GNUC__
	_declspec(align(16))
#endif
	UtVector<ArrayVector<SameSizeObject<T>, N> > m_pool;

	// dummy list item type
	struct ListItem : public ListElementSingle<ListItem> {void Destroy() {}};

	UtList<ListItem, SingleEndedList, OnDemandCounter> m_list;

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
	assert(m_pool.Validate((SameSizeObject<T>*)p)); // check that the pointer came from the pool

	// Call object's destructor.
	p->T::~T();

	m_list.PushFront((ListItem&) *p);
}

/////////////////////////////////////////////////////////////
// DynamicPool
/////////////////////////////////////////////////////////////

/// @addtogroup Allocators
/// @{

/// Dynamic pool of objects of type T.
/// When the pool is empty a new block is added automatically.
/// @param T The type of object allocated by the pool.
/// @param AllocatorType The allocator used to create memory blocks.
/// @param See @ref Threading_Models.
// todo: clean-up function to free unused blocks
// default parameters in forward definition
//template <class T, typename AllocatorType = ArrayAllocator<U8>, class ThreadingModel = SingleThreaded>
template <class T, typename AllocatorType, class ThreadingModel>
class DynamicPool
{
public:
	DynamicPool();
	~DynamicPool();

	T* Allocate(); ///< Allocate an object.
	void Free(T* p); ///< Free an object.

	/// This gives a pool type which allocates U instead of T.
	template <typename U> struct Rebind {typedef DynamicPool<U, AllocatorType, ThreadingModel> Other;};

	friend void Swap(DynamicPool& left, DynamicPool& right) {MoveSwap(left, right);}

private:
	void Grow();

	// hold pointers to blocks
	UtVector<DynamicVector<U8*> > m_pool; // deque may be better if the number of blocks is large

	// dummy list item type
	struct ListItem : public ListElementSingle<ListItem> {void Destroy() {}};

	UtList<ListItem, SingleEndedList, OnDemandCounter> m_list;

	ThreadingModel m_threadData;

	DynamicPool(DynamicPool& other); // copy protection
	void operator = (DynamicPool& other); // copy protection
};

/// @}

/////////////////////////////////////////////////////////////////////

// Separate class to Pool so that you can declare Pool<T> before defining T.
template <class T>
struct PoolTypes
{
	const static int N = UtMaths::StaticMax<4 * 1024 / (int)sizeof(T), 1>::RESULT;
	typedef UtVector<ArrayVector<SameSizeObject<T>, N> > Block;
};

template <class T, typename AllocatorType, class ThreadingModel>
inline DynamicPool<T, AllocatorType, ThreadingModel>::DynamicPool()
{
	BIZ_STATIC_ASSERT(sizeof(T) >= sizeof(ListItem)); // must be space for a ListItem inside a T
}

template <class T, typename AllocatorType, class ThreadingModel>
DynamicPool<T, AllocatorType, ThreadingModel>::~DynamicPool()
{
	assert(m_list.Size() == PoolTypes<T>::N * m_pool.Size());

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

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	// Call default constructor for new object.
	new(p) T;

	return p;
}

template <class T, typename AllocatorType, class ThreadingModel>
inline void DynamicPool<T, AllocatorType, ThreadingModel>::Free(T* p)
{
	typename ThreadingModel::Lock lock(m_threadData);

	// Call object's destructor.
	p->T::~T();

	m_list.PushFront((ListItem&) *p);
}

template <class T, typename AllocatorType, class ThreadingModel>
void DynamicPool<T, AllocatorType, ThreadingModel>::Grow()
{
	U8* data = AllocatorType::Allocate(sizeof(typename PoolTypes<T>::Block));//new PoolTypes<T>::Block;
	typename PoolTypes<T>::Block* newVector = (typename PoolTypes<T>::Block*) data;
	new (newVector) typename PoolTypes<T>::Block;

	m_pool.PushBack(data);

	for (int i = 0; i < PoolTypes<T>::N; i++)
	{
		m_list.PushFront((ListItem&) (*newVector)[i]);
	}
}

/// @} end group

} // end Ut
