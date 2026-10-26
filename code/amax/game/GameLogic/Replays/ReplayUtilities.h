#ifndef REPLAY_UTILITIES_H
#define REPLAY_UTILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\Replays\MemChunk.h>
#include <GameLogic/Replays/ReplayPacket.h>

namespace Game
{

//	Any info we need about the objects we're going to sample
struct C_GameObjectRecord
{
	Btl::String name;

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		stream << name;
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		stream >> name;
	}
};

//	Structure for storing the world matrix data for a game object at a given timestamp
struct C_GameObjectSample
{
	F32						time;
	MathsStorage::Float4*	pQuat;
	MathsStorage::Float3*	pPos;

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream, U32 nActors)
	{
		stream << time;
		for (U32 i = 0; i < nActors; ++i)
		{
			pQuat[i].Serialise(stream);
			pPos[i].Serialise(stream);
		}
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream, U32 nActors )
	{
		stream >> time;
		for (U32 i = 0; i < nActors; ++i)
		{
			pQuat[i].Deserialise(stream);
			pPos[i].Deserialise(stream);
		}
	}
};



	E_ReplayPacketType					PeekAtPacket(Game::C_MemChunk* p_Chunk);
	E_ReplayPacketType					PeekAtPacket(Game::C_MemChunk* p_Chunk, U8* p_StartAddress);
	U8*									FindNextPacketOfType(E_ReplayPacketType p_Type, 
																		C_MemChunk* p_pChunk, 
																		U8* p_StartAddress);

	int									GetMasterPacketSize(U8* p_pMasterPacketAddress);

	S8									FindLocalIndexFromCharacterID(S32 p_CharacterXTID);


} // namespace Game


#endif // REPLAY_UTILITIES_H