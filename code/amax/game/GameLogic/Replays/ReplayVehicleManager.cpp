#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <Game\Messaging\PickupActorMessages.h>
#include <Game\GameActors\Components\GameComponents.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <GameLogic\Replays\ReplayDirector.h>
#include <GameLogic\Replays\ReplayVehicleManager.h>
#include <GameLogic\Replays\ReplayBuffer.h>
#include <GameLogic\Replays\ReplayPacket.h>
#include <GameLogic\Replays\ReplayDefines.h>
#include <GameLogic\Replays\ReplayUtilities.h>
#include <GameLogic\Replays\MemChunk.h>

#include <System/Debug/I_DebugRender.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <Vehicle\Input.h>

#include <System\Memory.h>

Debug::Logger &g_ReplayManagerLog( Debug::GetLogger("amax.replay") );

Debug::Bool g_RecordEveryFrame("Replays.Recording.RecordEveryFrame", true, Debug::Persist());
Debug::Bool g_DebugOverrideFakePhysics("Replays.OverrideFakePhysicsDuringPlayback", true, Debug::Persist());

Debug::Bool g_DrawReplayDebugLines("Replays.DrawDebugLines", false, Debug::Persist());

const F32 JUMP_DISTANCE_THRESHOLD = 10;

namespace Game
{

C_ReplayVehicleManager::C_ReplayVehicleManager()
{
	m_VehicleIndex = -1;
	m_LastStateRecordTime = 0;
	m_pVehicleActor = NULL;

	m_ReplayMode = E_ReplayMode_Idle;	//	Default to idle - nothing will happen when Update() is called

	m_FirstBufferIsMemory = true;

	MEMORY_TRACKER_PUSH("game.replay");

	m_Buffers[0] = new C_ReplayBuffer();
	m_Buffers[1] = new C_ReplayBuffer();

	MEMORY_TRACKER_POP("game.replay");

	m_PreviousStateTime = (int)(REPLAY_START_TIME * REPLAY_RACETIME_SCALER);
	m_NextStateTime = (int)(REPLAY_START_TIME * REPLAY_RACETIME_SCALER);

	mFlip = false;
}

void C_ReplayVehicleManager::Destroy()
{
	FreeBuffers();
}


void C_ReplayVehicleManager::FreeBuffers()
{
	SAFE_DELETE_PTR(m_Buffers[0]);
	SAFE_DELETE_PTR(m_Buffers[1]);
}

void C_ReplayVehicleManager::SwapBuffers()
{
	m_FirstBufferIsMemory = !m_FirstBufferIsMemory;
}

void C_ReplayVehicleManager::CheckBuffersValid()
{
	if(!m_Buffers[0])
	{
		m_Buffers[0] = new C_ReplayBuffer();
	}
	if(!m_Buffers[1])
	{
		m_Buffers[1] = new C_ReplayBuffer();
	}
}

C_ReplayBuffer*	C_ReplayVehicleManager::GetMemoryBuffer()
{
	CheckBuffersValid();
	
	if(m_FirstBufferIsMemory)
	{
		return m_Buffers[0];
	}
	else
		return m_Buffers[1];
}

C_ReplayBuffer*	C_ReplayVehicleManager::GetDiskBuffer()
{
	CheckBuffersValid();

	if(m_FirstBufferIsMemory)
	{
		return m_Buffers[1];
	}
	else
		return m_Buffers[0];
}

E_ReplayReturnCode C_ReplayVehicleManager::CloneMemoryBuffer(C_ReplayVehicleManager* p_VehicleToCopy)
{
	GetMemoryBuffer()->Clone(p_VehicleToCopy->GetMemoryBuffer());
	return E_ReplayReturnCode_Success;
}

C_MemChunk*	C_ReplayVehicleManager::GetMemoryVehicleRec()
{
	return GetMemoryBuffer()->GetVehicleRecording();
}

C_MemChunk*	C_ReplayVehicleManager::GetMemoryControlsRec()
{
	return GetMemoryBuffer()->GetControlsRecording();
}

C_MemChunk*	C_ReplayVehicleManager::GetDiskVehicleRec()
{
	return GetDiskBuffer()->GetVehicleRecording();
}

C_MemChunk*	C_ReplayVehicleManager::GetDiskControlsRec()
{
	return GetDiskBuffer()->GetControlsRecording();
}

void C_ReplayVehicleManager::Update(U32 p_ReplayTime)
{
	switch(m_ReplayMode)
	{
	case E_ReplayMode_Record:
		{
			RecordStatesAndControls(p_ReplayTime);
		}
		break;
	case E_ReplayMode_GhostRecord:
		{
			GhostRecordStatesAndControls(p_ReplayTime);
		}
		break;
	case E_ReplayMode_Play:
		{
			Playback(p_ReplayTime);
		}
		break;
	case E_ReplayMode_Idle:
		{
			
		}
		break;
	case E_ReplayMode_Disabled:
		{
			//	Put anything in here that should be done if the vehicle is stopped at the end of its replay
			m_pVehicleActor->GetVehiclePhysics()->Controls().Hold();
		}
		break;
	default:
		break;
	}
}

void C_ReplayVehicleManager::NewScene()
{
	if(m_ReplayMode == E_ReplayMode_Disabled)
	{
		ChangeReplayMode(E_ReplayMode_Play);
	}
	if(m_ReplayMode == E_ReplayMode_Record)
	{
		GetMemoryBuffer()->Clear();
	}
	if(m_ReplayMode == E_ReplayMode_Play)
	{
		m_PreviousState = m_NextState;
		m_PreviousStateTime = m_NextStateTime;

		U8* nextStateAddress = FindNextPacketOfType(E_ReplayPacketType_VehicleState, GetMemoryVehicleRec(), GetMemoryVehicleRec()->GetStart());
		if(!nextStateAddress)
			return;
		C_ReplayPacketVehicleState* packet = (C_ReplayPacketVehicleState*)nextStateAddress;
		if(!packet)
			return;
		m_NextState = packet->m_CompressedNetworkState;
		m_NextStateTime = packet->GetTime();
	}
}

void C_ReplayVehicleManager::RecordStatesAndControls(U32 p_ReplayTime)
{
	//if(g_RecordEveryFrame || m_ReplayMode == E_ReplayMode_GhostRecord)
	//{
	//	AddState(p_ReplayTime);
	//	//	Keep record of new recording time
	//	m_LastStateRecordTime = p_ReplayTime;
	//}
	//else if((m_LastStateRecordTime + (int)(m_pVehicleActor->GetReplayTimestep() * REPLAY_RACETIME_SCALER))
	//	< p_ReplayTime)
	//{
	//	AddState(p_ReplayTime);
	//	//	Keep record of new recording time
	//	m_LastStateRecordTime = p_ReplayTime;
	//}

	AddState(p_ReplayTime);
	m_LastStateRecordTime = p_ReplayTime;
}

void C_ReplayVehicleManager::GetNetworkState(RacingPhysics::Vehicle::CompressedNetworkState* p_VehicleState)
{
	m_pVehicleActor->GetVehiclePhysics()->GetNetworkState(p_VehicleState);
}

void C_ReplayVehicleManager::GetVehicleDamage(Vehicle::C_DamageState& p_DamageState)
{
	m_pVehicleActor->GetDamageState(p_DamageState);
}

void C_ReplayVehicleManager::AddState(U32 p_ReplayTime, bool forcedRecord)
{
	//	Pack state and time into packet, and store
	C_ReplayPacketVehicleState statePacket;
	m_pVehicleActor->GetVehiclePhysics()->GetNetworkState(&statePacket.m_CompressedNetworkState);
	statePacket.SetTime(p_ReplayTime);

	////	Check if vehicle position has actually changed since last update.
	RacingPhysics::Car::NetworkState thisState;
	RacingPhysics::Car::NetworkState lastState;
	RacingPhysics::Car::UncompressNetworkState(statePacket.m_CompressedNetworkState, thisState);
	RacingPhysics::Car::UncompressNetworkState(m_LastRecordedCompressedNetworkState, lastState);

	Maths::HVec3 thisPosition = thisState.axis.Position();
	Maths::HVec3 lastPosition = lastState.axis.Position();
	
	if(g_RecordEveryFrame ||
		forcedRecord ||
		(thisPosition.X() != lastPosition.X()) || (thisPosition.Y() != lastPosition.Y()) || (thisPosition.Z() != lastPosition.Z()))
	{
		//	If the vehicle's moved really far in a single frame, then make a recording at the previous position first.
		//	This makes the vehicle jump in the replay rather than attempt to interpolate to this new position (needed for cutscenes...)
		if(Sqrt(Squared(thisPosition.X() - lastPosition.X()) + Squared(thisPosition.Y() - lastPosition.Y()) + Squared(thisPosition.Z() - lastPosition.Z())) > JUMP_DISTANCE_THRESHOLD)
		{
			C_ReplayPacketVehicleState lastStateAgain;
			lastStateAgain.m_CompressedNetworkState = m_LastRecordedCompressedNetworkState;
			lastStateAgain.SetTime(p_ReplayTime);
			if(GetMemoryVehicleRec()->Push((U8*)&lastStateAgain, sizeof(lastStateAgain)))
				LOGDEBUG(g_ReplayManagerLog, "Vehicle %d - state packet chunk had to be increased", m_VehicleIndex);
		}
		if(GetMemoryVehicleRec()->Push((U8*)&statePacket, sizeof(statePacket)))
			LOGDEBUG(g_ReplayManagerLog, "Vehicle %d - state packet chunk had to be increased", m_VehicleIndex);
		m_LastRecordedCompressedNetworkState = statePacket.m_CompressedNetworkState;

		AddControls(p_ReplayTime);
	}
}

void C_ReplayVehicleManager::AddControls(U32 p_ReplayTime)
{
	C_ReplayPacketControls controls;
	m_pVehicleActor->GetVehiclePhysics()->GetNetworkControls(controls.m_CompressedControls);
	controls.SetTime(p_ReplayTime);
	U32 controlsize = sizeof(controls);
	if(GetMemoryControlsRec()->Push((U8*)&controls, sizeof(controls)))
		LOGDEBUG(g_ReplayManagerLog, "Vehicle %d - control packet chunk had to be increased", m_VehicleIndex);
}

void C_ReplayVehicleManager::AddPerkFiredEvent(C_ReplayPacketPerkFire p_PerkFirePacket)
{
	GetMemoryVehicleRec()->Push((U8*)&p_PerkFirePacket, sizeof(p_PerkFirePacket));
}

void C_ReplayVehicleManager::AddPerkHitEvent(C_ReplayPacketPerkHit p_PerkHitPacket)
{
	GetMemoryVehicleRec()->Push((U8*)&p_PerkHitPacket, sizeof(p_PerkHitPacket));
}

void C_ReplayVehicleManager::AddShieldedPerkEvent( C_ReplayPacketShieldedPerk p_ShieldedPerkPacket )
{
	GetMemoryVehicleRec()->Push((U8*)&p_ShieldedPerkPacket, sizeof(p_ShieldedPerkPacket));
}

void C_ReplayVehicleManager::AddPickupStoreEvent( C_ReplayPacketPickupsStore p_PickupStorePacket )
{
	GetMemoryVehicleRec()->Push((U8*)&p_PickupStorePacket, sizeof(p_PickupStorePacket));
}

void C_ReplayVehicleManager::AddPickupAddedEvent( C_ReplayPacketPickupAdded p_PickupAddedPacket )
{
	GetMemoryVehicleRec()->Push((U8*)&p_PickupAddedPacket, sizeof(p_PickupAddedPacket));
}

void C_ReplayVehicleManager::Playback(U32 p_ReplayTime)
{
	PlaybackControls(p_ReplayTime);
	PlaybackVehiclePackets(p_ReplayTime);
}

void C_ReplayVehicleManager::PlaybackVehiclePackets(U32 p_ReplayTime)
{
	//PERF_EVENT("C_ReplayVehicleManager::PlaybackVehiclePackets:SkipThrough()");
	//	Scan through packets for any events packets belonging to this vehicle
	E_ReplayReturnCode keepChecking = E_ReplayReturnCode_Failed;	//	If this is _Success a packet has just been processed
	do
	{
		keepChecking = E_ReplayReturnCode_Failed;
		E_ReplayPacketType type = PeekAtPacket(GetMemoryVehicleRec());
		switch(type)
		{
		case E_ReplayPacketType_VehicleState:
			{
				keepChecking = ProcessVehicleState(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_PerkFired:
			{
				keepChecking = ProcessPerkFireEvent(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_PerkHit:
			{
				keepChecking = ProcessPerkHitEvent(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_ShieldedPickup:
			{
				keepChecking = ProcessShieldedPerkEvent(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_PickupsStore:
			{
				keepChecking = ProcessPickupStoreEvent(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_PickupAdded:
			{
				keepChecking = ProcessPickupAddedEvent(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_Unknown:	//	Fall through
		default:
			break;
		}
		GetMemoryVehicleRec()->AssertChunk();
	}while(keepChecking == E_ReplayReturnCode_Success);

	if(m_NextStateTime <= m_PreviousStateTime)
	{
		return;
	}
	//	Send this frame's interpolated vehicle position state packet...thing.
	InterpolateAndBroadcastState(p_ReplayTime);
}

E_ReplayReturnCode C_ReplayVehicleManager::ProcessVehicleState(U32 p_ReplayTime)
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketVehicleState* packet = (C_ReplayPacketVehicleState*)current;
	if(p_ReplayTime > packet->GetTime())
	{
		if(packet->GetTime() == m_NextStateTime)
		{
			bool seemsToHaveWorked = true;
		}
		//	This isn't where we actually send vehicle states, here we're just setting the next and previous states
		//	and moving the memory's current position past the packet
		m_PreviousState = m_NextState;
		m_PreviousStateTime = m_NextStateTime;

		current += sizeof(C_ReplayPacketVehicleState);

		U8* next = FindNextPacketOfType(E_ReplayPacketType_VehicleState, GetMemoryVehicleRec(), current);

		if(next)
		{
			C_ReplayPacketVehicleState* nextPacket = (C_ReplayPacketVehicleState*)next;
			m_NextState = nextPacket->m_CompressedNetworkState;
			m_NextStateTime = nextPacket->GetTime();
		}
		else
		{
			//	No more vehicle state data in this scene (for whatever reason...) so disable the vehicle until we have data for it.
			SetReplayMode(E_ReplayMode_Disabled);
		}
		//	Move past the packet (so we don't process it again)
		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketVehicleState));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}

E_ReplayReturnCode C_ReplayVehicleManager::ProcessPerkFireEvent(U32 p_ReplayTime)
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketPerkFire* packet = (C_ReplayPacketPerkFire*)current;
	if(p_ReplayTime > packet->GetTime())
	{
		PickupCreationStruct pickupInfo(packet->m_PickupType, packet->m_SubType, m_VehicleIndex);
		pickupInfo.mTargetIdx = packet->m_TargetIdx;
		pickupInfo.mDelay = packet->m_Delay;
		pickupInfo.mPickupId = packet->m_PickupUid;
		C_PickupsManager::Instance().CreatePickup(pickupInfo);

 		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketPerkFire));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}

E_ReplayReturnCode C_ReplayVehicleManager::ProcessPerkHitEvent(U32 p_ReplayTime)
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketPerkHit* packet = (C_ReplayPacketPerkHit*)current;
	if(p_ReplayTime > packet->GetTime())
	{	
		const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_VehicleIndex);
		if(pLogicActor && pLogicActor->pPickupComponent)
		{
			PickupHitInfo hitInfo;
			pLogicActor->pPickupComponent->RegisterHitByPickup(packet->m_PickupType, E_PICKUP_SUB_TYPE_NORMAL, E_PICKUP_HIT_TYPE_MAJOR, m_VehicleIndex, 0, E_TARGETTING_DIRECTION_NONE, g_DebugOverrideFakePhysics, hitInfo, 0);	//TODO - Add group Id
		}

		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketPerkHit));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}


Game::E_ReplayReturnCode C_ReplayVehicleManager::ProcessShieldedPerkEvent( U32 p_ReplayTime )
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketShieldedPerk* packet = (C_ReplayPacketShieldedPerk*)current;
	if(p_ReplayTime > packet->GetTime())
	{	
		const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_VehicleIndex);
		if(pLogicActor && pLogicActor->pPickupComponent)
		{
			pLogicActor->pPickupComponent->RegisterShieldedPickup(packet->m_PickupType, 0, 0);
		}

		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketShieldedPerk));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}


Game::E_ReplayReturnCode C_ReplayVehicleManager::ProcessPickupStoreEvent( U32 p_ReplayTime )
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketPickupsStore* packet = (C_ReplayPacketPickupsStore*)current;
	if(p_ReplayTime > packet->GetTime())
	{	
		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_VehicleIndex);
		if(pLogicActor)
		{
			VehiclePickupInfo& info = pLogicActor->GetPickupInfo();
			info.mPickupStore.Clear();
			for(int i = 0; i < 3; i++)
			{
				if(packet->m_PickupStore[i] != E_PICKUP_TYPE_NONE)
				{
					PickupStore singleStore;
					singleStore.mType		= packet->m_PickupStore[i];
					singleStore.mNumShots	= packet->m_NumShots[i];
					info.mPickupStore.PushBack(singleStore);
				}
			}
			info.mPickupStoreIndex = packet->m_StoreIndex;
		}

		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketPickupsStore));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}

Game::E_ReplayReturnCode C_ReplayVehicleManager::ProcessPickupAddedEvent( U32 p_ReplayTime )
{
	U8* current = GetMemoryVehicleRec()->GetCurrent();
	C_ReplayPacketPickupAdded* packet = (C_ReplayPacketPickupAdded*)current;
	if(p_ReplayTime > packet->GetTime())
	{	
		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_VehicleIndex);
		if(pLogicActor)
		{
			RLPickupType* pPickup		= (RLPickupType*)::RLDatabase::Instance().GetPickupType(packet->m_PickupType);
			RLSpecialFXBase* pSFXBase	= pPickup->mNormal->mSpecialFX;
			BIZ_ASSERT(pSFXBase);

			Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pSFXBase->mPickedUpSpecialFX, pLogicActor->GetWorldMatrix(), m_VehicleIndex, 
				-1, -1, Game::E_InGameSpecialFX_Trigger_PickedUp);
		}

		GetMemoryVehicleRec()->MoveForward(sizeof(C_ReplayPacketPickupAdded));
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}


void C_ReplayVehicleManager::PlaybackControls(U32 p_ReplayTime)
{
	if(m_ReplayMode != E_ReplayMode_Play)
	{
		return;
	}
	PERF_EVENT("C_ReplayVehicleManager::PlaybackControls()");
	E_ReplayReturnCode keepChecking = E_ReplayReturnCode_Failed;	//	If this is _Success a packet has just been processed
	do
	{
		keepChecking = E_ReplayReturnCode_Failed;
		E_ReplayPacketType type = PeekAtPacket(GetMemoryControlsRec());
		switch(type)
		{
		case E_ReplayPacketType_Controls:
			{
				keepChecking = ProcessControls(p_ReplayTime);
			}
			break;
		case E_ReplayPacketType_Unknown:	//	Fall through
		default:
			//	The only packet type in the controls stream should be...controls, so hitting this == bad!
			break;
		}
	}while(keepChecking == E_ReplayReturnCode_Success);
}

E_ReplayReturnCode C_ReplayVehicleManager::ProcessControls(U32 p_ReplayTime)
{
	U8* current = GetMemoryControlsRec()->GetCurrent();
	C_ReplayPacketControls *packet = (C_ReplayPacketControls*)current;
	if(p_ReplayTime > packet->GetTime())
	{
		C_ReplayMsg_ControlsMsg* controlsMessage = new C_ReplayMsg_ControlsMsg(packet->m_CompressedControls);
		m_pVehicleActor->SendMessageToActor(m_VehicleHandle, controlsMessage);
		GetMemoryControlsRec()->MoveForward(sizeof(C_ReplayPacketControls));
		delete controlsMessage;
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Failed;
}

//	Using this method to change to Record will place the vehicle under player control.
//	If you just want it to start recording (like if it's under AI control, use SetReplayMode()
void C_ReplayVehicleManager::ChangeReplayMode(E_ReplayMode p_Mode, U8 p_ControllerIndex)
{
	if(m_ReplayMode == p_Mode)
		return;
	switch(p_Mode)
	{
	case E_ReplayMode_Record:
		{
			m_ReplayMode = p_Mode;
			GetMemoryVehicleRec()->ClearForward();
			GetMemoryControlsRec()->ClearForward();
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			m_pVehicleActor->GetVehicleInput()->m_iControlType = RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER;
			levelState.mCars[m_VehicleIndex].mController = p_ControllerIndex;
			m_pVehicleActor->GetVehiclePhysics()->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER);
			m_LastStateRecordTime = Game::C_ReplayDirector::Instance().GetScaledReplayTime();
		}
		break;
	case E_ReplayMode_Play:
		{
			m_ReplayMode = p_Mode;

			m_pVehicleActor->GetVehicleInput()->m_iControlType = RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK;
			m_pVehicleActor->GetVehiclePhysics()->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK);
		}
		break;
	case E_ReplayMode_Disabled:
		{
			//	Stop vehicle dead in it's tracks
			m_ReplayMode = p_Mode;
			m_pVehicleActor->GetVehiclePhysics()->Controls().Clear();
			m_pVehicleActor->GetVehiclePhysics()->Controls().Hold();
			m_pVehicleActor->GetVehiclePhysics()->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_SHOWROOM);
			Actors::C_VehicleControlsDisableMsg* disableControlsMsg = new Actors::C_VehicleControlsDisableMsg(m_VehicleIndex);
			m_pVehicleActor->SendMessageToActor(m_VehicleHandle, disableControlsMsg);
			delete disableControlsMsg;
		}
		break;
	case E_ReplayMode_Idle:
		{
			m_ReplayMode = p_Mode;
			GetMemoryVehicleRec()->ClearForward();
			GetMemoryControlsRec()->ClearForward();
			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			if(levelState.mCars[m_VehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			{
				levelState.mCars[m_VehicleIndex].mController = p_ControllerIndex;
				m_pVehicleActor->GetVehicleInput()->m_iControlType = RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER;
				m_pVehicleActor->GetVehiclePhysics()->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER);
			}
			else if(levelState.mCars[m_VehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_AI)
			{
				levelState.mCars[m_VehicleIndex].mController = INVALID_CONTROLLER;
				m_pVehicleActor->GetVehicleInput()->m_iControlType = RacingPhysics::Vehicle::CONTROL_TYPE_AI;
				m_pVehicleActor->GetVehiclePhysics()->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_AI);
			}
		}
	default:
		break;
	}
}

void C_ReplayVehicleManager::FreezeVehicle(bool p_FreezeStatus)
{
	Actors::C_VehicleFreezeMsg freezeMsg(m_VehicleIndex, p_FreezeStatus);
	m_pVehicleActor->SendMessageToActor(m_VehicleHandle, &freezeMsg);
}

void C_ReplayVehicleManager::SetInitialNetworkStates(RacingPhysics::Car::CompressedNetworkState p_State1, U32 p_State1Time, RacingPhysics::Car::CompressedNetworkState p_State2, U32 p_State2Time)
{
	m_PreviousState = p_State1;
	m_PreviousStateTime = p_State1Time;

	m_NextState = p_State2;
	m_NextStateTime = p_State2Time;
}

E_ReplayReturnCode C_ReplayVehicleManager::InterpolateAndBroadcastState(U32 p_ReplayTime)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	if(g_DrawReplayDebugLines)
	{
		
		RacingPhysics::Car::NetworkState state1;
		RacingPhysics::Car::UncompressNetworkState(m_PreviousState, state1);
		Maths::HMat43 worldMatrix1 = RacingPhysics::ConvertHVec(state1.axis);
		pDebugRenderer->Draw3dLine(worldMatrix1.AW(), worldMatrix1.AW() + Maths::HVec3(0.0f, 2.0f, 0.0f), Maths::RGBA(255, 0, 255));

		RacingPhysics::Car::NetworkState state3;
		RacingPhysics::Car::UncompressNetworkState(m_NextState, state3);
		Maths::HMat43 worldMatrix3 = RacingPhysics::ConvertHVec(state3.axis);
		pDebugRenderer->Draw3dLine(worldMatrix3.AW(), worldMatrix3.AW() + Maths::HVec3(0.0f, 2.0f, 0.0f), Maths::RGBA(255, 0, 0));

	}


	//	Allow for the possibility of dropping a couple of milliseconds due to losing 
	if(p_ReplayTime < m_PreviousStateTime)
		p_ReplayTime = m_PreviousStateTime;
	float timeDifference = ((m_NextStateTime - m_PreviousStateTime)/(float)REPLAY_RACETIME_SCALER);
	float normalisedBlend = ((float)(p_ReplayTime - m_PreviousStateTime)/(float)(m_NextStateTime - m_PreviousStateTime));
	C_ReplayMsg_VehicleStateMsg packet;
	packet.m_CompressedNetworkState = RacingPhysics::Car::BlendStatesLinear(m_PreviousState, m_NextState, normalisedBlend, timeDifference);
	m_pVehicleActor->SendMessageToActor(m_VehicleHandle, &packet);

	if(g_DrawReplayDebugLines)
	{
		RacingPhysics::Car::NetworkState state2;
		RacingPhysics::Car::UncompressNetworkState(packet.m_CompressedNetworkState, state2);
		Maths::HMat43 worldMatrix2 = RacingPhysics::ConvertHVec(state2.axis);
		pDebugRenderer->Draw3dLine(worldMatrix2.AW(), worldMatrix2.AW() + Maths::HVec3(0.0f, 2.0f, 0.0f), Maths::RGBA(0, 255, 0));
	}

	return E_ReplayReturnCode_Success;
}

void C_ReplayVehicleManager::EraseRecording(void)
{
	m_LastStateRecordTime = 0;
	m_FirstBufferIsMemory = true;
	FreeBuffers();
}

void C_ReplayVehicleManager::ResetPlayback(void)
{
	GetMemoryVehicleRec()->GoToStart();
	GetMemoryControlsRec()->GoToStart();
}

E_ReplayReturnCode C_ReplayVehicleManager::AcquireVehicleHandle(void)
{
	m_VehicleHandle = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetActorHandleFromVehicleIndex(m_VehicleIndex);
	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplayVehicleManager::AcquireVehicleActor(void)
{
	const Actor::C_Actor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetGameWorld()->GetActorList()->GetActorPtr(m_VehicleHandle);
	m_pVehicleActor = Actor::ActorCast< Game::Actors::C_VehicleActor> (pActor);
	return E_ReplayReturnCode_Success;
}


/*------------------------------------------	GHOST METHODS	---------------------------------------*/

void C_ReplayVehicleManager::SetGhost(bool p_Ghosted)
{
	Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_VehicleIndex);

	if(p_Ghosted)
	{
		pLogicActor->Ghost(0.f);
	}
	else
	{
		pLogicActor->Deghost();
	}
}

//	Ghosts don't need to be super-accurate, so we record them at small regular intervals
void C_ReplayVehicleManager::GhostRecordStatesAndControls(U32 p_LapTime)
{
	if((m_LastStateRecordTime + (int)(0.2f * REPLAY_RACETIME_SCALER))
		< p_LapTime)
	{
		AddState(p_LapTime);
		AddControls(p_LapTime);
		//	Keep record of new recording time
		m_LastStateRecordTime = p_LapTime;
	}
}

void C_ReplayVehicleManager::GhostPlayback(U32 p_LapTime)
{
	GhostPlaybackVehiclePackets(p_LapTime);
	PlaybackControls(p_LapTime);
}



void C_ReplayVehicleManager::GhostPlaybackVehiclePackets(U32 p_LapTime)
{
	//	Much the same as playing back vehicle packets for a standard replay, but we're not interested in damage events etc
	//	and when there are no more vehicle packets we turn the car invisible rather than bring it to a halt.

}

} // namespace Game
