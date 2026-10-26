// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_SEMAPHORE_H
#define BD_SEMAPHORE_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdCore/bdMemory/bdMemory.h>

class bdSemaphore
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Creates a semaphore.
		/// \param initialCount [in] : The initial value of the semaphore.
		/// \param maxCount [in] : The maximum value the semaphore get.
		bdSemaphore(bdUInt initialCount, bdUInt maxCount);

		/// Releases the semaphore. This effectively increases the semaphore
		/// count by one.
		void release();

		/// Decrements the semaphore count and blocks if the semaphore count
		/// is zero.
		/// \return
		///  - True if the semaphore count is > 0
		///  - False if the semaphore has been destroyed by another thread.
		bdBool wait();

		/// Deletes the semaphore
		void destroy();

	protected:

		/// Prevents stack allocation of the semaphore. Call destroy() in
		/// order to delete the semaphore.
		~bdSemaphore();

		// A platform specific handle of the semaphore.
		bdSemaphoreHandle m_handle;

};
#endif //BD_SEMAPHORE_H

