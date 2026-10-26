#include "stdafx.h"
#include "netFriends/netFriends.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netFriends::netFriends()
{
	MEMORY_TRACKER_PUSH("network.multiplayer");
	m_enumerator = new netFriendEnumerator;
	m_enumerator->Initialise();
	MEMORY_TRACKER_POP("network.multiplayer");
	
	m_state		 = STATE_IDLE;
	m_status	 = STATUS_NOT_SET;
	clear();
}

netFriends::~netFriends()
{
	clear();
}

void netFriends::clear()
{
	m_enumerator->StopEnumeration();
	m_friendsCount	= 0;
	m_state			= STATE_IDLE;
	m_status		= STATUS_NOT_SET;
}

bool netFriends::enumerateFriends(const SysProfile::FullProfile& profile)
{
	if(enumerating())
	{
		m_enumerator->StopEnumeration();
	}

	if(!m_enumerator->StartEnumeration(profile, false))
	{
		assert(!"<netfriends> faioled to start enumeration.");
		m_state	 = STATE_IDLE;
		m_status = STATUS_ERROR;
		return false;
	}

	m_state	 = STATE_ENUMERATING;
	m_status = STATUS_NOT_SET;
	return true;
}

bool netFriends::enumerating() const
{
	return (m_state == STATE_ENUMERATING);
}

bool netFriends::started() const
{
	return (m_state == STATE_IDLE) && (m_status != STATUS_NOT_SET);
}

bool netFriends::success() const
{
	return (m_state == STATE_IDLE) && (m_status == STATUS_OK);
}

bool netFriends::failed() const
{
	return (m_state == STATE_IDLE) && (m_status != STATUS_OK);
}

void netFriends::update()
{
	switch(m_state)
	{
	default: 
		{
			break;
		}
	case STATE_IDLE:
		{
			break;
		}
	case STATE_ENUMERATING:
		{
			// done
			bool error;
			if(m_enumerator->ContinueEnumeration(error))
			{
				if(error)
				{
					m_state	 = STATE_IDLE;
					m_status = STATUS_ERROR;
				}
				else
				{
					m_state	 = STATE_IDLE;
					m_status = STATUS_OK;
					m_friendsCount = m_enumerator->ExtractFriends(m_friends, NET_MAX_FRIENDS, true);
				}
			}
			break;
		}
	}
}

netFriend* netFriends::findUser(const netUserId& id)
{
	for(int i = 0; i < m_friendsCount; i ++)
	{
		netUserId userId = netPlatform::Core::instance()->getUserId(m_friends[i].m_Profile);

		if(userId == id)
			return &(m_friends[i]);
	}
	return NULL;
}

netFriend* netFriends::getFriend(int i)
{
	if(i < 0 || i >= m_friendsCount)
		return NULL;

	return &m_friends[i];
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

