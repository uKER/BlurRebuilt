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
#include <bdLobby/bdMatchMaking/bdPerformanceValue.h>


#define MAX_NUM_SESSIONS (10u)
#define MAX_PERF_VALUES (2u)
#define MAX_SESSION_INVITES (5u)

class Simulation : public bdDemo, public bdLobbyEventHandler
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);
		
		virtual void quit();

		virtual void onNotifyLeave(bdOnlineUserID userID,
									 bdSessionID secID);

		virtual void onSessionInvite(bdSessionInvite &invite);

	
	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CREATE_ACCOUNT,			
			AUTHENTICATE_ACCOUNT,
			LOBBY_CONNECT,
			ADVERTISE_SESSION,
			INVITE_TO_SESSION,
			SUBMIT_PERFORMANCE,
			GET_PERFORMANCE,
			FIND_SESSION_BY_ID,
			UPDATE_SESSION,
			FIND_SESSIONS_BY_GAME_TYPE,
			NOTIFY_JOIN,
			GET_SESSION_INVITES,
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

		// Authentication service 	

		bdAuthService* getAuthService();

		const bdAuthInfo *getAuthInfo();

		void checkAuthServiceError();

		void pumpAuthServiceTask();

		// Create account

		void createAccountDescription(bdString& description);

		void startCreateAccount();

		void pumpCreateAccount();

		// Authenticate account

		void authenticateAccountDescription(bdString& description);

		void startAuthenticateAccount();

		void pumpAuthenticateAccount();

		// Lobby service connect

		void lobbyServiceConnectDescription(bdString& description);

		void startLobbyServiceConnect();

		void pumpLobbyServiceConnect();

		// MatchMaking

		void printSessionList(bdString& string);

		// Advertise session

		void advertiseSessionDescription(bdString& description);

		void startAdvertiseSession();

		void pumpAdvertiseSession();

		// Invite to session
		void inviteToSessionDescription(bdString& description);
		void startInviteToSession();
		void pumpInviteToSession();

		// Submit performance

		void submitPerformanceDescription(bdString& description);

		void startSubmitPerformance();

		void pumpSubmitPerformance();

		// Get performance

		void getPerformanceDescription(bdString& description);

		void startGetPerformance();

		void pumpGetPerformance();


		// Find session by id

		void findSessionByIDDescription(bdString& description);

		void startFindSessionByID();

		void pumpFindSessionByID();

		// Update session

		void updateSessionDescription(bdString& description);

		void startUpdateSession();

		void pumpUpdateSession();

		// Find sessions by game type

		void findSessionsByGameTypeDescription(bdString& description);

		void startFindSessionsByGameType();

		void pumpFindSessionsByGameType();

		// Notify join

		void notifyJoinDescription(bdString& description);

		void startNotifyJoin();

		void pumpNotifyJoin();

		// Get session invites
		void getSessionInvitesDescription(bdString& description);
		void startGetSessionInvites();
		void pumpGetSessionInvites();

		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();
		
	protected:

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreen;
		bdString m_screenText;
		bdString m_latestEvent;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdAuthService *m_authService;
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		CustomMatchMakingInfo m_session;
		bdRemoteTaskRef  m_remoteTask;

		bdPerformanceValue m_performanceValues[MAX_PERF_VALUES];

		CustomMatchMakingInfo m_sessionInfos[MAX_NUM_SESSIONS];
		bdUInt m_numSessions;
		bdSessionID m_sessionID;

		PrintableList<CustomMatchMakingInfo, MAX_NUM_SESSIONS> m_foundSessions;

		bdSessionInvite m_sesionInvites [MAX_SESSION_INVITES];

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;
};

#endif // SIMULATION_H
