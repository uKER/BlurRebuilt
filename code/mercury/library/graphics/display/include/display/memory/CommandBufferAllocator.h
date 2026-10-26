#ifndef __COMMANDBUFFERALLOCATOR_H__
#define __COMMANDBUFFERALLOCATOR_H__

namespace Display
{

namespace RingBuffer
{

#define RINGBUFFER_SET_SIZE 512		// good for 8MB (512 x 16K)

/// represents a collection of allocated memory
class MemorySet
{
	friend class Allocator;

public:
	/// constructor
	MemorySet();
	/// clear the contents of the set

	void Clear();

private:
	/// add a range of memory to the set of allocations
	void Add(U32 iAddr, U32 cSize);

	/// free a set of memory from another set of memory
	void Free(const MemorySet *pSet);

	/// get pointer to raw set data
	const U8 *GetRawDataPtr() const { return &m_Set[0]; }

	U8 m_Set[RINGBUFFER_SET_SIZE];
};

/// the main allocator which allows us to allocate memory from a ringbuffer
class Allocator
{
public:
	/// constructor
	Allocator();

	/// initialise
	void Initialise(U8 *pMemory, U32 cSize);

	/// allocate an amount of memory (size is rounded up to 128 byte aligned)
	U32 Allocate(U32 cSize, MemorySet *p = NULL);

	/// free some memory
	void Free(const MemorySet *pSet);

	/// get updated 'free memory' pointer
	U32 GetFreeMemPtr();

	/// return the allocation pointer (for debugging purposes)
	U32 GetAllocPtr();

	/// dump memory usage (debug)
	void DumpMemoryUsage();

private:
	U32 m_iAllocPtr;	// pointer to where the next allocation will come from
	U32 m_iFreePtr;		// pointer to the highest address that has been completely freed
	U8 *m_pBuffer;
	U32 m_cSize;
	U8 m_iHighBit;		// 0x80 or 0

	MemorySet m_Set;
};

}

}

#endif

