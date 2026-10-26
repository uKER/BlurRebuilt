// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_SEMAPHORE_H
#define BD_PLATFORM_SEMAPHORE_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#if defined BD_PLATFORM_PS3
#include <semaphore.h>
#endif // PS3

class bdPlatformSemaphore
{
	public:

		/// Creates a semaphore with an initial value and a maximum value
		/// \param initialCount[in]
		/// \param maxCount[in]
		/// \return A handle to the newly created semaphore.
		static bdSemaphoreHandle BD_CALL createSemaphore(bdUInt initialCount,
														 bdUInt maxCount);

		/// Destroys a semaphore. Threads that are waiting on the
		/// semaphore will be unblocked and wait() returns false
		/// \param handle[in] The handle for the semaphore.
		static void BD_CALL destroy(bdSemaphoreHandle &handle);

		/// Increments the semaphore count by one
		/// \param handle[in] The handle for the semaphore.
		static void BD_CALL release(bdSemaphoreHandle &handle);

		/// Waits on a semaphore until it is signaled
		/// \param handle[in] The handle for the semaphore.
		/// \return
		///  - True : if the semaphore was signaled
		///	 - False: the semaphore was destroyed
		static bdBool BD_CALL wait(bdSemaphoreHandle &handle);

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformSemaphore();

};
#endif //BD_PLATFORM_SEMAPHORE_H

