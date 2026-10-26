// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdLobby/bdFriends/bdFriendInfo.h>
#include <bdLobby/bdFriends/bdFriendProposal.h>
#include <bdLobby/bdFriends/bdGroupInfo.h>
#include <bdLobby/bdMessaging/bdNotification.h>

#define MAX_NUM_NOTIFICATIONS (10)
#define MAX_NUM_SESSIONS (10u)
#define MAX_NUM_FRIENDS  (10u)
#define MAX_NUM_GROUPS   (5u)
#define MAX_NUM_BLOCKED_USERS  (10u)

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

		virtual void onFriendshipProposal();
		
		virtual void onNewNotification();

		virtual void onPlayerStatusUpdate(bdOnlineUserID userID, 
							      bdNChar8 *userName,
								  bdUByte8 status);

		virtual void onRichPresenceUpdate(bdOnlineUserID userID,
										  bdNChar8 *userName,
										  void *data,
										  bdUInt dataSize,
										  bdTeamID teamID= 0);

		virtual void onSessionInvite(bdSessionInvite& invite);


	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CHOOSE_USER_ACCOUNT_1,
			CHOOSE_USER_ACCOUNT_2,
			CHOOSE_USER_ACCOUNT_3,
			AUTHENTICATE_PLAYER,
			CREATE_PLAYER,
			LOBBY_CONNECT,

			// friend specific tasks
			PROPOSE_FRIENDSHIPS,
			LIST_PROPOSALS,
			REJECT_PROPOSALS,
			GET_NOTIFICATIONS,
			GET_FRIENDSHIP_PROPOSALS,
			ACCEPT_PROPOSALS,
			GET_FRIENDS,
			WITHDRAW_PROPOSALS,
			REMOVE_FRIENDS,
		
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

		// Choose User Account 1
		void chooseUserAccount1Description(bdString& description);
		void startChooseUserAccount1();
		void pumpChooseUserAccount1();

		// Choose User Account 2
		void chooseUserAccount2Description(bdString& description);
		void startChooseUserAccount2();
		void pumpChooseUserAccount2();

		// Choose User Account 3
		void chooseUserAccount3Description(bdString& description);
		void startChooseUserAccount3();
		void pumpChooseUserAccount3();

		// Create player
		void createPlayerDescription(bdString& description);
		void startCreatePlayer();
		void pumpCreatePlayer();

		// Authenticate player
		void authenticatePlayerDescription(bdString& description);
		void startAuthenticatePlayer();
		void pumpAuthenticatePlayer();

		// Lobby service connect player
		void lobbyServiceConnectDescription(bdString& description);
		void startLobbyServiceConnect();
		void pumpLobbyServiceConnect();

		// Get notifications
		void createGetNotificationsDescription(bdString& description);
		void startGetNotifications();
		void pumpGetNotifications();

		// Get Friendship Proposals
		void createGetFriendshipProposalsDescription(bdString& description);
		void startGetFriendshipProposals();
		void pumpGetFriendshipProposals();

		// Accept friendship proposal
		void createAcceptProposalDescription(bdString& description);
		void startAcceptProposal();
		void pumpAcceptProposal();

		// Reject friendship proposal
		void createRejectProposalDescription(bdString& description);
		void startRejectProposal();
		void pumpRejectProposal();

		// Get friends
		void createGetFriendsDescription(bdString& description);
		void startGetFriends();
		void pumpGetFriends();

		// Propose friendship
		void createProposeFriendshipsDescription(bdString& description);
		void startProposeFriendships();
		void pumpProposeFriendships();

		// Display friendship proposal list
		void createListProposalsDescription(bdString& description);
		void startListProposals();
		void pumpListProposals();

		// Withdraw friendship proposal
		void createWithdrawProposalDescription(bdString& description);
		void startWithdrawProposal();
		void pumpWithdrawProposal();

		// remove friend
		void createRemoveFriendDescription(bdString& description);
		void startRemoveFriend();
		void pumpRemoveFriend();

		// Lobby disconnect
		void lobbyServiceDisconnectDescription(bdString& description);
		void startLobbyServiceDisconnect();
		void pumpLobbyServiceDisconnect();

		// Network stop
		void stopNetworkDescription(bdString& description);
		void stopNetwork();

		// Error checking
		void checkProfilesError();
		void checkFriendsError();
		
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

		bdRemoteTaskRef m_remoteTask;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		// friend specific data

		bdNotification m_notifications[MAX_NUM_NOTIFICATIONS];
		bdUInt m_numNotifications;

		bdFriendInfo m_friendsList[MAX_NUM_FRIENDS];
		bdUInt m_numFriends;

		bdFriendProposal m_friendProposalList[MAX_NUM_FRIENDS];
		bdUInt m_numFriendProposals;

		bdString m_messageText;
		bdStopwatch m_messageTimer;
		bdUInt m_playerNumber;
		bdString m_listString;

		bdString m_userName;
		bdUInt m_step;

};

#endif // SIMULATION_H
