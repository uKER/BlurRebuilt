// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Handles connection requests from peers to the session host.

#ifndef BD_SESSION_HANDLER_H
#define BD_SESSION_HANDLER_H

#include <bdCore/bdCore.h>

/// This handler interface is called back by the session instance on the
/// session host to determine whether or not a particular peer should be
/// allowed to join.
class bdSessionHandler
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdSessionHandler();

		/// Virtual destructor.
		virtual ~bdSessionHandler();

		/// Called on the session host whenever a new peer attempts to join the
		/// session. This would typically be used if there are particular
		/// reasons upon which a particular peer would not be allowed to join,
		/// e.g., if the maximum number of players has been exceeded, or if the
		/// QoS to the peer is unacceptable.
		/// \param requestUserData[in] The user data sent with the join
		///		request via bdSession::join.
		/// \param	replyUserData[in, out]	If the request is denied any data
		///		written this buffer will be sent back with the join reply.
		///		The data will be passed into the
		///		bdSessionListener::onSessionJoinRefused()
		///		method of any listeners registered on the peer that is
		///		requesting to join.
		/// \return
		///		- True, if the peer should be allowed to join the session.
		///		- False, if the peer should not be allowed to join the session.
		virtual bdBool onSessionConnectRequest(bdBitBufferRef requestUserData,
											   bdBitBuffer *const replyUserData);

};

#endif // BD_SESSION_HANDLER_H
