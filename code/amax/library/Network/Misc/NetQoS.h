// ***************************************************************
//  NetQoS   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines the structures that get passed around when a client
//			initaiates a QoS scan of a host.
// ***************************************************************
 

#ifndef __NET_QOS_H__
#define __NET_QOS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "NetRaceSettings.h"
#include "NetRaceSettingsQos.h"
#include "../Router/NetRouterDefines.h"
#include <Systemprofile/SystemProfile.h>

namespace RacerNet
{
	/// base structure for QoS packet
	struct NetQoSBase
	{	
		/// enum for differentiating between the different types of sessions we can create, race and Party.
		/** Since parties can only be joined from presence the only way the game knows its a party session to join is by
		 *	passing a flag in the QoS data during the the initial host ping.
		 */
		typedef enum NetQoSType
		{
			QoSTypeUnknown = 0,
			QoSTypeRace,
			QoSTypeParty,
		};

		/// enum to difine the different states the session can be in.. used to give the player joining a bit more info before deciding which session to join.
		typedef enum NetQoSState
		{
			QoSStateUnknown = 0,
			QoSStateInLobby,
			QoSStateInRace,
		};

		NetQoSBase(const NetQoSType q) :	m_Type(q), m_Version(0), m_HostConsoleId(0), m_MaxPlayers(0), 
											m_NumSLotsStillOpen(0), m_PublicGame(1), m_PackVersion(0)
		{}
		U32						m_Version;							
		NetQoSType				m_Type;
		SysProfile::BaseProfile	m_Host;
		U64						m_HostConsoleId;
		U16						m_MaxPlayers:7;
		U16						m_NumSLotsStillOpen:6;
		U16						m_PublicGame:1; 
		S32						m_PackVersion;
	};

	/// QoS packet that the client will receive when pinging a host of a race session
	struct NetQoSRace : NetQoSBase
	{
		NetQoSRace() :	NetQoSBase(NetQoSBase::QoSTypeRace), 
						m_SessionState(QoSStateInLobby), m_MatchingRegion(eMatchingRegionUnknown)
		{}

		// current status of session, in lobby or racing.
		NetQoSState			m_SessionState;

		// matchmaking region of the host player.
		eMatchingRegion		m_MatchingRegion;

		// curent race settings.
		NetRaceSettingsQos	m_Race;

		// what dlc is available / in use currently
		NetRaceDlcSettings	m_DlcSettings;

		// overloaded operators so we can compare structures.
		bool operator == (const NetQoSRace& other) const { return Compare(other); } 
		bool operator != (const NetQoSRace& other) const { return !Compare(other); }
		bool Compare(const NetQoSRace& other) const
		{
			// checks closest to the top are the ones that change the most.
			if (other.m_DlcSettings.m_AdditionalContentPacksAvailable != m_DlcSettings.m_AdditionalContentPacksAvailable ||
				other.m_DlcSettings.m_AdditionalContentPacksInUse != m_DlcSettings.m_AdditionalContentPacksInUse ||
				other.m_NumSLotsStillOpen != m_NumSLotsStillOpen ||
				other.m_HostConsoleId != m_HostConsoleId ||
				other.m_MaxPlayers != m_MaxPlayers ||
				other.m_SessionState != m_SessionState ||
				other.m_Version != m_Version ||
				other.m_Type != m_Type ||
				other.m_PublicGame != m_PublicGame ||
				other.m_Race != m_Race ||
				other.m_PackVersion != m_PackVersion ||
				other.m_MatchingRegion != m_MatchingRegion)
				return false;

			return true;
		}
	};

	/// QoS packet that the client will receive when pinging a host of a race session
	struct NetQoSParty : NetQoSBase
	{
		NetQoSParty() :	NetQoSBase(NetQoSBase::QoSTypeParty)	
		{}

		// TODO -> Any Data here that relates only to party sessions
	};

} // end namespace RacerNet

#endif // __NET_QOS_H__
