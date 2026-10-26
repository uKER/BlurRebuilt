// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A listener interface for events associated with a peer-to-peer 
// session.

#ifndef BD_SESSION_LISTENER_H
#define BD_SESSION_LISTENER_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdSession/bdSession.h>
#include <bdConnection/bdConnection.h>

/// This class defines a listener interface for events that may occur in a 
/// bdSession. Classes derived from bdSessionListener can be 
/// registered using bdSession::registerListener.
class bdSessionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		bdSessionListener();

		/// Virtual destructor.
		virtual ~bdSessionListener();

		/// Called on a peer when their request to join a session is 
		/// accepted by the host.
		/// \note This does not mean that the peer is now in the session.
		///		Them must first connect to all other peers.
		virtual void onSessionJoinAccepted();

		/// Called on a peer when their request to join a session is 
		/// refused by the host.
		/// \param userData[in] A bit buffer containing data written by the
		///		host. Typically this would be a refusal reason.
		virtual void onSessionJoinRefused(const bdBitBufferRef userData);

		/// Called when a request to join a session has failed because one or 
		/// more of the session peers were unreachable. This may be the case
		/// even if the peer(s) in question are actually up-and-running and
		/// contactable from other locations. Reasons for this are network
		/// partitions and NAT/firewall issues.
		virtual void onSessionConnectFail();

		/// Called when a request to connect to a session has completed and the
		/// peer has connected to all other peers in the session.
		virtual void onSessionConnectSuccess();

		/// Called when a new peer joins the session. This will \b not \b be 
		/// called when the local peer joins the session.
		/// \param connection[in] The connection to the new peer.
		virtual void onSessionConnect(const bdConnectionRef connection);

		/// Called when an peer disconnects from the session.
		/// \param connection[in] The connection to the disconnecting peer.		
		virtual void onSessionDisconnect(const bdConnectionRef connection);

		/// Called when the status of the session has changed.
		/// \param previous[in] The status from which the session is changing.
		/// \param current[in] The status to which the session is changing.
		virtual void onSessionStatusChange(const bdSession::bdSessionStatus previous,
											const bdSession::bdSessionStatus current);

		/// Called when the role of the peer in the session has changed.
		/// See bdSessionRole for more information.
		/// \param role[in] The new role of the peer.
		virtual void onSessionRoleUpdate(const bdSession::bdSessionRole role);

		/// May be called as the result of the host calling bdSession::sendUpdatePeerList.
		/// This is an indication that the session is partially connected, and this 
		/// peer is only connected to a subset of the peers to which the host is connected.
		/// In this case the peer should leave the session and call bdSession::leave.
		virtual void onForceLeave();

};

#endif // BD_SESSION_LISTENER_H
