#ifndef NETSESSIONENUMERATOR_H
#define NETSESSIONENUMERATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Network\SessionEnumerator.h"
#include "netTime/netTime.h"
#include "netSession/netEnumeratedSession.h"
#include "netSession/netSessionList.h"

#include <bdNet/bdLANDiscovery/bdLANDiscoveryClient.h>
#include <bdNet/bdLANDiscovery/bdLANDiscoveryListener.h>


BD_REFERENCE(netSessionEnumerator);
class netSessionEnumerator: public NetPlay::C_SessionEnumerator, public bdReferencable
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS
};

class netSessionSearch;
class netLANDiscoveryListener: public bdLANDiscoveryListener
{
public:
	netLANDiscoveryListener(netSessionSearch* search);

	void onDiscovery(bdGameInfoRef gameInfo);

	netEnumeratedSessionRef createGame(const bdUInt32 titleID);

	netSessionSearch* m_search;
};

BD_REFERENCE(netSessionSearchData);
BD_REFERENCE(netSessionSearch);
class netSessionSearch: public bdReferencable
{
public:
	enum
	{
		STATUS_NOT_SET=-1,
		STATUS_OK,
		STATUS_ABORTED,
		STATUS_ERROR,
	};
	enum
	{
		STATE_IDLE,
		STATE_SEARCHING,
	};

	netSessionSearch();
	virtual ~netSessionSearch();

	bool reset();
	bool start(netSessionSearchDataRef searchParams);	// perform a search
	bool stop();
	bool clear();
	bool update();
	
	bool searching() const { return (m_state == STATE_SEARCHING); }
	bool  finished() const { return (m_state == STATE_IDLE) && (m_status != STATUS_NOT_SET); }
	bool   success() const { return (m_state == STATE_IDLE) && (m_status == STATUS_OK); }
	bool   aborted() const { return (m_state == STATE_IDLE) && (m_status == STATUS_ABORTED); }
	bool    failed() const { return (m_state == STATE_IDLE) && (m_status == STATUS_ERROR); }
		
	netSessionSearchDataRef		m_searchParams;
	netSessionEnumeratorRef		m_onlineEnumerator;
	netSessionListRef			m_sessions;

	void setStatus(int status);
	void setState(int state);

	netLANDiscoveryListener m_listener;
	bdLANDiscoveryClient*	m_lanEnumerator;
	virtual void onDiscovery(bdGameInfoRef gameInfo);

	int			 m_state;
	int			 m_step;
	int			 m_status;
	netStopwatch m_stateWatch;

};

#endif //NETSESSIONENUMERATOR_H
