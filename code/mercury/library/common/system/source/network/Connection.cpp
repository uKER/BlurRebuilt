#include "SystemPch.h"

#include <core/threads.h>
#include <system/network/address.h>
#include <system/network/socket.h>
#include <system/network/connection.h>
#include <system/network/service.h>
#include <system/time/time.h>
#include <core/utility/coredebug.h>

namespace Network
{

/// Construct a connection class from an existing, bound TCP socket
Connection::Connection(Socket &s) :
	m_bDetectPending(true),
	m_bDetectOutput(false),
	m_iIdleTimeout(0),
	Socket(s.GetSocket())
{
//	m_iIdleTimeout = Time::GetCurrentTime();
	m_pMutex = new MT::Mutex();
#if defined( COMPILER_VC )
	m_Event = CreateEvent(0, FALSE, FALSE, 0);
#endif
}

Connection::Connection() :
	Socket(0,0)
{
	assert(0); // should never happen. ever.
}

/// Destructor
Connection::~Connection()
{
	delete m_pMutex;
	Close();
#if defined( COMPILER_VC )
	CloseHandle( m_Event );
#endif
}

/// Determine whether 'Pending' should be called
void Connection::SetDetectPending(bool bOn)
{
	m_bDetectPending = bOn;
}

/// Determine whether 'Output' should be called
void Connection::SetDetectOutput(bool bOn)
{
	m_bDetectOutput = bOn;
}

/// Set the idle timeout in ms (0 = no not call 'Idle')
void Connection::SetIdleTimeout(U32 i)
{
	m_iIdleTimeout = i;
}

}
