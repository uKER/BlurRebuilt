#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include <network/CommunicatorManager.h>
#include <Systemprofile/SystemProfile.h>
#include "../Misc/NetDefines.h"
#include "../Misc/NetDebug.h"
#include "NetPlayerManager.h"
#include "../Core/NetTimer.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Message/NetMessage.h"
#include "../Router/NetRouterBase.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#include <SystemProfile/SystemProfileManager.h>

#include <Ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <GameApp.h>
#include <Game/Game.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif
	
namespace RacerNet
{
	// how long do we still show  player as speaking after the decoding stops.
	const float NET_SPEAKING_UI_DELAY	=	(0.3f);

	C_NetPlayerManager::C_NetPlayerManager() :	m_Initialised(false), m_VoiceControlState(eVoiceControlStateNone), m_VoiceSendTimer(0.0f),
												m_PartyJoinReferenceCounter(0), m_RaceJoinReferenceCounter(0)
												
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

		// setup voice
		for (S32 i=0; i<m_Voice.Size(); i++)
		{
			for (S32 z=0; z<m_Voice[i].Size(); z++)
				ok == ok && m_Voice[i][z].Initialise(NET_VOICE_BUFFER_SIZE);
		}

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
		NetLocalVoiceArray* pVoice = NULL;
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

			// voice
			for (S32 i=0; NULL==pVoice && i<m_Voice.Size(); i++)
			{
				if (m_Voice[i][0].IsAvailable())
				{
					pVoice = &m_Voice[i];

					for (S32 z=0; z<pVoice->Size(); z++)
						(*pVoice)[z].SetAvailable(false);
				}
			}

			if (pIn && pOut && pVoice)
			{
				pPlr->m_pVoice = pVoice;
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
				if (pVoice)
				{
					for (S32 i=0; i<pVoice->Size(); i++)
						(*pVoice)[i].SetAvailable(true);
				}

				m_PlayerPool.Free(pPlr);

				return NULL;
			}
		}
		else
		{
			m_PlayerList.PushBack(*pPlr);
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
		BIZ_ASSERT(0 == m_PlayerList.Size());

		// remove all list entries and give back to pool..
		while (m_PlayerList.Size())
		{
			NetPlayerList::Iterator i = m_PlayerList.Begin();
			m_PlayerList.Erase(i);
			m_PlayerPool.Free(&(*i));
		}

		// destroy voice
		for (S32 i=0; i<m_Voice.Size(); i++)
		{
			for (S32 z=0; z<m_Voice[i].Size(); z++)
				m_Voice[i][z].Shutdown();
		}

		// destroy inboxes.
		for (S32 i=0; i<m_InBoxes.Size(); i++)
			m_InBoxes[i].Shutdown();

		// destroy outboxes.
		for (S32 i=0; i<m_OutBoxes.Size(); i++)
			m_OutBoxes[i].Shutdown();
			
		m_PartyJoinReferenceCounter = 0;
		m_RaceJoinReferenceCounter = 0;
		m_VoiceControlState = eVoiceControlStateNone;
		m_Initialised = false;
	}

	void C_NetPlayerManager::Flush(const bool partySession)
	{
		if (!m_Initialised)
			return;

		if (partySession)
		{
			m_PartyJoinReferenceCounter = 0;
		}
		else
		{
			m_RaceJoinReferenceCounter = 0;
		}
	}

	bool C_NetPlayerManager::AnyPlayersHandshaking(const bool partySession)
	{
		if (!m_Initialised)
			return false;
		
		U8 numHandshaking = 0;
		NetPlayerList::ConstIterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;
			const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->IsActive())
				numHandshaking += (pConn->m_HandshakingComplete == false) ? 1 : 0;
		}

		return numHandshaking > 0 ? true : false;
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
					return pConn->m_JoinId;
			}
		}
		LOGWARN(g_Log, "C_NetPlayerManager::LocalPrimaryJoinId() - no local players defined.");
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

		switch (m_VoiceControlState)
		{
		case eVoiceControlStateNone:
			m_VoiceSendTimer = 0;
			break;

		case eVoiceControlStateGamePlay:
			m_VoiceSendTimer = NET_DEFAULT_VOICE_SEND_TIME_IN_SECONDS; // by doing thsi we will send a single standalone voice updatewhen leaving game play.
			break;

		default:
			m_VoiceSendTimer += frameTimeInSeconds;
			break;
		}

		Communicator::C_Core& voiceComms = Communicator::C_Core::Instance();
		U8 voiceBuffer[200];
		U32 encodedSize = 0;	

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
			{
				if (eVoiceControlStateNone != m_VoiceControlState)
				{
					// read any voice that the comminicators have cached and assign it to the remote players
					if (voiceComms.IsLocalTalkerRegistered(it->m_ControllerPort))
					{
						// is local player speaking.
						if (voiceComms.IsLocalTalking(it->m_ControllerPort))
							it->m_IsSpeaking = NET_SPEAKING_UI_DELAY;
						else
						{
							it->m_IsSpeaking -= frameTimeInSeconds;
							if (it->m_IsSpeaking <= 0.0f)
								it->m_IsSpeaking -= 0.0f;
						}

						encodedSize = 0;
						if (voiceComms.GetLocalVoiceData(it->m_ControllerPort, voiceBuffer, sizeof(voiceBuffer), encodedSize))
						{
							if (encodedSize > 0)
							{
								//which remote players is this local player sending voice to.
								NetPlayerList::Iterator ir;
								for (ir = m_PlayerList.Begin(); ir != m_PlayerList.End(); ++ir)
								{
									// xbox 360 only really. if in NXE party chat with this remote player dont waste bandwidth, as guide will tranmitt voice for us.
									if (!SysProfile::C_Manager::Instance().IsInExternalChat(it->m_User, ir->m_User)) 
									{
										if (!ir->LocalPlayer() && ir->PrimaryPlayer())
										{
											// is local player flagged to send voice to this primary player.
											if (ir->m_VoiceEncodeArray.GetBitValue(it->m_LocalIndex))
											{
												// id remote player quick muted by local player, if so ignore this encoded voice data.
												if ( ir->m_QuickMute & (1<<it->m_LocalIndex))
													continue;

												if (!ir->AppendOutgoingVoice(it->m_LocalIndex, voiceBuffer, encodedSize))
												{
													LOGWARN(g_Log, "<Voice> %S -> %S : Not enough space in buffer to copy new voice data", it->m_User.Name(), ir->m_User.Name());
												}
											}
										}
									}
								}
							}
						}
					}
				}

				// no other logic for local players
				continue;
			}

			// is remote player talking
			if (!it->LocalPlayer() && !it->m_VoiceBanned && voiceComms.IsRemoteTalkerRegistered(it->m_User))
			{
				if (voiceComms.IsRemoteTalking(it->m_User))
					it->m_IsSpeaking = NET_SPEAKING_UI_DELAY;
				else
				{
					it->m_IsSpeaking -= frameTimeInSeconds;
					if (it->m_IsSpeaking <= 0.0f)
						it->m_IsSpeaking = 0.0f;
				}
			}

			// pump outbox for sending any queued up reliable data.
			if (it->m_pOutBox)
				it->m_pOutBox->Poll(it->m_PartyConnection.m_Address, it->m_RaceConnection.m_Address, router);

			// if primary remote player check if we need to send a keepalive.
			// this will only be sent after a period of time if no other unreliable data has been sent lately.
			if (it->PrimaryPlayer())
			{
				for (U8 a=0; a<2; a++)
				{
					const bool partyConnection = (a == 0) ? true : false;
					const U8 localPrimaryJoinId = LocalPrimaryJoinId(partyConnection);
					if (0 == localPrimaryJoinId)
						continue;

					C_NetPlayerConnection *pConn = partyConnection ? &(it->m_PartyConnection) : &(it->m_RaceConnection);

					// only start sending heartbeats once connection is complete and handshaking is done.
					if (pConn->CanMessage() && 0 != pConn->m_JoinId)
					{
						pConn->m_TimeSinceLastSentUnReliable += frameTimeInSeconds;

						if (pConn->m_TimeSinceLastSentUnReliable >= pConn->m_KeepAliveFrequency)
						{
							// send Keepalive packet..
							C_NetMessage& msg = router.GetOutgoingUnReliable();
							msg.CreateUnReliable(eNetMessage_KeepAlive, localPrimaryJoinId, partyConnection); 
							msg.TerminateMessage();

							it->SendUnReliable(msg, partyConnection, router);

							pConn->m_TimeSinceLastSentUnReliable = 0.0f;

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
			if (pConn->IsActive())
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

	C_NetPlayer* C_NetPlayerManager::HostPlayer(const bool partySession)
	{
		if (!m_Initialised)
			return NULL;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->IsActive() && pConn->m_Host)
				return &(*it);
		}

		return NULL;
	}

	void C_NetPlayerManager::PromoteToHost(const U8 joinId, const bool partySession)
	{
		BIZ_ASSERT(0 != joinId);
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->IsActive())
			{
				if (joinId == pConn->m_JoinId)
					pConn->m_Host = true;
				else
				{
					// flag old host to disconnect. then clear the host flag from this connection.
					if (pConn->m_Host)
					{
						if (eNetPlayerDisconnectionTypeNone == pConn->m_DisconnectRequested)
							pConn->m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
					}
					pConn->m_Host = false;
				}
			}
		}

		// validation to make sure a host exists after migration.
		C_NetPlayer* pHost = HostPlayer(partySession);
		BIZ_ASSERT(pHost != NULL);

		if (pHost && pHost->LocalPlayer())
		{
			// find highest join ref in the selected session and update our generator to use that new values.
			// new players joining the session whould get higher join references than currently joined players this way.
			if (partySession)
			{
				U8 highest = m_PartyJoinReferenceCounter;
				NetPlayerList::Iterator it;
				for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
				{
					if (it->m_PartyConnection.IsActive() && it->m_PartyConnection.m_JoinId > highest)
						highest = it->m_PartyConnection.m_JoinId;
				}
				m_PartyJoinReferenceCounter = highest;
			}
			else
			{
				U8 highest = m_RaceJoinReferenceCounter;
				NetPlayerList::Iterator it;
				for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
				{
					if (it->m_RaceConnection.IsActive() && it->m_RaceConnection.m_JoinId > highest)
						highest = it->m_RaceConnection.m_JoinId;
				}
				m_RaceJoinReferenceCounter = highest;

				//Data gathering time
				bdOnlineUserID userID;
				SysProfile::C_Manager::Instance().GetDemonwareUserId(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), userID);

				Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				if(pLayer)
				{
					pLayer->GetDataGathering().FlagPostEvent(Game::DATA_GATHERING_RECORD_TYPE_HOST_MIGRATED);
				}
			}
		}
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
			{
				pConn->m_TimeSinceLastReceivedKeepAlive.Start();
			}
		}
	}

	void C_NetPlayerManager::ReceivePlayerStatesMessage(C_NetMessage& msg, const U8 senderId, const bool partySession)
	{
		if (!m_Initialised)
			return;

		bool ok = false;
		if (msg.ReadOpen())
		{
			U8 num;
			ok = msg.ReadU8(num);
			if (ok && num <= NET_MAX_VEHICLES)
			{
				U8 joinId, state;
				ok = ok && msg.ReadU8(joinId);
				ok = ok && msg.ReadU8(state);

				if (!ok)
				{
					LOGWARN(g_Log, "C_NetPlayerManager::ReceivePlayerStatesMessage() - unable to read message data.");
					msg.ReadClose();
					return;
				}
				else
				{
					C_NetPlayer* p = FindFromJoinId(joinId, partySession);
					if (p)
					{
						C_NetPlayerConnection& connection = partySession ? (p->m_PartyConnection) : (p->m_RaceConnection);
						connection.m_PlayerState = static_cast<eNetPlayerState> (state);
						S32 plop = 0;
					}
				}
			}
			else
			{
				LOGWARN(g_Log, "C_NetPlayerManager::ReceivePlayerStatesMessage() - too many players states in message.");
			}
			msg.ReadClose();
		}

		return;
	}

	void C_NetPlayerManager::SetVoiceControlState(const eVoiceControlState state)
	{
		if (m_Initialised)
		{
			m_VoiceControlState = state;
			if (eVoiceControlStateNone == m_VoiceControlState)
				m_VoiceSendTimer = 0.0f;
		}
	}

	bool C_NetPlayerManager::VoiceTransferRequired(const float maxTime)
	{
		if (!m_Initialised || eVoiceControlStateNone == m_VoiceControlState)
			return false;

		if (m_VoiceSendTimer >= maxTime)
		{
			m_VoiceSendTimer = m_VoiceSendTimer - maxTime;
			return true;
		}
		return false;

	}

	void C_NetPlayerManager::SendBasicVoiceMessage(C_NetMessage& msg, C_NetRouterBase& router)
	{
		if (!m_Initialised || eVoiceControlStateNone == m_VoiceControlState)
			return;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer() || !it->PrimaryPlayer())
			{
				it->FlushOutgoingVoice(); // just incase the game logic put some voice into the wrong player type !.
				continue;
			}

			if (eVoiceControlStatePartyConnection == m_VoiceControlState)
			{
				if (it->m_PartyConnection.CanMessage() && it->m_PartyConnection.m_PlayerState >= eNetPlayerStateIdle)
				{
					SendBasicVoiceMessageToPlayer(msg, router, it->m_PartyConnection.m_JoinId, true);
				}
			}
			else if (eVoiceControlStateRaceConnection == m_VoiceControlState)
			{
				if (it->m_RaceConnection.CanMessage() && it->m_RaceConnection.m_PlayerState >= eNetPlayerStateIdle)
				{
					SendBasicVoiceMessageToPlayer(msg, router, it->m_RaceConnection.m_JoinId, false);
				}
			}

			it->FlushOutgoingVoice();
		}
	}

	void C_NetPlayerManager::SendBasicVoiceMessageToPlayer(C_NetMessage& msg, C_NetRouterBase& router, const U8 joinId, const bool partySession)
	{
		if (!m_Initialised)
			return;

		C_NetPlayer* p = FindFromJoinId(joinId, partySession);
		if (p)
		{
			if (!p->LocalPlayer() && p->PrimaryPlayer())
			{
				if (p->VoiceSize() > 0)
				{
					// send on party or race channel ?.
					C_NetPlayerConnection *pConn = NULL;
					if (eVoiceControlStatePartyConnection == m_VoiceControlState)
					{
						if (p->m_PartyConnection.CanMessage() && p->m_PartyConnection.m_PlayerState >= eNetPlayerStateIdle)
						{
							pConn = &(p->m_PartyConnection);
						}
					}
					else if (eVoiceControlStateRaceConnection == m_VoiceControlState || eVoiceControlStateGamePlay == m_VoiceControlState)
					{
						if (p->m_RaceConnection.CanMessage() && p->m_RaceConnection.m_PlayerState >= eNetPlayerStateIdle)
						{
							pConn = &(p->m_RaceConnection);
						}
					}

					// valid connection means we can attempt to send voice data
					if (pConn)
					{
						msg.Flush();
						msg.CreateUnReliable(eNetMessage_BasicVoice, LocalPrimaryJoinId(partySession), partySession);

						// how many bytes do we have for each local player
						NetLocalVoiceSizes localVoiceSize;
						for (S32 i=0; i<localVoiceSize.Size(); i++)
						{
							const U32 size = p->VoiceSize(i);
							BIZ_ASSERT(size <= 255); // only going to transmit this as U8 so can't go above 255 bytes per local player;
							localVoiceSize[i] = static_cast<U8> (size);

						}

						//LOGDEBUG(g_Log, "<Voice Out> [%d][%d]", localVoiceSize[0], localVoiceSize[1]);

						// write number of voice bytes per local player.
						for (S32 i=0; i<localVoiceSize.Size(); i++)
							msg.WriteU8(localVoiceSize[i]);

						// start unencrypted voice data.
						msg.StartUnEncryptedData();

						// copy buffered voice to message.
						p->WriteOutgoingVoiceToMessage(msg);

						msg.TerminateMessage();
						p->SendUnReliable(msg, partySession, router); 
					}
				}
			}
			else
			{
				LOGERROR(g_Log, "Trying to send voice to an invalid player.");
			}

			p->FlushOutgoingVoice();
		}
	}

	bool C_NetPlayerManager::CheckLowestVoicePrivileges()
	{
		if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() < 0 ) 
			return false;

		// check voice privilege for all connected profiles.
		SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
		bool voicePrivilege = (!profileMe.m_CommunicationsFriendsOnlyPrivilege & profileMe.m_CommunicationsPrivilege);		

		for( int i = 0; i < MAX_USERS; i++ )
		{
			SysProfile::FullProfile& profileOther = UI::C_ProfileInterface::Instance().GetPadProfile( i );
			if( profileOther.m_State == SysProfile::eProfileStateNotActive ) 
				continue;

			voicePrivilege &= (!profileOther.m_CommunicationsFriendsOnlyPrivilege & profileOther.m_CommunicationsPrivilege);
		}
		return voicePrivilege;
	}

	void C_NetPlayerManager::ReceiveBasicVoiceMessage(C_NetMessage& msg, const U8 senderId, const bool partySession)
	{
		if (!m_Initialised)
			return;

		//----------------------------------------------------
		// TCR #093 : bug 29766, 29767, 29744, 29745
		//----------------------------------------------------
		// most restrictive 
		if(!CheckLowestVoicePrivileges())
			return;

		C_NetPlayer* pSender = FindFromJoinId(senderId, partySession);
		if (NULL == pSender)
			return; // remote player no longer in our session.

		const U64 remoteConsoleId = pSender->m_ConsoleId;

		bool ok = true;
		if (msg.ReadOpen())
		{
			U8 data[NET_VOICE_BUFFER_SIZE];

			// read how much voice data we expect for each remote peer.
			NetLocalVoiceSizes localVoiceSize;
			for (S32 i=0; i<localVoiceSize.Size(); i++)
				ok = ok && msg.ReadU8(localVoiceSize[i]);

			//LOGDEBUG(g_Log, "<Voice In> [%d][%d]", localVoiceSize[0], localVoiceSize[1]);

			if (ok)
			{
				for (S32 i=0; i<localVoiceSize.Size() && ok; i++)
				{
					if (localVoiceSize[i] > 0)
					{
						// extract voice from msg
						ok = ok && msg.ReadBuffer(data, localVoiceSize[i]);

						if (ok)
						{
							C_NetPlayer* rt = FindFromConsoleId(remoteConsoleId, i);
							if (rt)
							{
								if (0 == rt->m_QuickMute) // TODO -> this wont work if we have more than 1 local talker. need a different way to handle quick muting in that case.
									Communicator::C_Core::Instance().SubmitRemoteVoiceData(rt->m_User, data, localVoiceSize[i]);
							}
							else
								ok = false;
						}
					}
				}
			}

			msg.ReadClose();
		}

		if (!ok)
		{
			LOGWARN(g_Log, "C_NetPlayerManager::ReceiveBasicVoiceMessage() - unable to read message data.");
		}
	}

	void C_NetPlayerManager::BuildMuteList(const U8 localNetIndex, NetMuteListJoinReferences& out, const bool partySession)
	{
		out.Clear();
		if (!m_Initialised)
			return;

		if (LocalSize() < localNetIndex)
		{
			LOGERROR(g_Log, "C_NetPlayerManager::BuildMuteList() - localIndex out of range.");
			return;
		}

		C_NetPlayer* local = Index(localNetIndex);
		BIZ_ASSERT(local != NULL);
		const S8 controllerPort = local->m_ControllerPort;

		Communicator::C_Core& voiceComms = Communicator::C_Core::Instance();
		if (!voiceComms.IsLocalTalkerRegistered(controllerPort))
			return;
	
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;

			const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
			if (pConn->IsActive() && pConn->m_JoinId > 0)
			{
				if (voiceComms.LocalMuted(controllerPort, it->m_User))
					out.PushBack(pConn->m_JoinId); // this local player currently has this remote player muted.
			}
		}
	}

	void C_NetPlayerManager::ReceiveMuteList(const U8 senderJoinId, const NetMuteListJoinReferences& in, const bool partySession)
	{
		if (!m_Initialised)
			return;
		
		Communicator::C_Core& voiceComms = Communicator::C_Core::Instance();

		C_NetPlayer* sender = FindFromJoinId(senderJoinId, partySession);
		if (NULL == sender)
			return;

		if (!voiceComms.IsRemoteTalkerRegistered(sender->m_User))
			return;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer() && voiceComms.IsLocalTalkerRegistered(it->m_ControllerPort))
			{
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);

				bool isInMuteList = false;
				for (S32 i=0; i<in.Size() && !isInMuteList; i++)
				{
					if (in[i] == pConn->m_JoinId)
						isInMuteList = true;
				}

				voiceComms.SetMutedRemotely(sender->m_User, it->m_ControllerPort, isInMuteList, true);
			}
		}
	}

	void C_NetPlayerManager::SetPeersToMigrationPhase(const bool partySession)
	{
		if (!m_Initialised)
			return;

		C_NetPlayer* pHost = HostPlayer(partySession);
		if (NULL == pHost || !pHost->LocalPlayer())
		{
			LOGWARN(g_Log, "C_NetPlayerManager::SetPeersToMigrationPhase() - peers should not call this function !!");
		}

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;

			if (it->PrimaryPlayer())
			{
				C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (pConn->IsActive() && pConn->m_JoinId > 0 && eNetPlayerDisconnectionTypeNone == pConn->m_DisconnectRequested)
					pConn->m_WaitingForMigrationReply = true;
			}
		}
	}

	void C_NetPlayerManager::SetPeerMigrationComplete(const U8 joinId, const bool partySession)
	{
		if (!m_Initialised)
			return;
		
		C_NetPlayer* pHost = HostPlayer(partySession);
		if (NULL == pHost || !pHost->LocalPlayer())
		{
			LOGWARN(g_Log, "C_NetPlayerManager::SetPeerMigrationComplete() - peers should not call this function !!");
		}

		C_NetPlayer* p = FindFromJoinId(joinId, partySession);
		if (p)
		{
			C_NetPlayerConnection *pConn = partySession ? &(p->m_PartyConnection) : &(p->m_RaceConnection);
			pConn->m_WaitingForMigrationReply = false;
		}
	}

	bool C_NetPlayerManager::AnyPeersStillMigrating(const bool partySession)
	{
		if (!m_Initialised)
			return false;

		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;

			if (it->PrimaryPlayer())
			{
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (pConn->IsActive() && pConn->m_JoinId > 0 && eNetPlayerDisconnectionTypeNone == pConn->m_DisconnectRequested)
				{
					if (pConn->m_WaitingForMigrationReply == true)
						return true;
				}
			}
		}

		return false;
	}

	void C_NetPlayerManager::PeerMigrationTimeout(const bool partySession)
	{
		if (!m_Initialised)
			return;

		C_NetPlayer* pHost = HostPlayer(partySession);
		if (NULL == pHost || !pHost->LocalPlayer())
		{
			LOGWARN(g_Log, "C_NetPlayerManager::PeerMigrationTimeout() - peers should not call this function !!");
		}

		NetConsoleIds toDisconnect;
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->LocalPlayer())
				continue;

			if (it->PrimaryPlayer())
			{
				const C_NetPlayerConnection *pConn = partySession ? &(it->m_PartyConnection) : &(it->m_RaceConnection);
				if (pConn->m_WaitingForMigrationReply)
					toDisconnect.PushBack(it->m_ConsoleId);
			}
		}

		for (S32 i=0; i<toDisconnect.Size(); i++)
		{
			for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
			{
				if (toDisconnect[i] == it->m_ConsoleId)
				{
					if (it->m_RaceConnection.IsActive() && it->m_RaceConnection.m_DisconnectRequested == eNetPlayerDisconnectionTypeNone)
						it->m_RaceConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;

					if (it->m_PartyConnection.IsActive() && it->m_PartyConnection.m_DisconnectRequested == eNetPlayerDisconnectionTypeNone)
						it->m_PartyConnection.m_DisconnectRequested = eNetPlayerDisconnectionTypeLostConnection;
				}
			}
		}
	}

	void C_NetPlayerManager::QuickMuteToggle(const U8 controllerIndex, const U8 remoteJoinReference, const bool partySession)
	{
		if (!m_Initialised)
			return;

		C_NetPlayer *pLocal = NULL;
		for (U8 i=0; i<LocalSize() && NULL == pLocal; i++)
		{
			C_NetPlayer* p = Index(i);
			if (p->LocalPlayer() && controllerIndex == p->m_ControllerPort)
				pLocal = p;
		}

		if (NULL == pLocal)
		{
			LOGWARN(g_Log, "C_NetPlayerManager::QuickMuteRemotePlayer() - requested local player not found");
			return;
		}

		C_NetPlayer* pRemote = FindFromJoinId(remoteJoinReference, partySession);
		if (NULL == pRemote)
		{
			LOGWARN(g_Log, "C_NetPlayerManager::QuickMuteRemotePlayer() - requested remote player not found");
			return;
		}

		// dont wory about local players or players that are voice banned.
		if (!pRemote->CanQuickMute())
			return; 

		// toggle current quick mute status.
		if (pRemote->m_QuickMute&(1<<pLocal->m_LocalIndex))
			pRemote->m_QuickMute &= ~(1<<pLocal->m_LocalIndex);
		else
			pRemote->m_QuickMute |= (1<<pLocal->m_LocalIndex);
	}

	eNatType C_NetPlayerManager::GetMostRestrictiveNatType(const bool partySession)
	{
		if (!m_Initialised)
			return eNatOpen;

		U8 t = eNatOpen;
		NetPlayerList::Iterator it;
		for (it = m_PlayerList.Begin(); it != m_PlayerList.End(); ++it)
		{
			if (it->m_ProfileData.m_NatType > t)
				t = it->m_ProfileData.m_NatType;
		}

		switch (t)
		{
		case eNatOpen:
			return eNatOpen;
			break;
		case eNatModerate:
			return eNatModerate;
			break;
		case eNatStrict:
			return eNatStrict;
			break;
		}

		BIZ_ASSERT_FAIL("C_NetPlayerManager::GetMostRestrictiveNatType() - invalid nat type detected");
		return eNatOpen;
	}

	bool C_NetPlayerManager::CheckAgainstMostRestrictiveNat(const bool partySession, const U8 joiningNatType, SysProfile::BaseProfile& out)
	{
		if (!m_Initialised)
			return false;

		const eNatType mostRestrictiveActiveNat = GetMostRestrictiveNatType(partySession);

		switch (joiningNatType)
		{
		case eNatOpen:
			return true;	// joining player should be possible to communicate with anybody.
			break;

		case eNatModerate:
			if (mostRestrictiveActiveNat > eNatModerate)
			{
				bool done = false;
				NetPlayerList::Iterator it;
				for (it = m_PlayerList.Begin(); it != m_PlayerList.End() && !done; ++it)
				{
					if (it->m_ProfileData.m_NatType > eNatModerate)
					{
						out = it->m_User;
						done = true;
					}
				}
				return false;
			}
			break;

		case eNatStrict:
			if (mostRestrictiveActiveNat != eNatOpen)
			{
				bool done = false;
				NetPlayerList::Iterator it;
				for (it = m_PlayerList.Begin(); it != m_PlayerList.End() && !done; ++it)
				{
					if (it->m_ProfileData.m_NatType != eNatOpen)
					{
						out = it->m_User;
						done = true;
					}
				}
				return false;
			}
			break;

		default:
			BIZ_ASSERT_FAIL("C_NetPlayerManager::CheckAgainstMostRestrictiveNat() - invalid nat type of joining peer");
			break;
		}

		return true;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
