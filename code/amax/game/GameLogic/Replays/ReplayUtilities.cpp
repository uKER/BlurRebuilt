#include "stdafx.h"
#include <GameLogic\Replays\ReplayUtilities.h>
#include <GameLogic\Replays\ReplayTypes.h>
#include <GameLogic\Replays\ReplayPacket.h>

#include "GameLogic/Structures/LevelState.h"
#include <GameApp.h>
#include <Game/Game.h>


namespace Game
{
/*
===============
NAME: PeekAtPacket

NOTE: Given a C_MemChunk, finds out what replay packet type the next one to process is
===============
*/
E_ReplayPacketType PeekAtPacket(Game::C_MemChunk* p_Chunk)
{
	U8* current = p_Chunk->GetCurrent();
	if(current < (p_Chunk->GetEnd() - sizeof(C_ReplayPacketBase)))
	{
		C_ReplayPacketBase *base = (C_ReplayPacketBase *)current;

		E_ReplayPacketType theType = base->GetType();
		if(theType)
			return theType;
	}
	
	// If we don't have a valid type, we return Unknown
	return E_ReplayPacketType_Unknown;
}


/*
===============
NAME: PeekAtPacket

NOTE: Same as above, but takes a U8* (for checking a packet that isn't necessarily at the memory chunk's m_Current pointer)
===============
*/
E_ReplayPacketType PeekAtPacket(Game::C_MemChunk* p_Chunk, U8* p_StartAddress)
{
	if((p_StartAddress >= p_Chunk->GetStart()) && (p_StartAddress < (p_Chunk->GetEnd() - sizeof(C_ReplayPacketBase))))
	{
		C_ReplayPacketBase *base = (C_ReplayPacketBase *)p_StartAddress;

		E_ReplayPacketType theType = base->GetType();
		if(theType != E_ReplayPacketType_Unknown)
			return theType;
	}
	// If we don't have a valid type, we return Unknown
	return E_ReplayPacketType_Unknown;
}

/*
===============
NAME: FindNextPacketOfType

NOTE: Searches through the given C_MemChunk object from the given address for the first replay packet of the given type.
===============
*/
U8* FindNextPacketOfType(E_ReplayPacketType p_Type, C_MemChunk* p_pChunk, U8* p_StartAddress)
{
	U8* currentPacket = p_StartAddress;
	while(PeekAtPacket(p_pChunk, currentPacket) != p_Type)
	{
		switch(PeekAtPacket(p_pChunk, currentPacket))
		{
		
		case E_ReplayPacketType_VehicleState:
			{
				currentPacket += sizeof(C_ReplayPacketVehicleState);
			}
			break;
		case E_ReplayPacketType_PerkFired:
			{
				currentPacket += sizeof(C_ReplayPacketPerkFire);
			}
			break;
		case E_ReplayPacketType_PerkHit:
			{
				currentPacket += sizeof(C_ReplayPacketPerkHit);
			}
			break;
		case E_ReplayPacketType_Controls:
			{
				currentPacket += sizeof(C_ReplayPacketControls);
			}
			break;
		case E_ReplayPacketType_PerkPostDelay:
			{
				currentPacket += sizeof(C_ReplayPacketPerkPostDelay);
			}
			break;
		case E_ReplayPacketType_PickupsStore:
			{
				currentPacket += sizeof(C_ReplayPacketPickupsStore);
			}
			break;
		case E_ReplayPacketType_PickupAdded:
			{
				currentPacket += sizeof(C_ReplayPacketPickupAdded);
			}
			break;
		case E_ReplayPacketType_ShieldedPickup:
			{
				currentPacket += sizeof(C_ReplayPacketShieldedPerk);
			}
			break;
		case E_ReplayPacketType_Master:
			{
				currentPacket += GetMasterPacketSize(currentPacket);
			}
			break;
			//	Add other packet types here when we start using them!
		case E_ReplayPacketType_Unknown:	//	Fall through
		default:
			{
				// Ruh-roh! We've either screwed up somewhere or gone out of bounds of the memory chunk. Either way, bad news!
				return NULL;
			}
			break;
		}
	}
	return currentPacket;
}

int GetMasterPacketSize(U8* p_pMasterPacketAddress)
{
	C_ReplayPacketMaster* masterPacket = (C_ReplayPacketMaster*)p_pMasterPacketAddress;
	switch(masterPacket->GetMasterType())
	{
	case E_ReplayPacketMaster_VehicleStates:
		{
			return sizeof(C_ReplayPacketMaster_VehicleStates);
		}
		break;
	case E_ReplayPacketMaster_VehicleDamage:
		{
			return sizeof(C_ReplayPacketMaster_VehicleDamage);
		}
		break;
	case E_ReplayPacketMaster_PickupOnTrack:
		{
			return sizeof(C_ReplayPacketMaster_PickupOnTrack);
		}
		break;
	default:
		break;
	}
	//	We have a problem if we hit this, so let's stick an assert here in case it finds a Master packet without a sub-type
	BIZ_ASSERT(false);
	return 0;
}

S8 FindLocalIndexFromCharacterID(S32 p_CharacterXTID)
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	for(U32 i = 0; i < levelState.mNumCars + levelState.mNumAdditionalCars; i++)
	{
		if(levelState.mCars[i].mCharacterID == p_CharacterXTID)
			return i;
	}
	return -1;
}


} // namespace Game