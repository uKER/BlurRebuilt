// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_MUTEX_H
#define BD_PLATFORM_MUTEX_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>

class bdPlatformMutex
{
	public:

		static bdMutexHandle BD_CALL createMutex();

		/// Attempts to lock the mutex. This function will block
		/// until ownership of the mutex has been gained.
		/// \param handle[in]
		static void BD_CALL lock(bdMutexHandle &handle);

		/// Releases ownership of the mutex
		/// \param handle[in]
		static void BD_CALL unlock(bdMutexHandle &handle);

		/// Attempts to delete the mutex.
		/// \param handle[in]
		static void BD_CALL destroy(bdMutexHandle &handle);

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformMutex();

};
#endif //BD_PLATFORM_MUTEX_H

