// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: This class implements a basic unreliable sliding window of sequence numbers for detection
// of duplicate, and old packets.

#ifndef BD_SEQUENCE_NUMBER_STORE_H
#define BD_SEQUENCE_NUMBER_STORE_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdSequenceNumber.h>

/// Implements a basic unreliable sliding window of sequence numbers for detection
/// of duplicate, and old packets. The window is of a constant size of 32 or
/// 64 numbers (see \c m_bitmap). Sequence number can be simply fed into the
/// window through \c check() method, which updates the window and returns the
/// status of the sequence number in question. At any one time only numbers
/// falling inside the window are expected. Older numbers (where sequence numbers
/// are expected to be incremented by 1, x "older" y refers to x < y) are
/// rejected. Numbers within the window are checked for duplicates. Numbers
/// newer than the newest one in the window cause it to advance forward. However,
/// numbers that are further away than a whole window are rejected as well.
class bdSequenceNumberStore
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Possible results of checking an incoming sequence number.
		enum bdSequenuceStatus
		{
			/// Invalid number: too old (outside the window) or too far in the
			/// future (much larger than the largest one in the window).
			BD_SN_INVALID_SMALLER	= -2,
			/// Number is valid and is in the window (though it does not cause
			/// window to advance, a larger number has been encountered before).
			BD_SN_VALID_SMALLER	= -1,
			/// Number has been encountered before.
			BD_SN_INVALID_DUPLICATE = 0,
			/// Number is valid and is the largest encountered so far. It caused
			/// the window to advance.
			BD_SN_VALID_LARGER	= 1,
			BD_SN_VALID_MUCH_LARGER = 2
		};

		/// Initializes with the first sequence number in the window.
		/// \param initial The first sequence number in the window.
		bdSequenceNumberStore(const bdSequenceNumber &initial);

		/// Checks and records a new incoming number.
		///
		/// \note This also reports as valid (and records) the case of the
		/// incoming sequence number being much larger than the ones in the
		/// window. This can correspond to, for example, long period of packet
		/// drop. However, because of this, this class should not be used to
		/// enforce security.
		/// \param thisSeq Incoming sequence number.
		/// \return Status of the sequence number.
		enum bdSequenuceStatus check(const bdSequenceNumber &thisSeq);

		/// Returns the last sequence number in the window.
		/// \return The last sequence number in the window.
		const bdSequenceNumber &getLastSequenceNumber() const;

		/// Resets the window to its initial state.
		/// \param initial The first sequence number in the window.
		/// \return The first sequence number in the window.
		void reset(const bdSequenceNumber &initial);

	protected:

		/// Using a 32 bit int for the window so the max window size is 32.
		/// Use a bdUInt64 for a window of 64.
		bdUInt32 m_bitmap;

		/// Largest number encountered so far. It corresponds to the LSB position
		/// in the window.
		bdSequenceNumber m_lastSeq;
};


#endif // BD_SEQUENCE_NUMBER_STORE_H
