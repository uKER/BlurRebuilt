// ***************************************************************
//  NetPlayerRaceData   version:  1.0   ·  date: 08/19/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Holds all game logic specific data related to the 
//			network race session.
// ***************************************************************

#ifndef __NET_PLAYER_RACE_DATA_H__
#define __NET_PLAYER_RACE_DATA_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/bitvector.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <GameLogic\MultiplayerUtilities.h>

namespace RacerNet
{
	/// Holds data on the currently selected vehicle.
	/** This is everything a remote peer needs to know about a vehicle
	 */ 
	struct NetPlayerVehicle
	{
		NetPlayerVehicle() { Flush(); }

		void Flush(void)
		{
			m_xtVehicleId = 0;
			m_xtColourId = 0;
			m_VehicleContentPack = 0;
			m_AutoGears = 1;

			m_CharacterId = 0;

			m_PickupSlots = 0;
			m_ShieldSlots = 0;
			m_AdditionalHealthPips = 0;
			m_AdditionalHP= 0;
		}

		U32	m_xtVehicleId;				// currently selected vehicle id from sorcery
		U32	m_xtColourId;		// currently selected vehicle colour id from sorcery
		U16	m_VehicleContentPack:15;	// what content pack does the vehicle come from. 0 means shipped with the game otherwise its the dlc pack where this vehicle resides.
		U16 m_AutoGears:1;				// automatic / manual transmission

		U32	m_CharacterId;						// the driver character we should show for this vehicle

		U32 m_PickupSlots			:	16;		//	Total pickup slots on player's vehicle, including any bonuses from vehicle-specific challenges.
		U32 m_ShieldSlots			:	8;		//	Total shield slots
		U32 m_AdditionalHealthPips	:	8;		//	Additional health pips (on top of vehicle's base health).

		U32 m_AdditionalHP          :   1;
	};

	/// Holds data the upgrades/specifications which describe a single network player
	struct NetPlayerLoadOut
	{
		NetPlayerLoadOut() { Flush(); }

		void Flush(void)
		{
			for(int i = 0; i < slots.Size(); i++)
			{
				slots[i] = UI::E_MultiplayerVehicleAbilities_None;
			}
		}

		Btl::Vector< UI::E_MultiplayerVehicleAbilities, Btl::VectorParameters_Array< UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX > > slots;
	};

	/// Holds any game logic data that is specific to a single network player in the race session.
	/** selected vehicle etc..
	 */
	struct NetPlayerRaceGameData
	{
		// defines what the bits of the m_BitArray are used for.
		typedef enum
		{
			eBitArrayData_HasMyLatestVehicleData = 0,	// this player player has not yet been sent this consoles chosen vehicle(s), send them asap. Only applicable to remote primary players.
			eBitArrayData_WantsToSkipNextRace,			// this player has trequested that we skip the next race.
		}eBitArrayData;

		NetPlayerRaceGameData() { Flush(); }

		void Flush(void)
		{
			m_VehicleData.Flush();
			m_LoadOut.Flush();
			m_InRaceVehicleIndex = -1;
			m_BitArray.ClearBits(); 
			m_Team = -1;
		}
		
		NetPlayerVehicle m_VehicleData;	// selected vehicle data.
		NetPlayerLoadOut m_LoadOut;
		S32 m_InRaceVehicleIndex;		// will be assigned when the C_levelState is constructed, used for tracking thbis player in the game logic.
		Btl::BitArray16	m_BitArray;
		S8 m_Team;
	};
} // end namespace RacerNet

#endif //__NET_PLAYER_RACE_DATA_H__
