#ifndef NETDEBUG_H
#define NETDEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netstate\StateEnums.h"

enum E_DbgRnd
{
	E_DbgRnd_None,
	E_DbgRnd_GameState,
	E_DbgRnd_Game,
	E_DbgRnd_Party,
	E_DbgRnd_System,
	E_DbgRnd_Migration,
	E_DbgRnd_Matchmaking,
	E_DbgRnd_Count
};

enum E_DbgNAT_Type
{
	E_DbgNAT_NotSet,
	E_DbgNAT_Open,
	E_DbgNAT_Moderate,
	E_DbgNAT_Strict,
};

enum E_DbgMode
{
	E_DbgMode_State,
	E_DbgMode_Bandwidth,
	E_DbgMode_BandwidthGUI,
	E_DbgMode_Memory,
	E_DbgMode_EntityData,
	E_DbgMode_Count,
};

class netDebug
{
public:
	static Debug::Logger& g_Log;
	static Debug::Logger& g_SendPacketLog;
	static Debug::Logger& g_RecvPacketLog;
	static Debug::Logger& g_sessionLog;
	static Debug::Logger& g_MigrationLog;
	static Debug::Logger& g_ArbitrationLog;
	static Debug::Logger& g_MediatorLog;
	static Debug::Logger& g_ReplicatorLog;

	template<typename T> static char getByte(const T& data, int byte) { return ((const char*)(&data))[byte]; }

	static char		   getByteInfo(int integer, int byte);
	static const char* getByteInfoShort(const void* bytes, int byteCount, char* str, int sizeofStr);
	static const char* getByteInfo(const void* ptr, int ptrlen, char* str, int len);
	static const char* getByteInfo(const void* ptr, int ptrlen);

	void log(Debug::Logger& logger, int level, const char* format, va_list args);

	// debug console
	bool getConsoleName(char* name, int len);
	bool getConsoleName(WCHAR* name, int len);
	
	static Debug::Bool							dbg_voiceCache;
	static Debug::Enum<E_DbgMode>				dbg_renderGameStateMode;
	static Debug::Enum<E_DbgRnd>				dbg_renderMode;
	static Debug::Enum<E_State_Type>			dbg_RenderGameState;
	static Debug::Bool							dbg_useDeltaCompression;
	static Debug::Int							dbg_entityDebugStartPoint;
	static Debug::Bool							dbg_matchMakingWaitForCompletion;
	static Debug::Bool							dbg_buildOverride;
	static Debug::Bool							dbg_enableCustomGameSearch;
	static Debug::Enum<E_DbgNAT_Type>			dbg_NAToverride;
	static Debug::Bool							dbg_randomiseStats;
	static Debug::Bool							dbg_stayConnected;
	static Debug::Bool							dbg_debugPacketMisalignments;
	
	static  unsigned int	dbg_build;
	static  bool			dbg_partyDiscovery;
	static  bool			dbg_enableRLEEncoder;
	static  bool			dbg_debugRLEEncoder;
	static  bool			dbg_enableEntityMarkers;
};

#endif //NETDEBUG_H