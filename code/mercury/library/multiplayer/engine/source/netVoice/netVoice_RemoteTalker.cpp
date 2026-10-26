
#include "stdafx.h"
#include "./netVoice_Cache.h"
#include "./netVoice_RemoteTalker.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


namespace netVoice
{
	RemoteTalker::RemoteTalker(const SysProfile::UserIdentifier& user, const netTransport::ConnGuid& connGuid)
	: m_user(user)
	, m_connGuid(connGuid)
	, m_userId(netPlatform::Core::instance()->getUserId(user))
	, m_cache(new Cache())
	{
		for(int i = 0; i < MAX_HEADSETS; i ++)
		{
			m_headsetStates[i] = DECODE_PRIORITY_NOT_SET;
			m_headsetRequests[i] = DECODE_PRIORITY_LISTEN;
		}		
		register_();
	}
	RemoteTalker::~RemoteTalker()
	{
		unregister();
	}

	bool RemoteTalker::unregister()
	{
		if(!registered())
			return true;

		if(!Communicator::C_Core::Instance().UnRegisterRemoteTalker(m_user))
		{
			LOGERROR(netDebug::g_Log, "<netVoice::RemoteTalker> UnRegisterRemoteTalker(%llu) FAILED!!!", m_userId);
			return false;
		}
		
		LOGINFO(netDebug::g_Log, "<netVoice::RemoteTalker> UnRegisterRemoteTalker(%llu) SUCCESS!", m_userId);
		return true;
	}

	bool RemoteTalker::registered()
	{
		return Communicator::C_Core::Instance().IsRemoteTalkerRegistered(m_user);
	}
	
	bool RemoteTalker::register_()
	{
		if(registered())
			return true;

		if(!Communicator::C_Core::Instance().RegisterRemoteTalker(m_user))
		{
			LOGERROR(netDebug::g_Log, "<netVoice::RemoteTalker> RegisterRemoteTalker(%llu) FAILED!!!", m_userId);
			return false;
		}

		LOGINFO(netDebug::g_Log, "<netVoice::RemoteTalker> RegisterRemoteTalker(%llu) SUCCESS!!!", m_userId);
		return true;			
	}

	bool RemoteTalker::receive(netTransport::MsgVoiceFragments* msg)
	{
		assert(msg->m_userId == m_userId);
		
		if(msg->m_fragmentSize != m_cache->m_frameSize)
		{
			LOGERROR(netDebug::g_Log, "<netVoice::RemoteTalker> mismatch in received voice packet size(%d instead of %d).", msg->m_fragmentSize, m_cache->m_frameSize);
			return false;
		}

		if (1)//!netDebug::dbg_voiceCache)
		{
			// submit voice to communicator
			U32 voiceDecoded;
			Communicator::C_Core::Instance().SubmitRemoteVoiceData(m_user, (U8*)msg->m_payload, msg->m_fragmentsTransmitted * msg->m_fragmentSize, &voiceDecoded);
			m_debounceTalk.start();
			return true;
		}
		else
		{
			// write bytes to cache
			return m_cache->write(msg->m_payload, msg->m_fragmentsTransmitted * msg->m_fragmentSize);
		}
	}

	bool RemoteTalker::updateHeadsets()
	{
		if(!registered())
			register_();

		if(!registered())
			return false;

		for(int i = 0; i < MAX_HEADSETS; i ++)
		{
			// no local talker. no point setting the decode priority
			if(!Communicator::C_Core::Instance().IsLocalTalkerRegistered(i))
			{
				m_headsetStates[i] = DECODE_PRIORITY_NOT_SET;
				continue;
			}

			// decode state equal to our requested state. nothing to do.
			if(m_headsetRequests[i] == m_headsetStates[i])
				continue;

			BIZ_ASSERT_MSG((m_headsetRequests[i] == DECODE_PRIORITY_MUTE || m_headsetRequests[i] == DECODE_PRIORITY_LISTEN), "invalid voice decode state");

			// do we want to mute?
			bool mute = (m_headsetRequests[i] == DECODE_PRIORITY_MUTE);

			if(!Communicator::C_Core::Instance().SetDecodingPriority(i, m_user, 0, mute))
			{
				m_headsetStates[i] = DECODE_PRIORITY_NOT_SET;
				LOGERROR(netDebug::g_Log, "<netVoice::RemoteTalker> DecodingPriority(%d, %llu, %d, %s) FAILED!!!", i, m_userId, 0, mute? "muted" : "unmuted");
				continue;
			}
			else
			{
				m_headsetStates[i] = m_headsetRequests[i];
				LOGINFO(netDebug::g_Log, "<netVoice::RemoteTalker> DecodingPriority(%d, %llu, %d, %s!)", i, m_userId, 0, mute? "muted" : "unmuted");
			}
		}
		return true;
	}

	bool RemoteTalker::talking()
	{
		return m_debounceTalk.started() && !m_debounceTalk.elapsed(1000 * 1000);
	}

	bool RemoteTalker::headset(int i, bool onOff)
	{
		if(i < 0 || i >= MAX_HEADSETS)
		{
			LOGERROR(netDebug::g_Log, "<netVoice::RemoteTalker> invalid headset (%d).", i);
			return false;
		}

		m_headsetRequests[i] = (onOff)? DECODE_PRIORITY_LISTEN : DECODE_PRIORITY_MUTE;
		return true;

	}

	bool RemoteTalker::decode()
	{
		if (1)//!netDebug::dbg_voiceCache)
			return true;

		if(!registered())
			return false;

		// retrieve a portion so we can send to communicator (try 160 bytes) for decoding.
		char voiceData[Cache::MAX_CACHED_VOICE_SIZE / 4];
		int	voiceRetrieved;	
		if(!m_cache->read(voiceData, sizeof(voiceData), voiceRetrieved))
			return false;
		
		if(voiceRetrieved == 0)
			return true;

		// submit voice to communicator
		unsigned int voiceDecoded;	
		if(Communicator::C_Core::Instance().SubmitRemoteVoiceData(m_user, (U8*)voiceData, voiceRetrieved, &voiceDecoded))
		{
			m_cache->debugBytes(voiceData, voiceRetrieved, "submit");
		}
		else
		{
			//LOGWARN(netDebug::g_Log, "<netVoice::RemoteTalker> decode error (%llu)", m_userId);
		}
		assert((voiceDecoded % m_cache->m_frameSize) == 0);
		assert((voiceRetrieved % m_cache->m_frameSize) == 0);

		//netDebug::logInfo("<netVoiceManager::updatePackets> submited %d (%d decoded) bytes of voice.", voiceRetrieved, voiceDecoded);

		// roll back the bytes that couldn't decode yet.
		m_cache->unread(voiceRetrieved - (int)voiceDecoded);
		return true;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

