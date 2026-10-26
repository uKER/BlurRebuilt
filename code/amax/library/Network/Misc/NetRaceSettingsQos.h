// ***************************************************************
//  NetRaceSettings   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines the currect race setting for the network race session.
// ***************************************************************

#ifndef __NET_RACE_SETTINGS_QOS_H__
#define __NET_RACE_SETTINGS_QOS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "NetDefines.h"

namespace RacerNet
{
	/// Holds a subset of the race the settings for the current network race.
	/** This structure is sent around in the QoS data stream when matching / systemlink games are enumerated.
		It's used to render the UI.
	 */
	struct NetRaceSettingsQos
	{
		NetRaceSettingsQos() :	m_EventID(0), m_PackVersion( 0 ), m_Ranked( 0 ), m_CustomRace( false )
		{}

		// overloaded operators so we can compare structures.
		bool operator == (const NetRaceSettingsQos& other) const { return Compare(other); } 
		bool operator != (const NetRaceSettingsQos& other) const { return !Compare(other); }
		bool Compare(const NetRaceSettingsQos& other) const
		{
			if (m_EventID != other.m_EventID || m_CustomRace != other.m_CustomRace || m_Ranked != other.m_Ranked || m_PackVersion != other.m_PackVersion )
				return false;

			return true;
		}

		U32		m_EventID;			// Sorcery event ID
		U16		m_PackVersion;		// Sorcery pack version
		U8		m_Ranked:1;			// Online ranked
		U8		m_CustomRace:1;		// Custom race flag
	};

} // end namespace RacerNet

#endif // __NET_RACE_SETTINGS_QOS_H__
