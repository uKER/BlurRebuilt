// ***************************************************************
//  NetRouter_pc   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Handles sending / receiving network datagrams. Also
//			responsible for connectong to the platform specific servers
//			and creating online user accounts.
// ***************************************************************

#ifndef NETWORK_ROUTER_PC_H
#define NETWORK_ROUTER_PC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NetRouterBase.h"
#include <bdLobby/bdAuthService.h>

namespace RacerNet
{
	// forward declerations.
	class C_NetAccountCreationListener;

	/// PC specific code used for connecting to the platform holders servers and any game specific servers.
	class C_NetRouterPC : public C_NetRouterBase
	{
	public:
		C_NetRouterPC();
		virtual ~C_NetRouterPC();

		typedef enum
		{
			ePCAuthStateNotActive = 0,
			ePCAuthStateStartingNetwork,
			ePCAuthStateDnsChecks,
			ePCAuthStateAuthenticating,
			ePCAuthStateConnectingToLobby,
			ePCAuthStateError,
		}ePCAuthState;

		/// Initialise dynamic resources.
		/** @param maxSendMessageSize the maximum size of a network message that the game will send.
		 *	@return true if resources were alloacted. 
		 */
		virtual bool Initialise(const U32 maxSendMessageSize);

		/// Shutdown router and release all dynamic resources.
		/** This will also disconnect from all game servers.
		 */
		virtual void Shutdown(void);

		/// Attempt to create a new account on the servers.
		/** This is only valid on PC since PS3 and Xbox360 all have their platfor account management handle by the OS.
		 *	Once account creation has finished we will be disconnected from the platform servers. and have to reconnect via PlatformConnect().
		 *	Don't call PlatformConnect() while trying to create an account as that will totally kill the account creation process.
		 *	@param username the name that the user wants to be reference by.
		 *	@param password the password that the user requires.
		 *	@param licenseCode the license code ffrom the users copy of the game.
		 *	@param pListener object that will be used to fire callback when the account creation process completes. NOTE : this object must stay resident for the complete
		 *	account creation process or it will stomp memory.
		 *	@return true if the account process was started or false if not.
		 */
		virtual bool CreateOnlineAccount(char* userName, char *password, char* licenseCode, C_NetAccountCreationListener* pListener);

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

	private:
		bool StartConnect(void);
		void ContinueNetworkStart(void);
		void ContinueDnsLookup(void);
		void StartAuthentication(void);
		void ContinueAuthentication(void);
		void ContinueLobbyConnection(void);

	private:
		bool							m_Initialised;
		char*							m_Lsg;
		ePCAuthState					m_PcAuthState;
		bdInetAddr						m_AuthAddr;
		bdGetHostByName*				m_pAuthGetHostByName;
		bdInetAddr						m_LsgAddr;
		bdGetHostByName*				m_pLsgGetHostByName;
		bdAuthService*					m_pAuthService;
		char*							m_UserName;
		char*							m_Password;
		char*							m_License;
		C_NetAccountCreationListener*	m_pAccountCreationListener;
	};

}// end namespace RacerNet

#endif // NETWORK_ROUTER_PC_H
