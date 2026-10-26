// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdThread/bdSemaphore.h>
#include <bdPlatform/bdPlatformThread/bdPlatformSemaphore.h>

bdSemaphore::bdSemaphore(bdUInt initialCount, bdUInt maxCount)
{
	m_handle = bdPlatformSemaphore::createSemaphore(initialCount, maxCount);
}

bdSemaphore::~bdSemaphore()
{
}

void bdSemaphore::release()
{
	bdPlatformSemaphore::release(m_handle);
}

bdBool bdSemaphore::wait()
{
	return bdPlatformSemaphore::wait(m_handle);
}


void bdSemaphore::destroy()
{
	bdPlatformSemaphore::destroy(m_handle);
	delete this;
}


