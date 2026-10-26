#ifndef REPLAY_VEHICLE_MANAGER_H
#define REPLAY_VEHICLE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>
#include <GameLogic/Replays/ReplayMessages.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include <GameLogic\Replays\ReplayTypes.h>
#include <GameLogic\Replays\ReplayUtilities.h>

namespace Game
{
	class C_ReplayBuffer;
	class C_ReplayPacketVehicleState;
	class C_ReplayPacketControls;
	class C_ReplayPacketPerkFire;
	class C_MemChunk;
}

namespace RacingPhysics
{
	struct Vehicle::CompressedNetworkState;
}


namespace Game
{

namespace Actors
{
	class C_VehicleActor;
}

/* 
===============================================================
					class C_ReplayVehicleManager

NOTES: Class responsible for handling all replay data/methods related to a single vehicle
===============================================================
*/
class C_ReplayVehicleManager
{
public:
	C_ReplayVehicleManager();
	~C_ReplayVehicleManager() {Destroy();};
	
	E_ReplayMode								GetReplayMode() const {return m_ReplayMode;};
	
	//	Simply sets the replay mode. Should only be call before a race/replay begins. Use ChangeReplayMode() during a race/replay.
	void										SetReplayMode(E_ReplayMode p_Mode) {m_ReplayMode = p_Mode;};

	void										RecordStatesAndControls(U32 p_ReplayTime);

	void										AddState(U32 p_ReplayTime, bool forcedRecord = false);

	void										AddControls(U32 p_ReplayTime);

	void										AddPerkFiredEvent(C_ReplayPacketPerkFire p_PerkFirePacket);
	void										AddPerkHitEvent(C_ReplayPacketPerkHit p_PerkHitPacket);
	void										AddShieldedPerkEvent(C_ReplayPacketShieldedPerk p_ShieldedPerkPacket);
	void										AddPickupStoreEvent(C_ReplayPacketPickupsStore p_PickupStorePacket);
	void										AddPickupAddedEvent(C_ReplayPacketPickupAdded p_PickupAddedPacket);

	void										Playback(U32 p_ReplayTime);

	void										SetVehicleIndex(U8 p_index){m_VehicleIndex = p_index;};

	void										SetInitialNetworkStates(RacingPhysics::Car::CompressedNetworkState p_State1, U32 p_State1Time, RacingPhysics::Car::CompressedNetworkState p_State2, U32 p_State2Time);

	//	Handles the issues that follow changing the replay state of a vehicle during a race/replay. Use this once the recording/playback has begun.
	void										ChangeReplayMode(E_ReplayMode p_Mode, U8 p_ControllerIndex = 0);

	void										FreezeVehicle(bool p_FreezeStatus);

	void										NewScene();

	void										EraseRecording(void);
	
	void										ResetPlayback(void);

	void										Update(U32 p_ReplayTime);

	void										SwapBuffers();

	E_ReplayReturnCode							AcquireVehicleHandle(void);
	E_ReplayReturnCode							AcquireVehicleActor(void);

	//	Get methods used to write to/read from replay save file
	void										CheckBuffersValid();
	C_ReplayBuffer*								GetMemoryBuffer();
	C_ReplayBuffer*								GetDiskBuffer();
	void										FreeBuffers();					//	Delete the replay buffers to free up memory

	E_ReplayReturnCode							CloneMemoryBuffer(C_ReplayVehicleManager* p_VehicleToCopy);

	C_MemChunk*									GetMemoryVehicleRec();
	C_MemChunk*									GetMemoryControlsRec();
	C_MemChunk*									GetDiskVehicleRec();
	C_MemChunk*									GetDiskControlsRec();

	void										GetNetworkState(RacingPhysics::Vehicle::CompressedNetworkState* p_VehicleState);
	void										GetVehicleDamage(Vehicle::C_DamageState& p_DamageState);

	bool										GetCompletionStatus() const {return m_ReplayComplete;};
	void										SetCompletionStatus(bool p_flag) {m_ReplayComplete = p_flag;};

	Actor::C_ActorHandle						GetHandle() const {return m_VehicleHandle;};
	int											GetIndex() const {return m_VehicleIndex;};

	U32											GetReplayLength() const { return m_ReplayLength; }


	/*------------------------------------------	GHOST METHODS	---------------------------------------*/

	void										GhostPlayback(U32 p_LapTime);
	void										SetGhost(bool p_Ghosted);
	/*-----------------------------------------------------------------------------------------------------*/
	


private:

	//	Replay mode of this vehicle - only matters for situations with a mix of recording and replaying cars
	//	such as time attacks (with ghosts) or when we're constructing cutscenes
	E_ReplayMode								m_ReplayMode;

	void										Destroy();							// Destructor calls this

	void										PlaybackVehiclePackets(U32 p_ReplayTime);
	void										PlaybackControls(U32 p_ReplayTime);

	E_ReplayReturnCode							ProcessVehicleState(U32 p_ReplayTime);	//	Process the next packet which is a vehicle state
	E_ReplayReturnCode							ProcessPerkFireEvent(U32 p_ReplayTime);	//	Process the next packet which is a perk firing event
	E_ReplayReturnCode							ProcessPerkHitEvent(U32 p_ReplayTime);	//	Process a vehicle being hit by a pickup
	E_ReplayReturnCode							ProcessShieldedPerkEvent(U32 p_ReplayTime);
	E_ReplayReturnCode							ProcessPickupStoreEvent(U32 p_ReplayTime);
	E_ReplayReturnCode							ProcessPickupAddedEvent(U32 p_ReplayTime);

	E_ReplayReturnCode							ProcessControls(U32 p_ReplayTime);	//	Process the next packet which is a vehicle control state

	//	Calculate the interpolated vehicle state between m_PreviousState and m_NextState, and broadcast to the vehicle actor
	E_ReplayReturnCode							InterpolateAndBroadcastState(U32 p_ReplayTime);


	/*------------------------------------------	GHOST METHODS	---------------------------------------*/

	//	Slightly different playback algorithm specifically for ghosts
	void										GhostPlaybackVehiclePackets(U32 p_LapTime);
	void										GhostRecordStatesAndControls(U32 p_LapTime);
	;

	/*-----------------------------------------------------------------------------------------------------*/

	S8											m_VehicleIndex;

	Actor::C_ActorHandle						m_VehicleHandle;					//	Handle to vehicle this manager is responsible for
																					//	(used to grab vehicle actor when race is loaded)

	const Actors::C_VehicleActor*				m_pVehicleActor;					//	Pointer to vehicle actor this manager is responsible for

	U32											m_LastStateRecordTime;				//	Last time in (in REPLAY SCALED seconds) that the vehicle's state was recorded
	
	RacingPhysics::Car::CompressedNetworkState	m_PreviousState;					//	Previous network state we are interpolating FROM
	RacingPhysics::Car::CompressedNetworkState	m_NextState;						//	Next network state we are interpolating TO
	RacingPhysics::Vehicle::CompressedNetworkState	m_LastRecordedCompressedNetworkState;	//	Last network state we recorded this vehicle with.

	U32											m_PreviousStateTime;				//	Time (in seconds multipled by REPLAY_RACETIME_SCALER) of replay at previous state
	U32											m_NextStateTime;					//	Time (in seconds multipled by REPLAY_RACETIME_SCALER) of replay at next state

	U32											m_ReplayLength;						//	Length (in seconds multipled by REPLAY_RACETIME_SCALER) of vehicle replay (for time attack ghosts this would be the lap time)

	C_ReplayBuffer*								m_Buffers[2];						//	Buffers holding replay data (swap which to record into at save time)

	bool										m_FirstBufferIsMemory;				//	Flag denoting whether the first of the two buffers is for writing to memory or saving to disk

	//	Flag that is set once the vehicle reaches the end of its replay
	//	- could be at different times for different vehicles, for example in a cut-scene or Elimination replay
	bool										m_ReplayComplete;

	// Oh God I'm sorry
	bool										mFlip;

	Maths::HVec3								m_LastRecordedPosition;
};

} // namespace Game

#endif // REPLAY_VEHICLE_MANAGER_H