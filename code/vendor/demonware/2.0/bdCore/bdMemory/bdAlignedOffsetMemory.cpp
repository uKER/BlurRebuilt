// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdAlignedOffsetMemory.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdCore/bdUtilities/bdBitOperations.h>

void* BD_CALL bdAlignedOffsetMalloc(const bdUWord size,
									const bdUWord align,
									const bdUWord offset)
{
	if(!BD_IS_POWER_OF_2(align))
	{	
		BD_ASSERT(BD_IS_POWER_OF_2(align), "bdAlignedOffsetMalloc, alignment must "
										   "a power of 2.");
		return BD_NULL;
	}
	
	const bdUWord padding = align + sizeof(void*) + offset;
	const bdUWord blockPtr = reinterpret_cast<bdUWord>(bdMalloc(padding + size));

	if(!blockPtr)
	{
		return BD_NULL;
	}

	const bdUWord alignedPtr = BD_PREVIOUS_MULTIPLE_OF_M(blockPtr + padding, align);
	const bdUWord dataPtr = alignedPtr - offset;

	// store the original blockPtr before the dataPtr
	bdUWord *const headerPtr =  reinterpret_cast<bdUWord*>(dataPtr - sizeof(void*));
	*headerPtr = blockPtr;

	// check the alignment
	if(!BD_IS_MULTIPLE_OF_M(alignedPtr, align))
	{
		BD_ASSERT(BD_IS_MULTIPLE_OF_M(dataPtr, align), "bdAlignedOffsetMalloc, incorrect alignment.");
	}
	
	return reinterpret_cast<void*>(dataPtr);
}

void BD_CALL bdAlignedOffsetFree(void *p)
{
	// get the pointer originally returned by bdPlatformMalloc
	const bdUWord dataPtr = reinterpret_cast<bdUWord>(p);
	const bdUWord *const headerPtr =  reinterpret_cast<bdUWord*>(dataPtr - sizeof(void*));
	void *const blockPtr = reinterpret_cast<void*>(*headerPtr);

	bdFree(blockPtr);
}

void* BD_CALL bdAlignedOffsetRealloc(void *p,
									 const bdUWord origSize,
									 const bdUWord size,
									 const bdUWord align,
									 const bdUWord offset)
{
	void *const dataPtr = bdAlignedOffsetMalloc(size, align, offset);
	bdMemcpy(dataPtr, p, BD_MIN(size, origSize));
	bdAlignedOffsetFree(p);
	return dataPtr;
}
