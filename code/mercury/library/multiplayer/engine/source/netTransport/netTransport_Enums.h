#ifndef MULTIPLAYER_ENUMS_H
#define MULTIPLAYER_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif 

#include <system/debug/log/DebugLog.h>
#include <system/debug/Variables.h>

namespace netTransport
{
	class Debugging
	{
	public:
		static ::Debug::Logger&		g_logger;
		static ::Debug::Float		dbg_bandwidthCap;
		static ::Debug::Bool		dbg_forwardEnabled;
		static ::Debug::Bool		dbg_timeoutEnabled;
		static ::Debug::Bool		dbg_migrationEnabled;
		static ::Debug::Function	dbg_migrate;
		static ::Debug::Function	dbg_outputMigrate;		
		static ::Debug::Int			dbg_unreachableConnection;
		static bool					dbg_useAddressMap;
		static bool					dbg_callMigrate;
		static bool					dbg_callOutputMigrate;
		static bool					dbg_ping;
		static int					dbg_pingId;

		static ::Debug::Bool		dbg_simulatorEnabled;
		static ::Debug::Int			dbg_simulatorLatency;
		static ::Debug::Int			dbg_simulatorPacketLoss;
		static ::Debug::Int			dbg_simulatorBlockConnection;

		static ::Debug::Int			dbg_logPacketSocketPort;
		static ::Debug::Int			dbg_logPacketDestination;
		static ::Debug::Int			dbg_logPacketOrigin;
		static ::Debug::Bool		dbg_logPacketReliable;
		static ::Debug::Bool		dbg_logPacketUnreliable;
		
		static void outputMigrate(void*);
		static void migrate(void*);
	};

	enum 
	{ 
		PACKET_UDP_OVERHEAD = 44, 
		PACKET_MTU = 1306, 
	};

	enum
	{
		CONN_QOS_UNAVAILABLE=-120,	// unknown connection, or not established.
		CONN_QOS_VERY_BAD = -10,
		CONN_QOS_BAD = -4,
		CONN_QOS_AVERAGE = 0,
		CONN_QOS_GOOD = 4,
		CONN_QOS_VERY_GOOD = 100,
	};	
	
	enum
	{
		CONN_STATE_NOT_SET=-1,	// connection's in limbo
		CONN_STATE_CONNECTING,	// connecting to a  host
		CONN_STATE_RESERVED,	// reserved slot
		CONN_STATE_PENDING,		// waiting for accept / close
		CONN_STATE_ACCEPTING,	// hand-shaking
		CONN_STATE_ESTABLISHED,	// send keep alives.
		CONN_STATE_UNRESPONSIVE,// peer not responding.
		CONN_STATE_CLOSING,		// closing connection (closed by local).
		CONN_STATE_CLOSED,		// connection closed or timed out
	};

	enum 
	{
		CLOSE_REASON_NOT_SET = 0,				// no error
		CLOSE_REASON_SYSTEM_ERROR,				// wtf?
		CLOSE_REASON_KICKED,					// kicked from game / party / graph
		CLOSE_REASON_FULL,						// kicked from game / party / graph
		CLOSE_REASON_CLOSED_BY_HOST,			// connection closed by host, so go bye bye too.
		CLOSE_REASON_SHUTDOWN,					// manual shutdown
		CLOSE_REASON_RESET,						// connection was reset
		CLOSE_REASON_UNAVAILABLE,				// unavailable
		CLOSE_REASON_NOT_RESPONDING,			// host is not responding
		CLOSE_REASON_REFUSED,					// refused trhe connection for X reason
		//----------------------------------------------
		CLOSE_REASON_COUNT,
	};

	// logging byte types for throttle and bandwidth report.
	enum
	{
	//---------------------------------
		TRAFFIC_TYPE_NOT_SET=-1,
		TRAFFIC_TYPE_SOCKET,
		TRAFFIC_TYPE_FIRST,
		TRAFFIC_TYPE_FORWARDED=TRAFFIC_TYPE_FIRST,
		TRAFFIC_TYPE_HEADER,
		TRAFFIC_TYPE_SYSTEM,
		TRAFFIC_TYPE_RELIABLE,
		TRAFFIC_TYPE_VOICE,
		TRAFFIC_TYPE_CUSTOM,
		TRAFFIC_TYPE_CUSTOM_START,  // 10 or so custom types possible.
		TRAFFIC_TYPE_CUSTOM_END=16, // 10 or so custom types possible.
	//---------------------------------
		TRAFFIC_TYPE_COUNT=TRAFFIC_TYPE_CUSTOM_END,
	};
}
			
#endif //MULTIPLAYER_ENUMS_H