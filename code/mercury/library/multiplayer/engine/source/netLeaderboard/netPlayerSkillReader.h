#ifndef NETPLAYERSKILLREADER_H
#define NETPLAYERSKILLREADER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netLeaderboardReader.h"
#include <Network/PlayerSkillReader.h>
#include <Network/PlayerSkill.h>


BD_REFERENCE(netPlayerSkillReader);
/////////////////////////////////////////////////////////////////////////////
// Class       : netPlayerSkillReader
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 05 June 2008
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
class netPlayerSkillReader: public NetPlay::C_PlayerSkillReader, public netLeaderboardReader
{
public:
	netPlayerSkillReader(int playerCount);
	
	virtual bool stopAndCleanup();
	virtual bool startReading();
	virtual bool continueReading(bool& error);
};

#endif //NETPLAYERSKILLREADER_H