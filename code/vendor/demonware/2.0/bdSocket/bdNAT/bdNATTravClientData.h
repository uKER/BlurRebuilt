// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NAT_TRAV_CLIENT_DATA_H
#define BD_NAT_TRAV_CLIENT_DATA_H

#include <bdCore/bdCore.h>

class bdNATTravListener;

/// Data structure to hold information about pending traversal request.
class bdNATTravClientData
{
	public:
		/// Traversal goes through several stages.
		enum bdNATTravClientDataState
		{
			BD_NTCDS_INITIAL,
			BD_NTCDS_STAGE_1,
			BD_NTCDS_STAGE_2,
			BD_NTCDS_STAGE_3
		};

		/// Default constructor.
		bdNATTravClientData();

		/// Constructor.
		/// \param local[in] Common address of the host.
		/// \param remote[in] Common address of the host we're trying to connect to.
		/// \param listener[in] Listener to notify.
		bdNATTravClientData(bdCommonAddrRef local,
							bdCommonAddrRef remote,
							bdNATTravListener* listener);

		/// Notify listeners about successful discovery.
		/// \param remote[in] Remote address of the discovery.
		/// \param realAddr[in] Discovered address to be used for sending data
		///  to the remote peer.
		void callOnNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr &realAddr);

		/// Notify listeners about failed discovery.
		/// \param remote[in] Common address of the host traversal was attempted to
		void callOnNATAddrDiscoveryFailed(bdCommonAddrRef remote);

		/// Current state.
		bdNATTravClientDataState m_state;
		/// Our local CA.
		bdCommonAddrRef m_local;
		/// CA of the remote host we're connecting to.
		bdCommonAddrRef m_remote;

		/// Primary listener for NAT discovery event.
		bdNATTravListener *m_listener;
		/// Second listener for NAT discovery event, if any.
		bdNATTravListener *m_secondaryListener;
		/// Number of times we attempted to send requests so far.
		bdUInt m_tries;
		/// Time since we sent a request.
		bdStopwatch m_lastSent;
		/// True, if this traversal request can be throttled by bandwidth arbitrator.
		bdBool m_throttled;
		/// Total age of the request. Used to purge starved requests.
		bdStopwatch m_age;

};

#endif // BD_NAT_TRAV_CLIENT_DATA_H
