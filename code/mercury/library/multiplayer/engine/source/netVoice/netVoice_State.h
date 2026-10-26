#ifndef NETVOICE_STATE_H
#define NETVOICE_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnGuid.h"

#include <SystemProfile/SystemProfile.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netVoice
{
	BD_REFERENCE(State);
	class State: public bdReferencable
	{
	public:
		class LocalUser
		{
		public:
			LocalUser();
			netUserId userId() const;
			bool m_active;						// active slot or not?
			bool m_headset;						// headset present.
			SysProfile::FullProfile m_profile;	// the full profile spec.
		};

		class RemoteUser
		{
		public:
			RemoteUser();
			netUserId userId() const;
			
			enum { MAX_LOCAL_USERS = 4 };
			bool m_active;						// active slot or not?
			bool m_headset;						// headset present.
			bool m_muted[MAX_LOCAL_USERS];		// remote listener muted against local talker
			netTransport::ConnGuid	m_connGuid;	// connection
			SysProfile::UserIdentifier m_user;	// the full profile spec.
		};

		State();
		~State();

		bool clear();
		bool addLocalUser(const State::LocalUser& user);
		bool addRemoteUser(const State::RemoteUser& user);
		bool removeLocalUser(const State::LocalUser& user);
		bool removeRemoteUser(const State::RemoteUser& user);
		
		State::LocalUser*			findLocalUser(const netUserId& userId);
		const State::LocalUser*		findLocalUser(const netUserId& userId) const;
		State::RemoteUser*			findRemoteUser(const netUserId& userId);
		const State::RemoteUser*	findRemoteUser(const netUserId& userId) const;

		enum { MAX_LOCAL_USERS = 4, MAX_REMOTE_USERS = 16 };

		int						m_socketPort;						// socket to send and receive voice from
		LocalUser				m_localUsers[MAX_LOCAL_USERS];		// us, talking.
		RemoteUser				m_remoteUsers[MAX_REMOTE_USERS];	// who's talking to us.
	};
}
#endif //NETVOICE_STATE_H