#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include "../Misc/NetDefines.h"
#include "../Misc/NetDebug.h"
#include "NetPlayerConnection.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{	
	C_NetPlayerConnection::C_NetPlayerConnection()	:	m_PlayerState(eNetPlayerStateUnSet), m_JoinId(0), m_DropRequested(false), 
														m_PublicPlayer(true), m_Host(false), m_AddedToOnlineSession(false), m_TimeSinceLastSentUnReliable(0.0f),
														m_KeepAliveFrequency(NET_KEEPALIVE_NORMAL_FREQUENCY)
	{
	}

	C_NetPlayerConnection::~C_NetPlayerConnection()
	{
		m_TimeSinceLastReceivedKeepAlive.Stop();

		if (m_Address.notNull())
		{
			// ideally we should have unregistered this address when we delete a player from the player manager.
			LOGWARN(g_Log, "C_NetPlayerConnection() - bdAddrHandleRef still valid upon deletion.");
		}

		m_Address = BD_NULL;
	}

	bool C_NetPlayerConnection::IsConnected(void) const
	{
		if (m_Address.notNull())
		{
			bdNetImpl *const net = bdNet::getInstance();
			if (net)
			{
				bdSocketRouter* socketRouter =  net->getSocketRouter();
				if (socketRouter)
				{
					if (socketRouter->getStatus(m_Address) == BD_SOCKET_CONNECTED)
						return true;			
				}
			}
		}
		return false;
	}

	void C_NetPlayerConnection::Disconnect(const bool unregisterAddress)
	{
		if (unregisterAddress)
		{
			if (m_Address.notNull())
			{
				bdNetImpl *const net = bdNet::getInstance();
				if (net)
				{
					bdSocketRouter* socketRouter =  net->getSocketRouter();
					if (socketRouter)
						socketRouter->disconnect(m_Address);
				}
			}
		}

		m_Address = BD_NULL;
		m_PlayerState = eNetPlayerStateUnSet;
		m_JoinId = 0;
		m_DropRequested = false; 
		m_PublicPlayer = true;
		m_Host = false; 
		m_AddedToOnlineSession = false;
		m_KeepAliveFrequency = NET_KEEPALIVE_NORMAL_FREQUENCY;
		m_TimeSinceLastSentUnReliable = 0.0f;
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
