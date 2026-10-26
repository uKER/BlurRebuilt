// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline 
bdReferencable::bdReferencable()
: m_refCount(0)
{
}

inline
bdInt bdReferencable::addRef() 
{
#ifdef BD_PLATFORM_PS3
	cellAtomicIncr32( reinterpret_cast<uint32_t *>( &m_refCount ) );
	return m_refCount;
#elif defined(BD_PLATFORM_XENON)
	InterlockedIncrement(reinterpret_cast<volatile LONG *> (&m_refCount));
	return m_refCount;
#else
	return ++m_refCount;
#endif
}

inline 
bdInt bdReferencable::releaseRef() 
{
#ifdef BD_PLATFORM_PS3
	cellAtomicDecr32( reinterpret_cast<uint32_t *>( &m_refCount ) );
	return m_refCount;
#elif defined(BD_PLATFORM_XENON)
	InterlockedDecrement(reinterpret_cast<volatile LONG *> (&m_refCount) );
	return m_refCount;
#else
	return --m_refCount;
#endif
}

inline 
bdInt bdReferencable::getRefCount() const
{
	return m_refCount;
}


