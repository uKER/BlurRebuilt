#include <StdAfx.h>
#include <core/threads.h>
#include <Network/FriendEnumerator.h>
#include <systemprofile/Systemprofile.h>
#include <systemprofile/SystemProfileManager.h>
#include "../../SystemProfile/PS3/NpThreadedFriendEnumerator.h"


// TODO -> Implement enumeration of offline friends.

namespace SysProfile
{
	extern bool bizUTF8ToUCS2(U8 utf8[], size_t len_utf8, WCHAR* pName, int len);
}

namespace NetPlay
{

	// constructor
	C_FriendsEnumerator::C_FriendsEnumerator() :m_Initialised(false), m_Enumerating(false), m_QuickEnumeration(true), m_NumFriendsEnumerated(0), 
												m_UserIndex(-1), m_RequestStarted(false)
	{
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
		m_Initialised = false;
	}


	// Try and start asynchronous friend enumeration for a specific local player
	// returns true if successful or false if enumeration failed to start
	bool C_FriendsEnumerator::StartEnumeration(const SysProfile::FullProfile& inUser, const bool quickEnumeration)
	{
		if (!m_Initialised)
			return false;

		SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();
		if (!sysManager.m_Initialised)
			return false;
		
		if (sysManager.m_PrimaryPadNo == -1)
			return false; // no primary player set so can't get friends.

		if (!inUser.Active() || SysProfile::eProfileStateOnline != inUser.m_State || inUser.m_IsGuest)
			return false; // cent get friends for inactive / offline players / guests

		if (!quickEnumeration)
		{
			// if another enumerator is active then this one can't start since we only have 1 thread.
			const SysProfile::C_NpThreadedFriendEnumerator::eStatus status = sysManager.m_pThreadedFriendEnumerator->Status();
			if (SysProfile::C_NpThreadedFriendEnumerator::eStatusIdle != status)
			{
				LOGWARN(g_Log, "C_FriendsEnumerator::StartEnumeration() - unable to start because its already in use.");
				return false;
			}
		}

		StopEnumeration();

		// store this for later reference
		m_UserIndex = inUser.m_UserIndex;

		m_RequestStarted = false;
		m_Enumerating = true;
		m_QuickEnumeration = quickEnumeration;

		return true;
	}

	bool C_FriendsEnumerator::ContinueEnumeration(bool& errorOccured)
	{
		errorOccured = true;
		if (!m_Initialised || !m_Enumerating)
			return true;

		if (m_QuickEnumeration)
		{
			// quick enumeration means we use the presence info to show what friends are online.
			m_Enumerating = false;
			errorOccured = false;
		}
		else
		{
			SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();
			if (sysManager.m_Initialised)
			{
				// we must enumerate the full friends list from the OS.
				const SysProfile::C_NpThreadedFriendEnumerator::eStatus status = sysManager.m_pThreadedFriendEnumerator->Status();
				if (!m_RequestStarted)
				{
					if (SysProfile::C_NpThreadedFriendEnumerator::eStatusIdle == status)
					{
						if (sysManager.m_pThreadedFriendEnumerator->StartEnumeration())
						{
							// enumeration request accepted.
							m_RequestStarted = true;
							return false;
						}
					}
					else
						return false; // still waiting to start
				}
				else
				{
					if (SysProfile::C_NpThreadedFriendEnumerator::eStatusIdle != status)
						return false; // enumeration thread still running it's tasks.

					// results should be valid now.
					m_Enumerating = false;
					errorOccured = false;
				}
			}
		}
		return true;
	}


	// terminate any active friends enumerations.
	void C_FriendsEnumerator::StopEnumeration(void)
	{
		SysProfile::C_Manager& sysManager = SysProfile::C_Manager::Instance();
		if (sysManager.m_Initialised && !m_QuickEnumeration)
			sysManager.m_pThreadedFriendEnumerator->CancelEnumeration();

		m_UserIndex = -1;
		m_NumFriendsEnumerated = 0;
		m_Enumerating = false;
		m_RequestStarted = false;
	}


	//extract enumerated friends. SystemManager will have our latest cached friends. This isn't thread safe.
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
		if (sysManager.m_Initialised)
		{
			assert(pOutput);
			assert(maxOutput);
			if (pOutput && maxOutput)
			{
				if (sysManager.m_PrimaryPadNo != -1)
				{
					// must be primary player since we can only have 1 active account on PSN
					if (sysManager.m_PrimaryPadNo == m_UserIndex)
					{
						// quick enumeration means look in presence cache for online friends only.
						if (m_QuickEnumeration)
						{
							// is the friend already in our cache ?. this will be online friends only.
							SysProfile::C_NpFriend* pFriend = NULL;
							SysProfile::NpFriendsList::Iterator it;
							for (it = sysManager.m_pFriendsList->Begin(); it != sysManager.m_pFriendsList->End(); ++it)
							{
								if (maxOutput > numFriendsReturned)
									pOutput[numFriendsReturned++] = it->m_BaseProfile;
							}
						}
						else
						{
							SysProfile::NpEnumeratedFriendsList enumerated;
							if (sysManager.m_pThreadedFriendEnumerator->ExtractFriends(enumerated))
							{
								for (S32 i=0; i<enumerated.Size(); i++)
								{
									if (maxOutput > numFriendsReturned)
									{
										SysProfile::UserIdentifier fr;
										fr.m_IsGuest = false;
										fr.m_Uid = enumerated[i];

										pOutput[numFriendsReturned++] = fr;
									}
								}
							}
						}
					}
				}
			}
		}
		return numFriendsReturned;
	}

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
		if (sysManager.m_Initialised)
		{
			assert(pOutput);
			assert(maxOutput);
			if (pOutput && maxOutput)
			{
				if (sysManager.m_PrimaryPadNo != -1)
				{
					// must be primary player since we can only have 1 active account on PSN
					if (sysManager.m_PrimaryPadNo == m_UserIndex)
					{
						// is the friend in our presence cache ?.
						SysProfile::C_NpFriend* pFriend = NULL;
						SysProfile::NpFriendsList::Iterator it;

						// list what friend states we wist to return and in what order.
						Btl::Vector<eFriendState, Btl::VectorParameters_Static<3> > friendStates;
						friendStates.PushBack(eFriendOnline);
						friendStates.PushBack(eFriendOnlineButPlayingAnotherGame);

						for (S32 i=0; i<friendStates.Size(); i++)
						{
							for (it = sysManager.m_pFriendsList->Begin(); it != sysManager.m_pFriendsList->End(); ++it)
							{
								if (maxOutput > numFriendsReturned)
								{
									if (it->m_State == friendStates[i])
									{
										FriendProfile profile;
										profile.m_CanInvite							=	(it->m_State == eFriendOnline) ? true : false;
										profile.m_FriendState						=	it->m_State;
										profile.m_Profile							=	it->m_BaseProfile;
										profile.m_GameInvite						=	it->m_GameInvite;
										profile.m_RichPresence						=	it->m_RichPresence;
										pOutput[numFriendsReturned++]				=	profile;
									}
								}
							}
						}

						if (!m_QuickEnumeration)
						{
							// add any offline friends that we can't track via presence.
							SysProfile::NpEnumeratedFriendsList enumerated;
							if (sysManager.m_pThreadedFriendEnumerator->ExtractFriends(enumerated))
							{
								for (S32 i=0; i<enumerated.Size(); i++)
								{
									if (maxOutput > numFriendsReturned)
									{
										bool add = true;
										for (it = sysManager.m_pFriendsList->Begin(); it != sysManager.m_pFriendsList->End() && add; ++it)
										{
											SceNpId presNpId = it->m_BaseProfile.m_Uid;
											if (sceNpUtilCmpNpId(&presNpId, &enumerated[i]) == 0)
												add = false; // enumerated friend also found in presence data so ignore him as he will have already been added to the output.
										}

										if (add)
										{
											FriendProfile profile;
											profile.m_Profile.m_IsGuest = false;
											profile.m_Profile.m_Uid = enumerated[i];
											if (SysProfile::bizUTF8ToUCS2((uint8_t*)profile.m_Profile.m_Uid.handle.data, SCE_NET_NP_ONLINEID_MAX_LENGTH, profile.m_Profile.m_Name, SYSTEM_PROFILE_NAME_LENGTH))
												pOutput[numFriendsReturned++] =	profile;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return numFriendsReturned;
	}
};
