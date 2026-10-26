// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A listener for new incoming connections and disconnected 
//          connections.

#ifndef BD_CONNECTION_LISTENER_H
#define BD_CONNECTION_LISTENER_H

#include <bdCore/bdCore.h>

// NOTE: Errors pointing to this line probably indicate that you need: 
//       #include <bdConnection/bdConnection.h>
//       in your *.cpp file
BD_REFERENCE(bdConnection);

/// A listener for new incoming connections and disconnected connections.
class bdConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS		

		bdConnectionListener();

		virtual ~bdConnectionListener();
		
		/// Call to notify of a new connection. This indicates a connection
		/// has been fully established. It will be called for a new incoming
		/// connection and when an outgoing connection is established.
		/// \param connection[in] The connection that has been connected.
		virtual void onConnect(bdConnectionRef connection);
		
		/// Call to notify when an outgoing connection failed to connect.
		/// \param connection[in] The connection that failed.
		virtual void onConnectFailed(bdConnectionRef connection);

		/// Call to notify of a disconnected connection. 
		/// The connection can have been disconnected remotely or locally and
		/// can have died due to network or remote host problems.
		/// \param connection[in] The connection that was disconnected.
		virtual void onDisconnect(bdConnectionRef connection);

		/// Call to notify of a reconnected connection. 
		/// A reconnection occurs when a client dies (doesn't disconnect remotely
		/// but disconnects locally) then tries to connect again. The remote side
		/// still has state indicating an active connection but the client doesn't.
		/// When the client reconnects the remote state is reset back to initial state
		/// and the reconnect handler is called.
		/// \param connection[in] The connection that was disconnected.
		/// \note The default implementation of this function is to call the onDisconnect
		/// then the onConnect function.
		virtual void onReconnect(bdConnectionRef connection);
};

#endif // BD_CONNECTION_LISTENER_H
