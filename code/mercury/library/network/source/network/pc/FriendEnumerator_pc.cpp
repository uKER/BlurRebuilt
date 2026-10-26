#include <StdAfx.h>
#include <Network/FriendEnumerator.h>
#include <Network/Core.h>
#include <Network/DemonwareTask.h>
#include <systemprofile/Systemprofile.h>
#include <systemprofile/SystemProfileManager.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdFriends/bdFriends.h>

namespace NetPlay
{
	// constructor
	C_FriendsEnumerator::C_FriendsEnumerator()     
	{
		m_Initialised = false;
		m_Enumerating = false;
		m_QuickEnumeration = true;
		m_NumFriendsEnumerated = 0;
		m_UserIndex = -1;
	}


	// destructor
	C_FriendsEnumerator::~C_FriendsEnumerator()
	{
		Cleanup();
	}

	
	// Create all dynamic objects.. only needs to be called once prior to first use.
	// returns true if all dynamic objects were created or false if error occurred.
	bool C_FriendsEnumerator::Initialise(void)
	{
		if (m_Initialised)
			return true;

		m_Initialised = true;
		return m_Initialised;
	}


	// private function to release dynamic resources..
	void C_FriendsEnumerator::Cleanup(void)
	{
		StopEnumeration();

		m_QuickEnumeration = true;
		m_Enumerating = false;
		m_Initialised = false;
	}


	// Try and start asynchronous friend enumeration for a specific local player
	// returns true if successful or false if enumeration failed to start
	bool C_FriendsEnumerator::StartEnumeration(const SysProfile::FullProfile& inUser, const bool quickEnumeration)
	{
		if (!m_Initialised)
			return false;

		bool ok = false;
		
		StopEnumeration();
		SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();
		if (!sysManager.m_Initialised)
			return false;

		if (sysManager.m_PrimaryPadNo == -1)
			return false; // no primary player set so can't get friends.

		if (!inUser.Active() || SysProfile::eProfileStateOnline != inUser.m_State || inUser.m_IsGuest)
			return false; // can't get friends for inactive / offline players / guests

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdFriends* friendsService =	lobbyService->getFriends();
			if (friendsService != NULL)
			{
				m_AsyncTask = friendsService->getFriends(m_Friends, MAX_FRIENDS);
					
				if (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask) == NetPlay::DemonwareRemoteTask::ePending)
				{
					m_QuickEnumeration = quickEnumeration;
					m_UserIndex = inUser.m_UserIndex;
					m_Enumerating = true;
					ok = true;
				}
				else
					StopEnumeration();
			}
		}

		return ok;
	}


	// Continue the asynchronous process of enumerating friends.
	// returns true when finished.
	bool C_FriendsEnumerator::ContinueEnumeration(bool& errorOccured)
	{
		errorOccured = true;
		if (!m_Initialised || !m_Enumerating)
			return true;

		bool failed = false;
		switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			return false;
			break;
		
		case NetPlay::DemonwareRemoteTask::eFailed:
			failed = true;
			break;

		default:
			break;
		}

		
		if (failed)
		{
			errorOccured = true;
			StopEnumeration();
		}
		else
		{
			bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
			if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
			{
				m_NumFriendsEnumerated = m_AsyncTask->getNumResults();
				NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);						
			}		
			else
				StopEnumeration();
		}
		
		return true;
	}

	
	// terminate any active friends enumerations.
	void C_FriendsEnumerator::StopEnumeration(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);	
	
		m_Enumerating = false;
		m_NumFriendsEnumerated = 0;
	}


	//extract enumerated friends
	U32 C_FriendsEnumerator::ExtractFriends(SysProfile::UserIdentifier* pOutput, const U32 maxOutput, const bool fullFriendsOnly)
	{
		if (!m_Initialised)
			return 0;

		if (m_Enumerating)
		{
			LOGWARN(g_Log, "can't extract friends while still enumerating !!\n");
			return 0;
		}

		U32 numFriendsReturned = 0;
		SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();

		assert(pOutput);
		assert(maxOutput);
		if (pOutput && maxOutput && sysManager.m_Initialised)
		{
			if (sysManager.m_PrimaryPadNo != -1)
			{
				// must be primary player since we can only have 1 active account on PSN
				if (sysManager.m_PrimaryPadNo == m_UserIndex)
				{
					for (U32 i=0; i<m_NumFriendsEnumerated; i++)
					{
						if (numFriendsReturned == maxOutput)
							return numFriendsReturned;

						// quick enumeration means we don't return offline users.
						if (m_QuickEnumeration)
						{
							if (!m_Friends[i].m_isOnline)
								continue;
						}

						pOutput[numFriendsReturned].m_IsGuest = false;
						pOutput[numFriendsReturned++].m_Uid = m_Friends[i].m_userID;
					}
				}
			}
		}

		return numFriendsReturned;
	}

	// extract more information on each friend.
	U32 C_FriendsEnumerator::ExtractFriends(FriendProfile* pOutput, const U32 maxOutput, const bool fullFriendsOnly)
	{
		if (!m_Initialised)
			return 0;

		if (m_Enumerating)
		{
			LOGWARN(g_Log, "can't extract friends while still enumerating !!\n");
			return 0;
		}

		U32 numFriendsReturned = 0;
		SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();

		assert(pOutput);
		assert(maxOutput);
		if (pOutput && maxOutput && sysManager.m_Initialised)
		{
			if (sysManager.m_PrimaryPadNo != -1)
			{
				// must be primary player since we can only have 1 active account on PSN
				if (sysManager.m_PrimaryPadNo == m_UserIndex)
				{
					// list what friend states we wist to return and in what order.
					Btl::Vector<eFriendState, Btl::VectorParameters_Static<3> > friendStates;
					friendStates.PushBack(eFriendOnline);

					// only return offline friends if we requested them.
					if (!m_QuickEnumeration)
						friendStates.PushBack(eFriendOffline);

					for (S32 i=0; i<friendStates.Size(); i++)
					{
						for (U32 z=0; z<m_NumFriendsEnumerated; z++)
						{
							if (maxOutput > numFriendsReturned)
							{
								bool ok = false;
								if (friendStates[i] == eFriendOnline && m_Friends[z].m_isOnline == true)
									ok = true;
								else if (friendStates[i] == eFriendOffline && m_Friends[z].m_isOnline == false)
									ok = true;
							
								// write out friend data ?
								if (ok)
								{
									FriendProfile fp;

									fp.m_FriendState	=	m_Friends[z].m_isOnline ? eFriendOnline : eFriendOffline;
									fp.m_CanInvite		=	m_Friends[z].m_isOnline;
									
									fp.m_Profile.m_Uid = m_Friends[z].m_userID;
									_snwprintf_s(fp.m_Profile.m_Name, lengthof(fp.m_Profile.m_Name), _TRUNCATE, L"%S", m_Friends[z].m_userName);
									fp.m_Profile.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;

									bool found = false;
									SysProfile::PcFriendsList::Iterator it;
									for (it = sysManager.m_pFriendsList->Begin(); it != sysManager.m_pFriendsList->End() && !found; ++it)
									{
										if (it->m_BaseProfile.m_Uid == m_Friends[z].m_userID)
										{
											found = true;
											if (it->m_State == friendStates[i])
											{
												fp.m_GameInvite		=	it->m_GameInvite;
												fp.m_RichPresence	=	it->m_RichPresence;
											}
										}
									}
								
									pOutput[numFriendsReturned++] = fp;
								}
							}
						}
					}
				}
			}
		}

		return numFriendsReturned;
	}
}
