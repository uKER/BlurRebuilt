#ifndef NET_EVENT_PACKETS_H
#define NET_EVENT_PACKETS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_NetEventType
{
	E_NetEventType_Unknown = 0,
	E_NetEventType_DroppedPickupPickedUp,
	E_NetEventType_PickupDroppedOnTrack,
	E_NetEventType_DecoyDroppedOnTrack,
	E_NetEventType_RegisterShocks,
	E_NetEventType_PickupFired,
	E_NetEventType_PickupKilled,
	E_NetEventType_PickupPostDelay,
	E_NetEventType_HitByPickup,
	E_NetEventType_Ghosted,
	E_NetEventType_Idle,
	E_NetEventType_MineMoved,
	E_NetEventType_Eliminated,
	E_NetEventType_Repaired,
	E_NetEventType_ShieldedPickup,
	E_NetEventType_Score_Aggressive,
	E_NetEventType_SetPowerupType,
	E_NetEventType_DamageDealt,
	E_NetEventType_AbilityActivated,
	E_NetEventType_JudgedPowerupShielded,
	E_NetEventType_JudgedPowerupHit,
	E_NetEventType_MineDestroyedByProjectile,
	E_NetEventType_SetPowerupRespawnDelay,
	E_NetEventType_EvadedShock,
	E_NetEventType_HitAllShockDomes,
	E_NetEventType_SilentRunning,
};

} // namespace Game

#endif // NET_EVENT_PACKETS_H
