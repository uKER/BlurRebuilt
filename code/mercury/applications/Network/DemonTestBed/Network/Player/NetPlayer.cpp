#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include <network/Network.h>
#include "../Misc/NetDebug.h"
#include "NetPlayer.h"
#include "../Message/NetMessageReliableInBox.h"
#include "../Message/NetMessageReliableOutBox.h"
#include "../Message/NetMessage.h"
#include "../Router/NetRouterBase.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor will be called when we allocate this object from the pool
	C_NetPlayer::C_NetPlayer()	:	m_ConsoleId(0), m_LocalIndex(0), m_IsSpeaking(false), m_ControllerPort(-1),
									m_pInBox(NULL), m_pOutBox(NULL)
	{
	}

	// destructor will be called when we add the object back into the pool after removing it from the linked list.
	C_NetPlayer::~C_NetPlayer()
	{
		Destroy();
	}

	void C_NetPlayer::Destroy(void)
	{
		if (!LocalPlayer() && PrimaryPlayer()) // only unregister address if it is the remote primary player.
		{
			bdNetImpl *const net = bdNet::getInstance();
			if (net)
			{
				bdSocketRouter *const socketRouter = net->getSocketRouter();	
				if (socketRouter)
				{				
					if (m_RaceConnection.m_Address.notNull())
					{
						LOGWARN(g_Log, "C_NetPlayer::Destroy() - race session address is still valid !!. going to unregister now.");
						socketRouter->disconnect(m_RaceConnection.m_Address);
					}
					
					if (m_PartyConnection.m_Address.notNull())
					{
						LOGWARN(g_Log, "C_NetPlayer::Destroy() - party session address is still valid !!. going to unregister now.");
						socketRouter->disconnect(m_PartyConnection.m_Address);
					}
				}
			}
		}

		m_RaceConnection.m_Address = BD_NULL;
		m_PartyConnection.m_Address = BD_NULL;

		// flush and release inbox if we have one
		if (m_pInBox)
		{
			m_pInBox->Flush();
			m_pInBox->SetAvailable(true);
		}

		// flush and release outbox if we have one.
		if (m_pOutBox)
		{
			m_pOutBox->Flush();
			m_pOutBox->SetAvailable(true);
		}

		m_pInBox = NULL;
		m_pOutBox = NULL;
	}

	bool C_NetPlayer::ConnectionActive(const bool partyConnection)
	{
		C_NetPlayerConnection& conn = partyConnection ? m_PartyConnection : m_RaceConnection;
		return conn.IsConnected();
	}

	bool C_NetPlayer::SendReliable(C_NetMessage& msg, const U8 messageId, const bool partyMessage, const U8 localSenderId, const bool initialConnection)
	{
		if (LocalPlayer() || NULL == m_pOutBox || m_LocalIndex != 0)
		{
			// not possible to send reliable messages to this player
			LOGERROR(g_Log, "C_NetPlayer::SendReliable() - unable to send a network message to this type of player.");
			return false; 
		}

		C_NetPlayerConnection& conn = partyMessage ? m_PartyConnection : m_RaceConnection;
		if (eNetPlayerStateUnSet == conn.m_PlayerState || conn.m_Address.isNull())
			return false;

		return m_pOutBox->Add(msg,messageId,partyMessage, localSenderId, initialConnection);
	}


	bool C_NetPlayer::SendUnReliable(C_NetMessage& msg, const bool partyMessage, C_NetRouterBase& router)
	{
		if (LocalPlayer() || m_LocalIndex != 0)
		{
			LOGERROR(g_Log, "C_NetPlayer::SendUnReliable() - unable to send a network message to this type of player.");
			return false; 
		}

		if (0 == msg.DataSize() || !msg.IsTerminated() || !msg.HasHeader())
			return false; // no data to send. or mesage not closed yet.

		if (msg.IsReliable())
			return false; //can't send reliable messages through this..

		C_NetPlayerConnection& conn = partyMessage ? m_PartyConnection : m_RaceConnection;

		// reset the time so we don't send keepalive packets if we have sent other unreliable data.
		conn.m_TimeSinceLastSentUnReliable = 0.0f;

		return router.SendMessage(msg, conn.m_Address);
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
