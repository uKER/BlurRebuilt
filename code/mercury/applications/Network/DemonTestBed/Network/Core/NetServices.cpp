#include <stdafx.h>
#include <Core/core.h>
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include "NetServices.h"
#include <network/Core.h>
#include <network/SessionDeletion.h>
#include <Network/ScoreboardReaderByRank.h>
#include <Network/ScoreboardReaderByUsers.h>
#include <Network/ScoreboardWriter.h>
#include <Network/CommunicatorManager.h>
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Core/NetTimer.h"
#include "../Player/NetPlayerManager.h"
#include "NetRace.h"
#include "NetParty.h"

#if defined(PLATFORM_XENON)
#include "../Router/NetRouter_xenon.h"
#elif defined (PLATFORM_PS3)
#include "../Router/NetRouter_ps3.h"
#elif defined (PLATFORM_PC)
#include "../Router/NetRouter_pc.h"
#else
#error ("*** ERROR: Unknown build configuration within NetService.cpp")
#endif

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// static data.
	C_NetServices C_NetServices::s_NetServices;
	static const F32 net30fps = 1.0f/30.0f;

	C_NetServices::C_NetServices() :	m_Initialised(false), m_pRouter(NULL), m_pPlayerManager(NULL), m_pScoreboardReaderByRank(NULL), m_pScoreboardReaderByUsers(NULL),
										m_pScoreboardWriter(NULL), m_pNetRace(NULL), m_pNetParty(NULL), m_pFrameTimer(NULL)
	{
	}

	C_NetServices::~C_NetServices()
	{
	}

	bool C_NetServices::Initialise(const bool networkStackAlreadyStarted)
	{
		if (m_Initialised)
			return true; 

		bool ok = false;

		NetPlay::C_Core::Instance().Initialise(true, networkStackAlreadyStarted);
		if (NetPlay::C_Core::Instance().IsInitialised())
		{
			ok = NetPlay::C_SessionDeletion::Instance().Initialise(10);
			ok = ok && Communicator::C_Core::Instance().Initialise(NET_MAX_LOCAL_PLAYERS, NET_MAX_VEHICLES-1);
			ok = ok && CreateScoreboardWriter();
			ok = ok && CreateScoreboardReaders();
			ok = ok && CreateRouter();
			ok = ok && CreatePlayerManager();
			ok = ok && CreateRace();
			ok = ok && CreateParty();
			ok = ok && CreateMisc();
		
			if (ok)
				m_Initialised = true;
			else
				Shutdown();
		}

		return m_Initialised;
	}

	void C_NetServices::Shutdown(void)
	{
		if (m_pFrameTimer)
		{
			m_pFrameTimer->Stop();
			delete m_pFrameTimer;
		}
		m_pFrameTimer = NULL;

		if (m_pNetParty)
		{
			m_pNetParty->Shutdown();
			delete m_pNetParty;
		}
		m_pNetParty = NULL;

		if (m_pNetRace)
		{
			m_pNetRace->Shutdown();
			delete m_pNetRace;
		}
		m_pNetRace = NULL;

		if (m_pPlayerManager)
		{
			m_pPlayerManager->Shutdown();
			delete m_pPlayerManager;
		}
		m_pPlayerManager = NULL;

		if (m_pRouter)
		{
			m_pRouter->Shutdown();
			delete m_pRouter;
		}
		m_pRouter = NULL;

		if (m_pScoreboardReaderByRank)
		{
			m_pScoreboardReaderByRank->StopAndCleanup();
			delete m_pScoreboardReaderByRank;
		}
		m_pScoreboardReaderByRank = NULL;

		if (m_pScoreboardReaderByUsers)
		{
			m_pScoreboardReaderByUsers->StopAndCleanup();
			delete m_pScoreboardReaderByUsers;
		}
		m_pScoreboardReaderByUsers = NULL;

		if (m_pScoreboardWriter)
		{
			m_pScoreboardWriter->StopAndCleanup();
			delete m_pScoreboardWriter;
		}
		m_pScoreboardWriter = NULL;

		Communicator::C_Core::Instance().Shutdown();
		
		if (NetPlay::C_Core::Instance().IsInitialised())
			NetPlay::C_SessionDeletion::Instance().Shutdown();

		if (NetPlay::C_Core::Instance().IsInitialised())
			NetPlay::C_Core::Instance().Shutdown(); // this will terminate demonware, any access to demonware after this point will kill the exectuable !.

		m_Initialised = false;
	}

	void C_NetServices::StopScoreboardRequests(void)
	{
		if (!m_Initialised)
			return;

		m_pScoreboardReaderByRank->StopAndCleanup();
		m_pScoreboardReaderByUsers->StopAndCleanup();

		m_pScoreboardWriter->StopAndCleanup();
	}

	bool C_NetServices::ReceiveNetMessage(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const U32 numUnEncryptedBytes, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		// update keep alive for player only is the message is not a join request.
		if (eNetMessageTypeJoinRequest != hdr.m_PacketType)
			m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);
		
		// if keep alive then all processing has been done already so tell router that packet 
		// has been processed and not to pass packet onto anyother listeners.
		if (eNetMessageTypeKeepAlive == hdr.m_PacketType)
			return true; 

		if (hdr.m_PartyChannel)
		{
			if (eNetMessageTypeJoinRequest == hdr.m_PacketType)
				return m_pNetParty->ReceiveJoinRequest(msg, hdr, addr);
			else
				return m_pNetParty->ReceiveUnReliable(msg, hdr.m_PacketType, numUnEncryptedBytes, hdr.m_SenderReference, addr);	
		}
		else
		{
			if (eNetMessageTypeJoinRequest == hdr.m_PacketType)
				return m_pNetRace->ReceiveJoinRequest(msg, hdr, addr);
			else
				return m_pNetRace->ReceiveUnReliable(msg, hdr.m_PacketType, numUnEncryptedBytes, hdr.m_SenderReference, addr);	
		}
		
		return false;
	}

	bool C_NetServices::ReceiveNetMessage(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		if (hdr.m_PartyChannel)
		{
			if (eNetMessageTypeJoinReply == hdr.m_PacketType)
			{
				m_pNetParty->ReceiveJoinReply(msg, hdr, addr);

				// since join replies are sent reliably but don't get added into the inbox we must manually reply with an acknowledgement packet.
				C_NetMessage& ackMsg  = m_pRouter->GetOutgoingUnReliable();
				ackMsg.CreateAcknowledgement(m_pPlayerManager->LocalPrimaryJoinId(true), hdr.m_PacketSequence , true);
				m_pRouter->SendMessage(ackMsg, addr);
				return true;
			}
		}
		else
		{
			if (eNetMessageTypeJoinReply == hdr.m_PacketType)
			{
				m_pNetRace->ReceiveJoinReply(msg, hdr, addr);

				// since join replies are sent reliably but don't get added into the inbox we must manually reply with an acknowledgement packet.
				C_NetMessage& ackMsg  = m_pRouter->GetOutgoingUnReliable();
				ackMsg.CreateAcknowledgement(m_pPlayerManager->LocalPrimaryJoinId(false), hdr.m_PacketSequence , false);
				m_pRouter->SendMessage(ackMsg, addr);
				return true;
			}
		}

		C_NetPlayer *pPlr =m_pPlayerManager->FindFromJoinId(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false);
		if (!pPlr)
			return false;

		m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);

		// reliable messages should only be sent by the primary player on each console.
		assert(pPlr->m_LocalIndex != 0);

		// add reliable message to inbox for later processing, acknowlegement will be sent internally.
		if (pPlr->m_pInBox)
			pPlr->m_pInBox->Add(msg, addr, m_pPlayerManager->LocalPrimaryJoinId(hdr.m_PartyChannel ? true : false));

		return true;
	}

	bool C_NetServices::ReceiveNetMessage(const DatagramAcknowledgementHeader& hdr, bdAddrHandleRef& addr)
	{
		if (!m_Initialised)
			return false;

		C_NetPlayer *pPlr = m_pPlayerManager->FindFromJoinId(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false);
		if (!pPlr)
			return false;

		m_pPlayerManager->UpdateKeepAliveForPlayer(hdr.m_SenderReference, hdr.m_PartyChannel ? true : false, addr);

		// reliable messages should only be sent by the primary player on each console.
		assert(pPlr->m_LocalIndex != 0);

		// acknowledge message in outbox if valid.
		if (pPlr->m_pOutBox)
			pPlr->m_pOutBox->ReceiveAcknowledgement(hdr);

		// return true to say packet has been processed. router won't pass packet on to any other listener.
		return true;
	}

	void C_NetServices::Poll(void)
	{
		if (!m_Initialised)
			return;

		F32 frameTimeInSeconds = net30fps;
		if (m_pFrameTimer->IsActive())
			frameTimeInSeconds = m_pFrameTimer->ElapsedTimeInSeconds();
		m_pFrameTimer->Start();

		// poll session deletion.
		NetPlay::C_SessionDeletion::Instance().Poll();

		// poll socket router for incomming datagrams.
		m_pRouter->Poll();

		// poll players connections and outgoing message queues.
		// send any keep alives if required.
		m_pPlayerManager->Poll(frameTimeInSeconds, *m_pRouter);

		// poll incoming reliable message queues and pass to required destination session..
		bool restart = false;
		do
		{	
			restart = false;
			for (S32 i=0; restart == false && i<m_pPlayerManager->TotalSize(); i++)
			{
				C_NetPlayer* pPlr = m_pPlayerManager->Index(i);
				if (pPlr && !pPlr->LocalPlayer())
				{
					if (!pPlr->LocalPlayer() && pPlr->PrimaryPlayer())
					{
						// remote primary players should have in / out boxes..
						assert(pPlr->m_pInBox);
						assert(pPlr->m_pOutBox);

						U8 msgType = 0;
						bool partySession = false;
						C_NetMessage& tempMsg = m_pRouter->GetReceiveMessage();
						if (pPlr->m_pInBox->Poll(tempMsg, msgType, partySession))
						{
							if (partySession)
								m_pNetParty->ReceiveReliable(tempMsg, msgType, pPlr->m_PartyConnection.m_JoinId);
							else
								m_pNetRace->ReceiveReliable(tempMsg, msgType, pPlr->m_RaceConnection.m_JoinId);
								
							restart = true;
						}
					}
				}
			}
		}while(restart);

		// poll party and race controllers.
		m_pNetParty->Poll(frameTimeInSeconds);
		m_pNetRace->Poll(frameTimeInSeconds);
	}

	void C_NetServices::CommunicatorMuteInformationChanged(void)
	{
		if (!m_Initialised)
			return;

		// ascertain what players are actually muted by the system.
		Communicator::C_Core::Instance().MuteUpdate();

		LOGWARN(g_Log, "C_NetServices::CommunicatorMuteInformationChanged() - Not propagating this change to the sessions yet!.");
	}

	bool C_NetServices::CreateRouter(void)
	{
		if (NULL != m_pRouter)
			return true;

#if defined(PLATFORM_XENON)
		m_pRouter = new C_NetRouterXenon;
#elif defined (PLATFORM_PS3)
		m_pRouter = new C_NetRouterPS3;
#elif defined (PLATFORM_PC)
		m_pRouter = new C_NetRouterPC;
#endif
		
		if (m_pRouter)
		{
			if (m_pRouter->Initialise(1024*3)) // maximum we can send in a single reliable packet.
			{
				m_pRouter->RegisterListener(this);
				return true;
			}
			else
			{
				delete m_pRouter;
				m_pRouter = NULL;
			}
		}
		else
		{
			// failed to create router..
			assert(false);
		}

		return false;
	}

	bool C_NetServices::CreatePlayerManager(void)
	{
		assert(NULL == m_pPlayerManager);

		bool ok = false;
		m_pPlayerManager = new C_NetPlayerManager;

		if (m_pPlayerManager)
			ok = m_pPlayerManager->Initialise();

		return ok;
	}

	bool C_NetServices::CreateScoreboardReaders(void)
	{
		assert(NULL == m_pScoreboardReaderByRank);
		assert(NULL == m_pScoreboardReaderByUsers);

		bool ok = false;

		m_pScoreboardReaderByRank = new Scoreboard::C_ReaderByRank;
		m_pScoreboardReaderByUsers = new Scoreboard::C_ReaderByUsers;

		if (m_pScoreboardReaderByRank && m_pScoreboardReaderByUsers)
		{
			ok = m_pScoreboardReaderByRank->Initialise();
			ok = ok && m_pScoreboardReaderByUsers->Initialise();
		}

		return ok;
	}

	bool C_NetServices::CreateScoreboardWriter(void)
	{
		assert(NULL == m_pScoreboardWriter);

		bool ok = false;
		m_pScoreboardWriter = new Scoreboard::C_Writer;
		if (m_pScoreboardWriter)
			ok = m_pScoreboardWriter->Initialise(3, 80); // on Xbox360 allocate 80 columns over 3 scoreboards, other platforms ignore column parameter.

		return ok;
	}

	bool C_NetServices::CreateRace(void)
	{
		assert(NULL == m_pNetRace);

		bool ok = false;
		m_pNetRace = new C_NetRace;
		if (m_pNetRace)
			ok = m_pNetRace->Initialise(&m_pRouter, &m_pPlayerManager);
		return ok;
	}

	bool C_NetServices::CreateParty(void)
	{
		assert(NULL == m_pNetParty);

		bool ok = false;
		m_pNetParty = new C_NetParty;
		if (m_pNetParty)
			ok = m_pNetParty->Initialise(&m_pRouter, &m_pPlayerManager);
		return ok;
	}

	bool C_NetServices::CreateMisc(void)
	{
		assert(NULL == m_pFrameTimer);

		bool ok = false;
		
		m_pFrameTimer = new C_NetTimer;
		ok = m_pFrameTimer != NULL ? true : false;

		return ok;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
