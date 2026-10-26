#ifndef NETENUMS_H
#define NETENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netstate\StateEnums.h"

typedef U64				netUserId;
typedef char			netTicket;
typedef U64				netNonce;
typedef unsigned int	netHash;

enum 
{ 
	NET_NONCE_NOT_SET	= 0,
};

enum
{
	NET_MAX_PADS				= 4,										// max local pads
	NET_PARTY_SIZE				= 8,										// max players in party
	NET_MAX_PLAYERS				= 16,										// max players in game
	NET_MAX_PARTIES				= 16,										// max parties in game (same as number of players).
	NET_MAX_PLAYER_STATS		= NET_MAX_PLAYERS + NET_MAX_PLAYERS / 2,	// buffer 4 players that have left
	NET_MAX_CONNECTIONS			= NET_MAX_PLAYERS,							// maximum one connection per player, plus an extra 8 for refused connections.
	NET_PLAYER_NAME_LENGTH		= 32,
	NET_MAX_FRIENDS				= 100,
	NET_MAX_ENUMERATED_SESSIONS = 64,
};

#endif //NETENUMS_H