#include "stdafx.h"
#include <GameLogic\Replays\ReplayBuffer.h>
#include <GameLogic\Replays\ReplayPacket.h>
#include <GameLogic\Replays\MemChunk.h>

#include <System\Memory.h>

namespace Game
{

C_ReplayBuffer::C_ReplayBuffer()
{
	MEMORY_TRACKER_PUSH("game.replay");

	m_VehicleRecording = new Game::C_MemChunk(sizeof(C_ReplayPacketVehicleState) * 200);
	m_VehicleRecording->SetMaxSize(10000);	//	10k of memory
	m_ControlsRecording = new Game::C_MemChunk(sizeof(C_ReplayPacketControls) * 150);
	m_ControlsRecording->SetMaxSize(20000);	//	20k of memory

	MEMORY_TRACKER_POP("game.replay");
}

void C_ReplayBuffer::Destroy()
{
	SAFE_DELETE_PTR(m_VehicleRecording);
	SAFE_DELETE_PTR(m_ControlsRecording);
}

void C_ReplayBuffer::Clear()
{
	GetVehicleRecording()->ClearMemory();
	GetControlsRecording()->ClearMemory();
}

/*===============
NAME: Clone

NOTE: Clones the C_MemChunk objects within the buffer to match those from another given buffer
===============*/
void C_ReplayBuffer::Clone(C_ReplayBuffer* p_ToCopy)
{
	GetVehicleRecording()->Clone(p_ToCopy->GetVehicleRecording());
	GetControlsRecording()->Clone(p_ToCopy->GetControlsRecording());
}

} // namespace Game