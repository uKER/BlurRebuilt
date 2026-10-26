// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Plugable memory management.
#ifndef BD_MEMORY_H
#define BD_MEMORY_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#if defined BD_PLATFORM_WII
#include <bdCore/bdMemory/bdMemory-wii.h>
#else
#include <bdCore/bdMemory/bdMemory-std.h>
#endif

/// A macro to define class specific new and delete operators.
/// Used to ensure class memory is allocated and deallocated through bdMemory.
#define BD_DECLARE_NEW_AND_DELETE_OPERATORS					\
		inline void* operator new(bdUWord nbytes)			\
		{													\
			return bdMemory::allocate(nbytes);				\
		}													\
															\
		inline void operator delete(void* p)				\
		{													\
			bdMemory::deallocate(p);						\
		}													\
															\
		inline void* operator new[](bdUWord nbytes)			\
		{													\
			return bdMemory::allocate(nbytes);				\
		}													\
															\
		inline void operator delete[](void* p)				\
		{													\
			bdMemory::deallocate(p);						\
		}													\
															\
		inline void* operator new(bdUWord,					\
								  void* p)					\
		{													\
			return p;										\
		}													\
															\
		inline void operator delete(void*,					\
									void*)					\
		{													\
															\
		}													\
															\
		inline void* operator new[](bdUWord,				\
									void* p)				\
		{													\
			return p;										\
		}													\
															\
		inline void operator delete[](void*,				\
									  void*)				\
		{													\
															\
		}

#endif // BD_MEMORY_H




