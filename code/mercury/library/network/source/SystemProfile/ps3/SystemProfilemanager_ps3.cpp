#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Securekey.h>
#include <Network/DemonwareTask.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/GameInvite.h>
#include <GameLogic/RichPresenceBuilder.h>
#include <np.h>
#include <cell/pad.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <Network/DemonwareTask.h>
#include "NpThreadedFriendEnumerator.h"
#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"

// removing this will use use sceNpOnlineName instead os sceNpOnlineId for profile / friends names
// sceNpOnlineId is guaranteed to be unique where as sceOnlineName is not.
#define USE_SCENPONLINEID_AS_ONLINE_NAME

/// current version number for the rich presence structure.
const S32 RICH_PRESENCE_VERSION = (0xAF000000);


namespace SysProfile
{
	/// convert between utf8 (ps3 stores its strings in this format) to wide char
	bool bizUTF8ToUCS2(U8 utf8[], size_t len_utf8, WCHAR* pName, int len)
	{
		assert(len < CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE);
		assert(pName);

		pName[0] = 0;

		// convert strings to our format
		uint16_t ucs2[CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE * 2]; // converted to 16 bit ucs2 buffer
		size_t len_ucs2 = CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE;
		L10nResult L10result = UTF8stoUCS2s(utf8, &len_utf8, ucs2, &len_ucs2);

		if(L10result != ConversionOK)
		{
			LOGDEBUG(NetPlay::g_Log, "UTF8toUCS2() failed. ret = 0x%x\n", L10result);
			return false;
		}

		for(int i = 0; i < len; i ++)
		{
			pName[i] = (WCHAR)(ucs2[i]);
			if(ucs2[i] == '\0')
				break;
		}

		// force termination
		pName[len-1] = '\0';
		return true;
	}

	/// convert between utf8 (ps3 stores its strings in this format) to unsigned char
	 bool bizUTF8ToUCHAR(U8 utf8[], size_t len_utf8, char* pName, int len)
	{
		assert(len < CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE);
		assert(pName);

		pName[0] = '\0';

		// convert strings to our format
		uint16_t ucs2[CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE * 2]; // converted to 16 bit ucs2 buffer
		size_t len_ucs2 = CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE;
		L10nResult L10result = UTF8stoUCS2s(utf8, &len_utf8, ucs2, &len_ucs2);

		if(L10result != ConversionOK)
		{
			LOGDEBUG(NetPlay::g_Log, "UTF8toUCS2() failed. ret = 0x%x\n", L10result);
			return false;
		}

		for(int i = 0; i < len; i ++)
		{
			// char isn't 8 bits. replace with default underscore.
			if(ucs2[i] >= 0x7F) ucs2[i] = '_';
			pName[i] = (char)(ucs2[i]);
			if(ucs2[i] == '\0')
				break;
		}

		// force termination
		pName[len-1] = '\0';
		return true;
	}

	 // when we are offline we can't read the users sceNpId from the OS so we will attempt to read the players
	 // ip address and generate this structure. The user ideentifier is used inside the voice communicator library to identify individual
	 // players online or offline. In offline mode the user id should not really be referenced by the game code.
	 static void BlagOfflineSceNpId(FullProfile& fp, const U32 padIndex)
	 {
		 assert(padIndex <= 7);

		 // blank out previous sceNpId.
		SceNpId id;
		memset(&id, 0, sizeof(id));
		fp.m_Uid = id;

		CellNetCtlInfo ctlInfo;

		bool goRandom = true;
		if (cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &ctlInfo) == 0)
		{
			// check against invalid network address
			if (strcmp("0.0.0.0", ctlInfo.ip_address) != 0)
			{
				assert(SCE_NET_NP_ONLINEID_MAX_LENGTH >= CELL_NET_CTL_IPV4_ADDR_STR_LEN);

				//strip . from ip address to leave just octet numbers. (all 4 octets can fit in 12 bytes. octet = 255 max)
				U32 moved = 0;
				for (U32 i=0; i<CELL_NET_CTL_IPV4_ADDR_STR_LEN; i++)
				{
					if (ctlInfo.ip_address[i] != '.')
						fp.m_Uid.handle.data[moved++] = ctlInfo.ip_address[i];
				}

				goRandom = false;
			}
		}


		// don't have a valid network address so lets go random..
		if (goRandom)
			bdTrulyRandom::getInstance()->getRandomUByte8((U8*)fp.m_Uid.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH-1);
	
		// last byte in sceNpIdhandle is reserved for the pad (for handling multiple players on the same console.
		fp.m_Uid.handle.data[SCE_NET_NP_ONLINEID_MAX_LENGTH-1] = (char)padIndex;

		// by setting this reserved value this we can start to use sceNpUtilCmpNpId() on the blagged sceNpId structure
		// but i'm not sure it will be compatiable with future firmware updates. 
		// Since sceNpId structures should not be referenced in offline games then i'm leaving this commented out.
		//fp.m_Uid.reserved[0] = 1;
	 }

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
	m_pFriendsList(NULL),
	m_pFriendsPool(NULL),
	m_CableState(false),
	m_pThreadedFriendEnumerator(NULL),
	m_NumNpBasicMessagesWaiting(0),
	m_GameInviteBeingSent(false),
	m_InitialPresenceSent(false),
	m_PresenceDataModified(false),
	m_pRichPresence(NULL),
	m_ForcePresenceToServer(true)
	{
	}


	// destructor..
	C_Manager::~C_Manager()
	{
		Shutdown();
	}


	bool C_Manager::Initialise(const S32 ageRequirement)
	{
		if (m_Initialised)
			return true;

		m_LastPresenceUpdate = Time::GetPerformanceCounter();

		// create dynamic resources
		m_pCritSection = new MT::Lock;
		m_pFriendsPool = new NpFriendsPool;
		m_pFriendsList = new NpFriendsList;
		m_pThreadedFriendEnumerator = new C_NpThreadedFriendEnumerator;
		m_pRichPresence = new NetPlay::RichPresence;

		if (m_pCritSection && m_pFriendsList && m_pFriendsPool && m_pRichPresence && m_pThreadedFriendEnumerator)
		{		
			m_pThreadedFriendEnumerator->Initialise();

			m_AgeRequirement = ageRequirement;

			m_ActiveServiceState = eServiceStateDisconnected;
			m_LatestServiceState = eServiceStateDisconnected;

			S32 currentState = 0;
			if (sceNpManagerGetStatus(&currentState) == 0)
			{
				if (SCE_NP_MANAGER_STATUS_ONLINE == currentState)
				{
					LOGINFO(NetPlay::g_Log, "Connected to network Platform on initialization.");
					m_ActiveServiceState = eServiceStateConnected;
					m_LatestServiceState = eServiceStateConnected;

					m_InitialPresenceSent	= true;
					m_ForcePresenceToServer	= true;
					m_PresenceDataModified	= true;
				}
			}

			m_Initialised = true;
		}
		else
			Shutdown(); 

		return m_Initialised;
	}


	void C_Manager::Shutdown(void)
	{
		if (m_pRichPresence)
		{
			delete m_pRichPresence;
			m_pRichPresence = NULL;
		}

		if (m_GameInviteBeingSent)
		{
			NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
			m_GameInviteBeingSent = false;
		}

		if (m_pThreadedFriendEnumerator)
		{
			m_pThreadedFriendEnumerator->Shutdown();
			delete m_pThreadedFriendEnumerator;
			m_pThreadedFriendEnumerator = NULL;
		}

		if (m_pFriendsList)
		{
			if (m_pFriendsPool)
			{
				while (!m_pFriendsList->Empty())
				{
					NpFriendsList::Iterator i = m_pFriendsList->Begin();

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

		if (m_pCritSection)
			delete m_pCritSection;
		m_pCritSection = NULL;

		m_GameInviteBeingSent = false;
		m_NumNpBasicMessagesWaiting = 0;
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


	void C_Manager::CheckCableState( Notifications& notifications )
	{
		bool newCableState= NetPlay::C_Core::Instance().CableConnected();
		notifications.m_NetworkCableRemoved=m_CableState&&!newCableState;
		m_CableState=newCableState;
	}

	// poll once per frame
	void C_Manager::Poll(GameInvite& outGameInvite, Notifications& notifications, const F32 frameTime)
	{
		CheckCableState(notifications);
		//U64 startTime = Time::GetPerformanceCounter();

		// if the pad is being intercepted by the OS then we must be showing a system overlay.
#if CELL_SDK_VERSION >= 0x250000
		CellPadInfo2 info;
		if (cellPadGetInfo2(&info) == CELL_PAD_OK)
			m_SystemHudActive = (info.system_info & CELL_PAD_INFO_INTERCEPTED) ? true : false;
#else
		CellPadInfo info;
		if (cellPadGetInfo(&info) == CELL_PAD_OK)
			m_SystemHudActive = (info.info&CELL_PAD_INFO_INTERCEPTED) ? true : false;
#endif

		outGameInvite.m_Active = false;

		// have connection to network plaform service changed.
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
					SetPresence(0, (RacerNet::eRichPresence)0, false);
				}
			}
			else
			{
				notifications.m_ServiceConnectionLost = true;

				// need to resend initial presence when we sign on again.
				m_InitialPresenceSent = false;

				// kill any friends data in our cache.
				while (!m_pFriendsList->Empty())
				{
					NpFriendsList::Iterator i = m_pFriendsList->Begin();

					m_pFriendsList->Erase(i); 
					m_pFriendsPool->Free(&(*i));
				}

				// if we were sending a game invite cancel it now. as we have gone offline.
				if (m_GameInviteBeingSent)
				{
					NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
					m_GameInviteBeingSent = false;
				}
			}

			m_ActiveServiceState = m_LatestServiceState;
		}

		// check for update presence info and send to server if required
		if (eServiceStateConnected == m_LatestServiceState)
		{
			if (m_PresenceDataModified)
			{
				bool updateServer = false;
				if (m_ForcePresenceToServer)
					updateServer = true;
				else
				{
					const float deltaTime = (Time::GetPerformanceCounter() - m_LastPresenceUpdate) / 1000.0f;
					if (deltaTime > 12000.0f) // time in miliseconds (NP restricts us to max of 6 updates per minute. so we update evert 12 seconds to be safe..)
						updateServer = true;
				}

				if (updateServer)
				{
					LOGINFO(NetPlay::g_Log, "<biz NP Presence> Updating server %f",((F32)Time::GetPerformanceCounter())/1000.0f);

					U8 buffer[SCE_NP_BASIC_MAX_PRESENCE_SIZE];

					U32 size = sizeof(NetPlay::RichPresence);
					assert(size <= SCE_NP_BASIC_MAX_PRESENCE_SIZE);

					memcpy(buffer, m_pRichPresence, sizeof(NetPlay::RichPresence));

					S32 ret = sceNpBasicSetPresence(buffer, size);

					// record when we last sent presence.
					m_LastPresenceUpdate = Time::GetPerformanceCounter();
					m_ForcePresenceToServer = false;
					if (0 == ret)
						m_PresenceDataModified = false; // if function failed then by not changing this the presence will be sent again on next presence update
				}
			}
		}

		// check the NP basic message queue for friends / message notifications
		U8 eventBuffer[SCE_NP_BASIC_MAX_MESSAGE_SIZE];
		U32 msgCount = m_NumNpBasicMessagesWaiting;
		m_NumNpBasicMessagesWaiting = 0;
		for (U32 z = 0; z < msgCount; z++)
		{
			bool newFriendToTrack = false;
			int eventID = 0;
			size_t buffSize = sizeof(eventBuffer);
			SceNpUserInfo from;
			memset(eventBuffer, 0, buffSize);

			const S32 basicResult = sceNpBasicGetEvent(&eventID, &from, eventBuffer, &buffSize);

			if (SCE_NP_BASIC_ERROR_NO_EVENT == static_cast<S32>(basicResult))
				continue;

			// is the friend already in our cache ?.
			C_NpFriend* pFriend = NULL;
			NpFriendsList::Iterator it;
			for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
			{
				if (sceNpUtilCmpNpId(&from.userId, &(it->m_BaseProfile.m_Uid)) == 0)
				{
					// friend found in our cache.
					pFriend = &(*it);
					break;
				}
			}

			if (eServiceStateConnected == m_LatestServiceState)
			{
				switch (eventID)
				{
				case SCE_NP_BASIC_EVENT_FRIEND_REMOVED: 
					notifications.m_FriendsUpdated = true;
					break;

				// friend is offline
				case SCE_NP_BASIC_EVENT_OFFLINE: 
					if (pFriend)
					{
						// deallocate friend.
						NpFriendsList::Iterator it;
						bool done = false;
						for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End() && !done; ++it)
						{
							if (sceNpUtilCmpNpId(&(pFriend->m_BaseProfile.m_Uid), &(it->m_BaseProfile.m_Uid)) == 0)
							{
								LOGINFO(NetPlay::g_Log, "<biz NP Friends> '%S' removed from tracking.", pFriend->m_BaseProfile.Name());
								m_pFriendsList->Erase(it); 
								m_pFriendsPool->Free(&(*it));
								done = true;
							}
						}

						pFriend = NULL;
					}
					break; 

					// friend is online but in a different game than ours
				case SCE_NP_BASIC_EVENT_OUT_OF_CONTEXT: 
					if (NULL == pFriend && m_pFriendsPool->Remaining() > 0)
					{
						pFriend = m_pFriendsPool->Allocate();
						if (pFriend)
						{
							newFriendToTrack = true;
							m_pFriendsList->PushBack(*pFriend);
						}
					}

					// make friend online but not available to the game
					if (pFriend)
					{
						pFriend->m_State = NetPlay::eFriendOnlineButPlayingAnotherGame;

#if defined USE_SCENPONLINEID_AS_ONLINE_NAME
						if (!bizUTF8ToUCS2((uint8_t*)from.userId.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH, pFriend->m_BaseProfile.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							pFriend->m_BaseProfile.m_Name[0] = 0;
#else
						// TODO -> Check this name is valid for offline friends.. it may not be !
						if (!bizUTF8ToUCS2((uint8_t*)from.name.data, SCE_NET_NP_ONLINENAME_MAX_LENGTH, pFriend->m_BaseProfile.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							pFriend->m_BaseProfile.m_Name[0] = 0;
#endif

						pFriend->m_GameInvite.m_Active	= false;	// cross game invites are not possible.
						pFriend->m_BaseProfile.m_Uid	= from.userId;
						pFriend->m_BaseProfile.m_IsGuest= false;

						// no presence unless playing same game.
						NetPlay::RichPresence rp;
						pFriend->m_RichPresence = rp;

						if (newFriendToTrack)
						{
							LOGINFO(NetPlay::g_Log, "<biz NP Friends> '%S' online but not playing same game.", pFriend->m_BaseProfile.Name());
						}
					}
					break;

				// friend is online
				case SCE_NP_BASIC_EVENT_PRESENCE: 
					if (NULL == pFriend && m_pFriendsPool->Remaining() > 0)
					{
						pFriend = m_pFriendsPool->Allocate();
						if (pFriend)
						{
							newFriendToTrack = true;
							m_pFriendsList->PushBack(*pFriend);
						}
					}

					// make friend online 
					if (pFriend)
					{
						pFriend->m_State = NetPlay::eFriendOnline;

#if defined USE_SCENPONLINEID_AS_ONLINE_NAME
						if (!bizUTF8ToUCS2((uint8_t*)from.userId.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH, pFriend->m_BaseProfile.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							pFriend->m_BaseProfile.m_Name[0] = 0;
#else
						if (!bizUTF8ToUCS2((uint8_t*)from.name.data, SCE_NET_NP_ONLINENAME_MAX_LENGTH, pFriend->m_BaseProfile.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							pFriend->m_BaseProfile.m_Name[0] = 0;
#endif
						pFriend->m_BaseProfile.m_Uid	= from.userId;
						pFriend->m_BaseProfile.m_IsGuest= false;

						// read rich presence
						bool invalidPresence = true;
						const U32 headerSize = sizeof(NetPlay::RichPresenceBase);
						const U32 requiredDataSize = sizeof(NetPlay::RichPresence);
						if (buffSize > headerSize)
						{
							NetPlay::RichPresenceBase* rpb = (reinterpret_cast<NetPlay::RichPresenceBase*> (eventBuffer));
							if (RICH_PRESENCE_VERSION == rpb->m_Version)
							{
								if (buffSize == requiredDataSize)
								{
									pFriend->m_RichPresence = *(reinterpret_cast<NetPlay::RichPresence*> (eventBuffer));
									invalidPresence = false;
								}
							}
						}

						if (invalidPresence)
						{
							// default presence if the data was invalid.
							NetPlay::RichPresence rp;
							pFriend->m_RichPresence = rp;
						}
						
						if (newFriendToTrack)
						{
							LOGINFO(NetPlay::g_Log, "<biz NP Friends> '%S' online.", pFriend->m_BaseProfile.Name());
						}
					}
					break;
				}
			}
			else
			{
				LOGWARN(NetPlay::g_Log, "SysProfile::C_Manager::Poll() - trying to process friends events while offline !!");
			}
		}
		
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	

		if (lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
		}
		else
		{
			// no connection to the lobby so close invite message if its active.
			if (m_GameInviteBeingSent)
			{
				NetPlay::DemonwareRemoteTask::Close(m_GameInviteTask);
				m_GameInviteBeingSent = false;
			}
		}

		/*
		static U64 maxTime = 0;
		U64 totalTime = (Time::GetPerformanceCounter() - startTime);
		if (totalTime >maxTime)
			maxTime = totalTime;
		LOGINFO(NetPlay::g_Log, "sysProfile::Poll() - total time %d  : (max %d)", totalTime, maxTime);
		*/
	}

	
	// is a system profile active for a specific user.. get profile data
	bool C_Manager::GetProfile(const U32 userIndex, FullProfile& outProfileData) 
	{
		FullProfile blank;

		if (-1 == m_PrimaryPadNo)
		{
			// primary player has yet to be set to all profiles are undefined
			LOGWARN(NetPlay::g_Log, "SysProfile::C_Manager::GetProfile() - primary controller is yet to be set.");
			outProfileData = blank;
		}
		else
		{
			if (userIndex == (U32)m_PrimaryPadNo)
			{
				bool createOffline = false;
				if (eServiceStateConnected == m_LatestServiceState)
				{
					bool parentalLock = false;
					if (m_AgeRequirement > -1)
					{
						S32 isRestricted, age;
						const S32 ret = sceNpManagerGetContentRatingFlag(&isRestricted, &age);
						if(CELL_OK == ret)
						{
							if (isRestricted)
								parentalLock = (age < m_AgeRequirement) ? true : false;
						}
						else
							parentalLock = true; // lock if function failed for any reason
					}

					 // try and create an online profile, if it fails we will fallback to an offline profile.
					outProfileData.m_Ref									=	++m_RefCounter;
					outProfileData.m_State									=	SysProfile::eProfileStateOnline;
					outProfileData.m_UserIndex								=	userIndex;
					outProfileData.m_IsGuest								=	false;

					// privileges..
					outProfileData.m_ValidPrivileges						=	true;
					outProfileData.m_OnlineMultiplayerPrivilege				=	!parentalLock;
					outProfileData.m_ScoreboardPrivilege					=	!parentalLock;
					outProfileData.m_ParentalLocked							=	parentalLock;
					outProfileData.m_CommunicationsPrivilege				=	true;					
					outProfileData.m_CommunicationsFriendsOnlyPrivilege		=	false;		
					outProfileData.m_ProfileViewingPrivilege				=	false;					
					outProfileData.m_ProfileViewingFriendsOnlyPrivilege		=	false;		
					outProfileData.m_ViewUserContentPrivilege				=	true;				
					outProfileData.m_ViewUserContentFriendsOnlyPrivilege	=	false;		
					outProfileData.m_PurchaseContentPrivilege				=	false;	

#if defined USE_SCENPONLINEID_AS_ONLINE_NAME
					// getting online Id will fail if not connected to NP
					// online may not be unique, its dependent on territory of the console.
					SceNpOnlineId onlineId;
					memset(&onlineId, 0, sizeof(onlineId));
					S32 ret = sceNpManagerGetOnlineId(&onlineId);
					if (CELL_OK == ret)
					{
						if (!bizUTF8ToUCS2((uint8_t*)onlineId.data, SCE_NET_NP_ONLINEID_MAX_LENGTH, outProfileData.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							createOffline = true;
						else
						{
							if (CELL_OK != sceNpManagerGetNpId(&outProfileData.m_Uid))
								createOffline = true;
							else
							{
								// take voice communication settings from np account
								S32 isRestricted = 0; 
								if (0 == sceNpManagerGetChatRestrictionFlag(&isRestricted))
									outProfileData.m_CommunicationsPrivilege = (isRestricted == 0) ? true : false;
								else
									outProfileData.m_CommunicationsPrivilege = false;
							}
						}
					}
					else
						createOffline = true;
#else
					// getting name will fail if player is not actually online
					// online may not be uniqui, its dependent on territory of the console.
					SceNpOnlineName onlineName;
					memset(&onlineName, 0, sizeof(onlineName));
					S32 ret = sceNpManagerGetOnlineName(&onlineName);
					if (CELL_OK == ret)
					{
						if (!bizUTF8ToUCS2((uint8_t*)onlineName.data, SCE_NET_NP_ONLINENAME_MAX_LENGTH, outProfileData.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
							createOffline = true;
						else
						{
							if (CELL_OK != sceNpManagerGetNpId(&outProfileData.m_Uid))
								createOffline = true;
						}
					}
					else
						createOffline = true;
#endif
				}
				else
					createOffline = true;

				// either offline or online profile creation failed so attempt to create an offline profile.
				if (createOffline)
				{
					// reset to default.
					outProfileData = blank;

				 	// try and create an online profile, if it fails we will fallback to an offline profile.
				 	outProfileData.m_Ref									=	++m_RefCounter;
				 	outProfileData.m_State									=	SysProfile::eProfileStateOffline;
				 	outProfileData.m_UserIndex								=	userIndex;
				 	outProfileData.m_IsGuest								=	false;

				 	// privileges..
				 	outProfileData.m_ValidPrivileges						=	true;
				 	outProfileData.m_OnlineMultiplayerPrivilege				=	false;	
				 	outProfileData.m_ScoreboardPrivilege					=	false;
				 	outProfileData.m_ParentalLocked							=	false;
				 	outProfileData.m_CommunicationsPrivilege				=	true;					
				 	outProfileData.m_CommunicationsFriendsOnlyPrivilege		=	false;		
				 	outProfileData.m_ProfileViewingPrivilege				=	false;					
				 	outProfileData.m_ProfileViewingFriendsOnlyPrivilege		=	false;		
				 	outProfileData.m_ViewUserContentPrivilege				=	true;				
				 	outProfileData.m_ViewUserContentFriendsOnlyPrivilege	=	false;		
				 	outProfileData.m_PurchaseContentPrivilege				=	false;	

				 	U8 utf8[CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE * 2]; // uft8 raw buffer
				 	memset(utf8, 0, CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE * 2);
				 	S32 ret = cellSysutilGetSystemParamString(CELL_SYSUTIL_SYSTEMPARAM_ID_CURRENT_USERNAME, (char*)utf8, CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE);

				 	if (CELL_OK == ret)
				 	{
						if (bizUTF8ToUCS2(utf8, CELL_SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE, outProfileData.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
						{
							BlagOfflineSceNpId(outProfileData, userIndex);
						}
						else
						{
							LOGWARN(NetPlay::g_Log, "SysProfile::C_Manager::GetProfile() - unable to create an offline profile.");
							outProfileData = blank;
						}
						
				 	}
				 	else
				 	{
						LOGWARN(NetPlay::g_Log, "SysProfile::C_Manager::GetProfile() - unable to create an offline profile.");
						outProfileData = blank;
				 	}
				}
			}
			else
			{
				// need to create a blagged profile for this player in the game code as they dont have a system one.
				outProfileData = blank;
			}
		}
		// Sorry Dave
		for( WCHAR *p = outProfileData.m_Name; *p; p++ )
			if( *p == L'~' || *p == L'%' )
				*p = 0xffff;

		return true;
	}

	
	// monitor a profile for changes..
	void C_Manager::MonitorProfile(FullProfile& activeProfile)
	{
		if (SysProfile::eProfileStateNotActive == activeProfile.m_State)
			return; // don't change non active profiles

		if (SysProfile::eProfileStateBlagged == activeProfile.m_State)
			return; // don't change blagged profiles

		if (SysProfile::eProfileStateOffline == activeProfile.m_State && eServiceStateConnected == m_ActiveServiceState)
			GetProfile(activeProfile.m_UserIndex, activeProfile); // re read as we have gone online now

		if (SysProfile::eProfileStateOnline == activeProfile.m_State && eServiceStateDisconnected == m_ActiveServiceState)
			GetProfile(activeProfile.m_UserIndex, activeProfile); // re read as we have gone offline now
	}


	// setup a blagged system profile
	bool C_Manager::CreateBlaggedProfile(const U32 userIndex, const WCHAR* pDefaultName, FullProfile& outProfileData)
	{
		if (!m_Initialised)
			return false;

		if (userIndex >= m_MaxUsers)
		{
			return false; // user index out of range.
		}

		// start with an empty profile.
		FullProfile blank;
		outProfileData = blank;

		outProfileData.m_Ref		=	++m_RefCounter;
		outProfileData.m_State		=	SysProfile::eProfileStateBlagged;
		outProfileData.m_UserIndex	=	userIndex;
		outProfileData.m_IsGuest	=	false;

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
		if (pDefaultName)
			swprintf(outProfileData.m_Name, SYSTEM_PROFILE_NAME_LENGTH, pDefaultName, userIndex + 1 );
		else
			swprintf(outProfileData.m_Name, SYSTEM_PROFILE_NAME_LENGTH, L"P(%d)",userIndex+1);
		outProfileData.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0; // termination..
			
		return true;
	}

	// send presence to the NP servers
	bool C_Manager::SetPresence(const U32 userIndex, RacerNet::eRichPresence presence, bool joinInProgress)
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

		if (eServiceStateConnected != m_LatestServiceState)
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

		if (0 == numRecipients || NULL == pRecipients)
		{
			// problem with the passed in data
			assert(false);
			return false;
		}

		// demonware imposes this limit of a maximum of 10 remote users per message.
		if (numRecipients > 10)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - trying to invite too many recipients, maximum is %d", 10);
			return false;
		}

		if (eServiceStateConnected != m_LatestServiceState)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - Can't send invite when offline.");
			return false;
		}

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || NULL == lobbyService || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - Connection do demonWare is not in a state where we can send invite messages!.");
			return false;
		}

		// for creating the recipients demonware Id's into.
		Btl::Vector<bdOnlineUserID, Btl::VectorParameters_Array<10> > users;

		// check all recipients are in our friends list and that they are playing the same game as me, cross game invites don't work on ps3
		U32 numValidatedRecipients = 0;

		for (U32 i=0; i<numRecipients; i++)
		{
			// is this recipient a friend and are they online ?
			NpFriendsList::Iterator it;
			for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
			{
				if (sceNpUtilCmpNpId(&(pRecipients[i].m_Uid), &(it->m_BaseProfile.m_Uid)) == 0)
				{
					if (NetPlay::eFriendOnline == it->m_State)
						users[numValidatedRecipients++] =  bdAuthUtility::getUserID(pRecipients[i].m_Uid.handle.data);
						
					break;
				}
			}
		}

		if (0 == numValidatedRecipients)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - None of the invite recipients are online at this time.");
			return false;
		}

		if (numValidatedRecipients != numRecipients)
		{
			LOGWARN(NetPlay::g_Log, "SysProfile::StartSendInvite() - Not every invite recipients is online at this time. It will only be sent to online friends.");
		}

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
		// not used on ps3.
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
		NpFriendsList::Iterator it;
		for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
		{
			if (sceNpUtilCmpNpId(&(inviteSender.m_Uid), &(it->m_BaseProfile.m_Uid)) == 0)
			{
				if (NetPlay::eFriendOnline == it->m_State && it->m_GameInvite.m_Active)
				{
					it->m_GameInvite.m_Active = false;
					return true;
				}
			}
		}

		return false;
	}

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
					demonwareUserId = bdAuthUtility::getUserID(fp.m_Uid.handle.data); // use Online ID as it's unique per player on Network Platform.
					return true;
				}
			}
		}

		return false;
	}


	// private function used for reading user profiles from the system.
	void C_Manager::ReadUsers(Notifications& notifications)
	{
	}

	bool C_Manager::NetworkPlatformAccountAvailable(void) const
	{
		int np;
		if (cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_CURRENT_USER_HAS_NP_ACCOUNT, &np) != CELL_OK) 
			return false;

		return np == 1 ? true : false;
	}

	// Game should register a callback using sceNpManagerRegisterCallback() and call this function from it so we can take what data we need 
	// this callback is used for connection changes etc..
	void C_Manager::NpManagerCallback(S32 ev, S32 result, void *arg)
	{
		if (!m_Initialised)
			return;

		//LOGINFO(NetPlay::g_Log, "SysProfile::NpManagerCallback() - event Id : %d", ev);

		switch (ev)
		{
		case SCE_NP_MANAGER_STATUS_OFFLINE:
			if (m_ActiveServiceState != eServiceStateDisconnected)
			{
				m_LatestServiceState = eServiceStateDisconnected;

				// remove all friends fom the cache.
				while (!m_pFriendsList->Empty())
				{
					NpFriendsList::Iterator i = m_pFriendsList->Begin();

					m_pFriendsList->Erase(i); 
					m_pFriendsPool->Free(&(*i));
				}
			}
			break;

		case SCE_NP_MANAGER_STATUS_ONLINE:
			if (m_ActiveServiceState != eServiceStateConnected)
				m_LatestServiceState = eServiceStateConnected;
			break;
		}
	}

	// Game should register for Np basic events (friends presence etc) via its own callback function and call this function from within.
	void C_Manager::NpBasicEventHandler(S32 ev, S32 retCode, uint32_t reqId, void *arg)
	{
		if (!m_Initialised)
			return;

		// we actually retrieve the messages in the Poll() function each game frame. (hopefully)
		m_NumNpBasicMessagesWaiting += 1;
	}

	void  C_Manager::DemonwarePlayerStatusUpdate(bdOnlineUserID userID, bdNChar8 *userName, bdUByte8 status)
	{
		if (!m_Initialised)
			return;
		// not used on ps3. status stuff comes fron NP.
	}

	void C_Manager::DemonwareSessionInvite(bdOnlineUserID senderID, char *senderName, bdSecurityID& secID, void *attachment, U32 attachmentSize)
	{
		if (!m_Initialised)
			return;

		NpFriendsList::Iterator it;
		for (it = m_pFriendsList->Begin(); it != m_pFriendsList->End(); ++it)
		{
			const bdOnlineUserID friendId = bdAuthUtility::getUserID(it->m_BaseProfile.m_Uid.handle.data);
			if (friendId == senderID)
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

		// not used on ps3, presence comes from NP.
	}

	bool C_Manager::IsInExternalChat(const UserIdentifier& local, const UserIdentifier& remote)
	{
		// not possible on ps3...
		return false;
	}

	bool C_Manager::IsInExternalParty(const U8 minRemote)
	{
		// not possible on ps3.
		return false;
	}
}
