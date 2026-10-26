// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSocketRouterConfig.h>

#define BD_LOG_LEVEL  "socketRouterConfig"

#ifdef BD_DEBUG
#define BD_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET (false)
#define BD_RECOMMENDED_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET (false)
#else
#define BD_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET (true)
#define BD_RECOMMENDED_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET (true)
#endif //


/// Timeouts for the socket-layer association
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
// On XBox, underlying security associations timeout at around 2 minutes.
#define	BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (120.0f)
#define BD_MAX_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (120.0f)
#define BD_MIN_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (120.0f)

#elif defined BD_PLATFORM_PSP_ADHOC
// On PSP Adhoc, underlying encrypted transport is not connection-oriented.
#define BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT		(60.0f * 60.0f * 24.0f)
#define BD_MAX_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (60.0f * 60.0f * 24.0f)
#define BD_MIN_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (60.0f * 60.0f * 24.0f)
#else // !XBOX && !PSP_ADHOC
// On PC, set timeout to 30 seconds in release. To aid debugging, set to 30
// minutes in debug.
#if defined BD_RELEASE
// The association timeout is 30 seconds
#define BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT	(30.f)
#define BD_MAX_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (50.0f)
#define BD_MIN_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (10.0f)
#else
// The association timeout is 30 minutes
#define BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT	(30.f * 60.0f)
#define BD_MAX_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (30.0f * 60.0f)
#define BD_MIN_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT (10.0f )
#endif // RELEASE
#endif // !XBOX && !PSP_ADHOC

// How many connection resets should be ignored before failing..
#define BD_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE	(10u)
#define BD_MAX_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE	(100u)
#define BD_MIN_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE	(1u)


bdSocketRouterConfig::bdSocketRouterConfig()
: m_ignoreConnectionReset(BD_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET),
  m_DTLSAssociationReceiveTimeout(BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT),
  m_maxConnectionResets(BD_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE)
{
}

void bdSocketRouterConfig::reset()
{
	m_ignoreConnectionReset = BD_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET;
	m_DTLSAssociationReceiveTimeout = BD_DEFAULT_DTLS_ASSOCIATION_TIMEOUT;
	m_maxConnectionResets = BD_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE;

}

void bdSocketRouterConfig::sanityCheckConfig()
{
	// As a sanity check will warn if the supplied values are outside
	// the expected ballpark!
	if(m_ignoreConnectionReset != BD_RECOMMENDED_SOCKET_ROUTER_DEFAULT_IGNORE_CONNECTION_RESET)
	{
		BD_WARN(BD_LOG_LEVEL, "Ignore connection reset differs from recommended setting of %s" , ( m_ignoreConnectionReset? "False":"True" ) );
	}

	if( (m_DTLSAssociationReceiveTimeout > BD_MAX_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT) ||
		(m_DTLSAssociationReceiveTimeout < BD_MIN_RECOMMENDED_DEFAULT_DTLS_ASSOCIATION_TIMEOUT ))
	{
		BD_WARN(BD_LOG_LEVEL, "Receive timeout configured to %f. This is outside the normal recommended range", m_DTLSAssociationReceiveTimeout);
	}

	if( (m_maxConnectionResets > BD_MAX_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE) ||
		(m_maxConnectionResets < BD_MIN_DEFAULT_NUM_RESET_CONNECTIONS_TO_IGNORE ))
	{
		BD_WARN(BD_LOG_LEVEL, "Max connection resets to ignore configured to %u. This is outside the normal recommended range", m_maxConnectionResets);
	}


}
