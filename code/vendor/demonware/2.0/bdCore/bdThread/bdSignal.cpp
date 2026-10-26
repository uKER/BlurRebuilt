// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdThread/bdSignal.h>
#include <bdPlatform/bdPlatformThread/bdPlatformSignal.h>

bdSignal::bdSignal()
: m_handle(bdPlatformSignal::create())
{
}

bdSignal::~bdSignal()
{
	bdPlatformSignal::destroy(m_handle);
}

void bdSignal::wait()
{
	bdPlatformSignal::wait(m_handle);
}

void bdSignal::signal()
{

	bdPlatformSignal::signal(m_handle);
}



