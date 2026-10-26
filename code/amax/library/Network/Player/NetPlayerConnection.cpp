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
	C_NetPlayerConnection::C_NetPlayerConnection()	:	m_PlayerState(eNetPlayerStateUnSet), m_JoinId(0), m_DisconnectRequested(eNetPlayerDisconnectionTypeNone), 
														m_Host(false), m_AddedToOnlineSession(false), m_HandshakingComplete(false), m_WaitingForMigrationReply(false), 
														m_IgnoreMigration(false), m_TimeSinceLastSentUnReliable(0.0f), m_KeepAliveFrequency(NET_KEEPALIVE_NORMAL_FREQUENCY)
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
					bdDTLSAssociationStatus status = socketRouter->getStatus(m_Address);
					if (BD_SOCKET_CONNECTED == status)
						return true;			
				}
			}
		}
		return false;
	}

	bool C_NetPlayerConnection::CanMessage(void) const
	{
		if (!IsConnected() || m_DisconnectRequested != eNetPlayerDisconnectionTypeNone)
			return false;

		return m_HandshakingComplete;
	}

	void C_NetPlayerConnection::StartKeepAliveTimer(void)
	{
		m_TimeSinceLastReceivedKeepAlive.Stop();
		m_TimeSinceLastReceivedKeepAlive.Start();
	}

	bool C_NetPlayerConnection::KeepAliveTimedOut(const float maxTime)
	{
		if (IsActive())
		{
			// timer isnt active so assume disconnection must occur. game logic should only test for keepalive disconnections
			// when it knows a connection is actually complete.
			if (!m_TimeSinceLastReceivedKeepAlive.IsActive())
				return true; 

			F32 elapsed = m_TimeSinceLastReceivedKeepAlive.ElapsedTimeInSeconds();
			if (elapsed >= maxTime)
				return true;
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

		m_PlayerState							=	eNetPlayerStateUnSet;						
		m_DisconnectRequested					=	eNetPlayerDisconnectionTypeNone;				
		m_Host									=	false;								
		m_AddedToOnlineSession					=	false;				
		m_HandshakingComplete					=	false;				
		m_WaitingForMigrationReply				=	false;
		m_IgnoreMigration						=	false;
		m_TimeSinceLastReceivedKeepAlive.Stop();	
		m_TimeSinceLastSentUnReliable			=	0.0f;		
		m_KeepAliveFrequency					=	NET_KEEPALIVE_NORMAL_FREQUENCY;				
		m_Address								=	BD_NULL;							
		m_JoinId								=	0;	
		m_VoiceMuteData.Clear();
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
