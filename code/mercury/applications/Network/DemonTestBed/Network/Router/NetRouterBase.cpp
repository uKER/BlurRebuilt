#include <stdafx.h>
#include <Core/core.h>
#include <Network/Network.h>
#include <Network/BandwidthEvaluator.h>
#include "../Misc/NetDefines.h"
#include "../Misc/NetDebug.h"
#include "NetRouterbase.h"
#include "NetAccountCreationListener.h"
#include "../Message/NetMessageListener.h"
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetDatagramPacketTypes.h"

#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	// constructor
	C_NetRouterBase::C_NetRouterBase() :	m_BaseInitialised(false), m_PlatformConnectionState(eConnectionStateNotActive), 
											m_LastConnectionErrorCode(eConnectionResultNone), m_OnlineMode(false),
											m_pBandwidthEvaluator(NULL), m_BandwidthEvaluated(false),
											m_UpstreamBandwithKbps(NET_MINIMUM_BANDWIDTH_KBPS), m_DownstreamBandwithKbps(NET_MINIMUM_BANDWIDTH_KBPS)
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

		assert(sendBufferSize);
		if (0 == sendBufferSize)
			return false;

		// configure send buffer size.
		const bool sendOk = m_SendMessage.Initialise(sendBufferSize);

		// configure reveive buffer size.
		const bool receiveOk = m_ReceiveMessage.Initialise(BD_MAX_PAYLOAD_SIZE);

		if (sendOk && receiveOk)
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

	void C_NetRouterBase::Poll(void)
	{
		if (!m_BaseInitialised)
			return;

		// poll lobby connection if required.
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdSocketRouter* router =  net->getSocketRouter();
			if (router)
			{
				// poll socket router
				router->pump();

				// poll demonware misc services..
				net->receiveAll();
				net->dispatchAll();
				net->sendAll();

				S32 bytesReceived = 0;
				bdAddrHandleRef addr = BD_NULL;
				do 
				{
					// flush data from last received net message.
					m_ReceiveMessage.Flush();
					
					// receive from socket.
					bytesReceived = router->receiveFrom(addr, m_ReceiveMessage.Data(), m_ReceiveMessage.MaxDataSize());
					
					if (bytesReceived > 0)
					{
						if (bytesReceived > sizeof(DatagramPacketHeader))
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
								if (eNetMessageTypeReliableAcknowlegement == pDatagram->m_PacketType)
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
									if (bytesReceived > sizeof(DatagramUnReliablePacketHeader))
									{
										bdUInt16 encryptedBytes = 0;
										U32 offset = 0; 
										if (bdBytePacker::removeBasicType(pUnReliableDatagram, m_ReceiveMessage.m_CurrentBufferSize, offset, offset, encryptedBytes))
										{
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
					}

				} while(bytesReceived > 0);
			}
		}
	}

	bool C_NetRouterBase::CreateOnlineAccount(char* userName, char *password, char* licenseCode, C_NetAccountCreationListener* pListener)
	{
		if (!m_BaseInitialised)
			return false;

		LOGERROR(NetPlay::g_Log, "C_NetRouterBase::CreateOnlineAccount() - Online account creation not possible on this platform!");
		return false;
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
			if (m_pBandwidthEvaluator)
			{
				m_pBandwidthEvaluator->Update();
				if (!m_pBandwidthEvaluator->IsActive())
				{
					U32 up, down;
					m_pBandwidthEvaluator->GetResults(up, down);
					
					StopBandwidthEvaluation();

					m_UpstreamBandwithKbps = (up < NET_MINIMUM_BANDWIDTH_KBPS) ? NET_MINIMUM_BANDWIDTH_KBPS : up;
					m_DownstreamBandwithKbps = (down < NET_MINIMUM_BANDWIDTH_KBPS) ? NET_MINIMUM_BANDWIDTH_KBPS : down;
					m_BandwidthEvaluated = true;
				}
			}

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
						assert(false);
						return false; // malformed message, not enough data or message was not closed after creation.
					}
				}
				else
				{
					// trying to send malformed message or incorrect message type.. reliable messages are not sent through here
					// they are sent through the reliable outbox class.
					assert(false);
					return false;
				}

				const S32 sent = router->sendTo(addr, msg.Data(), msg.DataSize());
				return (sent == msg.DataSize()) ? true : false;
			}
		}

		return false;
	}

	void C_NetRouterBase::StartBandwidthEvaluation(void)
	{
		if (!m_BaseInitialised || m_BandwidthEvaluated || !m_OnlineMode)
			return;

		if (BandwithEvaluationInProgress())
		{
			LOGWARN(g_Log, "C_NetRouterBase::StartBandwidthEvaluation()- bandwidth evaluation already in progress, can't start another.");
			return;
		}

		assert(m_pBandwidthEvaluator);

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

}// end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
