#include "stdafx.h"
#include <Core/Core.h>
#include "../Misc/NetDebug.h"
#include "../Misc/NetDefines.h"
#include <Network/Network.h>
#include "NetLanDiscovery.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_LanDiscoveryManager::C_LanDiscoveryManager() :	m_Initialised(false), m_Advertising(false), m_Enumerating(false), m_TitleId(0), m_AdvertisedNumSlotsOpen(0),
														m_pEnumeratedSessions(NULL)
	{
	}

	C_LanDiscoveryManager::~C_LanDiscoveryManager()
	{
		Shutdown();
	}

	bool C_LanDiscoveryManager::Initialise(const U32 titleId)
	{
		if (m_Initialised)
			return true;

		// create a broadcast socket.
		if (m_Socket.create(false, true))
		{
			const bdAddr bindAddr(bdInetAddr::Any(), AMAX_LAN_DISCOVERY_PORT);
			if (m_Socket.bind(bindAddr))
			{
				m_TitleId = htonl(titleId);
				m_Initialised = true;
			}
			else
			{
				LOGWARN(g_Log, "C_LanDiscoveryManager::Initialise() - unable to bind lan discovery socket.");
				Shutdown();
			}
		}

		return m_Initialised;
	}

	void C_LanDiscoveryManager::Shutdown()
	{
		if (m_pEnumeratedSessions)
			delete m_pEnumeratedSessions;
		m_pEnumeratedSessions = NULL;

		m_Initialised = false;
		m_Enumerating = false;
		m_Advertising = false;
		m_TitleId = 0;
		m_EnumerateTimer.Stop();
		if (m_Socket.getHandle() != BD_INVALID_SOCKET_HANDLE)
			m_Socket.close();
	}

	bool C_LanDiscoveryManager::Advertise(const SysProfile::BaseProfile& host, const NetPlay::SessionSecurity& security, const LanDiscoverySessionParameters& params, const U32 numSlotsOpen)
	{
		if (!m_Initialised)
			return false;

		m_AdvertisedNumSlotsOpen = numSlotsOpen;
		m_AdvertisedHost = host;
		m_AdvertisedSecurity = security;
		m_AdvertisedParameters = params;
		m_Advertising = true;
		return true;
	}

	void C_LanDiscoveryManager::UnAdvertise(void)
	{
		if (!m_Initialised)
			return;

		m_Advertising = false;
		m_AdvertisedParameters.Clear();
		m_AdvertisedNumSlotsOpen = 0;
	}

	bool C_LanDiscoveryManager::Enumerate(const U32 numToJoin)
	{
		if (!m_Initialised || 0 == numToJoin)
			return false;

		m_Enumerating = false;
		if (m_pEnumeratedSessions)
			m_pEnumeratedSessions->Clear();
		else
		{
			m_pEnumeratedSessions = new LanDiscoveryReplies;
			if (NULL == m_pEnumeratedSessions)
				return false;
		}

		// generate a nonce..
		m_EnumerateNonce.Create();

		// send request broadcast.
		LanDiscoveryRequest req;
		req.m_PacketVersion		= htonl(NET_PACKET_VERSION);
		req.m_TitleId			= m_TitleId;
		req.m_PacketType		= eLanDiscoveryPacketType_Request;	
		req.m_Nonce				= m_EnumerateNonce;
		req.m_NumPlayersToJoin	= numToJoin;

		// broadcast this packet..
		const bdAddr bcAddr(bdInetAddr::Broadcast(), AMAX_LAN_DISCOVERY_PORT);
		const S32 sendResult = m_Socket.sendTo(bcAddr , &req, sizeof(req));

		m_Enumerating = true;
		m_EnumerateTimer.Start();

		return true;
	}

	bool C_LanDiscoveryManager::IsEnumerating(void)
	{
		if (!m_Initialised)
			return false;
		return m_Enumerating;
	}

	U32 C_LanDiscoveryManager::NumEnumerated(void) const
	{
		if (!m_Initialised || m_Enumerating || NULL == m_pEnumeratedSessions)
			return 0;
		return static_cast <U32> (m_pEnumeratedSessions->Size());
	}

	bool C_LanDiscoveryManager::GetEnumeratedSesionData(const U32 sessionIndex, LanDiscoveryReply& sessionData)
	{
		if (!m_Initialised || m_Enumerating || NULL == m_pEnumeratedSessions)
			return false; // no sesions to return at this time.

		if (sessionIndex >= (U32)m_pEnumeratedSessions->Size())
		{
			LOGERROR(NetPlay::g_Log, "C_LanDiscoveryManager::GetEnumeratedSesionData() - trying to access enumerated lan session that doesn't exist!");
			return false;
		}

		sessionData = (*m_pEnumeratedSessions)[sessionIndex];
		return true;
	}

	void C_LanDiscoveryManager::DestroyEnumerated(void)
	{
		if (!m_Initialised)
			return;
		m_Enumerating = false;
		m_EnumerateTimer.Stop();
		if (m_pEnumeratedSessions)
			delete m_pEnumeratedSessions;
		m_pEnumeratedSessions = NULL;
	}

	void C_LanDiscoveryManager::Poll(void)
	{
		if (!m_Initialised)
			return;

		if (m_Socket.getHandle() == BD_INVALID_SOCKET_HANDLE)
			return;

		bdAddr from;
		bdUByte8 data[BD_MAX_DATAGRAM_SIZE];
		S32 received =  -1;
		do
		{
			received = m_Socket.receiveFrom(from, data, BD_MAX_DATAGRAM_SIZE);
			if (received > sizeof(LanDiscoveryHeader))
			{
				LanDiscoveryHeader* pHdr = reinterpret_cast<LanDiscoveryHeader*> (data);
			
				// m_TitleId if is already stored in network byte order.	
				if (pHdr->m_TitleId == m_TitleId) 
				{
					// overwrite to put the header variables back into the correct format for this platform
					// game logic will need to reference the packet version before attemptting to join.
					pHdr->m_PacketVersion = ntohl(NET_PACKET_VERSION);
					pHdr->m_TitleId = ntohl(m_TitleId);

						// if we get here then this packet is for our game and platform.
					if (pHdr->m_PacketType == eLanDiscoveryPacketType_Request)
					{
						if (m_Advertising)
						{
#if defined (PLATFORM_XENON) 
							// xbox360 must reply to the broadcast address...
							bdAddr bcAddr(bdInetAddr::Broadcast(), from.getPort());
							from = bcAddr;
#endif
							if (received == sizeof(LanDiscoveryRequest))
							{
								const LanDiscoveryRequest* pReq = reinterpret_cast<LanDiscoveryRequest*> (data);
								if (m_AdvertisedNumSlotsOpen >= pReq->m_NumPlayersToJoin)
								{
									LanDiscoveryReply reply;
									reply.m_PacketVersion	= htonl(NET_PACKET_VERSION);
									reply.m_TitleId			= m_TitleId;
									reply.m_PacketType		= eLanDiscoveryPacketType_Reply;	
									reply.m_Nonce			= pReq->m_Nonce;

									reply.m_HostProfile		= m_AdvertisedHost;
									reply.m_SessionSecurity	= m_AdvertisedSecurity;
									reply.m_Parameters		= m_AdvertisedParameters;

									// send packet back..
									const S32 sendResult = m_Socket.sendTo(from , &reply, sizeof(reply));
								}
							}
						}
					}
					else if (pHdr->m_PacketType == eLanDiscoveryPacketType_Reply)
					{
						if (m_Enumerating)
						{
							if (received == sizeof(LanDiscoveryReply))
							{
								const LanDiscoveryReply* pReply = reinterpret_cast<LanDiscoveryReply*> (data);
								if (pReply->m_Nonce == m_EnumerateNonce)
								{
									// woot a reply to our original enumeration request.
									if (m_pEnumeratedSessions && m_pEnumeratedSessions->Size() != m_pEnumeratedSessions->Capacity())
										m_pEnumeratedSessions->PushBack(*pReply);
								}
							}
						}
					}
				}
			}
		}while (received > 0);

		// stop enumerating after a set time period.
		if (m_Enumerating)
		{
			if (m_EnumerateTimer.ElapsedTimeInSeconds() >= 2.0f)
			{
				m_EnumerateTimer.Stop();
				m_Enumerating = false;
			}
		}
	}

}

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
