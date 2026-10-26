// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the configuration information for the socket router.

#ifndef BD_IP_DISCOVERY_CONFIG_H
#define BD_IP_DISCOVERY_CONFIG_H

#include <bdCore/bdCore.h>

class bdIPDiscoveryConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		/// This sets up the config to use all the default values..
		bdIPDiscoveryConfig();

		/// Reset the config to use all the default settings.
		void reset();

		bdUByte8 m_retries;
		
		bdFloat32 m_requestTimeout;

		void sanityCheckConfig();


};

#endif // BD_IP_DISCOVERY_CONFIG_H

