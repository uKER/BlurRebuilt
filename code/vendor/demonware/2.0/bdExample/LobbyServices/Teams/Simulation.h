// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "TeamInfo.h"

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include<bdLobby/bdMessaging/bdMailHeader.h>
#include<bdLobby/bdMessaging/bdMailBody.h>
#include<bdLobby/bdMessaging/bdNotification.h>
#include <bdLobby/bdTeams/bdTeamProposal.h>


#define MAX_NUM_NOTIFICATIONS (10)
#define MAX_NUM_MAILS (10)

class Simulation : public bdDemo, public bdLobbyEventHandler
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();


		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual void onTeamProposal();

		virtual void onNewNotification();

		virtual void onPlayerStatusUpdate(bdOnlineUserID userID, 
							bdNChar8 *userName,
							bdUByte8 status);

		virtual void onRichPresenceUpdate(bdOnlineUserID userID,
										  bdNChar8 *userName,
                                          void *data,
                                          bdUInt dataSize,
                                          bdTeamID teamID);

		virtual bdBool step(const bdFloat32 time);
		
		virtual void quit();

	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CHOOSE_USER_ACCOUNT_1,
			CHOOSE_USER_ACCOUNT_2,
			CREATE_ACCOUNT,
			AUTHENTICATE_ACCOUNT,
			LOBBY_CONNECT,
			GET_NOTIFICATIONS,
			GET_INCOMING_PROPOSALS,
			ACCEPT_MEMBERSHIP,
			REJECT_MEMBERSHIP,
			CHECK_MEMBERSHIPS,
			CHECK_MEMBERSHIPS_USER,
			CREATE_TEAM,
			GET_INFO_TEAM,
			UPDATE_TEAM,
			GET_MEMBERS,
			PROPOSE_MEMBERSHIP,
			GET_OUTGOING_PROPOSALS,
			WITHDRAW_PROPOSAL,
			PROMOTE_MEMBER,
			DEMOTE_MEMBER,
			KICK_MEMBER,
			SEND_TEAM_MAIL,
			GET_TEAM_MAILS,
			DELETE_TEAM_MAIL,
			SEND_INSTANT_MESSAGE_TO_TEAM,
			LEAVE_TEAM,
			SET_PUBLIC_TEAM_INFO,
			GET_PUBLIC_TEAM_INFO,
			SET_PRIVATE_TEAM_INFO,
			GET_PRIVATE_TEAM_INFO,
			SET_PUBLIC_MEMBER_INFO,
			GET_PUBLIC_MEMBER_INFO,
			SET_PRIVATE_MEMBER_INFO,
			GET_PRIVATE_MEMBER_INFO,
			SET_RICH_PRESENCE,
			GET_MAIL,
			SEND_MEMBER_MAIL,
			SEND_MEMBER_INSTANT_MESSAGE,
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

		// Create profile

		void getNotificationsDescription(bdString& description);

		void startGetNotifications();

		void pumpGetNotifications();

		// Teams

		// Get team proposals
		void getIncomingTeamProposalsDescription(bdString& description);

		void startGetIncomingTeamProposals();

		void pumpGetIncomingTeamProposals();

		// Accept membership
		void acceptMembershipDescription(bdString& description);

		void startAcceptMembership();

		void pumpAcceptMembership();

		// Reject membership
		void rejectMembershipDescription(bdString& description);

		void startRejectMembership();

		void pumpRejectMembership();

	
		// Check memberships
		void checkMembershipsDescription(bdString& description);

		void startCheckMemberships();

		void pumpCheckMemberships();

		// Check memberships user
		void checkMembershipsUserDescription(bdString& description);

		void startCheckMembershipsUser();

		void pumpCheckMembershipsUser();


		// Create Team

		void createTeamDescription(bdString& description);

		void startCreateTeam();

		void pumpCreateTeam();

		// Get Team Info

		void getTeamInfoDescription(bdString& description);

		void startGetTeamInfo();

		void pumpGetTeamInfo();


		// Update Team

		void updateTeamDescription(bdString& description);

		void startUpdateTeam();

		void pumpUpdateTeam();


		// Get Members

		void getMembersDescription(bdString& description);

		void startGetMembers();

		void pumpGetMembers();


		// Propose Membership

		void proposeMembershipDescription(bdString& description);

		void startProposeMembership();

		void pumpProposeMembership();


		// Get Proposals

		void getProposalsDescription(bdString& description);

		void startGetProposals();

		void pumpGetProposals();


		// Withdraw Proposal

		void withdrawProposalDescription(bdString& description);

		void startWithdrawProposal();

		void pumpWithdrawProposal();


		// Promote Member

		void promoteMemberDescription(bdString& description);

		void startPromoteMember();

		void pumpPromoteMember();


		// Demote Member

		void demoteMemberDescription(bdString& description);

		void startDemoteMember();

		void pumpDemoteMember();


		// Kick Member

		void kickMemberDescription(bdString& description);

		void startKickMember();

		void pumpKickMember();

		// Send Team Mail

		void sendTeamMailDescription(bdString& description);

		void startSendTeamMail();
		
		void pumpSendTeamMail();

		// Get Team Mail

		void getTeamMailsDescription(bdString& description);

		void startGetTeamMails();

		void pumpGetTeamMails();

		// Delete Team Mail

		void deleteTeamMailDescription(bdString& description);

		void startDeleteTeamMail();

		void pumpDeleteTeamMail();

		// Send Instant Message To Team

		void sendInstantMessageToTeamDescription(bdString& description);

		void startSendInstantMessageToTeam();

		void pumpSendInstantMessageToTeam();

		// Leave Team

		void leaveTeamDescription(bdString& description);

		void startLeaveTeam();

		void pumpLeaveTeam();

		
		// Set Public Team Info

		void setPublicTeamInfoDescription(bdString& description);

		void startSetPublicTeamInfo();

		void pumpSetPublicTeamInfo();

		// Get Public Team Info

		void getPublicTeamInfoDescription(bdString& description);

		void startGetPublicTeamInfo();

		void pumpGetPublicTeamInfo();

		// Set Private Team Info

		void setPrivateTeamInfoDescription(bdString& description);

		void startSetPrivateTeamInfo();

		void pumpSetPrivateTeamInfo();

		// Get Private Team Info

		void getPrivateTeamInfoDescription(bdString& description);

		void startGetPrivateTeamInfo();

		void pumpGetPrivateTeamInfo();

		// Set Public Member Info

		void setPublicMemberInfoDescription(bdString& description);

		void startSetPublicMemberInfo();

		void pumpSetPublicMemberInfo();

		// Get Public Member Info

		void getPublicMemberInfoDescription(bdString& description);

		void startGetPublicMemberInfo();

		void pumpGetPublicMemberInfo();

		// Set Private Member Info

		void setPrivateMemberInfoDescription(bdString& description);

		void startSetPrivateMemberInfo();

		void pumpSetPrivateMemberInfo();

		// Get Private Member Info

		void getPrivateMemberInfoDescription(bdString& description);

		void startGetPrivateMemberInfo();

		void pumpGetPrivateMemberInfo();


		// Rich Presence
		void setRichPresenceDescription(bdString& description);
		void startSetRichPresence();
		void pumpSetRichPresence();

		// Get Mail

		void getMailDescription(bdString& description);

		void startGetMail();

		void pumpGetMail();

		// Send Member Mail

		void sendMailDescription(bdString& description);

		void startSendMail();

		void pumpSendMail();

		// Send Instant Message

		void sendInstantMessageDescription(bdString& description);

		void startSendInstantMessage();

		void pumpSendInstantMessage();

		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();

		// A generic pump method, for all channel chat tasks that do not return results.
		void pumpGenericTeamsTask();
		

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

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;
		bdLobbyService* m_lobbyService;

		bdString m_userName;

		bdRemoteTaskRef m_remoteTask;
		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		// The team that I created
		bdTeamInfo m_myOwnTeam;
		bdTeamInfo m_myOwnTeam2;
		bdTeamMember m_myTeamMembers[BD_MAX_NUM_TEAM_MEMBERS];
		bdUInt m_numTeamMembers;
		PublicTeamMemberInfo m_publicTeamMemberInfo[BD_MAX_NUM_TEAM_MEMBERS];
		PrivateTeamMemberInfo m_privateTeamMemberInfos[BD_MAX_NUM_TEAM_MEMBERS];

		// Teams that I am member of
		bdTeamInfo m_myTeams[5];
		bdUInt m_numTeamMemberships;

		PublicTeamProfile m_publicTeamProfiles[5];

		PrivateTeamProfile m_privateTeamProfile;

		
		bdNotification m_notifications[MAX_NUM_NOTIFICATIONS];
		

		PublicTeamProfile m_publicTeamInfos[2];
		PublicTeamMemberInfo  m_publicTeamMemberInfos[5];
		PrivateTeamMemberInfo m_multiplePrivateTeamMemberInfoResult[5];

		bdTeamProposal m_teamProposals[2];
		bdUInt m_numTeamProposals;

		bdString m_messageText;
		bdStopwatch m_messageTimer;



		bdMailHeader m_mailHeaders[MAX_NUM_MAILS];
		bdUInt32 m_numMails;
		bdBool m_gotMail;
		bdNChar8 m_mails[MAX_NUM_MAILS][BD_MAXIMUM_CUSTOM_MSG_SIZE];
		bdMailBody m_mailBody[MAX_NUM_MAILS];
};

#endif // SIMULATION_H
