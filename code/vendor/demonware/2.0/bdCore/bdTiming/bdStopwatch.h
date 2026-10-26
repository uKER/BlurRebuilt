// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A platform independent timing class interface.

#ifndef BD_STOPWATCH_H
#define BD_STOPWATCH_H

#include <bdCore/bdMemory/bdMemory.h>

/// A platform independent timing class interface.
/// A class that times the duration of operations and events.
/// The main purpose is to provide an operating system independent interface
/// to the programmer.
class bdStopwatch
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdStopwatch();

		/// Start the stopwatch running.
		/// The current timings are not cleared before starting the timer.
		/// To clear the current timings call reset().
		void start();

		/// Clear the current timings.
		/// The time values stored in the stopwatch are reset to zero.
		void reset();

		/// Get the elapsed time between the call to start() and now.
		/// This is equivalent to calling stop() followed by
		/// getTimeInSeconds(), but is a better alternative since stop()
		/// is non-const.
		/// \return		A float in the form of 'a.b' where 'a' is the second
		///				count and 'b' is the fraction of the second.
		bdFloat32 getElapsedTimeInSeconds() const;

	protected:

		/// The last start time.
		bdUInt64 m_start;

};

#endif // BD_STOPWATCH_H
