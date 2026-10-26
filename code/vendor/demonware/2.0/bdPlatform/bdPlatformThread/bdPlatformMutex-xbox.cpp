// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformThread/bdPlatformMutex.h>

#include "assert.h"

bdMutexHandle BD_CALL bdPlatformMutex::createMutex()
{
	const BOOL isInitialOwner = false;

	HANDLE result = CreateMutex(NULL, isInitialOwner, NULL);
	const DWORD err = GetLastError();
	if(ERROR_ALREADY_EXISTS == err)
	{
		// the mutex is not owned by us..
		// however result is valid..
		// if implementation uses NULL as the name attribute..
		// this should not occur
#ifdef BD_DEBUG
		const bool mutexAlreadyExists = false;
		assert(mutexAlreadyExists); //  "This is very bad"
#endif
	}
	else if (ERROR_INVALID_HANDLE == err)
	{
		// the name supplied in create collided with an existing name
		// in other code that uses handles...
		// if implementation uses NULL as the name attribute..
		// this should not occur
#ifdef BD_DEBUG
		const bool invalidHandle = false;
		assert(invalidHandle); //  "This is very bad"
#endif
	}
	else if(NULL == result)
	{
		// the mutex was not created..
		// err will have an interesting value here..
#ifdef BD_DEBUG
		const bool nullHandle = false;
		assert(nullHandle); //  "This is bad"
#endif
	}
	else
	{
		// all good...
	}
	return result;
}

void BD_CALL bdPlatformMutex::lock(bdMutexHandle &handle)
{
	const DWORD timeout = INFINITE;
	const DWORD result = WaitForSingleObject(handle, timeout);
	if(WAIT_ABANDONED == result)
	{
		// Owning thread has terminated... 
		// There is no way to indicate this to the caller
#ifdef BD_DEBUG
		const bool waitAbandoned = false;
		assert(waitAbandoned); //  "This is bad"
#endif
	}
	else if(WAIT_OBJECT_0 == result)
	{
		// this is what we've been waiting for...
	}
	else if(WAIT_TIMEOUT == result)
	{
		// As the timeout is infinite this should never happen ..
#ifdef BD_DEBUG
		const bool waitTimedOut = false;
		assert(waitTimedOut); //  "This is very bad"
#endif
	}
	else if (WAIT_FAILED == result)
	{
		// when debugging it may be useful to know the error, however this 
		// generates a compile time warning as there is no way to use the value
		// with the current design.
		//const DWORD err = GetLastError();
#ifdef BD_DEBUG
		const bool waitFailed = false;
		assert(waitFailed); //  "This is bad"
#endif
	}
	else
	{
		// unexpected error code..
#ifdef BD_DEBUG
		const bool unexpectedErrorCode = false;
		assert(unexpectedErrorCode); //  "This is really bad"
#endif
	}
}

void BD_CALL bdPlatformMutex::unlock(bdMutexHandle &handle)
{
	const BOOL result = ReleaseMutex(handle);
	if(false == result)
	{
		const DWORD err = GetLastError();
		if(ERROR_NOT_OWNER == err)
		{
			// Only mutices owned by us can be released by us.
#ifdef BD_DEBUG
			const bool notOwner = false;
			assert(notOwner); //  "This is bad"
#endif
		}
		else
		{
			// unexpected error code..
#ifdef BD_DEBUG
			const bool unexpectedErrorCode = false;
			assert(unexpectedErrorCode); //  "This is very bad"
#endif
		}
	}
	else
	{
		// all good
	}
}

void BD_CALL bdPlatformMutex::destroy(bdMutexHandle &handle)
{
	const BOOL result = CloseHandle(handle);
	if(false == result)
	{
		// when debugging it may be useful to know the error, however this 
		// generates a compile time warning as there is no way to use the value
		// with the current design.
		//const DWORD err = GetLastError();
#ifdef BD_DEBUG
		const bool failedToCloseHandle = false;
		assert(failedToCloseHandle); //  "This is bad"
#endif
	}
	else
	{
		// all good
	}
}

