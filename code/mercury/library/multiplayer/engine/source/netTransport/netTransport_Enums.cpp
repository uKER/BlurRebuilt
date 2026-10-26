
#include "stdafx.h"
#include "./netTransport_Enums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	bool			Debugging::dbg_ping	= false;
	int				Debugging::dbg_pingId = -1;
	bool			Debugging::dbg_callMigrate = false;
	bool			Debugging::dbg_callOutputMigrate = false;
	bool			Debugging::dbg_useAddressMap = true;
	Debug::Logger&	Debugging::g_logger = ::Debug::GetLogger("netdebug.transport");

	Debug::Float	Debugging::dbg_bandwidthCap				("Multiplayer.Transport.BandwidthCap", 0.0f, Debug::Range(0.0f, 128.0f));
	Debug::Bool		Debugging::dbg_forwardEnabled			("Multiplayer.Transport.ForwardPackets", true);
	Debug::Bool		Debugging::dbg_timeoutEnabled			("Multiplayer.Connect.Timeout", true);
	Debug::Bool		Debugging::dbg_migrationEnabled			("Multiplayer.Connect.MigrationEnabled", false);
	Debug::Int		Debugging::dbg_unreachableConnection	("Multiplayer.Connect.UnreachableConnection", 255, Debug::Range(0, 200));
	Debug::Function	Debugging::dbg_migrate					("Multiplayer.Connect.Migrate", migrate, NULL);
	Debug::Function	Debugging::dbg_outputMigrate			("Multiplayer.Connect.PrintMigrationStatus", outputMigrate, NULL);

	Debug::Bool		Debugging::dbg_simulatorEnabled			("Multiplayer.InternetSimulator.Enabled", true);
	Debug::Int		Debugging::dbg_simulatorLatency			("Multiplayer.InternetSimulator.Latency", 100, Debug::Range(0, 1000));
	Debug::Int		Debugging::dbg_simulatorPacketLoss		("Multiplayer.InternetSimulator.PacketLoss", 2, Debug::Range(0, 100));
	Debug::Int		Debugging::dbg_simulatorBlockConnection	("Multiplayer.InternetSimulator.BlockConnection", -1, Debug::Range(-1, 16));

	::Debug::Int	Debugging::dbg_logPacketSocketPort		("Multiplayer.DebugPacket.SocketPort", -1, Debug::Range(-1, 10));
	::Debug::Int	Debugging::dbg_logPacketDestination		("Multiplayer.DebugPacket.Destination",-1, Debug::Range(-1, 16));
	::Debug::Int	Debugging::dbg_logPacketOrigin			("Multiplayer.DebugPacket.Origin", -1, Debug::Range(-1, 16));
	::Debug::Bool	Debugging::dbg_logPacketReliable		("Multiplayer.DebugPacket.Reliable", false);
	::Debug::Bool	Debugging::dbg_logPacketUnreliable		("Multiplayer.DebugPacket.Unreliable", false);

	void Debugging::outputMigrate(void*)
	{
		dbg_callOutputMigrate = true;
	}

	void Debugging::migrate(void*)
	{
		dbg_callMigrate = true;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif


