#ifndef REPLAY_MESSAGES_H
#define REPLAY_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>
#include <RacingPhysics/RacingPhysics.h>
#include <Vehicle\Damage\DamageLayer.h>

#include <misc_shared\Messaging\MessageBase.h>

namespace Game
{


enum E_ReplayMessage
{
	// Sent by Replay Director to a vehicle along with its new state
	E_ReplayMessage_UpdateVehicleState = FOURCC_HI20('REME'),

	// Sent by Replay Director to a vehicle along with its new controls
	E_ReplayMessage_UpdateControls,

	//	Tell the vehicle actor to jump its network state
	E_ReplayMessage_NetworkJump,

	//	Message that systems (ie. Cutscene Manager) can send to the Replay Director to jump to a replay time
	E_ReplayMessage_GoToTime,

	//	Sent when playback of the current replay/cutscene is complete
	E_ReplayMessage_PlaybackFinished,

	//	Tells a vehicle to start recording (for editing replays/cutscenes)
	E_ReplayMessage_ChangeModeRecord,

	//	Instructs the RP to write over the existing session ghost with the player's last lap
	E_ReplayMessage_UpdateSessionGhost,

	//	Instructs the RP to restart playback of all ghosts
	E_ReplayMessage_TimeAttackNewLap,

	E_ReplayMessage_SetVehicleDamage,

	E_ReplayMessage_Pause,

	E_ReplayMessage_Resume,
};


class C_ReplayMsg_VehicleStateMsg : public Messaging::C_MessageDerived<C_ReplayMsg_VehicleStateMsg, E_ReplayMessage_UpdateVehicleState>
{
public:
	C_ReplayMsg_VehicleStateMsg() {};
	C_ReplayMsg_VehicleStateMsg(RacingPhysics::Vehicle::CompressedNetworkState p_CompressedNetworkState) {m_CompressedNetworkState = p_CompressedNetworkState;};

	RacingPhysics::Vehicle::CompressedNetworkState m_CompressedNetworkState;
};


class C_ReplayMsg_ControlsMsg : public Messaging::C_MessageDerived<C_ReplayMsg_ControlsMsg, E_ReplayMessage_UpdateControls>
{
public:
	C_ReplayMsg_ControlsMsg(RacingPhysics::Vehicle::CompressedNetworkControls p_CompressedControls) {m_CompressedControls = p_CompressedControls;};

	RacingPhysics::Vehicle::CompressedNetworkControls m_CompressedControls;
};


class C_ReplayMsg_GoToTimeMsg : public Messaging::C_MessageDerived<C_ReplayMsg_GoToTimeMsg, E_ReplayMessage_GoToTime>
{
public:
	C_ReplayMsg_GoToTimeMsg(float p_TimeIn) { m_ReplayTime = p_TimeIn; }

	float m_ReplayTime;
};

class C_ReplayMsg_ChangeModeRecordMsg : public Messaging::C_MessageDerived<C_ReplayMsg_ChangeModeRecordMsg, E_ReplayMessage_ChangeModeRecord>
{
public:
	C_ReplayMsg_ChangeModeRecordMsg(int p_Index, int p_Controller)
		: m_VehicleIndex(p_Index), m_ControllerIndex(p_Controller)
	{
	}

	U8 m_VehicleIndex;
	U8 m_ControllerIndex;
};

class C_ReplayMsg_SetVehicleDamageMsg : public Messaging::C_MessageDerived<C_ReplayMsg_SetVehicleDamageMsg, E_ReplayMessage_SetVehicleDamage>
{
public:
	C_ReplayMsg_SetVehicleDamageMsg(U8 p_Index, Vehicle::C_DamageState p_DamageState)
		: m_VehicleIndex(p_Index), m_DamageState(p_DamageState)
	{
	}

	U8											m_VehicleIndex;
	Vehicle::C_DamageState						m_DamageState;
};


} // namespace Game

#endif // REPLAY_MESSAGES_H
