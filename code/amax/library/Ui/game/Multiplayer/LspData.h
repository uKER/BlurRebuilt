
#ifndef UI_LSP_DATA_H
#define UI_LSP_DATA_H

#include "Ui/Game/Multiplayer/MultiplayerDefines.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Struct	: LspRace

	Desc	: Actual data format for the event race on the content server
---------------------------------------------------------------------------------------*/
struct LspRace
{
	U32					city_id;
	U32					route_id;

											// Variables mean different things based on the game mode so
											// we dont have lots of variables.
	S32					race_mode;			// Race Mode

	U8					laps;				// Racing		: Number of laps to complete
											// Points		: Max laps to complete

	S32					time;				// Racing		: Max time limit
											// Elimintation : Elimination timer
											// Points		: Max time limit

	S32					score_limit;		// Points		: Score Limit
	U32					score_rules;		// Points		: The rules to determine what you earn points for

	U16					elimination_event;	// Elimination	: When to eliminate
	U16					elimination_type;	// Elimination	: Why to eliminate


	U32					reserved_1;			// reserved just incase we need extra data
	U32					reserved_2;
	S32					reserved_3;
	S32					reserved_4;
};

/*---------------------------------------------------------------------------------------
	Struct	: LspEvent

	Desc	: Actual data format for the events on the content server
---------------------------------------------------------------------------------------*/
struct LspEvent
{
	U32					version;						// File version

	WCHAR				name[MAX_EVENT_NAME];			// User name of the event
	WCHAR				description[MAX_EVENT_DESC];	// User description of the event
	WCHAR				owner[MAX_OWNER_NAME];			// Creators gamertag
	U64					owner_id;						// Creators platform user id

	U8					max_players;					// Maximum number of players
	U8					teams;							// Number of teams, 0(Off) - 4
	U8					classes;						// Current car classes
	U32					misc;							// Misc settings. Voting, Shield, Repair
	U16					damage_mode;					// Damage mode
	U16					powerups;						// Powerups active Use : E_Pickup
	U8					powerup_frequency;				// Amount of pickups on track
	U8					powerup_mode;					// Powerups mode Use : E_PowerupMode

	U8					num_races;						// Races size
	LspRace				races[MAX_EVENT_RACES];			// Races in the event.


	U32					reserved_1;						// reserved just incase we need extra data
	U32					reserved_2;
	S32					reserved_3;
	S32					reserved_4;
};

};//namespace UI

#endif // UI_LSP_DATA_H