// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the configuration information for the socket router.

#ifndef BD_SOCKET_ROUTER_CONFIG_H
#define BD_SOCKET_ROUTER_CONFIG_H

#include <bdCore/bdCore.h>

/// Configuration information for the bdSocketRouter class.
/// These configuration settings do not normally needed to be altered.
/// When making alterations it is wise to remain with the recommended ranges.
/// sanityCheckConfig() will print a warning when settings lie outside this 
/// range.
class bdSocketRouterConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		/// This sets up the config to use all the default values..
		bdSocketRouterConfig();

		/// Reset the config to use all the default settings.
		void reset();

		/// Ensure the current config settings are within the recommended limits.
		void sanityCheckConfig();

		/// Indicates whether the bdSocketRouter should pay attention to 
		/// connection reset packets. This is useful in Debug mode as timeouts
		/// can be very high, but potentially dangerous in release quality code as 
		/// it can be used to disrupt a game
		bdBool m_ignoreConnectionReset;

		/// The length of time before a DTLS will be considered to have timed out.
		/// This setting will only have effect on platforms that use the DTLS.
		bdFloat32 m_DTLSAssociationReceiveTimeout;

		/// To prevent the socket router from looping forever while ignoring 
		/// connection resets, the number to ignore in one iteration is limited.
		bdUInt m_maxConnectionResets;

};

#endif // BD_SOCKET_ROUTER_CONFIG_H

