// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Get textual information describing a session.

#ifndef BD_SESSION_INFO_H
#define BD_SESSION_INFO_H

#include <bdCore/bdCore.h>

class bdSession;

/// Get a text string describing a bdSession.
class bdSessionInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Writes textual information about a bdSession to a buffer.
		/// \param session[in] A pointer to the bdSession whose information 
		///		is required.
		/// \param buf[in,out] The pointer to a buffer into which the 
		///		information should be written.
		/// \param length[in] The length of the data pointer to by \a buf 
		///		in bytes.
		/// \return The number for bytes written to the buffer.
		///			Not including the null terminator.
		static bdUInt BD_CALL getInfo(const bdSession *const session,
									  bdNChar8 *buf, 
									  const bdUInt length);

		/// Writes textual information about a given peer to a buffer.
		/// \param session[in] A pointer to the bdSession whose information 
		///		is required.
		/// \param buf[in,out] The pointer to a buffer into which the 
		///		information should be written.
		/// \param length[in] The length of the data pointer to by \a buf 
		///		in bytes.
		/// \return The number for bytes written to the buffer.
		///			Not including the null terminator.
		static bdUInt BD_CALL getPeerInfo(const bdSession *const session,
							 			  bdNChar8 *buf, 
										  const bdUInt length,
										  const bdUInt peerIndex);
	protected:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdSessionInfo();
};

#endif // BD_SESSION_INFO_H
