// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A listener class for LAN discovery related events

#ifndef BD_LAN_DISCOVERY_LISTENER_H
#define BD_LAN_DISCOVERY_LISTENER_H

#include <bdNet/bdLANDiscovery/bdLANDiscovery.h>

BD_REFERENCE(bdGameInfo);

class bdLANDiscoveryListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdLANDiscoveryListener();

		virtual ~bdLANDiscoveryListener();

		/// Called in the event a game has been discovered (client side only).
		/// gameInfo contains the necessary information for clients to connect to
		/// the game
		/// \param gameInfo[in] : Game specific information, such as security credentials
		virtual void onDiscovery(bdGameInfoRef gameInfo);

		/// Signals the end of a discovery process (client side only).
		virtual void onDiscoveryFinished();

		/// Called in the event a discovery request has been received (server side only)
		virtual void onRequest();

};

#endif //BD_LAN_DISCOVERY_LISTENER_H
