// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#include <bdCore/bdMemory/bdDefaultMemory.h>

#define BD_LOG_LEVEL "core"

bdBool bdCore::m_initialized = false;

void BD_CALL bdCore::init(const bdBool defaultMemoryFunctions)
{
	if(!m_initialized)
	{
		if(defaultMemoryFunctions)
		{
			// Setup the memory manager
			bdMemory::setAllocateFunc(bdDefaultMemory::allocate);
			bdMemory::setAlignedAllocateFunc(bdDefaultMemory::alignedAllocate);
			
			bdMemory::setDeallocateFunc(bdDefaultMemory::deallocate);
			bdMemory::setAlignedDeallocateFunc(bdDefaultMemory::alignedDeallocate);

			bdMemory::setReallocateFunc(bdDefaultMemory::reallocate);
			bdMemory::setAlignedReallocateFunc(bdDefaultMemory::alignedReallocate);

#if defined BD_PLATFORM_WII

			bdMemory::setAllocateFunc2(bdDefaultMemory::allocate2);
			bdMemory::setAlignedAllocateFunc2(bdDefaultMemory::alignedAllocate2);

			// Note the use of regular deallocate, as it is clever and knows which heap to free from
			bdMemory::setDeallocateFunc2(bdDefaultMemory::deallocate);
			bdMemory::setAlignedDeallocateFunc2(bdDefaultMemory::alignedDeallocate);

			bdMemory::setReallocateFunc2(bdDefaultMemory::reallocate2);
			bdMemory::setAlignedReallocateFunc2(bdDefaultMemory::alignedReallocate2);

			bdDefaultMemory::init();
			
#endif // BD_PLATFROM_WII
			
		}

		m_initialized = true;	
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "init() has been called twice without an intermediate quit()");
	}
}

void BD_CALL bdCore::quit()
{
	if(m_initialized)
	{

		bdSingletonRegistry::getInstance()->cleanUp();

#if defined (BD_MALLOC_MEMORY) || defined (BD_DEBUG_MEMORY)
		bdDefaultMemory::leakCheck();
#endif // (BD_MALLOC_MEMORY) || defined (BD_DEBUG_MEMORY)

		/// Test facilitation.
		/// In order to check that bdCore::init() is working we must set these pointers
		/// to BD_NULL so that they can be seen to change if init and quit are called repeatedly.
		/// Set these pointers to null so that we can call init -> quit -> init sensibly.
		bdMemory::setAllocateFunc(BD_NULL);
		bdMemory::setAlignedAllocateFunc(BD_NULL);
		
		bdMemory::setDeallocateFunc(BD_NULL);
		bdMemory::setAlignedDeallocateFunc(BD_NULL);

		bdMemory::setReallocateFunc(BD_NULL);
		bdMemory::setAlignedReallocateFunc(BD_NULL);

#if defined BD_PLATFORM_WII

		bdMemory::setAllocateFunc2(BD_NULL);
		bdMemory::setAlignedAllocateFunc2(BD_NULL);
		
		bdMemory::setDeallocateFunc2(BD_NULL);
		bdMemory::setAlignedDeallocateFunc2(BD_NULL);

		bdMemory::setReallocateFunc2(BD_NULL);
		bdMemory::setAlignedReallocateFunc2(BD_NULL);

#endif

		m_initialized = false;
	}
	else
	{	
		BD_WARN(BD_LOG_LEVEL, "quit() has been called twice without an intermediate init()");
	}
}

bdCore::bdCore()
{

}
