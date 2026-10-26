// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdNAT/bdNATTypeDiscoveryConfig.h>

#define BD_DEFAULT_NTDCS_MAX_RESENDS	(5u)
#define BD_DEFAULT_NTDCS_SEND_TIMEOUT	(0.5f)

#define BD_MIN_RECOMMENDED_NTDCS_RESENDS (2u)
#define BD_MAX_RECOMMENDED_NTDCS_RESENDS (BD_DEFAULT_NTDCS_MAX_RESENDS*2)

#define BD_MIN_RECOMMENDED_SEND_TIMEOUT (0.05f)
#define BD_MAX_RECOMMENDED_SEND_TIMEOUT (BD_DEFAULT_NTDCS_SEND_TIMEOUT*4)

#define BD_LOG_LEVEL "NATTypeDiscoveryConfig"

bdNATTypeDiscoveryConfig::bdNATTypeDiscoveryConfig()
: m_NtdcsMaxResends(BD_DEFAULT_NTDCS_MAX_RESENDS),
  m_NtdcsSendTimeout(BD_DEFAULT_NTDCS_SEND_TIMEOUT)
{
}

void bdNATTypeDiscoveryConfig::reset()
{
	m_NtdcsMaxResends = BD_DEFAULT_NTDCS_MAX_RESENDS;
	m_NtdcsSendTimeout = BD_DEFAULT_NTDCS_SEND_TIMEOUT;
}

void bdNATTypeDiscoveryConfig::sanityCheckConfig()
{
	// As a sanity check will warn if the supplied values are outside
	// the expected ballpark!
	if( (m_NtdcsMaxResends > BD_MAX_RECOMMENDED_NTDCS_RESENDS) ||
		(m_NtdcsMaxResends < BD_MIN_RECOMMENDED_NTDCS_RESENDS))
	{
		BD_WARN(BD_LOG_LEVEL, "Max resends configured to %u. This is outside the normal recommended range", m_NtdcsMaxResends);
	}

	if( (m_NtdcsSendTimeout > BD_MAX_RECOMMENDED_SEND_TIMEOUT) ||
		(m_NtdcsSendTimeout < BD_MIN_RECOMMENDED_SEND_TIMEOUT ))
	{
		BD_WARN(BD_LOG_LEVEL, "Send timeout configured to %f. This is outside the normal recommended range", m_NtdcsSendTimeout);
	}
}
