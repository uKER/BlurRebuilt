#ifndef REPLAY_TYPES_H
#define REPLAY_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

//	Different replay packet types
enum E_ReplayPacketType
{
	E_ReplayPacketType_Unknown = 0,

	E_ReplayPacketType_Master,				//	If a packet is found with this type, the specific Master type below is interrogated

	E_ReplayPacketType_VehicleState,
	E_ReplayPacketType_Controls,
	E_ReplayPacketType_PerkFired,
	E_ReplayPacketType_PerkPostDelay,
	E_ReplayPacketType_PerkHit,
	E_ReplayPacketType_KillPickup,
	E_ReplayPacketType_ShieldedPickup,
	E_ReplayPacketType_PhotoOp,
	E_ReplayPacketType_PickupsStore,
	E_ReplayPacketType_PickupAdded,
};

//	Different replay Master packet types
enum E_ReplayPacketMaster
{
	E_ReplayPacketMaster_Unknown = 0,
	
	E_ReplayPacketMaster_VehicleStates,
	E_ReplayPacketMaster_VehicleDamage,
	E_ReplayPacketMaster_PickupOnTrack,
};

enum E_ReplayMode
{
	E_ReplayMode_Disabled = 0,

	E_ReplayMode_FrontEnd,
	E_ReplayMode_Idle,
	E_ReplayMode_Paused,
	E_ReplayMode_CutscenePaused,
	E_ReplayMode_Record,
	E_ReplayMode_GhostRecord,
	E_ReplayMode_Ghost,
	E_ReplayMode_PreCutsceneEdit,
	E_ReplayMode_CutsceneEdit,
	E_ReplayMode_PrePlay,
	E_ReplayMode_Play,
	E_ReplayMode_PreTimeAttack,
	E_ReplayMode_TimeAttack,
};

enum E_ReplayReturnCode
{
	E_ReplayReturnCode_Unknown = -1,

	E_ReplayReturnCode_Failed = 0,
	E_ReplayReturnCode_Success,
	E_ReplayReturnCode_Proceed,
	E_ReplayReturnCode_Skip,
	E_ReplayReturnCode_Fatal,

	E_ReplayReturnCode_Num,
};

//	IMPORTANT - DON'T CHANGE THIS ORDER. If for some reason you really need to, let me know please. [GY]
enum E_GhostType
{
	E_GhostType_PlayerPlaceholder = 0,
	E_GhostType_SessionBest,
	E_GhostType_LifetimeBest,
	E_GhostType_WorldBest,
	E_GhostType_Misc,
};

} // namespace Game

#endif // REPLAY_TYPES_H