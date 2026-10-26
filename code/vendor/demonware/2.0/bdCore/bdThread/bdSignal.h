// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A platform independent implementation of a signaling class

#ifndef BD_SIGNAL_H
#define BD_SIGNAL_H

#include <bdCore/bdMemory/bdMemory.h>

/// A platform independent implementation of a signaling class
class bdSignal
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdSignal();

		/// Destructor
		~bdSignal();

		/// Waits for the signal to be signaled
		void wait();

		/// Signals the signal
		void signal();

	protected:

		// A platform specific handle of the signal.
		bdSignalHandle m_handle;

};
#endif //BD_SIGNAL_H

