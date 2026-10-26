// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdGetHostByName/bdGetHostByNameConfig.h>

// Time in seconds before we timeout
#define BD_DEFAULT_HOST_NAME_LOOKUP_TIMEOUT (10.0f)

#define BD_MIN_RECOMMENDED_HOST_NAME_LOOKUP_TIMEOUT (1.0f)
#define BD_MAX_RECOMMENDED_HOST_NAME_LOOKUP_TIMEOUT (BD_DEFAULT_HOST_NAME_LOOKUP_TIMEOUT*4)

#define BD_LOG_LEVEL "getHostByNameConfig"

bdGetHostByNameConfig::bdGetHostByNameConfig()
: m_timeout(BD_DEFAULT_HOST_NAME_LOOKUP_TIMEOUT)
{
}

void bdGetHostByNameConfig::reset()
{
	m_timeout = BD_DEFAULT_HOST_NAME_LOOKUP_TIMEOUT;
}

void bdGetHostByNameConfig::sanityCheckConfig()
{
	// As a sanity check will warn if the supplied values are outside
	// the expected ballpark!
	if( (m_timeout > BD_MAX_RECOMMENDED_HOST_NAME_LOOKUP_TIMEOUT) ||
		(m_timeout < BD_MIN_RECOMMENDED_HOST_NAME_LOOKUP_TIMEOUT))
	{
		BD_WARN(BD_LOG_LEVEL, "Lookup timeout set to %f seconds. This is outside the normal recommended range", m_timeout);
	}
}
