// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "UserInfo.h"

#include <bdExample/bdExampleCommonLobby/LiveServices.h>

//////////////////////////////////////////////////////////////////////////
//The following values need to be set as explained inline
//////////////////////////////////////////////////////////////////////////

// XBox Live LSP Service ID
#define LIVE_LSP_SERVICE_ID (0x01234ABC)
#define LIVE_TITLE_ID		(0x56789DEF)
#define LSP_PORT			(9999u)

// The 0th userID of each the 2 xenon DevKits that will be running this
// code. The code automatically logs on the the 0th user, and attempts
// to interact with the userId that is not it..
#define USERID1 (0x0009000000028105) // xenondev05
#define USERID2 (0x0009000000043043) // 10.0.0.19

// The DemonWare TitleID
#define TITLE_ID (0x140A)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <xtitleidbegin.h>
XEX_TITLE_ID(LIVE_TITLE_ID)
#include <xtitleidend.h>

//#define RUN_TEAMS_EXAMPLE_AUTOMATEDLY

Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreen(true),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR),
  m_numTeamMembers(0),
  m_numTeamMemberships(0),
  m_numTeamProposals(0),
  m_gotMail(false)

{
	bdMemset(m_mails,0, sizeof(m_mails));
}

Simulation::~Simulation()
{
}

bdBool Simulation::init(bdInt32 ,
						bdNChar8* [])
{
	bdBool ok = true;

	// start logging
	// we want to hear about all warnings and errors
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");
	m_lobbyService = new bdLobbyService();
	m_lobbyService->registerEventHandler(this);
	return ok;
}

void Simulation::onTeamProposal()
{
	bdPrintf("Received new team proposal\n");
}

void Simulation::onNewNotification()
{
	bdPrintf("Received new notification\n");
}

void Simulation::onPlayerStatusUpdate(bdOnlineUserID userID,
							      bdNChar8 *userName,
							      bdUInt nameLength,
								  bdUByte8 status)
{
	if (status == BD_STATUS_ONLINE)
	{
		bdPrintf("Player %s is now online\n", userName);

	}
	else
	{
		bdPrintf("Player %s is now offline\n", userName);
	}
}

void Simulation::onSessionInvite(bdOnlineUserID senderID,
							 bdNChar8 *senderName,
							 bdUInt nameLength,
							 bdSecurityID secID,
							 void *attachment,
							 bdUInt attachmentSize)
{

	if (attachmentSize > 0)
	{
		bdPrintf("Received session invitation: %s\n", (bdNChar8*)attachment);
	}
}

void Simulation::onRichPresenceUpdate(bdOnlineUserID userID,
									  bdNChar8 *userName,
									  void *data,
									  bdUInt dataSize,
									  bdTeamID teamID)
{
	bdPrintf("Received rich presence update from %s : %s\n", userName, data);
}

bdBool Simulation::step(const bdFloat32)
{
	const bdBool quitKey = getScreen().wasKeyPressed(BD_QUIT);

	TaskStatus oldTaskStatus = m_taskStatus;

	if(m_taskStatus == READY)
	{
		startTask();
	}

	if(m_taskStatus == PENDING || m_taskStatus == DONE || m_taskStatus == FAILED)
	{
		// pump even if the status is DONE or FAILED to read out any results
		// and close any remote task handles
		pumpTask();
	}

	if(m_taskStatus == DONE || m_taskStatus == FAILED || m_taskStatus == SKIPPED)
	{
		// incase the task fails or completes in a single loop
		if(oldTaskStatus == READY)
		{
			oldTaskStatus = m_taskStatus;
		}

		displayTaskResult();
	}

	m_changedScreen = (oldTaskStatus != m_taskStatus);

	if(m_pumpLobbyConnection)
	{
		bdBool lostConnection = true;
		if(m_lobbyService)
		{
			m_lobbyService->pump();
			const bdLobbyConnection::Status status = m_lobbyService->getStatus();
			switch(status)
			{
				case bdConnection::BD_NOT_CONNECTED:
				case bdConnection::BD_CONNECTING:
				case bdConnection::BD_DISCONNECTING:
				case bdConnection::BD_DISCONNECTED:
				{
					lostConnection = true;
				}
				break;
				case bdConnection::BD_CONNECTED:
				{
					lostConnection = false;
				}
				break;
			}
		}

		if(lostConnection)
		{
			// set up for a reconnect
			m_pumpLobbyConnection = false;
			m_task = STOP_NETWORK;
			m_taskStatus = READY;
			m_changedScreen = true;
			m_remoteTask = BD_NULL;
			m_lastTaskDescription = "Lost connection to lobby.";
		}
	}


	getScreen().printToScreen(m_screenText + m_messageText);

	if(m_changedScreen)
	{
		m_screenText = "";
	}

	return !quitKey;
}

void Simulation::quit()
{
	m_logSubscriber.removeChannel("warn");
	m_logSubscriber.removeChannel("err");
	m_logSubscriber.removeChannel("info");

	stopNetwork();

	delete m_lobbyService;

}

void Simulation::startTask()
{
	const bdBool skipKey = getScreen().wasKeyPressed(SKIP_KEY);

	if (skipKey)
	{
		m_taskStatus = SKIPPED;
	}
	else
	{
#ifdef RUN_TEAMS_EXAMPLE_AUTOMATEDLY
		const bdBool startKey = true;
#else
		const bdBool startKey = getScreen().wasKeyPressed(START_KEY);
#endif
		bdString taskDescription("Unknown task");

		switch(m_task)
		{
			case START_NETWORK:
			{
				startNetworkDescription(taskDescription);
				if(startKey)
				{
					startNetwork();
				}
			}
			break;

			case CHOOSE_USER_ACCOUNT_1:
			{
				chooseUserAccount1Description(taskDescription);
				if(startKey)
				{
					startChooseUserAccount1();
				}
			}
			break;
			case CHOOSE_USER_ACCOUNT_2:
			{
				chooseUserAccount2Description(taskDescription);
				if(startKey)
				{
					startChooseUserAccount2();
				}
			}
			break;

			case LOBBY_CONNECT:
			{
				lobbyServiceConnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceConnect();
				}
			}
			break;
			case GET_NOTIFICATIONS:
			{
				getNotificationsDescription(taskDescription);
				if(startKey)
				{
					startGetNotifications();
				}
			}
			break;
			case GET_INCOMING_PROPOSALS:
			{
				getIncomingTeamProposalsDescription(taskDescription);
				if(startKey)
				{
					startGetIncomingTeamProposals();
				}
			}
			break;
			case ACCEPT_MEMBERSHIP:
			{
				acceptMembershipDescription(taskDescription);
				if(startKey)
				{
					startAcceptMembership();
				}
			}
			break;
			case REJECT_MEMBERSHIP:
			{
				rejectMembershipDescription(taskDescription);
				if(startKey)
				{
					startRejectMembership();
				}
			}
			break;

			case CHECK_MEMBERSHIPS:
			{
				checkMembershipsDescription(taskDescription);
				if(startKey)
				{
					startCheckMemberships();
				}
			}
			break;
			case CREATE_TEAM:
			{
				createTeamDescription(taskDescription);
				if(startKey)
				{
					startCreateTeam();
				}
			}
			break;
			case UPDATE_TEAM:
			{
				updateTeamDescription(taskDescription);
				if(startKey)
				{
					startUpdateTeam();
				}
			}
			break;
			case GET_MEMBERS:
			{
				getMembersDescription(taskDescription);
				if(startKey)
				{
					startGetMembers();
				}
			}
			break;
			case PROPOSE_MEMBERSHIP:
			{
				proposeMembershipDescription(taskDescription);
				if(startKey)
				{
					startProposeMembership();
				}
			}
			break;
			case GET_OUTGOING_PROPOSALS:
			{
				getProposalsDescription(taskDescription);
				if(startKey)
				{
					startGetProposals();
				}
			}
			break;
			case WITHDRAW_PROPOSAL:
			{
				withdrawProposalDescription(taskDescription);
				if(startKey)
				{
					startWithdrawProposal();
				}
			}
			break;
			case INVITE_TO_SESSION:
			{
				inviteToSessionDescription(taskDescription);
				if(startKey)
				{
					startInviteToSession();
				}
			}
			break;
			case PROMOTE_MEMBER:
			{
				promoteMemberDescription(taskDescription);
				if(startKey)
				{
					startPromoteMember();
				}
			}
			break;
			case DEMOTE_MEMBER:
			{
				demoteMemberDescription(taskDescription);
				if(startKey)
				{
					startDemoteMember();
				}
			}
			break;
			case KICK_MEMBER:
			{
				kickMemberDescription(taskDescription);
				if(startKey)
				{
					startKickMember();
				}
			}
			break;
			case LEAVE_TEAM:
			{
				leaveTeamDescription(taskDescription);
				if(startKey)
				{
					startLeaveTeam();
				}
			}
			break;
			case SET_PUBLIC_TEAM_INFO:
			{
				setPublicTeamInfoDescription(taskDescription);
				if(startKey)
				{
					startSetPublicTeamInfo();
				}
			}
			break;
			case GET_PUBLIC_TEAM_INFO:
			{
				getPublicTeamInfoDescription(taskDescription);
				if(startKey)
				{
					startGetPublicTeamInfo();
				}
			}
			break;
			case SET_PRIVATE_TEAM_INFO:
			{
				setPrivateTeamInfoDescription(taskDescription);
				if(startKey)
				{
					startSetPrivateTeamInfo();
				}
			}
			break;
			case GET_PRIVATE_TEAM_INFO:
			{
				getPrivateTeamInfoDescription(taskDescription);
				if(startKey)
				{
					startGetPrivateTeamInfo();
				}
			}
			break;
			case SET_PUBLIC_MEMBER_INFO:
			{
				setPublicMemberInfoDescription(taskDescription);
				if(startKey)
				{
					startSetPublicMemberInfo();
				}
			}
			break;
			case GET_PUBLIC_MEMBER_INFO:
			{
				getPublicMemberInfoDescription(taskDescription);
				if(startKey)
				{
					startGetPublicMemberInfo();
				}
			}
			break;
			case SET_PRIVATE_MEMBER_INFO:
			{
				setPrivateMemberInfoDescription(taskDescription);
				if(startKey)
				{
					startSetPrivateMemberInfo();
				}
			}
			break;
			case GET_PRIVATE_MEMBER_INFO:
			{
				getPrivateMemberInfoDescription(taskDescription);
				if(startKey)
				{
					startGetPrivateMemberInfo();
				}
			}
			break;
			case GET_MAIL:
			{
				getMailDescription(taskDescription);
				if(startKey)
				{
					startGetMail();
				}
			}
			break;
			case SEND_MEMBER_MAIL:
			{
				sendMailDescription(taskDescription);
				if(startKey)
				{
					startSendMail();
				}
			}
			break;
			case SEND_MEMBER_INSTANT_MESSAGE:
			{
				sendInstantMessageDescription(taskDescription);
				if(startKey)
				{
					startSendInstantMessage();
				}
			}
			break;
			case SET_RICH_PRESENCE:
			{
				setRichPresenceDescription(taskDescription);
				if(startKey)
				{
					startSetRichPresence();
				}
			}
			break;
			case LOBBY_DISCONNECT:
			{
				lobbyServiceDisconnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceDisconnect();
				}
			}
			break;
			case STOP_NETWORK:
			{
				stopNetworkDescription(taskDescription);
				if(startKey)
				{
					stopNetwork();
				}
			}
			break;
		}

		if(m_changedScreen)
		{
			m_screenText += taskDescription;
			m_screenText += "\n" START_TEXT;
			m_screenText += "\n" SKIP_TEXT;

			if(m_lastTaskDescription.getLength())
			{
				m_screenText += "\n\n";
				m_screenText += m_lastTaskDescription;
			}
		}
	}
}

void Simulation::pumpTask()
{
	bdString taskDescription("Unknown task");

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(taskDescription);
			pumpStartNetwork();
		}
		break;

		case CHOOSE_USER_ACCOUNT_1:
		{
			chooseUserAccount1Description(taskDescription);
			pumpChooseUserAccount1();
		}
		break;
		case CHOOSE_USER_ACCOUNT_2:
		{
			chooseUserAccount2Description(taskDescription);
			pumpChooseUserAccount2();
		}
		break;

		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(taskDescription);
			pumpLobbyServiceConnect();
		}
		break;
		case GET_NOTIFICATIONS:
		{
			getNotificationsDescription(taskDescription);
			pumpGetNotifications();
		}
		break;
		case GET_INCOMING_PROPOSALS:
		{
			getIncomingTeamProposalsDescription(taskDescription);
			pumpGetIncomingTeamProposals();
		}
		break;
		case ACCEPT_MEMBERSHIP:
		{
			acceptMembershipDescription(taskDescription);
			pumpAcceptMembership();
		}
		break;
		case REJECT_MEMBERSHIP:
		{
			rejectMembershipDescription(taskDescription);
			pumpRejectMembership();
		}
		break;

		case CHECK_MEMBERSHIPS:
		{
			checkMembershipsDescription(taskDescription);
			pumpCheckMemberships();
		}
		break;
		case CREATE_TEAM:
		{
			createTeamDescription(taskDescription);
			pumpCreateTeam();
		}
		break;
		case UPDATE_TEAM:
		{
			updateTeamDescription(taskDescription);
			pumpUpdateTeam();
		}
		break;
		case GET_MEMBERS:
		{
			getMembersDescription(taskDescription);
			pumpGetMembers();
		}
		break;
		case PROPOSE_MEMBERSHIP:
		{
			proposeMembershipDescription(taskDescription);
			pumpProposeMembership();
		}
		break;
		case GET_OUTGOING_PROPOSALS:
		{
			getProposalsDescription(taskDescription);
			pumpGetProposals();
		}
		break;
		case WITHDRAW_PROPOSAL:
		{
			withdrawProposalDescription(taskDescription);
			pumpWithdrawProposal();
		}
		break;
		case INVITE_TO_SESSION:
		{

			(taskDescription);
			pumpInviteToSession();
		}
		break;
		case PROMOTE_MEMBER:
		{
			promoteMemberDescription(taskDescription);
			pumpPromoteMember();
		}
		break;
		case DEMOTE_MEMBER:
		{
			demoteMemberDescription(taskDescription);
			pumpDemoteMember();
		}
		break;
		case KICK_MEMBER:
		{
			kickMemberDescription(taskDescription);
			pumpKickMember();
		}
		break;
		case LEAVE_TEAM:
		{
			leaveTeamDescription(taskDescription);
			pumpKickMember();
		}
		break;
		case SET_PUBLIC_TEAM_INFO:
		{
			setPublicTeamInfoDescription(taskDescription);
			pumpSetPublicTeamInfo();
		}
		break;
		case GET_PUBLIC_TEAM_INFO:
		{
			getPublicTeamInfoDescription(taskDescription);
			pumpGetPublicTeamInfo();
		}
		break;
		case SET_PRIVATE_TEAM_INFO:
		{
			setPrivateTeamInfoDescription(taskDescription);
			pumpSetPrivateTeamInfo();
		}
		break;
		case GET_PRIVATE_TEAM_INFO:
		{
			getPrivateTeamInfoDescription(taskDescription);
			pumpGetPrivateTeamInfo();
		}
		break;
		case SET_PUBLIC_MEMBER_INFO:
		{
			setPublicMemberInfoDescription(taskDescription);
			pumpSetPublicMemberInfo();
		}
		break;
		case GET_PUBLIC_MEMBER_INFO:
		{
			getPublicMemberInfoDescription(taskDescription);
			pumpGetPublicMemberInfo();
		}
		break;
		case SET_PRIVATE_MEMBER_INFO:
		{
			setPrivateMemberInfoDescription(taskDescription);
			pumpSetPrivateMemberInfo();
		}
		break;
		case GET_PRIVATE_MEMBER_INFO:
		{
			getPrivateMemberInfoDescription(taskDescription);
			pumpGetPrivateMemberInfo();
		}
		break;
		case GET_MAIL:
		{
			getMailDescription(taskDescription);
			pumpGetMail();
		}
		break;
		case SEND_MEMBER_MAIL:
		{
			sendMailDescription(taskDescription);
			pumpSendMail();
		}
		break;
		case SEND_MEMBER_INSTANT_MESSAGE:
		{
			sendInstantMessageDescription(taskDescription);
			pumpSendInstantMessage();
		}
		break;
		case SET_RICH_PRESENCE:
		{
			setRichPresenceDescription(taskDescription);
			pumpSetRichPresence();
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(taskDescription);
			pumpLobbyServiceDisconnect();
		}
		break;
		case STOP_NETWORK:
		{
			// nothing to to here
		}
		break;
	}

	if(m_changedScreen)
	{
		m_screenText += taskDescription;
	}
}

void Simulation::displayTaskResult()
{
	bdString taskDescription("Unknown task result");

	Task nextTaskIfDone = m_task;
	Task nextTaskIfFailed = m_task;
	Task nextTaskIfSkipped = m_task;

	bdBool displayLobbyError = true;

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = CHOOSE_USER_ACCOUNT_1;
			nextTaskIfFailed = START_NETWORK;
			nextTaskIfSkipped = START_NETWORK;
			displayLobbyError = false;
		}
		break;

		case CHOOSE_USER_ACCOUNT_1:
		{
			chooseUserAccount1Description(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = CHOOSE_USER_ACCOUNT_1;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_2;
			displayLobbyError = false;
		}
		break;
		case CHOOSE_USER_ACCOUNT_2:
		{
			chooseUserAccount2Description(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = CHOOSE_USER_ACCOUNT_2;
			nextTaskIfSkipped = CHOOSE_USER_ACCOUNT_1;
			displayLobbyError = false;
		}
		break;

		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_NOTIFICATIONS;
			nextTaskIfFailed = LOBBY_DISCONNECT;
			nextTaskIfSkipped = GET_NOTIFICATIONS;
			displayLobbyError = false;
		}
		break;
		case GET_NOTIFICATIONS:
		{
			getNotificationsDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_INCOMING_PROPOSALS;
			nextTaskIfFailed = GET_INCOMING_PROPOSALS;
			nextTaskIfSkipped = GET_INCOMING_PROPOSALS;
		}
		break;
		case GET_INCOMING_PROPOSALS:
		{
			getIncomingTeamProposalsDescription(m_lastTaskDescription);
			nextTaskIfDone = ACCEPT_MEMBERSHIP;
			nextTaskIfFailed = GET_INCOMING_PROPOSALS;
			nextTaskIfSkipped = CHECK_MEMBERSHIPS;
		}
		break;
		case ACCEPT_MEMBERSHIP:
		{
			acceptMembershipDescription(m_lastTaskDescription);
			nextTaskIfDone = REJECT_MEMBERSHIP;
			nextTaskIfFailed = ACCEPT_MEMBERSHIP;
			nextTaskIfSkipped = REJECT_MEMBERSHIP;
		}
		break;
		case REJECT_MEMBERSHIP:
		{
			rejectMembershipDescription(m_lastTaskDescription);
			nextTaskIfDone = CHECK_MEMBERSHIPS;
			nextTaskIfFailed = REJECT_MEMBERSHIP;
			nextTaskIfSkipped = CHECK_MEMBERSHIPS;
		}
		break;
		case CHECK_MEMBERSHIPS:
		{
			checkMembershipsDescription(m_lastTaskDescription);
			nextTaskIfDone = CREATE_TEAM;
			nextTaskIfFailed = CHECK_MEMBERSHIPS;
			nextTaskIfSkipped = CREATE_TEAM;
		}
		break;
		case CREATE_TEAM:
		{
			createTeamDescription(m_lastTaskDescription);
			nextTaskIfDone = UPDATE_TEAM;
			nextTaskIfFailed = CREATE_TEAM;
			nextTaskIfSkipped = UPDATE_TEAM;
		}
		break;
		case UPDATE_TEAM:
		{
			updateTeamDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_MEMBERS;
			nextTaskIfFailed = UPDATE_TEAM;
			nextTaskIfSkipped = GET_MEMBERS;
		}
		break;
		case GET_MEMBERS:
		{
			getMembersDescription(m_lastTaskDescription);
			if(m_myTeams[0].m_teamID != BD_NULL)
			{
				nextTaskIfDone = PROPOSE_MEMBERSHIP;
				nextTaskIfFailed = GET_MEMBERS;
				nextTaskIfSkipped = PROPOSE_MEMBERSHIP;
			}
			else
			{
				nextTaskIfDone = GET_PUBLIC_TEAM_INFO;
				nextTaskIfFailed = GET_MEMBERS;
				nextTaskIfSkipped = GET_PUBLIC_TEAM_INFO;
			}
		}
		break;
		case PROPOSE_MEMBERSHIP:
		{
			proposeMembershipDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_OUTGOING_PROPOSALS;
			nextTaskIfFailed = PROPOSE_MEMBERSHIP;
			nextTaskIfSkipped = GET_OUTGOING_PROPOSALS;
		}
		break;
		case GET_OUTGOING_PROPOSALS:
		{
			getProposalsDescription(m_lastTaskDescription);
			nextTaskIfDone = WITHDRAW_PROPOSAL;
			nextTaskIfFailed = GET_OUTGOING_PROPOSALS;
			nextTaskIfSkipped = WITHDRAW_PROPOSAL;
		}
		break;
		case WITHDRAW_PROPOSAL:
		{
			withdrawProposalDescription(m_lastTaskDescription);
			nextTaskIfDone = INVITE_TO_SESSION;
			nextTaskIfFailed = INVITE_TO_SESSION;
			nextTaskIfSkipped = INVITE_TO_SESSION;
		}
		break;
		case INVITE_TO_SESSION:
		{
			inviteToSessionDescription(m_lastTaskDescription);
			if(m_myTeams[0].m_memberStatus == BD_OWNER)
			{
				nextTaskIfDone = PROMOTE_MEMBER;
				nextTaskIfFailed = PROMOTE_MEMBER;
				nextTaskIfSkipped = PROMOTE_MEMBER;
			}
			else
			{
				nextTaskIfDone = GET_PUBLIC_TEAM_INFO;
				nextTaskIfFailed = GET_PUBLIC_TEAM_INFO;
				nextTaskIfSkipped = GET_PUBLIC_TEAM_INFO;
			}
		}
		break;
		case PROMOTE_MEMBER:
		{
			promoteMemberDescription(m_lastTaskDescription);
			nextTaskIfDone = DEMOTE_MEMBER;
			nextTaskIfFailed = PROMOTE_MEMBER;
			nextTaskIfSkipped = DEMOTE_MEMBER;
		}
		break;
		case DEMOTE_MEMBER:
		{
			demoteMemberDescription(m_lastTaskDescription);
			nextTaskIfDone = KICK_MEMBER;
			nextTaskIfFailed = DEMOTE_MEMBER;
			nextTaskIfSkipped = KICK_MEMBER;
		}
		break;
		case KICK_MEMBER:
		{
			kickMemberDescription(m_lastTaskDescription);
			nextTaskIfDone = LEAVE_TEAM;
			nextTaskIfFailed = KICK_MEMBER;
			nextTaskIfSkipped = LEAVE_TEAM;
		}
		break;
		case LEAVE_TEAM:
		{
			leaveTeamDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_PUBLIC_TEAM_INFO;
			nextTaskIfFailed = LEAVE_TEAM;
			nextTaskIfSkipped = SET_PUBLIC_TEAM_INFO;
		}
		break;
		case SET_PUBLIC_TEAM_INFO:
		{
			setPublicTeamInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_PUBLIC_TEAM_INFO;
			nextTaskIfFailed = GET_PUBLIC_TEAM_INFO;
			nextTaskIfSkipped = GET_PUBLIC_TEAM_INFO;
		}
		break;
		case GET_PUBLIC_TEAM_INFO:
		{
			getPublicTeamInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_PRIVATE_TEAM_INFO;
			nextTaskIfFailed = SET_PRIVATE_TEAM_INFO;
			nextTaskIfSkipped = SET_PRIVATE_TEAM_INFO;
		}
		break;
		case SET_PRIVATE_TEAM_INFO:
		{
			setPrivateTeamInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_PRIVATE_TEAM_INFO;
			nextTaskIfFailed = GET_PRIVATE_TEAM_INFO;
			nextTaskIfSkipped = GET_PRIVATE_TEAM_INFO;
		}
		break;
		case GET_PRIVATE_TEAM_INFO:
		{
			getPrivateTeamInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_PUBLIC_MEMBER_INFO;
			nextTaskIfFailed = SET_PUBLIC_MEMBER_INFO;
			nextTaskIfSkipped = SET_PUBLIC_MEMBER_INFO;
		}
		break;
		case SET_PUBLIC_MEMBER_INFO:
		{
			setPublicMemberInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_PUBLIC_MEMBER_INFO;
			nextTaskIfFailed = GET_PUBLIC_MEMBER_INFO;
			nextTaskIfSkipped = GET_PUBLIC_MEMBER_INFO;
		}
		break;
		case GET_PUBLIC_MEMBER_INFO:
		{
			getPublicMemberInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_PRIVATE_MEMBER_INFO;
			nextTaskIfFailed = SET_PRIVATE_MEMBER_INFO;
			nextTaskIfSkipped = SET_PRIVATE_MEMBER_INFO;
		}
		break;
		case SET_PRIVATE_MEMBER_INFO:
		{
			setPrivateMemberInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_PRIVATE_MEMBER_INFO;
			nextTaskIfFailed = GET_PRIVATE_MEMBER_INFO;
			nextTaskIfSkipped = GET_PRIVATE_MEMBER_INFO;
		}
		break;
		case GET_PRIVATE_MEMBER_INFO:
		{
			getPrivateMemberInfoDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_MAIL;
			nextTaskIfFailed = GET_PRIVATE_MEMBER_INFO;
			nextTaskIfSkipped = GET_MAIL;
		}
		break;
		case GET_MAIL:
		{
			getMailDescription(m_lastTaskDescription);
			nextTaskIfDone = SEND_MEMBER_MAIL;
			nextTaskIfFailed = GET_MAIL;
			nextTaskIfSkipped = SEND_MEMBER_MAIL;
		}
		break;

		case SEND_MEMBER_MAIL:
		{
			sendMailDescription(m_lastTaskDescription);
			nextTaskIfDone = SEND_MEMBER_INSTANT_MESSAGE;
			nextTaskIfFailed = SEND_MEMBER_MAIL;
			nextTaskIfSkipped = SEND_MEMBER_INSTANT_MESSAGE;
		}
		break;
		case SEND_MEMBER_INSTANT_MESSAGE:
		{
			sendInstantMessageDescription(m_lastTaskDescription);
			nextTaskIfDone = SET_RICH_PRESENCE;
			nextTaskIfFailed = LOBBY_DISCONNECT;
			nextTaskIfSkipped = SET_RICH_PRESENCE;
		}
		break;
		case SET_RICH_PRESENCE:
		{
			setRichPresenceDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = SET_RICH_PRESENCE;
			nextTaskIfSkipped = LOBBY_DISCONNECT;
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(m_lastTaskDescription);
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = LOBBY_DISCONNECT;
			nextTaskIfSkipped = STOP_NETWORK;
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = START_NETWORK;
		}
		break;
	}

	// if task uses sets a lobby error code and has failed
	// display the error description
	if(displayLobbyError && (m_taskStatus == FAILED))
	{
		bdNChar8 buf[32];
		LobbyErrorCodeToString(m_lobbyErrorCode, buf, sizeof(buf));
		m_lastTaskDescription += "\nError: ";
		m_lastTaskDescription += buf;
		m_lobbyErrorCode = BD_NO_ERROR;
	}

	switch(m_taskStatus)
	{
		case READY:
		case PENDING:
		{
			BD_WARN(BD_LOG_LEVEL, "Unexpected task status.");
		}
		break;
		case DONE:
		{
			m_task = nextTaskIfDone;
		}
		break;
		case FAILED:
		{
			m_task = nextTaskIfFailed;
		}
		break;
		case SKIPPED:
		{
			m_task = nextTaskIfSkipped;
		}
		break;
	}



	m_taskStatus = READY;
}

void Simulation::startNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start Network",				// READY
											"Starting Network",				// PENDING
											"Started Network",				// DONE
											"Starting Network Failed",		// FAILED
											"Starting Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startNetwork()
{
	// fail safe
	m_taskStatus = FAILED;

	bdNetImpl *const net = bdNet::getInstance();
	if(net)
	{
		bdNetStartParams params;

		// platform specific startup parameters
		bdPlatformParams platformParams;
		//TODO: check the return value here
		bdExamplePlatformInit::init(params, &platformParams);

		if(net->start(params))
		{
			m_taskStatus = PENDING;
		}
	}
}


void Simulation::pumpStartNetwork()
{
	// fail safe
	m_taskStatus = FAILED;

	bdNetImpl *const net = bdNet::getInstance();
	if(net)
	{
		net->pump();

		const bdNetImpl::bdNetStatus networkStatus = net->getStatus();
		switch(networkStatus)
		{
			case bdNetImpl::BD_NET_BIND_FAILED:
			case bdNetImpl::BD_NET_ONLINE_FAILED:
			case bdNetImpl::BD_NET_INIT_FAILED:
			case bdNetImpl::BD_NET_STOPPED:
			case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
			{
				// error conditions
				m_taskStatus = FAILED;
			}
			break;
			case bdNetImpl::BD_NET_PENDING:
			{
				// wait
				m_taskStatus = PENDING;
			}
			break;
			case bdNetImpl::BD_NET_DONE:
			{
				// complete
				m_taskStatus = DONE;
			}
		}
	}
}



const bdAuthInfo *Simulation::getAuthInfo()
{
	/*
	if(m_authService != BD_NULL)
	{
		return m_authService->getAuthInfo();
	}
*/
	return BD_NULL;
}





void Simulation::startChooseUserAccount1()
{
	// fail safe
	m_taskStatus = DONE;
}

void Simulation::pumpChooseUserAccount1()
{
}

void Simulation::chooseUserAccount1Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose User Account 1",			// READY
											"Choosing User Account 1",			// PENDING
											"Chose User Account 1",				// DONE
											"Choose User Account 1 Failed",		// FAILED
											"Choose User Account 1 Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startChooseUserAccount2()
{
	// fail safe
	m_taskStatus = DONE;
}

void Simulation::pumpChooseUserAccount2()
{
}

void Simulation::chooseUserAccount2Description(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Choose User Account 2",			// READY
											"Choosing User Account 2",			// PENDING
											"Chose User Account 2",			// DONE
											"Choose User Account 2 Failed",		// FAILED
											"Choose User Account 2 Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}




void Simulation::lobbyServiceConnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Connect to Lobby Service",				// READY
											"Connecting to Lobby Service",			// PENDING
											"Connected to Lobby Service",			// DONE
											"Connect to Lobby Service Failed",		// FAILED
											"Connect to Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		LiveServices m_liveServices;
		m_liveServices.signOn(0);
		bdBool m_errorFlag = false;
		while (	(false == m_liveServices.isSignedOn()) &&
			(false == m_errorFlag))
		{
			m_errorFlag = !(m_liveServices.pump());
			bdPlatformTiming::sleep(50);
		}

		if(false == m_errorFlag)
		{
			bdAddr ca;
			bdSecurityID id;
			bdSecurityKey key;
			if (!m_liveServices.resolveTitleAddr(LIVE_LSP_SERVICE_ID, LSP_PORT, ca, id, key))
			{
				BD_WARN("test", "Failed to resolve LSG address. Failing now.");
				m_errorFlag = true;
			}
			else
			{
				// setup local userID
				m_userID = m_liveServices.getXUserId();
				bdNChar8 buffer [BD_MAX_USERNAME_LENGTH];
				m_liveServices.getXUserName(buffer);

				BD_INFO("test", "SG Lite address found.");

				// Prepare auth info
				bdAuthTicket ticket;
				ticket.m_userID = m_liveServices.getXUserId();

				ticket.m_titleID = TITLE_ID;
				m_liveServices.getXUserName(ticket.m_username);

				bdAuthInfo authInfo;
				authInfo.m_titleID = TITLE_ID;
				ticket.serialize(authInfo.m_data);

				// Register SG Lite's key
				//bdNet::getInstance()->getKeyMap()->put(id, key);

				BD_INFO("test", "Connecting to matchmaking...");
				m_errorFlag = !(m_lobbyService->connect(ca, id, &authInfo));

				bdPrintf ("Connecting to MatchMaking+ sandbox...");
			}
		}
		if(false == m_errorFlag)
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	m_pumpLobbyConnection = false;

	if(m_lobbyService)
	{
		m_lobbyService->pump();
		const bdLobbyConnection::Status connectionStatus = m_lobbyService->getStatus();
		switch(connectionStatus)
		{
			case bdConnection::BD_NOT_CONNECTED:
			case bdConnection::BD_DISCONNECTING:
			case bdConnection::BD_DISCONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_CONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = DONE;
				m_pumpLobbyConnection = true;
			}
			break;
		}
	}
}

void Simulation::getNotificationsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Notifications",			// READY
											"Getting Notifications",			// PENDING
											"Got Notifications",			// DONE
											"Get Notifications Failed",	// FAILED
											"Get Notifications Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetNotifications()
{
	// fail safe
	m_taskStatus = FAILED;

	bdMessaging *const messaging = m_lobbyService->getMessaging();
	if(messaging)
	{
		m_remoteTask = messaging->getNotifications(m_notifications, 0, MAX_NUM_NOTIFICATIONS, false);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetNotifications()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		bdUInt numNotifications = m_remoteTask->getNumResults();
		bdUInt totalNumNotifications = m_remoteTask->getTotalNumResults();
		for (bdUInt i=0; i<numNotifications; i++)
		{
			switch (m_notifications[i].m_type)
			{
				case BD_TEAMMEMBER_PROMOTED_TO_ADMIN:
				{
					bdPrintf("%s was promoted in team %s \n", m_notifications[i].m_sourceUserName, m_notifications[i].m_teamName);
				}
				break;
				case BD_TEAMMEMBER_KICKED:
				{
					bdPrintf("%s was kicked out of the the team %s \n", m_notifications[i].m_sourceUserName, m_notifications[i].m_teamName);
				}
				break;
				case BD_TEAMMEMBER_LEFT:
				{
					bdPrintf("%s left the team team \n", m_notifications[i].m_sourceUserName);
				}
				break;
			}
		}
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getIncomingTeamProposalsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Team Proposals",				// READY
											"Getting Team Proposals",		// PENDING
											"Got Team Proposals",			// DONE
											"Get Team Proposals Failed",		// FAILED
											"Get Team Proposals Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetIncomingTeamProposals()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		//m_remoteTask = messaging->getMessages(0, 0, 20, false, true);
		m_remoteTask = teams->getIncomingProposals(m_teamProposals, 2);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetIncomingTeamProposals()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_numTeamProposals = m_remoteTask->getNumResults();
		if (m_numTeamProposals > 0)
		{
			bdPrintf("Got %u new team proposals:\n");
			for (bdUInt i=0; i< m_numTeamProposals; i++)
			{
				bdPrintf("Team Name: %s \n", m_teamProposals[i].m_teamName);
			}
		}
		else
		{
			bdPrintf("No new team proposals\n");
		}
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::acceptMembershipDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Accept Membership (skip to goto reject)",				// READY
											"Accepting Membership",			// PENDING
											"Accepted Membership",			// DONE
											"Accept Membership Failed",		// FAILED
											"Accept Membership Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAcceptMembership()
{
	// fail safe
	m_taskStatus = FAILED;

	if (m_numTeamProposals > 0)
	{

		bdTeams *teams = m_lobbyService->getTeams();
		m_remoteTask = teams->acceptMembership(m_teamProposals[0].m_teamID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}

}

void Simulation::pumpAcceptMembership()
{
	pumpGenericTeamsTask();

}

void Simulation::rejectMembershipDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Reject Membership",				// READY
											"Rejecting Membership",			// PENDING
											"Rejected Membership",			// DONE
											"Reject Membership Failed",		// FAILED
											"Reject Membership Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startRejectMembership()
{
	// fail safe
	m_taskStatus = FAILED;
	if (m_numTeamProposals > 0)
	{

		bdTeams *teams = m_lobbyService->getTeams();
		m_remoteTask = teams->rejectMembership(m_teamProposals[0].m_teamID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpRejectMembership()
{
	pumpGenericTeamsTask();
}


void Simulation::checkMembershipsDescription(bdString& description)
{
	bdString membershipsStr = "Got Memberships:\n";
	bdUInt i;
	for(i = 0; i < m_numTeamMemberships; i++)
	{
		membershipsStr += m_myTeams[i].m_teamName;
		if(m_myTeams[i].m_memberStatus == BD_ADMIN)
		{
			membershipsStr += "[ADMIN]\n";
		}
		else if(m_myTeams[i].m_memberStatus == BD_OWNER)
		{
			membershipsStr += "[OWNER]\n";
		}
		else
		{
			membershipsStr += "\n";
		}
	}
	const bdNChar8 *const descriptions[] = {"Get Memberships",				// READY
											"Getting Memberships",			// PENDING
											membershipsStr.getBuffer(),		// DONE
											"Get Memberships Failed",		// FAILED
											"Get Memberships Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startCheckMemberships()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		m_remoteTask = teams->getMemberships(m_myTeams, 2);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpCheckMemberships()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		bdPrintf("My Team Memberships: \n");
		m_numTeamMemberships = m_remoteTask->getNumResults();
		for (bdUInt i=0; i < m_numTeamMemberships; i++)
		{
			bdPrintf("     Name %s,		TeamID: " BD_UINT_64_FORMAT "\n",m_myTeams[i].m_teamName, m_myTeams[i].m_teamID);
			// If we are owner of a team, copy the team details so that we can send
			// membership proposals
			if (m_myTeams[i].m_memberStatus == BD_OWNER)
			{
				m_myOwnTeam = m_myTeams[i];

			}
		}
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::startCreateTeam()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		bdString teamName = m_userName;
		teamName += "\'s Team";
		m_remoteTask = teams->createTeam(teamName.getBuffer(), &m_myOwnTeam);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpCreateTeam()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		bdPrintf("My Team: Name %s, TeamID: " BD_UINT_64_FORMAT "\n",m_myOwnTeam.m_teamName, m_myOwnTeam.m_teamID);
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::createTeamDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Create Team",			// READY
											"Creating Team",		// PENDING
											"Created Team",			// DONE
											"Create Team Failed",	// FAILED
											"Create Team Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startUpdateTeam()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		bdString teamName = m_userName;
		teamName += "\'s New Team";
		m_remoteTask = teams->updateTeamName( m_myOwnTeam.m_teamID, teamName);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpUpdateTeam()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::updateTeamDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Update Team",			// READY
											"Updating Team",		// PENDING
											"Updated Team",			// DONE
											"Update Team Failed",	// FAILED
											"Update Team Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetMembers()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myTeams[0].m_teamID != BD_NULL)
	{
		m_remoteTask = teams->getMembers(m_myTeams[0].m_teamID, m_myTeamMembers, BD_MAX_NUM_TEAM_MEMBERS);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetMembers()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_numTeamMembers = m_remoteTask->getNumResults();
	}

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getMembersDescription(bdString& description)
{
	bdString resultStr = "Got Members:\n";
	bdUInt i;
	for(i = 0; i < m_numTeamMembers; i++)
	{
		resultStr += m_myTeamMembers[i].m_userName;
		if(m_myTeamMembers[i].m_status == BD_ORDINARY)
		{
			resultStr += "[ORDINARY MEMBER]";
		}
		else if(m_myTeamMembers[i].m_status == BD_ADMIN)
		{
			resultStr += "[ADMIN]";
		}
		else if(m_myTeamMembers[i].m_status == BD_OWNER)
		{
			resultStr += "[OWNER]";
		}
		if(m_myTeamMembers[i].m_isOnline)
		{
			resultStr += " - online\n";
		}
		else
		{
			resultStr += " - offline\n";
		}
	}

	const bdNChar8 *const descriptions[] = {"Get Members",			// READY
											"Getting Members",		// PENDING
											resultStr.getBuffer(),	// DONE
											"Get Members Failed",	// FAILED
											"Get Members Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startProposeMembership()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		bdOnlineUserID otherUserID = 0;
		if(USERID1 == m_userID)
		{
			otherUserID = USERID2;
		}
		else if(USERID2 == m_userID)
		{
			otherUserID = USERID1;
		}
		else
		{
			BD_ASSERT(false, "User logged on whose ID ain't hardcoded ...");
		}
		bdNChar8 * invitationMessage = "Please join my team.";
		m_remoteTask = teams->proposeMembership(m_myOwnTeam.m_teamID, otherUserID, invitationMessage, bdStrnlen(invitationMessage, 128) + 1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpProposeMembership()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::proposeMembershipDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Propose Membership",			// READY
											"Proposing Membership",			// PENDING
											"Proposed Membership",			// DONE
											"Propose Membership Failed",		// FAILED
											"Propose Membership Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetProposals()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams)
	{
		m_remoteTask = teams->getOutgoingProposals(m_teamProposals, 2);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetProposals()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getProposalsDescription(bdString& description)
{
	bdString getProposalsStr = "Got outstanding proposals:\n";
	bdUInt i;
	for(i = 0; i < m_numTeamProposals; i++)
	{
		getProposalsStr += m_teamProposals[i].m_userName;
		getProposalsStr += " to join ";
		getProposalsStr += m_teamProposals[i].m_teamName;
		getProposalsStr += "\n";
	}
	const bdNChar8 *const descriptions[] = {"Get Proposals",			// READY
											"Getting Proposals",		// PENDING
											getProposalsStr.getBuffer(),// DONE
											"Get Proposals Failed",		// FAILED
											"Get Proposals Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startWithdrawProposal()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_numTeamProposals > 0)
	{
		bdOnlineUserID userID = m_teamProposals[0].m_userID;
		bdTeamID teamID = m_teamProposals[0].m_teamID;
		m_remoteTask = teams->withdrawProposal(userID, teamID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpWithdrawProposal()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::withdrawProposalDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Withdraw Proposal",			// READY
											"Withdrawing Proposal",			// PENDING
											"Withdrew Proposal",			// DONE
											"Withdraw Proposal Failed",		// FAILED
											"Withdraw Proposal Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startInviteToSession()
{
	// fail safe
	m_taskStatus = FAILED;

	// Find a userID that is not the local user to invite
	bdOnlineUserID invitedUsers[20];
	bdUInt i;
	bdUInt numInvitedUsers=0;
	for(i = 0; i < m_numTeamMembers; i++)
	{
		if(m_myTeamMembers[i].m_userID != m_userID)
		{
			invitedUsers[numInvitedUsers] = m_myTeamMembers[i].m_userID;
			numInvitedUsers++;
		}
	}

	bdMatchMaking *matchmaking = m_lobbyService->getMatchMaking();
	bdNChar8 *invitationMsg = "Join my session!";
	if(numInvitedUsers > 0)
	{
		// We leave this uninitialized here. In a real implementation, this value
		// would be been retrieved from the bdMatchMaking API.
		bdSessionID sessionID;
		m_remoteTask = matchmaking->inviteToSession(sessionID, invitedUsers, numInvitedUsers, invitationMsg, bdStrnlen(invitationMsg, 100)+1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpInviteToSession()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::inviteToSessionDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Invite To Session",			// READY
											"Inviting To Session",			// PENDING
											"Invited To Session",			// DONE
											"Invite To Session Failed",		// FAILED
											"Invite To Session Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startPromoteMember()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();

	// Check for team ownership, and that there is a member who can be promoted.
	bdBool amOwner = m_myOwnTeam.m_memberStatus == BD_OWNER;
	bdOnlineUserID userID = BD_NULL;
	bdUInt i;
	for(i = 0; i < m_numTeamMembers && userID == BD_NULL; i++)
	{
		if(m_myTeamMembers[i].m_status == BD_ORDINARY)
		{
			userID = m_myTeamMembers[i].m_userID;
		}
	}

	if(teams && amOwner && userID != BD_NULL)
	{
		m_remoteTask = teams->promoteMember( m_myOwnTeam.m_teamID, userID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpPromoteMember()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::promoteMemberDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Promote Member",			// READY
											"Promoting Member",			// PENDING
											"Promoted Member",			// DONE
											"Promote Member Failed",	// FAILED
											"Promote Member Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startDemoteMember()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();

	// Check for team ownership, and that there is a member who can be demoted.
	bdBool amOwner = m_myOwnTeam.m_memberStatus == BD_OWNER;
	bdOnlineUserID userID = BD_NULL;
	bdUInt i;
	for(i = 0; i < m_numTeamMembers && userID == BD_NULL; i++)
	{
		if(m_myTeamMembers[i].m_userID != m_userID)
		{
			userID = m_myTeamMembers[i].m_userID;
		}
	}
	if(teams && amOwner && userID != BD_NULL)
	{
		m_remoteTask = teams->demoteMember(m_myOwnTeam.m_teamID, userID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpDemoteMember()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::demoteMemberDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Demote Member",			// READY
											"Demoting Member",			// PENDING
											"Demoted Member",			// DONE
											"Demote Member Failed",		// FAILED
											"Demote Member Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startKickMember()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();

	// Check for team ownership, and that there is a member who can be demoted.
	bdBool amAdmin = m_myOwnTeam.m_memberStatus != BD_ORDINARY;
	bdOnlineUserID userID = BD_NULL;
	bdUInt i;
	for(i = 0; i < m_numTeamMembers && userID == BD_NULL; i++)
	{
		if(m_myTeamMembers[i].m_status == BD_ORDINARY
		   && m_myTeamMembers[i].m_userID != m_userID)
		{
			userID = m_myTeamMembers[i].m_userID;
		}
	}
	if(teams && amAdmin && userID != BD_NULL)
	{
		m_remoteTask = teams->kickMember(m_myOwnTeam.m_teamID, userID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpKickMember()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::kickMemberDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Kick Member",			// READY
											"Kicking Member",		// PENDING
											"Kicked Member",		// DONE
											"Kick Member Failed",	// FAILED
											"Kick Member Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLeaveTeam()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();

	// Check for team ownership, and that there is a member who can be demoted.
	if (m_numTeamMemberships > 0)
	{
		bdTeamID teamID = m_myTeams[0].m_teamID;
		m_remoteTask = teams->leaveTeam(teamID);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpLeaveTeam()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::leaveTeamDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Leave Team",			// READY
											"Leaving Team",		// PENDING
											"Left Team",		// DONE
											"Leave Team Failed",	// FAILED
											"LeaveTeam Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startSetPublicTeamInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_memberStatus == BD_OWNER)
	{
		PublicTeamProfile teamProfile;
		teamProfile.m_int1 = 0;
		teamProfile.m_int2 = 0;
		teamProfile.m_int3 = 0;
		teamProfile.m_int4 = 0;
		teamProfile.m_int5 = 0;
		teamProfile.m_int6 = 0;
		bdStrlcpy(teamProfile.m_string1, "Motto: live and let die", 24);
		bdStrlcpy(teamProfile.m_string2, "key words", 10);
		m_remoteTask = teams->setPublicProfile(m_myOwnTeam.m_teamID, &teamProfile);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetPublicTeamInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setPublicTeamInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Public Team Info",				// READY
											"Setting Public Team Info",			// PENDING
											"Set Public Team Info Successful",	// DONE
											"Set Public Team Info Failed",		// FAILED
											"Set Public Team Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetPublicTeamInfo()
{
	// fail safe
	m_taskStatus = FAILED;


	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != 0)
	{
		bdTeamID teamIDs[5];
		for (bdUInt i=0; i< m_numTeamMemberships; i++)
		{
			teamIDs[i] = m_myTeams[i].m_teamID;
		}
		m_remoteTask = teams->getPublicProfiles(teamIDs, m_publicTeamProfiles, m_numTeamMemberships );
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetPublicTeamInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getPublicTeamInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Public Team Info",			// READY
		"Getting Public Team Info",		// PENDING
		"Got Public Team Info",			// DONE
		"Get Public Team Info Failed",	// FAILED
		"Get Public Team Info Skipped"};// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		for(bdUInt i=0; i<m_numTeamMemberships; i++){
			description += "\n";
			description += m_publicTeamProfiles[i].m_string1;
			description += "\n";
			description += m_publicTeamProfiles[i].m_string2;
		}
	}
}

void Simulation::startSetPrivateTeamInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_memberStatus == BD_OWNER)
	{
		PrivateTeamProfile teamProfile;
		bdStrlcpy(teamProfile.m_string1, "Use strategy 16b from clan website.", 100);
		bdStrlcpy(teamProfile.m_string2, "Enemy base C.", 100);
		m_remoteTask = teams->setPrivateProfile(m_myOwnTeam.m_teamID, &teamProfile);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetPrivateTeamInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setPrivateTeamInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Private Team Info",			// READY
											"Setting Private Team Info",			// PENDING
											"Set Private Team Info Successful",			// DONE
											"Set Private Team Info Failed",		// FAILED
											"Set Private Team Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetPrivateTeamInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != BD_NULL)
	{
		m_remoteTask = teams->getPrivateProfile(m_myOwnTeam.m_teamID, &m_privateTeamProfile);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetPrivateTeamInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getPrivateTeamInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Private Team Info",			// READY
											"Getting Private Team Info",			// PENDING
											"Got Private Team Info",			// DONE
											"Get Private Team Info Failed",		// FAILED
											"Get Private Team Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		description += "\n";
		description += m_privateTeamProfile.m_string1;
		description += "\n";
		description += m_privateTeamProfile.m_string2;
	}
}

void Simulation::startSetPublicMemberInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != BD_NULL)
	{
		PublicTeamMemberInfo memberInfo;
		memberInfo.m_int1 = 0;
		memberInfo.m_int2 = 0;
		memberInfo.m_int3 = 0;
		bdStrlcpy(memberInfo.m_string1, "Scout", 100);
		bdStrlcpy(memberInfo.m_string2, "PingIsland", 100);
		m_remoteTask = teams->setPublicMemberProfile(m_myOwnTeam.m_teamID, &memberInfo);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetPublicMemberInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setPublicMemberInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Public Member Info",			// READY
											"Setting Public Member Info",			// PENDING
											"Set Public Member Info Succeed",		// DONE
											"Set Public Member Info Failed",		// FAILED
											"Set Public Member Info Skipped"};		// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetPublicMemberInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_numTeamMembers > 0)
	{
		m_remoteTask = teams->getPublicMemberProfiles(m_myOwnTeam.m_teamID, m_publicTeamMemberInfo, m_numTeamMembers);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetPublicMemberInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getPublicMemberInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Public Member Info",			// READY
											"Getting Public Member Info",		// PENDING
											"Got Public Member Info",			// DONE
											"Get Public Member Info Failed",	// FAILED
											"Get Public Member Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		for(bdUInt i=0; i<m_numTeamMembers; i++){
			description += "\n";
			description += m_publicTeamMemberInfo[i].m_string1;
			description += "\n";
			description += m_publicTeamMemberInfo[i].m_string2;
		}
	}
}

void Simulation::startSetPrivateMemberInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != BD_NULL)
	{
		PrivateTeamMemberInfo memberInfo;
		memberInfo.m_int1 = 3;
		bdStrlcpy(memberInfo.m_string1, "Sahara", 100);
		bdStrlcpy(memberInfo.m_string2, "Defender", 100);

		m_remoteTask = teams->setPrivateMemberProfile(m_myOwnTeam.m_teamID, &memberInfo);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetPrivateMemberInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setPrivateMemberInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Private Member Info",			// READY
											"Setting Private Member Info",		// PENDING
											"Set Private Member Info Successful",// DONE
											"Set Private Member Info Failed",	// FAILED
											"Set Private Member Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetPrivateMemberInfo()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != BD_NULL)
	{
		bdOnlineUserID userID[2];
		userID[0] = m_userID;
		userID[1] = 0;

		m_remoteTask = teams->getPrivateMemberProfiles( m_myOwnTeam.m_teamID, m_privateTeamMemberInfos, m_numTeamMembers);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpGetPrivateMemberInfo()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getPrivateMemberInfoDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Private Member Info",			// READY
											"Getting Private Member Info",		// PENDING
											"Got Private Member Info",			// DONE
											"Get Private Member Info Failed",	// FAILED
											"Get Private Member Info Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == DONE)
	{
		for(bdUInt i=0; i<m_numTeamMembers; i++){
			description += "\n";
			description += m_privateTeamMemberInfos[i].m_string1;
			description += "\n";
			description += m_privateTeamMemberInfos[i].m_string2;
		}
	}
}

void Simulation::startSetRichPresence()
{
	// fail safe
	m_taskStatus = FAILED;

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && m_myOwnTeam.m_teamID != 0)
	{
		bdNChar8 *msg= "Attack from the left";
		m_remoteTask = teams->setRichPresence(m_myOwnTeam.m_teamID, msg, 21);

		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}
}

void Simulation::pumpSetRichPresence()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::setRichPresenceDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Set Rich Presence",			// READY
		"Setting Rich Presence",			// PENDING
		"Set Rich Presence Successful",			// DONE
		"Set Rich Presence Failed",		// FAILED
		"Set Rich Presence Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetMail()
{
	// fail safe
	m_taskStatus = FAILED;


	bdMessaging *messaging = m_lobbyService->getMessaging();

	m_remoteTask = messaging->getMailHeaders(m_mailHeaders, 0, 10);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

}

void Simulation::pumpGetMail()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE && !m_gotMail)
	{
		m_numMails = m_remoteTask->getNumResults();
		bdPrintf("Got %u new mails\n", m_numMails);
		// Start a new task to retrieve an actual mail
		if (m_numMails > 0)
		{
			bdUInt64 mailIDs [MAX_NUM_MAILS];
			bdUInt actualNumMails = 0;
			for (actualNumMails = 0; actualNumMails< m_numMails && actualNumMails<MAX_NUM_MAILS; actualNumMails++)
			{
				mailIDs[actualNumMails] = m_mailHeaders[actualNumMails].m_mailID;
				m_mailBody[actualNumMails].m_mailBody = m_mails[actualNumMails];
				m_mailBody[actualNumMails].m_size = sizeof(m_mails[actualNumMails]);
			}
			bdMessaging *messaging = m_lobbyService->getMessaging();
			const bdBool deleteMails = false;	
			m_remoteTask = messaging->getMails(mailIDs, actualNumMails, m_mailBody, deleteMails);
			m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
		}
		m_gotMail = true;
	}
	else if (m_taskStatus == DONE && m_gotMail)
	{
		bdPrintf("%s", m_mails[0]);
		m_gotMail = false;
	}


	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::getMailDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Get Mail",			// READY
											"Getting Mail",		// PENDING
											"Got Mail",			// DONE
											"Get Mail Failed",	// FAILED
											"Get Mail Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startSendMail()
{
	// fail safe
	m_taskStatus = FAILED;

	// Find a userID that is not the local user to send a message to.
	bdOnlineUserID userID = BD_NULL;
	bdUInt i;
	for(i = 0; i < m_numTeamMembers && userID == BD_NULL; i++)
	{
		if(m_myTeamMembers[i].m_userID != m_userID)
		{
			userID = m_myTeamMembers[i].m_userID;
		}
	}

	bdMessaging *messaging = m_lobbyService->getMessaging();

	if(userID)
	{
		bdNChar8 *message = "All your base are belonging to us";
		m_remoteTask = messaging->sendMail(message, bdStrnlen(message,100)+1 , &userID, 1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}

}

void Simulation::pumpSendMail()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::sendMailDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Send Member Message",			// READY
											"Sending Member Message",		// PENDING
											"Sent Member Message",			// DONE
											"Send Member Message Failed",	// FAILED
											"Send Member Message Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startSendInstantMessage()
{
	// fail safe
	m_taskStatus = FAILED;

	bdOnlineUserID userID = BD_NULL;
	bdUInt i;
	for(i = 0; i < m_numTeamMembers && userID == BD_NULL; i++)
	{
		if(m_myTeamMembers[i].m_userID != m_userID)
		{
			userID = m_myTeamMembers[i].m_userID;
		}
	}

	bdTeams *teams = m_lobbyService->getTeams();
	if(teams && userID)
	{
		bdNChar8 *msg = "Hey dude";

		m_remoteTask = teams->sendInstantMessage(userID, msg, bdStrnlen(msg, 100)+1);
		m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	}

}

void Simulation::pumpSendInstantMessage()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}

void Simulation::sendInstantMessageDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Send Instant Message",			// READY
											"Sending Instant Message",		// PENDING
											"Sent Instant Message",			// DONE
											"Send Instant Message Failed",	// FAILED
											"Send Instant Message Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::lobbyServiceDisconnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Disconnect from Lobby Service",			// READY
											"Disconnecting from Lobby Service",			// PENDING
											"Disconnected from Lobby Service",			// DONE
											"Disconnect from Lobby Service Failed",		// FAILED
											"Disconnect from Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		m_lobbyService->disconnect();
		m_taskStatus = PENDING;
	}
}

void Simulation::pumpLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		const bdLobbyConnection::Status status = m_lobbyService->getStatus();
		switch(status)
		{
			case bdConnection::BD_DISCONNECTED:
			case bdConnection::BD_NOT_CONNECTED:
			{
				m_taskStatus = DONE;
			}
			break;
			case bdConnection::BD_CONNECTING:
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_DISCONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
		}
	}
}

void Simulation::stopNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Stop Network",					// READY
											"Stopping Network",				// PENDING
											"Stopped Network",				// DONE
											"Stopping Network Failed",		// FAILED
											"Stopping Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::stopNetwork()
{
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();
	if(net->getStatus() == bdNetImpl::BD_NET_STOPPED)
	{
		m_taskStatus = DONE;
	}
	else
	{
		m_taskStatus = FAILED;
	}
}



void Simulation::pumpGenericTeamsTask()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == FAILED)
	{
		m_lobbyErrorCode = m_remoteTask->getErrorCode();
	}
}




