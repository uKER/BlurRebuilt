#include "stdafx.h"
#include "./netEnumeratedSession.h"
#include "./netSession.h"
#include "./netSessionState.h"
#include "./netSessionJoinData.h"
#include "./netSessionCreateData.h"
#include "./netSessionQoS.h"
#include "netDebug/netDebug.h"
#include "netStream/netArray.h"
#include "netPlatform/netPlatform_Core.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

enum
{
	NET_SESSION_TASK_CREATE_PEER,
	NET_SESSION_TASK_CREATE_HOST,
	NET_SESSION_TASK_ADD_PLAYERS,
	NET_SESSION_TASK_REMOVE_PLAYERS,
	NET_SESSION_TASK_MODIFY_SESSION,
	NET_SESSION_TASK_ENABLE_QOS,
	NET_SESSION_TASK_DISABLE_QOS,
	NET_SESSION_TASK_REGISTER_ARBITRATION,
	NET_SESSION_TASK_MIGRATE_HOST,
	NET_SESSION_TASK_MIGRATE_PEER,
	NET_SESSION_TASK_START_PLAY,
	NET_SESSION_TASK_END_PLAY,
};


BD_REFERENCE(netSessionTask);
class netSessionTask: public netTask
{
public:
	netSessionTask(int id, const char* taskName, const char* sessionName, NetPlay::C_Session* session);
	virtual ~netSessionTask();
	virtual bool update();
	virtual bool end(int status);
	
	char m_name[64];
	NetPlay::C_Session* m_session;
	NetPlay::SessionCommandNotification	m_notification;
};

BD_REFERENCE(netSessionTask_CreatePeer);
class netSessionTask_CreatePeer: public netSessionTask
{
public:
	netSessionTask_CreatePeer(netSession* session, netSessionStateRef createState);
	bool init();
	bool update();
	netSessionStateRef m_sessionState;
};

BD_REFERENCE(netSessionTask_CreateHost);
class netSessionTask_CreateHost: public netSessionTask
{
public:
	netSessionTask_CreateHost(netSession* session, netSessionStateRef createState);
	bool init();
	bool update();
	netSessionStateRef m_sessionState;
};

BD_REFERENCE(netSessionTask_AddPlayers);
class netSessionTask_AddPlayers: public netSessionTask
{
public:
	netSessionTask_AddPlayers(netSession* session, const NetPlay::C_Session::SessionUserList& players);
	bool init();
	NetPlay::C_Session::SessionUserList	m_players;
};	

BD_REFERENCE(netSessionTask_RemovePlayers);
class netSessionTask_RemovePlayers: public netSessionTask
{
public:
	netSessionTask_RemovePlayers(netSession* session, const NetPlay::C_Session::SessionUserList& players);
	bool init();
	NetPlay::C_Session::SessionUserList	m_players;
};

BD_REFERENCE(netSessionTask_ModifySession);
class netSessionTask_ModifySession: public netSessionTask
{
public:
	netSessionTask_ModifySession(netSession* session, int flags, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig);
	bool init();
	int								m_flags;
	NetPlay::SessionParameterList	m_parameters;
	NetPlay::SessionSlotConfig		m_slotConfig;
};

BD_REFERENCE(netSessionTask_EnableQoS);
class netSessionTask_EnableQoS: public netSessionTask
{
public:
	netSessionTask_EnableQoS(netSession* session, netSessionQoSRef m_sessionQoS);
	bool update();
	netSessionQoSRef m_sessionQoS;
};

BD_REFERENCE(netSessionTask_DisableQoS);
class netSessionTask_DisableQoS: public netSessionTask
{
public:
	netSessionTask_DisableQoS(netSession* session);
	bool update();
};

BD_REFERENCE(netSessionTask_RegisterArbitration);
class netSessionTask_RegisterArbitration: public netSessionTask
{
public:
	netSessionTask_RegisterArbitration(netSession* session, const NetPlay::ArbitrationNonce& arbitrationInfo);
	bool init();
	NetPlay::ArbitrationNonce m_arbitrationInfo;
};	

BD_REFERENCE(netSessionTask_MigrateHost);
class netSessionTask_MigrateHost: public netSessionTask
{
public:
	netSessionTask_MigrateHost(netSession* session, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig);
	bool init();
	NetPlay::SessionParameterList	m_parameters;
	NetPlay::SessionSlotConfig		m_slotConfig;
};	

BD_REFERENCE(netSessionTask_MigratePeer);
class netSessionTask_MigratePeer: public netSessionTask
{
public:
	netSessionTask_MigratePeer(netSession* session, const NetPlay::MigrationInfo& migrationInfo, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig);
	bool init();
	NetPlay::SessionParameterList	m_parameters;
	NetPlay::SessionSlotConfig		m_slotConfig;
	NetPlay::MigrationInfo			m_migrationInfo;
};		

BD_REFERENCE(netSessionTask_StartPlay);
class netSessionTask_StartPlay: public netSessionTask
{
public:
	netSessionTask_StartPlay(netSession* session);
	bool init();
	bool update();
};

BD_REFERENCE(netSessionTask_EndPlay);
class netSessionTask_EndPlay: public netSessionTask
{
public:
	netSessionTask_EndPlay(netSession* session);
	bool init();
	bool update();
};

netSessionTask::netSessionTask(int id, const char* taskname, const char* sessionname, NetPlay::C_Session* session)
: netTask(id, m_name)
, m_session(session)
{
	sprintf_s(m_name, sizeof(m_name), "session(%s)::task(%s)", sessionname, taskname);
}

netSessionTask::~netSessionTask()
{
	if(!m_notification.m_Finished)
	{
		logWarn("task not finished.");
	}
}

bool netSessionTask::update()
{
	if(m_notification.m_Finished)
	{
		return end(STATUS_OK);
	}
	return true;
}

bool netSessionTask::end(int status)
{
	m_notification.m_Finished = true;
	return netTask::end(status);
}

netSessionTask_CreatePeer::netSessionTask_CreatePeer(netSession* session, netSessionStateRef createState)
: netSessionTask(NET_SESSION_TASK_CREATE_PEER, "CreatePeer", session->m_name, session)
, m_sessionState(new netSessionState)
{
	*m_sessionState = *createState;
}

bool netSessionTask_CreatePeer::init()
{
	return m_session->CreatePeer(	m_sessionState->m_pad, 
									m_sessionState->m_flags, 
									m_sessionState->m_parameters.First(), 
									m_sessionState->m_parameters.Size(), 
									m_sessionState->m_securityInfo, 
									m_sessionState->m_slotConfig);
}


bool netSessionTask_CreatePeer::update()
{
	if (m_session->SessionState() == NetPlay::C_Session::eStateCreating)
		return true;

	if (m_session->SessionState() == NetPlay::C_Session::eStateRunning)
	{
		return end(STATUS_OK);
	}
	return end(STATUS_ERROR);
}

netSessionTask_CreateHost::netSessionTask_CreateHost(netSession* session, netSessionStateRef createState)
: netSessionTask(NET_SESSION_TASK_CREATE_HOST, "CreateHost", session->m_name, session)
, m_sessionState(new netSessionState)
{
	*m_sessionState = *createState;
	m_sessionState->m_players.Clear();
	memset(&m_sessionState->m_securityInfo, 0, sizeof(m_sessionState->m_securityInfo));
}

bool netSessionTask_CreateHost::init()
{
	return m_session->Create(	m_sessionState->m_pad, 
								m_sessionState->m_flags, 
								m_sessionState->m_parameters.First(), 
								m_sessionState->m_parameters.Size(), 
								m_sessionState->m_slotConfig);
}

bool netSessionTask_CreateHost::update()
{
	if (m_session->SessionState() == NetPlay::C_Session::eStateCreating)
		return true;

	if (m_session->SessionState() == NetPlay::C_Session::eStateRunning)
	{
		return end(STATUS_OK);
	}
	return end(STATUS_ERROR);
}

netSessionTask_AddPlayers::netSessionTask_AddPlayers(netSession* session, const NetPlay::C_Session::SessionUserList& players)
: netSessionTask(NET_SESSION_TASK_ADD_PLAYERS, "AddPlayers", session->m_name, session)
, m_players(players)
{}

bool netSessionTask_AddPlayers::init()
{
	if(!m_session->AddPlayers(m_players, &m_notification))
	{
		return end(STATUS_ERROR);
	}
	return true;
}


netSessionTask_RemovePlayers::netSessionTask_RemovePlayers(netSession* session, const NetPlay::C_Session::SessionUserList& players)
: netSessionTask(NET_SESSION_TASK_REMOVE_PLAYERS, "RemovePlayers", session->m_name, session)
, m_players(players)
{}

bool netSessionTask_RemovePlayers::init()
{
	if(!m_session->RemovePlayers(m_players, &m_notification))
	{
		return end(STATUS_ERROR);
	}
	return true;
}


netSessionTask_ModifySession::netSessionTask_ModifySession(netSession* session, int flags, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig)
: netSessionTask(NET_SESSION_TASK_MODIFY_SESSION, "ModifySession", session->m_name, session)
, m_flags(flags)
, m_parameters(parameters)
, m_slotConfig(slotConfig)
{}

bool netSessionTask_ModifySession::init()
{
	// modify params
	if(!m_session->Modify(	0, //
							m_slotConfig, 
							m_parameters.First(),
							m_parameters.Size(),
							&m_notification))
	{
		return end(STATUS_ERROR);
	}
	return true;
}

netSessionTask_EnableQoS::netSessionTask_EnableQoS(netSession* session, netSessionQoSRef sessionQoS)
: netSessionTask(NET_SESSION_TASK_ENABLE_QOS, "EnableQoS", session->m_name, session)
, m_sessionQoS(sessionQoS)
{}

bool netSessionTask_EnableQoS::update()
{
	netArray<1024> buffer;
	if(!m_sessionQoS->serialise(&buffer))
	{
		return end(STATUS_ERROR);
	}

	if(!m_session->EnableQos(1000, (U8*)buffer.buffer(), buffer.getWriteByteCount()))
	{
		logError("EnableQos() failed!");
		return end(STATUS_ERROR);
	}
	else
	{
		return end(STATUS_OK);
	}
}

netSessionTask_DisableQoS::netSessionTask_DisableQoS(netSession* session)
: netSessionTask(NET_SESSION_TASK_DISABLE_QOS, "DisableQoS", session->m_name, session)
{}

bool netSessionTask_DisableQoS::update()
{
	if(!m_session->QosEnabled())
	{
		logWarn("DisableQos() already disabled!");
		return end(STATUS_OK);
	}

	if(!m_session->DisableQos())
	{
		logError("DisableQos() failed!");
		return end(STATUS_ERROR);
	}
	else
	{
		return end(STATUS_OK);
	}
}



netSessionTask_RegisterArbitration::netSessionTask_RegisterArbitration(netSession* session, const NetPlay::ArbitrationNonce& arbitrationInfo)
: netSessionTask(NET_SESSION_TASK_REGISTER_ARBITRATION, "RegisterArbitration", session->m_name, session)
, m_arbitrationInfo(arbitrationInfo)
{}

bool netSessionTask_RegisterArbitration::init()
{
	if(!m_session->RegisterForArbitration(m_arbitrationInfo, &m_notification))
	{
		return end(STATUS_ERROR);
	}
	return true;
}

netSessionTask_MigrateHost::netSessionTask_MigrateHost(netSession* session, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig)
: netSessionTask(NET_SESSION_TASK_MIGRATE_HOST, "MigrateHost", session->m_name, session)
, m_parameters(parameters)
, m_slotConfig(slotConfig)
{}

bool netSessionTask_MigrateHost::init()
{
	if(!m_session->Migrate(&m_notification, m_parameters.First(), m_parameters.Size(), m_slotConfig))
	{
		return end(STATUS_ERROR);
	}
	return true;
}

netSessionTask_MigratePeer::netSessionTask_MigratePeer(netSession* session, const NetPlay::MigrationInfo& migrationInfo, const NetPlay::SessionParameterList& parameters, const NetPlay::SessionSlotConfig& slotConfig)
: netSessionTask(NET_SESSION_TASK_MIGRATE_PEER, "MigratePeer", session->m_name, session)
, m_parameters(parameters)
, m_slotConfig(slotConfig)
, m_migrationInfo(migrationInfo)
{}

bool netSessionTask_MigratePeer::init()
{
	if(!m_session->MigratePeer(m_migrationInfo, &m_notification, m_slotConfig))
	{
		return end(STATUS_ERROR);
	}
	return true;
}

netSessionTask_StartPlay::netSessionTask_StartPlay(netSession* session)
: netSessionTask(NET_SESSION_TASK_START_PLAY, "StartPlay", session->m_name, session)
{}

bool netSessionTask_StartPlay::init()
{
	if(!m_session->StartPlay())
	{
		logError("failed.");
		return end(STATUS_ERROR);
	}
	return true;
}

bool netSessionTask_StartPlay::update()
{
	if (m_session->SessionState() == NetPlay::C_Session::eStateRunning)
		return true;

	return end(STATUS_OK);
}


netSessionTask_EndPlay::netSessionTask_EndPlay(netSession* session)
: netSessionTask(NET_SESSION_TASK_END_PLAY, "EndPlay", session->m_name, session)
{}

bool netSessionTask_EndPlay::init()
{
	if(!m_session->EndPlay())
	{
		logError("failed.");
		return end(STATUS_ERROR);
	}
	return true;
}

bool netSessionTask_EndPlay::update()
{
	if (m_session->SessionState() == NetPlay::C_Session::eStatePlaying)
		return true;

	return end(STATUS_OK);
}






netSession::netSession(const char* name)
: m_name(name)
{
	Initialise(NET_MAX_PLAYERS, 64);
}

netSession::~netSession()
{
}

bool netSession::matchmaking() const
{
	return m_sessionState.notNull() && (m_sessionState->m_flags & NetPlay::SESSION_CREATE_USES_MATCHMAKING);
}

bool netSession::creating()
{
	return taskPending(NET_SESSION_TASK_CREATE_HOST) || taskPending(NET_SESSION_TASK_CREATE_PEER);
}

bool netSession::migrating()
{
	if(taskPending(NET_SESSION_TASK_MIGRATE_HOST) || taskPending(NET_SESSION_TASK_MIGRATE_PEER))
		return true;
	
	if(m_pendingState.isNull() || m_sessionState.isNull())
		return false;

	if(m_pendingState->m_hosting)
	{
		return (!m_sessionState->m_hosting);
	}
	else
	{
		return 0 != memcmp(&(m_pendingState->m_migrationInfo), &(m_sessionState->m_migrationInfo), sizeof(NetPlay::MigrationInfo));
	}
}

bool netSession::arbitrating()
{
	if(taskPending(NET_SESSION_TASK_REGISTER_ARBITRATION))
		return true;

	if(m_pendingState.isNull() || m_sessionState.isNull())
		return false;

	return 0 != memcmp(&m_pendingState->m_arbitrationInfo, &m_sessionState->m_arbitrationInfo, sizeof(NetPlay::ArbitrationNonce));
}

NetPlay::ArbitrationNonce netSession::arbitrationInfo()
{
	NetPlay::SessionSecurity sec;
	GetSecurityInfo(sec);
	return sec;
}

bool netSession::startPlay()
{
	if(!running())
		return false;

	m_tasks.addHead(new netSessionTask_StartPlay(this));
	return true;
}

bool netSession::endPlay()
{
	if(!playing())
		return false;

	m_tasks.addHead(new netSessionTask_EndPlay(this));
	return true;
}

bool netSession::usesArbitration()
{
	if(m_sessionState.isNull())
	{
		return false;
	}
	return ((m_sessionState->m_flags & NetPlay::SESSION_CREATE_USES_ARBITRATION) != 0);
}

bool netSession::usesStats()
{
	if(m_sessionState.isNull())
	{
		return false;
	}
	return ((m_sessionState->m_flags & NetPlay::SESSION_CREATE_USES_STATS) != 0);
}

netSessionStateRef netSession::pendingState() 
{ 
	return m_pendingState; 
}

netSessionQoSRef netSession::sessionQoS()
{
	return m_sessionQoS;
}

netSessionStateRef netSession::sessionState() 
{ 
	return m_sessionState; 
}

void netSession::pendingQoS(netSessionQoSRef qos)
{
	m_pendingQoS = qos;
}

bool netSession::create(netSessionStateRef state)
{
	if(!notActive())
	{
		LOGERROR(netDebug::g_sessionLog, "<session '%s'> session is not inactive. cant create.", m_name);
		return false;
	}

	m_pendingState = state;

	if(!state->m_hosting)
	{
		m_tasks.addTail(new netSessionTask_CreatePeer(this, state));
	}
	else
	{
		m_tasks.addTail(new netSessionTask_CreateHost(this, state));
	}
	return true;
}

bool netSession::update()
{
	updateTasks();

	if(error() || notActive())
		return false;

	if(busy())
		return false;

	updateQoS();
	updatePlayers();
	updateParams();
	updateArbitration();
	updateMigration();
	return true;
}

bool netSession::busy()
{
	if(creating())
		return true;

	if(error())
		return false;

	return !(m_tasks.isEmpty());
}

bool netSession::taskPending(int taskId)
{
	if(m_tasks.isEmpty())
		return false;

	bdLinkedList<netSessionTaskRef>::Position pos = m_tasks.getHeadPosition();
	while(pos)
	{
		netSessionTaskRef task = m_tasks.forward(pos);
		if(task->id() == taskId)
			return true;
	}
	return false;
}

bool netSession::onTasksSuccess(netSessionTaskRef task)
{
	switch(task->id())
	{
	default:
		{
			LOGERROR(netDebug::g_sessionLog, "<session '%s'> task %s unhandled!", m_name, task->name());
			return false;
		}
	case NET_SESSION_TASK_CREATE_PEER:
		{
			netSessionTask_CreatePeerRef create = (netSessionTask_CreatePeerRef)task;
			m_sessionState = create->m_sessionState;
			GetSecurityInfo(m_sessionState->m_securityInfo);
			GetSecurityInfo(m_pendingState->m_securityInfo);
			return true;
		}
	case NET_SESSION_TASK_CREATE_HOST:
		{
			netSessionTask_CreateHostRef create = (netSessionTask_CreateHostRef)task;
			m_sessionState = create->m_sessionState;
			GetSecurityInfo(m_sessionState->m_securityInfo);
			GetSecurityInfo(m_pendingState->m_securityInfo);
			return true;
		}
	case NET_SESSION_TASK_ADD_PLAYERS:
		{
			netSessionTask_AddPlayersRef add = (netSessionTask_AddPlayersRef)task;
			for(int i = 0; i < add->m_players.Size(); i ++)
			{
				m_sessionState->addUser(add->m_players[i]);
			}
			return true;
		}
	case NET_SESSION_TASK_REMOVE_PLAYERS:
		{
			netSessionTask_RemovePlayersRef remv = (netSessionTask_RemovePlayersRef)task;
			for(int i = 0; i < remv->m_players.Size(); i ++)
			{
				m_sessionState->removeUser(remv->m_players[i]);
			}
			return true;
		}
	case NET_SESSION_TASK_MODIFY_SESSION:
		{
			netSessionTask_ModifySessionRef mod = (netSessionTask_ModifySessionRef)task;
			m_sessionState->m_slotConfig	= mod->m_slotConfig;
			m_sessionState->m_flags			= mod->m_flags;
			m_sessionState->m_parameters	= mod->m_parameters;
			return true;
		}
	case NET_SESSION_TASK_DISABLE_QOS:
		{
			netSessionTask_DisableQoSRef qos = (netSessionTask_DisableQoSRef)task;
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> qos disabled!", m_name);
			m_sessionQoS = NULL;
			return true;
		}
	case NET_SESSION_TASK_ENABLE_QOS:
		{
			netSessionTask_EnableQoSRef qos = (netSessionTask_EnableQoSRef)task;

			if (m_sessionQoS.isNull())
			{
				m_sessionQoS = new netSessionQoS;
				LOGINFO(netDebug::g_sessionLog, "<session '%s'> qos created!", m_name);
			}
			else
			{
				LOGINFO(netDebug::g_sessionLog, "<session '%s'> qos updated!", m_name);
			}			
			*m_sessionQoS = *qos->m_sessionQoS;
			return true;
		}
	case NET_SESSION_TASK_REGISTER_ARBITRATION:
		{
			netSessionTask_RegisterArbitrationRef arb = (netSessionTask_RegisterArbitrationRef)task;
			m_sessionState->m_arbitrationInfo = arb->m_arbitrationInfo;
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> arbitrated nonce(%s)!", m_name, netDebug::getByteInfo(&arb->m_arbitrationInfo, sizeof(arb->m_arbitrationInfo)));
			return true;
		}
	case NET_SESSION_TASK_MIGRATE_HOST:
		{
			netSessionTask_MigrateHostRef mig = (netSessionTask_MigrateHostRef)task;
			m_sessionState->m_parameters = mig->m_parameters;
			m_sessionState->m_slotConfig = mig->m_slotConfig;
			m_sessionState->m_hosting = true;
			GetSecurityInfo(m_sessionState->m_securityInfo);
			GetMigrationInfo(m_sessionState->m_migrationInfo);
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> host-migrated!", m_name);
			return true;
		}
	case NET_SESSION_TASK_MIGRATE_PEER:
		{
			netSessionTask_MigratePeerRef mig = (netSessionTask_MigratePeerRef)task;
			m_sessionState->m_parameters = mig->m_parameters;
			m_sessionState->m_slotConfig = mig->m_slotConfig;
			m_sessionState->m_hosting = false;
			GetSecurityInfo(m_sessionState->m_securityInfo);
			GetMigrationInfo(m_sessionState->m_migrationInfo);
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> peer-migrated!", m_name);
			return true;
		}
	case NET_SESSION_TASK_START_PLAY:
		{
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> started!", m_name);
			return true;
		}
	case NET_SESSION_TASK_END_PLAY:
		{
			LOGINFO(netDebug::g_sessionLog, "<session '%s'> ended!", m_name);
			return true;
		}
	}
	return false;
}

bool netSession::updateTasks()
{
	// now update current task in the queue.
	if(m_tasks.isEmpty())
		return true;

	netSessionTaskRef task = m_tasks.getTail(); 

	task->poll();

	if(task->finished())
	{
		if(task->success())
		{
			onTasksSuccess(task);
		}
		else
		{
			LOGERROR(netDebug::g_sessionLog, "<session '%s'> task %s failed!", m_name, task->name());
		}
		m_tasks.removeTail();
	}
	return true;
}


bool netSession::updateParams()
{
	if(taskPending(NET_SESSION_TASK_MODIFY_SESSION))
		return false;

	if(m_sessionState.isNull() || m_pendingState.isNull())
		return false;

	int flags1 = m_sessionState->m_flags;
	int flags2 = m_pendingState->m_flags;

	// flags have changed.
	bool diff = false;

	if(flags1 != flags2)
	{
		LOGINFO(netDebug::g_Log, "<netSession::Modify> flags changed (0x%x -> 0x%x)", flags1, flags2);
		diff = true;
	}
	
	// check for other differences.
	if(!diff)
	{
		// check slot config.
		NetPlay::SessionSlotConfig slots1 = m_sessionState->m_slotConfig;
		NetPlay::SessionSlotConfig slots2 = m_pendingState->m_slotConfig;
		diff |= (slots1.m_MaxPublic		!= slots2.m_MaxPublic);
		diff |= (slots1.m_MaxPrivate	!= slots2.m_MaxPrivate);
		diff |= (slots1.m_PrivateInUse	!= slots2.m_PrivateInUse);
		diff |= (slots1.m_PublicInUse	!= slots2.m_PublicInUse);

		// check for change in session parameters
		if(!diff)
		{
			NetPlay::SessionParameterList parametersAdded;
			NetPlay::SessionParameterList parametersRemoved;
			NetPlay::SessionParameterList parametersChanged;

			// scan old list. see if parameters were removed.
			for(int i = 0; i < m_sessionState->m_parameters.Size(); i ++)
			{
				NetPlay::SessionParameter oldParam = m_sessionState->m_parameters[i];

				NetPlay::SessionParameter* newParam = m_pendingState->findParameter(oldParam);
				
				if(newParam == NULL)
				{
					parametersRemoved.PushBack(oldParam);
				}
				else
				{
					// param changed.
					if(netSessionState::compareParameters(&oldParam, newParam) != 0)
					{
						parametersChanged.PushBack(m_pendingState->m_parameters[i]);
					}
				}
			}

			// scan new list. see if parameters were added.
			for(int i = 0; i < m_pendingState->m_parameters.Size(); i ++)
			{
				NetPlay::SessionParameter newParam = m_pendingState->m_parameters[i];

				NetPlay::SessionParameter* oldParam = m_sessionState->findParameter(newParam);
				
				if(oldParam == NULL)
				{
					parametersAdded.PushBack(newParam);
				}
			}

			// check if we added / removed / cahnged a parameter.
			// if so, we need to update the session parameters.
			if(	!parametersAdded.Empty() ||
				!parametersRemoved.Empty() ||
				!parametersChanged.Empty())
			{
			#ifndef CFG_GOLD
				debugparamsList("REMOVED",	parametersRemoved);
				debugparamsList("ADDED",	parametersAdded);
				debugparamsList("CHANGED",	parametersChanged);
			#endif
				diff = true;
			}
		}
	}

	// parameters have changed.
	// push a task to modify the session params.
	if (diff)
	{
		m_tasks.addHead(new netSessionTask_ModifySession(this, m_pendingState->m_flags, m_pendingState->m_parameters, m_pendingState->m_slotConfig));
	}
	return true;
}

bool netSession::updatePlayers()
{
	if(taskPending(NET_SESSION_TASK_REMOVE_PLAYERS) || taskPending(NET_SESSION_TASK_ADD_PLAYERS))
		return false;

	if(m_sessionState.isNull() || m_pendingState.isNull())
		return false;

	NetPlay::C_Session::SessionUserList	addList;
	NetPlay::C_Session::SessionUserList	removeList;
	
	// players to remove first
	for(int i = 0; i < m_sessionState->m_players.Size(); i ++)
	{
		NetPlay::C_Session::SessionUser oldPlayer = m_sessionState->m_players[i];

		NetPlay::C_Session::SessionUser* newPlayer = m_pendingState->findUser(oldPlayer);
	
		if(newPlayer == NULL)
		{
			removeList.PushBack(oldPlayer);
		}
		else
		{
			if(newPlayer->m_PublicSlot != oldPlayer.m_PublicSlot)
			{
				LOGWARN(
					netDebug::g_sessionLog, 
					"<netSession::updatePlayers> player slot (%llu) changed from '%s' to '%s'", 
					netPlatform::Core::instance()->getUserId(newPlayer->m_User), 
					oldPlayer.m_PublicSlot? "public" : "private", 
					newPlayer->m_PublicSlot? "public" : "private");
			}
		}
	}

	// players to remove first
	for(int i = 0; i < m_pendingState->m_players.Size(); i ++)
	{
		NetPlay::C_Session::SessionUser newPlayer = m_pendingState->m_players[i];

		NetPlay::C_Session::SessionUser* oldPlayer = m_sessionState->findUser(newPlayer);
	
		if(oldPlayer == NULL)
		{
			addList.PushBack(newPlayer);
		}
	}

	// push task
	if(removeList.Size() > 0)
	{
		m_tasks.addHead(new netSessionTask_RemovePlayers(this, removeList));
	}
	// push task
	if(addList.Size() > 0)
	{
		m_tasks.addHead(new netSessionTask_AddPlayers(this, addList));
	}
	return true;
}

bool netSession::updateMigration()
{
	// migration in progress
	if(taskPending(NET_SESSION_TASK_MIGRATE_HOST) || taskPending(NET_SESSION_TASK_MIGRATE_PEER))
		return false;

	if(m_sessionState.isNull())
		return false;

	// we are hosting now. migrate as host.
	if(m_pendingState->m_hosting && !m_sessionState->m_hosting)
	{
		m_tasks.addHead(new netSessionTask_MigrateHost(this, m_pendingState->m_parameters, m_pendingState->m_slotConfig));
		return true;
	}

	// we are a peer. check if a host migrated
	if(!m_pendingState->m_hosting)
	{
		// migration info changed. Migrate to that info.
		if(memcmp(&(m_pendingState->m_migrationInfo), &(m_sessionState->m_migrationInfo), sizeof(NetPlay::MigrationInfo)))
		{
			m_tasks.addHead(new netSessionTask_MigratePeer(this, m_pendingState->m_migrationInfo, m_pendingState->m_parameters, m_pendingState->m_slotConfig));
		}
	}
	return true;
}
	
bool netSession::updateArbitration()
{
	if(!usesArbitration())
		return false;

	if(taskPending(NET_SESSION_TASK_REGISTER_ARBITRATION))
		return false;

	if(m_sessionState.isNull() || m_pendingState.isNull())
		return false;

	if(memcmp(&m_pendingState->m_arbitrationInfo, &m_sessionState->m_arbitrationInfo, sizeof(NetPlay::ArbitrationNonce)))
	{
		m_tasks.addTail(new netSessionTask_RegisterArbitration(this, m_pendingState->m_arbitrationInfo));
	}
	return true;
}

bool netSession::updateQoS()
{
	if(!m_qosTick.ticked(2 * 1000 * 1000))
		return false;

	if(taskPending(NET_SESSION_TASK_ENABLE_QOS) || taskPending(NET_SESSION_TASK_DISABLE_QOS))
		return false;

	bool createQoS  = (m_pendingQoS.notNull() && m_sessionQoS.isNull());
	bool destroyQoS = (m_pendingQoS.isNull()  && m_sessionQoS.notNull());
	bool updateQoS  = (m_pendingQoS.notNull() && m_sessionQoS.notNull() && (*m_sessionQoS != *m_pendingQoS));
	
	if(createQoS || updateQoS)
	{
		m_tasks.addHead(new netSessionTask_EnableQoS(this, m_pendingQoS));
	}
	if(destroyQoS)
	{
		m_tasks.addHead(new netSessionTask_DisableQoS(this));
	}
	return true;		
}

bool netSession::debugparamsList(const char* header, const NetPlay::SessionParameterList& list)
{
	for(NetPlay::SessionParameterList::ConstIterator it = list.Begin(); it != list.End(); ++it)
	{
		const NetPlay::SessionParameter& param = *it;

		switch(param.m_ParameterType)
		{
		default:
			{
				LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s invalid session parameter type %d", m_name, header, param.m_ParameterType);
				break;
			}
		case NetPlay::SessionParameter::eTypeContext:
			{
				LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s Context(0x%8x), Value(%d)", m_name, header, param.m_Data.m_ContextData.m_Id, param.m_Data.m_ContextData.m_Value);
				break;
			}
		case NetPlay::SessionParameter::eTypeProperty:
			{
				switch(param.m_Data.m_PropertyData.m_ValueType)
				{
				default: 
					{
						LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s unhandled property type(%d) for property(0x%8x)", m_name, header, param.m_Data.m_PropertyData.m_ValueType, param.m_Data.m_PropertyData.m_Id);
						break;
					}
				case NetPlay::SessionParameter::Int32:
					{
						LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s Property(0x%8x), Int32(%d)", m_name, header, param.m_Data.m_PropertyData.m_Id, param.m_Data.m_PropertyData.m_Value.m_S32);
						break;
					}
				case NetPlay::SessionParameter::Int64:
					{
						LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s Property(0x%8x), Int64(%d)", m_name, header, param.m_Data.m_PropertyData.m_Id, param.m_Data.m_PropertyData.m_Value.m_S64);
						break;
					}
				case NetPlay::SessionParameter::Float64:
					{
						LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s Property(0x%8x), Float64(%d)", m_name, header, param.m_Data.m_PropertyData.m_Id, param.m_Data.m_PropertyData.m_Value.m_F64);
						break;
					}
				case NetPlay::SessionParameter::Float32:
					{
						LOGINFO(netDebug::g_sessionLog, "<session '%s'> %s Property(0x%8x), Float32(%d)", m_name, header, param.m_Data.m_PropertyData.m_Id, param.m_Data.m_PropertyData.m_Value.m_F32);
						break;
					}
				}
			}
		}
	}
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

