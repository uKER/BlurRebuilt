
#include "stdafx.h"
#include "netVoice_State.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netVoice
{
	State::LocalUser::LocalUser()
	{
		m_active = false;
		m_headset = false;
	}
	
	netUserId State::LocalUser::userId() const
	{
		return netPlatform::Core::instance()->getUserId(m_profile);
	}
	
	State::RemoteUser::RemoteUser()
	{
		m_active = false;
		m_headset = false;
		memset(m_muted, 0, sizeof(m_muted));
	}

	netUserId State::RemoteUser::userId() const
	{
		return netPlatform::Core::instance()->getUserId(m_user);
	}
	
	State::State()
	{
		m_socketPort = -1;
	}
	
	State::~State()
	{
	}

	bool State::clear()
	{
		m_socketPort = -1;

		for(int i = 0; i < MAX_LOCAL_USERS; i ++)
		{
			m_localUsers[i] = LocalUser();
		}

		for(int i = 0; i < MAX_REMOTE_USERS; i ++)
		{
			m_remoteUsers[i] = RemoteUser();
		}
		return true;
	}

	bool State::addLocalUser(const State::LocalUser& user)
	{
		if(!user.m_active)
			return false;

		if(user.userId() == 0)
			return false;

		
		State::LocalUser* localUser = findLocalUser(user.userId());
		if(localUser != NULL)
		{
			LOGWARN(netDebug::g_Log, "<netVoice::State> local user(%llu) already present.", user.userId());
			*localUser = user;
			return true;
		}

		for(int i = 0; i < MAX_LOCAL_USERS; i ++)
		{
			if(!m_localUsers[i].m_active)
			{
				m_localUsers[i] = user;
				return true;
			}
		}
		LOGWARN(netDebug::g_Log, "<netVoice::State> no free slot for local user(%llu).", user.userId());
		return false;
	}

	bool State::removeLocalUser(const State::LocalUser& user)
	{
		State::LocalUser* localUser = findLocalUser(user.userId());
		if(localUser == NULL) return false;
		*localUser = State::LocalUser();
		return true;
	}

	bool State::removeRemoteUser(const State::RemoteUser& user)
	{
		State::RemoteUser* remoteUser = findRemoteUser(user.userId());
		if(remoteUser == NULL) return false;
		*remoteUser = State::RemoteUser();
		return true;
	}
	
	bool State::addRemoteUser(const State::RemoteUser& user)
	{
		if(!user.m_active)
			return false;

		if(user.userId() == 0)
			return false;
		
		State::RemoteUser* remoteUser = findRemoteUser(user.userId());
		if(remoteUser != NULL)
		{
			LOGWARN(netDebug::g_Log, "<netVoice::State> remote user(%llu) already present.", user.userId());
			*remoteUser = user;
			return true;
		}

		for(int i = 0; i < MAX_REMOTE_USERS; i ++)
		{
			if(!m_remoteUsers[i].m_active)
			{
				m_remoteUsers[i] = user;
				return true;
			}
		}
		LOGWARN(netDebug::g_Log, "<netVoice::State> no free slot for remote user(%llu).", user.userId());
		return false;
	}
	
	State::LocalUser* State::findLocalUser(const netUserId& userId)
	{
		if(userId == 0)
			return NULL;

		for(int i = 0; i < MAX_LOCAL_USERS; i ++)
		{
			if(!m_localUsers[i].m_active)
				continue;
			
			if(m_localUsers[i].userId() == userId)
				return &(m_localUsers[i]);
		}
		return NULL;
	}

	const State::LocalUser* State::findLocalUser(const netUserId& userId) const
	{
		if(userId == 0)
			return NULL;

		for(int i = 0; i < MAX_LOCAL_USERS; i ++)
		{
			if(!m_localUsers[i].m_active)
				continue;
			
			if(m_localUsers[i].userId() == userId)
				return &(m_localUsers[i]);
		}
		return NULL;
	}

	State::RemoteUser* State::findRemoteUser(const netUserId& userId)
	{
		if(userId == 0)
			return NULL;

		for(int i = 0; i < MAX_REMOTE_USERS; i ++)
		{
			if(!m_remoteUsers[i].m_active)
				continue;

			if(m_remoteUsers[i].userId() == userId)
			{
				return &(m_remoteUsers[i]);
			}
		}
		return NULL;
	}

	const State::RemoteUser* State::findRemoteUser(const netUserId& userId) const
	{
		if(userId == 0)
			return NULL;

		for(int i = 0; i < MAX_REMOTE_USERS; i ++)
		{
			if(!m_remoteUsers[i].m_active)
				continue;

			if(m_remoteUsers[i].userId() == userId)
			{
				return &(m_remoteUsers[i]);
			}
		}
		return NULL;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

