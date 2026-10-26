// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the configuration information for the UPnP module.

#ifndef BD_UPNP_CONFIG_H
#define BD_UPNP_CONFIG_H

#include <bdCore/bdCore.h>

class bdUPnPConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// There are currently 2 ways that UPnP can operate.
		// 1) The first device on the network is used, and other devices
		//    are completely ignored.
		// 2) All devices are detected, and the developer must implement
		//    a callback to choose which device is the correct one.
		// Note: This second method has not yet been fully implemented.
		enum bdUPnPRunMode
		{
			BD_UPNP_DO_PORT_MAPPING,
			BD_UPNP_EXTERNAL_IP_ONLY
		};

		/// Basic Constructor.
		/// This sets up the config to use all the default values..
		bdUPnPConfig();

		/// Reset the config to use all the default settings.
		void reset();

		/// Check that config settings are within recommended ranges,
		/// and print a warning if they are not.
		void sanityCheckConfig();

		/// Timeout for discovery of UPnP NATs in seconds.
		/// The UPnP code will always spend this amount of time waiting for
		/// UPnP discovery replies.
		bdFloat32 m_discoveryTimeout;

		/// Maximum amount of time to wait for a router to respond in seconds.
		/// During each phase of UPnP traversal, if a response is not received
		/// after this length of time the code will assume communication to the
		/// device has failed.
		/// \note This time out does not apply during device discovery.
		bdFloat32 m_responseTimeout;

		bdFloat32 m_connectTimeout;

		/// Indicates how the UPnP code should deal with multiple devices on
		/// the LAN, the default is to simply use the first one.
		bdUPnPRunMode m_runMode;

		/// Indicates how many times the UPnP discovery broadcast should be retried.
		bdUInt m_discoveryRetries;

		/// Flag to prevent UPnP code from running.
		/// This is set to false by default.
		bdBool m_disabled;

		// The address of our gateway. We will attempt
		// to use a device that matches our gateway.
		bdInetAddr m_gatewayAddr;
};

#endif // BD_UPNP_CONFIG_H

