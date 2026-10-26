#ifndef MYMEMORY_H
#define MYMEMORY_H

#ifdef _WIN32
#pragma once
#endif

#pragma warning (disable:4100)
#pragma warning (disable:4514)

#ifndef Assert
#define Assert(X) assert(X)
#endif
#ifndef Mem_Alloc
#define Mem_Alloc(X) malloc(X)
#endif 
#ifndef Mem_Free
#define Mem_Free(X) free(X)
#endif
#ifndef Mem_Realloc
#define Mem_Realloc(X,Y) realloc(X,Y)
#endif

// CMyMemory
template< class T >
class CMyMemory
{
public:
	CMyMemory( int nGrowSize = 0, int nInitSize = 0 );
	CMyMemory( T* pMemory, int numElements );
	~CMyMemory();

	// element access
	T& operator[]( int i );
	T const& operator[]( int i ) const;
	T& Element( int i );
	T const& Element( int i ) const;

	bool IsIdxValid( int i ) const;	// check index

	T* Base();					// Return pointer to the first element
	T const* Base() const;

	int NumAllocated() const;	// Return amount of allocated memory
	int Count() const;			// Return number of elements

	void Grow( int num = 1 );	// Increase the number of elements by 'num'

	void EnsureCapacity( int num ); // Allocate at least this much memory

	void Purge();				// Free up the entire array

	// is the memory externally allocated?
	bool IsExternallyAllocated() const;

	void SetGrowSize( int size );	// Set the size by which the memory grows

	// Attaches the buffer to external memory
	void SetExternalBuffer( T* pMemory, int numElements );
private:
	enum
	{
		CMyMEM_EXTERNAL_BUFFER = -1,
	};

	T* m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};


// ctor, dtor
template< class T >
CMyMemory<T>::CMyMemory( int nGrowSize, int nInitAllocationCount ) : m_pMemory(0), 
	m_nAllocationCount( nInitAllocationCount ), m_nGrowSize( nGrowSize )
{
	Assert( (nGrowSize >= 0) && (nGrowSize != CMyMEM_EXTERNAL_BUFFER) );
	if (m_nAllocationCount)
	{
		m_pMemory = (T*)Mem_Alloc( m_nAllocationCount * sizeof(T) );
		if (m_pMemory == NULL)
			throw std::bad_alloc();
	}
}

template< class T >
CMyMemory<T>::CMyMemory( T* pMemory, int numElements ) : m_pMemory(pMemory),
	m_nAllocationCount( numElements )
{
	// Special marker indicating externally supplied memory
	m_nGrowSize = CMyMEM_EXTERNAL_BUFFER;
}

template< class T >
CMyMemory<T>::~CMyMemory()
{
	Purge();
}


// Attach the buffer to external memory....
template< class T >
void CMyMemory<T>::SetExternalBuffer( T* pMemory, int numElements )
{
	Purge();

	m_pMemory = pMemory;
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = CMyMEM_EXTERNAL_BUFFER;
}


// element access
template< class T >
inline T& CMyMemory<T>::operator[]( int i )
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T const& CMyMemory<T>::operator[]( int i ) const
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T& CMyMemory<T>::Element( int i )
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}

template< class T >
inline T const& CMyMemory<T>::Element( int i ) const
{
	Assert( IsIdxValid(i) );
	return m_pMemory[i];
}


// is the memory externally allocated?
template< class T >
bool CMyMemory<T>::IsExternallyAllocated() const
{
	return m_nGrowSize == CMyMEM_EXTERNAL_BUFFER;
}


template< class T >
void CMyMemory<T>::SetGrowSize( int nSize )
{
	Assert( (nSize >= 0) && (nSize != CMyMEM_EXTERNAL_BUFFER) );
	m_nGrowSize = nSize;
}


// Return pointer to the first element
template< class T >
inline T* CMyMemory<T>::Base()
{
	return m_pMemory;
}

template< class T >
inline T const* CMyMemory<T>::Base() const
{
	return m_pMemory;
}


// Size
template< class T >
inline int CMyMemory<T>::NumAllocated() const
{
	return m_nAllocationCount;
}

template< class T >
inline int CMyMemory<T>::Count() const
{
	return m_nAllocationCount;
}


// Check index
template< class T >
inline bool CMyMemory<T>::IsIdxValid( int i ) const
{
	return (i >= 0) && (i < m_nAllocationCount);
}
 

// Add more memory
template< class T >
void CMyMemory<T>::Grow( int num )
{
	Assert( num > 0 );

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = m_nAllocationCount + num;
	while (m_nAllocationCount < nAllocationRequested)
	{
		if ( m_nAllocationCount != 0 )
		{
			if (m_nGrowSize)
			{
				m_nAllocationCount += m_nGrowSize;
			}
			else
			{
				m_nAllocationCount += m_nAllocationCount;
			}
		}
		else
		{
			// Compute an allocation which is at least as big as a cache line...
			m_nAllocationCount = (31 + sizeof(T)) / sizeof(T);
			Assert(m_nAllocationCount != 0);
		}
	}

	if (m_pMemory)
	{
		m_pMemory = (T*)Mem_Realloc( m_pMemory, m_nAllocationCount * sizeof(T) );
		if (m_pMemory == NULL)
			throw std::bad_alloc();
	}
	else
	{
		m_pMemory = (T*)Mem_Alloc( m_nAllocationCount * sizeof(T) );
		if (m_pMemory == NULL)
			throw std::bad_alloc();
	}
}


// Allocate at least this much memory
template< class T >
inline void CMyMemory<T>::EnsureCapacity( int num )
{
	if (m_nAllocationCount >= num)
		return;

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated 
		Assert(0);
		return;
	}

	m_nAllocationCount = num;
	if (m_pMemory)
	{
		m_pMemory = (T*)Mem_Realloc( m_pMemory, m_nAllocationCount * sizeof(T) );
		if (m_pMemory == NULL)
			throw std::bad_alloc();
	}
	else
	{
		m_pMemory = (T*)Mem_Alloc( m_nAllocationCount * sizeof(T) );
		if (m_pMemory == NULL)
			throw std::bad_alloc();
	}
}


// Memory deallocation
template< class T >
void CMyMemory<T>::Purge()
{
	if (!IsExternallyAllocated())
	{
		if (m_pMemory)
		{
			Mem_Free( (void*)m_pMemory );
			m_pMemory = 0;
		}
		m_nAllocationCount = 0;
	}
}


#endif
