
#include "stdafx.h"
#include "./netVoice_LocalTalker.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netVoice
{
	LocalTalker::LocalTalker(const SysProfile::FullProfile& profile)
	: m_profile(profile)
	, m_userId(netPlatform::Core::instance()->getUserId(profile))
	{
		register_();
	}

	LocalTalker::~LocalTalker()
	{
		unregister();
	}

	bool LocalTalker::talking()
	{
		return m_debounceTalk.started() && !m_debounceTalk.elapsed(1000 * 1000);
	}

	bool LocalTalker::registered()
	{
		return Communicator::C_Core::Instance().IsLocalTalkerRegistered(m_profile.m_UserIndex);
	}

	bool LocalTalker::unregister()
	{
		if(!registered())
			return true;

		if(!Communicator::C_Core::Instance().UnRegisterLocalTalker(m_profile.m_UserIndex))
		{
			LOGERROR(netDebug::g_Log, "<netVoice::LocalTalker> UnRegisterLocalTalker(%S) FAILED!!!", m_profile.m_UserIndex);
			return false;
		}
		
		LOGINFO(netDebug::g_Log, "<netVoice::LocalTalker> UnRegisterLocalTalker(%S) SUCCESS!!!", m_profile.m_UserIndex);
		return true;
	}
	
	bool LocalTalker::register_()
	{
		if(registered())
			return true;

		if(!Communicator::C_Core::Instance().RegisterLocalTalker(m_profile, false))
		{
			LOGERROR(netDebug::g_Log, "<netVoice::LocalTalker> RegisterLocalTalker(%S) FAILED!!!", m_profile.m_Name);
			return false;
		}		
		LOGINFO(netDebug::g_Log, "<netVoice::LocalTalker> RegisterLocalTalker(%S) SUCCESS!", m_profile.m_Name);
			
		if (!Communicator::C_Core::Instance().SetLocalMode(m_profile.m_UserIndex, Communicator::C_Core::eModeNormal))
		{
			LOGERROR(netDebug::g_Log, "<netVoice::LocalTalker> SetLocalMode(%S, %s) FAILED!!!", m_profile.m_Name, "normal");
			Communicator::C_Core::Instance().UnRegisterLocalTalker(m_profile.m_UserIndex);
			return false;
		}
		LOGINFO(netDebug::g_Log, "<netVoice::LocalTalker> SetLocalMode(%S, %s) SUCCESS!", m_profile.m_Name, "normal");
		return true;
	}

	int LocalTalker::poll(char* data, int maxData)
	{
		if(!registered())
			register_();

		if(!registered())
			return false;

		unsigned int len=0;

		if(!Communicator::C_Core::Instance().GetLocalVoiceData(m_profile.m_UserIndex, (U8*)data, maxData, len))
			return 0;

		if(len > 0)
			m_debounceTalk.start();

		return (int)len;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

