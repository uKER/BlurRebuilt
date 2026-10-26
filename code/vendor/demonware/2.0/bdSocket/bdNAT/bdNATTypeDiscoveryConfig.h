// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the configuration information for the socket router.

#ifndef BD_NAT_TYPE_DISCOVERY_CONFIG_H
#define BD_NAT_TYPE_DISCOVERY_CONFIG_H

#include <bdCore/bdCore.h>

class bdNATTypeDiscoveryConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		/// This sets up the config to use all the default values..
		bdNATTypeDiscoveryConfig();

		/// Reset the config to use all the default settings.
		void reset();

		/// Ensure the current config settings are within the recommended limits.
		void sanityCheckConfig();

		/// The number of times to resend a packet.
		bdUInt m_NtdcsMaxResends;

		/// The length of time in seconds to wait for 
		/// a reply to a request packet.
		bdFloat32 m_NtdcsSendTimeout;


};

#endif // BD_NAT_TYPE_DISCOVERY_CONFIG_H

