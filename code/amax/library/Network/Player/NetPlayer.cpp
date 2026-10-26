#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include <network/Network.h>
#include "../Misc/NetDebug.h"
#include "NetPlayer.h"
#include "netPlayerVoice.h"
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
	C_NetPlayer::C_NetPlayer()	:	m_VoiceBanned(false), m_HasCommunicator(false), m_DlcMask(0), m_ConsoleId(0), m_IsSpeaking(0.0f), 
									m_UpStreamKbps(0), m_DownStreamKbps(0), m_LocalIndex(0), 
									m_ControllerPort(-1), m_pInBox(NULL), m_pOutBox(NULL), m_pVoice(NULL), m_QuickMute(0)
	{
		for (U32 i=0; i<BD_COMMON_ADDR_SERIALIZED_SIZE; i++)
			m_CommonAddress[i] = 0;

		m_VoiceEncodeArray.ClearBits();
		m_PlayerNonce.Create();
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
		m_RaceData.Flush();

		m_PartyConnection.m_Address = BD_NULL;
		m_PartyData.Flush();

		m_ProfileData.Flush();

		// flush voice
		if (m_pVoice)
		{
			for (S32 i=0; i<m_pVoice->Size(); i++)
			{
				(*m_pVoice)[i].Flush();
				(*m_pVoice)[i].SetAvailable(true);
			}
		}

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

		m_VoiceEncodeArray.ClearBits();

		m_HasCommunicator = false;
		m_VoiceBanned =false;
		m_DlcMask = 0;
		m_ConsoleId = 0;
		m_IsSpeaking = 0.0f;
		m_UpStreamKbps = 0;
		m_DownStreamKbps = 0; 
		m_LocalIndex = 0;
		m_ControllerPort = -1;
		m_pInBox = NULL;
		m_pOutBox = NULL; 
		m_pVoice = NULL;
		m_QuickMute = 0;
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

		if (0 == localSenderId)
		{
			LOGERROR(g_Log, "C_NetPlayer::SendReliable() - trying to send reliable message before we have a valid join reference.");
			return false;
		}

		C_NetPlayerConnection& conn = partyMessage ? m_PartyConnection : m_RaceConnection;
		if (!conn.CanMessage())
		{
			if (!conn.m_HandshakingComplete)
			{
				LOGWARN(g_Log, "C_NetPlayer::SendReliable() - trying to send reliable message before handshaking is complete.");
			}
			else if (conn.m_DisconnectRequested != eNetPlayerDisconnectionTypeNone)
			{
				LOGWARN(g_Log, "C_NetPlayer::SendReliable() - trying to send reliable message when player is flagged to be disconnected.");
			}
			return false;
		}

		const bool sent = m_pOutBox->Add(msg, messageId, partyMessage, localSenderId, initialConnection);
		if (!sent)
		{
			LOGINFO(g_Log, "C_NetPlayer::SendReliable() - unable to add reliable message to queue type %d", messageId);
		}

		return sent;
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
		
		if (!conn.CanMessage())
		{
			if (!conn.m_HandshakingComplete)
			{
				LOGWARN(g_Log, "C_NetPlayer::SendUnReliable() - trying to send unreliable message before handshaking is complete.");
			}
			else if (conn.m_DisconnectRequested != eNetPlayerDisconnectionTypeNone)
			{
				LOGWARN(g_Log, "C_NetPlayer::SendUnReliable() - trying to send unreliable message when player is flagged to be disconnected.");
			}
			return false;
		}

		// reset the time so we don't send keepalive packets if we have sent other unreliable data.
		conn.m_TimeSinceLastSentUnReliable = 0.0f;

		return router.SendMessage(msg, conn.m_Address);
	}

	void C_NetPlayer::RemovePendingReliableMessages(const bool partyMessages)
	{
		if (m_pOutBox)
			m_pOutBox->RemoveUnSentMessages(partyMessages);
		if (m_pInBox)
			m_pInBox->RemoveUnSentMessages(partyMessages);
	}

	void C_NetPlayer::FlushOutgoingVoice(void)
	{
		if (NULL == m_pVoice)
			return;
		for (S32 i=0; i<m_pVoice->Size(); i++)
			(*m_pVoice)[i].Flush();
	}

	bool C_NetPlayer::AppendOutgoingVoice(const U8 localNetPlayerIndex, const U8* pBuffer, const U32 bufferSize)
	{
		if (NULL == m_pVoice || NULL == pBuffer || 0 == bufferSize)
			return false;

		if (localNetPlayerIndex >= m_pVoice->Size())
			return false; // not enough local player voice queues defined.

		return (*m_pVoice)[localNetPlayerIndex].Append(pBuffer, bufferSize);
	}
	
	U32 C_NetPlayer::VoiceSize(void) const
	{
		if (NULL == m_pVoice)
			return 0;
		
		U32 size = 0;
		for (S32 i=0; i<m_pVoice->Size(); i++)
		{
			size += (*m_pVoice)[i].Size();
		}
		return size;
	}

	U32 C_NetPlayer::VoiceSize(const U8 localPlayerIndex) const
	{
		if (NULL == m_pVoice)
			return 0;
		return (*m_pVoice)[localPlayerIndex].Size();
	}

	bool C_NetPlayer::WriteOutgoingVoiceToMessage(C_NetMessage& msg)
	{
		if (NULL == m_pVoice)
			return false;
		
		bool ok = true;
		for (S32 i=0; i<m_pVoice->Size(); i++)
		{
			if ((*m_pVoice)[i].Size() > 0)
				ok = ok && (*m_pVoice)[i].WriteToMessage(msg);

			// clear data once copied.
			(*m_pVoice)[i].Flush();
		}
		return ok;
	}
	
	bool C_NetPlayer::IsInRace(void)
	{
		if (m_RaceConnection.IsActive())
		{
			switch (m_RaceConnection.m_PlayerState)
			{
			case eNetPlayerStateRacing:
			case eNetPlayerStateFinished:
				return true;
				break;
			}
		}
		return false;
	}

	bool C_NetPlayer::CanQuickMute(void) const
	{
		if (LocalPlayer() || m_VoiceBanned)
			return false;
		return true;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
