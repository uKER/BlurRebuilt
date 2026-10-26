// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_THREAD_H
#define BD_THREAD_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdThread/bdRunnable.h>
#include <bdCore/bdThread/bdMutex.h>

class bdThread;

struct bdThreadArgs
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdThreadArgs()
		:m_args(BD_NULL),
		 m_runnable(BD_NULL)
	{
	};

	void *m_args;
	bdRunnable *m_runnable;
	bdThread *m_thread;
};

class bdThread
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Creates a new thread with a runnable object.
		/// \param runnable [in] A pointer to the object that implements
		/// the bdRunnable interface.
		/// \param priority [in] The priority of the thread.
		/// \param stackSize [in]
		bdThread (bdRunnable *runnable,
		          bdInt priority = BD_DEFAULT_THREAD_PRIORITY,
				  bdUInt stackSize = BD_DEFAULT_STACKSIZE);

		/// Starts executing the thread.
		/// \param args [in]		A pointer to arguments for the thread.
		/// \param size [in]		The size of the arguments in bytes. The
		///							arguments are memcopied for the calling
		///							thread to delete them safely.
		/// \return
		///  - True if the thread could be started.
		///  - False otherwise.
		bdBool start(const void *args, 
					 const bdUWord size);

		/// Stops execution of the thread. This function merely signals the
		/// running thread to stop execution. This is the preferred method in order
		/// for the running thread to shutdown gracefully and to release any
		/// resources it may hold. No assumption can be made when the running
		/// thread has actually stopped.
		void stop();

		/// Causes the calling thread to cease execution until this
		/// thread has finished.
		void join();

		/// Calls the destructor in order to release any resources held
		/// by the thread.
		void cleanup();

		/// Sets the priority of the thread. The value is operating system
		/// specific.
		/// \param priority [in] : The new priority of the thread.
		void setPriority(bdInt priority);

		/// Gets the priority of the thread. The value is operating system
		/// specific.
		bdInt getPriority();

		bdBool isRunning() const;

		bdRunnable *getRunnable();

		bdThreadHandle getThreadHandle();

		/// Called after the thread function has finished. Only used on the
		/// PS2 in order to signal a joining thread to the end of this thread
		void wakeupJoiningThread();

	protected:

		/// We make the destructor protected in order to prevent stack
		/// allocation of threads. Calling cleanup() will in turn call
		/// the destructor.
		~bdThread();

		bdRunnable *m_runnable;

		bdThreadArgs m_threadArgs;

		bdThreadHandle m_handle;

		bdBool m_isRunning;

		bdUInt m_stackSize;

		bdUInt m_priority;

#if defined BD_PLATFORM_PSP

		static bdInt BD_CALL threadProc(bdUInt argSize, bdThreadArgs *args);

#else
		static bdInt BD_CALL threadProc(bdThreadArgs *args);
#endif // BD_PLATFORM_PSP

#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP || defined BD_PLATFORM_WII)

		// requires the stack to be allocated (PS2 & PSP: on a 16 byte boundary)
		void *m_stackPointer;
#endif

#if (defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PSP)

		// There is no proper join functionality on the PS2. Here we store
		// the handle of the thread that performed a join. Upon cleanup, the
		// thread will wakeup the calling thread
		bdThreadHandle m_joiningThread;
#endif


};

#endif // BD_THREAD_H
