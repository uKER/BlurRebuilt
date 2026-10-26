// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdGetHostByName/bdGetHostByName.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>
#include <bdCore/bdThread/bdThread.h>

#define BD_LOG_LEVEL "gethostbyname"

bdUInt bdGetHostByName::run(void* args)
{
	// Todo investigate appropriate result values..
	//   for this function...... 0 seems arbitrary..
	const bdUInt result = 0;
	if(BD_LOOKUP_PENDING == m_status)
	{
		m_numAddresses = bdPlatformSocket::getHostByName((bdNChar8*)args, m_adresses, BD_MAX_NUM_ADDRESSES, m_handle);
		if (m_numAddresses > 0)
		{
			m_status = BD_LOOKUP_SUCCEEDED;
		}
		else
		{
			m_status = BD_LOOKUP_FAILED;
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Host name lookup thread cannot be started unless a request is pending.");
	}
	return result;
}

void bdGetHostByName::quit()
{
	if(BD_LOOKUP_UNINITIALIZED != m_status)
	{
		if(BD_LOOKUP_PENDING == m_status)
		{
			// If thread is still running, we need to abort it.
			cancelLookup();
		}
		
		if(m_thread != BD_NULL)
		{
			m_thread->join();
			m_thread->cleanup();
			m_thread = BD_NULL;
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL , "Thread pointer already null during quit, this is unexpected.");
		}
		m_status = BD_LOOKUP_UNINITIALIZED;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "quit has no effect on class in the uninitialized state.");
	}
}

