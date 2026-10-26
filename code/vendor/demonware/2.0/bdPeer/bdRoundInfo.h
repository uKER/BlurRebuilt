// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Get textual information describing a round of play.

#ifndef BD_ROUND_INFO_H
#define BD_ROUND_INFO_H

#include <bdCore/bdCore.h>

class bdRound;

/// Get a text string describing a bdRound.
class bdRoundInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Writes textual information about a bdRound to a buffer.
		/// \param round[in] A pointer to the bdRound whose
		///		information is required.
		/// \param buf[in,out] The pointer to a buffer into which the
		///		information should be written.
		/// \param length[in] The length of the data pointer to by \a buf
		///		in bytes.
		static bdUInt BD_CALL getInfo(const bdRound *const round,
									  bdNChar8 *buf,
									  const bdUInt length);
	protected:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdRoundInfo();
};

#endif // BD_ROUND_INFO_H
