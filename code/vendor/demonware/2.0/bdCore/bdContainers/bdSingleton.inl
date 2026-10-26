// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

template <typename T> inline
T* BD_CALL bdSingleton<T>::getInstance()
{	
	// lazy singleton creation
	if(m_instance == BD_NULL)
	{
		m_instance = new T;
		if(m_instance)
		{
			if(!bdSingletonRegistry::getInstance()->add(bdSingleton<T>::destroyInstance))
			{
				// failed to register singleton for clean up
				delete m_instance;
				m_instance = BD_NULL;
				BD_BREAKPOINT();
			}
		}
		else
		{
			// failed to create singleton
			BD_BREAKPOINT();
		}
	}

	return m_instance;	
}

template <typename T> inline 
void bdSingleton<T>::replaceInstance(const T *const newInstance)
{
	if(m_instance)
	{
		delete m_instance;
	}
	m_instance = newInstance;
}

template <typename T> inline
void BD_CALL bdSingleton<T>::destroyInstance()
{
	if (m_instance)
	{
		delete m_instance;
		m_instance = BD_NULL;
	}
}

template <typename T> inline
bdSingleton<T>::bdSingleton()
{

}

