#ifndef NETLEADERBOARDREADERBYUSERS_H
#define NETLEADERBOARDREADERBYUSERS_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netLeaderboardReader.h"
#include <Network/ScoreboardStructures.h>
#include <Network/ScoreboardReaderByUsers.h>


/////////////////////////////////////////////////////////////////////////////
// Class       : netLeaderboardReaderByUsers
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 05 June 2008
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netLeaderboardReaderByUsers);
class netLeaderboardReaderByUsers: public Scoreboard::C_ReaderByUsers, public netLeaderboardReader
{
public:
	netLeaderboardReaderByUsers();

	virtual bool stopAndCleanup();
	virtual bool startReading();
	virtual bool continueReading(bool& error);
};


#endif //NETLEADERBOARDREADERBYUSERS_H