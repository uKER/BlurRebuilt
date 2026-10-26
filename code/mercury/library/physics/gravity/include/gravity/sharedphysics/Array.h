#ifndef NEWARRAY_H
#define NEWARRAY_H

#include "gravity/sharedPhysics/spu_memory.h"

namespace SharedPhysics
{
#define DEFAULT_ARRAY_SIZE 512

template <class Storage>
class Array
{
public:
	C_SPUMemoryPool* m_pPool;
	Storage *data;
	U32 maxSize;


	Array() : m_pPool(NULL),data(NULL), maxSize(0)
	{
	}

	Array(C_SPUMemoryPool* pPool) : m_pPool(pPool), data(NULL), maxSize(0)
	{
	}

	~Array()
	{
#ifndef PLATFORM_SPU
		if(data && m_pPool == NULL)
			delete [] data;
		data = NULL;
#endif
	}

	void Resize(U32 desiredSize)
	{
		if(desiredSize > maxSize)
		{
			if(m_pPool)
			{
				maxSize = desiredSize + 1;
				data = m_pPool->Allocate<Storage, false>(maxSize);
				
			}
#ifndef PLATFORM_SPU
			else
			{
				if(data)
					delete [] data;
				maxSize = desiredSize + 1;
				data = new Storage[maxSize];
				
			}
#else
			else
			{
				maxSize = desiredSize + 1;
				data = (Storage*)Spu::Alloc(sizeof(Storage) * maxSize);
			}
#endif
		}
	}

	void Resize(U32 desiredSize, U32 align)
	{
		if(desiredSize > maxSize)
		{
			if(m_pPool)
			{
				maxSize = desiredSize + 1;
				data = m_pPool->Allocate<Storage, false>(maxSize, align);
				
			}
#ifndef PLATFORM_SPU
			else
			{
				if(data)
					delete [] data;
				maxSize = desiredSize + 1;
				data = new Storage[maxSize];	
			}
#endif
		}
	}

	Storage& operator [] (size_t index)
	{
		BIZ_ASSERT(index < maxSize);
		return data[index];
	}

	const Storage& operator [] (size_t index) const
	{
		BIZ_ASSERT(index < maxSize);
		return data[index];
	}
};

};

#endif
