// ***************************************************************
//  NetRouter_xenon   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Handles sending / receiving network datagrams. Also
//			responsible for connectong to the platform specific servers.
// ***************************************************************

#ifndef NETWORK_ROUTER_XENON_H
#define NETWORK_ROUTER_XENON_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NetRouterBase.h"

namespace RacerNet
{
	/// PC specific code used for connecting to the platform holders servers and any game specific servers.
	class C_NetRouterXenon : public C_NetRouterBase
	{
	public:
		C_NetRouterXenon();
		virtual ~C_NetRouterXenon();

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

	private:
		bool m_Initialised;
	};

}// end namespace RacerNet

#endif // NETWORK_ROUTER_XENON_H
