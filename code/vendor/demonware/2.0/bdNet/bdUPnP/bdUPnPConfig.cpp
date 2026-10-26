// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdUPnP/bdUPnPConfig.h>

#define BD_DEFAULT_UPNP_DISCOVERY_TIMEOUT (3.0f)
#define BD_MIN_RECOMMENDED_UPNP_DISCOVERY_TIMEOUT (0.5f)
#define BD_MAX_RECOMMENDED_UPNP_DISCOVERY_TIMEOUT (BD_DEFAULT_UPNP_DISCOVERY_TIMEOUT*4)

#define BD_DEFAULT_UPNP_RESPONSE_TIMEOUT (10.0f)
#define BD_MIN_RECOMMENDED_UPNP_RESPONSE_TIMEOUT (1.0f)
#define BD_MAX_RECOMMENDED_UPNP_RESPONSE_TIMEOUT (BD_DEFAULT_UPNP_RESPONSE_TIMEOUT*4)

#define BD_DEFAULT_UPNP_CONNECT_TIMEOUT (5.0f)
#define BD_MIN_RECOMMENDED_UPNP_CONNECT_TIMEOUT (1.0f)
#define BD_MAX_RECOMMENDED_UPNP_CONNECT_TIMEOUT (BD_DEFAULT_UPNP_CONNECT_TIMEOUT*4)

#define BD_DEFAULT_UPNP_RUN_MODE (BD_UPNP_DO_PORT_MAPPING)

#define BD_DEFAULT_UPNP_DISCOVERY_NUM_RETRIES (1u)
#define BD_MAX_RECOMMENDED_UPNP_DISCOVERY_NUM_RETRIES (BD_DEFAULT_UPNP_DISCOVERY_NUM_RETRIES*4)

#define BD_LOG_LEVEL "UPnPConfig"

bdUPnPConfig::bdUPnPConfig()
: m_discoveryTimeout(BD_DEFAULT_UPNP_DISCOVERY_TIMEOUT),
  m_responseTimeout(BD_DEFAULT_UPNP_RESPONSE_TIMEOUT),
  m_connectTimeout(BD_DEFAULT_UPNP_CONNECT_TIMEOUT),
  m_runMode(BD_DEFAULT_UPNP_RUN_MODE),
  m_discoveryRetries(BD_DEFAULT_UPNP_DISCOVERY_NUM_RETRIES),
  m_disabled(false)
{
}

void bdUPnPConfig::reset()
{
	m_discoveryTimeout = BD_DEFAULT_UPNP_DISCOVERY_TIMEOUT;
	m_responseTimeout = BD_DEFAULT_UPNP_RESPONSE_TIMEOUT;
	m_connectTimeout = BD_DEFAULT_UPNP_CONNECT_TIMEOUT;
	m_runMode = BD_DEFAULT_UPNP_RUN_MODE;
	m_discoveryRetries = BD_DEFAULT_UPNP_DISCOVERY_NUM_RETRIES;
	m_gatewayAddr = bdInetAddr();
}

void bdUPnPConfig::sanityCheckConfig()
{
	// As a sanity check will warn if the supplied values are outside
	// the expected ballpark!
	// Note that this code only prints a warning, it does not prevent 
	// the settings from being used.
	if( (m_discoveryTimeout > BD_MAX_RECOMMENDED_UPNP_DISCOVERY_TIMEOUT) ||
		(m_discoveryTimeout < BD_MIN_RECOMMENDED_UPNP_DISCOVERY_TIMEOUT))
	{
		BD_WARN(BD_LOG_LEVEL, "UPnP discovery timeout set to %f seconds. This is outside the normal recommended range", m_discoveryTimeout);
	}

	if( (m_responseTimeout > BD_MAX_RECOMMENDED_UPNP_RESPONSE_TIMEOUT) ||
		(m_responseTimeout < BD_MIN_RECOMMENDED_UPNP_RESPONSE_TIMEOUT))
	{
		BD_WARN(BD_LOG_LEVEL, "UPnP response timeout set to %f seconds. This is outside the normal recommended range", m_responseTimeout);
	}

	if( (m_connectTimeout > BD_MAX_RECOMMENDED_UPNP_CONNECT_TIMEOUT) ||
		(m_connectTimeout < BD_MIN_RECOMMENDED_UPNP_CONNECT_TIMEOUT))
	{
		BD_WARN(BD_LOG_LEVEL, "UPnP response timeout set to %f seconds. This is outside the normal recommended range", m_connectTimeout);
	}

	if(	(m_runMode != BD_UPNP_DO_PORT_MAPPING) &&
		(m_runMode != BD_UPNP_EXTERNAL_IP_ONLY))
	{
		BD_WARN(BD_LOG_LEVEL, "UPnP run mode misconfigured. This will cause undefined behaviour");
	}

	if  (m_discoveryRetries > BD_MAX_RECOMMENDED_UPNP_DISCOVERY_NUM_RETRIES)
	{
		// Note that the minimum safe value for retries is 0 which is as small 
		// as an unsigned int can be.
		// We could add a test here to make sure retries was an unsigned int,
		// but a negative limit will not break anything so there is no real 
		// point
		BD_WARN(BD_LOG_LEVEL, "UPnP discovery retries set to %u . This is outside the normal recommended range", m_discoveryRetries);
	}
}


