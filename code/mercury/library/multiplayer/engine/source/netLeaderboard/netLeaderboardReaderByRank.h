#ifndef NETLEADERBOARDREADERBYRANK_H
#define NETLEADERBOARDREADERBYRANK_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netLeaderboardReader.h"
#include <Network/ScoreboardReaderByRank.h>

/////////////////////////////////////////////////////////////////////////////
// Class       : netLeaderboardReaderByRank
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 05 June 2008
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netLeaderboardReaderByRank);
class netLeaderboardReaderByRank: public Scoreboard::C_ReaderByRank, public netLeaderboardReader
{
public:
	netLeaderboardReaderByRank();

	virtual bool stopAndCleanup();
	virtual bool startReading();
	virtual bool continueReading(bool& error);
};

#endif //NETLEADERBOARDREADERBYRANK_H