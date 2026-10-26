#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "NetParty.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Message/NetMessage.h"
#include "../Core/NetServices.h"
#include "../Router/NetRouterBase.h"
#include "../Player/NetPlayerManager.h"
#include "../Core/NetSessionKeyTracker.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetParty::C_NetParty() : m_Initialised(false), m_Running(false), m_ppPlayerMgr(NULL), m_ppRouter(NULL), m_pSessionKeyTracker(NULL)
	{
	}

	C_NetParty::~C_NetParty()
	{
		Shutdown();
	}

	bool C_NetParty::Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager **ppPlayerManager)
	{
		if (m_Initialised)  
			return true;

		m_ppRouter = ppRouter; 
		m_ppPlayerMgr = ppPlayerManager;

		m_pSessionKeyTracker = new C_NetSessionKeyTracker;

		if (m_pSessionKeyTracker)
		{
			m_QoS.m_Version = NET_PACKET_VERSION;
			m_PartyNonce.Create(0);
			m_Initialised = true;
		}
		else
			Shutdown();

		return m_Initialised;
	}

	void C_NetParty::Shutdown(void)
	{
		DisconnectAndClear();

		if (m_pSessionKeyTracker)
			delete m_pSessionKeyTracker;
		m_pSessionKeyTracker = NULL;

		m_ppRouter = NULL;
		m_ppPlayerMgr = NULL;
		m_Initialised = false;
	}

	void C_NetParty::Poll(const F32 frameTimeInSeconds)
	{
		if (!m_Initialised)
			return;

		// track security keys still in use
		PollSecurityKeyTracker();
	}

	void C_NetParty::DisconnectAndClear(void)
	{
		if (!m_Initialised)
			return;

		m_Running = false;
		m_PartyNonce.Create(0);
		m_pSessionKeyTracker->Flush();
	}

	bool C_NetParty::ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId)
	{
		if (!m_Initialised || !m_Running)
			return false;

		return true;
	}

	bool C_NetParty::ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bdAddrHandleRef& addr)
	{
		if (!m_Initialised || !m_Running)
			return false;

		return false;
	}

	bool C_NetParty::ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const bdAddrHandleRef& addr)
	{
		if (!m_Initialised || !m_Running)
			return false;

		return false;

	}

	bool C_NetParty::ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, const bdAddrHandleRef& addr)
	{
		if (!m_Initialised || !m_Running)
			return false;

		return false;
	}

	void C_NetParty::SendJoinFailed(const eNetJoinSessionErrors reason, const bdAddrHandleRef& returnAddr)
	{
		if (!m_Initialised || !m_Running)
			return;

		C_NetMessage& msg = (*m_ppRouter)->GetOutgoingUnReliable();

		// create base join failed message.
		msg.CreateUnreliable(eNetMessageTypeJoinReply, (*m_ppPlayerMgr)->LocalPrimaryJoinId(true), true);

		// pack failed reason.
		msg.WriteU8(reason);

		// end writing.
		msg.TerminateMessage();

		(*m_ppRouter)->SendMessage(msg, returnAddr);
	}

	void C_NetParty::PollSecurityKeyTracker(void)
	{
		if (!m_Initialised || !m_Running)
			return;
#if !defined (PLATFORM_XENON)
		SecurityKeyIds activeKeyIds;
		C_NetPlayerManager& mgr = **m_ppPlayerMgr;

		for (S32 i=0; i<mgr.TotalSize(); i++)
		{
			const C_NetPlayer* pPlayer = mgr.Index(i);
			if (pPlayer->LocalPlayer() || !pPlayer->PrimaryPlayer())
				continue; // dont check against local players or non remote primary players.

			if (pPlayer->m_PartyConnection.m_Address.isNull())
				continue; // player is not connected on the party channel.

			bool alreadyAdded = false;
			for (S32 z=0; !alreadyAdded && z<activeKeyIds.Size(); z++)
			{
				if (activeKeyIds[z] == pPlayer->m_PartyConnection.m_Address->m_endpoint.getSecID())
					alreadyAdded = true;
			}

			if (!alreadyAdded)
				activeKeyIds.PushBack(pPlayer->m_PartyConnection.m_Address->m_endpoint.getSecID());
		}

		m_pSessionKeyTracker->Poll(activeKeyIds);
#endif
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
