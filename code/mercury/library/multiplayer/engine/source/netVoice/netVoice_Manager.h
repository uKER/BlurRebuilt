#ifndef NETVOICE_MANAGER_H
#define NETVOICE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "./netVoice_LocalTalker.h"
#include "./netVoice_RemoteTalker.h"
#include "./netVoice_RemoteListener.h"
#include "./netVoice_State.h"
#include "netEnums/netEnums.h"
#include "netTime/netTime.h"

#include <SystemProfile/SystemProfile.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netVoice
{
	BD_REFERENCE(Manager);
	class Manager: public bdReferencable
	{
	public:
		Manager(netTransport::SocketRef socket);
		~Manager();

		bool update(const State& state);
		bool talking(const netUserId userId);
		bool headset(const netUserId userId);
		bool receive(netTransport::Message* msg);

		State m_state;		
		netTransport::SocketRef m_socket;
		
		bdLinkedList<LocalTalkerRef> m_localTalkers;
		bdLinkedList<RemoteTalkerRef> m_remoteTalkers;
		bdLinkedList<RemoteListenerRef> m_remoteListeners;
	
	private:
		void syncLocalUsers(State& prevState, const State& newState);
		void syncRemoteUsers(State& prevState, const State& newState);
		void syncMute(const State& state);
		void transmit();
		void decode();
		

		int remoteTalkerCount(const netTransport::ConnGuid& connGuid);
			
		LocalTalkerRef		findLocalTalker(const netUserId user);
		RemoteTalkerRef		findRemoteTalker(const netUserId user);
		RemoteListenerRef	findRemoteListener(const netTransport::ConnGuid& connGuid, const netUserId talker);

		bool addLocalUser(const State::LocalUser& user);
		bool removeLocalUser(const State::LocalUser& user);
		
		bool addRemoteUser(const State::RemoteUser& user);
		bool removeRemoteUser(const State::RemoteUser& user);
	};
}

#endif //NETVOICE_MANAGER_H