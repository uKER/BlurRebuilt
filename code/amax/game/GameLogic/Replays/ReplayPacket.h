#ifndef REPLAY_PACKET_H
#define REPLAY_PACKET_H

#ifdef _MSC_VER
#pragma once
#endif

#include <RacingPhysics/RacingPhysics.h>
#include <GameLogic\Replays\ReplayTypes.h>
#include <GameLogic\Replays\ReplayMessages.h>
#include "misc_shared/Messaging/MessageBase.h"

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include <Vehicle\Damage\DamageLayer.h>

namespace Game
{

/* 
===============================================================
					class C_ReplayPacketBase

NOTES: Base class for replay data packets
===============================================================
*/
class C_ReplayPacketBase
{
public:
	C_ReplayPacketBase()
	{
		m_PacketType = E_ReplayPacketType_Unknown;

		m_RaceTime = 0;
	}

	//	Returns packet type
	E_ReplayPacketType							GetType(void) const { return (E_ReplayPacketType)m_PacketType; }

	//	Sets packet type
	void										SetType(E_ReplayPacketType p_Type) { m_PacketType = (DWORD)p_Type; }

	//	Returns race time - this is the float time in seconds, multiplied by 10,000
	DWORD										GetTime(void) const { return m_RaceTime; }

	//	Sets race time - compress float first
	void										SetTime(DWORD p_Time) { m_RaceTime = p_Time; }

protected:
	struct
	{
		U32										m_PacketType	: 6;					//	Type of replay packet
		
		U32										m_RaceTime		: 26;					//	Time since race start where packet belongs,
																								//	24-bit allows around 111 minutes of race time
	};
};

/* 
===============================================================
					class C_ReplayPacketVehicleState

NOTES: Replay data packet holding vehicle's compressed state at a specific moment
===============================================================
*/
class C_ReplayPacketVehicleState : public C_ReplayPacketBase
{
public:
	C_ReplayPacketVehicleState()
	{
		m_PacketType = E_ReplayPacketType_VehicleState;

		memset(&m_CompressedNetworkState, 0, sizeof(m_CompressedNetworkState));
	}
public:

	//	Make use of the existing network state compression structure(s)
	RacingPhysics::Vehicle::CompressedNetworkState m_CompressedNetworkState;
};

/* 
===============================================================
					class C_ReplayPacketControls

NOTES: Replay data packet holding vehicle's compressed controls at a specific moment
===============================================================
*/
class C_ReplayPacketControls : public C_ReplayPacketBase
{
public:
	C_ReplayPacketControls()
	{
		m_PacketType = E_ReplayPacketType_Controls;

		memset(&m_CompressedControls, 0, sizeof(m_CompressedControls));
	}

	//	Make use of the existing network control compression structure(s)
	RacingPhysics::Vehicle::CompressedNetworkControls m_CompressedControls;
};

/* 
===============================================================
					class C_ReplayPacketPerkFire

NOTES: Replay data packet holding details on a perk being fired off
===============================================================
*/

class C_ReplayPacketPerkFire : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPerkFire()
	{
		m_PacketType = E_ReplayPacketType_PerkFired;
	};

	S8											m_TargetIdx;
	E_PICKUP_TYPE								m_PickupType;
	E_PICKUP_SUB_TYPE							m_SubType;
	F32											m_Delay;
	U32											m_PickupUid;
};

class C_ReplayPacketPerkHit : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPerkHit()
	{
		m_PacketType = E_ReplayPacketType_PerkHit;
	};

	E_PICKUP_TYPE								m_PickupType;
};

class C_ReplayPacketPerkPostDelay : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPerkPostDelay()
	{
		m_PacketType = E_ReplayPacketType_PerkPostDelay;
	};

	U32											m_PickupUid;
	MathsStorage::Float43						m_TargetMatrix;
	float										m_fVal;
};

class C_ReplayPacketPickupsStore : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPickupsStore()
	{
		m_PacketType = E_ReplayPacketType_PickupsStore;
	};

	E_PICKUP_TYPE								m_PickupStore[3];
	U8											m_NumShots[3];
	int											m_StoreIndex;
};

class C_ReplayPacketPickupAdded : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPickupAdded()
	{
		m_PacketType = E_ReplayPacketType_PickupAdded;
	};

	E_PICKUP_TYPE								m_PickupType;
};

class C_ReplayPacketShieldedPerk : public C_ReplayPacketBase
{
public:
	C_ReplayPacketShieldedPerk()
	{
		m_PacketType = E_ReplayPacketType_ShieldedPickup;
	};

	E_PICKUP_TYPE								m_PickupType;
};

class C_ReplayPacketKillPickup : public C_ReplayPacketBase
{
public:
	C_ReplayPacketKillPickup()
	{
		m_PacketType = E_ReplayPacketType_KillPickup;
	};

	U32											m_PickupUid;
};

class C_ReplayPacketPhotoOp : public C_ReplayPacketBase
{
public:
	C_ReplayPacketPhotoOp()
	{
		m_PacketType = E_ReplayPacketType_PhotoOp;
	};
};

/*---------------------------------------------	MASTER PACKETS -----------------------------------------*/

class C_ReplayPacketMaster : public C_ReplayPacketBase
{
public:
	C_ReplayPacketMaster()
	{
		m_PacketType = E_ReplayPacketType_Master;

		m_MasterType = E_ReplayPacketMaster_Unknown;
	}
	E_ReplayPacketMaster						GetMasterType(void) const { return m_MasterType; }

protected:
	E_ReplayPacketMaster						m_MasterType;
};


class C_ReplayPacketMaster_VehicleStates : public C_ReplayPacketMaster
{
public:
	C_ReplayPacketMaster_VehicleStates()
	{
		m_MasterType = E_ReplayPacketMaster_VehicleStates;

		memset(&m_CompressedNetworkStates, 0, MAX_VEHICLES * sizeof(RacingPhysics::Vehicle::CompressedNetworkState));
	}
	RacingPhysics::Vehicle::CompressedNetworkState m_CompressedNetworkStates[MAX_VEHICLES];
};


class C_ReplayPacketMaster_VehicleDamage : public C_ReplayPacketMaster
{
public:
	C_ReplayPacketMaster_VehicleDamage()
	{
		m_MasterType = E_ReplayPacketMaster_VehicleDamage;
		memset(&m_DamageState, 0, sizeof(Vehicle::C_DamageState));
	}
	U8											m_VehicleIndex;
	Vehicle::C_DamageState						m_DamageState;
};

class C_ReplayPacketMaster_PickupOnTrack : public C_ReplayPacketMaster
{
public:
	C_ReplayPacketMaster_PickupOnTrack()
	{
		m_MasterType = E_ReplayPacketMaster_PickupOnTrack;
	}
	U32											m_PickupId;
	E_PICKUP_TYPE								m_PickupType;
};

}	//	namespace Game

#endif //REPLAY_PACKET_H