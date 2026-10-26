#ifndef UT_LISTPOOL_H
#define UT_LISTPOOL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Ut
{

template<class T>
class ListPool
{
public:
	ListPool() : m_nElements(0) {}
	~ListPool() { assert(m_nElements == 0); }
	// use lists to store active and cached emitters and event instances
	typedef Ut::UtListIntrusive<T, Ut::DoubleEndedList, Ut::MaintainedCounter>		List;
	typedef typename List::Iterator													Iterator;
	typedef typename List::ConstIterator											ConstIterator;

	void Cache(T* pElement)
	{
		m_Cache.PushBack(*pElement);
		++m_nElements;
	}

	T* Allocate()
	{
		assert(m_Active.Size() + m_Cache.Size() == (S32)m_nElements);
		T* pElement = NULL;

		if(m_Cache.Size() > 0)
		{
			// get emitter from back of list
			pElement = &m_Cache.Back();
			m_Cache.PopBack();
			assert(m_Cache.Size() > -1);
			// push it onto active list
			m_Active.PushBack(*pElement);
		}
		return pElement;
	}

	void Free(T* pElement)
	{
		assert(m_Active.Size() + m_Cache.Size() == (S32)m_nElements);
	
		// erase the emitter from the active list
		m_Active.Erase(typename List::Iterator(pElement));
		assert(m_Active.Size() > -1);
		// push it onto the emitter cache
		m_Cache.PushBack(*pElement);
	}

	typename List::Iterator Free(typename List::Iterator pIterator)
	{
		T* pElement = &(*pIterator);
		typename List::Iterator pReturnIterator;
		assert(m_Active.Size() + m_Cache.Size() == (S32)m_nElements);

		pReturnIterator = m_Active.Erase(pIterator);
		assert(m_Active.Size() > -1);
		m_Cache.PushBack(*pElement);

		return pReturnIterator;
	}

	void Free()
	{
		while(!m_Active.Empty())
		{
			T* pDelete = &m_Active.Front();
			m_Active.PopFront();
			delete pDelete;
			--m_nElements;
		}

		while(!m_Cache.Empty())
		{
			T* pDelete = &m_Cache.Front();
			m_Cache.PopFront();
			delete pDelete;
			--m_nElements;
		}
	}
	
	List				m_Cache;
	List				m_Active;
private:
	U32					m_nElements;
};

}

#endif // UT_LISTPOOL_H
