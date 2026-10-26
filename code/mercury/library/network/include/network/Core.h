// ***************************************************************
//  Core   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Controls the network stack.
// ***************************************************************

#ifndef NETWORK_CORE_H
#define NETWORK_CORE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network\SystemHeaders.h>
#include <Network\Network.h>

// forward declerations
class bdLobbyService;

namespace NetPlay
{
	// forward declarations..
	struct Nonce;

	/// Main class for controlling the network layer, Network cable removal etc.
	class C_Core 
	{
	public:
		/// Get a reference to the static class instance.
		static C_Core& Instance(void)	{return s_NetworkCore;}

		/// Startup the network stack.
		/** For PS3 the following modules should be loaded before calling this function
		 *	CELL_SYSMODULE_SYSUTIL, CELL_SYSMODULE_SYSUTIL_NP, CELL_SYSMODULE_NET, CELL_SYSMODULE_NETCTL.
		 *	Call once on application startup.
		 *	@param bypassSecurity used on Xbox360 to bypass secure network library while debugging.
		 *	@param networkStackAlreadyStarted true if the game has already setup the network stack for for connection to the debug tool prior to calling this function.
		 */
		void Initialise(const bool bypassSecurity, const bool networkStackAlreadyStarted);
		
		/// Close down the network stack.
		/**	Call upon application exit.
		 *	@see Initialise()
		 */
		void Shutdown(void);

		/// Call once per frame to perform any asynchronous tasks.
		/** @param forceNetworkLinkCheck if true then we will force a new scan of the network link.
		 */
		void Poll(const bool forceNetworkLinkCheck);

		/// Test the status of the network stack.
		/**	@return true if the network stackhas been started up.
		 */
		bool IsInitialised(void) const {return m_StartedUp;}

		/// Test the status of the network connection.
		/**	The Network stack must have been started up for this function to work.
		 *	@see Startup().
		 *	@return true if a network connection exists.
		 */
		bool CableConnected(void) const {return m_NetCableConnected;}

		/// Force a scan for the local network address.
		/**	The Network stack must have been started up for this function to work.
		 *	@see Startup().
		 */
		void ForceLocalAddressScan(void);

		/// is our local network address valid
		/**	The Network stack must have been started up for this function to work.
		 *	@see Startup().
		 *	@see ForceLocalAdressScan().
		 *	@return true if our network address is valid.
		 */
		bool ValidLocalAddress(void) const {return m_ValidLocalAddress;}

		/// Attempt to generate a unique machine identifier.
		/**	@param machineId upon successfull return holds the unique identifier for this console.
		 *	@return true if a valid identifier was created.
		 */
		bool GenerateMachineIdentifier(U64& machineId);

		/// Get access to the demonware lobby service.
		bdLobbyService* DemonwareLobbyService(void) {return m_pDemonwareLobby;}

	private:
		explicit C_Core();
		~C_Core();
		C_Core(const C_Core& original);				// private copy constructor
		C_Core& operator = (const C_Core rhs);		// private un coded assignment operator

		void Cleanup(void);
		
	private:
		static C_Core	s_NetworkCore;
		bool			m_StartedUp;
		bool			m_ValidLocalAddress;
		bool			m_NetCableConnected;
		bool			m_ScanForLocalAddress;
		U64				m_ScanAddressTimer;
		U32				m_BitFields;

		bdLobbyService*	m_pDemonwareLobby;

#if defined (PLATFORM_PS3)
		U8*	m_pNetworkPlatformPool;
#endif
	};
}

#endif


