#ifndef NETLEADERBOARDREADER_H
#define NETLEADERBOARDREADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTime/netTime.h"
#include "netLeaderboardReadQuery.h"

/////////////////////////////////////////////////////////////////////////////
// Class       : netLeaderboardReader
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 05 June 2008
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netFriends);
BD_REFERENCE(netLeaderboardReader);
class netLeaderboardReader: public bdReferencable
{
public:
	netLeaderboardReader(int type);
	virtual ~netLeaderboardReader();

	virtual bool busy	 () const { return (m_state != STATE_IDLE); }
	virtual bool started () const { return (m_state != STATE_IDLE) || (m_status != STATUS_NOT_SET); }
	virtual bool finished() const { return (m_state == STATE_IDLE) && (m_status != STATUS_NOT_SET); }
	virtual bool success () const { return (m_state == STATE_IDLE) && (m_status == STATUS_OK); }
	virtual bool failed  ()	const { return (m_state == STATE_IDLE) && (m_status == STATUS_ERROR); }
	virtual bool aborted () const { return (m_state == STATE_IDLE) && (m_status == STATUS_ABORTED); }

	bool clear();
	bool stop();
	bool start(netLeaderboardReadQueryRef query);
	bool update();

	const int												m_type;
	netLeaderboardReadQueryRef								m_query;
	bdLinkedList<netLeaderboardReadQueryDataRef>::Position	m_currentBatch;
	bdLinkedList<netLeaderboardReadQueryDataRef>::Position	m_nextBatch;
	int														m_state;
	int														m_step;
	int														m_status;
	netStopwatch											m_stateWatch;
	netFriendsRef											m_friends;

protected:
	
	virtual bool stopAndCleanup()=0;
	virtual bool startReading()=0;
	virtual bool continueReading(bool& error)=0;

	bool startFriendsEnumeration(bool& error);
	bool continueFriendsEnumeration(bool& error);
	
	enum
	{
		STATE_IDLE,
		STATE_ENUMERATING_FRIENDS,
		STATE_READING,
		STATE_PAUSING,
	};

	enum
	{
		STATUS_NOT_SET,
		STATUS_OK,
		STATUS_ABORTED,
		STATUS_ERROR,
	};

	bool end(int state);
	void setState(int state);
	void setStatus(int status);
	
};

#endif //NETLEADERBOARDREADER_H