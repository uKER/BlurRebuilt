#ifndef NETTRANSPORT_POOL_H
#define NETTRANSPORT_POOL_H

#ifdef _MSC_VER
#pragma once
#endif


namespace netTransport
{
	template<typename TYPE>
	struct Element
	{
		Element() { m_nextReleasedElement = NULL; }

		TYPE		 m_value;
		Element*	 m_nextReleasedElement;
	};

	template<typename TYPE>
	class Pool
	{
	public:
		Pool(int size);
		virtual ~Pool();

		TYPE* acquire();
		void release(TYPE* ptr);
		
		int acquiredCount() const { return m_acquiredCount; }
		int dynamicCount() const { return m_dynamicCount; }
				
	private:
		Element<TYPE>* element(const TYPE* ptr);

		const int		m_poolSize;
		Element<TYPE>*	m_elements;
		Element<TYPE>*	m_firstReleasedElement;
		int				m_acquiredCount;
		int				m_dynamicCount;
	};

	template<typename TYPE>
	Pool<TYPE>::Pool(int size)
	: m_poolSize(size)
	{
		MEMORY_TRACKER_PUSH("network.multiplayer");
		m_elements = new Element<TYPE>[size];
		MEMORY_TRACKER_POP("network.multiplayer");


		m_firstReleasedElement = &(m_elements[0]);

		for(int i = 0;  i < m_poolSize-1; i ++)
		{
			m_elements[i].m_nextReleasedElement = &(m_elements[i+1]);
		}
		m_elements[m_poolSize-1].m_nextReleasedElement = NULL;
		m_acquiredCount = 0;
		m_dynamicCount = 0;
	}

	template<typename TYPE>
	Pool<TYPE>::~Pool()
	{
		assert(m_acquiredCount == 0);
		assert(m_dynamicCount == 0);
		delete[] m_elements;
	}

	template<typename TYPE>
	Element<TYPE>* Pool<TYPE>::element(const TYPE* ptr)
	{
		TYPE* first = &(m_elements[0].m_value);

		int index = (((const char*)ptr - (const char*)first) / sizeof(m_elements[0]));

		if(index < 0 || index >= m_poolSize)
			return NULL;
		
		assert(&(m_elements[index].m_value) == ptr);
		return &(m_elements[index]);
	}

	template<typename TYPE>
	TYPE* Pool<TYPE>::acquire()
	{
		Element<TYPE>* freeElement = m_firstReleasedElement;
		if(freeElement == NULL)
		{
			m_dynamicCount++;
			return new TYPE();
		}
		m_firstReleasedElement = m_firstReleasedElement->m_nextReleasedElement;
		freeElement->m_nextReleasedElement = NULL;
		m_acquiredCount++;
		return &(freeElement->m_value);
	}

	template<typename TYPE>
	void Pool<TYPE>::release(TYPE* ptr)
	{
		Element<TYPE>* e = element(ptr);
		if(e == NULL)
		{
			m_dynamicCount--;
			delete ptr;
			return;
		}
		e->m_nextReleasedElement = m_firstReleasedElement;
		m_firstReleasedElement = e;
		m_acquiredCount--;
	}
}

#endif //NETTRANSPORT_POOL_H