#ifndef __CORE_THREAD_LOCAL_STORAGE_H__
#define __CORE_THREAD_LOCAL_STORAGE_H__

#ifdef PLATFORM_PC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace MT
{

#ifdef PLATFORM_PS3
#define TLS_MINIMUM_AVAILABLE 16
extern __thread uint32_t tlsStorage[ TLS_MINIMUM_AVAILABLE ];
extern bool tls_master_used[TLS_MINIMUM_AVAILABLE];
#endif

/// encapsulate that the system's per-thread local storage
class ThreadLocalStorage
{
#if defined (PLATFORM_SPU)
// SPU doesn't support TLS
public:
	/// set thread-local data, return 0 if no data has been stored yet
	void *GetData() const
	{
		return pData;
	}

	/// retrieve previously stored thread-local data
	void SetData(void *data)
	{
		pData = data;
	}
private:
	void *pData;
#elif defined (COMPILER_VC)
public:
	/// constructor
	ThreadLocalStorage()
	{
		m_key = TlsAlloc();
	}

	/// set thread-local data, return 0 if no data has been stored yet
	void *GetData() const
	{
		return TlsGetValue(m_key);
	}

	/// retrieve previously stored thread-local data
	void SetData(void *data)
	{
	    TlsSetValue(m_key, data);
	}
private:
	DWORD m_key;
#elif defined (PLATFORM_PS3)
public:
	/// constructor
	ThreadLocalStorage()
	{
		for (int i = 0 ; i < TLS_MINIMUM_AVAILABLE ; i++)
		{
			if (!tls_master_used[i])
			{
				tls_master_used[i] = true;
				tlsStorage[i] = 0;
				m_key = i;
				return;
			}
		}
		m_key = 0;
		assert(0);
	}

	/// set thread-local data, return 0 if no data has been stored yet
	void *GetData() const
	{
		return (void *)tlsStorage[ m_key ];
	}

	/// retrieve previously stored thread-local data
	void SetData(void *data)
	{
		tlsStorage[ m_key ] = (uint32_t)data;
	}
private:
	DWORD m_key;
#else
// linux
public:
	/// constructor
	ThreadLocalStorage()
	{
	    pthread_key_create(&m_key, NULL);
	}

	/// set thread-local data
	void *GetData()
	{
		return pthread_getspecific(m_key);
	}

	/// retrieve previously stored thread-local data
	void SetData(void *data)
	{
	    pthread_setspecific(m_key, data);
	}
private:
	pthread_key_t m_key;
#endif
};

}// namespace Ut

#endif
