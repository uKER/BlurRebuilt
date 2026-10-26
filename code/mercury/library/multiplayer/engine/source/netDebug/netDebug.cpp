#include "stdafx.h"
#include "netDebug\netDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

Debug::Logger& netDebug::g_Log				= Debug::GetLogger("netdebug.root");
Debug::Logger& netDebug::g_SendPacketLog	= Debug::GetLogger("netdebug.packet.outgoing");
Debug::Logger& netDebug::g_RecvPacketLog	= Debug::GetLogger("netdebug.packet.incoming");
Debug::Logger& netDebug::g_sessionLog		= Debug::GetLogger("netdebug.session");
Debug::Logger& netDebug::g_MigrationLog		= Debug::GetLogger("netdebug.migration");
Debug::Logger& netDebug::g_ArbitrationLog	= Debug::GetLogger("netdebug.arbitration");
Debug::Logger& netDebug::g_MediatorLog		= Debug::GetLogger("netdebug.mediator");
Debug::Logger& netDebug::g_ReplicatorLog	= Debug::GetLogger("netdebug.replicator");

unsigned int	netDebug::dbg_build						= 0;
bool			netDebug::dbg_partyDiscovery			= true;
bool			netDebug::dbg_enableRLEEncoder			= true;
bool			netDebug::dbg_debugRLEEncoder			= false;
bool			netDebug::dbg_enableEntityMarkers		= true;

DEBUG_ENUM(E_DbgRnd)
	DE_OPTION(E_DbgRnd_None,		"Off")
	DE_OPTION(E_DbgRnd_GameState,	"GameState")
	DE_OPTION(E_DbgRnd_Game,		"Game")
	DE_OPTION(E_DbgRnd_Party,		"Party")
	DE_OPTION(E_DbgRnd_System,		"System")
	DE_OPTION(E_DbgRnd_Migration,	"Migration")
	DE_OPTION(E_DbgRnd_Matchmaking,	"Matchmaking")
END_DEBUG_ENUM

DEBUG_ENUM(E_State_Type)
	DE_OPTION(E_State_GameWorld,	"GameWorld")
	DE_OPTION(E_State_GameLobby,	"GameLobby")
	DE_OPTION(E_State_PartyLobby,	"PartyLobby")
END_DEBUG_ENUM

DEBUG_ENUM(E_DbgMode)
	DE_OPTION(E_DbgMode_State,			"State")
	DE_OPTION(E_DbgMode_Bandwidth,		"Bandwidth")
	DE_OPTION(E_DbgMode_BandwidthGUI,	"BandwidthGUI")
	DE_OPTION(E_DbgMode_Memory,			"Memory")
	DE_OPTION(E_DbgMode_EntityData,		"EntityDebug")
END_DEBUG_ENUM

DEBUG_ENUM(E_DbgNAT_Type)
	DE_OPTION(E_DbgNAT_NotSet,			"NotSet")
	DE_OPTION(E_DbgNAT_Open,			"Open")
	DE_OPTION(E_DbgNAT_Moderate,		"Moderate")
	DE_OPTION(E_DbgNAT_Strict,			"Strict")
END_DEBUG_ENUM

// general net
Debug::Enum<E_DbgRnd>			netDebug::dbg_renderMode					("Multiplayer.DebugRenderMode", E_DbgRnd_None);
	
// matchmaking and stats
Debug::Bool						netDebug::dbg_matchMakingWaitForCompletion	("Multiplayer.Stats.MatchmakingWaitForCompletion",	true);
Debug::Bool						netDebug::dbg_randomiseStats				("Multiplayer.Stats.RandomiseStats", false);
Debug::Bool						netDebug::dbg_enableCustomGameSearch		("Multiplayer.Stats.EnableCustomGameSearch", false);

// packet and transport
Debug::Bool						netDebug::dbg_buildOverride					("Multiplayer.Transport.BuildStampOverride", false);
Debug::Enum<E_DbgNAT_Type>		netDebug::dbg_NAToverride					("Multiplayer.Transport.NATOverride", E_DbgNAT_NotSet);
Debug::Bool						netDebug::dbg_debugPacketMisalignments		("Multiplayer.Transport.DebugPackets", false);
Debug::Bool						netDebug::dbg_voiceCache					("Multiplayer.Transport.VoiceCache", true);
Debug::Bool						netDebug::dbg_stayConnected					("Multiplayer.Transport.StayConnectedToServices", false);

// game state
Debug::Enum<E_State_Type>		netDebug::dbg_RenderGameState				("Multiplayer.State.RenderState", E_State_GameWorld);
Debug::Enum<E_DbgMode>			netDebug::dbg_renderGameStateMode			("Multiplayer.State.RenderStateMode", E_DbgMode_State);
Debug::Bool						netDebug::dbg_useDeltaCompression			("Multiplayer.State.DeltaCompression", true);
Debug::Int						netDebug::dbg_entityDebugStartPoint			("Multiplayer.State.DebugEntityStartPoint",	0, Debug::Range(0, g_uMaxGameWorldEntities));


enum
{
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
};

void netDebug::log(Debug::Logger& logger, int level, const char* format, va_list args)
{
	char buffer[512];
	vsprintf_s(buffer, sizeof(buffer), format, args);
	
	switch(level)
	{
	default:
	case LOG_LEVEL_DEBUG: 
		LOGDEBUG(logger, buffer); 
		break;
	case LOG_LEVEL_INFO: 
		LOGINFO(logger, buffer); 
		break;
	case LOG_LEVEL_WARN:
		LOGWARN(logger, buffer); 
		break;
	case LOG_LEVEL_ERROR:
		LOGERROR(logger, buffer); 
		break;
	case LOG_LEVEL_FATAL:
		LOGFATAL(logger, buffer); 
		break;
	}	
}

const char* netDebug::getByteInfoShort(const void* ptr, int byteCount, char* str, int sizeofStr)
{
	const unsigned char* bytes = (const unsigned char*)ptr;

	switch(byteCount)
	{
	default:
		sprintf_s(	str, sizeofStr, "%02x.%02x.%02x.%02x...", (bytes[0] & 255), (bytes[1] & 255), (bytes[2] & 255), (bytes[3] & 255));
		return str;
	case 1:
		sprintf_s(	str, sizeofStr, "%02x", (bytes[0] & 255));
		return str;
	case 2:
		sprintf_s(	str, sizeofStr, "%02x.%02x", (bytes[0] & 255), (bytes[1] & 255));
		return str;
	case 3:
		sprintf_s(	str, sizeofStr, "%02x.%02x.%02x", (bytes[0] & 255), (bytes[1] & 255), (bytes[2] & 255));
		return str;
	case 4:
		sprintf_s(	str, sizeofStr, "%02x.%02x.%02x.%02x", (bytes[0] & 255), (bytes[1] & 255), (bytes[2] & 255), (bytes[3] & 255));
		return str;
	}
}

const char* netDebug::getByteInfo(const void* ptr, int ptrlen)
{
	static char buffer[4][100];
	static int  bufferptr = 0;
	
	char* buff = buffer[bufferptr];
	
	if(!getByteInfo(ptr, ptrlen, buff, 80))
	{
		return "";
	}

	bufferptr  = (bufferptr + 1) % 3;
	return buff;
}

const char* netDebug::getByteInfo(const void* ptr, int ptrlen, char* str, int len)
{
	bool clipped = false;
	int chr = 0;
	for(int i = 0; i < ptrlen; i++)
	{
		const char characters[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

		const char val	= ((const char*)ptr)[i];
		const bool last = (i >= ptrlen-1);
		int c0	= (val & 0x0F);
		int c1	= (val & 0xF0) >> 4;

		if(chr < len-1) { str[chr++] = characters[c1]; } else { clipped = true; }
		if(chr < len-1) { str[chr++] = characters[c0]; } else { clipped = true; }
		if(!last) { if(chr < len-1) { str[chr++] = '.'; } else { clipped = true; } }
		if(clipped) break;
	}

	// null terminate
	if(len >= 1) str[len-1] = 0;
	
	// we couldn't print everything, put '...' at the end.
	if(clipped)
	{
		if(len >= 2) str[len-2] = '.';
		if(len >= 3) str[len-3] = '.';
		if(len >= 4) str[len-4] = '.';
	}
	return str;
}



#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

