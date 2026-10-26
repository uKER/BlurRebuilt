// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSecurityKeyMap.h>

#include <bdCore/bdUtilities/bdTrulyRandom.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdCore/bdContainers/bdByteBuffer.h>

#define BD_LOG_LEVEL "bdCore/bdSocket/bdSecurityKeyMap"

bdSecurityKeyMap::bdSecurityKeyMap()
{
}

bdBool bdSecurityKeyMap::initKey(bdSecurityID &id, bdSecurityKey &key)
{
	return XNetCreateKey(&id, &key) == 0;
}

bdBool bdSecurityKeyMap::registerKey(const bdSecurityID &id, const bdSecurityKey &key)
{
	return XNetRegisterKey(&id, &key) == 0;

}

bdBool bdSecurityKeyMap::unregisterKey(const bdSecurityID &id)
{
	return XNetUnregisterKey(&id) == 0;
}

