#ifndef GENERIC_STACK_ALLOCATOR_H
#define GENERIC_STACK_ALLOCATOR_H


namespace SharedPhysics
{

template <U32 size>
class ALIGN16 C_GenericStackBlock
{
public:
	ALIGN16 U8 m_Data[size];
	U8* m_pCurrentPtr;
	C_GenericStackBlock<size>* m_pNextBlock;

	C_GenericStackBlock() : m_pCurrentPtr(&m_Data[0]), m_pNextBlock(NULL)
	{
	}

	~C_GenericStackBlock()
	{
		if(m_pNextBlock)
			delete m_pNextBlock;
		m_pNextBlock = NULL;
	}

	NOALIAS inline void Reset()
	{
		m_pCurrentPtr = &m_Data[0];	
	}

	NOALIAS inline bool HasSpaceFor(U32 dataSize)
	{
		return ((m_pCurrentPtr + dataSize) - &m_Data[0]) < size;
	}

	template <typename Thing, bool bConstruct, bool bByteAlign>
	NOALIAS inline Thing* Allocate()
	{
		//16-byte align address now!
		Thing* thing;
		if(bByteAlign)
		{
			INTPTR curAddy((INTPTR)m_pCurrentPtr);	// [Blur2PC][x64] align the full address
			curAddy = (curAddy + 15) & ~(INTPTR)0xf;
			thing = (Thing*)curAddy;
			U8* newPtr = (U8*)(curAddy + sizeof(Thing));
			BIZ_ASSERT((newPtr - &m_Data[0]) <= size);
			m_pCurrentPtr = newPtr;
		}
		else
		{
			thing= (Thing*)m_pCurrentPtr;
			U8* newPtr = m_pCurrentPtr + sizeof(Thing);
			BIZ_ASSERT((newPtr - &m_Data[0]) <= size);
			m_pCurrentPtr = newPtr;
		}

		if(bConstruct)
		{
			new (thing) Thing;
		}
		return thing;		
	}

	C_GenericStackBlock<size>* Grow()
	{
		if(!m_pNextBlock)
			m_pNextBlock = new C_GenericStackBlock<size>;
		m_pNextBlock->Reset();
		return m_pNextBlock;
	}
};

#ifndef PLATFORM_SPU
template <U32 size>
class ALIGN16 C_GenericStackAllocator
{
	C_GenericStackBlock<size> m_DataRoot;
	C_GenericStackBlock<size>* m_pCurrentBlock;
#else
class C_GenericStackAllocator
{

#endif

public:
#ifndef PLATFORM_SPU
	C_GenericStackAllocator() : m_pCurrentBlock(&m_DataRoot)
	{
	}

	~C_GenericStackAllocator()
	{
	}

	void Reset()
	{
		m_pCurrentBlock = &m_DataRoot;
	}

	//NB - if you use this and construct, you must ensure you explicitly call the destructor if it is 
	//needed, otherwise you WILL HAVE A MEMORY LEAK!!!
	template <typename Thing, bool bConstruct, bool bByteAlign>
	NOALIAS inline Thing* Allocate()
	{
		if(!m_pCurrentBlock->HasSpaceFor(sizeof(Thing)))
			m_pCurrentBlock = m_pCurrentBlock->Grow();
		return m_pCurrentBlock->Allocate<Thing, bConstruct, bByteAlign>();
	}

#else

	void Reset()
	{
		//Do nothing
	}

	//NB - if you use this and construct, you must ensure you explicitly call the destructor if it is 
	//needed, otherwise you WILL HAVE A MEMORY LEAK!!!
	template <typename Thing, bool bConstruct, bool bByteAlign>
	NOALIAS inline Thing* Allocate()
	{
		Thing* thing = (Thing*)Spu::Alloc(sizeof(Thing));
		new (thing) Thing;
		return thing;		
	}
#endif

	

};

};

#endif
