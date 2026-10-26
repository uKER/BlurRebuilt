#ifndef REPLAY_BUFFER_H
#define REPLAY_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>


namespace Game
{
	class C_MemChunk;
}

namespace Game
{

/* 
===============================================================
					class C_ReplayBuffer

NOTES: This buffer class holds all the actual replay data for a vehicle.
		Used so we can double-buffer the saving/loading of replay data
		to/from files to avoid a negative effect on performance.
===============================================================
*/
class C_ReplayBuffer
{
public:
	C_ReplayBuffer();
	~C_ReplayBuffer() {Destroy();};

	void										Destroy();

	//	Get methods used to write to/read from replay save file
	C_MemChunk*									GetVehicleRecording() const {return m_VehicleRecording;};
	C_MemChunk*									GetControlsRecording() const {return m_ControlsRecording;};

	void										Clear();
	void										Clone(C_ReplayBuffer* p_ToCopy);
	
private:
	
	C_MemChunk*									m_VehicleRecording;
	C_MemChunk*									m_ControlsRecording;
};

} // namespace Game

#endif // REPLAY_BUFFER_H
