// ***************************************************************
//  NetRouter_ps3   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Handles sending / receiving network datagrams. Also
//			responsible for connectong to the platform specific servers.
// ***************************************************************

#ifndef NETWORK_ROUTER_PS3_H
#define NETWORK_ROUTER_PS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NetRouterBase.h"
#include <bdLobby/bdAuthService.h>

namespace RacerNet
{
	/// PC specific code used for connecting to the platform holders servers and any game specific servers.
	class C_NetRouterPS3 : public C_NetRouterBase
	{
	public:
		C_NetRouterPS3();
		virtual ~C_NetRouterPS3();

		typedef enum
		{
			ePS3AuthStateNotActive = 0,
			ePS3AuthStateStartingNetwork,
			ePS3AuthStateAuthDnsCheck,
			ePS3AuthStateRequestingTicket,
			ePS3AuthStateGotTicket,
			ePS3AuthStateAuthorizingTicket,
			ePS3AuthStateLsgDnsCheck,
			ePS3AuthStateConnectingToLobby,
			ePS3AuthStateError,
		}ePS3AuthState;

		/// Initialise dynamic resources.
		/** @param maxSendMessageSize the maximum size of a network message that the game will send.
		 *	@return true if resources were alloacted. 
		 */
		virtual bool Initialise(const U32 maxSendMessageSize);

		/// Shutdown router and release all dynamic resources.
		/** This will also disconnect from all game servers.
		*/
		virtual void Shutdown(void);

		/// Attempt to connect to our service provider for online functionality.
		/** @param onlineMode if true then if using demonware we will go through NAT negotiation otherwise LAN games will only be possible.
		 *	@param username name of player who will be authenticated (only relavant on PC).
		 *	@param username password for the player who will be authenticated (only relavant on PC).
		 *	@return true if the connection logic was started.
		 */
		virtual bool PlatformConnect(const bool onlineMode, char* userName, char* password);

		/// Attempt to disconnect from our platform service provider.
		/** Network multiplayer won't be available if disconnected.
		 */
		virtual void PlatformDisconnect(void);

		/// Call once per frame to pump any active tasks.
		virtual void Poll(void);

		/// Look for ticket replies from NP.
		virtual void PlatformManagerCallback(S32 ev, S32 result, void *arg);

	private:
		bool StartConnect(void);
		void ContinueNetworkStart(void);
		void ContinueAuthDnsLookup(void);
		void StartTicketRequest(void);
		void ContinueTicketRequest(void);
		void ContinueCrossAuth(void);
		void ContinueLsgDnsCheck(void);
		void StartLobbyConnect(void);
		void ContinueLobbyConnect(void);
		void ChooseLsgAddressFromTicket(void);

	private:
		bool			m_Initialised;
		char*			m_Lsg;
		char*			m_Ps3Cookie;
		U32				m_Ps3CookieSize;
		ePS3AuthState	m_Ps3AuthState;
		bdInetAddr		m_AuthAddr;
		bdGetHostByName*m_pAuthGetHostByName;
		bdAuthService*	m_pAuthService;
		bdInetAddr		m_LsgAddr;
		bdGetHostByName*m_pLsgGetHostByName;
	};

}// end namespace RacerNet

#endif // NETWORK_ROUTER_PS3_H

