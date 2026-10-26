// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PS3 PSP-INFRA WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdMatchMaking/bdSessionID.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdSocket/bdAddressMap.h>
#include <bdSocket/bdSocketRouter.h>
#include <bdSocket/bdDTLS/bdDTLSAssociation.h>
#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#include "CustomMatchMakingInfo.h"
#include "MatchMakingQueries.h"

#define MAX_SESSION_RESULTS 5

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
		m_authService(BD_NULL),
		m_nextTask(ADVERTISE_GAME),
		m_matchMaking(BD_NULL),
		m_connecting(false),
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
		m_timer.start();
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
		net->pump();
		// we won't do anything until the network layer is initialised fully
		if(net->getStatus() == bdNetImpl::BD_NET_PENDING)
		{
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

			if(false == m_once)
			{
				m_once = true;
				BD_INFO(BD_LOG_LEVEL , "Net layer successfully initialized. ");
				authorizeExampleAccount();
			}

			bdSocketRouter *socketRouter = net->getSocketRouter();

			// Pump socket router every network frame
			socketRouter->pump();

			// track all lost addr handles
			bdSet<bdAddrHandleRef> toRemove;

			bdBool resetTimerFlag = false;

			// process received data
			bdUByte8 recvBuffer[ BD_MAX_DATAGRAM_SIZE];
			bdAddrHandleRef handle = BD_NULL;
			while (socketRouter->receiveFrom(handle, recvBuffer, BD_MAX_DATAGRAM_SIZE) > 0)
			{
				// all we care about is new data
				if(m_addrHandleStore.insert(handle))
				{
					bdNChar8 buffer2 [BD_COMMON_ADDR_INFO_SIZE];
					bdCommonAddrInfo::getBriefInfo(handle->m_endpoint.getCommonAddr() , buffer2 , BD_COMMON_ADDR_INFO_SIZE);

					BD_INFO(BD_LOG_LEVEL , "New Incoming DTLS association detected from %s " , buffer2);
				}
			}

			// Iterate all bdAddrhandles
			bdSet<bdAddrHandleRef>::Iterator iterator = m_addrHandleStore.getIterator();
			while(BD_NULL != iterator)
			{
				bdAddrHandleRef next = m_addrHandleStore.next(iterator);
				const bdDTLSAssociationStatus status = socketRouter->getStatus(next);

				if(BD_SOCKET_LOST == status )
				{
					// we cannot remove handles during container iteration so
					// we will store them until the end and then remove all of them.
					toRemove.insert(next);

					const bdAddr nextAddr = next->getRealAddr();
					const bdUInt size2 = BD_ADDR_STRING_SIZE;
					bdNChar8 buffer2 [size2];
					nextAddr.toString(buffer2 , size2);
					BD_INFO(BD_LOG_LEVEL , "Removing lost addr handle for %s " ,  buffer2);

				}
				else
				{
					
					// Send keep alive for every connected socket.
					// Send every timeout/3 to tolerate lost packet(s)
					bdSocketRouterConfig config;
					socketRouter->getConfig(config);
					const bdFloat32 socketTimout = 20;//config.m_DTLSAssociationReceiveTimeout;
					const bdFloat32 elapsedTime = m_timer.getElapsedTimeInSeconds();
					if(( (elapsedTime > (socketTimout/3)) || (true == resetTimerFlag)) && BD_SOCKET_CONNECTED == status)
					{
						// Reset keep alive timer!
						resetTimerFlag = true;

						// Create message to send over the network
						const bdUInt bufferSize = 16;
						bdUByte8 buffer [bufferSize];

						// First 2 bytes in buffer represent the size of the encrypted portion.
						const bdUInt16 headerSize = 2;

						bdUInt16 sizeOfEncryptedData = bufferSize - headerSize;
						bdUInt newOffset = 0;

						// Write encrypted data size into buffer
						bdBytePacker::appendBasicType(buffer , bufferSize, 0, newOffset, sizeOfEncryptedData );

						bdNChar8 buffer2 [BD_COMMON_ADDR_INFO_SIZE];
						bdCommonAddrInfo::getBriefInfo(next->m_endpoint.getCommonAddr(), buffer2 , BD_COMMON_ADDR_INFO_SIZE);

						// send data
						socketRouter->sendTo(next, buffer , bufferSize);
						BD_INFO(BD_LOG_LEVEL , "Sending Keep Alive to %s " , buffer2);

					}
				}
			}

			// Now cleanup lost handles
			bdSet<bdAddrHandleRef>::Iterator removalIterator = toRemove.getIterator();
			while(BD_NULL != removalIterator)
			{
				bdAddrHandleRef lostHandle = toRemove.next(removalIterator);
				m_addrHandleStore.remove(lostHandle);
			}

			if(resetTimerFlag)
			{
				m_timer.reset();
				m_timer.start();
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

					BD_INFO(BD_LOG_LEVEL , "Account successfully authorized. ");
					connectToMatchMakingServer();
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL , "Account authorization failed. ");
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
				BD_INFO(BD_LOG_LEVEL , "Connected successfully to MatchMaking server. ");
				m_matchMaking = m_lobbyService->getMatchMaking();
			}


			// if no task is active then start a new one..
			if(	m_createTask.isNull() &&
				m_findTask.isNull()   &&
				false == m_connecting &&
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
			BD_INFO(BD_LOG_LEVEL , "Disconnected from MatchMaking server. ");
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
		bdInAddr authAddr;
		bdPlatformSocket::getHostByName(AUTH_ADDRESS, &authAddr, 1, 0);

		bdInetAddr inAddr(authAddr.m_inAddri);
		bdAddr authServerAddr(inAddr, AUTH_PORT);
		m_authService = new bdAuthService(ANON_AUTH_TITLE_ID, authServerAddr);

        m_errorFlag = !(m_authService->authorizeAnonymous());

	};

	void connectToMatchMakingServer()
	{

		bdInAddr lsgAddr;

		bdPlatformSocket::getHostByName(LSG_ADDRESS , &lsgAddr, 1, 0);
		bdInetAddr inLSGAddr(lsgAddr.m_inAddri);
		bdAddr LSGServerAddr(inLSGAddr, LSG_PORT );

		m_errorFlag = !(m_lobbyService->connect(LSGServerAddr, m_authService->getAuthInfo()));
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
				BD_INFO(BD_LOG_LEVEL , "  Starting Advertise Game Task. ");

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
				BD_INFO(BD_LOG_LEVEL , " Starting Find Game Task. ");

				// This implementation searches for games with type 5
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
				BD_INFO(BD_LOG_LEVEL , "Querying Each For Game Info " );

				for(bdUInt i = 0; i < m_numSessionInfos; i++)
				{
					const bdCommonAddrRef hostAddress = bdNet::getInstance()->getLocalCommonAddr();

					bdSocketRouter *socketRouter = bdNet::getInstance()->getSocketRouter();
					socketRouter->getKeyMap()->registerKey(	m_queryResults[i].m_sessionID,
															m_queryResults[i].m_secKey);
					bdAddrHandleRef addrHandle;
					socketRouter->getAddressMap()->getAddrHandle(m_queryResults[i].getHostAddrAsCommonAddr(hostAddress),
																	m_queryResults[i].m_sessionID,
																	addrHandle);

					bdNChar8 buffer2 [BD_COMMON_ADDR_INFO_SIZE];
					bdCommonAddrInfo::getBriefInfo(m_queryResults[i].getHostAddrAsCommonAddr(hostAddress) , buffer2 , BD_COMMON_ADDR_INFO_SIZE);
					if(false == socketRouter->connect(addrHandle))
					{
						BD_INFO(BD_LOG_LEVEL , " Failed to initiate connection to %s. " , buffer2);
					}
					else
					{
						const bdBool insertResult = m_addrHandleStore.insert(addrHandle);
						if(insertResult)
						{
							BD_INFO(BD_LOG_LEVEL , " Initiating connection to %s.  " , buffer2);
						}
						else
						{
							BD_WARN(BD_LOG_LEVEL , " Error adding handle for %s.  " , buffer2);
						}
					}
					m_connecting = true;
				}
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
			m_createTask = BD_NULL;
			m_nextTask = FIND_GAME;
		}	
		else if (m_createTask.notNull() && (m_createTask->getStatus() == bdRemoteTask::BD_FAILED))
		{
			BD_INFO(BD_LOG_LEVEL , "Game advertisement failure  " );
			m_errorFlag = true;
		}
			
		if(	m_findTask.notNull() && (m_findTask->getStatus() == bdRemoteTask::BD_DONE))
		{
			m_numSessionInfos = m_findTask->getNumResults();
			// Note : task may be successful, but not actually find any sessions
			BD_INFO(BD_LOG_LEVEL , "Found %i sessions " , m_numSessionInfos);
			if(m_numSessionInfos > 0)
			{
				m_nextTask = QOS_INFO_PROBE;
				m_findTask = BD_NULL;
			}
			else
			{
				m_nextTask = FINISHED;
				BD_INFO(BD_LOG_LEVEL , "No instances found to connect to " );
			}
		}
		else if (m_findTask.notNull() && (m_findTask->getStatus() == bdRemoteTask::BD_FAILED))
		{
			BD_INFO(BD_LOG_LEVEL , "Game search failure  " );
			m_errorFlag = true;
		}
		
		
		
	};

	void pumpAuth()
	{
		if(m_authService->getStatus() == bdAuthService::BD_READY)
		{
			m_signedIn = true;
		}
	};

	bdBool isAuthorisedOK()
	{
		return (m_authService->getErrorCode() == BD_AUTH_NO_ERROR);
	}

	bdRemoteTaskRef m_createTask;
	bdRemoteTaskRef m_findTask;

	bdAuthService *m_authService;
	bdLobbyService *m_lobbyService;

	AppStages m_nextTask;

	// Pointer to the matchmaking client object
	bdMatchMaking *m_matchMaking;

	CustomMatchMakingInfo m_customMMInfo;
	bdSessionID	m_sessionID;

	CustomMatchMakingInfo m_queryResults[10];
	bdUInt m_numSessionInfos;

	bdLogSubscriber m_logSubscriber;

	bdSet<bdAddrHandleRef> m_addrHandleStore;

	bdBool m_connecting;
	bdStopwatch m_timer;
	bdBool m_once;
	bdBool m_once2;
	bdBool m_once3;

	bdBool m_signedIn;


};
int BD_CALL main(int argc , char* argv[])
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

