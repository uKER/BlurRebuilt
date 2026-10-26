#ifndef NETLEADERBOARDWRITEDATA_H
#define NETLEADERBOARDWRITEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "network/scoreboardStructures.h"
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

BD_REFERENCE(netColumnData);
class netColumnData: public Scoreboard::ColumnData, public bdReferencable
{
public:
};

BD_REFERENCE(netLeaderboardWriteData);
class netLeaderboardWriteData: public bdReferencable
{
public:
	netLeaderboardWriteData(int scoreboardId=0);

	int								m_scoreboardId;
	S64								m_rating;
	Scoreboard::eWriteType			m_writeType;
	bdLinkedList<netColumnDataRef>	m_properties;

	netColumnDataRef   findProperty(U32 fieldId);
	netColumnDataRef createProperty(U32 fieldId);

	bool setProperty(Scoreboard::ColumnData* data);
	bool setPropertyS32(U32 fieldId, S32 value);
	bool setPropertyF32(U32 fieldId, F32 value);
	bool setPropertyS64(U32 fieldId, S64 value);
	bool setPropertyF64(U32 fieldId, F64 value);
};


#endif //NETLEADERBOARDWRITEDATA_H