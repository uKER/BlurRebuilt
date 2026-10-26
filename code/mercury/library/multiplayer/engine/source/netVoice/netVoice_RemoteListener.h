#ifndef NETVOICE_REMOTELISTENER_H
#define NETVOICE_REMOTELISTENER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netGuid/netGuid.h"
#include "./netVoice/netVoice_Cache.h"
#include "netTime/netTime.h"
#include "netTransport/netTransport_ConnGuid.h"
#include "netTransport/netTransport_Socket.h"
#include <SystemProfile/SystemProfile.h>

namespace netVoice
{
	BD_REFERENCE(RemoteListener);
	class RemoteListener: public bdReferencable
	{
	public:
		RemoteListener(const SysProfile::FullProfile& talker, const netTransport::ConnGuid& connGuid);
		~RemoteListener();

		bool mute(bool onOff);
		bool transmit(netTransport::SocketRef socket, int socketPort);
		bool cache(const char* voiceData, int voiceDataSize);

		netTransport::ConnGuid		m_connGuid;		// connection we are linked to
		SysProfile::FullProfile		m_talker;		// local talker id.
		netUserId					m_talkerId;		// local talker id.
		bool						m_muted;		// muted voice?
		float						m_volume;		// local talkers voice volume.
		CacheRef					m_cache;		// voice data cache.
		netStopwatch				m_transmitTick;	// transmit with a tick
	};
}

#endif //NETVOICE_REMOTELISTENER_H