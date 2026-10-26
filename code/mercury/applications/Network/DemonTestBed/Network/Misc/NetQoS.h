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

		NetQoSBase(const NetQoSType q) : m_Type(q), m_Version(0), m_NumSLotsStillOpen(0)
		{}
		U32						m_Version;							
		NetQoSType				m_Type;
		SysProfile::BaseProfile	m_Host;
		S32						m_NumSLotsStillOpen;
	};

	/// QoS packet that the client will receive when pinging a host of a race session
	struct NetQoSRace : NetQoSBase
	{
		NetQoSRace() :	NetQoSBase(NetQoSBase::QoSTypeRace), m_SessionState(QoSStateInLobby), m_RaceInProgress(false), 
						m_DlcContentPacksAvailableForUse(0), m_DlcContentPacksCurrentlyInUse(0)
		{}

		// current status of session, in lobby or racing.
		NetQoSState			m_SessionState;

		// actual race settings..
		NetRaceSettings		m_RaceSettings;

		U32					m_DlcContentPacksAvailableForUse;	// what dlc packs can the session actually use 
		U32					m_DlcContentPacksCurrentlyInUse;	// what dlc packs are currently in use by any player in the session.
		U8					m_RaceInProgress;						// are the players currently racing.
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
