// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_THREAD_H
#define BD_PLATFORM_THREAD_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

class bdPlatformThread
{
	public:

		/// Creates a new thread. Execution is not started until start() has been called.
		/// \param threadProc [in] : a function pointer to the thread
		/// \param args [in] : function arguments
		/// \param handle [out] : A platform specific handle that is assigned by
		///						  the OS to refer to the thread
		/// \param priority [in] : The thread's priority
		/// \param stackSize [in] : The number of bytes to be allocated for the thread's stack
		/// \param stackPointer [in] : A pointer to the stack allocated for the thread
		/// \note : Only the PS2/PSP needs to have the stack size specified and the stack
		///         allocated on a 16 byte boundary.
		/// \return
		///  - True if a new thread was created successfully.
		///  - False otherwise.
		static bdBool BD_CALL createThread(bdThreadProc threadProc,
										   void *args,
										   bdThreadHandle &handle,
										   bdInt priority = BD_DEFAULT_THREAD_PRIORITY,
										   bdUInt stackSize = BD_DEFAULT_STACKSIZE,
										   void *stackPointer = BD_NULL);

		/// Starts a previously created thread.
		/// \param handle[in] : The handle of the thread
		/// \param args[in] : additional run-time arguments
		/// \param argSize[in] : size of the additional arguments in bytes
		/// \return
		///  - True : the thread could be started
		///	 - False: an error occurred
		static bdBool BD_CALL startThread(bdThreadHandle& handle,
										  void *args = BD_NULL,
										  bdUInt argSize = 0);

		/// Causes the calling thread to cease execution until the thread
		/// identified by handle exits.
		/// \param handle [in] : The handle of the thread we want to join with.
		/// \note : On the PS2 there is no equivalent system call. Instead, joining of threads
		/// is implemented at a higher level in the bdThread class
		static void BD_CALL joinThread(bdThreadHandle& handle);

		/// Deletes the thread identified by handle.
		/// \param handle [in] : The handle of the thread that is deleted.
		static void BD_CALL deleteThread(bdThreadHandle& handle);

		/// Sets the priority of a given thread.
		/// \param handle [in] : The handle of the thread whose priority is changed.
		/// \param priority [in] : The thread's priority
		/// \note : Priority values are platform specific. There is no equivalent
		/// system call for pthreads.
		static void BD_CALL setPriority(bdThreadHandle& handle,
										const bdInt priority);

		/// Gets the priority of a given thread.
		/// \param handle [in] : The handle of the thread whose priority is queried.
		/// \note : Priority values are platform specific. There is no equivalent
		/// system call for pthreads.
		static bdInt BD_CALL getPriority(bdThreadHandle& handle);

		/// Gets the priority of the calling thread.
		/// \note : Priority values are platform specific. There is no equivalent
		/// system call for pthreads.
		static bdInt BD_CALL getPriority();

		/// Gets the thread handle of the calling  thread.
		static bdThreadHandle getThreadHandle();

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformThread();

};

#endif //BD_PLATFORM_THREAD_H

