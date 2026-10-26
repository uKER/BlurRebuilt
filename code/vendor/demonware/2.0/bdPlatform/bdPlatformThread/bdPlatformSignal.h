// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_SIGNAL_H
#define BD_PLATFORM_SIGNAL_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>
#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#if defined BD_PLATFORM_PS3
#include <pthread_types.h>
#endif

class bdPlatformSignal
{
	public:

		/// Creates a signal
		/// \return A handle to the newly created signal.
		///			- False: otherwise
		static bdSignalHandle BD_CALL create();

		/// Destroys a signal
		/// \param handle[in] The signal to be destroyed
		static void BD_CALL destroy(bdSignalHandle &handle);

		/// Signals a signal
		/// \param handle[in] The signal to be signaled
		static void BD_CALL signal(bdSignalHandle &handle);

		/// Blocks until the signal becomes signaled
		/// \param handle[in] The signal to be waited on
		/// \return
		///  - True
		///  - False otherwise.
		static bdBool BD_CALL wait(bdSignalHandle &handle);

};

#endif // BD_PLATFORM_SIGNAL_H

