#ifndef NETVOICE_REMOTETALKER_H
#define NETVOICE_REMOTETALKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include "netTime/netTime.h"
#include "netTransport/netTransport_Message.h"
#include "netTransport/netTransport_ConnGuid.h"
#include <SystemProfile/SystemProfile.h>

namespace netVoice
{
	BD_REFERENCE(Cache);
	BD_REFERENCE(RemoteTalker);
	class RemoteTalker: public bdReferencable
	{
	public:
		enum 
		{
			DECODE_PRIORITY_NOT_SET,
			DECODE_PRIORITY_MUTE,
			DECODE_PRIORITY_LISTEN,
		};

		RemoteTalker(const SysProfile::UserIdentifier& user, const netTransport::ConnGuid& connGuid);
		~RemoteTalker();

		bool talking();
		bool headset(int i, bool onOff);
		bool decode();
		bool receive(netTransport::MsgVoiceFragments* msg);

		netTransport::ConnGuid		m_connGuid;		// connection attached to player
		SysProfile::UserIdentifier	m_user;			// user profile.
		netUserId					m_userId;		// online user Id.
		netStopwatch				m_debounceTalk; // notifies whe a player starts talking
		CacheRef					m_cache;		// cached voice data
		
		enum { MAX_HEADSETS = 4 };
		int							m_headsetStates[MAX_HEADSETS];
		int							m_headsetRequests[MAX_HEADSETS];
		
	private:
		bool updateHeadsets();
		bool unregister();
		bool registered();
		bool register_();
	};
}

#endif //NETVOICE_REMOTETALKER_H