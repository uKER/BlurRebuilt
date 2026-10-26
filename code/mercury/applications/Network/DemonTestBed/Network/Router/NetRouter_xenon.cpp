#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include "../Misc/NetDebug.h"
#include "NetRouter_xenon.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>
#include <bdconnection/bdConnection.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterXenon::C_NetRouterXenon()
	{
	}

	// destructor
	C_NetRouterXenon::~C_NetRouterXenon()
	{
		Shutdown();
	}

	bool C_NetRouterXenon::Initialise(const U32 sendBufferSize)
	{
		if (m_Initialised)
			return true;

		if (C_NetRouterBase::Initialise(sendBufferSize))
			m_Initialised = true;

		return m_Initialised;
	}

	void C_NetRouterXenon::Shutdown(void)
	{
		PlatformDisconnect();
		C_NetRouterBase::Shutdown();	
		m_Initialised = false;
	}

	bool C_NetRouterXenon::PlatformConnect(const bool onlineMode, char* userName, char* password)
	{
		if (!m_Initialised)
			return false;

		// no point disconnecting and re connecting unless online mode changes.
		if (GetPlatformConnectionState() != eConnectionStateNotActive && GetPlatformConnectionState() != eConnectionStateFailed)
		{
			if (onlineMode == IsOnlineMode())
				return true; // may as well carry on the same connection attempt.
		}
			
		// disconnect if we are currently connected .
		PlatformDisconnect();
	
		// store the connection mode
		m_OnlineMode = onlineMode;

		// startup demonware, it won't try and access the servers since it doesn't need to.
		m_PlatformConnectionState = eConnectionStateInProgress;
		m_LastConnectionErrorCode = eConnectionResultNone;

		return true;
	}

	void C_NetRouterXenon::PlatformDisconnect(void)
	{
		if (!m_Initialised)
			return;

		StopBandwidthEvaluation();

		if (GetPlatformConnectionState() == eConnectionStateNotActive)
			return;

		// stop demonware..
		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (lobbyService)
			lobbyService->disconnect();

		net->stop();

		if (m_PlatformConnectionState != eConnectionStateFailed)
			m_PlatformConnectionState = eConnectionStateNotActive;
		m_OnlineMode = false;
	}

	void C_NetRouterXenon::Poll(void)
	{
		if (!m_Initialised)
			return;

		C_NetRouterBase::Poll();

		// no lsp connections so just say connected all the time. sinec we can't use any demonware
		// functionality without an lsp.
		if (eConnectionStateInProgress == m_PlatformConnectionState)
			m_PlatformConnectionState = eConnectionStateComplete;
	}
}// end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
