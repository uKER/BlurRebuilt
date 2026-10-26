#ifndef NETSESSION_H
#define NETSESSION_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netEnums/netEnums.h"
#include "netSession/netSessionEnums.h"
#include "netTask/netTask.h"
#include "netTime/netTime.h"

#include <network\session.h>
#include <network\secureKey.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>

BD_REFERENCE(netSessionQoS);
BD_REFERENCE(netSessionState);
BD_REFERENCE(netSessionTask);
BD_REFERENCE(netSession);
class netSession: public NetPlay::C_Session, public bdReferencable
{
public:
	netSession(const char* name);
	~netSession();

	bool create(netSessionStateRef state);
	bool update();
	
	bool matchmaking() const;
	bool notActive	() const { return (SessionState() == NetPlay::C_Session::eStateNotActive); }
	bool creating	() const { return (SessionState() == NetPlay::C_Session::eStateCreating); }
	bool running	() const { return (SessionState() == NetPlay::C_Session::eStateRunning); }
	bool playing	() const { return (SessionState() == NetPlay::C_Session::eStatePlaying); }
	bool error		() const { return (SessionState() == NetPlay::C_Session::eStateError); }
	NetPlay::ArbitrationNonce arbitrationInfo();
	
	bool creating();
	bool migrating();
	bool arbitrating();

	bool usesArbitration();
	bool usesStats();
	bool busy();

	// to do : remove me
	bool startPlay();
	bool endPlay();
	
	netSessionStateRef sessionState();
	netSessionStateRef pendingState();
	netSessionQoSRef   sessionQoS();
	void pendingQoS(netSessionQoSRef qos);
	
	bool debugparamsList(const char* header, const NetPlay::SessionParameterList& list);

	bool updateTasks();
	bool updatePlayers();
	bool updateParams();
	bool updateArbitration();
	bool updateMigration();
	bool updateQoS();

	const char*							m_name;			// debug name.
	bdLinkedList<netSessionTaskRef>		m_tasks;		// tasks.
	netSessionStateRef					m_pendingState;	// state we want to be in.
	netSessionStateRef					m_sessionState;	// state of the session after tasks are succeeded.
	netSessionQoSRef					m_pendingQoS;	// qos we want to use
	netSessionQoSRef					m_sessionQoS;	// qos curently active
	netStopwatch						m_qosTick;		// update QoS every second or so

private:
	bool onTasksSuccess(netSessionTaskRef task);
	bool taskPending(int taskId);
};

#endif //NETSESSION_H