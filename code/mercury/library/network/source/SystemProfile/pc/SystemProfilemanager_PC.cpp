#include <StdAfx.h>
#include <Network/Securekey.h>
#include <Network/Core.h>
#include <Systemprofile/SystemProfile.h>
#include <Systemprofile/GameInvite.h>
#include <Systemprofile/SystemProfileManager.h>
#include <GameLogic\RichPresenceBuilder.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>	// [Blur2PC]
#include <bdLobby/bdFriends/bdFriends.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdMessaging/bdMessaging.h>
#include <Network/DemonwareTask.h>

/// current version number for the rich presence structure.
const S32 RICH_PRESENCE_VERSION = (0xAF000000);


namespace SysProfile
{
	// static instance..
	C_Manager C_Manager::s_ProfileManager;

	// constructor
	C_Manager::C_Manager() : 
	m_Initialised(false), 
	m_RefCounter(0),
	m_pProfileData(NULL),
	m_MaxUsers(4),
	m_SystemHudActive(false),
	m_pCritSection(NULL),
	m_PrimaryPadNo(-1),
	m_ActiveServiceState(eServiceStateUnknown),
	m_LatestServiceState(eServiceStateUnknown),
	m_AgeRequirement(-1),
	m_GameInviteBeingSent(false),
	m_GameInviteBeingProcessed(false),
	m_FriendshipProposalActive(false),
	m_FriendshipRemovalActive(false),
	m_InitialPresenceSent(false),
	m_PresenceDataModified(false),
	m_DuplicateLogon(false),
	m_pRichPresence(NULL),
	m_ForcePresenceToServer(false),
	m_PresenceUpdateActive(false),
	m_pFriendsList(NULL),
	m_pFriendsPool(NULL)
	{
		m_DemonwareUser.m_Uid = 0;
		m_DemonwareUser.m_Name[0] = 0;
	}


	// destructor..
	C_Manager::~C_Manager()
	{
		Shutdown();
	}


	bool C_Manager::Initialise(const S32 ageRequirement)
	{
		if (!m_Initialised)
		{
			m_LastPresenceUpdate = Time::GetPerformanceCounter();

			m_pCritSection = new MT::Lock;
			m_pRichPresence = new NetPlay::RichPresence;
			m_pFriendsList = new PcFriendsList;
			m_pFriendsPool = new PcFriendsPool;
		
			if (m_pCritSection && m_pRichPresence && m_pFriendsList && m_pFriendsPool)
				m_Initialised = true;
			else
				Shutdown();
		}
		return m_Initialised;
	}


	void C_Manager::Shutdown(void)
	{
		if (m_Initialised)
		{
			NetPlay::DemonwareRemoteTask::Close(m_PresenceUpdateTask);
			NetPlay::DemonwareRemoteTask::Close(m_GameInviteProcessTask);
			NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
			NetPlay::DemonwareRemoteTask::Close(m_FriendshipRemovalTask);
			NetPlay::DemonwareRemoteTask::Close(m_FriendshipProposalTask);
		}
		
		if (m_pCritSection)
			delete m_pCritSection;
		m_pCritSection = NULL;

		if (m_pRichPresence)
			delete m_pRichPresence;
		m_pRichPresence = NULL;

		if (m_pFriendsList)
		{
			if (m_pFriendsPool)
			{
				while (!m_pFriendsList->Empty())
				{
					PcFriendsList::Iterator i = m_pFriendsList->Begin();
					m_pFriendsList->Erase(i); 
					m_pFriendsPool->Free(&(*i));
				}
			}	

			delete m_pFriendsList;
			m_pFriendsList = NULL;
		}

		if (m_pFriendsPool)
			delete m_pFriendsPool;
		m_pFriendsPool = NULL;

		m_PresenceUpdateActive = false;
		m_InitialPresenceSent = false;
		m_PresenceDataModified = false;
		m_ForcePresenceToServer = false;
		m_FriendshipRemovalActive = false;
		m_FriendshipProposalActive = false;
		m_GameInviteBeingSent = false;
		m_GameInviteBeingProcessed = false;
		m_PrimaryPadNo = -1;
		m_AgeRequirement = -1;
		m_ActiveServiceState = eServiceStateUnknown;
		m_LatestServiceState = eServiceStateUnknown;
		m_Initialised = false;
	}


	// Try and enter a critical section.. useful if you want to access multiple profiles at once via GetProfile() 
	void C_Manager::EnterCritical(void)
	{
		if (m_Initialised)
			m_pCritSection->Enter();
	}


	// Try an leave the critical section.. Must call after using EnterCritical() or bad stuff will happen !!
	void C_Manager::LeaveCritical(void)
	{
		if (m_Initialised)
			m_pCritSection->Exit();
	}


	void C_Manager::Poll(GameInvite& outGameInvite, Notifications& notifications, const F32 frameTime)
	{
		outGameInvite.m_Active = false;
		
		if (!m_Initialised)
			return;
		
		if (!NetPlay::C_Core::Instance().IsInitialised())
			return;

		// will be told externally if duplicate signon has occurred.
		notifications.m_DuplicateSignIn = m_DuplicateLogon;
		m_DuplicateLogon = false;

		bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		// test wether we are connected to the demonware lobby service if not then we can't be online !!
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			if (m_ActiveServiceState != eServiceStateDisconnected)
				m_LatestServiceState = eServiceStateDisconnected;
		}
		else
		{
			if (m_ActiveServiceState != eServiceStateConnected)
				m_LatestServiceState = eServiceStateConnected;
		}

		if (m_ActiveServiceState != m_LatestServiceState)
		{
			notifications.m_SignInChanged = true;
			notifications.m_ProfilesChanged = true;

			if (eServiceStateConnected == m_LatestServiceState)
			{
				notifications.m_ServiceConnectionEstablished = true;
			
				if (!m_InitialPresenceSent)
				{
					m_InitialPresenceSent = true;
					m_ForcePresenceToServer = true;
					U32 pad = m_PrimaryPadNo == -1 ? 0 : (U32)m_PrimaryPadNo;
					SetPresence(pad, (RacerNet::eRichPresence)0, false);
				}
			}
			else
			{
				notifications.m_ServiceConnectionLost = true;

				// need to resend initial presence when we sign on again.
				m_InitialPresenceSent = false;

				// presence update to server ?
				if (m_PresenceUpdateActive)
				{
					NetPlay::DemonwareRemoteTask::Close(m_PresenceUpdateTask);
					m_PresenceUpdateActive = false;
				}

				// if we were accepting / declining a game invite then close it.
				if (m_GameInviteBeingProcessed)
				{
					NetPlay::DemonwareRemoteTask::Close(m_GameInviteProcessTask);
					m_GameInviteBeingProcessed = false;
				}

				// if we were sending a game invite cance it now. as we have gone offline.
				if (m_GameInviteBeingSent)
				{
					NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
					m_GameInviteBeingSent = false;
				}

				// if we were removing a friend close this task.
				if (m_FriendshipRemovalActive)
				{
					NetPlay::DemonwareRemoteTask::Close(m_FriendshipRemovalTask);
					m_FriendshipRemovalActive = false;
				}

				// if we were adding a friend then close the task.
				if (m_FriendshipProposalActive)
				{
					NetPlay::DemonwareRemoteTask::Close(m_FriendshipProposalTask);
					m_FriendshipProposalActive = false;
				}

				// kill any friends data in our cache.
				while (!m_pFriendsList->Empty())
				{
					PcFriendsList::Iterator i = m_pFriendsList->Begin();
					m_pFriendsList->Erase(i); 
					m_pFriendsPool->Free(&(*i));
				}
			}
	
			m_ActiveServiceState = m_LatestServiceState;
		}

		// check for update presence info and send to server if required
		if (eServiceStateConnected == m_ActiveServiceState)
		{
			// track presencet task until complete.
			if (m_PresenceUpdateActive)
			{
				switch (NetPlay::DemonwareRemoteTask::CheckState(m_PresenceUpdateTask))
				{
				case NetPlay::DemonwareRemoteTask::ePending:
					break;

				case NetPlay::DemonwareRemoteTask::eFailed:
					NetPlay::DemonwareRemoteTask::Close(m_PresenceUpdateTask);
					m_PresenceUpdateActive = false;
					m_PresenceDataModified = true; // this update failed so we need to retry again after the timeout
					break;

				default:
					NetPlay::DemonwareRemoteTask::Close(m_PresenceUpdateTask);
					m_PresenceUpdateActive = false;
					break;
				}
			}

			if (m_PresenceDataModified && !m_PresenceUpdateActive)
			{
				bool updateServer = false;
				if (m_ForcePresenceToServer)
					updateServer = true;
				else
				{
					const float deltaTime = (Time::GetPerformanceCounter() - m_LastPresenceUpdate) / 1000.0f;
					if (deltaTime > 6000.0f) // time in miliseconds (6 seconds between server updates.)
						updateServer = true;
				}

				if (updateServer)
				{
					bdLobbyService* lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
					if (lobbyService)
					{
						bdFriends* friends = lobbyService->getFriends();
						if (friends)
						{
							LOGINFO(NetPlay::g_Log, "<biz Demonware Presence> Updating server %f",((F32)Time::GetPerformanceCounter())/1000.0f);

							U32 size = sizeof(NetPlay::RichPresence);
							assert(size <= BD_MAX_RICH_PRESENCE_UPDATE);

							m_PresenceUpdateTask = friends->setRichPresence(m_pRichPresence, size);
							if (NetPlay::DemonwareRemoteTask::IsBusy(m_PresenceUpdateTask))
							{
								// record when we last sent presence.
								m_PresenceUpdateActive = true;
								m_LastPresenceUpdate = Time::GetPerformanceCounter();
								m_ForcePresenceToServer = false;
								m_PresenceDataModified = false; 
							}
						}
					}
				}
			}
		}
	}
	
	// is a system profile active for a specific user.. get profile data
	bool C_Manager::GetProfile(const U32 userIndex, FullProfile& outProfileData)
	{
		
		// as default copy a blank profile..
		FullProfile blank;
		outProfileData = blank;

		if (0 == m_DemonwareUser.m_Uid)
		{
			LOGWARN(NetPlay::g_Log, "Unable to generate system profile as no user name has been supplied via SetDemonwareAuthenticatedPlayerData().");
		}

		if (m_PrimaryPadNo == -1 || m_ActiveServiceState == eServiceStateUnknown || m_DemonwareUser.m_Uid == 0)
		{
			outProfileData = blank;
		}
		else
		{
			if ((U32)m_PrimaryPadNo == userIndex)
			{
				outProfileData.m_Ref				=	++m_RefCounter;
				outProfileData.m_LastModifiedTick	=	GetTickCount();
				outProfileData.m_State				=	m_ActiveServiceState == eServiceStateConnected ? SysProfile::eProfileStateOnline : SysProfile::eProfileStateOffline;
				outProfileData.m_UserIndex			=	userIndex;
				outProfileData.m_IsGuest			=	false;

				// privileges..
				outProfileData.m_ValidPrivileges						=	true;							
				outProfileData.m_OnlineMultiplayerPrivilege				=	outProfileData.m_State == SysProfile::eProfileStateOnline ? true : false;		
				outProfileData.m_ScoreboardPrivilege					=	outProfileData.m_State == SysProfile::eProfileStateOnline ? true : false;
				outProfileData.m_ParentalLocked							=	false;
				outProfileData.m_CommunicationsPrivilege				=	true;					
				outProfileData.m_CommunicationsFriendsOnlyPrivilege		=	false;		
				outProfileData.m_ProfileViewingPrivilege				=	false;					
				outProfileData.m_ProfileViewingFriendsOnlyPrivilege		=	false;		
				outProfileData.m_ViewUserContentPrivilege				=	false;				
				outProfileData.m_ViewUserContentFriendsOnlyPrivilege	=	false;		
				outProfileData.m_PurchaseContentPrivilege				=	false;				
					
				_snwprintf_s(outProfileData.m_Name, lengthof(outProfileData.m_Name), _TRUNCATE, L"%s", m_DemonwareUser.m_Name); // [Blur2PC] names are data, not format strings
				outProfileData.m_Uid = m_DemonwareUser.m_Uid;
			}
			else
				outProfileData = blank;
		}

		return true;
	}


	// monitor a profile for changes..
	void C_Manager::MonitorProfile(FullProfile& activeProfile)
	{
		if (SysProfile::eProfileStateNotActive == activeProfile.m_State)
			return; // don't change non active profiles

		if (SysProfile::eProfileStateBlagged == activeProfile.m_State)
			return; // don't change blagged profiles

		bool dwConnection = false;
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
			dwConnection = true;

		if (dwConnection == false && activeProfile.m_State == SysProfile::eProfileStateOnline)
			GetProfile(activeProfile.m_UserIndex, activeProfile); // gone offline so re read.

		if (dwConnection == true && activeProfile.m_State == SysProfile::eProfileStateOffline)
			GetProfile(activeProfile.m_UserIndex, activeProfile); // gone online so re read.
	}


	// setup a blagged system profile
	bool C_Manager::CreateBlaggedProfile(const U32 userIndex, const WCHAR* pDefaultName, FullProfile& outProfileData)
	{
		if (userIndex >= m_MaxUsers)
		{
			assert(false);
			return false; // user index out of range.
		}

		// start with an empty profile.
		FullProfile blank;
		outProfileData = blank;

		outProfileData.m_Ref				=	++m_RefCounter;
		outProfileData.m_LastModifiedTick	=	GetTickCount();
		outProfileData.m_State				=	SysProfile::eProfileStateBlagged;
		outProfileData.m_UserIndex			=	userIndex;
		outProfileData.m_IsGuest			=	false;

		// privileges..
		outProfileData.m_ValidPrivileges						=	true;							
		outProfileData.m_OnlineMultiplayerPrivilege				=	false;		
		outProfileData.m_ScoreboardPrivilege					=	false;
		outProfileData.m_ParentalLocked							=	false;
		outProfileData.m_CommunicationsPrivilege				=	false;					
		outProfileData.m_CommunicationsFriendsOnlyPrivilege		=	false;		
		outProfileData.m_ProfileViewingPrivilege				=	false;					
		outProfileData.m_ProfileViewingFriendsOnlyPrivilege		=	false;		
		outProfileData.m_ViewUserContentPrivilege				=	true;				
		outProfileData.m_ViewUserContentFriendsOnlyPrivilege	=	false;		
		outProfileData.m_PurchaseContentPrivilege				=	false;				

		assert(SYSTEM_PROFILE_NAME_LENGTH > 0); // WTF !!

		// player name
		if (pDefaultName && wcslen(pDefaultName) > 0)
			_snwprintf_s(outProfileData.m_Name, lengthof(outProfileData.m_Name), _TRUNCATE, L"%s", pDefaultName ); // [Blur2PC] preserve literal percent characters
		else
			_snwprintf_s(outProfileData.m_Name, lengthof(outProfileData.m_Name), _TRUNCATE, L"P(%d)",userIndex+1);
		return true;
	}

	bool C_Manager::SetPresence( const U32 userIndex, RacerNet::eRichPresence presence, bool joinInProgress )	// [Blur2PC] qualified
	{
		if (!m_Initialised)
			return false;

		//	Build the RP here
		//	Convert the Rich Presence enum into a non-XLAST one if required
		bool presenceSet = Game::C_RichPresenceBuilder::Instance().BuildRichPresence(userIndex, presence);

		if(!presenceSet)
			return true;

		NetPlay::RichPresence presenceData;
		Game::C_RichPresenceBuilder::Instance().SetJoinSessionInfo(presenceData, presence, joinInProgress);

		if (!m_ForcePresenceToServer)
		{
			// check to make sure we dont spam the same presence data to the server.
			if (presenceData.m_Index == m_pRichPresence->m_Index && presenceData.m_JoinInProgressPossible == m_pRichPresence->m_JoinInProgressPossible)
				return true;
		}

		// store current presence data..
		*m_pRichPresence = presenceData;

		// set version.
		m_pRichPresence->m_Version = RICH_PRESENCE_VERSION;

		m_PresenceDataModified	=	true;

		bdLobbyService* lobbyService  = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return false;

		return true;
	}

	bool C_Manager::StartSendGameInvite(const U32 userIndex, const UserIdentifier* pRecipients, const U32 numRecipients, NetPlay::SessionSecurity& sessionData, WCHAR* inviteText)
	{
		if (!m_Initialised)
			return false;

		if (m_GameInviteBeingSent)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - can't send another invite until previous one finishes!");
			return false;
		}

		// set not active yet.
		m_GameInviteBeingSent = false;

		if (NULL == pRecipients || numRecipients == 0 || numRecipients > 10)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - recipient problem, chack numRecipients.");
			return false;
		}

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - Connection do demonWare is not in a state where we can send invite messages!.");
			return false;
		}

		Btl::Vector<bdOnlineUserID, Btl::VectorParameters_Array<10> > users;
		for (U32 i=0; i<numRecipients; i++)
			users[i] = pRecipients[i].m_Uid;

		bdSessionID sessionId;
		sessionId.m_sessionID = sessionData.m_Id;

		m_GameInviteTask = lobbyService->getMatchMaking()->inviteToSession(sessionId, &users[0], numRecipients); 
		if (NetPlay::DemonwareRemoteTask::CheckState(m_GameInviteTask) == NetPlay::DemonwareRemoteTask::ePending)
			m_GameInviteBeingSent = true;
		else
			NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);

		return m_GameInviteBeingSent;
	}

	bool C_Manager::StartSendExternalPartyInvite(const U32 userIndex, const U8 minRemote)
	{
		// not used on pc
		return false;
	}

	bool C_Manager::ContinueSendGameInvite(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (!m_GameInviteBeingSent)
			return true;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_GameInviteTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;

		case NetPlay::DemonwareRemoteTask::eFailed:
			errorOccurred = true;

			// going to test for specific error codes that we can ignore as failures.
			if (m_GameInviteTask && m_GameInviteTask.notNull())
			{
				const bdLobbyErrorCode code = m_GameInviteTask->getErrorCode();
				if (BD_SESSION_INVITE_EXISTS == code)
				{
					LOGWARN(NetPlay::g_Log, "SysProfile::ContinueSendGameInvite() - invite already sent to recipient, ignoring.");
					errorOccurred = false;
				}
			}
			break;

		default:
			errorOccurred = false;
			break;
		}

		m_GameInviteBeingSent = false;
		NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
		return true;
	}

	bool C_Manager::ProcessGameInvite(const U32 userIndex, const UserIdentifier& inviteSender, const bool accepted)
	{
		if (!m_Initialised)
			return false;

		// find friend who sent the invite and clear their friends invite data.
		PcFriendsList::Iterator it;
		for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
		{
			if (inviteSender.m_Uid == it->m_BaseProfile.m_Uid)
			{
				if (NetPlay::eFriendOnline == it->m_State && it->m_GameInvite.m_Active)
				{
					it->m_GameInvite.m_Active = false;
					return true;
				}
			}
		}

		return false;	}

	bool C_Manager::GetDemonwareUserId(const U32 userIndex, bdOnlineUserID& demonwareUserId)
	{
		demonwareUserId = 0;

		if (m_Initialised)
		{
			FullProfile fp;
			if (GetProfile(userIndex, fp))
			{
				if (SysProfile::eProfileStateOnline == fp.m_State && !fp.m_IsGuest)
				{
					demonwareUserId = fp.m_Uid;
					return true;
				}
			}
		}

		return false;
	}

	// private function sued for reading user profiles from the system.
	void C_Manager::ReadUsers(Notifications& notifications)
	{
	}

	bool C_Manager::StartAddFriend(const U32 userIndex, const char* name)
	{
		if (!m_Initialised || NULL == name || strlen(name) == 0)
			return false;

		if (m_FriendshipProposalActive)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartAddFriend() - can't add friend until previous request finishes.");
			return false;
		}

		// set not active yet.
		m_FriendshipProposalActive = false;

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartAddFriend() - Connection do demonWare is not in a state where we can add friends.");
			return false;
		}

		bdFriends *const friends = lobbyService->getFriends();
		if (NULL == friends)
			return false;

		const bdOnlineUserID otherID = bdAuthUtility::getUserID(name);
		m_FriendshipProposalTask = friends->proposeFriendship(otherID);
		if (NetPlay::DemonwareRemoteTask::CheckState(m_FriendshipProposalTask) == NetPlay::DemonwareRemoteTask::ePending)
			m_FriendshipProposalActive = true;
		else
			NetPlay::DemonwareRemoteTask::Close(m_FriendshipProposalTask);

		return m_FriendshipProposalActive;
	}

	bool C_Manager::ContinueAddFriend(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (!m_FriendshipProposalActive)
			return true;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_FriendshipProposalTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;

		case NetPlay::DemonwareRemoteTask::eFailed:
			errorOccurred = true;
			break;

		default:
			errorOccurred = false;
			break;
		}

		m_FriendshipProposalActive = false;
		NetPlay::DemonwareRemoteTask::Close(m_FriendshipProposalTask);
		return true;
	}

	bool C_Manager::StartRemoveFriend(const U32 userIndex, const UserIdentifier& toRemove)
	{
		if (!m_Initialised)
			return false;

		if (m_FriendshipRemovalActive)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartRemoveFriend() - can't remove friend until previous request finishes.");
			return false;
		}

		// set not active yet.
		m_FriendshipRemovalActive = false;

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartRemoveFriend() - Connection do demonWare is not in a state where we can remove friends.");
			return false;
		}

		bdFriends *const friends = lobbyService->getFriends();
		if (NULL == friends)
			return false;

		m_FriendshipRemovalTask = friends->removeFriend(toRemove.m_Uid);
		if (NetPlay::DemonwareRemoteTask::CheckState(m_FriendshipRemovalTask) == NetPlay::DemonwareRemoteTask::ePending)
			m_FriendshipRemovalActive = true;
		else
			NetPlay::DemonwareRemoteTask::Close(m_FriendshipRemovalTask);

		return m_FriendshipRemovalActive;
	}

	bool C_Manager::ContinueRemoveFriend(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (!m_FriendshipRemovalActive)
			return true;

		switch (NetPlay::DemonwareRemoteTask::CheckState(m_FriendshipRemovalTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;

		case NetPlay::DemonwareRemoteTask::eFailed:
			errorOccurred = true;
			break;

		default:
			errorOccurred = false;
			break;
		}

		m_FriendshipRemovalActive = false;
		NetPlay::DemonwareRemoteTask::Close(m_FriendshipRemovalTask);
		return true;
	}


	bool C_Manager::SetDemonwareAuthenticatedPlayerData(const char *pName)
	{
		if (!m_Initialised)
			return false;
	
		assert(NULL != pName);

		if (NULL != pName)
		{
			assert(strlen(pName) < SYSTEM_PROFILE_NAME_LENGTH); // [Blur2PC] 16 characters plus terminator fit

			_snwprintf_s(m_DemonwareUser.m_Name, lengthof(m_DemonwareUser.m_Name), _TRUNCATE, L"%S", pName);
			m_DemonwareUser.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;
			m_DemonwareUser.m_Uid = bdAuthUtility::getUserID(pName); 
			return true;
		}

		return false;
	}

	void  C_Manager::DemonwarePlayerStatusUpdate(bdOnlineUserID userID, bdNChar8 *userName, bdUByte8 status)
	{
		if (!m_Initialised)
			return;
		if (m_LatestServiceState == eServiceStateConnected)
		{
			if (BD_STATUS_ONLINE == status)
			{
				PcFriendsList::Iterator it;
				for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
				{
					if (it->m_BaseProfile.m_Uid == userID)
						return; // player is already being tracked.
				}

				if (m_pFriendsPool->Remaining())
				{
					C_PcFriend* pFriend = m_pFriendsPool->Allocate();
					if (pFriend)
					{
						pFriend->m_State = NetPlay::eFriendOnline;

						pFriend->m_BaseProfile.m_Uid = userID;
						_snwprintf_s(pFriend->m_BaseProfile.m_Name, lengthof(pFriend->m_BaseProfile.m_Name), _TRUNCATE, L"%S", userName);
						pFriend->m_BaseProfile.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;

						LOGINFO(NetPlay::g_Log, "<biz Friends> '%S' online.", pFriend->m_BaseProfile.Name());

						// track friend for presence and invites.
						m_pFriendsList->PushBack(*pFriend);
					}
				}
				else
				{
					LOGWARN(NetPlay::g_Log, "C_Manager::DemonwarePlayerStatusUpdate() - out of cache slots to track friends.");
				}
			}
			else if (BD_STATUS_OFFLINE == status)
			{
				PcFriendsList::Iterator it;
				for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
				{
					if (it->m_BaseProfile.m_Uid == userID)
					{
						LOGINFO(NetPlay::g_Log, "<biz Friends> '%S' removed from tracking.", it->m_BaseProfile.Name());
						m_pFriendsList->Erase(it); 
						m_pFriendsPool->Free(&(*it));
						return;
					}
				}
			}
		}
	}

	void C_Manager::DemonwareSessionInvite(bdOnlineUserID senderID, char *senderName, bdSecurityID& secID, void *attachment, U32 attachmentSize)
	{
		if (!m_Initialised)
			return;

		PcFriendsList::Iterator it;
		for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
		{
			if (senderID == it->m_BaseProfile.m_Uid)
			{
				if (NetPlay::eFriendOnline == it->m_State)
				{
					it->m_GameInvite.m_Active = true;

					for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
						it->m_GameInvite.m_Id.ab[z] = secID.ab[z];

					// security key is not valid for invites anymore so clear it out, game must query session on server to attain key and QoS.
					for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
						it->m_GameInvite.m_Key.ab[z] = 0;

					return;
				}
			}
		}

		LOGDEBUG(NetPlay::g_Log, "C_Manager::DemonwareSessionInvite() - received invite from unknown player.");
	}

	void C_Manager::DemonwareRichPresenceUpdate(bdOnlineUserID userID, char *userName, void *data, U32 dataSize)
	{
		if (!m_Initialised)
			return;

		bool validPresence = false;
		const U32 headerSize = sizeof(NetPlay::RichPresenceBase);
		const U32 requiredDataSize = sizeof(NetPlay::RichPresence);
		if (dataSize > headerSize)
		{
			NetPlay::RichPresenceBase* rpb = (reinterpret_cast<NetPlay::RichPresenceBase*> (data));
			if (RICH_PRESENCE_VERSION == rpb->m_Version)
			{
				if (dataSize == requiredDataSize)
				{
					validPresence = true;
					PcFriendsList::Iterator it;
					for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
					{
						if (userID == it->m_BaseProfile.m_Uid)
						{
							if (NetPlay::eFriendOnline == it->m_State)
							{
								it->m_RichPresence = *(reinterpret_cast<NetPlay::RichPresence*> (data));
								return;
							}
						}
					}
				}
			}
		}

		if (validPresence)
		{
			LOGDEBUG(NetPlay::g_Log, "C_Manager::DemonwareRichPresenceUpdate() - received presence data from unknown player.");
		}
	}

	bool C_Manager::IsInExternalChat(const UserIdentifier& local, const UserIdentifier& remote)
	{
		// not possible on pc...
		return false;
	}

	bool C_Manager::IsInExternalParty(const U8 minRemote)
	{
		// not possible on pc
		return false;
	}

	void C_Manager::DemonwareDuplicateLogon(void)
	{
		if (!m_Initialised)
			return;
		m_DuplicateLogon = true;
	}
}
