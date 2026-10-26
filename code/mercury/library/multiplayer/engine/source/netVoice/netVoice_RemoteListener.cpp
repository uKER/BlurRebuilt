
#include "stdafx.h"
#include "netVoice_RemoteListener.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netVoice
{
	RemoteListener::RemoteListener(	const SysProfile::FullProfile& talker, const netTransport::ConnGuid& connGuid)
	: m_talker(talker)
	, m_talkerId(netPlatform::Core::instance()->getUserId(talker))
	, m_connGuid(connGuid)
	, m_volume(1.0f)
	, m_cache(new Cache())
	, m_muted(false)
	{
	}

	RemoteListener::~RemoteListener()
	{
	}

	bool RemoteListener::mute(bool onOff) 
	{ 
		m_muted = onOff; 
		return true; 
	}

	bool RemoteListener::transmit(netTransport::SocketRef socket, int socketPort)
	{
		if(!m_transmitTick.ticked(100 * 1000))
			return false;

		if(m_muted)
		{
			m_cache->clear();
			return false;
		}

		if(socket.isNull())
			return false;

		char voiceData[Cache::MAX_CACHED_VOICE_SIZE];
		int  voiceLen;
		if(!m_cache->read(voiceData, sizeof(voiceData), voiceLen))
			return false;

		if(voiceLen == 0)
			return true;

		netTransport::MsgVoiceFragments msg;
		assert((voiceLen % m_cache->m_frameSize) == 0);
		assert(voiceLen <= msg.MAX_PAYLOAD);

		msg.m_userId		= m_talkerId;
		msg.m_fragmentSize	= Communicator::C_Core::Instance().AudioFrameSize();
		msg.m_maxFragments	= voiceLen / msg.m_fragmentSize;
		msg.m_minFragments	= 1;
		memcpy(msg.m_payload, voiceData, voiceLen);

		if(!socket->send(&msg, socketPort, m_connGuid))
			return false;

		int rollback = (msg.m_maxFragments - msg.m_fragmentsTransmitted) * msg.m_fragmentSize;
		m_cache->unread(rollback);

		if(!netDebug::dbg_voiceCache)
		{
			m_cache->clear();
		}
		else
		{
			int rollback = (msg.m_maxFragments - msg.m_fragmentsTransmitted) * msg.m_fragmentSize;
			m_cache->unread(rollback);
		}
		return true;
	}

	bool RemoteListener::cache(const char* voiceData, int voiceDataSize)
	{
		if(m_muted)
		{
			m_cache->clear();
			return false;
		}

		if(!netDebug::dbg_voiceCache)
				m_cache->clear();

		return m_cache->write(voiceData, voiceDataSize);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

