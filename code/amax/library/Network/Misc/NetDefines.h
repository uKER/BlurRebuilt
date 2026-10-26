// ***************************************************************
//  NetDefines   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	generic defines for the network systems.
// ***************************************************************

#ifndef __NET_DEFINES_H__
#define __NET_DEFINES_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>
#include <btl/Containers/vector.h>
#include <btl/Containers/list.h>
#include <btl/Allocators/pool.h>
#include <bdCore/bdSocket/bdSecurityID.h>

namespace RacerNet
{
	#ifndef CFG_GOLD
	// define this to disable player dropouts from the game
	//#define NO_DROPOUTS		
	#endif

	/// if defined then the network packet routing will enable peer relay mode. this is an attempt to get around NAT connection issues.
	/// if this is not defined then the normal network packet routing will run.

	/// defining this means that the searching and invite code will check player privileges
	//#define LSP_GROUPS_IS_CUSTOM_CONTENT

	// largest unreliable datagram that the game should attempt to send.
	// should not be bigger than BD_MAX_DATAGRAM_SIZE
	const U32 NET_MAX_DATAGRAM_SIZE	= (1024);

	/// defines maximum buffer size for a single player voice queue.
	const U32 NET_VOICE_BUFFER_SIZE	=	(200);

	// maximum number of vehicles allowed in network races, can be lower than what the single player uses but not larger.
	const U8 NET_MAX_VEHICLES = (20);

	// maximum number of players we can allow in a party, can never be greater then NET_MAX_VEHICLES.
	const U8 NET_MAX_PARTY_MEMBERS = (8);

	// how many players can be on a single console.
#if !defined (PLATFORM_PC)
	const U8 NET_MAX_LOCAL_PLAYERS = (1);
#else
	const U8 NET_MAX_LOCAL_PLAYERS = (1);
#endif

	// how many seperate teams can we have in network race.
	const U8 NET_MAX_TEAMS = (2);

	// team a index
	const U8 NET_TEAM_A = 0;

	// team b index
	const U8 NET_TEAM_B = 1;

	// the number that we start assigning AI vehicles their "join IDs" from (must be higher than any real player can be assigned)
	const U8 NET_AI_STARTING_JOIN_ID = (50);

	// how many seconds do we wait while joining the session before we say connection cant be established.
	const float NET_MAX_JOIN_TIME = (15.0f);

	// how many seconds do we wait while joining the session before we say connection cant be established.
	// this version is used if we have enabled peer relay of data in the router.
	const float NET_MAX_JOIN_TIME_WITH_PEER_ROUTING = (30.0f);

	// how many seconds the host wait before dropping a peer
	const float NET_HOST_DROPOUT_TIMER = (4.0f);

	// how many seconds does the peer wait before dropping another peer or informing the host.
	const float NET_PEER_DROPOUT_TIMER = (6.0f);

	// how often do we wish to send keepalive packets to remote peers, under normal conditions.
	const float	NET_KEEPALIVE_NORMAL_FREQUENCY = (0.5f);

	// how often do we wish to ask the host to disconnect a console we have lost connection to.
	const float	NET_DISCONNECT_RESEND_TIMER = (3.0f);

	// how often to send voice that has been queued up.
	const float NET_DEFAULT_VOICE_SEND_TIME_IN_SECONDS = (0.1f);

	// how often to update the players waiting in lobby during race-in-progress with the race info
	const F32 NET_LOBBY_WAITING_INFO_FREQUENCY = (.5f);

	// how big will each remote players inbox be.
	const U32 NET_MAX_INBOX_SIZE = (1024*6);

	// how big will each remote players outbox be.
	const U32 NET_MAX_OUTBOX_SIZE = (1024*12);

	// the base time between sending vehicle updates - we don't go any longer than this but can send more frequently.
	const U32 NET_VEHICLE_UPDATE_TIME = (100);

	// defines the lowest bandwidth threshold we recognise for online play.
	const U16 NET_MINIMUM_BANDWIDTH_KBPS = (64);

	// defines the bandwidth to assume for network races (10mbps)
	const U16 NET_LAN_BANDWIDTH_KBPS = (10*1024);

	// defines a small delay before we issue commands to the matching server.
	const F32 NET_MATCHING_SERVER_UPDATE_DELAY = (0.75f);

	// defines a small delay before we issue commands to the matching server.
	const F32 NET_MATCHING_SERVER_UPDATE_DELAY_QUICK = (0.001f);

	// list of player identifiers for a particular side.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES/2> > NetRaceTeamMembers;

	// list of players seperated into teams.
	typedef Btl::Vector<NetRaceTeamMembers, Btl::VectorParameters_Static<NET_MAX_TEAMS> > NetRaceTeams;

	// list of join references in a single stack.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > NetJoinReferences;

	// for tracking a list of unique console Ids
	typedef Btl::Vector<U64, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > NetConsoleIds;

	// list of join references but only containing players from a single console.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_LOCAL_PLAYERS> > NetJoinReferencesSingleConsole;

	// number of bytes each local player has buffered up for a specific peer
	typedef Btl::Vector<U8, Btl::VectorParameters_Array<NET_MAX_LOCAL_PLAYERS> > NetLocalVoiceSizes;

	// list of security keys Ids so we can track what is active.
	typedef Btl::Vector<bdSecurityID, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > SecurityKeyIds;

	// list of join references that represent the players in a session who we have muted.
	typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES-1> > NetMuteListJoinReferences;

	// title storage variables

	// the amount of time before the title storage files are downloaded again in seconds
	#define NUM_SECONDS_BETWEEN_TITLESTORAGE_REFRESH 3600 // Must be over 1 hour for TCR 063
	
	// time to wait on net-connect pop-up
	#define NUM_SECONDS_WAITING_FOR_INIT_TIMEOUT 5

	// title storage download buffer size
	const int TITLE_STORAGE_BUFFER_SIZE = (256 * 1024);
		
	#define	TITLE_STORAGE_LOC_UI_TABLE "MultiplayerTS"

	// defines how lan session enumeration will filter out other platforms and versions.
#if defined (PLATFORM_PS3)
	const U32 AMAX_LAN_ID =  (0x0000002B);
#elif defined (PLATFORM_PC)
	const U32 AMAX_LAN_ID =  (0x0000002C);
#else
#endif

} // end namespace RacerNet

#endif // __NET_DEFINES_H__
