// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformError/bdPlatformError.h>

template <typename T> inline 
bdReference<T>::bdReference() 
: m_ptr(BD_NULL)
{

}

template <typename T> inline 
bdReference<T>::bdReference(T* p) 
: m_ptr(p)
{
	if (m_ptr != BD_NULL) 
	{
		m_ptr->addRef();
	}
}

template <typename T> inline 
bdReference<T>::bdReference(const bdReference<T>& other) 
: m_ptr(other.m_ptr)
{
	if (m_ptr != BD_NULL) 
	{
		m_ptr->addRef();
	}
}

/*
template <typename T>
template <typename U> inline
bdReference<T>::bdReference(const bdReference<U>& other)
{

	m_ptr = static_cast<T*>(other.operator->());
	if (m_ptr != BD_NULL) 
	{
		m_ptr->addRef();
	}

}
*/

template <typename T> inline 
bdReference<T>::~bdReference() 
{
	if (m_ptr != BD_NULL) 
	{
		if (m_ptr->releaseRef() == 0) 
		{
			delete m_ptr;
			m_ptr = BD_NULL;
		}
	}
}

template <typename T> inline 
void bdReference<T>::operator =(T* p) 
{
	if (m_ptr != BD_NULL) 
	{
		if (m_ptr->releaseRef() == 0) 
		{
			delete m_ptr;
		}
	}
	m_ptr = p;
	if (m_ptr != BD_NULL) 
	{
		m_ptr->addRef();
	}
}

#ifdef BD_COMPILER_MSVC
template <typename T> inline 
typename const bdReference<T>& bdReference<T>::operator =(const bdReference<T>& other) 
{
	if (&other != this) 
	{
		if (m_ptr != BD_NULL) 
		{
			if (m_ptr->releaseRef() == 0) 
			{
				delete m_ptr;
			}
		}
		m_ptr = other.m_ptr;
		if (m_ptr != BD_NULL) 
		{
			m_ptr->addRef();
		}
	}
	return *this;
}
#endif

/*
template <typename T>
template <typename U> inline 
void bdReference<T>::operator =(const bdReference<U>& other)
{
    if ((reinterpret_cast<void*>(this) != reinterpret_cast<void*>(&other)) 
	//if (&other != this) 
	{
		if (m_ptr != BD_NULL) 
		{
			if (m_ptr->releaseRef() == 0) 
			{
				delete m_ptr;
			}
		}
		// assign the embedded m_ptr of the other reference to this.
		// If you are looking at this it is most likely that you tried
		// to assign two unrelated reference classes.
		const T* temp = other.operator->();
		m_ptr = const_cast<T*>(temp);
		if (m_ptr != BD_NULL) 
		{
			m_ptr->addRef();
		}
	}
}

template <typename T>
template <typename U> inline 
bdBool bdReference<T>::operator ==(const bdReference<U>& other) const
{
	return (m_ptr == other.operator->());
}

template <typename T>
template <typename U> inline 
bdBool bdReference<T>::operator !=(const bdReference<U>& other) const
{
	return (m_ptr != other.operator->());
}
*/
/*
template <typename T> inline 
bdReference<T>::operator bdBool() const
{
	return (m_ptr != BD_NULL);
}
*/

template <typename T> inline 
bdBool bdReference<T>::operator !() const
{
	return (m_ptr == BD_NULL);
}

template <typename T> inline 
bdBool bdReference<T>::isNull() const
{
	return (m_ptr == BD_NULL);
}

template <typename T> inline 
bdBool bdReference<T>::notNull() const
{
	return (m_ptr != BD_NULL);
}

template <typename T> inline 
bdReference<T>::operator T*() 
{
	return m_ptr;
}

template <typename T> inline 
T* bdReference<T>::operator ->() 
{
	return m_ptr;
}

template <typename T> inline 
T* bdReference<T>::operator ->() const
{
	return m_ptr;
}

template <typename T> inline 
T& bdReference<T>::operator *() 
{
	return *m_ptr;
}

template <typename T> inline 
const T& bdReference<T>::operator *() const
{
	return *m_ptr;
}

template <typename T> inline 
bdBool bdReference<T>::operator<(const bdReference<T>& other) const
{
	return m_ptr < other.m_ptr;
}

template <typename T> inline 
bdBool bdReference<T>::operator>(const bdReference<T>& other) const
{
	return m_ptr > other.m_ptr;
}
