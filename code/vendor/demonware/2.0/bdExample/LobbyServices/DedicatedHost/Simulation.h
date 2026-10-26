// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "CustomMatchMakingInfo.h"

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>

#include <bdConnection/bdDispatch/bdDispatchInterceptor.h>
#include <bdConnection/bdConnectionListener.h>

#define MAX_NUM_SESSIONS (10u)


// Define the one message type that this application send via the DemonWare transport layer
#define DEDICATED_SERVER_CREDENTIALS_MSG	(BD_USER_MESSAGE_TYPE+1)

class Simulation : public bdDemo, 
				   public bdDispatchInterceptor, 
				   public bdConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);
		
		virtual void quit();

	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			START_HOST,
			CREATE_ACCOUNT,			
			AUTHENTICATE_CLIENT_ACCOUNT,
			AUTHENTICATE_HOST_LICENSE,
			LOBBY_CONNECT,
			ADVERTISE_SESSION,
			WAIT_FOR_TICKETS,
			WRITE_DEDICATED_STATS,
			FIND_SESSION,
			CLIENT_GETTING_HOST_TICKET,
			CLIENT_CONNECTING_TO_HOST,
			DELETE_SESSION,
			LOBBY_DISCONNECT,
			STOP_NETWORK
		};

		void startTask();

		void pumpTask();

		void displayTaskResult();

		// Network startup

		void startNetworkDescription(bdString& description);

		void startNetwork();

		void pumpStartNetwork();

		// DNS lookup

		void checkDNSDescription(bdString& description);

		void startCheckDNS();

		void pumpCheckDNS();

		// Start host
		void startHostDescription(bdString& description);

		// Authentication service 	

		bdAuthService* getAuthService();
		const bdAuthInfo *getAuthInfo();

		void pumpAuthServiceTask();

		// Create account

		void createAccountDescription(bdString& description);

		void startCreateAccount();

		void pumpCreateAccount();

		// Authenticate client account

		void authenticateClientAccountDescription(bdString& description);

		void startAuthenticateClientAccount();

		void pumpAuthenticateClientAccount();

		// Authenticate host license

		void authenticateHostLicenseDescription(bdString& description);

		void startAuthenticateHostLicense();

		void pumpAuthenticateHostLicense();

		// Lobby service connect

		void lobbyServiceConnectDescription(bdString& description);

		void startLobbyServiceConnect();

		void pumpLobbyServiceConnect();

		// MatchMaking

		void printSessionList(bdString& string) const;

		// Advertise session

		void advertiseSessionDescription(bdString& description);

		void startAdvertiseSession();

		void pumpAdvertiseSession();

		// Dedicated server waiting for client tickets
		void waitForTicketsDescription(bdString& description);

		void startWaitForTickets();

		void pumpWaitForTickets();

		// Stats

		// Dedicated server writing stats for users
		void writeDedicatedStatsDescription(bdString& description);

		void startWriteDedicatedStats();

		void pumpWriteDedicatedStats();

		// Find session by id

		void findSessionDescription(bdString& description);

		void startFindSession();

		void pumpFindSession();

		// Client obtains ticket for dedicated host

		void authenticateClientForHostDescription(bdString& description);

		void startAuthenticateClientForHost();

		void pumpAuthenticateClientForHost();

		// Client connects to dedicated host

		void connectToHostDescription(bdString& description);

		void startConnectToHost();

		void pumpConnectToHost();

		// Client sends credentials to the dedicated host

		void sendCredentialsToDedicatedServer();

		// Delete session

		void deleteSessionDescription(bdString& description);

		void startDeleteSession();

		void pumpDeleteSession();

		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();

		// Inherited from bdDispatchInterceptor
		virtual bdBool accept(const bdReceivedMessage &receivedMsg);

		// Inherited from bdConnectionListener
		virtual void onConnect(bdConnectionRef connection);

		// Inherited from bdConnectionListener
		virtual void onConnectFailed(bdConnectionRef connection);

		virtual void onDisconnect(bdConnectionRef connection);

		
	protected:

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreen;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdAuthService *m_authService;
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		CustomMatchMakingInfo m_session;
		bdSessionID m_sessionID;
		bdRemoteTaskRef m_remoteTask;

		CustomMatchMakingInfo m_foundSessions[MAX_NUM_SESSIONS];
		bdUInt m_numSessions;

		// Flag to indicate if we are running as a host
		bdBool m_runningAsHost;

		bdArray<bdAuthTicket> m_connectedUsers;
		
		bdHashMap<bdConnectionRef, bdOnlineUserID> m_userConnections;

		// The client's connection to the dedicated host
		bdConnectionRef m_connection;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		bdFastArray<bdOnlineUserID> m_usersPendingNotifyJoin;
		bdFastArray<bdOnlineUserID> m_usersPendingNotifyLeave;
		bdRemoteTaskRef m_notifyJoinTask;
		bdRemoteTaskRef m_notifyLeaveTask;
		bdStopwatch m_lastNotify;
};

#endif // SIMULATION_H
