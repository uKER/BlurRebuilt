#ifndef NETLEADERBOARDREADMANAGER_H
#define NETLEADERBOARDREADMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

/////////////////////////////////////////////////////////////////////////////
// Class       : netLeaderboardReadManager
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 28 May 2008
// Description : leaderboard manager. schedule searches, ect...
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netLeaderboardReadQuery);
BD_REFERENCE(netLeaderboardReadData);
BD_REFERENCE(netLeaderboardReadResults);
BD_REFERENCE(netLeaderboardReader);
class netLeaderboardReadManager: public bdReferencable
{
public:
	netLeaderboardReadManager();
	virtual ~netLeaderboardReadManager();

	netLeaderboardReadResultsRef database();
	
	virtual bool queueQuery(netLeaderboardReadQueryRef query);
	virtual bool busy() const;
		
	virtual bool stop();
	virtual bool update();
	virtual bool clear();

private:
	bdLinkedList<netLeaderboardReadQueryRef>	m_pendingQueries;	// queries to perform.
	bdLinkedList<netLeaderboardReaderRef>		m_searches;			// ongoing reads.
	netLeaderboardReadResultsRef				m_joinedResults;	// shared results.
};
#endif //NETLEADERBOARDREADMANAGER_H