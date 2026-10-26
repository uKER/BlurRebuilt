// ***************************************************************
//  NetCreationHelper   version:  1.0   ·  date: 08/11/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Stores data required to create or join a network session.
//			local player data for example
// ***************************************************************

#ifndef __NET_CREATION_HELPER_H__
#define __NET_CREATION_HELPER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/vector.h>
#include <SystemProfile/SystemProfile.h>
#include "../Misc/NetDefines.h"
#include "../Player/Data/NetPlayerProfileData.h"
#include "../Router/NetRouterDefines.h"

namespace RacerNet
{
	// forwatd declerations.
	class C_NetParty;
	class C_NetPlayerManager;

	/// Stores what local players need to be setup when we create or join a network session.
	class C_NetCreationHelper
	{
	public:
		friend class C_NetRace;
		friend class C_NetParty;

		struct LocalPlayer 
		{
			LocalPlayer()
			{
				m_ProfileData.Flush();
				m_VoiceBanned = false;
			}

			SysProfile::FullProfile m_SystemProfile;	// user name + system 
			NetPlayerProfileGameData m_ProfileData;	
			bool m_VoiceBanned;
		};

		C_NetCreationHelper();
		virtual ~C_NetCreationHelper();

		/// Set which profiles will be participating in the lan game.
		/** Players not in this list won't be added to the network session.
		 *	@return true if the player data was successfully setup.
		 */
		bool BuildLocalPlayersForLan(const U64 consoleId, const U16 bandwidthUpKbps, const U16 bandwidthDownKbps);

		/// Set which profiles will be participating in the online game.
		/** Players not in this list won't be added to the network session.
		 *	@return true if the player data was successfully setup.
		 */		
		bool BuildLocalPlayersForOnline(const eNatType natType, const U64 consoleId, const U16 bandwidthUpKbps, const U16 bandwidthDownKbps);

		/// Set which profiles will be participating in the online game.
		/** References party session for some data.
		 */
		bool BuildLocalPlayersForOnlineFromParty(const eNatType natType, C_NetParty& party, C_NetPlayerManager& mgr);

		/// How many local players are to be created for the required network session.
		/** Must call one of the Build functions first to actually generate the local player data.
		 *	@see BuildLocalPlayersForLan()
		 */
		U32 NumPlayers(void) const { return (U32)m_LocalPlayerData.Size(); }

		/// Used in online games to denote which local player controls the matching session.
		S32 SessionManagerIndex(void) const { return m_LocalSessionManagerIndex; }

		/// used when matching in online games so we can match better.
		S32 PrimaryOnlinePlayersRank(void) const { BIZ_ASSERT(m_LocalSessionManagerIndex > -1); return m_LocalPlayerData[0].m_ProfileData.m_Game.m_Rank; }

		/// Is user created content available to all local players.
		bool CanUseUserCreatedContent(void) const { return m_UserCreatedContentAvailable; }

	private:
		void SetupGameProfileDataForPlayers(const bool lanMode, const eNatType natType);

		Btl::Vector<LocalPlayer, Btl::VectorParameters_Static<NET_MAX_LOCAL_PLAYERS> > m_LocalPlayerData;
		S32 m_LocalSessionManagerIndex;	// used in online games as the local player who controls the matchmaking sessions. 
		U64 m_ConsoleId; 
		U16 m_UpStreamBandwidthKbps;
		U16 m_DownStreamBandwidthKbps;
		U32	m_DlcMask;
		bool m_UserCreatedContentAvailable;
	};

}// end namespace RacerNet

#endif //__NET_CREATION_HELPER_H__

