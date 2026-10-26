// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_NAT_TRAV_LISTENER_H
#define BD_NAT_TRAV_LISTENER_H

#include <bdCore/bdCore.h>

/// Callback interface for NAT traversal.
class bdNATTravListener
{
	public:

		// BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdNATTravListener();

		virtual ~bdNATTravListener();

		/// Called when NAT Traversal succeeds.
		/// \param remote[in] Common address of the host traversed to.
		/// \param realAddr[in] Discovered address to be used for communication with \a remote.
		virtual void onNATAddrDiscovery(bdCommonAddrRef remote, 
										const bdAddr &realAddr);

		/// Called when NAT traversal fails.
		/// \param remote[in] Common address of the host traversal was attempted to.
		virtual void onNATAddrDiscoveryFailed(bdCommonAddrRef remote);

};

#endif // BD_NAT_TRAV_LISTENER_H
