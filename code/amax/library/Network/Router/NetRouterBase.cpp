#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <Network/BandwidthEvaluator.h>
#include <SystemProfile/GameInvite.h>
#include "../Misc/NetDefines.h"
#include "../Misc/NetDebug.h"
#include "NetRouterbase.h"
#include "NetAccountCreationListener.h"
#include "../Message/NetMessageListener.h"
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include "../Misc/NetEncrypted.h"

#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdStorage/bdStorage.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdTags/bdTags.h>
#include <bdLobby/bdVoteRank/bdVoteRank.h>
#include <bdLobby/bdMessaging/bdMessaging.h>
#include <bdLobby/bdFacebook/bdFacebook.h>
#include <bdLobby/bdTwitter/bdTwitter.h>
#include <bdLobby/bdLinkCode/bdLinkCode.h>
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterBase::C_NetRouterBase() :	m_BaseInitialised(false), m_ContentStreamingEnabled(false), m_PlatformConnectionState(eConnectionStateNotActive), 
											m_LastConnectionErrorCode(eConnectionResultNone), m_OnlineMode(false),
											m_pBandwidthEvaluator(NULL), m_BandwidthEvaluated(false),
											m_UpstreamBandwithKbps(NET_MINIMUM_BANDWIDTH_KBPS), m_DownstreamBandwithKbps(NET_MINIMUM_BANDWIDTH_KBPS),
											m_UsageOut(0.0f), m_UsageIn(0.0f), m_UsageSent(0.0f), m_UsageReceived(0.0f), m_UsageTimer(0.0f),
											m_MatchingRegion(eMatchingRegionUnknown), m_UseRelaySocketRouter(false)
	{
	}

	// destructor
	C_NetRouterBase::~C_NetRouterBase()
	{
		Shutdown();
	}

	bool C_NetRouterBase::Initialise(const U32 sendBufferSize)
	{
		if (m_BaseInitialised)
			return true;

		BIZ_ASSERT(sendBufferSize > 0);
		if (0 == sendBufferSize)
			return false;

		// configure send buffer size.
		const bool sendOk = m_SendMessage.Initialise(sendBufferSize);

		// configure reveive buffer size.
		const bool receiveOk = m_ReceiveMessage.Initialise(sendBufferSize); 

		// allocate game invite storage.
		m_pGameInvite = new SysProfile::GameInvite;

		if (sendOk && receiveOk && m_pGameInvite)
			m_BaseInitialised = true;
		else
			Shutdown();

		return m_BaseInitialised;
	}

	void C_NetRouterBase::Shutdown(void)
	{
		if (m_pBandwidthEvaluator)
		{
			m_pBandwidthEvaluator->Stop();
			delete m_pBandwidthEvaluator;
			m_pBandwidthEvaluator = NULL;
		}

		if (m_pGameInvite)
			delete m_pGameInvite;
		m_pGameInvite = NULL;

		m_ReceiveMessage.Destroy();
		m_SendMessage.Destroy();

		m_OnlineMode = false;
		m_PlatformConnectionState = eConnectionStateNotActive;
		m_LastConnectionErrorCode = eConnectionResultNone;
		m_BandwidthEvaluated = false;

		// clear message listener list.
		m_ReceiveListeners.Clear();

		m_BaseInitialised = false;
	}

	void C_NetRouterBase::Poll(const F32 frameTimeInSeconds)
	{
		PERF_EVENT("C_NetRouterBase::Poll");

		if (!m_BaseInitialised)
			return;

		// bandwidth usage
		m_UsageTimer += frameTimeInSeconds;
		if (m_UsageTimer >= 1.0f)
		{
			m_UsageTimer -= 1.0f;

			m_UsageIn = m_UsageReceived / 1024;
			m_UsageOut = m_UsageSent / 1024;		

			m_UsageSent = m_UsageReceived = 0.0f;
		}

		// poll lobby connection if required.
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			if (m_pBandwidthEvaluator)
			{
				m_pBandwidthEvaluator->Update();
				if (!m_pBandwidthEvaluator->IsActive())
				{
					U32 up, down;
					m_pBandwidthEvaluator->GetResults(up, down);

					// convert to kbps from bps
					up /= 1024;
					down /= 1024;

					StopBandwidthEvaluation();

					m_UpstreamBandwithKbps = (up < NET_MINIMUM_BANDWIDTH_KBPS) ? NET_MINIMUM_BANDWIDTH_KBPS : up;
					m_DownstreamBandwithKbps = (down < NET_MINIMUM_BANDWIDTH_KBPS) ? NET_MINIMUM_BANDWIDTH_KBPS : down;
					m_BandwidthEvaluated = true;

					LOGINFO(g_Log, "C_NetRouterBase::Poll() - Bandwidth evaluation complete, Up : %d kbps  Down %d kbps", m_UpstreamBandwithKbps, m_DownstreamBandwithKbps);
				}
			}

			bdSocketRouter* router =  net->getSocketRouter();
			if (router)
			{
				// poll socket router
				router->pump();

				// poll demonware misc services..
				// DON'T Run these demonware services. message transport goes belly up if you do.
				//net->receiveAll();
				//net->dispatchAll();
				//net->sendAll();

#ifndef CFG_GOLD
				// Profiling information to help diagnose stalls in the network code (though to be in Peer Relay)
				unsigned long	profile_NumReceiveFrom(0);
				unsigned long	profile_TotalBytesReceived(0);
				float			profile_TotalTimeInReceiveFrom(0.f);

				float			profile_LongestReceiveFrom(0.f);
				unsigned long	profile_BytesInLongestReceive(0);
#endif

				S32 bytesReceived = 0;
				do 
				{
					// flush data from last received net message.
					m_ReceiveMessage.Flush();
					
					bdAddrHandleRef addr = BD_NULL;

#ifndef CFG_GOLD
					float timeTakenByReceiveFromInMs;
					{
						PERF_EVENT("receiveFrom");

						Time::C_Time timeStartReceiveFrom = Time::GetPerformanceCounter();
#endif

						// receive from socket.
						bool bError = false;
						bytesReceived = router->receiveFrom(addr, m_ReceiveMessage.Data(), m_ReceiveMessage.MaxDataSize(), bError);

#ifndef CFG_GOLD
						timeTakenByReceiveFromInMs = (1000 * (Time::GetPerformanceCounter() - timeStartReceiveFrom)) / (float)Time::GetPerformanceFrequency();
					}

					unsigned long profileBytes = (bytesReceived > 0) ? bytesReceived : 0;
					profile_NumReceiveFrom++;
					profile_TotalBytesReceived += profileBytes;
					profile_TotalTimeInReceiveFrom += timeTakenByReceiveFromInMs;

					if (timeTakenByReceiveFromInMs > profile_LongestReceiveFrom)
					{
						profile_LongestReceiveFrom = timeTakenByReceiveFromInMs;
						profile_BytesInLongestReceive = profileBytes;
					}
#endif
					
					if (bytesReceived > 0)
					{
						// track bytes for bandwidth usage.
						TrackIncomingBytes(bytesReceived);

						const S32 minPacketSize = sizeof(DatagramPacketHeader);
						if (bytesReceived >= minPacketSize)
						{
							const DatagramPacketHeader* pDatagram = reinterpret_cast<DatagramPacketHeader*> (m_ReceiveMessage.Data());
							
							// configure receive message so we can open it for reading in the listeners..
							m_ReceiveMessage.m_CurrentBufferSize	= bytesReceived;	
							m_ReceiveMessage.m_HasHeader			= true;				
							m_ReceiveMessage.m_EncryptedBytesSet	= true;				
							m_ReceiveMessage.m_Reliable				= pDatagram->m_Reliable == 0 ? false : true;
							 
							if (pDatagram->m_Reliable)
							{
								// build a reliable message out of the incomming data and send to listeners
								const DatagramReliablePacketHeader * pReliableDatagram = reinterpret_cast<DatagramReliablePacketHeader*> (m_ReceiveMessage.Data());
								if (bytesReceived > sizeof(DatagramReliablePacketHeader))
								{
									bool accepted = false;
									for (S32 i=0; !accepted && i<m_ReceiveListeners.Size(); i++)
										accepted = m_ReceiveListeners[i]->ReceiveNetMessage(m_ReceiveMessage, *pReliableDatagram, addr);
								}
							}
							else
							{
								if (eNetMessage_ReliableAcknowlegement == pDatagram->m_PacketType)
								{
									// received an acknowledgement
									const DatagramAcknowledgementHeader *pAckDatagram = reinterpret_cast<DatagramAcknowledgementHeader*> (m_ReceiveMessage.Data());
									if (bytesReceived == sizeof(DatagramAcknowledgementHeader))
									{
										bool accepted = false;
										for (S32 i=0; !accepted && i<m_ReceiveListeners.Size(); i++)
											accepted = m_ReceiveListeners[i]->ReceiveNetMessage(*pAckDatagram, addr);
									}
								}
								else
								{
									// build a unreliable message out of the incomming data and send to listeners
									const DatagramUnReliablePacketHeader *pUnReliableDatagram = reinterpret_cast<DatagramUnReliablePacketHeader*> (m_ReceiveMessage.Data());
									if (bytesReceived >= sizeof(DatagramUnReliablePacketHeader))
									{
										bdUInt16 encryptedBytes = 0;

										// get the encrypted bytes size.
										EncryptedSize(encryptedBytes, pUnReliableDatagram->m_EncryptedBytes);
										
										// add the 2 bytes since this is in the data buffer
										encryptedBytes += sizeof(pUnReliableDatagram->m_EncryptedBytes); 

										S32 numUnEncrypted = m_ReceiveMessage.m_CurrentBufferSize - encryptedBytes;
										if (numUnEncrypted < 0)
											numUnEncrypted = 0;

										bool accepted = false;
										for (S32 i=0; !accepted && i<m_ReceiveListeners.Size(); i++)
											accepted = m_ReceiveListeners[i]->ReceiveNetMessage(m_ReceiveMessage, *pUnReliableDatagram, numUnEncrypted, addr);
									}
								}
							}
						 }
					}

				} while(bytesReceived > 0);

#ifndef CFG_GOLD
				// Profiling information added to PIX
				Perf::AddNamedCounter((F32)profile_NumReceiveFrom,			"NetRB - NumReceiveFrom");
				Perf::AddNamedCounter((F32)profile_TotalBytesReceived,		"NetRB - TotalBytes");
				Perf::AddNamedCounter((F32)profile_TotalTimeInReceiveFrom,	"NetRB - TotalTimeInMs");
				Perf::AddNamedCounter((F32)profile_LongestReceiveFrom,		"NetRB - LongestInMs");
				Perf::AddNamedCounter((F32)profile_BytesInLongestReceive,	"NetRB - LongestSize");
#endif
			}
		}
	}

	bool C_NetRouterBase::Disconnect(bdAddrHandleRef& addr)
	{
		if (!m_BaseInitialised || addr.isNull())
			return false;

		bdNetImpl *const net = bdNet::getInstance();
		if (net && net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdSocketRouter* socketRouter =  net->getSocketRouter();
			if (socketRouter)
			{
				socketRouter->disconnect(addr);
				return true;
			}
		}
		return false;
	}

	bool C_NetRouterBase::CreateOnlineAccount(char* userName, char *password, char* licenseCode, C_NetAccountCreationListener* pListener)
	{
		if (!m_BaseInitialised)
			return false;

		LOGERROR(NetPlay::g_Log, "C_NetRouterBase::CreateOnlineAccount() - Online account creation not possible on this platform!");
		return false;
	}

	bdContentStreaming*	C_NetRouterBase::GetContentStreamingServer(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		if (!m_ContentStreamingEnabled)
		{
			LOGWARN(RacerNet::g_Log, "C_NetRouterBase::GetContentStreamingServer() - called before content streaming has been enabled.");
			return NULL;
		}

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getContentStreaming();
	}

	bdKeyArchive* C_NetRouterBase::GetKeyArchive(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getKeyArchive();
	}


	bdTags* C_NetRouterBase::GetTags(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getTags();
	}

	bdStorage* C_NetRouterBase::GetStorage(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getStorage();
	}

	bdVoteRank* C_NetRouterBase::GetVoteRank(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getVoteRank();
	}

	bdMessaging* C_NetRouterBase::GetMessaging(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getMessaging();
	}

	bdFacebook* C_NetRouterBase::GetFacebook(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getFacebook();
	}

	bdTwitter* C_NetRouterBase::GetTwitter(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getTwitter();
	}

	bdLinkCode* C_NetRouterBase::GetAgoraLink(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getLinkCode();
	}

	bdTitleUtilities* C_NetRouterBase::GetTitleUtilities(void)
	{
		if (!m_BaseInitialised || !m_OnlineMode)
			return NULL;

		if (eConnectionStateComplete != m_PlatformConnectionState) 
			return NULL;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return NULL; // must be fully connected to demonware lobby

		return lobbyService->getTitleUtilities();
	}

	bool C_NetRouterBase::RegisterListener(C_NetMessageListener* pListener)
	{
		if (!m_BaseInitialised || NULL == pListener || m_ReceiveListeners.Size() == m_ReceiveListeners.Capacity())
			return false;

		for (S32 i=0; i<m_ReceiveListeners.Size(); i++)
		{
			if (pListener == m_ReceiveListeners[i])
				return true; // already registered once !
		}
	
		m_ReceiveListeners.PushBack(pListener);
		return true;
	}

	bool C_NetRouterBase::UnRegisterListener(C_NetMessageListener* pListener)
	{
		if (!m_BaseInitialised || NULL == pListener)
			return false;

		for (S32 i=0; i<m_ReceiveListeners.Size(); i++)
		{
			if (pListener == m_ReceiveListeners[i])
			{
				m_ReceiveListeners.EraseByIndex(i);
				return true;
			}
		}
		return false;
	}

	bool C_NetRouterBase::SendMessage(C_NetMessage& msg, bdAddrHandleRef addr)
	{
		if (!m_BaseInitialised)
			return false;

		// poll lobby connection if required.
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdSocketRouter* router =  net->getSocketRouter();
			if (router)
			{
				if (addr.isNull())
					return false; // invalid remote address..
				
				const bdDTLSAssociationStatus socketStatus = router->getStatus(addr);
				if (BD_SOCKET_CONNECTED != socketStatus)
					return false; // not fully connected yet so no use sending data.

				if (msg.HasHeader() && !msg.m_Reliable)
				{
					if (msg.DataSize() < sizeof(DatagramUnReliablePacketHeader) || !msg.IsTerminated())
					{
						BIZ_ASSERT(false);
						return false; // malformed message, not enough data or message was not closed after creation.
					}
				}
				else
				{
					// trying to send malformed message or incorrect message type.. reliable messages are not sent through here
					// they are sent through the reliable outbox class.
					BIZ_ASSERT(false);
					return false;
				}

				/*
				U16* pGameDataSize = (U16*)msg.Data();
				U16 out = 0;
				EncryptedSize(out, *pGameDataSize);
				LOGINFO(g_Log, "send message %d : %d", msg.DataSize(), out);
				*/

				const S32 sent = router->sendTo(addr, msg.Data(), msg.DataSize());
				if (sent == msg.DataSize())
				{
					// track outgoing bytes for bandwidth usage.
					TrackOutgoingBytes(sent);
					return true;
				}
			}
		}

		return false;
	}

	bool C_NetRouterBase::SendMessage(U8* pData, const U32 dataSize, bdAddrHandleRef addr)
	{
		if (!m_BaseInitialised || NULL == pData || 0 == dataSize)
			return false;

		// poll lobby connection if required.
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdSocketRouter* router =  net->getSocketRouter();
			if (router)
			{
				if (addr.isNull())
					return false; // invalid remote address..

				const bdDTLSAssociationStatus socketStatus = router->getStatus(addr);
				if (BD_SOCKET_CONNECTED != socketStatus)
					return false; // not fully connected yet so no use sending data.
/*
				U16* pGameDataSize = (U16*)pData;
				U16 out = 0;
				EncryptedSize(out, *pGameDataSize);
				LOGINFO(g_Log, "send message raw %d : %d", dataSize, out);
*/
				const S32 sent = router->sendTo(addr, pData, dataSize);
				if (sent == dataSize)
				{
					// track outgoing bytes for bandwidth usage.
					TrackOutgoingBytes(sent);
					return true;
				}
			}
		}

		return false;
	}

	void C_NetRouterBase::StartBandwidthEvaluation(void)
	{
		if (!m_BaseInitialised || m_BandwidthEvaluated || !m_OnlineMode)
			return;

		// since the game code doesnt reference the bandwidth we evaluate, there is no point wasting time doing this..
		return;

		if (m_PlatformConnectionState != eConnectionStateComplete)
		{
			LOGWARN(g_Log, "C_NetRouterBase::StartBandwidthEvaluation()- can't start bandwidth scans until connected to demonware.");
			return;
		}

		if (BandwidthEvaluationInProgress())
		{
			LOGWARN(g_Log, "C_NetRouterBase::StartBandwidthEvaluation()- bandwidth evaluation already in progress, can't start another.");
			return;
		}

		BIZ_ASSERT(NULL == m_pBandwidthEvaluator);

		LOGINFO(g_Log, "C_NetRouterBase::StartBandwidthEvaluation()- bandwidth evaluation started.");

		m_pBandwidthEvaluator = new NetPlay::C_BandwidthEvaluator;
		if (m_pBandwidthEvaluator)
			m_pBandwidthEvaluator->Start();
	}

	void C_NetRouterBase::StopBandwidthEvaluation(void)
	{
		if (!m_BaseInitialised)
			return;

		if (m_pBandwidthEvaluator)
		{
			m_pBandwidthEvaluator->Stop();
			delete m_pBandwidthEvaluator;
			m_pBandwidthEvaluator = NULL;
		}
	}

	void C_NetRouterBase::TrackIncomingBytes(const U32 size)
	{
		if (0 == size)
			return;

	}

	void C_NetRouterBase::TrackOutgoingBytes(const U32 size)
	{
		if (0 == size)
			return;

	}

	bool C_NetRouterBase::PeerRoutingEnabled(void)
	{
		if (!m_BaseInitialised ||m_PlatformConnectionState != eConnectionStateComplete)
			return false;

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
			return net->getSocketRouter()->isRelayEnabled();

		return false;
	}

}// end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
