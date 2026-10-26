// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the configuration information for the host name lookup.

#ifndef BD_GET_HOST_BY_NAME_CONFIG_H
#define BD_GET_HOST_BY_NAME_CONFIG_H

#include <bdCore/bdCore.h>

class bdGetHostByNameConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		/// This sets up the config to use all the default values..
		bdGetHostByNameConfig();

		/// Reset the config to use all the default settings.
		void reset();

		/// Check that config settings are within recommended ranges,
		/// and print a warning if they are not.
		void sanityCheckConfig();

		/// The timeout on a DNS lookup.
		bdFloat32 m_timeout;

};

#endif // BD_GET_HOST_BY_NAME_CONFIG_H

