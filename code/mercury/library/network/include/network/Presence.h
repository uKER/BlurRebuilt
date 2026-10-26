// ***************************************************************
//  Presence   version:  1.0   ·  date: 09/11/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Rich presence structure. On PS3 we can try and 
//			replicate the rich presence features of xbox360. 
//			Looks like demonware dont allow rich presence so this
//			can't be used on PC
// ***************************************************************

#ifndef NETWORK_PRESENCE_H
#define NETWORK_PRESENCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdSocket/bdSecurityID.h>

namespace RacerNet
{
/// game defines that relate to what the user is doing within the game at the present time.
	/** on xbox 360 these will also have to have xlast data to make them work.
	 */
	typedef enum eRichPresence
	{
		//	Global
		eRichPresence_SignedIn = 0,			//	Just getting started - not chosen SP or MP yet
		eRichPresence_Idle,				//	Inactive pad

		//	SP/MP presence
		eRichPresence_Stickerbook,		//	Checking out Stickers
		eRichPresence_Leaderboards,		//	Leaderboards (SP or MP)
		eRichPresence_Help,				//	Help menus
		eRichPresence_Showroom,			//	In the showroom - SP or MP
		eRichPresence_InRace,			//	<TIER NAME> - event description ||
										//	Friend Challenges - challenge description ||
										//	Xbox LIVE - <RANK> - race description
										
		//	SP presence
		eRichPresence_SpMain,			//	Main SP carousel
		eRichPresence_Photos,			//	Single Player - Checking out Photos
		eRichPresence_FriendChallenges,	//	Friend Challenges - Reviewing Challenges
		eRichPresence_SpCareer,			//	Single Player - Checking out Career
		eRichPresence_Sp_Choosing,		//	<TIER NAME> - Choosing an event

		//	MP presence
		eRichPresence_MpMain,			//	Main MP carousel
		eRichPresence_SplitScreen,		//	Multiplayer - <X> player Split Screen
		eRichPresence_RaceHistory,		//	Multiplayer - Race History
		eRichPresence_LAN,				//	Multiplayer - System Link
		eRichPresence_OnlineMain,		//	Xbox LIVE - Choosing what to do
		eRichPresence_ModShop,			//	Xbox LIVE - Checking out Mods
		eRichPresence_Challenges,		//	Xbox LIVE - Checking out Challenges
		eRichPresence_MpLobby,			//	Xbox LIVE - Getting ready to race

		//	These are the horrible PS3-specific enum values that we simplify to instead of doing the awesome XLAST stuff with 360.
		//	This list contains duplicates, but at the end of the day the PS3 only has to read these at the other side on user's Friends List screens.
		//	Still gives us slightly more detail than just "in a race".
		eRichPresencePS3_SpBoss,
		eRichPresencePS3_SpRace,
		eRichPresencePS3_SpDestruction,
		eRichPresencePS3_SpCheckpoint,
		eRichPresencePS3_FC_Challenge,
		eRichPresencePS3_RC_ReChallenge,
		eRichPresencePS3_LAN,
		eRichPresencePS3_OnlineRacing,
		eRichPresencePS3_OnlineMM,
		eRichPresencePS3_OnlinePrivate,
	};
}

namespace NetPlay
{
	/// Base structore for rich presence.
	/** Used for version checking. 
	 */
	struct RichPresenceBase
	{
		RichPresenceBase() {m_Version = 0;}
		U32 m_Version;
	};

	/// Structure for rich presence.
	/** On Xbox 360 presence is defined within XLAST, the Guide is responsible for rendering the presence.
	 *	On PS3 / PC the presence data will need to be game defined as it's the game who will interpret and render the 
	 *	presence data. When a friend is enumerated on PS3 / PC the presence data will be attached.
	 *	For join in progress to work correctly on PS3 / PC the player will have to update his presence when they
	 *	join / leave a session or the session becomes full. On Xbox 360 the guide handles all this for us.
	 */
	struct RichPresence : RichPresenceBase
	{
		RichPresence()
		{
			/*m_ContextId = 0;*/
			m_Index = 0;
			/*m_PresenceDisplayId = 0;*/
			m_JoinInProgressPossible = false;

		}

		//U32 m_ContextId;								/**< defines the current context for the presence as defined by XLAST when using Xbox360. */
		U32 m_Index;									/**< the index into current context that specified the current presence value. */
		//U32 m_PresenceDisplayId;						/**< Only valid on Xbox 360. The Presence Id within XLAST that's used to render the current presence string to the Guide. */
		bool m_JoinInProgressPossible;					/**< can we join the friends session ?*/
		bdSecurityID m_OnlineSessionId;					/**< Session ID to search for via matchmaking if joinable. */

	};
}

#endif


