// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdIPDiscoveryConfig.h>

// The number of retransmissions before we give up
#define BD_DEFAULT_IP_DISC_NUM_RETRANSMISSIONS (5u)
// Time in seconds before we timeout
#define BD_DEFAULT_IP_DISC_REQUEST_TIMEOUT	(.5f)

#define BD_MIN_RECOMMENDED_IP_DISC_NUM_RETRANSMISSIONS (2u)
#define BD_MAX_RECOMMENDED_IP_DISC_NUM_RETRANSMISSIONS (BD_DEFAULT_IP_DISC_NUM_RETRANSMISSIONS*2)

#define BD_MIN_RECOMMENDED_IP_DISC_REQUEST_TIMEOUT (0.05f)
#define BD_MAX_RECOMMENDED_IP_DISC_REQUEST_TIMEOUT (BD_DEFAULT_IP_DISC_REQUEST_TIMEOUT*4)

#define BD_LOG_LEVEL "IPDiscoveryConfig"

bdIPDiscoveryConfig::bdIPDiscoveryConfig()
: m_retries(BD_DEFAULT_IP_DISC_NUM_RETRANSMISSIONS),
  m_requestTimeout(BD_DEFAULT_IP_DISC_REQUEST_TIMEOUT)
{
}

void bdIPDiscoveryConfig::reset()
{
	m_retries = BD_DEFAULT_IP_DISC_NUM_RETRANSMISSIONS;
	m_requestTimeout = BD_DEFAULT_IP_DISC_REQUEST_TIMEOUT;
}

void bdIPDiscoveryConfig::sanityCheckConfig()
{
	// As a sanity check will warn if the supplied values are outside
	// the expected ballpark!
	if( (m_retries > BD_MAX_RECOMMENDED_IP_DISC_NUM_RETRANSMISSIONS) ||
		(m_retries < BD_MIN_RECOMMENDED_IP_DISC_NUM_RETRANSMISSIONS))
	{
		BD_WARN(BD_LOG_LEVEL, "Max resends configured to %u. This is outside the normal recommended range", m_retries);
	}

	if( (m_requestTimeout > BD_MAX_RECOMMENDED_IP_DISC_REQUEST_TIMEOUT) ||
		(m_requestTimeout < BD_MIN_RECOMMENDED_IP_DISC_REQUEST_TIMEOUT ))
	{
		BD_WARN(BD_LOG_LEVEL, "Send timeout configured to %f. This is outside the normal recommended range", m_requestTimeout);
	}
}
