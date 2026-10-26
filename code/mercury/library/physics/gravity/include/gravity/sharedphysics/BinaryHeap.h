#ifndef _NEW_BINARYHEAP_H
#define _NEW_BINARYHEAP_H

namespace SharedPhysics
{

/*
	BinaryHeap will sort the elements based on the smallest value. 
	Remeber to overwrite the < operator for the compared elements
	This binaryheap can sort elements based on the largest value as long as the < operator of the elements
	are overwrite properly
*/
template <class Elem>
class BinaryHeap
{
public:
	BinaryHeap( const U32 capacity = 100 ): maxSize(capacity), currentSize(0),
		m_bDestruct(true)
	{
#ifndef PLATFORM_SPU
		heap = new Elem[maxSize];
#else
		heap = (Elem*)Spu::Alloc(sizeof(Elem) * maxSize);
#endif
	}

	BinaryHeap( Elem* pElems, const U32 capacity) : maxSize(capacity), currentSize(0),
		heap(pElems), m_bDestruct(false)
	{
	}

	~BinaryHeap()
	{
#ifndef PLATFORM_SPU
		if(heap && m_bDestruct)
		{
			delete[] heap;
		}
		heap = NULL;
#endif
	}

    NOALIAS inline bool isEmpty( ) const;
    NOALIAS inline bool isFull( ) const;
    NOALIAS inline Elem getTop( ) const;

	NOALIAS inline U32 left(const U32 root) const;
	NOALIAS inline U32 right(const U32 root) const;
	NOALIAS inline int parent(const int child) const;
	NOALIAS inline Elem value(const U32 pos) const;

	NOALIAS inline U32 size() const;

	/*NOALIAS __forceinline*/void insert( const Elem PCREF x);
    NOALIAS inline void setSize(const U32 size);
	NOALIAS inline void deleteTop( );
	NOALIAS inline void deleteTop( Elem& item);
    NOALIAS inline void makeEmpty( );
	

private:
	U32 maxSize;	//the capacity of the heap
    U32	currentSize;  // Number of elements in heap
	Elem* heap;        // The heap array
	bool m_bDestruct;

	NOALIAS inline void reheap(const U32 currentSize);
};

template <class Elem>
class BHStorage
{
public:
	Elem* pElem;

	NOALIAS inline BHStorage() : pElem(NULL)
	{
	}

	NOALIAS inline BHStorage(Elem* elem) : pElem(elem)
	{
	}

	NOALIAS inline BHStorage(const BHStorage& other) : pElem(other.pElem)
	{
	}

	NOALIAS inline BHStorage& operator = (const BHStorage& other)
	{
		pElem = other.pElem;
		return *this;
	}

	NOALIAS inline bool operator < (BHStorage& other)
	{
		return *pElem < *other.pElem;
	}

	NOALIAS inline bool operator <= (BHStorage& other)
	{
		return *pElem <= *other.pElem;
	}

	NOALIAS inline bool operator > (BHStorage& other)
	{
		return *pElem > *other.pElem;
	}

	NOALIAS inline bool operator >= (BHStorage& other)
	{
		return *pElem >= *other.pElem;
	}

	NOALIAS inline bool operator == (BHStorage& other)
	{
		return *pElem == *other.pElem;
	}
};

template <class Elem>
class BinaryHeapStorage
{
public:
	public:
	BinaryHeapStorage( const U32 capacity = 100 ): m_maxSize(capacity), m_currentSize(0), m_bDestruct(true)
	{
#ifndef PLATFORM_SPU
		m_heap = new BHStorage<Elem>[m_maxSize];
#else
		m_heap = (BHStorage<Elem>*)Spu::Alloc(sizeof(BHStorage<Elem>) * m_maxSize);
#endif
	}

	BinaryHeapStorage( BHStorage<Elem>* data, const U32 capacity ): m_maxSize(capacity), m_currentSize(0),
		m_heap(data), m_bDestruct(false)
	{
	}

	~BinaryHeapStorage()
	{
#ifndef PLATFORM_SPU
		if(m_bDestruct && m_heap)
		{
			delete[] m_heap;
		}
#endif
	}

    NOALIAS inline bool isEmpty( ) const;
    NOALIAS inline bool isFull( ) const;
    NOALIAS inline Elem* getTop( ) const;

	NOALIAS inline U32 left(const U32 root) const;
	NOALIAS inline U32 right(const U32 root) const;
	NOALIAS inline int parent(const int child) const;
	NOALIAS inline Elem* value(const U32 pos) const;

	NOALIAS inline U32 size() const;

	NOALIAS inline void insert( Elem* x, U32 currentSize);
	NOALIAS inline void insert( Elem* x);
    NOALIAS inline void setSize(const U32 size);
	NOALIAS inline void deleteTop( );
	NOALIAS inline void deleteTop( Elem** item);
    NOALIAS inline void makeEmpty( );
	

private:
	U32 m_maxSize;	//the capacity of the heap
    U32	m_currentSize;  // Number of elements in heap
	BHStorage<Elem>* m_heap;        // The heap array
	bool m_bDestruct;

	NOALIAS inline void reheap(const U32 currentSize);
};
};

#include "gravity/sharedphysics/BinaryHeap.inl"



#endif
