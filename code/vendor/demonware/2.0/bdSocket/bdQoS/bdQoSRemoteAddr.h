// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_QOS_REMOTE_ADDR_H
#define BD_QOS_REMOTE_ADDR_H

#include <bdCore/bdCore.h>

class bdQoSRemoteAddr
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		bdQoSRemoteAddr()
		{
		}

		bdQoSRemoteAddr(const bdCommonAddrRef &addr,
						const bdSecurityID &id,
						const bdSecurityKey &key)
		: m_addr(addr),
		  m_id(id),
		  m_key(key)
		{
		}

		bdCommonAddrRef m_addr;
		bdSecurityID m_id;
		bdSecurityKey m_key;
};


#endif // BD_QOS_REMOTE_ADDR_H
