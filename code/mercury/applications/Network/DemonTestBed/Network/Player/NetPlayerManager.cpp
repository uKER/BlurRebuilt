#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include "../Misc/NetDebug.h"
#include "NetPlayerManager.h"
#include "../Core/NetTimer.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Message/NetMessage.h"
#include "../Router/NetRouterBase.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetPlayerManager::C_NetPlayerManager() : m_Initialised(false), m_PartyJoinReferenceCounter(0), m_RaceJoinReferenceCounter(0)
	{
	}

	C_NetPlayerManager::~C_NetPlayerManager()
	{
		Shutdown();
	}

	bool C_NetPlayerManager::Initialise(void)
	{
		if (m_Initialised)
			return true;

		bool ok = true;

		// setup outboxes
		for (S32 i=0; i<m_OutBoxes.Size(); i++)
			ok = ok && m_OutBoxes[i].Initialise(NET_MAX_OUTBOX_SIZE);

		// setup inboxes
		for (S32 i=0; i<m_InBoxes.Size(); i++)
			ok = ok && m_InBoxes[i].Initialise(NET_MAX_INBOX_SIZE);

		if (ok)
			m_Initialised = true;
		else
			Shutdown();

		return ok;
	}

	C_NetPlayer* C_NetPlayerManager::AllocatePlayer(bool requiresMessageQueue)
	{
		if (!m_Initialised)
			return NULL;

		if (m_PlayerPool.Remaining() == 0)
		{
			LOGWARN(g_Log, "C_NetPlayerManager::Allocate() - ran out of pre allocated player slots");
			return NULL;
		}

		C_NetPlayer* pPlr = m_PlayerPool.Allocate();
		if (NULL == pPlr)
			return NULL;

		C_NetMessageReliableInBox* pIn = NULL;
		C_NetMessageReliableOutBox* pOut = NULL;
		if (requiresMessageQueue)
		{
			// inbox
			for (S32 i=0; NULL==pIn && i<m_InBoxes.Size(); i++)
			{
				if (m_InBoxes[i].IsAvailable())
				{
					pIn = &m_InBoxes[i];
					pIn->SetAvailable(false);
				}
			}

			// outbox
			for (S32 i=0; NULL==pOut && i<m_OutBoxes.Size(); i++)
			{
				if (m_OutBoxes[i].IsAvailable())
				{
					pOut = &m_OutBoxes[i];
					pOut->SetAvailable(false);
				}
			}

			if (pIn && pOut)
			{
				pPlr->m_pInBox = pIn;
				pPlr->m_pOutBox = pOut;

				m_PlayerList.PushBack(*pPlr);
			}
			else
			{
				// failed to allocate a message box so release what was allocated and fail
				if (pIn)
					pIn->SetAvailable(true);
				if (pOut)
					pOut->SetAvailable(true);

				m_PlayerPool.Free(pPlr);

				return NULL;
			}
		}

		return pPlr;
	}

	void C_NetPlayerManager::DeallocatePlayer(C_NetPlayer *pPlr)
	{
		if (!m_Initialised || NULL == pPlr)
			return;

		pPlr->m_RaceConnection.Disconnect(!pPlr->LocalPlayer() && pPlr->PrimaryPlayer());
		pPlr->m_PartyConnection.Disconnect(!pPlr->LocalPlayer() && pPlr->PrimaryPlayer());

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (&(*it) == pPlr)
			{
				m_PlayerList.Erase(it);
				m_PlayerPool.Free(&(*it));
				return;
			}
		}

		LOGERROR(g_Log, "C_NetPlayerManager::DeallocatePlayer() - player not found!");
	}

	void C_NetPlayerManager::Shutdown(void)
	{
		// the game should have un registed all registered network players before we get here
		// if not then their addresses will still be registered for communication.
		assert(0 == m_PlayerList.Size());

		// remove all list entries and give back to pool..
		while (m_PlayerList.Size())
		{
			NetPlayerList::Iterator i = m_PlayerList.Begin();
			m_PlayerList.Erase(i);
			m_PlayerPool.Free(&(*i));
		}

		// destroy inboxes.
		for (S32 i=0; i<m_InBoxes.Size(); i++)
			m_InBoxes[i].Shutdown();

		// destroy outboxes.
		for (S32 i=0; i<m_OutBoxes.Size(); i++)
			m_OutBoxes[i].Shutdown();
			
		m_Initialised = false;
	}

	U8 C_NetPlayerManager::LocalPrimaryJoinId(bool partySession) const
	{
		if (!m_Initialised)
			return 0;

		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
			{
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (0 == it->m_LocalIndex)
				{
					assert(pConn->m_JoinId); // join id of 0 should be invalid.. 
					return pConn->m_JoinId;
				}
			}
		}
		LOGWARN(g_Log, "C_NetPlayerManager::LocalPrimaryJoinId() - no join Id assigned yet.");
		return 0;
	}

	bool C_NetPlayerManager::ValidateLocalJoinId(const bool partySession) const
	{
		if (!m_Initialised)
			return false;
		
		U8 numLocal = 0;
		U8 numValid = 0;

		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
			{
				numLocal += 1;
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (pConn->m_JoinId != 0)
					numValid += 1;
			}
		}
		
		if (numLocal+numValid > 0 && numLocal == numValid)
			return true;

		return false;
	}

	void C_NetPlayerManager::Poll(const float frameTimeInSeconds, C_NetRouterBase& router)
	{
		if (!m_Initialised)
			return;

		// update remote players outboxes.
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;

			// pump outbox for sending any queued up reliable data.
			if (it->m_pOutBox)
				it->m_pOutBox->Poll(it->m_PartyConnection.m_Address, it->m_RaceConnection.m_Address);

			// if primary remote player check if we need to send a keepalive.
			// this will only be sent after a period of time if no other unreliable data has been sent lately.
			if (it->PrimaryPlayer())
			{
				for (U8 a=0; a<2; a++)
				{
					const bool partyConnection = (a == 0) ? true : false;
					const U8 localPrimaryJoinId = LocalPrimaryJoinId(partyConnection);
					C_NetPlayerConnection *pConn = (partyConnection) ? &(it->m_PartyConnection) : &(it->m_RaceConnection);

					if (pConn->IsConnected() && 0 != pConn->m_JoinId)
					{
						pConn->m_TimeSinceLastSentUnReliable += frameTimeInSeconds;

						if (pConn->m_TimeSinceLastSentUnReliable >= pConn->m_KeepAliveFrequency)
						{
							// send Keepalive packet..
							C_NetMessage& msg = router.GetOutgoingUnReliable();
							msg.CreateUnreliable(eNetMessageTypeKeepAlive, localPrimaryJoinId, partyConnection); 
							msg.TerminateMessage();

							it->SendUnReliable(msg, partyConnection, router);
						}
					}
				}
			}
		}
	}

	C_NetPlayerManager::eHostCheckState C_NetPlayerManager::CheckHost(const bool partySession) const
	{
		if (!m_Initialised)
			return eHostCheckStateNoneFound;

		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->m_Host)
				return it->LocalPlayer() ? eHostCheckStateLocal : eHostCheckStateRemote;
		}

		return eHostCheckStateNoneFound;
	}

	bool C_NetPlayerManager::ValidateHost(const U8 hostJoinId, const bool partySession) const
	{
		if (!m_Initialised || 0 == hostJoinId)
			return false;

		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->m_Host && hostJoinId == pConn->m_JoinId)
				return true;
		}

		return false;
	}

	U8 C_NetPlayerManager::GenerateJoinReference(const bool partySession)
	{
		if (!m_Initialised)
			return 0;

		// reference the correct session.
		U8& counter = partySession ? m_PartyJoinReferenceCounter : m_RaceJoinReferenceCounter;
	
		bool finished;
		do
		{
			finished = true;

			counter++;
			counter = counter == 0 ? 1 : counter;  // dont allow join reference index 0 this is invalid..
			counter = counter > 32 ? 1 : counter; // wrap at 32.

			// check current players to make sure this join ref is not in use..
			NetPlayerList::ConstIterator it;
			for (it = m_PlayerList.Begin(); finished && it != m_PlayerList.End(); ++it)
			{
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (counter == pConn->m_JoinId)
					finished = false;
			}
		}while(!finished);

		return counter;
	}

	U8 C_NetPlayerManager::Size(const bool partySession) const
	{
		if (!m_Initialised)
			return 0;	
		U8 count = 0;
		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (eNetPlayerStateUnSet != pConn->m_PlayerState)
				count += 1;
		}

		return count;
	}

	U8 C_NetPlayerManager::LocalSize(void) const
	{
		if (!m_Initialised)
			return 0;	
		U8 localCount = 0;
		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				localCount += 1;
		}
		return localCount;
	}

	C_NetPlayer* C_NetPlayerManager::Index(const U8 index)
	{
		if (!m_Initialised)
			return NULL;

		if (index >= TotalSize())
			return NULL;

		U8 count = 0;
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it, count++)
		{
			if (index == count)
				return &(*it);
		}

		return NULL;
	}

	C_NetPlayer* C_NetPlayerManager::FindFromJoinId(const U8 joinId, const bool partySession)
	{
		if (!m_Initialised)
			return NULL;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->m_JoinId == joinId)
				return &(*it);
		}

		return NULL;
	}

	C_NetPlayer* C_NetPlayerManager::FindFromConsoleId(const U64 consoleId, const U8 index)
	{
		if (!m_Initialised)
			return NULL;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (consoleId == it->m_ConsoleId && index == it->m_LocalIndex)
				return &(*it);
		}

		return NULL;
	}

	C_NetPlayer* C_NetPlayerManager::FindLocal(const U8 controllerIndex)
	{
		if (!m_Initialised)
			return NULL;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer() && controllerIndex == it->m_ControllerPort)
				return &(*it);
		}

		return NULL;
	}

	bool C_NetPlayerManager::ConsoleExists(const U64 consoleId) const
	{
		if (!m_Initialised)
			return false;

		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (consoleId == it->m_ConsoleId)
				return true;
		}

		return false;
	}

	void C_NetPlayerManager::UpdateKeepAliveForPlayer(const U8 joinId, const bool partySession, bdAddrHandleRef& latestAddress)
	{
		if (!m_Initialised)
			return;

		if (0 == joinId)
			return; // 0 is an invalid join id !!

		C_NetPlayer* p = FindFromJoinId(joinId, partySession);
		if (NULL == p)
			return;

		const U64 consoleId = p->m_ConsoleId;

		// update heartbeats for all players on the same console.
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->m_ConsoleId != consoleId)
				continue;
			
			C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);

			// if latest address is valid the update our connection info.
			if (latestAddress.notNull())
				pConn->m_Address = latestAddress;

			// update keepalive for specified connection.
			if (pConn->m_JoinId != 0)
				pConn->m_TimeSinceLastReceivedKeepAlive.Start();
		}
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
