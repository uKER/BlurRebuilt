#ifndef SYSTEM_H__INCLUDED
#define SYSTEM_H__INCLUDED

#define USE_CRT_MALLOC 1

// file call-back suite

enum
{
	SYS_FILE_BEGIN,
	SYS_FILE_CURRENT,
	SYS_FILE_END
};

typedef struct tagActToolsLibSystemCallbacks
{
	// File operations.
	int (*theWriteFn)( void* Handle, const void* Data, const size_t Count, size_t* NumberOfBytesWritten );
	int (*theReadFn)( void* Handle, void* Data, const size_t Count );
	unsigned (*theSetPosFn)( void* Handle, const size_t DistanceToMove, int MoveMethod );
	unsigned (*theGetPosFn)( const void* Handle );
	// Errors/debugging.

	// Memory allocation.
	void* (*theMemAllocFn)( size_t aSize );
	void* (*theMemReallocFn)( void* aPtr, size_t aSize );
	void (*theMemFreeFn)( void* aPtr );
} SystemCallbacks;

#endif // SYSTEM_H__INCLUDED

