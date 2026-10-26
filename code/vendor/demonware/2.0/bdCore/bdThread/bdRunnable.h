// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_RUNNABLE_H
#define BD_RUNNABLE_H

#include <bdCore/bdMemory/bdMemory.h>

/// Purpose: A class that implements a function that can be started
/// in a separate thread.

class bdRunnable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdRunnable();

		/// Virtual destructor
		virtual ~bdRunnable();

		/// The function that is started in a new thread.
		/// \param args[in] : A pointer to arguments that are needed by the thread
		/// \return The exit code of the thread.
		virtual bdUInt run(void* args) = 0;

		/// Signals the thread to stop execution. An implementation
		/// of the run() function should check the state of the m_stop
		/// flag and should return if m_stop is set to true.
		virtual void stop();

		/// Called by bdThread() before a new thread is started.
		void start();

	protected:

		/// A flag to indicate whether execution of the thread should be
		/// terminated.
		bdBool m_stop;
};

#endif //BD_RUNNABLE_H
