#include "stdafx.h"

#include <GameLogic\Replays\ReplayHeadersAndFooters.h>

namespace Game
{


void C_ReplayHeader::Clear()
{
	m_Version = 0;
	m_SceneLength = 0;
	m_NumCars = 0;
	m_Event = 0;
	m_Route = 0;
	m_RouteNumber = 0;
	m_NumLaps = 0;

	for(int i=0; i< MAX_VEHICLES; i++)
	{
		m_CarInfo[i].m_CarID = 0;
		m_CarInfo[i].m_CharacterID = 0;

		m_CarInfo[i].m_NameAboveCar = "";
		m_CarInfo[i].m_ResprayId = 0;
		m_CarInfo[i].m_Padding2 = 0;
		m_CarInfo[i].m_Padding3 = 0;
		m_CarInfo[i].m_Padding4 = 0;
		m_CarInfo[i].m_Padding5 = 0;
	}
	m_Padding1 = 0;
	m_Padding2 = 0;
	m_Padding3 = 0;
	m_Padding4 = 0;
	m_Padding5 = 0;
}

int C_ReplayHeader::GetHeaderSize()
{
	int size = 0;

	size += sizeof(m_Version);
	size += sizeof(m_SceneLength);
	size += sizeof(m_NumCars);
	size += sizeof(m_Event);
	size += sizeof(m_Route);
	size += sizeof(m_RouteNumber);
	size += sizeof(m_NumLaps);
	
	for(int i=0; i< MAX_VEHICLES; i++)
	{
		size += sizeof(m_CarInfo[i].m_CarID);
		size += sizeof(m_CarInfo[i].m_CharacterID);
		if(m_Version > 4)
		{
			size += sizeof(m_CarInfo[i].m_NameAboveCar);
			size += sizeof(m_CarInfo[i].m_ResprayId);
			size += sizeof(m_CarInfo[i].m_Padding2);
			size += sizeof(m_CarInfo[i].m_Padding3);
			size += sizeof(m_CarInfo[i].m_Padding4);
			size += sizeof(m_CarInfo[i].m_Padding5);
		}
	}
	size += sizeof(m_Padding1);
	if(m_Version > 4)
	{
		size += sizeof(m_Padding2);
		size += sizeof(m_Padding3);
		size += sizeof(m_Padding4);
		size += sizeof(m_Padding5);
	}
	return size;
}

void C_ReplayHeader::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	stream << m_Version;
	stream << m_SceneLength;
	stream << m_NumCars;
	stream << m_Event;
	stream << m_Route;
	stream << m_RouteNumber;
	stream << m_NumLaps;

	for(int i = 0; i < MAX_VEHICLES; i++)
	{
		stream << m_CarInfo[i].m_CarID;
		stream << m_CarInfo[i].m_CharacterID;

		stream << m_CarInfo[i].m_NameAboveCar;
		stream << m_CarInfo[i].m_ResprayId;
		stream << m_CarInfo[i].m_Padding2;
		stream << m_CarInfo[i].m_Padding3;
		stream << m_CarInfo[i].m_Padding4;
		stream << m_CarInfo[i].m_Padding5;
	}

	stream << m_Padding1;
	stream << m_Padding2;
	stream << m_Padding3;
	stream << m_Padding4;
	stream << m_Padding5;
}

void C_ReplayHeader::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	//	We should have already deserialised the replay version before this so we don't do it again
	stream >> m_SceneLength;
	stream >> m_NumCars;
	stream >> m_Event;
	stream >> m_Route;
	stream >> m_RouteNumber;
	stream >> m_NumLaps;

	for(int i = 0; i < MAX_VEHICLES; i++)
	{
		stream >> m_CarInfo[i].m_CarID;
		stream >> m_CarInfo[i].m_CharacterID;

		if(m_Version > 4)
		{
			stream >> m_CarInfo[i].m_NameAboveCar;
			stream >> m_CarInfo[i].m_ResprayId;
			stream >> m_CarInfo[i].m_Padding2;
			stream >> m_CarInfo[i].m_Padding3;
			stream >> m_CarInfo[i].m_Padding4;
			stream >> m_CarInfo[i].m_Padding5;
		}
	}

	stream >> m_Padding1;

	if(m_Version > 4)
	{
		stream >> m_Padding2;
		stream >> m_Padding3;
		stream >> m_Padding4;
		stream >> m_Padding5;
	}
}


void C_ReplayFooter::Clear()
{
	m_EndOfReplayTime = 0;
	m_NumScenes = 0;

	m_Padding1 = 0;
	m_Padding2 = 0;
	m_Padding3 = 0;
	m_CompletedMarker = 0;
}

void C_ReplayFooter::Serialise(Sys::BinarySerialiserAutoEndian &stream)
{
	stream << m_EndOfReplayTime;
	stream << m_NumScenes;

	stream << m_Padding1;
	stream << m_Padding2;
	stream << m_Padding3;
	stream << m_CompletedMarker;
}

void C_ReplayFooter::Deserialise(Sys::BinaryDeserialiserAutoEndian &stream)
{
	stream >> m_EndOfReplayTime;
	stream >> m_NumScenes;

	stream >> m_Padding1;
	stream >> m_Padding2;
	stream >> m_Padding3;
	stream >> m_CompletedMarker;
}


void C_ReplayVehicleHeader::Clear()
{
	m_VehicleClass = 0;
	m_Padding1 = 0;
	m_Padding2 = 0;
	m_Padding3 = 0;

	m_RouteNumber = 0;
	m_ReplayTimeLength = 0;
	m_CarID = 0;
	m_CharacterID = 0;
}

void C_ReplayVehicleHeader::Serialise(Sys::BinarySerialiserAutoEndian &stream)
{
	stream << m_VehicleClass;
	stream << m_Padding1;
	stream << m_Padding2;
	stream << m_Padding3;
	stream << m_RouteNumber;
	stream << m_ReplayTimeLength;
	stream << m_CarID;
	stream << m_CharacterID;
}

void C_ReplayVehicleHeader::Deserialise(Sys::BinaryDeserialiserAutoEndian &stream)
{
	stream >> m_VehicleClass;
	stream >> m_Padding1;
	stream >> m_Padding2;
	stream >> m_Padding3;
	stream >> m_RouteNumber;
	stream >> m_ReplayTimeLength;
	stream >> m_CarID;
	stream >> m_CharacterID;
}


} // namespace Game