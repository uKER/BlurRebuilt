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
#include<bdLobby/bdMessaging/bdNotification.h>

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

	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CREATE_PLAYER1,
			CREATE_PLAYER2,
			AUTHENTICATE_PLAYER1,
			AUTHENTICATE_PLAYER2,
			LOBBY_CONNECT,

			// friend specific tasks
			GET_NOTIFICATIONS,
			GET_FRIENDSHIP_PROPOSALS,
			ACCEPT_PROPOSAL,
			REJECT_PROPOSAL,
			GET_FRIENDS,
			PROPOSE_FRIENDSHIP,
			LIST_PROPOSALS,
			WITHDRAW_PROPOSAL,

			SEND_MESSAGE,
			UPDATE_RICH_PRESENCE,
			CREATE_GROUP,
			LIST_GROUPS,
			ADD_FRIEND_TO_GROUP,
			DELETE_GROUP,
			REMOVE_FRIEND,
			
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

		// Create player1
		void createPlayer1Description(bdString& description);
		void startCreatePlayer1();
		void pumpCreatePlayer1();

		// Create player2
		void createPlayer2Description(bdString& description);
		void startCreatePlayer2();
		void pumpCreatePlayer2();

		// Authenticate player1
		void authenticatePlayer1Description(bdString& description);
		void startAuthenticatePlayer1();
		void pumpAuthenticatePlayer1();

		// Authenticate player2
		void authenticatePlayer2Description(bdString& description);
		void startAuthenticatePlayer2();
		void pumpAuthenticatePlayer2();

		// Lobby service connect
		void lobbyServiceConnectDescription(bdString& description);
		void startLobbyServiceConnect();
		void pumpLobbyServiceConnect();

		// Create notifications
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
		void createProposeFriendshipDescription(bdString& description);
		void startProposeFriendship();
		void pumpProposeFriendship();

		// Display friendship proposal list
		void createListProposalsDescription(bdString& description);
		void startListProposals();
		void pumpListProposals();

		// Withdraw friendship proposal
		void createWithdrawProposalDescription(bdString& description);
		void startWithdrawProposal();
		void pumpWithdrawProposal();

		// Setting rich presence, triggering messages
		void createSetRichPresenceDescription(bdString& description);
		void startSetRichPresence();
		void pumpSetRichPresence();

		// Send custom message to a friend
		void createSendCustomDescription(bdString& description);
		void startSendCustom();
		void pumpSendCustom();

		// Create friend group
		void createGroupDescription(bdString& description);
		void startGroup();
		void pumpGroup();

		// List groups
		void createListGroupsDescription(bdString& description);
		void startListGroups();
		void pumpListGroups();

		// Add friend to group
		void createAddFriendToGroupDescription(bdString& description);
		void startAddFriendToGroup();
		void pumpAddFriendToGroup();

		// delete group
		void createDeleteGroupDescription(bdString& description);
		void startDeleteGroup();
		void pumpDeleteGroup();

		// remove friend
		void createRemoveFriendDescription(bdString& description);
		void startRemoveFriend();
		void pumpRemoveFriend();

		// block user
		void createBlockUserDescription(bdString& description);
		void startBlockUser();
		void pumpBlockUser();

		// list blocked users
		void createListBlockedDescription(bdString& description);
		void startListBlocked();
		void pumpListBlocked();

		// unblock user
		void createUnblockUserDescription(bdString& description);
		void startUnblockUser();
		void pumpUnblockUser();

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
		bdFriendProposal m_friendProposalList[MAX_NUM_FRIENDS];
		bdFriendInfo m_blockedList[MAX_NUM_BLOCKED_USERS];
		bdGroupInfo  m_groupList[MAX_NUM_GROUPS];
		bdString m_messageText;
		bdStopwatch m_messageTimer;
		bdUInt m_playerNumber;
		bdString m_listString;
};

#endif // SIMULATION_H
