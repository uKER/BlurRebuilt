#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <Gamelogic\SocialNetworking\FriendsManager.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <ui\Game\Profiles\ProfileInterface.h>

#include <Gamelogic\SocialNetworking\EventOwnerManager.h>

#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"

#include <Game/Messaging/GameflowMessages.h>
#include "Ui/UiMessages.h"

namespace Game
{

class C_FriendChallenges;

C_FriendsManager C_FriendsManager::s_instance;

void C_FriendsManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( GameMessaging::E_GameMsg_PausedUpdate );

	mFriendsEnumerator.Initialise();

	mEnumerating=false;

	Clear();

}

void C_FriendsManager::Clear()
{
	if(mEnumerating)
	{
		mFriendsEnumerator.StopEnumeration();
		mEnumerating=false;
	}

	mFriends.Clear();
	mCurrentID=435;
}

void C_FriendsManager::StartEnumeration()
{
	if(mEnumerating)
		return;

	const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

	bool ok=mFriendsEnumerator.StartEnumeration(fp,false);

	if(ok)
	{
		mEnumerating=true;
	}
	else
	{
		BIZ_ASSERT_MSG_SPEED(false, "Unable to start friends enumeration");
		mEnumerating=false;
	}
}

void C_FriendsManager::AddFriend(SysProfile::BaseProfile prof)
{
	if(mEnumerating)
		return;

	bool add=true;

	//do we already have this friend?
	for(int b=0;b<mFriends.Size();b++)
	{
#ifdef PLATFORM_PS3
		bdOnlineUserID id1=bdAuthUtility::getUserID(prof.m_Uid.handle.data);
		bdOnlineUserID id2=bdAuthUtility::getUserID(mFriends[b].mProfile.m_Uid.handle.data);
		if(id1 == id2)
#else
		if(XOnlineAreUsersIdentical(mFriends[b].mProfile.m_Uid, prof.m_Uid))
#endif
		{
			add=false;
		}
	}

	if(add)
	{
		bdOnlineUserID id;
#ifdef PLATFORM_PS3
		id=bdAuthUtility::getUserID(prof.m_Uid.handle.data);
#else
		id=prof.m_Uid;
#endif
		FriendManager_Friend fmf;
		fmf.mProfile=prof;
		fmf.mID=mCurrentID;
		mCurrentID++;

		mFriends.PushBack(fmf);
	}
}

void C_FriendsManager::Update()
{

	if(mEnumerating)
	{
		bool error=false;

		if(mFriendsEnumerator.ContinueEnumeration(error))
		{
			mEnumerating=false;

			if(error)
			{
				//BIZ_ASSERT_MSG_SPEED(false, "Unable to enumerate friends - this isn't harmful just a warning");
				//BIZ_ASSERT_FAIL("There was an error in the Friends enumeration");
			}
			else
			{
				int num_friends=mFriendsEnumerator.ExtractFriends(mEnumeratedFriends,MAX_FRIEND_MANAGER_FRIENDS);

				if(num_friends == 0 && mFriends.Size())
				{
					BIZ_ASSERT_MSG_SPEED(false, "You had friends and now you don't");
				}

				//search through all enumerated friends for ones to add
				for(int a=0;a<num_friends;a++)
				{
					AddFriend(mEnumeratedFriends[a].m_Profile);
				}

				//now work out ones to remove
				for(int a=0;a<mFriends.Size();)
				{
					bool remove=true;

					for(int b=0;b<num_friends;b++)
					{
#ifdef PLATFORM_PS3
						bdOnlineUserID id1=bdAuthUtility::getUserID(mEnumeratedFriends[b].m_Profile.m_Uid.handle.data);
						bdOnlineUserID id2=bdAuthUtility::getUserID(mFriends[a].mProfile.m_Uid.handle.data);
						if(id1 == id2)
#else
						if(XOnlineAreUsersIdentical(mFriends[a].mProfile.m_Uid, mEnumeratedFriends[b].m_Profile.m_Uid))
#endif
						{
							remove=false;
						}
					}

					if(remove)
					{
						mFriends.EraseByIndex(a);
					}
					else
					{
						a++;
					}
				}

				_UI_MESSAGE( Game::E_GameflowMessage_FriendsUpdated );

				if(mFriends.Size())
				{
					C_EventOwnerManager::Instance().ReadFriendsStats();
				}

				C_EventOwnerManager::Instance().OwnerStillFriendCheck();

				C_FriendChallenges::Instance().FriendsUpdated();
			}
		}
	}
}

SysProfile::BaseProfile *C_FriendsManager::GetFriendProfileFromOnlineID(bdOnlineUserID id)
{
	for(int a=0;a<mFriends.Size();a++)
	{
#ifdef PLATFORM_PS3
		if(bdAuthUtility::getUserID(mFriends[a].mProfile.m_Uid.handle.data) == id)
#else
		if(mFriends[a].mProfile.m_Uid == id)
#endif
		{
			return &mFriends[a].mProfile;
		}
	}

	return NULL;
}

SysProfile::BaseProfile *C_FriendsManager::GetFriendProfileFromID(int id)
{
	for(int a=0;a<mFriends.Size();a++)
	{
		if(id == mFriends[a].mID)
		{
			return &mFriends[a].mProfile;
		}
	}

	return NULL;
}

bool C_FriendsManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case GameMessaging::E_GameMsg_PausedUpdate:
		case GameMessaging::E_GameMsg_Update:
			{
				Update();
			}
			break;

		default :
			handledMessage = false;
			break;
	}

	return handledMessage;
}

}; // namespace Game
