#ifndef DYNAMIC_ATOMIC_H
#define DYNAMIC_ATOMIC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/threads/atomic.h>
#include <btl/allocators/allocator.h>
#include <system/scheduler/jobqueue.h>

namespace DynamicEffect
{

template<class T>
class AtomicVector : public Btl::Vector<T>
{
public:
	AtomicVector() : Btl::Vector<T>(), m_Size(0), m_pSize(&m_Size), m_Resize(32) {}
	~AtomicVector() {}

	typedef Btl::Vector<T>	Base;

	void Update()
	{
		S32 currentSize = Base::Size();
		if(m_Size >= currentSize)
		{
			Resize(currentSize + m_Resize);
			m_Size = currentSize;
		}
	}

	void UpdateElementPtr()
	{
		m_pElements = &((Btl::Vector<T>&)(*this))[0];
	}

	T* AtomicPushBack()
	{
		S32 queuePosition = MT::AtomicIncrement(m_pSize);
		if(queuePosition < Btl::Vector<T>::Capacity())
		{
			T* pElement = m_pElements[queuePosition];
			return m_pElements[queuePosition];
		}
		else
			return NULL;
	}

	void Resize(S32 resize)
	{
		if(m_Size > resize)
			m_Size = resize;
		Base::Resize(resize);
		m_pElements = &((Btl::Vector<T>&)(*this))[0];
	}

	void Clear()
	{
		m_Size = 0;
	}

	S32 Size() const
	{
		return m_Size;
	}

	S32* SizePtr()
	{
		return &m_Size;
	}

	void Size(S32 size)
	{
		m_Size = size;
	}

	FORCE_INLINE T& operator [] (int i) { return m_pElements[i]; } ///< Random access to vector.
	FORCE_INLINE const T& operator [] (int i) const { return m_pElements[i]; }  ///< Random access to vector.

private:
	S32		m_Size;
	S32*	m_pSize;
	T*		m_pElements;
	S32		m_Resize;
};

class AtomicPoolElement
{
public:
	template<class T> friend class AtomicActiveCache;
// TODO: remove, trying to catch 29790
//private:
// TODO: remove, trying to catch 29790
	S32 m_VectorPosition;
};

// Separate class to Pool so that you can declare Pool<T> before defining T.
#ifdef PLATFORM_SPU
template<class T>
#else
template<class T, typename AllocatorType = Btl::ArrayAllocator<U8> >
#endif
class AtomicDynamicPool
{
public:
	#ifndef PLATFORM_SPU
	AtomicDynamicPool() : m_TotalElements(0), m_PoolElements(0)
	{
		m_pPoolElements = &m_PoolElements;
	}
	~AtomicDynamicPool()
	{
		while(!m_Memory.Empty())
		{
			typedef typename Btl::PoolTypes<T>::Block U;
			U* pVector = (U*) m_Memory.Back();

			for(S32 i = 0; i < Btl::PoolTypes<T>::N; ++i)
			{
				T* pElement = (T*)&(*pVector)[i];
				pElement->T::~T();
			}

			pVector->U::~U();
			AllocatorType::Free((U8*)pVector, sizeof(U));
			m_Memory.PopBack();
		}
	}
	#endif

	#ifndef PLATFORM_SPU
	void Update()
	{
		if(m_PoolElements <= 0)
		{
			m_PoolElements = 0;
			// allocate some new memory and push onto memory
			U8* pData = AllocatorType::Allocate(sizeof(typename Btl::PoolTypes<T>::Block));
			typename Btl::PoolTypes<T>::Block* pVector = (typename Btl::PoolTypes<T>::Block*) pData;
			new (pVector) typename Btl::PoolTypes<T>::Block;

			m_Memory.PushBack(pData);
			m_Pool.Resize(m_Pool.Size() + Btl::PoolTypes<T>::N);

			for(S32 i = 0; i < Btl::PoolTypes<T>::N; ++i)
			{
				T* pElement = (T*)&(*pVector)[i];
				new (pElement) T;

				m_Pool[m_PoolElements] = pElement;

				++m_TotalElements;
				++m_PoolElements;
			}
		}
	}
	#endif

	T* AtomicAllocate()
	{
		T* pElement = NULL;
		S32 poolIndex = MT::AtomicDecrement(m_pPoolElements);
		poolIndex -= 1;
		
		if(poolIndex >= 0)
		{
			// get element from back of list
			pElement = (T*)Spu::Get((INTPTR*)&m_Pool[poolIndex]);	// [Blur2PC][x64] pointer-wide
		}
		return pElement;
	}

	void AtomicFree(T* pElement)
	{
		if(pElement)
		{
			S32 deletedIndex = MT::AtomicIncrement(m_pPoolElements);
			Spu::Set((INTPTR*)&m_Pool[deletedIndex], (INTPTR)pElement);	// [Blur2PC][x64] pointer-wide
		}
	}

	void Free(T* pElement)
	{
		if(pElement)
		{
			S32 deletedIndex = m_PoolElements++;
			m_Pool[deletedIndex] = pElement;
		}
	}

	S32					m_TotalElements;
	S32					m_PoolElements;
	S32*				m_pPoolElements;
	Btl::Vector<T*>		m_Pool;
	Btl::Vector<U8*>	m_Memory;
};

template<class T>
class AtomicActiveCache
{
public:
	#ifndef PLATFORM_SPU
	AtomicActiveCache() : m_TotalElements(0), m_CachedElements(0), m_ActiveElements(0), m_DeletedElements(0)
	{
		m_pCachedElements = &m_CachedElements;
		m_pActiveElements = &m_ActiveElements;
		m_pDeletedElements = &m_DeletedElements;
	}
	~AtomicActiveCache() { }

	// use lists to store active and cached emitters and event instances
	void Reserve(S32 reserve)
	{
		m_Cache.Resize(reserve);
		m_Active.Resize(reserve);
		m_Deleted.Resize(reserve);
	}

	void Update()
	{
		for(S32 i = 0; i < m_DeletedElements; ++i)
			Free(m_Deleted[i]);
		m_DeletedElements = 0;
	}

	void Cache(T* pElement)
	{
		if(m_TotalElements == m_Cache.Size())
		{
			m_Cache.Resize(m_TotalElements * 2 + 1);
			m_Active.Resize(m_TotalElements * 2 + 1);
			m_Deleted.Resize(m_TotalElements * 2 + 1);
		}
		pElement->m_VectorPosition = m_TotalElements;

		m_Cache[m_TotalElements] = pElement;
		m_Active[m_TotalElements] = NULL;
		m_Deleted[m_TotalElements] = NULL;

		++m_CachedElements;
		++m_TotalElements;
	}

	void Cache(S32 elements)
	{
		if(m_TotalElements + elements >= m_Cache.Size())
		{
			m_Cache.Resize(m_TotalElements + elements);
			m_Active.Resize(m_TotalElements + elements);
			m_Deleted.Resize(m_TotalElements + elements);
		}
		T* pElements = Btl::ArrayAllocator<T>::Allocate(elements);
		
		for(S32 i = 0; i < elements; ++i, ++m_TotalElements)
		{
			pElements[elements].m_VectorPosition = m_TotalElements;
			m_Cache[m_TotalElements] = &pElements[elements];
			m_Active[m_TotalElements] = NULL;
			m_Deleted[m_TotalElements] = NULL;
		}

		m_CachedElements += elements;
	}
	#endif

	T* Allocate()
	{
		T* pElement = NULL;

		S32 cachedIndex = --m_CachedElements;
		
		if(cachedIndex >= 0)
		{
			pElement = m_Cache[cachedIndex];
			
			// place onto active list
			S32 activeIndex = m_ActiveElements++;
			pElement->m_VectorPosition = activeIndex;
			m_Active[activeIndex] = pElement;
		}
		return pElement;
	}

	T* AtomicAllocate()
	{
		T* pElement = NULL;
		S32 cachedIndex = MT::AtomicDecrement(m_pCachedElements);
		cachedIndex -= 1;
		
		if(cachedIndex >= 0)
		{
			// get element from back of list
			pElement = (T*)Spu::Get((INTPTR*)&m_Cache[cachedIndex]);	// [Blur2PC][x64] pointer-wide
			
			// place onto active list
			S32 activeIndex = MT::AtomicIncrement(m_pActiveElements);
			Spu::Set((U32*)&pElement->m_VectorPosition, (U32)activeIndex);
			Spu::Set((INTPTR*)&m_Active[activeIndex], (INTPTR)pElement);	// [Blur2PC][x64] pointer-wide
		}
		return pElement;
	}

	void AtomicFree(T* pElement)
	{
		S32 deletedIndex = MT::AtomicIncrement(m_pDeletedElements);
		Spu::Set((INTPTR*)&m_Deleted[deletedIndex], (INTPTR)pElement);	// [Blur2PC][x64] pointer-wide
	}

	void Free(T* pElement)
	{
		// TODO: remove, trying to catch 29790
#ifndef CFG_GOLD		
		if(m_ActiveElements <= 0 || pElement->m_VectorPosition == -1)
			DebugBreak();
#endif
		// TODO: remove, trying to catch 29790
		m_CachedElements =  m_CachedElements < 0 ? 0 : m_CachedElements;

		S32 freePosition = pElement->m_VectorPosition;
		pElement->m_VectorPosition = -1;
		// place onto cached list
		S32 cachedIndex = m_CachedElements++;
		m_Cache[cachedIndex] = pElement;

		// remove from active by copying last element
		T* pEnd = m_Active[--m_ActiveElements];
		pEnd->m_VectorPosition = freePosition;
		m_Active[freePosition] = pEnd;
	}

	#ifndef PLATFORM_SPU
	void Free()
	{
		for(S32 i = 0; i < m_CachedElements; ++i)
		{
			if(T* pDelete = m_Cache[i])
				delete pDelete;
		}

		for(S32 i = 0; i < m_ActiveElements; ++i)
		{
			if(T* pDelete = m_Active[i])
				delete pDelete;
		}

		m_CachedElements = 0;
		m_ActiveElements = 0;
		m_TotalElements = 0;
	}
	#endif
	
	Btl::Vector<T*>		m_Cache;
	Btl::Vector<T*>		m_Active;
	S32					m_CachedElements;
	S32					m_ActiveElements;
	S32*				m_pCachedElements;
	S32*				m_pActiveElements;

private:
	Btl::Vector<T*>		m_Deleted;
	S32					m_DeletedElements;
	S32*				m_pDeletedElements;
	S32					m_TotalElements;
};

}

#endif // DYNAMIC_ATOMIC_H
