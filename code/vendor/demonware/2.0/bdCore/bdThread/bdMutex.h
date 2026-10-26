// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_MUTEX_H
#define BD_MUTEX_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdCore/bdMemory/bdMemory.h>

class bdMutex
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdMutex();

		/// Destructor
		~bdMutex();

		/// Attempts to lock the mutex. This function will block
		/// until ownership of the mutex has been gained.
		void lock();

		/// Releases ownership of the mutex
		void unlock();

	protected:

		// A pointer to the handle of the mutex. This is platform
		// specific.
		bdMutexHandle m_handle;

};
#endif //BD_MUTEX_H

