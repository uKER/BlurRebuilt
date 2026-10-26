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
#include <bdLobby/bdTitleUtilities/bdVerifyString.h>

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
			ePS3AuthStateInitialDelay,
			ePS3AuthStateStartingService,
			ePS3AuthStateStartingNetwork,
			ePS3AuthStateAuthDnsCheck,
			ePS3AuthStateRequestingTicket,
			ePS3AuthStateGotTicket,
			ePS3AuthStateAuthorizingTicket,
			ePS3AuthStateLsgDnsCheck,
			ePS3AuthStateConnectingToLobby,
			ePS3AuthStateEvaluatingBandwidth,
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
		virtual void Poll(const F32 frameTimeInSeconds);

		/// Look for ticket replies from NP.
		virtual void PlatformManagerCallback(S32 ev, S32 result, void *arg);

		/// this should be called when we get the notification from the os that the net control dialog has finished.
		/** the dialog is started up whenever the router tries to connect in online or offline mode.
		*/
		virtual void NetStartDialogFinished(void);

		/// this should be called when we get the notification from the os that the net control dialog has been unloaded.
		/** the dialog is started up whenever the router tries to connect in online or offline mode.
		 */
		virtual void NetStartDialogUnloaded(void);

		/// Enable or disable content streaming functionality.
		/** content streaming MUST be enabled prior to calling GetContentStreamingServer() to allocate resources.
		 *	once content streaming is no longer required we can disable it to release resources.
		 *	@param enabled true is we wish to enable content streaming or false if we wish to disable it.
		 *	@return true if succeeded.
		 */
		virtual bool EnableContentStreaming(const bool enabled);

		/// Attempt to verify a character string.
		/** Only ised when creating groups so if no connection to the LSP exists this functionality will fail.
		 *	if the function returns true then continue to call PollVerifyString() until completion
	 	 *	@param pString string to be verified. This needs to stay in memory for the complete duration of the verification process.
		 *	@return true if the async string verification was begun.
		 */
		virtual bool VerifyString(const char* pString);

		/// Continues the asynchronous process of validating a string.
		/** @param validString once the function returns true then this value determins if the string has passed the validation
		 *	@return true once the validation process hhas completed, false if its still in progress..
		 */
		virtual bool PollVerifyString(bool& validString);

		/// Retrieve the type of NAT this player has.
		/** If thre router is not connected then this will return eNatOpen. 
		*/
		virtual eNatType GetNatType(void);

		/// Used on the PS3 to verify the age restriction parental control settings for online play.
		virtual bool CheckOnlineAgeRestriction(void);

	private:
		bool StartInitialDelay(void);
		bool ContinueInitialDelay(void);
		bool StartService(void);
		void ContinueStartService(void);
		bool StartConnect(void);
		void ContinueStartConnect(void);
		void ContinueAuthDnsLookup(void);
		void StartTicketRequest(void);
		void ContinueTicketRequest(void);
		void ContinueCrossAuth(void);
		void ContinueLsgDnsCheck(void);
		void StartLobbyConnect(void);
		void ContinueLobbyConnect(void);
		void ChooseLsgAddressFromTicket(void);
		void UnloadNetStartDialog(void);

		void KillStringVerification(void);

		void UpdateMatchingRegion(const char* code);

	private:
		bool			m_Initialised;
		bool			m_ShutDownNetStartDialog;
		char*			m_Lsg;
		char*			m_Ps3Cookie;
		U8*				m_pHttpPool;
		U8*				m_pHttpsPool;
		U32				m_Ps3CookieSize;
		ePS3AuthState	m_Ps3AuthState;
		bdInetAddr		m_AuthAddr;
		bdGetHostByName*m_pAuthGetHostByName;
		bdAuthService*	m_pAuthService;
		bdInetAddr		m_LsgAddr;
		bdGetHostByName*m_pLsgGetHostByName;
		bool			m_NetStartDialogComplete;
		bool			m_NetStartDialogSuccessful;
		U64				m_AuthTicketTimer;
		U64				m_ConnectionTimer;
		U64				m_InitialDelay;
		bdAddr			m_ContentStreamingAddress;

		bool			m_StringVerifyActive;
		bdRemoteTaskRef	m_StringVerifyTask;
		bdVerifyString	m_StringVerifyResult;	
	};

}// end namespace RacerNet

#endif // NETWORK_ROUTER_PS3_H

