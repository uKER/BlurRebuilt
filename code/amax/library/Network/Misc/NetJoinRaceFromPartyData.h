// ***************************************************************
//  NetQoS   version:  1.0   ·  date: 04/02/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines the structures that get passed around when a
//			party host wants the party peres to join a specific
//			race session.
// ***************************************************************

#ifndef __NET_JOINRACEFROMPARTY_H__
#define __NET_JOINRACEFROMPARTY_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Securekey.h>
#include <bdNet/bdNet.h>
#include <SystemProfile/SystemProfile.h>

namespace RacerNet
{
	/// Passed from the party host to party peers when it's time to join a race session.
	/** The data within here is everything that is needed to join a race session without 
	 *	first issuing a matchmaing search.
	 */
	struct NetJoinRaceFromPartyData
	{
		NetJoinRaceFromPartyData() : m_Active(false), m_PartyHostConsoleId(0), m_RaceHostConsoleId(0), m_MaxPlayers(0), m_PublicGame(0), m_Ranked(0), m_CustomRace(0), m_EventID( 0 )
		{}
		void Clear(void) { m_Active = false; }

		bool m_Active;
		U64 m_RaceHostConsoleId;
		U64 m_PartyHostConsoleId;
		NetPlay::SessionSecurity m_Security;
		SysProfile::BaseProfile m_HostName;
		U8 m_HostTextAddress[BD_COMMON_ADDR_SERIALIZED_SIZE];
		U8 m_MaxPlayers:6;
		U8 m_PublicGame:1;
		U8 m_Ranked:1;
		U8 m_CustomRace:1;
		U32 m_EventID;
	};
} // end namespace RacerNet

#endif // __NET_JOINRACEFROMPARTY_H__
