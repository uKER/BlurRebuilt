// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdSocket/bdQoS/bdQoSProbe.h>
#include <bdSocket/bdSocketRouter.h>

#include "CustomMatchMakingInfo.h"
#include "MatchMakingQueries.h"
#include "QOSListener.h"
#include "QOSManager.h"

#define MAX_SESSION_RESULTS 5

#if defined BD_PLATFORM_PSP
SCE_MODULE_INFO(bitdemon, 0, 1, 1);
SceInt32 sce_newlib_heap_kb_size = 1000;
#endif

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON

#include <bdExample/bdExampleCommonLobby/LiveServices.h>

// To use the QOS Example, you need to provide your own 
// LIVE_LSP_SERVICE_ID and LIVE_TITLE_ID.  
// You will also need to ensure that the LSP port is setup
// correctly.
#define LIVE_LSP_SERVICE_ID (0x1234ABCD)// -> DUMMY VALUE
#define LIVE_TITLE_ID		(0x1234ABCD)// -> DUMMY VALUE
#define LSP_PORT	(1234u)				// -> DUMMY VALUE

#ifdef BD_PLATFORM_XENON
	#include <xtitleidbegin.h>
	XEX_TITLE_ID(LIVE_TITLE_ID)
	#include <xtitleidend.h>
#endif //#ifdef BD_PLATFORM_XENON

#endif


// Anonymous Authentication
#define ANON_AUTH_TITLE_ID (0x14CD)

class SimpleMatchMakingExample
{
	enum AppStages
	{
		ADVERTISE_GAME,
		FIND_GAME,
		QOS_INFO_PROBE,
		FINISHED
	};

public:
	SimpleMatchMakingExample()
	:	m_errorFlag(false),
		m_createTask(BD_NULL),
		m_findTask(BD_NULL),
		m_QoSProbeActiveFlag(false),
		m_authService(BD_NULL),
		m_nextTask(ADVERTISE_GAME),
		m_matchMaking(BD_NULL),
		m_qosProber(BD_NULL),
		m_once(false),
		m_once2(false),
		m_once3(false),
		m_signedIn(false)
	{

	};

	~SimpleMatchMakingExample()
	{

	};

	void init(const bdBool dispayLogOutput = false)
	{

		if(true == dispayLogOutput)
		{
			// we want to hear about all warnings and errors.
			m_logSubscriber.addChannel("warn");
			m_logSubscriber.addChannel("err");
			m_logSubscriber.addChannel("info");
		}

		m_lobbyService = new bdLobbyService();
		startNetworkLayer();

	};

	void step()
	{
		if(true == m_errorFlag)
		{
			return;
		}

		bdNetImpl *const net = bdNet::getInstance();
		// we won't do anything until the network layer is initialised fully
		if(net->getStatus() == bdNetImpl::BD_NET_PENDING)
		{
			net->pump();
			return;
		}
		

		// Check bdNet has been started successfully.
		if(net->getStatus() != bdNetImpl::BD_NET_DONE)
		{
			// an error has occurred.
			m_errorFlag = true;
			return;
		}
		else
		{
			net->receiveAll();
			net->dispatchAll();
			net->sendAll();

			if(false == m_once)
			{
				m_once = true;
				bdPrintf("Net layer successfully initialized. \n");
				authorizeExampleAccount();
			}
		}

		// getStatus also pumps the auth service
		if(	m_signedIn == false )
		{
			pumpAuth();
			// don't do anything until more account authorization is finished
			return;
		}
		else
		{
			if(false == m_once2)
			{
				m_once2 = true;
				if (true == isAuthorisedOK())
				{

					bdPrintf("Account successfully authorized. \n");
					connectToMatchMakingServer();
				}
				else
				{
					bdPrintf("Account authorization failed. \n");
					m_errorFlag = true;
				}
			}
		}
		m_lobbyService->pump();
		if(	m_lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			if(false == m_once3)
			{
				m_once3 = true;
				bdPrintf("Connected successfully to MatchMaking server. \n");
				m_matchMaking = m_lobbyService->getMatchMaking();
			}


			// if no task is active then start a new one..
			if(	m_createTask.isNull() &&
				m_findTask.isNull()   &&
				false == m_QoSProbeActiveFlag &&
				FINISHED != m_nextTask )
			{
				startNextTask();
			}
			else
			{
				// pump the active task..
				pumpActiveTask();
			}
		}
		else if (m_lobbyService->getStatus() == bdLobbyConnection::BD_DISCONNECTED)
		{
			bdPrintf("Disconnected from MatchMaking server. \n");
			m_errorFlag = true;
		}
	};

	void quit()
	{
		if(m_authService)
		{
			delete m_authService;
		}

		bdNetImpl *const net = bdNet::getInstance();
		if(net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			if (m_lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
			{
				m_lobbyService->disconnect();
			}

			net->sendAll();
			net->stop();
		}
		if(m_lobbyService)
		{
			delete m_lobbyService;
		}
		bdExamplePlatformInit::quit();
	};

public:
	bdBool m_errorFlag;


protected:

	void startNetworkLayer()
	{
		bdNetImpl *const net = bdNet::getInstance();

		// try to start bdNet
		bdNetStartParams params;

		// initialize params with platform specific values.
		bdExamplePlatformInit::init(params);

		params.m_onlineGame = true;
		m_errorFlag = !(net->start(params));

	};

	void authorizeExampleAccount()
	{
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		m_liveServices.signOn(LIVE_LSP_SERVICE_ID);
		m_signedIn = false;
#else
		bdInAddr authAddr;
		bdPlatformSocket::getHostByName(AUTH_ADDRESS, &authAddr, 1, 0);

		bdInetAddr inAddr(authAddr.m_inAddri);
		bdAddr authServerAddr(inAddr, AUTH_PORT);
		m_authService = new bdAuthService(ANON_AUTH_TITLE_ID, authServerAddr);

        m_errorFlag = !(m_authService->authorizeAnonymous());
#endif

	};

	void connectToMatchMakingServer()
	{

		bdInAddr lsgAddr;

#if defined BD_PLATFORM_XBOX
		// do nothing
		m_errorFlag = true;
		BD_ERR(BD_LOG_LEVEL, "Not Implemented on XBox");

#elif defined BD_PLATFORM_XENON

		BD_INFO("test", "Signed in... Looking up SG Lite's address.");

		//bdCommonAddrRef ca;
		bdAddr ca;
		bdSecurityID id;
		bdSecurityKey key;
		if (!m_liveServices.resolveTitleAddr(LIVE_LSP_SERVICE_ID, LSP_PORT, ca, id, key))
		{
			BD_WARN("test", "Failed to resolve LSG address. Failing now.");
			m_errorFlag = true;
		}
		
		if(false == m_errorFlag)
		{
			BD_INFO("test", "SG Lite address found.");

			// Prepare auth info
			bdAuthTicket ticket;
			ticket.m_userID = m_liveServices.getXUserId();
			ticket.m_titleID = ANON_AUTH_TITLE_ID;
			m_liveServices.getXUserName(ticket.m_username);

			bdAuthInfo authInfo;
			authInfo.m_titleID = ANON_AUTH_TITLE_ID;
			ticket.serialize(authInfo.m_data);

			// Register SG Lite's key
			//bdNet::getInstance()->getKeyMap()->put(id, key);

			BD_INFO("test", "Connecting to matchmaking...");
			m_errorFlag = !(m_lobbyService->connect(ca, id, &authInfo));

			bdPrintf ("Connecting to MatchMaking+ sandbox...");
		}

#else 
		bdPlatformSocket::getHostByName(LSG_ADDRESS , &lsgAddr, 1, 0);
		bdInetAddr inLSGAddr(lsgAddr.m_inAddri);
		bdAddr LSGServerAddr(inLSGAddr, LSG_PORT );

		m_errorFlag = !(m_lobbyService->connect(LSGServerAddr, m_authService->getAuthInfo()));
#endif
	};

	void startNextTask()
	{
		// this code loops through 4 steps repeatedly
		// where m_nextTask is setup at the end of the preceding task.
		// 1) advertise a game
		// 2) find a game
		// 3) Run QoS Probes on discovered games..
		// 4) Wait and reply to other QoS probes

		switch(m_nextTask)
		{
		case ADVERTISE_GAME:
			{
				bdPrintf("\n Starting Advertise Game Task. \n");

				bdNetImpl *const net = bdNet::getInstance();
				// Create and setup a game info
				bdCommonAddrRef sessionsHostAddress = net->getLocalCommonAddr();

				// Create a game for 8 players, with ourselves being one of them
				bdByte8 numPublicOpen    = 2;
				// These are title specific fields
				bdNChar8 sessionName[8]   = "game101";
				bdByte8 levelNum		 = 5;
				bdByte8 gameType		 = 101;

				m_customMMInfo.setHostAddr(sessionsHostAddress);
				m_customMMInfo.m_maxPlayers = numPublicOpen;
				m_customMMInfo.setSessionName(sessionName);
				m_customMMInfo.m_levelNum = levelNum;
				m_customMMInfo.m_gameType = gameType;
				bdTrulyRandom::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8 *>(m_customMMInfo.m_secKey.ab), BD_SECURITY_KEY_LENGTH);
				m_createTask = m_matchMaking->createSession(&m_customMMInfo, &m_sessionID);
				if (m_createTask == BD_NULL)
				{
					m_errorFlag = true;
				}
			}
			break;
		case FIND_GAME:
			{
				bdPrintf("\n Starting Find Game Task. \n");
				FindSessionsByGameType queryParams(101);
				m_findTask = m_matchMaking->findSessions(FINDSESSIONSBYGAMETYPE, 0, MAX_SESSION_RESULTS, &queryParams, m_queryResults);
				if (m_findTask == BD_NULL)
				{
					// handle errors
					m_errorFlag = true;
				}
			}
			break;
		case QOS_INFO_PROBE:
			{
				bdPrintf("Querying Each For Game Info \n" );
				m_qosListener.init(m_numSessionInfos);
				bdCommonAddrRef localCommonAddr = bdNet::getInstance()->getLocalCommonAddr();

				for(bdUInt i = 0; i < m_numSessionInfos; i++)
				{
					bdQoSRemoteAddr qosAddr(m_queryResults[i].getHostAddrAsCommonAddr(localCommonAddr),
											m_queryResults[i].m_sessionID,
											m_queryResults[i].m_secKey);

					m_qosProber->probe(	qosAddr , &m_qosListener);
				}
				m_QoSProbeActiveFlag = true;
			}
			break;
		default:
			{
				m_errorFlag = true;
			}
		}
	};

	void pumpActiveTask()
	{
		m_lobbyService->pump();
		if(	m_createTask.notNull() && (m_createTask->getStatus() == bdRemoteTask::BD_DONE))
		{
		
			bdPrintf("Game advertised successfully \n" );

			// Retrieve a pointer to the bdQoSProbe class
			if(BD_NULL == m_qosProber)
			{
				bdNetImpl *const net = bdNet::getInstance();
				m_qosProber = net->getSocketRouter()->getQoSProber();
			}

			// Once the game is advertised we can initialise the QoS Manager
			m_qosManager.init(m_qosProber , m_sessionID.m_sessionID);
			m_createTask = BD_NULL;
			m_nextTask = FIND_GAME;
		}
		else if (m_createTask.notNull() && (m_createTask->getStatus() == bdRemoteTask::BD_FAILED))
		{
			bdPrintf("Game advertisement failure  \n" );
			m_errorFlag = true;
		}
		

		if(	m_findTask.notNull() && (m_findTask->getStatus() == bdRemoteTask::BD_DONE))
		{
			
			// Note : task may be successful, but not actually find any sessions
			m_numSessionInfos = m_findTask->getNumResults();
			bdPrintf("Found %i sessions \n" , m_numSessionInfos);
			if(m_numSessionInfos > 0)
			{
				m_nextTask = QOS_INFO_PROBE;
				m_findTask = BD_NULL;
			}
			else
			{
				m_nextTask = FINISHED;
				bdPrintf("No games found to probe \n" );
				bdPrintf(" \n \n -- Listening for incoming QoS probes -- \n \n" );
			}

		}
		else if (m_findTask.notNull() && (m_findTask->getStatus() == bdRemoteTask::BD_FAILED))
		{
			bdPrintf("Game search failure  \n" );
			m_errorFlag = true;
		}
		
		if(m_qosProber)
		{
			m_qosProber->pump();
			if(true == m_QoSProbeActiveFlag)
			{
				if(true == m_qosListener.isFinished())
				{
					m_QoSProbeActiveFlag = false;
					m_nextTask = FINISHED;
					bdPrintf("Finished Querying Each For Game Info \n" );
					bdPrintf(" \n \n -- Listening for incoming QoS probes -- \n \n" );
				}
			}
		}
	};

	void pumpAuth()
	{
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		const bdBool pumpResult = m_liveServices.pump();
		if(true == pumpResult)
		{

			if (!m_signedIn && m_liveServices.isSignedOn())
			{
				m_signedIn = true;
			}
		}
		else
		{
			m_errorFlag = true;
		}
#else
		if(m_authService->getStatus() == bdAuthService::BD_READY)
		{
			m_signedIn = true;
		}
#endif
	};

	bdBool isAuthorisedOK()
	{
#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		return m_liveServices.isSignedOn();
#else
		return (m_authService->getErrorCode() == BD_AUTH_NO_ERROR);
#endif
	}	

	bdRemoteTaskRef  m_createTask;
	bdRemoteTaskRef  m_findTask;
	bdBool m_QoSProbeActiveFlag;

	bdAuthService *m_authService;
	bdLobbyService *m_lobbyService;
	AppStages m_nextTask;

	// Pointer to the matchmaking client object
	bdMatchMaking *m_matchMaking;

	CustomMatchMakingInfo m_customMMInfo;
	bdSessionID	m_sessionID;

	CustomMatchMakingInfo m_queryResults[MAX_SESSION_RESULTS];
	bdUInt m_numSessionInfos;

	bdQoSProbe *m_qosProber;
	QOSListener m_qosListener;

	bdLogSubscriber m_logSubscriber;

	QOSManager m_qosManager;

	bdBool m_once;
	bdBool m_once2;
	bdBool m_once3;

	bdBool m_signedIn;

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
	LiveServices m_liveServices;
#endif 

};
int BD_CALL main(int argc, char* argv[])
{

	bdCore::init();
	
	{
		// Application will run forever, 
		// Change this flag in a debugger to end gracefully..
		bdBool quitFlag = false;

		SimpleMatchMakingExample app;
		app.init(true);
		while(false == quitFlag)
		{
			app.step();
			if(true == app.m_errorFlag )
			{
				quitFlag = true;
			}
			// remove this to improve ping times :)
			bdPlatformTiming::sleep(30);
		}
		app.quit();

	}

	bdCore::quit();
	return 0;
}
