// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdConnectionStoreConfig.h>


#define BD_DEFAULT_MAX_CONNECTION_STORE_FLUSH_ITERATIONS (5u)
#define BD_DEFAULT_MAX_CONNECTION_STORE_RECEIVE_ITERATIONS (100u)
#define BD_DEFAULT_MAX_CONNECTION_STORE_SHUTDOWN_DURATION (5.0f)


bdConnectionStoreConfig::bdConnectionStoreConfig()
: m_maxReceiveIterations(BD_DEFAULT_MAX_CONNECTION_STORE_RECEIVE_ITERATIONS),
  m_maxShutdownDuration(BD_DEFAULT_MAX_CONNECTION_STORE_SHUTDOWN_DURATION),
  m_maxFlushIterations(BD_DEFAULT_MAX_CONNECTION_STORE_FLUSH_ITERATIONS)
{
}

void bdConnectionStoreConfig::reset()
{
	m_maxReceiveIterations = BD_DEFAULT_MAX_CONNECTION_STORE_RECEIVE_ITERATIONS;
	m_maxShutdownDuration = BD_DEFAULT_MAX_CONNECTION_STORE_SHUTDOWN_DURATION;
	m_maxFlushIterations = BD_DEFAULT_MAX_CONNECTION_STORE_FLUSH_ITERATIONS;
}



