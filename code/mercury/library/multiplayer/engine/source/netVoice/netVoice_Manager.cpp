
#include "stdafx.h"
#include "netVoice_Manager.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform.h"
#include "netPlatform/netPlatform_Core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netVoice
{
	Manager::Manager(netTransport::SocketRef socket)
	: m_socket(socket)
	{
		netPlatform::Platform::InitialiseVoice();
	}

	Manager::~Manager()
	{
	}

	bool Manager::receive(netTransport::Message* msg)
	{
		switch(msg->messageId())
		{
		default:
			{
				return false;
			}
		case netTransport::MESSAGE_ID_VOICE_FRAGMENTS:
			{
				netTransport::MsgVoiceFragments* voiceFragment = (netTransport::MsgVoiceFragments*)msg;

				RemoteTalkerRef remoteTalker = findRemoteTalker(voiceFragment->m_userId);
				if(remoteTalker.isNull())
					return true;

				return remoteTalker->receive(voiceFragment);
			}
		}
	}

	bool Manager::headset(const netUserId userId)
	{
		const State::LocalUser* local = m_state.findLocalUser(userId);

		if(local != NULL)
			return local->m_headset;
		
		const State::RemoteUser* remote = m_state.findRemoteUser(userId);

		if(remote != NULL)
			return remote->m_headset;
		
		return false;
	}
	
	bool Manager::talking(const netUserId userId)
	{
		LocalTalkerRef localTalker = findLocalTalker(userId);
		if(localTalker.notNull()) return localTalker->talking();
	
		RemoteTalkerRef remoteTalker = findRemoteTalker(userId);
		if(remoteTalker.notNull()) return remoteTalker->talking();

		return false;
	}

	bool Manager::update(const State& state)
	{
		// update communicator mute system (and others apparently).
		Communicator::C_Core::Instance().Instance().MuteUpdate();

		syncLocalUsers(m_state, state);
		syncRemoteUsers(m_state, state);
		syncMute(m_state);
		m_state.m_socketPort = state.m_socketPort;
		transmit();
		decode();
		return true;
	}

	void Manager::decode()
	{
		// iterate through our current microphones, and mute channels
		bdLinkedList<RemoteTalkerRef>::Position pos = m_remoteTalkers.getHeadPosition();
		while(pos)
		{
			RemoteTalkerRef remoteTalker = m_remoteTalkers.forward(pos);
			
			remoteTalker->decode();
		}
	}

	void Manager::transmit()
	{
		// iterate through our current microphones, and mute channels
		bdLinkedList<LocalTalkerRef>::Position pos1 = m_localTalkers.getHeadPosition();
		while(pos1)
		{
			LocalTalkerRef localTalker = m_localTalkers.forward(pos1);
		
			char data[256];
			int len = localTalker->poll(data, sizeof(data));
			if(len <= 0) continue;

			// remove all listener buffers from list
			bdLinkedList<RemoteListenerRef>::Position pos2 = m_remoteListeners.getHeadPosition();
			while(pos2)
			{
				RemoteListenerRef remoteListener = m_remoteListeners.forward(pos2);
				if(remoteListener->m_talkerId != localTalker->m_userId)
					continue;

				remoteListener->cache(data, len);
				remoteListener->transmit(m_socket, m_state.m_socketPort);
			}
		}
	}

	int Manager::remoteTalkerCount(const netTransport::ConnGuid& connGuid)
	{
		int count = 0;

		// add listening buffers to list.
		bdLinkedList<RemoteTalkerRef>::Position pos = m_remoteTalkers.getHeadPosition();
		while(pos)
		{
			// listenting to that guy
			RemoteTalkerRef remote = m_remoteTalkers.forward(pos);
		
			if(remote->m_connGuid == connGuid)
				count++;
		}
		return count;
	}

	LocalTalkerRef Manager::findLocalTalker(const netUserId userId)
	{
		// iterate through our current microphones, and mute channels
		bdLinkedList<LocalTalkerRef>::Position pos = m_localTalkers.getHeadPosition();
		while(pos)
		{
			LocalTalkerRef localTalker = m_localTalkers.forward(pos);
			if(localTalker->m_userId == userId)
				return localTalker;
		}
		return NULL;
	}

	RemoteTalkerRef Manager::findRemoteTalker(const netUserId userId)
	{
		// add listening buffers to list.
		bdLinkedList<RemoteTalkerRef>::Position pos = m_remoteTalkers.getHeadPosition();
		while(pos)
		{
			// listenting to that guy
			RemoteTalkerRef remote = m_remoteTalkers.forward(pos);
			if(remote->m_userId == userId)
				return remote;
		}
		return NULL;
	}
	
	RemoteListenerRef Manager::findRemoteListener(const netTransport::ConnGuid& connGuid, const netUserId talkerId)
	{
		// remove all listener buffers from list
		bdLinkedList<RemoteListenerRef>::Position pos = m_remoteListeners.getHeadPosition();
		while(pos)
		{
			RemoteListenerRef listener = m_remoteListeners.forward(pos);
			if(listener->m_connGuid == connGuid && listener->m_talkerId == talkerId)
				return listener;
		}
		return NULL;
	}

	void Manager::syncMute(const State& state)
	{
		for(int i = 0; i < State::MAX_REMOTE_USERS; i ++)
		{
			// get mute status for remote user
			const State::RemoteUser* remoteUser = &(state.m_remoteUsers[i]);
			if(!remoteUser->m_active) continue;

			netUserId remoteUserId = netPlatform::Core::instance()->getUserId(remoteUser->m_user);
				
			// iterate through our current microphones, and mute channels
			bdLinkedList<LocalTalkerRef>::Position pos = m_localTalkers.getHeadPosition();
			while(pos)
			{
				LocalTalkerRef localTalker = m_localTalkers.forward(pos);

				// local player index
				int index = localTalker->m_profile.m_UserIndex;
				if(index < 0 || index >= State::MAX_LOCAL_USERS)
					continue;

				// mute status requested
				bool muted = remoteUser->m_muted[index];
				
				// mute outgoing voice
				RemoteListenerRef listener = findRemoteListener(remoteUser->m_connGuid, localTalker->m_userId);
				if(listener.notNull())
					listener->mute(muted);

				// mute incoming voice
				RemoteTalkerRef remoteTalker = findRemoteTalker(remoteUserId);
				if(remoteTalker.notNull())
					remoteTalker->headset(index, muted);
			}
		}
	}
	
	void Manager::syncLocalUsers(State& current, const State& update)
	{
		for(int i = 0; i < State::MAX_LOCAL_USERS; i ++)
		{
			State::LocalUser* old = &(current.m_localUsers[i]);
			if(!old->m_active) continue;
			
			const State::LocalUser* now = update.findLocalUser(old->userId());
			
			// local talker removed from list,
			// or user changed.
			if(	(now == NULL) || 
				(current.m_socketPort != update.m_socketPort) ||
				(old->m_profile.m_UserIndex != now->m_profile.m_UserIndex))
			{
				removeLocalUser(*old);
			}
		}

		for(int i = 0; i < State::MAX_LOCAL_USERS; i ++)
		{
			const State::LocalUser* now = &(update.m_localUsers[i]);
			if(!now->m_active) continue;
			
			State::LocalUser* old = current.findLocalUser(now->userId());
						
			// local talker added to list.
			if(old == NULL)
			{
				addLocalUser(*now);
			}
			// local user updated.
			else
			{
				*old = *now;
			}
		}
	}
	
	void Manager::syncRemoteUsers(State& current, const State& update)
	{
		for(int i = 0; i < State::MAX_REMOTE_USERS; i ++)
		{
			State::RemoteUser* old = &(current.m_remoteUsers[i]);
			if(!old->m_active) continue;
			
			const State::RemoteUser* now = update.findRemoteUser(old->userId());
			
			// remote talker removed from list, or remote talker 
			// changed.
			if(	(now == NULL) || 
				(current.m_socketPort != update.m_socketPort) ||
				(old->m_connGuid != now->m_connGuid))
			{
				removeRemoteUser(*old);
			}
		}

		for(int i = 0; i < State::MAX_REMOTE_USERS; i ++)
		{
			const State::RemoteUser* now = &(update.m_remoteUsers[i]);
			if(!now->m_active) continue;
			
			State::RemoteUser* old = current.findRemoteUser(now->userId());
						
			// local talker added to list.
			if(old == NULL)
			{
				addRemoteUser(*now);
			}
			// local user updated.
			else
			{
				*old = *now;
			}
		}
	}

	bool Manager::removeLocalUser(const State::LocalUser& user)
	{
		netUserId userId = user.userId();
		
		// remove local talker from list
		bdLinkedList<LocalTalkerRef>::Position pos1 = m_localTalkers.getHeadPosition();
		while(pos1)
		{
			LocalTalkerRef talker = m_localTalkers.getAt(pos1);
		
			// found listening buffer referenced to that user. remove.
			if(talker->m_userId == userId)
			{
				m_localTalkers.removeAt(pos1);
			}
			else
			{
				m_localTalkers.forward(pos1);
			}
		}		
	
		// remove all listener buffers from list
		bdLinkedList<RemoteListenerRef>::Position pos2 = m_remoteListeners.getHeadPosition();
		while(pos2)
		{
			RemoteListenerRef listener = m_remoteListeners.getAt(pos2);
		
			// found listening buffer referenced to that user. remove.
			if(listener->m_talkerId == userId)
			{
				m_remoteListeners.removeAt(pos2);
			}
			else
			{
				m_remoteListeners.forward(pos2);
			}
		}	

		m_state.removeLocalUser(user);

		return true;
	}

	bool Manager::removeRemoteUser(const State::RemoteUser& user)
	{
		netUserId userId = user.userId();
		
		// remove local talker from list
		bdLinkedList<RemoteTalkerRef>::Position pos1 = m_remoteTalkers.getHeadPosition();
		while(pos1)
		{
			RemoteTalkerRef talker = m_remoteTalkers.getAt(pos1);

			// found listening buffer referenced to that user. remove.
			if(talker->m_userId == userId)
			{
				m_remoteTalkers.removeAt(pos1);
			}
			else
			{
				m_remoteTalkers.forward(pos1);
			}
		}

		// remove listeners, since we have no more players attached.
		int count = remoteTalkerCount(user.m_connGuid);

		if(count == 0)
		{
			// remove all listener buffers from list
			bdLinkedList<RemoteListenerRef>::Position pos2 = m_remoteListeners.getHeadPosition();
			while(pos2)
			{
				RemoteListenerRef listener = m_remoteListeners.getAt(pos2);
			
				// found listening buffer referenced to that user. remove.
				if(listener->m_connGuid == user.m_connGuid)
				{
					m_remoteListeners.removeAt(pos2);
				}
				else
				{
					m_remoteListeners.forward(pos2);
				}
			}
		}
		
		m_state.removeRemoteUser(user);
		
		return true;
	}
	
	bool Manager::addLocalUser(const State::LocalUser& user)
	{
		m_state.addLocalUser(user);

		netUserId userId = user.userId();
		
		// add remote talker to list
		if(findLocalTalker(userId).notNull())
		{
			LOGWARN(netDebug::g_Log, "<netVoice::Manager> RemoteTalker(%llu) already present.", userId);
		}
		else
		{
			m_localTalkers.addTail(new LocalTalker(user.m_profile));
		}

		// add listening buffers to list.
		bdLinkedList<RemoteTalkerRef>::Position pos = m_remoteTalkers.getHeadPosition();
		while(pos)
		{
			// listenting to that guy
			RemoteTalkerRef remote = m_remoteTalkers.forward(pos);
		
			// already listening?
			if(findRemoteListener(remote->m_connGuid, userId).notNull())
			{
				LOGWARN(netDebug::g_Log, "<netVoice::Manager> RemoteListener(%llu) for mic(%llu) already present.", remote->m_userId, userId);
			}
			else
			{
				// add listening buffer.
				m_remoteListeners.addTail(new RemoteListener(user.m_profile, remote->m_connGuid));
			}
		}
		return true;
	}

	bool Manager::addRemoteUser(const State::RemoteUser& user)
	{
		m_state.addRemoteUser(user);

		netUserId userId = user.userId();
		
		// add remote talker to list
		if(findRemoteTalker(userId).notNull())
		{
			LOGWARN(netDebug::g_Log, "<netVoice::Manager> RemoteTalker(%llu) already present.", userId);
		}
		else
		{
			m_remoteTalkers.addTail(new RemoteTalker(user.m_user, user.m_connGuid));
		}

		// add listening buffers to list.
		bdLinkedList<LocalTalkerRef>::Position pos = m_localTalkers.getHeadPosition();
		while(pos)
		{
			// listenting to that guy
			LocalTalkerRef talker = m_localTalkers.forward(pos);
		
			// already listening?
			if(findRemoteListener(user.m_connGuid, talker->m_userId).notNull())
			{
				LOGWARN(netDebug::g_Log, "<netVoice::Manager> RemoteListener(%llu) for mic(%llu) already present.", userId, talker->m_userId);
			}
			else
			{
				// add listening buffer.
				m_remoteListeners.addTail(new RemoteListener(talker->m_profile, user.m_connGuid));
			}
		}
		return true;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

