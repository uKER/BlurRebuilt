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

#ifndef __NET_RACE_SETTINGS_H__
#define __NET_RACE_SETTINGS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "NetDefines.h"
#include "UI/Game/Multiplayer/MultiplayerDefines.h"

namespace RacerNet
{
	const U8 NET_MAX_RACES = 10;
	const U8 NET_RACE_MAX_RULES = 4;

	struct Rule
	{
		U32	m_xtRuleId;
		S32	m_Value;
	};

	struct RaceSettings
	{
		RaceSettings()
		{
			Clear();
		}

		void Clear()
		{
			m_xtCityId		= 0;
			m_xtRouteId		= 0;

			m_MinPlayers	= 2;
			m_MaxPlayers	= 20;

			m_Type			= 0;
			m_NumLaps		= 1;
			m_TimeLimit		= 0;

			m_Class			= 0;
			m_DamageMode	= 0;
			m_Respawn		= 0;
			m_Timeout		= 0;
			m_Mods			= 0;
			m_Upgrades		= 0;
			m_LockToHostCar = 0;
			m_AI			= 0;
			m_AIMaxPadding	= 0;
			m_Handicap		= 0;

			m_Pickups			 = 0xffff;
			m_RandomPickups		 = 0;
			m_PickupsRespawnMode = 0;
		}

		U32	m_xtCityId;				// Currently chosen city
		U32	m_xtRouteId;			// Currently chosen route

		U8	m_MinPlayers;			// Minimum number of players needed for race
		U8	m_MaxPlayers;			// Maximum number of players needed for race

		S8	m_Type;					// Current race mode
		S8	m_NumLaps;				// Currently chosen number of laps
		S32	m_TimeLimit;			// Currently chosen time limit

		S8	m_Class;				// Car class for this race
		S8	m_DamageMode;			// Damage mode for this race
		S8	m_Respawn;				// Weather cars respawn after wrecking
		S8	m_Timeout;				// Timeout on/off
		S8	m_Mods;					// Mods on/off/lock to host
		S8	m_Upgrades;				// Updates on/off/lock to host
		S8	m_LockToHostCar;		// force clients to use host car on/off
		S8	m_AI;					// ai level off/easy/medium/hard
		S8	m_AIMaxPadding;			// ai padding
		S8	m_Handicap;				// handicap level off/easy/medium/hard

		U16	m_Pickups;				// Pickup bitfield on/off
		S8	m_RandomPickups;		// Random powerups on/off
		S8	m_PickupsRespawnMode;	// Powerup respawn mode
	};

	/// Holds all the settings for the current network race.
	/** This structure is replicated onto the peers at join time, and when it changes in custom race mode.
	 */
	struct NetRaceSettings
	{
		NetRaceSettings()
		{ 
			Clear(); 
		}

		void Clear()
		{
			m_PackVersion			= 0;

			m_EventId				= 0;
			m_CustomRace			= 0;
			m_PublicGame			= 1;
			m_Ranked				= 0;
			m_Championship			= 0;

			m_Teams					= 0;
			m_MinDrivers			= 2;
			m_MaxDrivers			= NET_MAX_VEHICLES;
			m_FanMultiplier			= 1.0f;
			m_Misc					= 0xff;

			m_HasAnyRacesStarted	= 0;

			m_LobbyState			= 0;
			m_LobbyTimer			= 0.0f;
			m_VoteCountA			= 0;
			m_VoteCountB			= 0;
			m_VoteIndexA			= -1;
			m_VoteIndexB			= -1;

			m_PlaylistSeed			= 0;
			m_PlaylistIndex			= 0;
		}

		U16 m_PackVersion;			//pack version for multiplayer play lists, can't have someone playing someone with the wrong playlist number

		U32 m_Name;
		U32 m_Description;

		// Taken from custom race
		U32	m_EventId;				// Sorcery event ID, or server event ID
		U8	m_CustomRace:1;			// User's custom race or playlist
		U8	m_PublicGame:1;			// defines how the session is created on the matching servers, 0 = friends only, 1 = session will be returned in matching queries.
		U8	m_Ranked:1;				// is it the special lets break all TCR's and live functionality mode.
		U8	m_Championship:1;		// is it a championship

		// General settings
		U8	m_Teams:1;				// Teams on/off
		U8	m_MinDrivers:5;			// Minimum number of drivers allowed
		U8	m_MaxDrivers:5;			// Maximum number of drivers allowed
		F32 m_FanMultiplier;		// Fan multiplier for double xp weekends

		// General
		U8	m_HasAnyRacesStarted:1;	// goes to one onwe have started any race.
		U16	m_Misc;					// voting, shield, repair etc

		S8	m_LobbyState;			// Current state of the lobby as we join
		F32	m_LobbyTimer;			// Time of any countdown if acitve
		S8	m_VoteCountA;			// Current votes for A
		S8	m_VoteCountB;			// Current votes for B
		S16	m_VoteIndexA;			// Current race index for A
		S16	m_VoteIndexB;			// Current race index for B

		U32 m_PlaylistSeed;			// The seed used to shuffle the playlist
		S16	m_PlaylistIndex;		// Current race index for the current playlist
		
		RaceSettings m_Race;		// Current race settings
	};

	/// Holds data on what dlc content packs are available and which packs are currently in use by any peer.
	/** A peer can validate his own vehicle based on the current NetRaceSettings and what dlc is in use.
	 */
	struct NetRaceDlcSettings
	{
		NetRaceDlcSettings()
		{ Clear(); }
			
		void Clear(void)
		{
			m_AdditionalContentPacksAvailable = 0;
			m_AdditionalContentPacksInUse = 0;
		}

		// each bit represents a content pack the can be used by all players currently in the lobby.
		// if the local player has content that is not listed in here then they can't select it as not all player have access to it.
		U32 m_AdditionalContentPacksAvailable;

		// each bit represents a content pack that is in use, that is some one in the lobby is using some content from that pack.
		// before joining we should make sure that we have access to all content that is in use otherwise the join request will fail.
		U32	 m_AdditionalContentPacksInUse;
	};

} // end namespace RacerNet

#endif // __NET_RACE_SETTINGS_H__
