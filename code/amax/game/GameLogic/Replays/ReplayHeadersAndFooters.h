#ifndef REPLAY_HEADERS_H
#define REPLAY_HEADERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <core\containers\String.h>

namespace Game
{

struct CarSpec
{
	U32											m_CarID;
	U32											m_CharacterID;

	//	Added in Replay Version 5 -

	Util::C_String64							m_NameAboveCar;
	U32											m_ResprayId;
	U32											m_Padding2;
	U32											m_Padding3;
	U32											m_Padding4;
	U32											m_Padding5;
};

//	Struct containing the stuff we need to know about the replay to set up the playback
class C_ReplayHeader
{
public:
	U32											m_SceneLength;	//	Length of time (in REPLAY SCALED seconds between start of one replay chunk to the next)
	U32											m_Event;
	U32											m_Route;
	S32											m_RouteNumber;

	CarSpec										m_CarInfo[MAX_VEHICLES];

	U8											m_Version;
	U8											m_NumCars;
	U8											m_NumLaps;

	U8											m_Padding1;
	
	//	Added in Replay Version 5 -
	U32											m_Padding2;
	U32											m_Padding3;
	U32											m_Padding4;
	U32											m_Padding5;

	void										Clear();
	int											GetHeaderSize();		//	Returns the size in bytes of the header - DEPENDING ON THE VERSION IT WAS SAVED UNDER
	void										Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream);
};


//	Struct containing other details about the replay that we didn't know until the end of the recording - goes at the end of the file
class C_ReplayFooter
{
public:
	U32											m_EndOfReplayTime;
	U32											m_NumScenes;
	
	U8											m_Padding1;
	U8											m_Padding2;
	U8											m_Padding3;
	U8											m_CompletedMarker;

	void										Clear();
	void										Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );
};


//	Header we fill and put at the start of a single-vehicle replay
class C_ReplayVehicleHeader
{
public:
	U8											m_VehicleClass;
	U8											m_Padding1;							//	Padding to keep 4-byte order correct
	U8											m_Padding2;
	U8											m_Padding3;
	S32											m_RouteNumber;
	U32											m_ReplayTimeLength;					//	Replay time length of vehicle recording (for example, lap time for a single-lap ghost)
	U32											m_CarID;
	U32											m_CharacterID;

	void										Clear();
	void										Serialise( Sys::BinarySerialiserAutoEndian& stream ); 
	void										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );
};

} // namespace Game

#endif	//	REPLAY_HEADERS_H