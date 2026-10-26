// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A platform independent timing class interface.

#ifndef BD_PLATFORM_TIMING_H
#define BD_PLATFORM_TIMING_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

/// A platform independent interface for timing.
class bdPlatformTiming
{
	public:

		/// Returns a high resolution time stamp. The resolution depends on the system implementation
		/// but is always at least microsecond accurate.
		/// This value cannot be used to determine the current system time but can be used (in conjunction
		/// with \c getElapsedTime()) to determine the time (in microseconds) between 2 time stamps.
		/// \return A time stamp.
		/// \todo (win32): http://www.gamedev.net/community/forums/topic.asp?topic_id=195892	
		/// heavy pci load on some chipsets can cause trouble: Q274323
		/// \todo (ps2): Timer wraps every 13/14 seconds.
		/// \note This function is not intended to be used directly.
		static bdUInt64 BD_CALL getHiResTimeStamp();

		/// Returns the elapsed time (in seconds) between the 2 time stamps returned by \c getHiResTimeStamp().
		/// \param time1[in]: The first time stamp.
		/// \param time2[in]: The second (later) time stamp.
		/// \return The time (in seconds) between the 2 time stamps. Example format: 1.232f which would be
		/// 1.232 seconds (or 1 second 232 milliseconds).
		static bdFloat32 BD_CALL getElapsedTime(const bdUInt64 time1, 
												const bdUInt64 time2);

		/// Puts the calling thread to sleep for the specified amount of time.
		/// \param timeInMs[in]: The amount of time in milliseconds
		static void BD_CALL sleep(bdUInt timeInMs);

		/// Returns a low resolution time stamp. 
		/// The resolution depends on the system implementation but is always accurate to at 
		/// least a second. Unlike \c getHiResTimeStamp, the time stamp returned by this function 
		/// does not wrap often. (every 49.7 days)
		///
		/// This value cannot be used to determine current system time but can be used (in conjunction
		/// with \c getLoResElapsedTime()) to determine the time (in seconds) between 2 time stamps.
		/// \return A time stamp, or 0 if time stamp acquisition fails.
		/// \todo (ps2): Because PS2 clock access needs to account for DVD accesses, this can only
		/// be implemented during title integration. Otherwise on PS2, this returns time stamps that
		/// wrap every 13-14 seconds.
		static bdUInt32 BD_CALL getLoResTimeStamp();

		/// Returns the elapsed time (in seconds) between the 2 time stamps returned by \c getLoResTimeStamp().
		/// \param time1[in]: The first time stamp.
		/// \param time2[in]: The second (later) time stamp.
		/// \return The time (in seconds) between the 2 time stamps.
		static bdUInt32 BD_CALL getLoResElapsedTime(const bdUInt32 time1,
													const bdUInt32 time2);

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformTiming();
};

#endif // BD_PLATFORM_TIMING_H
