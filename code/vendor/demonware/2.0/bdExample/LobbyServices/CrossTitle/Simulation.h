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
#include <bdLobby/bdFriends/bdFriends.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdLobby/bdFriends/bdFriendInfo.h>
#include <bdLobby/bdFriends/bdFriendProposal.h>

#define MAX_NUM_SESSIONS (10u)
#define MAX_NUM_NOTIFICATIONS (10)
#define MAX_NUM_FRIENDS  (10u)

class Simulation : public bdDemo
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
			//
			CHOOSE_PLAYER,
			CREATE_COMMON_ACCOUNT,			
			CREATE_GAME_ACCOUNT,			
			AUTHENTICATE_COMMON_ACCOUNT,
			AUTHENTICATE_GAME_ACCOUNT,
			
			GAME_LOBBY_CONNECT,
			COMMON_LOBBY_CONNECT,

			ADVERTISE_SESSION,
			FIND_SESSION_BY_ID,
			
			PROPOSE_FRIENDSHIP,
			ACCEPT_PROPOSAL,
			GET_FRIENDS,
			INVITE_FRIEND,

			GAME_LOBBY_DISCONNECT,
			COMMON_LOBBY_DISCONNECT,

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

		// Choose Player

		void choosePlayerDescription(bdString& description);

		void startChoosePlayer();

		void pumpChoosePlayer();

		// Authentication service 	

		bdAuthService* getAuthService();

		const bdAuthInfo* getAuthInfo();

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

		void printSessionList(bdString& string) const;

		// Advertise session

		void advertiseSessionDescription(bdString& description);

		void startAdvertiseSession();

		void pumpAdvertiseSession();

		// Find session by id

		void findSessionByIDDescription(bdString& description);

		void startFindSessionByID();

		void pumpFindSessionByID();

		// Get messages
		void createGetMessagesDescription(bdString& description);
		void startGetMessages();
		void pumpGetMessages();

		// Get friends
		void createGetFriendsDescription(bdString& description);
		void startGetFriends();
		void pumpGetFriends();

		// Propose friendship
		void createProposeFriendshipDescription(bdString& description);
		void startProposeFriendship();
		void pumpProposeFriendship();

		// Accept friendship proposal
		void createAcceptProposalDescription(bdString& description);
		void startAcceptProposal();
		void pumpAcceptProposal();

		// Invite friend to a session
		void createInviteDescription(bdString& description);
		void startInvite();
		void pumpInvite();


		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();
		
		// Redefined methods:
		bdFriends *const GetFriends();
		bdMatchMaking *const GetMatchMaking();

		// These methods should be used to toggle between the 
		// two connections maintained
		void useGameSpecificLobbyAndAuth();
		void useCommunityWideLobbyAndAuth();
		void usePlayerOne();
		void usePlayerTwo();
		Task determineNextTask(Task currentTask);

	protected:
		
		// state variables
		bdBool m_usePlayerOne;
		bdBool m_usingCommunityConnections;

		// To facilitate code reuse we maintain pointers to the auth service and 
		// lobby service currently in use
		bdAuthService* m_currentAuthService;
		bdLobbyService* m_currentLobbyService;

		// The community-wide lobby and auth services		
		bdAuthService* m_commonAuthService;
		bdLobbyService* m_commonLobbyService;

		// The game-specific lobby and auth services		
		bdAuthService *m_authService;
		bdLobbyService* m_lobbyService;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreen;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		CustomMatchMakingInfo m_sessionInfos[MAX_NUM_SESSIONS];
		CustomMatchMakingInfo m_session;
		bdUInt m_numSessions;
		bdRemoteTaskRef m_remoteTask;

		bdFriendInfo m_friendsList[MAX_NUM_FRIENDS];
		bdFriendProposal m_friendProposalList[MAX_NUM_FRIENDS];

		PrintableList<CustomMatchMakingInfo, MAX_NUM_SESSIONS> m_foundSessions;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;
		bdString m_listString;
		bdString m_messageText;
		bdStopwatch m_messageTimer;
		bdSecurityID m_sessionSecurityID;
		bdSessionID m_sessionID;
		bdUInt m_playerNumber;
};

#endif // SIMULATION_H
