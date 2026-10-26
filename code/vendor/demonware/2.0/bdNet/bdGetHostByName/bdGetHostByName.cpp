// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdGetHostByName/bdGetHostByName.h>

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <bdCore/bdThread/bdThread.h>

#define BD_LOG_LEVEL "gethostbyname"

bdGetHostByName::bdGetHostByName()
: m_status(BD_LOOKUP_UNINITIALIZED),
  m_thread(BD_NULL),
  m_numAddresses(0)
{
}

bdGetHostByName::~bdGetHostByName()
{
}

bdBool bdGetHostByName::start(const bdNChar8 *hostname, 
							  bdGetHostByNameConfig config )
{
	bdBool success = false;
	if(BD_LOOKUP_UNINITIALIZED == m_status)
	{
		m_thread = new bdThread(this);
		m_config = config;
		m_timer.reset();
		m_timer.start();
		config.sanityCheckConfig();
		m_status = BD_LOOKUP_PENDING;

		success = m_thread->start(hostname, bdStrlen(hostname)+1);
		if (success)
		{
			BD_INFO(BD_LOG_LEVEL , "Lookup started for %s." , hostname);
		}
		else
		{
			// TODO Research what if anything needs to be done with a thread
			// that fails to start.
			m_status = BD_LOOKUP_ERROR;
			BD_ERR(BD_LOG_LEVEL , "Failed to start host name lookup thread for %s." , hostname);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Cannot start a host name lookup unless class is in the uninitialized state.");
	}
	return success;
}

void bdGetHostByName::pump()
{
	if (m_status == BD_LOOKUP_PENDING)
	{
		if (m_timer.getElapsedTimeInSeconds() > m_config.m_timeout)
		{
			BD_ERR(BD_LOG_LEVEL , "Lookup has timed out after %f seconds, cancelling thread." , m_config.m_timeout);
			cancelLookup();
			m_status = BD_LOOKUP_TIMED_OUT;
		}
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL , "Pumping class while no active lookup. This is perfectly safe but not necessary.");
	}
}

bdGetHostByName::bdStatus  bdGetHostByName::getStatus() const
{
	return m_status;
}

bdUInt bdGetHostByName::getNumAddresses() const
{
	return m_numAddresses;
}

bdInAddr bdGetHostByName::getAddressAt(bdUInt index) const 
{
	bdInAddr result;
	if(index < m_numAddresses)
	{
		result = m_adresses[index];
	}
	else if(index < BD_MAX_NUM_ADDRESSES)
	{
		BD_WARN(BD_LOG_LEVEL , "No address at index %u , blank bdInAddr returned" ,index);
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Address at index %u is outside the array , blank bdInAddr returned" ,index);
	}
	return  result;
}

void bdGetHostByName::cancelLookup()
{
	if(BD_LOOKUP_PENDING == m_status)
	{
		bdPlatformSocket::abortGetHostByName(m_handle);
		m_status = BD_LOOKUP_CANCELLED;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "No pending lookup to cancel" );
	}
}


