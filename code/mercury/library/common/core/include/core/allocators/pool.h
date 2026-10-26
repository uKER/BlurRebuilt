///////////////////////////////////////////////////////
// Pool
///////////////////////////////////////////////////////

// Allocate fixed size objects from pool.

#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <core/utility/coredebug.h>
#include <core/utility/utility.h>
#include <core/utility/threads.h>
#include <core/containers/vector.h>
#include <core/containers/list.h>

namespace Ut {

template <class T>
class ConstructOnAllocate
{
public:
	static void ConstructorOnce(T& r)
	{
		
	}
	static void DestructorOnce(T& r)
	{

	}
	static void ConstructorAllocate(T& r)
	{
		// call constructor
		new(&r) T;
	}
	static void DestructorFree(T& r)
	{
		r.T::~T();
	}
};

template <class T>
class ConstructOnce
{
public:
	static void ConstructorOnce(T& r)
	{
		// call constructor
		new(&r) T;
	}
	static void DestructorOnce(T& r)
	{
		r.T::~T();
	}
	static void ConstructorAllocate(T& r)
	{
		
	}
	static void DestructorFree(T& r)
	{
		
	}
};

////////////////////////////////////////////////////////

/// @addtogroup Allocators
/// @{

/// Pre-allocated pool of objects of type T.
/// @param T The type of object in the pool.
/// @param N The number of pre-allocated objects. 
///	@remarks T must be at least the size of a pointer.
template <class T, int N, template <typename Type> class ConstructionType = ConstructOnAllocate>
class StaticPool
{
public:
	StaticPool();
	~StaticPool();

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
	UtVector<ArrayVector<SameSizeAndAlignment<T>, N> > m_pool;

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

template <class T, int N, template <typename Type> class ConstructionType>
StaticPool<T, N, ConstructionType>::StaticPool()
{
	BIZ_STATIC_ASSERT(sizeof(T) >= sizeof(ListItem)); // must be space for a ListItem inside a T

	for (int i = 0; i < N; i++)
	{
		T& r = (T&)m_pool[i];
		ConstructionType<T>::ConstructorOnce(r);
		m_list.PushFront((ListItem&) r);
	}
}

template <class T, int N, template <typename Type> class ConstructionType>
StaticPool<T, N, ConstructionType>::~StaticPool()
{
	assert(m_list.Size() == N); // check all items have been returned to pool

	for (int i = 0; i < N; i++)
	{
		ConstructionType<T>::DestructorOnce((T&) m_pool[i]);
	}
}

template <class T, int N, template <typename Type> class ConstructionType>
inline T* StaticPool<T, N, ConstructionType>::Allocate()
{
	assert(!m_list.Empty()); // error to call Allocate() when no items remaining

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	// Invoke default constructor on allocated object - this is essential if T has virtual functions.
	ConstructionType<T>::ConstructorAllocate(*p);
	//new(p) T;

	return p;
}

template <class T, int N, template <typename Type> class ConstructionType>
inline void StaticPool<T, N, ConstructionType>::Free(T* p)
{
	assert(m_pool.Validate((SameSizeAndAlignment<T>*)p)); // check that the pointer came from the pool

	// Call object's destructor.
	ConstructionType<T>::DestructorFree(*p);
	//p->T::~T();

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
template <class T, typename AllocatorType, class ThreadingModel, template <typename Type> class ConstructionType>
class DynamicPool
{
public:
	DynamicPool();
	~DynamicPool();

	T* Allocate(); ///< Allocate an object.
	void Free(T* p); ///< Free an object.

	/// This gives a pool type which allocates U instead of T.
	template <typename U> struct Rebind {typedef DynamicPool<U, AllocatorType, ThreadingModel, ConstructionType> Other;};

	friend void Swap(DynamicPool& left, DynamicPool& right) {SwapRaw(left, right);}

	int TotalSize() {return m_size;}
	int UsedCount() {return m_used;}
	int FreeCount() {return m_list.Size();}

private:
	void Grow();

	int m_size;
	int m_used;
	
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
	typedef UtVector<ArrayVector<SameSizeAndAlignment<T>, N> > Block;
};

template <class T, typename AllocatorType, class ThreadingModel, template <typename Type> class ConstructionType>
inline DynamicPool<T, AllocatorType, ThreadingModel, ConstructionType>::DynamicPool() : m_size(0), m_used(0)
{
	BIZ_STATIC_ASSERT(sizeof(T) >= sizeof(ListItem)); // must be space for a ListItem inside a T
}

template <class T, typename AllocatorType, class ThreadingModel, template <typename Type> class ConstructionType>
DynamicPool<T, AllocatorType, ThreadingModel, ConstructionType>::~DynamicPool()
{
	assert(m_list.Size() == PoolTypes<T>::N * m_pool.Size());

	m_list.Abandon(); // stop list clearing

	while (!m_pool.Empty())
	{
		typedef typename PoolTypes<T>::Block U;
		U* pBlock = (U*) m_pool.Back();
		
		for (int i = 0; i < PoolTypes<T>::N; i++)
		{
			ConstructionType<T>::DestructorOnce((T&)(*pBlock)[i]);
		}
		
		pBlock->U::~U();
		AllocatorType::Free(m_pool.Back(), sizeof(U));
		m_pool.PopBack();
	}
}

template <class T, typename AllocatorType,  class ThreadingModel, template <typename Type> class ConstructionType>
inline T* DynamicPool<T, AllocatorType, ThreadingModel, ConstructionType>::Allocate()
{
	typename ThreadingModel::Lock lock(m_threadData);

	if (m_list.Empty())
		Grow();

	T* p = (T*) &m_list.Front();
	m_list.PopFront();

	// Call default constructor for new object.
	ConstructionType<T>::ConstructorAllocate(*p);

	m_used++;

	return p;
}

template <class T, typename AllocatorType, class ThreadingModel, template <typename Type> class ConstructionType>
inline void DynamicPool<T, AllocatorType, ThreadingModel, ConstructionType>::Free(T* p)
{
	typename ThreadingModel::Lock lock(m_threadData);

	// Call object's destructor.
	ConstructionType<T>::DestructorFree(*p);

	m_list.PushFront((ListItem&) *p);

	m_used--;
}

template <class T, typename AllocatorType, class ThreadingModel, template <typename Type> class ConstructionType>
void DynamicPool<T, AllocatorType, ThreadingModel, ConstructionType>::Grow()
{
	U8* data = AllocatorType::Allocate(sizeof(typename PoolTypes<T>::Block));//new PoolTypes<T>::Block;
	typename PoolTypes<T>::Block* newVector = (typename PoolTypes<T>::Block*) data;
	new (newVector) typename PoolTypes<T>::Block;

	m_pool.PushBack(data);

	for (int i = 0; i < PoolTypes<T>::N; i++)
	{
		T& r = (T&)(*newVector)[i];
		ConstructionType<T>::ConstructorOnce(r);
		m_list.PushFront((ListItem&) r);
	}

	m_size += (4 * 1024);
}

/// @} end group

} // end Ut
#endif //end _MEM_POOL_H_
