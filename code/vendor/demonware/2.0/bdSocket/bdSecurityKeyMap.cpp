// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-ADHOC PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdSecurityKeyMap.h>
#include <bdCore/bdSocket/bdSecurityInfo.h>
#include <bdSocket/bdSecurityKeyMapListener.h>

#define BD_LOG_LEVEL "bdCore/bdSocket/bdSecurityKeyMap"

bdSecurityKeyMap::bdSecurityKeyMap()
{
	m_listener = BD_NULL;
}

bdBool BD_CALL bdSecurityKeyMap::initKey(bdSecurityID &id, 
										 bdSecurityKey &key)
{
	bdTrulyRandom::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8 *>(id.ab), BD_SECURITY_ID_LENGTH);
	bdTrulyRandom::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8 *>(key.ab), BD_SECURITY_KEY_LENGTH);

	bdNChar8 tmp[BD_SECURITY_KEY_STRING_LENGTH];
	bdSecurityInfo::toString(id, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Created bdSecurityID: %s", tmp);
	bdSecurityInfo::toString(key, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Created bdSecurityKey: %s", tmp);

	return true;
}

bdBool bdSecurityKeyMap::registerKey(const bdSecurityID &id, const bdSecurityKey &key)
{
	const bdBool added = m_map.put(id, key);

	bdNChar8 tmp[BD_SECURITY_KEY_STRING_LENGTH];
	bdSecurityInfo::toString(id, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Putting bdSecurityID: %s", tmp);
	bdSecurityInfo::toString(key, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Putting bdSecurityKey: %s", tmp);
	
	if(!added)
	{
		BD_INFO(BD_LOG_LEVEL, "bdSecurityID already present.");
	}	

	return added;
}

bdBool bdSecurityKeyMap::unregisterKey(const bdSecurityID &id)
{
	if (m_listener && m_map.containsKey(id))
	{
		m_listener->onSecurityKeyRemove(id);
	}
	
	const bdBool removed = m_map.remove(id);

	bdNChar8 tmp[BD_SECURITY_KEY_STRING_LENGTH];
	bdSecurityInfo::toString(id, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Removing bdSecurityID: %s", tmp);

	if(!removed)
	{
		BD_INFO(BD_LOG_LEVEL, "bdSecurityID not present.");
	}

	return removed;
}

bdBool bdSecurityKeyMap::get(const bdSecurityID &id, bdSecurityKey &key)
{
	const bdBool found = m_map.get(id, key);

	bdNChar8 tmp[BD_SECURITY_KEY_STRING_LENGTH];
	bdSecurityInfo::toString(id, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	BD_INFO(BD_LOG_LEVEL, "Searching for bdSecurityID: %s", tmp);

	if(found)
	{
		bdSecurityInfo::toString(key, tmp, BD_SECURITY_KEY_STRING_LENGTH);
		BD_INFO(BD_LOG_LEVEL, "Found bdSecurityKey: %s", tmp);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "bdSecurityID not present");
	}

	return found;
}

bdBool bdSecurityKeyMap::contains(const bdSecurityID &id) const
{
	bdBool found = m_map.containsKey(id);


	bdNChar8 tmp[BD_SECURITY_KEY_STRING_LENGTH];
	bdSecurityInfo::toString(id, tmp, BD_SECURITY_KEY_STRING_LENGTH);
	
	if(found)
	{
		BD_INFO(BD_LOG_LEVEL, "Found bdSecurityID: %s", tmp);
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "bdSecurityID: %s not present.", tmp);
	}

	return found;
}

void bdSecurityKeyMap::clear()
{
	if (m_listener)
	{
		bdHashMap<bdSecurityID, bdSecurityKey>::Iterator it = m_map.getIterator();
	
		while (it)
		{
			m_listener->onSecurityKeyRemove(m_map.getKey(it));
			m_map.next(it);
		}
	}
	m_map.clear();
}

void bdSecurityKeyMap::registerListener(bdSecurityKeyMapListener* listener)
{
	BD_ASSERT(m_listener == BD_NULL, "Only one listener is allowed at once.");
	m_listener = listener;
}

void bdSecurityKeyMap::unregisterListener()
{
	m_listener = BD_NULL;
}
