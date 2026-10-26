// ***************************************************************
//  NetLanDiscovery   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles session discovery when running over the LAN.
// ***************************************************************

#ifndef __NET_LAN_DISCOVERY_H__
#define __NET_LAN_DISCOVERY_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Nonce.h>
#include <bdCore/bdSocket/bdDefaultSocket.h>
#include "NetLanDiscoveryDefines.h"
#include "../Core/NetTimer.h"
#include "../Misc/NetRaceSettingsQos.h"


namespace RacerNet
{
	/// Manager class for handling lan session enumeration.
	/** Takes care of advertising and enumerating lan sessions.
	 *	LAN discovery works across all versions of the game, after a player enumerates a session he is responsible for checking the packet version of the host data.
	 *	If the host has a higher packet version then the host has a newer version of the game client, likewise if the host has a lower packet version then he has 
	 *	an older game client. It's only possible to join hosts with the same network packet version as we have.
	 *	Ideally session hosts would ignore enumeration requests from clients who don't have the same packet version but xbox360 tcr states we need to show the
	 *	client that the session exists but is not joinable.. gits.
	 */
	class C_LanDiscoveryManager
	{
	public:
		C_LanDiscoveryManager();
		virtual ~C_LanDiscoveryManager();

		/// Setup Socket and allocate any dynamic reasources.
		/** @param titleId Specific title Id for this platform.
		 *	@return true if successfully initialised.
		 */
		bool Initialise(const U32 titleId);

		/// Close socket and deallocate any dynamic resources.
		void Shutdown();

		/// Call once per frame to pump socket.
		void Poll(void);

		/// Advertise your session over the LAN.
		/** This must be updated as players join / leave the lan game on the hosts console.
		 *	@param host the session host information.
		 *	@param security  the security information so peers can connect.
		 *	@param pCommonAddress pointer to the common address of the host for connection purposes. 
		 *	@param numSlotsOpen how many slots are stull available for peers to join.
		 *	@param groupId the selected group unique identifier that this lobby takes its settings from.
		 *	@param consoleId console Id of the host player.
		 *	@param raceQos race settings used by the UI on the session browser screen.
		 *	@return true if the session was advertised successfully.
		 */
		bool Advertise(const SysProfile::BaseProfile& host, const NetPlay::SessionSecurity& security, const U8* pCommonAddress, const U32 maxSlots, const U32 numSlotsOpen, const U64 consoleId, const NetRaceSettingsQos& raceQos);

		/// Stops the session from being advertised on the LAN.
		/** @see Advertise().
		 */
		void UnAdvertise(void);

		/// Start the process of enumerating lan games.
		/** @param numToJoin how many local players will be joining.
		 *	@param groupId the selected group that we wish to find lobbies from.
		 *	@return true if the enumerate started successfully.
		 */
		bool Enumerate(const U32 numToJoin);

		/// Are we currently enumerating lan sessions.
		/** @see Enumerate()
		 */
		bool IsEnumerating(void);

		/// How many sessions have been enumerated.
		/** If called when we are still enumerating this will return 0
		 *	@return number of lan session found.
		 */
		U32 NumEnumerated(void) const;

		/// Get a specific enumerated session data structure.
		/** After enumeration call this to get acess to any session that were enumerated.
		 *	@see Enumerate()
		 *	@param sessionIndex which session to return, @see NumEnumerated()
		 *	@sessionData structure will be filled in if the supplied session index is valid.
		 *	@return true if the specified sessionIndex is valid or false if not. if false then sessionData will be invalid.
		 */
		bool GetEnumeratedSesionData(const U32 sessionIndex, LanDiscoveryReply& sessionData); 

		/// Stops enumeration and destroys any session data that has been enumerated.
		/** Call this when the player has chose a sssion to join or when the enumerated data is not required anymore.
		 */
		void DestroyEnumerated(void);

	private:
		bool m_Initialised;
		U32	m_TitleId;
		bdDefaultSocket m_Socket;
		
		// data used when advertising a session.
		bool m_Advertising;
		U32 m_AdvertisedNumSlotsOpen;
		U32 m_AdvertisedMaxSlots;
		U64	m_AdvertisedConsoleId;
		NetRaceSettingsQos m_RaceSettingsQos;
		NetPlay::SessionSecurity m_AdvertisedSecurity;
		SysProfile::BaseProfile	m_AdvertisedHost;
		U8 m_AdvertisedCommonAddress[BD_COMMON_ADDR_SERIALIZED_SIZE];	

		// data used when enumerating sessions.
		bool m_Enumerating;
		C_NetTimer	m_EnumerateTimer;
		LanDiscoveryReplies* m_pEnumeratedSessions;
		NetPlay::Nonce	m_EnumerateNonce;
	};

} // end namespace RacerNet

#endif // __NET_LAN_DISCOVERY_H__
