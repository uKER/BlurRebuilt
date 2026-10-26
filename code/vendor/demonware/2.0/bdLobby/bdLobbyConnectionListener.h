// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A listener for new incoming connections and disconnected 
//          connections.

#ifndef BD_LOBBYCONNECTION_LISTENER_H
#define BD_LOBBYCONNECTION_LISTENER_H

#include <bdCore/bdCore.h>


BD_REFERENCE(bdLobbyConnection);

/// A listener for new incoming connections and disconnected connections.
class bdLobbyConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		bdLobbyConnectionListener();

		virtual ~bdLobbyConnectionListener();
		
		/// Call to notify of a new connection. This indicates a connection
		/// has been fully established. It will be called for a new incoming
		/// connection and when an outgoing connection is established.
		/// \param connection[in] The connection that has been connected.
		virtual void onConnect(bdLobbyConnectionRef connection);
		
		/// Call to notify when an outgoing connection failed to connect.
		/// \param connection[in] The connection that failed.
		virtual void onConnectFailed(bdLobbyConnectionRef connection);

		/// Call to notify of a disconnected connection. 
		/// The connection can have been disconnected remotely or locally and
		/// can have died due to network or remote host problems.
		/// \param connection[in] The connection that was disconnected.
		virtual void onDisconnect(bdLobbyConnectionRef connection);

};

#endif // BD_LOBBYCONNECTION_LISTENER_H
