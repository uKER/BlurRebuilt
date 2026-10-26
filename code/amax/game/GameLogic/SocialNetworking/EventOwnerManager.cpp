#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <Gamelogic\SocialNetworking\EventOwnerManager.h>
#include <GameLogic\ContentServer\KeyArchiveManager.h>
#include <GameLogic\SinglePlayer\SinglePlayer.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\SocialNetworking\FriendsManager.h>

#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui\Game\Profiles\GamerPictureManager.h>

#include "bdLobby/bdLobbyCommon/bdAuthCommon.h"
#include <ui/Game/UILocaliser.h>

Debug::Logger &g_EventOwnerLogger( Debug::GetLogger("Amax.EventOwner") );

namespace Game
{

C_EventOwnerManager C_EventOwnerManager::s_instance;


void C_EventOwnerManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

	AddSubscriber( GameMessaging::E_GameMsg_Update );

	Clear();
}

void C_EventOwnerManager::Clear()
{
	mNeedToWriteStats=false;
	mNeedToReadStats=false;
	mReadingStats=false;

	mFriend=0;
}

void C_EventOwnerManager::OwnerStillFriendCheck()
{
	if(C_FriendsManager::Instance().GetFriendProfileFromOnlineID(mFriend) == NULL)
	{
		Clear();
	}
}

void C_EventOwnerManager::ReadFriendsStats()
{
	mNeedToReadStats=true;
	
}

void C_EventOwnerManager::WritePlayersStats()
{
	mNeedToWriteStats=true;
}

void C_EventOwnerManager::Update()
{
	if(mNeedToWriteStats)
	{
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			Game::C_GameProfile *gp=Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			RaceBook::C_RaceBookProfile* prof = gp->mRaceBookProfile;
					
			bool gotDemonwareID=Game::C_KeyArchiveManager::Instance().SetUserIndex(0);	//need to get proper index

			if(gotDemonwareID)
			{
				Game::C_KeyArchiveManager::Instance().ClearKeyArchiveToWrite();
				int index=0;

				if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
				{
					int tierCount = SP_GetTierCount(sg);

					for(int tierNo = 0; tierNo < tierCount; tierNo++)
					{
						if (SP_Tier* tier = SP_GetTier(sg, tierNo))
						{
							for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
							{
								if (SP_Event* event = tier->events[eventNo])
								{
									SP_EventState state=SP_GetEventState(sg, event);

									C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index), event->id);
									index++;

									U32 medals=0;

									//add the diff
									if(state != kSP_Incomplete)
									{
										U32 diff=SP_GetEventDifficulty(sg, event);
										medals+=diff<<24;
									}

									C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index), medals);
									index++;

									U32 score=0;
									if (event->kind == kSP_BossEvent || event->kind == kSP_CheckpointEvent)
										score=SP_GetEventBestTime(sg, event) * 1000;	// this is a float and key archive is an int, so * 1000

									if (event->kind == kSP_DestructionEvent)
										score=SP_GetEventBestPoints(sg, event);

									if (event->kind == kSP_RaceEvent || event->kind == kSP_FanEvent)
										score=SP_GetEventBestFans(sg, event);

									C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index), score);
									index++;

									LOGDEBUG(g_EventOwnerLogger, "Writing - (T%dE%d)EventId %x Medals %d Score %d", tierNo+1, eventNo+1, event->id, medals, score );
								}
							}
						}
					}
				}
				
				Game::C_KeyArchiveManager::Instance().WriteKeyArchive();

				mNeedToWriteStats=false;
			}
		}
	}

	if(mNeedToReadStats)
	{
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			RaceBook::C_RaceBookProfile* prof = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mRaceBookProfile;
			
			Game::C_KeyArchiveManager::Instance().SetDemonwareId(mFriend);

			mReadingStats=C_KeyArchiveManager::Instance().ReadKeyArchiveSubset(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS, KEY_ARCHIVE_MAX_SINGLE_PLAYER_EVENTS_SIZE);

			mNeedToReadStats=false;
		}
	}

	if(mReadingStats)
	{
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			int index=0;
			mFriendsStats.Clear();

			while(index < KEY_ARCHIVE_MAX_SINGLE_PLAYER_EVENTS_SIZE)
			{
				EventOwnerStats stats;
				U64 value;

				value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index));
				stats.mEventId=(U32)value;
				index++;
				
				value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index));
				stats.mMedals=(U32)value;
				index++;

				value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS+index));
				stats.mScore=(U32)value;
				index++;

				LOGDEBUG(g_EventOwnerLogger, "Reading - EventId %x Medals %d Score %d", stats.mEventId, stats.mMedals, stats.mScore );

				mFriendsStats.PushBack(stats);
			}
			
			UI::I_GamerPictureManager* pGamerPictureManager = UI::C_ProfileInterface::Instance().GamerPictureManager();

			if(pGamerPictureManager)
			{
			}

			mReadingStats=false;
		}
	}
}

const WCHAR *C_EventOwnerManager::GetScoreString(U32 sp_event_id)
{
	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		SP_Event *spevent = SP_GetEventById(sg, sp_event_id);

		if(spevent)
		{
			for(int a=0;a<mFriendsStats.Size();a++)
			{
				if(mFriendsStats[a].mEventId == sp_event_id)
				{
					if (spevent->kind == kSP_BossEvent || spevent->kind == kSP_CheckpointEvent)
					{
						float time=(float)mFriendsStats[a].mScore/1000.f;
						mScoreString.Format( UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( time ) );
					}
					else	//fans or points
					{
						
						mScoreString.Format( UI::C_UILocaliser::Instance().ConvertNumberToCommas(mFriendsStats[a].mScore) );
					}

					return mScoreString.Chars();
				}
			}
		}
	}

	mScoreString.Format(L"--");
	return mScoreString.Chars();
}

EVENT_OWNER_RESULT C_EventOwnerManager::BeatingFriend(U32 sp_event_id)
{
	if(C_FriendsManager::Instance().GetNumFriends() == 0 || mFriend == 0)
		return EVENT_OWNER_RESULT_NO_SCORE_DRAW;

	Game::C_GameProfile *gp=Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	U32 friends_medals=0;
	U32 friends_score=0;

	U32 your_medals=0;
	U32 your_score=0;

	SP_EventKind event_kind = kSP_RaceEvent;

	for(int a=0;a<mFriendsStats.Size();a++)
	{
		if(mFriendsStats[a].mEventId == sp_event_id)
		{
			friends_medals=mFriendsStats[a].mMedals;
			friends_score=mFriendsStats[a].mScore;
		}
	}

	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		SP_Event *event = SP_GetEventById(sg, sp_event_id);

		if(event)
		{
			event_kind=event->kind;

			SP_EventState state=SP_GetEventState(sg, event);

			//add the diff
			if(state != kSP_Incomplete)
			{
				U32 diff=SP_GetEventDifficulty(sg, event);
				your_medals+=diff<<24;
			}

			if (event->kind == kSP_BossEvent || event->kind == kSP_CheckpointEvent)
				your_score=SP_GetEventBestTime(sg, event) * 1000;	// this is a float and key archive is an int, so * 1000

			if (event->kind == kSP_DestructionEvent)
				your_score=SP_GetEventBestPoints(sg, event);

			if (event->kind == kSP_RaceEvent || event->kind == kSP_FanEvent)
				your_score=SP_GetEventBestFans(sg, event);
		}
	}

	if(your_medals == 0 || friends_medals == 0)
		return EVENT_OWNER_RESULT_NO_SCORE_DRAW;

	if(your_medals > friends_medals)
		return EVENT_OWNER_RESULT_WINNING;

	if(your_medals < friends_medals)
		return EVENT_OWNER_RESULT_LOSING;

	if(your_medals == friends_medals)
	{
		if (event_kind == kSP_BossEvent)
		{
			if(your_score < friends_score)
				return EVENT_OWNER_RESULT_WINNING;

			if(your_score > friends_score)
				return EVENT_OWNER_RESULT_LOSING;
		}
		else
		{
			if(your_score > friends_score)
				return EVENT_OWNER_RESULT_WINNING;

			if(your_score < friends_score)
				return EVENT_OWNER_RESULT_LOSING;
		}

		if(your_score == friends_score)
			return EVENT_OWNER_RESULT_SCORE_DRAW;
	}

	return EVENT_OWNER_RESULT_NO_SCORE_DRAW;
}

bool C_EventOwnerManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
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
