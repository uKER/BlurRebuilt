#include "stdafx.h"

#include <Game/Game.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <Game\Messaging\GameflowMessages.h>

#include <Gamelogic\gamedata\GameVehicleData.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include <ui\Game\UILocaliser.h>

#include <ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic\Profile\GameProfileManager.h>

#include <GameLogic\SocialNetworking\FriendsManager.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

#include <ui\Game\Profiles\GamerPictureManager.h>

#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

#include <GameLogic\ContentServer\KeyArchiveManager.h>

#include <GameLogic\Pickups\PickupBase.h>

#include <ui\Game\UILua.h>

#include <Game\Achievements\AvatarAwardSystem.h>

#include <GameLogic\ActorStudioActors\FanDemandPositionActor.h>

#include <algorithm>

namespace Game
{

C_FriendChallenges C_FriendChallenges::s_instance;

void C_FriendChallenges::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_GameflowMessage_ResultsCalculated );
	AddSubscriber( Game::E_GameflowMessage_StartUILoad );

	Clear();

}

void C_FriendChallenges::Clear()
{
	mAttempting=-1;
	mAttemptingFriend=-1;

	mNeedToUploadStats=false;
	mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_NOTHING;

	for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
	{
		mFriendDemandStore[a].mEventID=0;	//0 in the eventID means its empty
		mFriendDemandUserIDStore[a]=0;
	}

	for(int a=0;a<MAX_FRIEND_DEMAND_STATS;a++)
	{
		mFriendDemandStatsStore[a].Clear();		
	}
}

void C_FriendChallenges::Update()
{
	UploadStats();


	if(mGettingTaleOfTheTape==FRIEND_CHALLENGE_TOTT_STAGE_GETTING_NUMBER)
	{
		//wait for key archive to finish
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			//error, set the stage and return
			if(C_KeyArchiveManager::Instance().GetLastRequestFailed())
			{
				mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_ERROR;
				return;
			}

			mNumberofTOTT=C_KeyArchiveManager::Instance().GetValueFromKey(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS_NUMBER);

			if(mNumberofTOTT > (KEY_ARCHIVE_MAX_FRIEND_DEMAND_STATS_SIZE/KEY_ARCHIVE_NUM_FRIEND_DEMAND_KEYS_PER_USER))
			{
				BIZ_ASSERT_MSG(false, "Friend Challenges: Key archive is corrupt, trying to get too many friend stats");
				mNumberofTOTT=0;
			}

			//grab the stats num * 4, as there are four keys for each user (see KeyArchiveManager.h)
			if(C_KeyArchiveManager::Instance().ReadKeyArchiveSubset(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS, mNumberofTOTT * KEY_ARCHIVE_NUM_FRIEND_DEMAND_KEYS_PER_USER))
			{
				mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_GETTING_STATS;
			}
			else
			{
				mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_ERROR;
			}
		}
	}
	else if(mGettingTaleOfTheTape==FRIEND_CHALLENGE_TOTT_STAGE_GETTING_STATS)
	{
		//wait for key archive to finish
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			//error, set the stage and return
			if(C_KeyArchiveManager::Instance().GetLastRequestFailed())
			{
				mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_ERROR;
				return;
			}

			int index=0;

			while(index < (mNumberofTOTT*KEY_ARCHIVE_NUM_FRIEND_DEMAND_KEYS_PER_USER))
			{
				U64 value;

				value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index));
				index++;
				
				//find me in the archive
				SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
#ifdef PLATFORM_PS3
				bdOnlineUserID me=BLUR_USER_ID(profileMe.m_Uid);
#else
				bdOnlineUserID me=profileMe.m_Uid;
#endif
				if(value == me)
				{
					value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index));
					mTaleOfTheTape.mSent = (U32)(value >> 32);
					mTaleOfTheTape.mCompleted = (U32)(value&0xffffffff);
					index++;

					value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index));
					mTaleOfTheTape.mKOs = (U32)(value >> 32);
					mTaleOfTheTape.mMostAttempts = (U32)(value&0xffffffff);
					index++;

					value=C_KeyArchiveManager::Instance().GetValueFromKey((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index));
					mTaleOfTheTape.mSpare = (U32)(value >> 32);
					mTaleOfTheTape.mTotalAttempts = (U32)(value&0xffffffff);
					index++;

					mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_ALL_DONE;

					return;
				}
				else
				{
					index+=3;
				}
			}

			mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_NOTHING;
		}
	}
}

int C_FriendChallenges::HaveWeOldChallenges()
{
	int count=0;

	for(int i=0;i<Game::MAX_FRIEND_DEMANDS;i++)
	{
		const Game::FriendChallenge *fc=&mFriendDemandStore[i];
		const bdOnlineUserID fc_id=mFriendDemandUserIDStore[i];

		if(fc->mEventID)
		{
			bool old=true;

			for(int a=0;a<Game::C_FriendsManager::Instance().GetNumFriends();a++)
			{
				SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromIndex(a);

#ifdef PLATFORM_PS3
				bool identical_ids=fc_id == BLUR_USER_ID(prof->m_Uid);
				if(!wcscmp(fc->mChallenger, prof->Name()) || identical_ids )
#else
				bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, fc_id) ? true : false;
				if(!wcsicmp(fc->mChallenger, prof->Name()) || identical_ids )
#endif
				{
					old=false;
					break;
				}
			}
			
			if(old)
			{
				count++;
			}
		}
	}

	return count;
}

void C_FriendChallenges::CleanOutOldChallenges()
{
	for(int i=0;i<Game::MAX_FRIEND_DEMANDS;i++)
	{
		Game::FriendChallenge *fc=&mFriendDemandStore[i];
		const bdOnlineUserID fc_id=mFriendDemandUserIDStore[i];

		if(fc->mEventID)
		{
			bool old=true;

			for(int a=0;a<Game::C_FriendsManager::Instance().GetNumFriends();a++)
			{
				SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromIndex(a);

#ifdef PLATFORM_PS3
				bool identical_ids=fc_id == BLUR_USER_ID(prof->m_Uid);
				if(!wcscmp(fc->mChallenger, prof->Name()) || identical_ids)
#else
				bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, fc_id) ? true : false;
				if(!wcsicmp(fc->mChallenger, prof->Name()) || identical_ids)
#endif
				{
					old=false;
					break;
				}
			}
			
			if(old)
			{
				fc->mEventID=0;	//we just delete things by setting this to zero
			}
		}
	}
}

int C_FriendChallenges::GetKeyedServerIndex()
{
	int index=-1;


	return index;
}

int C_FriendChallenges::GetKeyedLocalIndex()
{
	int index=-1;


	return index;
}

void C_FriendChallenges::GetFriendStats(int friend_index)
{
	mTaleOfTheTape.Clear();

	if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
	{

#ifdef PLATFORM_PS3
		mTOTTUser=BLUR_USER_ID(C_FriendsManager::Instance().GetFriendProfileFromID(friend_index)->m_Uid);
#else
		mTOTTUser=C_FriendsManager::Instance().GetFriendProfileFromID(friend_index)->m_Uid;
#endif

		Game::C_KeyArchiveManager::Instance().SetDemonwareId(mTOTTUser);

		if(C_KeyArchiveManager::Instance().ReadKeyArchiveSubset(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS_NUMBER, 1))
		{
			mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_GETTING_NUMBER;
		}
		else
		{
			mGettingTaleOfTheTape=FRIEND_CHALLENGE_TOTT_STAGE_ERROR;
		}
	}
}

void C_FriendChallenges::CreateFriendDemandPostRace()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();

	for(int a=0;a<MAX_FRIEND_DEMAND_INTERNAL_PARAMS;a++)
	{
		mParameters[a].Clear();
	}

	for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
		mFriendsAdded[a]=-1;

	switch (levelstate.mRaceMode)
	{
		case E_RaceMode_StreetRace:
		case E_RaceMode_FriendDemand_StreetRace:
			mType=FRIEND_CHALLENGE_TYPE_RACE;
			CreateRaceDemand();
			break;
		case E_RaceMode_Destruction:
		case E_RaceMode_FriendDemand_Destruction:
			mType=FRIEND_CHALLENGE_TYPE_DESTRUCTION;
			CreateDestructionDemand();
			break;
		case E_RaceMode_Checkpoint:
		case E_RaceMode_FriendDemand_Checkpoint:
			mType=FRIEND_CHALLENGE_TYPE_CHECKPOINT;
			CreateCheckpointDemand();
			break;
		case E_RaceMode_FanDemand:
		case E_RaceMode_FriendDemand_FanDemand:
			mType=FRIEND_CHALLENGE_TYPE_FAN_DEMAND;
			CreateFanAttackDemand();
			break;
		case E_RaceMode_BossBattle:
		case E_RaceMode_FriendDemand_BossBattle:
			mType=FRIEND_CHALLENGE_TYPE_BOSS;
			CreateBossBattleDemand();
			break;
	};
}

const WCHAR *C_FriendChallenges::GetCriteriaTitleString(int index, int store_index, const FriendChallenge *fc)
{
	static UI::C_WideString64 str;
	str.Clear();

	FRIEND_CHALLENGE_TYPE type;
	if(store_index == -1)
	{
		if(fc)
		{
			type=fc->mType;

		}
		else
		{
			type=mType;
		}
	}
	else
	{
		type=mFriendDemandStore[store_index].mType;
	}

	switch(index)
	{
		//demand type dependant
		case 0:
			{
				if(type == FRIEND_CHALLENGE_TYPE_RACE)
				{
					str=UI::C_TextStringID("FDE_FANS_GAINED").String();
				}
				if(type == FRIEND_CHALLENGE_TYPE_FAN_DEMAND)
				{
					str=UI::C_TextStringID("FDE_FAN_DEMANDS").String();
				}
				if(type == FRIEND_CHALLENGE_TYPE_DESTRUCTION)
				{
					str=UI::C_TextStringID("FDE_POINTS_GAINED").String();
				}
				if(type == FRIEND_CHALLENGE_TYPE_CHECKPOINT)
				{
					str=UI::C_TextStringID("FDE_TIME_REMAINING").String();
				}
				if(type == FRIEND_CHALLENGE_TYPE_BOSS)
				{
					str=UI::C_TextStringID("FDE_RACE_TIME").String();
				}
			}
			break;
		
		//always difficulty (at the moment)
		case 1: str=UI::C_TextStringID("FDE_DIFFICULTY").String(); break;
		//always medal achieved (at the moment)
		case 2:	str=UI::C_TextStringID("FDE_MEDAL_ACHIEVED").String(); break;
		//always vehicle used (at the moment)
		case 3:	str=UI::C_TextStringID("FDE_VEHICLE_USED").String(); break;
	};

	return str.Chars();
}


const WCHAR *C_FriendChallenges::GetCriteriaVariableString(int index, int store_index, const FriendChallenge *fc)
{
	static UI::C_WideString64 str;
	str.Clear();

	switch(index)
	{
		//demand type dependant
		case 0:
			{
				U32 u32_criteria;
				float float_criteria;
				FRIEND_CHALLENGE_TYPE type;
				if(store_index == -1)
				{
					if(fc)
					{
						type=fc->mType;
						u32_criteria=fc->mU32Criteria;
						float_criteria=fc->mFloatCriteria;
					}
					else
					{
						type=mType;
						u32_criteria=mU32Criteria;
						float_criteria=mFloatCriteria;
					}
				}
				else
				{
					type=mFriendDemandStore[store_index].mType;
					u32_criteria=mFriendDemandStore[store_index].mU32Criteria;
					float_criteria=mFriendDemandStore[store_index].mFloatCriteria;
				}

				if(type == FRIEND_CHALLENGE_TYPE_RACE)
				{
					str.Format(UI::C_TextStringID("FDE_S_FANS").String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(u32_criteria));
				}

				if(type == FRIEND_CHALLENGE_TYPE_FAN_DEMAND)
				{
					str.Format(UI::C_TextStringID("FDE_S_FAN_DEMANDS").String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas(u32_criteria));
				}

				if(type == FRIEND_CHALLENGE_TYPE_DESTRUCTION)
				{
					str.Format(UI::C_TextStringID("FDE_S_POINTS").String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas(u32_criteria));
				}

				if(type == FRIEND_CHALLENGE_TYPE_CHECKPOINT)
				{
					str.Format(UI::C_TextStringID("FDE_MORE_THAN_S").String(),UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(float_criteria));
				}

				if(type == FRIEND_CHALLENGE_TYPE_BOSS)
				{
					str.Format(UI::C_TextStringID("FDE_LESS_THAN_S").String(),UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(float_criteria));
				}
			}
			break;
		
		//always difficulty (at the moment)
		case 1:
			{
				U8 diff;
				if(store_index == -1)
				{
					if(fc)
					{
						diff=fc->mDifficulty;
					}
					else
					{
						diff=mDifficulty;
					}
				}
				else
				{
					diff=mFriendDemandStore[store_index].mDifficulty;
				}

				if(diff == GAME_DIFFICULTY_VERY_EASY)
					str.Format(UI::C_TextStringID("CMN_DIFFICULTY_1").String());
				else if(diff == GAME_DIFFICULTY_EASY)
					str.Format(UI::C_TextStringID("CMN_DIFFICULTY_2").String());
				else if(diff == GAME_DIFFICULTY_NORMAL)
					str.Format(UI::C_TextStringID("CMN_DIFFICULTY_3").String());
				else if(diff == GAME_DIFFICULTY_HARD)
					str.Format(UI::C_TextStringID("CMN_DIFFICULTY_4").String());
				else if(diff == GAME_DIFFICULTY_HARDCORE)
					str.Format(UI::C_TextStringID("CMN_DIFFICULTY_5").String());
			}
			break;
		
		//always medal achieved (at the moment)
		case 2:
			{
				U8 medal;
				if(store_index == -1)
				{
					if(fc)
					{
						medal=fc->mMedal;
					}
					else
					{
						medal=mMedal;
					}
				}
				else
				{
					medal=mFriendDemandStore[store_index].mMedal;
				}

				if(medal == kSP_Gold)
					str.Format(L"~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~");
				else if(medal == kSP_Silver)
					str.Format(L"~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~");
				else if(medal == kSP_Bronze)
					str.Format(L"~~{brolt}~~ ~~{brolt}~~ ~~{brolt}~~");
			}
			break;
		
		//always vehicle used (at the moment)
		case 3:
			{
				U32 vehicle;
				if(store_index == -1)
				{
					if(fc)
					{
						vehicle=fc->mVehicleUsed;
					}
					else
					{
						vehicle=mVehicleUsed;
					}
				}
				else
				{
					vehicle=mFriendDemandStore[store_index].mVehicleUsed;
				}

				str.Format(UI::C_TextStringID(C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)vehicle]->loc_string.id).String());
			}
			break;
	};

	return str.Chars();
}

FRIEND_CHALLENGE_PARAM_TYPE C_FriendChallenges::GetParamType(int index, int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mParameters[index].mParamType;
		}
		else
		{
			return mParameters[index].mParamType;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mParameters[index].mParamType;
	}
}

const WCHAR *C_FriendChallenges::GetParamTitleString(int index, int store_index, const FriendChallenge *fc)
{
	static UI::C_WideString64 str;
	str.Clear();

	const FriendChallengeParameter *param;
	if(store_index == -1)
	{
		if(fc)
		{
			param=&fc->mParameters[index];
		}
		else
		{
			param=&mParameters[index];
		}
	}
	else
	{
		param=&mFriendDemandStore[store_index].mParameters[index];
	}

	switch(param->mParamType)
	{
		case FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT:				str=UI::C_TextStringID("FDE_SHUNTS_HIT").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT:				str=UI::C_TextStringID("FDE_BARGES_HIT").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT:					str=UI::C_TextStringID("FDE_MINES_HIT").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT:			str=UI::C_TextStringID("FDE_BLIND_FIRES_HIT").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED:				str=UI::C_TextStringID("FDE_NITROUS_USED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE:				str=UI::C_TextStringID("FDE_FAN_RUN_SCORE").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME:					str=UI::C_TextStringID("FDE_RACE_TIME").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED:				str=UI::C_TextStringID("FDE_FANS_GAINED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY: 			str=UI::C_TextStringID("FDE_BOLT_ACCURACY").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED: 			str=UI::C_TextStringID("FDE_TOTAL_WRECKED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED:		str=UI::C_TextStringID("FDE_BLUE_WRECKED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED:			str=UI::C_TextStringID("FDE_RED_WRECKED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED:		str=UI::C_TextStringID("FDE_YELLOW_WRECKED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED:		str=UI::C_TextStringID("FDE_GREEN_WRECKED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED:			str=UI::C_TextStringID("FDE_STOPWATCHES_USED").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY:			str=UI::C_TextStringID("FDE_PICKUP_ACCURACY").String();	break;
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS:				str=UI::C_TextStringID("FDE_PICKUPS_HIT").String();	break;
	};

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetParamVariableString(int index, int store_index, const FriendChallenge *fc)
{
	static UI::C_WideString64 str;
	str.Clear();

	const FriendChallengeParameter *param;
	if(store_index == -1)
	{
		if(fc)
		{
			param=&fc->mParameters[index];
		}
		else
		{
			param=&mParameters[index];
		}
	}
	else
	{
		param=&mFriendDemandStore[store_index].mParameters[index];
	}

	switch(param->mParamType)
	{
		case FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT:
		case FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT:
		case FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT:
		case FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT:
		case FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED:
		case FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE:
		case FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED:
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED:
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED:
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED:
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED:
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED:
		case FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED:
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS:
			{
				str.Format(L"%d",param->mU32Value);
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY:
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY:
			{
				str.Format(L"%d~~/p~~",param->mU32Value);
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME:
			{
				str.Format(L"%s",UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(param->mFloatValue));
			}
			break;
	};

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetParamSwitchString(int index, int store_index, const FriendChallenge *fc)
{
	static UI::C_WideString64 str;
	str.Clear();

	const FriendChallengeParameter *param;
	if(store_index == -1)
	{
		if(fc)
		{
			param=&fc->mParameters[index];
		}
		else
		{
			param=&mParameters[index];
		}
	}
	else
	{
		param=&mFriendDemandStore[store_index].mParameters[index];
	}

	switch(param->mModifier)
	{
		case FRIEND_CHALLENGE_MODIFIER_NOT_USED:			str=UI::C_TextStringID("FDE_DONT_USE").String(); break;
		case FRIEND_CHALLENGE_MODIFIER_LESS_THAN:			str=UI::C_TextStringID("FDE_LESS_THAN").String(); break;
		case FRIEND_CHALLENGE_MODIFIER_EQUALS:				str=UI::C_TextStringID("FDE_EQUALS").String(); break;
		case FRIEND_CHALLENGE_MODIFIER_GREATER_THAN:		str=UI::C_TextStringID("FDE_GREATER_THAN").String(); break;
	};

	return str.Chars();
}

int C_FriendChallenges::GetParamCount()
{
	int num=0;
	
	do
	{
		num++;
	} while(mParameters[num].mParamType != FRIEND_CHALLENGE_PARAM_TYPE_NONE);

	return num;
}


U32	C_FriendChallenges::GetEventID(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mEventID;
		}
		else
		{
			return mEventID;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mEventID;
	}
}

U8 C_FriendChallenges::GetMedal(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mMedal;
		}
		else
		{
			return mMedal;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mMedal;
	}
}

U8 C_FriendChallenges::GetDifficulty(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mDifficulty;
		}
		else
		{
			return mDifficulty;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mDifficulty;
	}
}

U32 C_FriendChallenges::GetVehicleUsed(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mVehicleUsed;
		}
		else
		{
			return mVehicleUsed;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mVehicleUsed;
	}
}

const FriendChallengeParameter& C_FriendChallenges::GetExtraChallenge(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mParameters[0];
		}
		else
		{
			return mFriendDemand.mParameters[0];
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mParameters[0];
	}
}

bool C_FriendChallenges::IsRechallenge(int store_index, const FriendChallenge *fc)
{
	if(store_index == -1)
	{
		if(fc)
		{
			return fc->mReChallenge;
		}
		else
		{
			return mFriendDemand.mReChallenge;
		}
	}
	else
	{
		return mFriendDemandStore[store_index].mReChallenge;
	}
}

void C_FriendChallenges::ChangeSwitch(int index, int switch_enum)
{
	mParameters[index].mModifier=switch_enum;
}

int C_FriendChallenges::NumActiveSwitches()
{
	int num=0;

	for(int a=0;a<MAX_FRIEND_DEMAND_INTERNAL_PARAMS;a++)
	{
		if(mParameters[a].mModifier != FRIEND_CHALLENGE_MODIFIER_NOT_USED)
		{
			num++;
		}
	}

	return num;
}

FRIEND_CHALLENGE_MODIFIER C_FriendChallenges::GetSwitch(int index)
{
	return (FRIEND_CHALLENGE_MODIFIER)mParameters[index].mModifier;
}

bool C_FriendChallenges::CreateReChallengeFriendDemand()
{
	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();
	const bdOnlineUserID fc_id=mFriendDemandUserIDStore[mAttempting];

	mFriendDemand.mType=fc->mType;

	wcsncpy(mFriendDemand.mChallenger,UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name(),MAX_FRIEND_CHALLENGE_USERNAME);

	mFriendDemand.mDate=Time::GetCurrentTime();

	wcsncpy(mFriendDemand.mName, fc->mName, MAX_FRIEND_CHALLENGE_NAME);

	mFriendDemand.mVehicleUsed=fc->mVehicleUsed;
	
	mFriendDemand.mDifficulty=fc->mDifficulty;

	mFriendDemand.mEventID=mEventID;

	mFriendDemand.mMedal=mMedal;

	mFriendDemand.mAttempts=0;
	
	mFriendDemand.mReChallenge=true;

	for(int a=0;a<MAX_FRIEND_DEMAND_KEY;a++)
		mFriendDemand.mKey[a]=fc->mKey[a];

	mFriendDemand.mBossMod = fc->mBossMod;

	if(mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_RACE || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_FAN_DEMAND || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_DESTRUCTION)
		mFriendDemand.mU32Criteria=mU32Criteria;

	if(mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_CHECKPOINT || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_BOSS)
		mFriendDemand.mFloatCriteria=mFloatCriteria;

	int param=-1;
	for(int a=0;a<MAX_FRIEND_DEMAND_INTERNAL_PARAMS;a++)
	{
		if(mParameters[a].mParamType == fc->mParameters[0].mParamType)
		{
			param=a;
		}
	}

	if(param == -1)
	{
		BIZ_ASSERT_MSG(false, "Unable to find Friend Demand parameter");
	}

	mFriendDemand.mParameters[0].Clear();
	mFriendDemand.mParameters[0]=fc->mParameters[0];

	if(mFriendDemand.mParameters[0].mParamType == FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME)
		mFriendDemand.mParameters[0].mFloatValue=mParameters[param].mFloatValue;
	else
		mFriendDemand.mParameters[0].mU32Value=mParameters[param].mU32Value;

	FRIEND_CHALLENGE_PARAM_USE use=GetParamUse(mFriendDemand.mParameters[0].mParamType);
	if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_LESS_THAN)
	{
		if(use == FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN)
		{
			mFriendDemand.mParameters[0].mModifier=FRIEND_CHALLENGE_MODIFIER_EQUALS;
		}
	}
	if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_GREATER_THAN)
	{
		if(use == FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS)
		{
			mFriendDemand.mParameters[0].mModifier=FRIEND_CHALLENGE_MODIFIER_EQUALS;
		}
	}

	int slot_id=-1;

	for(int a=0;a<C_FriendsManager::Instance().GetNumFriends();a++)
	{
		SysProfile::BaseProfile *prof=C_FriendsManager::Instance().GetFriendProfileFromIndex(a);

#ifdef PLATFORM_PS3
		bool identical_ids=fc_id == BLUR_USER_ID(prof->m_Uid);
		if(!wcscmp(prof->Name(),fc->mChallenger) || identical_ids)
#else
		bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, fc_id) ? true : false;
		if(!wcsicmp(prof->Name(),fc->mChallenger) || identical_ids)
#endif
		{
			slot_id=C_FriendsManager::Instance().GetFriendIDFromIndex(a);
			break;
		}
	}

	if(slot_id == -1)
	{
		return false;
	}
	else
	{
		for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
			mFriendsAdded[a]=-1;

		AddFriend(slot_id,0,false);

		return true;
	}
}

void C_FriendChallenges::CreateActualFriendDemand()
{
	mFriendDemand.mType=mType;

	wcsncpy(mFriendDemand.mChallenger,UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name(),MAX_FRIEND_CHALLENGE_USERNAME);

	mFriendDemand.mDate=Time::GetCurrentTime();

	mFriendDemand.mVehicleUsed=mVehicleUsed;
	
	mFriendDemand.mMedal=mMedal;
	mFriendDemand.mDifficulty=mDifficulty;

	mFriendDemand.mEventID=mEventID;

	mFriendDemand.mAttempts=0;

	mFriendDemand.mReChallenge=false;

	for(int a=0;a<MAX_FRIEND_DEMAND_KEY;a++)
	{
		mFriendDemand.mKey[a]=0;
	}
	Game::C_GameProfile* game_profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();

	if(levelstate.mCars[0].mLoadOuts.Size() > 0)
		mFriendDemand.mBossMod = levelstate.mCars[0].mLoadOuts[0];
	else
		mFriendDemand.mBossMod = -1;

	if(mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_RACE || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_FAN_DEMAND || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_DESTRUCTION)
		mFriendDemand.mU32Criteria=mU32Criteria;

	if(mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_CHECKPOINT || mFriendDemand.mType == FRIEND_CHALLENGE_TYPE_BOSS)
		mFriendDemand.mFloatCriteria=mFloatCriteria;

	for(int a=0;a<MAX_FRIEND_DEMAND_SENT_PARAMS;a++)
	{
		mFriendDemand.mParameters[a].Clear();
	}

	int num=0;
	for(int a=0;a<MAX_FRIEND_DEMAND_INTERNAL_PARAMS;a++)
	{
		if(mParameters[a].mModifier != FRIEND_CHALLENGE_MODIFIER_NOT_USED)
		{
			BIZ_ASSERT(num < MAX_FRIEND_DEMAND_SENT_PARAMS);

			mFriendDemand.mParameters[num]=mParameters[a];

			num++;
		}
	}
}

void C_FriendChallenges::CreateRaceDemand()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	mU32Criteria=results.mRaceStatistics.mFansEarned;

	mVehicleUsed=levelstate.mCars[0].mCarID;
	mMedal=results.mRaceStatistics.mMedal;
	mDifficulty=levelstate.m_difficulty;
	mEventID=levelstate.mEvent;

	mParameters[0].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT;
	mParameters[0].mU32Value=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
	
	mParameters[1].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT;
	mParameters[1].mU32Value=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
	
	mParameters[2].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT;
	mParameters[2].mU32Value=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];
	
	mParameters[3].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT;
	mParameters[3].mU32Value=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
	
	mParameters[4].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED;
	mParameters[4].mU32Value=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_NITROUS];

	mParameters[5].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME;
	mParameters[5].mFloatValue=results.mCars[0].mStatistics.mTotalRaceTime;
	
	mParameters[6].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE;
	mParameters[6].mU32Value=results.mRaceStatistics.mGoldenFanDemandScore;

	mParameters[7].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	mParameters[7].mU32Value=0;
}

void C_FriendChallenges::CreateFanAttackDemand()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	mU32Criteria=results.mRaceStatistics.mFanDemandsComplete;

	mVehicleUsed=levelstate.mCars[0].mCarID;
	mMedal=results.mRaceStatistics.mMedal;
	mDifficulty=levelstate.m_difficulty;
	mEventID=levelstate.mEvent;

	mParameters[0].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED;
	mParameters[0].mU32Value=results.mRaceStatistics.mFansEarned;
	
	mParameters[1].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE;
	mParameters[1].mU32Value=results.mRaceStatistics.mGoldenFanDemandScore;
	
	mParameters[2].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	mParameters[2].mU32Value=0;
}

void C_FriendChallenges::CreateDestructionDemand()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	mU32Criteria=results.mRaceStatistics.mDestruction_NumPoints;

	mVehicleUsed=levelstate.mCars[0].mCarID;
	mMedal=results.mRaceStatistics.mMedal;
	mDifficulty=levelstate.m_difficulty;
	mEventID=levelstate.mEvent;

	int index=0;

	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED;
	mParameters[index].mU32Value=results.mRaceStatistics.mFansEarned;
	index++;
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE;
	mParameters[index].mU32Value=results.mRaceStatistics.mGoldenFanDemandScore;
	index++;
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY;
	mParameters[index].mU32Value=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE] ? int(((float)results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE] / (float)results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE]) * 100) : 0;
	index++;
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED;
	mParameters[index].mU32Value=results.mRaceStatistics.mDestruction_TotalCarsWrecked;
	index++;

	if(GetNumDestructionVehicles(DM_AI__VHard))
	{
		mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED;
		mParameters[index].mU32Value=results.mRaceStatistics.mDestruction_VHardCarsWrecked;
		index++;
	}

	if(GetNumDestructionVehicles(DM_AI__Hard))
	{
		mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED;
		mParameters[index].mU32Value=results.mRaceStatistics.mDestruction_HardCarsWrecked;
		index++;
	}

	if(GetNumDestructionVehicles(DM_AI__Medium))
	{
		mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED;
		mParameters[index].mU32Value=results.mRaceStatistics.mDestruction_MediumCarsWrecked;
		index++;
	}	
	
	if(GetNumDestructionVehicles(DM_AI__Easy))
	{
		mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED;
		mParameters[index].mU32Value=results.mRaceStatistics.mDestruction_EasyCarsWrecked;
		index++;
	}

	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	mParameters[index].mU32Value=0;
}

void C_FriendChallenges::CreateCheckpointDemand()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	mFloatCriteria=results.mRaceStatistics.mTotalRaceTime;

	mVehicleUsed=levelstate.mCars[0].mCarID;
	mMedal=results.mRaceStatistics.mMedal;
	mDifficulty=levelstate.m_difficulty;
	mEventID=levelstate.mEvent;

	mParameters[0].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED;
	mParameters[0].mU32Value=results.mRaceStatistics.mFansEarned;
	
	int index=1;
	if(GetNumPickups(E_PICKUP_TYPE_NITROUS))
	{
		mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED;
		mParameters[index].mU32Value=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_NITROUS];
		index++;
	}
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED;
	mParameters[index].mU32Value=results.mCars[0].mStatistics.mStopwatchPickupsCollected;
	index++;
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE;
	mParameters[index].mU32Value=results.mRaceStatistics.mGoldenFanDemandScore;
	index++;
	
	mParameters[index].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	mParameters[index].mU32Value=0;
}

void C_FriendChallenges::CreateBossBattleDemand()
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	mFloatCriteria=results.mCars[0].mStatistics.mTotalRaceTime;

	mVehicleUsed=levelstate.mCars[0].mCarID;
	mMedal=results.mRaceStatistics.mMedal;
	mDifficulty=levelstate.m_difficulty;
	mEventID=levelstate.mEvent;

	float agressive_pickups_fired=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHOCK];
	agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHUNT];
	agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE];
	agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BARGE];
	agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BOMB];
	
	float agressive_pickups_hit=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHOCK];
	agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
	agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
	agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
	agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];

	mParameters[0].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY;
	mParameters[0].mU32Value=agressive_pickups_fired ? U32((agressive_pickups_hit/agressive_pickups_fired) * 100) : 0;
	
	mParameters[1].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED;
	mParameters[1].mU32Value=results.mRaceStatistics.mFansEarned;
	
	mParameters[2].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS;
	mParameters[2].mU32Value=(U32)agressive_pickups_hit;
	
	mParameters[3].mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	mParameters[3].mU32Value=0;
}

int C_FriendChallenges::NumFriendsAdded()
{
	int num=0;
	for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
	{
		if(mFriendsAdded[a] != -1)
			num++;
	}

	return num;
}

int C_FriendChallenges::GetFriendSlot(int index)
{
	return mFriendsAdded[index];
}

void C_FriendChallenges::RequestRemotePicture(int friend_slot, int ref)
{
	UI::I_GamerPictureManager* pGamerPictureManager = UI::C_ProfileInterface::Instance().GamerPictureManager();
	if(pGamerPictureManager && C_FriendsManager::Instance().GetFriendProfileFromID(friend_slot))
	{
		pGamerPictureManager->RequestRemotePicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), *C_FriendsManager::Instance().GetFriendProfileFromID(friend_slot), ref );
	}
}

const WCHAR *C_FriendChallenges::GetFriendDemandName(int store_index)
{
	if(store_index == -1)
		return mFriendDemand.mName;
	else
		return mFriendDemandStore[store_index].mName;
}

const WCHAR *C_FriendChallenges::GetFriendDemandDescription(int store_index)
{
	if(store_index == -1)
		return mFriendDemand.mDescription;
	else
		return mFriendDemandStore[store_index].mDescription;
}


void C_FriendChallenges::SetFriendDemandName(const WCHAR *name, bool do_check)
{
	mIsNameOk=true;
	wcsncpy(mFriendDemand.mName, name, MAX_FRIEND_CHALLENGE_NAME);
}

void C_FriendChallenges::SetFriendDemandDescription(const WCHAR *desc, bool do_check)
{
	mIsDescOk=true;
	wcsncpy(mFriendDemand.mDescription, desc, MAX_FRIEND_CHALLENGE_DESC);
}

FriendChallenge *C_FriendChallenges::GetFriendDemand(int store_index)
{
	if(store_index == -1)
		return &mFriendDemand;
	else
		return &mFriendDemandStore[store_index];
}

void C_FriendChallenges::AddFriend(int friend_slot, int friend_demand_slot, bool request_gamerpic)
{
	mFriendsAdded[friend_demand_slot]=friend_slot;

	if ( friend_slot == -1 )
		return;

	if (request_gamerpic)
		RequestRemotePicture(friend_slot, friend_demand_slot);
}

void C_FriendChallenges::AttemptingFriendDemand(int index, int friend_index)
{
	mAttempting=index;
	
	mAttemptingFriend=friend_index;
	
	int slot=GetStatSlot(mAttemptingFriend);

	if(slot == -1)
		return;

	mFriendDemandStatsStore[slot].mTotalAttempts++;
	
	mFriendDemandStore[mAttempting].mAttempts++;
}

void C_FriendChallenges::ReAttemptFriendDemand()
{
	int slot=GetStatSlot(mAttemptingFriend);

	if(slot == -1)
		return;

	mFriendDemandStatsStore[slot].mTotalAttempts++;

	mFriendDemandStore[mAttempting].mAttempts++;
}

FRIEND_CHALLENGE_RESULT_TYPE C_FriendChallenges::AttemptingMedalResult()
{
	FRIEND_CHALLENGE_RESULT_TYPE result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;

	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	SP_EventState medal=kSP_Incomplete;

	C_RaceModeBase* pRaceModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	switch (levelstate.mRaceMode)
	{
		case E_RaceMode_StreetRace:
		case E_RaceMode_FriendDemand_StreetRace:
			medal=((C_RaceModeStreetRace*)pRaceModeBase)->GetEventState();
			break;

		case E_RaceMode_Destruction:
		case E_RaceMode_FriendDemand_Destruction:
			medal=((C_RaceModeDestruction*)pRaceModeBase)->GetEventState();
			break;

		case E_RaceMode_Checkpoint:
		case E_RaceMode_FriendDemand_Checkpoint:
			medal=((C_RaceModeCheckpoint*)pRaceModeBase)->GetEventState();
			break;

		case E_RaceMode_FanDemand:
		case E_RaceMode_FriendDemand_FanDemand:
			medal=((C_RaceModeFanDemand*)pRaceModeBase)->GetEventState();
			break;

		case E_RaceMode_BossBattle:
		case E_RaceMode_FriendDemand_BossBattle:
			medal=((C_RaceModeBossBattle*)pRaceModeBase)->GetEventState();
			break;
	};

	if(medal >= fc->mMedal)
	{
		result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
	}

	return result_type;
}

FRIEND_CHALLENGE_RESULT_TYPE C_FriendChallenges::AttemptingCriteriaResult()
{
	FRIEND_CHALLENGE_RESULT_TYPE result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;

	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	switch(fc->mType)
	{
		case FRIEND_CHALLENGE_TYPE_RACE:
			{
				const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

				if((U32)scoring.mFinalScoreForUI > fc->mU32Criteria)
				{
					result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSED;
				}
			}
			break;

		case FRIEND_CHALLENGE_TYPE_FAN_DEMAND:
			{
				const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

				if((U32)scoring.mFanDemandsComplete > fc->mU32Criteria)
				{
					result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSED;
				}
			}
			break;

		case FRIEND_CHALLENGE_TYPE_DESTRUCTION:
			{
				const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

				if((U32)scoring.mDestructionPoints > fc->mU32Criteria)
				{
					result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSED;
				}
			}
			break;

		case FRIEND_CHALLENGE_TYPE_CHECKPOINT:
			{
				const VehicleScoringInfo& scoring = pLogicActor->GetScoring();

				if(scoring.mCheckpointTimeLeft > fc->mFloatCriteria)
				{
					result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
				}
			}
			break;

		case FRIEND_CHALLENGE_TYPE_BOSS:
			{
				if(pLogicActor->GetStatistics().mTotalRaceTime < fc->mFloatCriteria)
				{
					result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
				}
			}
			break;
	}

	return result_type;
}

FRIEND_CHALLENGE_RESULT_TYPE C_FriendChallenges::GetParamResult(int value)
{
	FRIEND_CHALLENGE_RESULT_TYPE result_type = FRIEND_CHALLENGE_RESULT_TYPE_MAX;

	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_LESS_THAN)
	{
		if((U32)value < fc->mParameters[0].mU32Value)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILED;
		}
	}
	else if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_EQUALS)
	{
		if(fc->mParameters[0].mU32Value == value)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
		}
		else if((U32)value < fc->mParameters[0].mU32Value)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILED;
		}
	}
	else if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_GREATER_THAN)
	{
		if((U32)value > fc->mParameters[0].mU32Value)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSED;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;
		}
	}

	return result_type;
}

FRIEND_CHALLENGE_RESULT_TYPE C_FriendChallenges::GetParamResult(float value)
{
	FRIEND_CHALLENGE_RESULT_TYPE result_type = FRIEND_CHALLENGE_RESULT_TYPE_FAILED;

	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_LESS_THAN)
	{
		if(value < fc->mParameters[0].mFloatValue)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILED;
		}
	}
	else if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_EQUALS)
	{
		if(fc->mParameters[0].mFloatValue == value)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
		}
		else if(value < fc->mParameters[0].mFloatValue)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILED;
		}
	}
	else if(fc->mParameters[0].mModifier == FRIEND_CHALLENGE_MODIFIER_GREATER_THAN)
	{
		if(value > fc->mParameters[0].mFloatValue)
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_PASSING;
		}
		else
		{
			result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILING;
		}
	}

	return result_type;
}

FRIEND_CHALLENGE_RESULT_TYPE C_FriendChallenges::AttemptingParamResult(UI::C_WideString64 *score_string)
{
	FRIEND_CHALLENGE_RESULT_TYPE result_type=FRIEND_CHALLENGE_RESULT_TYPE_FAILED;

	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	//no param set, so we've won :)
	if(fc->mParameters[0].mParamType == FRIEND_CHALLENGE_PARAM_TYPE_NONE)
	{
		return FRIEND_CHALLENGE_RESULT_TYPE_PASSED;
	}

	switch(fc->mParameters[0].mParamType)
	{
		case FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT:
			{
				int shunts=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				result_type=GetParamResult(shunts);

				if(score_string)
				{
					score_string->Format(L"%d", shunts);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT:
			{
				int barges=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BARGE];
				result_type=GetParamResult(barges);

				if(score_string)
				{
					score_string->Format(L"%d", barges);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT:
			{
				int mines=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BOMB];
				result_type=GetParamResult(mines);

				if(score_string)
				{
					score_string->Format(L"%d", mines);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT:
			{
				int blind_fires=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				result_type=GetParamResult(blind_fires);

				if(score_string)
				{
					score_string->Format(L"%d", blind_fires);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED:
			{
				int nitrous=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_NITROUS];
				result_type=GetParamResult(nitrous);

				if(score_string)
				{
					score_string->Format(L"%d", nitrous);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE:
			{
				bool frs_complete=pLogicActor->GetScoring().mGoldenFanDemandComplete;
				int frs=pLogicActor->GetScoring().mGoldenFanDemandScore;
				result_type=GetParamResult(frs);

				if(score_string)
				{
					if(frs_complete)
					{
						score_string->Format(L"%d", frs);
					}
					else
					{
						score_string->Format(L"--");
					}
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME:
			{
				float race_time=pLogicActor->GetStatistics().mTotalRaceTime;
				result_type=GetParamResult(race_time);

				if(score_string)
				{
					score_string->Format(L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(race_time));
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED:
			{
				int fans=pLogicActor->GetScoring().mFinalScoreForUI;
				result_type=GetParamResult(fans);

				if(score_string)
				{
					score_string->Format(L"%d", fans);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY:
			{
				int bolt_acc=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE] ? (int)(((float)pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE] / (float)pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE]) * 100) : 0;
				result_type=GetParamResult(bolt_acc);

				if(score_string)
				{
					score_string->Format(L"%d~~/p~~", bolt_acc);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED:
			{
				int wrecks=pLogicActor->GetScoring().mDestructionWrecks;
				result_type=GetParamResult(wrecks);

				if(score_string)
				{
					score_string->Format(L"%d", wrecks);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED:
			{
				int wrecks=pLogicActor->GetScoring().mDestructionBlueWrecks;
				result_type=GetParamResult(wrecks);

				if(score_string)
				{
					score_string->Format(L"%d", wrecks);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED:
			{
				int wrecks=pLogicActor->GetScoring().mDestructionRedWrecks;
				result_type=GetParamResult(wrecks);

				if(score_string)
				{
					score_string->Format(L"%d", wrecks);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED:
			{
				int wrecks=pLogicActor->GetScoring().mDestructionYellowWrecks;
				result_type=GetParamResult(wrecks);

				if(score_string)
				{
					score_string->Format(L"%d", wrecks);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED:
			{
				int wrecks=pLogicActor->GetScoring().mDestructionGreenWrecks;
				result_type=GetParamResult(wrecks);

				if(score_string)
				{
					score_string->Format(L"%d", wrecks);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED:
			{
				int stop_watches=pLogicActor->GetStatistics().mStopwatchPickupsCollected;
				result_type=GetParamResult(stop_watches);

				if(score_string)
				{
					score_string->Format(L"%d", stop_watches);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY:
			{
				float agressive_pickups_fired=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_fired+=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_fired+=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_fired+=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BARGE];
				agressive_pickups_fired+=pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BOMB];
				
				float agressive_pickups_hit=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BARGE];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BOMB];

				int pu_acc=agressive_pickups_fired ? int((agressive_pickups_hit/agressive_pickups_fired) * 100) : 0;
				result_type=GetParamResult(pu_acc);

				if(score_string)
				{
					score_string->Format(L"%d~~/p~~", pu_acc);
				}
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS:
			{
				int agressive_pickups_hit=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BARGE];
				agressive_pickups_hit+=pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BOMB];

				result_type=GetParamResult(agressive_pickups_hit);

				if(score_string)
				{
					score_string->Format(L"%d", agressive_pickups_hit);
				}
			}
			break;
	}

	return result_type;
}

bool C_FriendChallenges::GetCriteriaResults( UI::C_WideString512* title, UI::C_WideString512* info_you, UI::C_WideString512* info_them )
{
	bool passed = false;

	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	switch(fc->mType)
	{
		case FRIEND_CHALLENGE_TYPE_RACE:
			{
				passed = ((U32)results.mRaceStatistics.mFansEarned > fc->mU32Criteria);

				if(title)
					title->Format( UI::C_TextStringID("FDE_CRITERIA_FANS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("HUD_FANS_STRING").String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(results.mRaceStatistics.mFansEarned) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(fc->mU32Criteria) );
			}
			break;

		case FRIEND_CHALLENGE_TYPE_FAN_DEMAND:
			{
				passed = ((U32)results.mRaceStatistics.mFanDemandsComplete > fc->mU32Criteria);

				if(title)
					title->Format( UI::C_TextStringID("FDE_FAN_DEMANDS_NEEEDED").String() );
				if(info_you)
					info_you->Format( L"%d", results.mRaceStatistics.mFanDemandsComplete );		// TODO - properly if this is revived!
				if(info_them)
					info_them->Format( L"%d", fc->mU32Criteria );
			}
			break;

		case FRIEND_CHALLENGE_TYPE_DESTRUCTION:
			{
				passed = ((U32)results.mRaceStatistics.mDestruction_NumPoints > fc->mU32Criteria);

				if(title)
					title->Format( UI::C_TextStringID("FDE_POINTS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("SP_RESULT_DESTRUCTION").String(), results.mRaceStatistics.mDestruction_NumPoints );
				if(info_them)
					info_them->Format( L"%d", fc->mU32Criteria );
			}
			break;

		case FRIEND_CHALLENGE_TYPE_CHECKPOINT:
			{
				passed = (results.mRaceStatistics.mTotalRaceTime > fc->mFloatCriteria);

				if(title)
					title->Format( UI::C_TextStringID("FDE_TIME_LEFT_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("SP_RESULT_CHECKPOINT").String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(results.mRaceStatistics.mTotalRaceTime) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(fc->mFloatCriteria) );
			}
			break;

		case FRIEND_CHALLENGE_TYPE_BOSS:
			{
				passed = (results.mRaceStatistics.mTotalRaceTime < fc->mFloatCriteria);

				if(title)
					title->Format( UI::C_TextStringID("FDE_TIME_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("SP_RESULT_TIME_FD").String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(results.mRaceStatistics.mTotalRaceTime) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(fc->mFloatCriteria) );
			}
			break;
	}

	return passed;
}

bool C_FriendChallenges::GetParamResults( UI::C_WideString512* title, UI::C_WideString512* info_you, UI::C_WideString512* info_them )
{
	bool passed = false;

	FRIEND_CHALLENGE_RESULT_TYPE result_type = FRIEND_CHALLENGE_RESULT_TYPE_MAX;

	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	switch(fc->mParameters[0].mParamType)
	{
		case FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT:
			{
				int shunts=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				result_type=GetParamResult(shunts);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_SHUNTS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_SHUNTS_HIT_YOU").String(), shunts );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT:
			{
				int barges=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				result_type=GetParamResult(barges);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_BARGES_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_BARGES_HIT_YOU").String(), barges );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT:
			{
				int mines=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];
				result_type=GetParamResult(mines);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_MINES_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_MINES_HIT_YOU").String(), mines );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT:
			{
				int blind_fires=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				result_type=GetParamResult(blind_fires);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_BOLTS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_BLIND_FIRES_HIT_YOU").String(), blind_fires );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED:
			{
				int var=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_NITROUS];
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_NITROUS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_NITROUS_USED_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE:
			{
				int var=results.mRaceStatistics.mGoldenFanDemandScore;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_FANRUN_SCORE_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_S_FANS").String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas( fc->mParameters[0].mU32Value ) );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME:
			{
				float var=results.mCars[0].mStatistics.mTotalRaceTime;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_RACE_TIME_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("SP_RESULT_TIME_FD").String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(var) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(fc->mParameters[0].mFloatValue) );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED:
			{
				int var=results.mRaceStatistics.mFansEarned;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_FANS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_S_FANS").String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ) );
				if(info_them)
					info_them->Format( L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas( fc->mParameters[0].mU32Value ) );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY:
			{
				int var=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE] ? int(((float)results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE] / (float)results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE]) * 100) : 0;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_BOLT_ACC_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_ACC_YOU").String(), var );
				if(info_them)
					info_them->Format( UI::C_TextStringID("FDE_ACC").String(), fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED:
			{
				int var=results.mRaceStatistics.mDestruction_TotalCarsWrecked;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_WRECKS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_WRECKS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED:
			{
				int var=results.mRaceStatistics.mDestruction_VHardCarsWrecked;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_BLUE_CAR_WRECKS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_WRECKS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED:
			{
				int var=results.mRaceStatistics.mDestruction_HardCarsWrecked;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_RED_CAR_WRECKS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_WRECKS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED:
			{
				int var=results.mRaceStatistics.mDestruction_MediumCarsWrecked;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_YELLOW_CAR_WRECKS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_WRECKS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED:
			{
				int var=results.mRaceStatistics.mDestruction_EasyCarsWrecked;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_GREEN_CAR_WRECKS_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_WRECKS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED:
			{
				int var=results.mCars[0].mStatistics.mStopwatchPickupsCollected;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_STOPWATCHES_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_COLLECTED_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;
	
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY:
			{
				float agressive_pickups_fired=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BARGE];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BOMB];
				
				float agressive_pickups_hit=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];

				int var=agressive_pickups_fired ? int((agressive_pickups_hit/agressive_pickups_fired) * 100) : 0;
				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_PICKUP_ACC_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_ACC_YOU").String(), var );
				if(info_them)
					info_them->Format( UI::C_TextStringID("FDE_ACC").String(), fc->mParameters[0].mU32Value );
			}
			break;

		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS:
			{
				float var=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				var+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				var+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				var+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				var+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];

				result_type=GetParamResult(var);

				passed = (result_type >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING);

				if(title)
					title->Format( UI::C_TextStringID("FDE_POWER_UPS_HIT_NEEDED").String() );
				if(info_you)
					info_you->Format( UI::C_TextStringID("FDE_HITS_YOU").String(), var );
				if(info_them)
					info_them->Format( L"%d", fc->mParameters[0].mU32Value );
			}
			break;
	}

	return passed;
}

const WCHAR *C_FriendChallenges::GetPrimaryReChallengeString()
{
	static UI::C_WideString256 str;

	const FriendChallenge *fc=GetAttemptingFriendDemand();

	switch(fc->mType)
	{
		case FRIEND_CHALLENGE_TYPE_RACE:
			str.Format(UI::C_TextStringID("FDE_FANS_MORE_THAN_YOU").String(), mU32Criteria,mU32Criteria-fc->mU32Criteria);
			break;

		case FRIEND_CHALLENGE_TYPE_FAN_DEMAND:
			str.Format(UI::C_TextStringID("FDE_FAN_DEMANDS_MORE_THAN_YOU").String(),mU32Criteria,mU32Criteria-fc->mU32Criteria);
			break;

		case FRIEND_CHALLENGE_TYPE_DESTRUCTION:
			str.Format(UI::C_TextStringID("FDE_POINTS_MORE_THAN_YOU").String(),mU32Criteria,mU32Criteria-fc->mU32Criteria);
			break;

		case FRIEND_CHALLENGE_TYPE_CHECKPOINT:
			{
				UI::C_WideString128 time1(UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mFloatCriteria));
				UI::C_WideString128 time2(UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mFloatCriteria-fc->mFloatCriteria));

				str.Format(UI::C_TextStringID("FDE_TIME_LEFT_MORE_THAN_YOU").String(), time1.Chars(), time2.Chars());
			}
			break;

		case FRIEND_CHALLENGE_TYPE_BOSS:
			{
				UI::C_WideString128 time1(UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mFloatCriteria));
				UI::C_WideString128 time2(UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(fc->mFloatCriteria-mFloatCriteria));

				str.Format(UI::C_TextStringID("FDE_RACE_TIME_MORE_THAN_YOU").String(), time1.Chars(), time2.Chars());
			}
			break;
	}

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetMedalReChallengeString()
{
	static UI::C_WideString256 str;

	const FriendChallenge *fc=GetAttemptingFriendDemand();

	static UI::C_WideString128 str1;
	static UI::C_WideString128 str2;

	if (mMedal==kSP_Gold)
		str1.Format(L"~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~");
	else if (mMedal == kSP_Silver)
		str1.Format(L"~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~");
	else if (mMedal == kSP_Bronze)
		str1.Format(L"~~{brolt}~~ ~~{brolt}~~ ~~{brolt}~~");
	else
		str1.Format(L"No Medal");

	if(fc->mMedal == kSP_Gold)
		str2.Format(L"~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~ ~~{gollt}~~");
	else if(fc->mMedal == kSP_Silver)
		str2.Format(L"~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~ ~~{sillt}~~");
	else if(fc->mMedal == kSP_Bronze)
		str2.Format(L"~~{brolt}~~ ~~{brolt}~~ ~~{brolt}~~");

	if(mMedal == fc->mMedal)
		str.Format(UI::C_TextStringID("FDE_MEDAL_SAME_AS_YOU").String(), str1.Chars(), str2.Chars() );
	else
		str.Format(UI::C_TextStringID("FDE_MEDAL_BETTER_THAN_YOU").String(), str1.Chars(), str2.Chars() );

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetSecondaryTitleReChallengeString()
{
	static UI::C_WideString128 str;

	const FriendChallenge *fc=GetAttemptingFriendDemand();

	str.Format(GetParamTitleString(0,-1,fc));

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetSecondarySwitchReChallengeString()
{
	static UI::C_WideString128 str;

	const FriendChallenge *fc=GetAttemptingFriendDemand();

	str.Format(GetParamSwitchString(0,-1,fc));

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetStoreName(int index)
{
	const FriendChallenge *fc=&mFriendDemandStore[index];
	return fc->mName;
}

const WCHAR *C_FriendChallenges::GetStoreDesc(int index)
{
	const FriendChallenge *fc=&mFriendDemandStore[index];
	return fc->mDescription;
}

const WCHAR *C_FriendChallenges::GetStoreSender(int index)
{
	const FriendChallenge *fc=&mFriendDemandStore[index];
	return fc->mChallenger;
}

const WCHAR *C_FriendChallenges::GetStoreAttempts(int index)
{
	const FriendChallenge *fc=&mFriendDemandStore[index];

	static UI::C_WideString64 str;

	if (fc->mAttempts==1)
		str.Format( UI::C_TextStringID( "FDE_STORE_ATTEMPT" ).String(), fc->mAttempts );
	else
		str.Format( UI::C_TextStringID( "FDE_STORE_ATTEMPTS" ).String(), fc->mAttempts );

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetStoreTimeSent(int index)
{
	const FriendChallenge *fc=&mFriendDemandStore[index];

	const Time::C_DecomposedTime time( fc->mDate );

	static UI::C_WideString64 str;

	if(fc->mReChallenge)
		str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_SENT_RE" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );
	else
		str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_SENT" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );

	return str.Chars();
}

const WCHAR *C_FriendChallenges::GetSecondaryVarReChallengeString()
{
	static UI::C_WideString128 str;

	const FriendChallenge *fc=GetAttemptingFriendDemand();

	int param=-1;
	for(int a=0;a<MAX_FRIEND_DEMAND_INTERNAL_PARAMS;a++)
	{
		if(mParameters[a].mParamType == fc->mParameters[0].mParamType)
		{
			param=a;
		}
	}

	if(param == -1)
	{
		BIZ_ASSERT_MSG(false, "Unable to find Friend Demand parameter");
	}

	switch(fc->mParameters[0].mModifier)
	{
		case FRIEND_CHALLENGE_MODIFIER_LESS_THAN:
			{
				if(fc->mParameters[0].mParamType == FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME)
				{
					str.Format(L"%s",UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mParameters[param].mFloatValue));
				}
				else
				{
					str.Format(L"%d",mParameters[param].mU32Value);
				}
			}
			break;
		case FRIEND_CHALLENGE_MODIFIER_EQUALS:
			{
				if(fc->mParameters[0].mParamType == FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME)
				{
					str.Format(L"%s",UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mParameters[param].mFloatValue));
				}
				else
				{
					str.Format(L"%d",mParameters[param].mU32Value);
				}
			}
			break;
		case FRIEND_CHALLENGE_MODIFIER_GREATER_THAN:
			{
				if(fc->mParameters[0].mParamType == FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME)
				{
					str.Format(L"%s",UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(mParameters[param].mFloatValue));
				}
				else
				{
					str.Format(L"%d",mParameters[param].mU32Value);
				}
			}
			break;
	};

	return str.Chars();
}

int C_FriendChallenges::GetStatSlot(int friend_index)
{
	bdOnlineUserID id;
	
#ifdef PLATFORM_PS3
	SysProfile::BaseProfile *prof=C_FriendsManager::Instance().GetFriendProfileFromID(friend_index);
	if(prof)
		id=BLUR_USER_ID(prof->m_Uid);
#else
	SysProfile::BaseProfile *prof=C_FriendsManager::Instance().GetFriendProfileFromID(friend_index);
	if(prof)
		id=prof->m_Uid;
#endif

	if(prof == NULL)
		return -1;

	int first_available=-1;

	for(int b=0;b<MAX_FRIEND_DEMAND_STATS;b++)
	{
		if(mFriendDemandStatsStore[b].mUserId == 0 && first_available == -1)
		{
			first_available=b;
		}

		if(mFriendDemandStatsStore[b].mUserId == id)
		{
			return b;
		}
	}

	//if we've got here then this is the first stat for this friend
	
	if(first_available == -1)		//-1 means we're full, clear out some padding
	{
		first_available=ClearStatsForRemovedFriends();
	}

	mFriendDemandStatsStore[first_available].Clear();
	mFriendDemandStatsStore[first_available].mUserId = id;
	
	return first_available;
}

int C_FriendChallenges::ClearStatsForRemovedFriends()
{
	for(int a=0;a<MAX_FRIEND_DEMAND_STATS;a++)
	{
		for(int b=0;b<C_FriendsManager::Instance().GetNumFriends();b++)
		{
			bdOnlineUserID id;

#ifdef PLATFORM_PS3
			id=BLUR_USER_ID(C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->m_Uid);
#else
			id=C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->m_Uid;
#endif

			bool still_friend=false;
			if(mFriendDemandStatsStore[a].mUserId == id)
			{
				still_friend=true;
			}

			if(!still_friend)
			{
				mFriendDemandStatsStore[a].Clear();
				return a;
			}
		}
	}

	BIZ_ASSERT_FAIL("Something really bad has happened, you seem to have more than a 100 friends");

	//should never get here, the above assert is really bad.
	return MAX_FRIEND_DEMAND_STATS-1;
}

void C_FriendChallenges::AcceptDemand(int friend_slot)
{
	int slot=GetStatSlot(friend_slot);

	if(slot == -1)
		return;

	mFriendDemandStatsStore[slot].mSent++;

	mNeedToUploadStats=true;
}

void C_FriendChallenges::UploadStats()
{
	if(mNeedToUploadStats)
	{
		if(C_KeyArchiveManager::Instance().GetCurrentTaskType() == KEY_ARCHIVE_TASK_TYPE_NOTHING)
		{
			bool gotDemonwareID=Game::C_KeyArchiveManager::Instance().SetUserIndex(0);	//need to get proper index

			if(gotDemonwareID)
			{
				int num_stats=0;

				Game::C_KeyArchiveManager::Instance().ClearKeyArchiveToWrite();

				int index=0;
				for(int b=0;b<MAX_FRIEND_DEMAND_STATS;b++)
				{
					if(mFriendDemandStatsStore[b].mUserId != 0)
					{
						//user id
						C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index), mFriendDemandStatsStore[b].mUserId);
						index++;

						bdInt64 stat1=(bdInt64(mFriendDemandStatsStore[b].mSent)<<32) + mFriendDemandStatsStore[b].mCompleted;
						C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index), stat1);
						index++;

						bdInt64 stat2=(bdInt64(mFriendDemandStatsStore[b].mKOs)<<32) + mFriendDemandStatsStore[b].mMostAttempts;
						C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index), stat2);
						index++;

						bdInt64 stat3=(bdInt64(mFriendDemandStatsStore[b].mSpare)<<32) + mFriendDemandStatsStore[b].mTotalAttempts;
						C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS+index), stat3);
						index++;

						num_stats++;
					}
				}

				C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_FRIEND_DEMAND_STATS_NUMBER, num_stats );

				Game::C_KeyArchiveManager::Instance().WriteKeyArchive();

				mNeedToUploadStats=false;
			}
		}
	}
}

void C_FriendChallenges::FinishedAttempt(int passed)
{
	int slot=GetStatSlot(mAttemptingFriend);

	if(slot != -1)
	{
		if(GetAttemptingFriendDemand()->mAttempts > mFriendDemandStatsStore[slot].mMostAttempts)
			mFriendDemandStatsStore[slot].mMostAttempts=GetAttemptingFriendDemand()->mAttempts;
		
		mNeedToUploadStats=true;
	}

	if(passed)
	{
		if(slot != -1)
		{
			mFriendDemandStatsStore[slot].mCompleted++;

			if(GetAttemptingFriendDemand()->mAttempts == 1)
				mFriendDemandStatsStore[slot].mKOs++;
		}

		//wipe the completed demand
		mFriendDemandStore[mAttempting].mEventID=0;

		if(GetAttemptingFriendDemand()->mReChallenge)
		{
			Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), Game::BLUR_AVATAR_AWARD_RE_CHALLENGE_TEE);
		}
	}
}

void C_FriendChallenges::Sort(FRIEND_CHALLENGE_SORT_TYPE type)
{
	mFriendChallengeSorted.Clear();

	for(int a=0;a<Game::C_FriendsManager::Instance().GetNumFriends();a++)
	{
		FriendChallengeSort fcs;
		
		fcs.mFriendIndex=C_FriendsManager::Instance().GetFriendIDFromIndex(a);
		fcs.mName = NULL;

		int count=0;
		if(type == FRIEND_CHALLENGE_SORT_TYPE_NEW)
		{
			for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
			{
				Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
				const bdOnlineUserID fc_id=Game::C_MessagingManager::Instance().GetMailHeader(i)->m_senderID;

				SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromIndex(a);

#ifdef PLATFORM_PS3
				bool identical_ids=fc_id == BLUR_USER_ID(prof->m_Uid);
				if(!wcscmp(fc->mChallenger, prof->Name()) || identical_ids)
#else
				bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, fc_id) ? true : false;
				if(!wcsicmp(fc->mChallenger, prof->Name()) || identical_ids)
#endif
				{
					count++;
				}
			}
		}
		else if(type == FRIEND_CHALLENGE_SORT_TYPE_ACTIVE)
		{
			for(int i=0;i<Game::MAX_FRIEND_DEMANDS;i++)
			{
				SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromIndex(a);
#ifdef PLATFORM_PS3
				bool identical_ids=mFriendDemandUserIDStore[i] == BLUR_USER_ID(prof->m_Uid);
				if(mFriendDemandStore[i].mEventID != 0 && (!wcscmp(mFriendDemandStore[i].mChallenger, prof->Name()) || identical_ids ) )
#else
				bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, mFriendDemandUserIDStore[i]) ? true : false;
				if(mFriendDemandStore[i].mEventID != 0 && (!wcsicmp(mFriendDemandStore[i].mChallenger, prof->Name()) || identical_ids ) )
#endif
				{
					count++;
				}
			}
		}
		else if(type == FRIEND_CHALLENGE_SORT_TYPE_ALPHA)
		{
			fcs.mName=Game::C_FriendsManager::Instance().GetFriendProfileFromIndex(a)->Name();
		}

		fcs.mValue=count;
		mFriendChallengeSorted.PushBack(fcs);
	}

	if(type == FRIEND_CHALLENGE_SORT_TYPE_ALPHA)
		std::sort( mFriendChallengeSorted.Begin(), mFriendChallengeSorted.End(), FriendChallengeAlphaSorter() );
	else
		std::sort( mFriendChallengeSorted.Begin(), mFriendChallengeSorted.End(), FriendChallengeSorter() );
}

int C_FriendChallenges::GetNumPickups(E_PICKUP_TYPE type)
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	int num=C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mLivePickups[type] * levelstate.mLaps;

	Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mPickupPositionActorMap.Begin();
	while(it != C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mPickupPositionActorMap.End())
	{
		Actor::C_ActorHandle handle = (*it).GetValue();
		::Actors::PickupPositionActor* pActor = (::Actors::PickupPositionActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(handle);
		if(pActor && pActor->GetEnumPickupType() == E_PICKUP_TYPE_FAN)
		{
			const ::Actors::FanDemandPositionCreationData *fd_data = ((::Actors::FanDemandPositionActor *)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(handle))->GetCreationData();

			if(fd_data)
			{
				if(fd_data->mFanDemand.gift_slot1 && type == fd_data->mFanDemand.slot1)
				{
					num+=levelstate.mLaps;
				}

				if(fd_data->mFanDemand.gift_slot2 && type == fd_data->mFanDemand.slot2)
				{
					num+=levelstate.mLaps;
				}

				if(fd_data->mFanDemand.gift_slot3 && type == fd_data->mFanDemand.slot3)
				{
					num+=levelstate.mLaps;
				}
			}
		}

		it++;
	}

	if(type == E_PICKUP_TYPE_NITROUS)
	{
		if(levelstate.mCars[0].mLoadOuts.Size() > 0)
		{
			if(levelstate.mCars[0].mLoadOuts[0] == UI::E_MultiplayerVehicleAbilities_Boss_FansNitroGift)
			{
				num += 1000000;	//arbitray big number as in theory there is no max cap (as its skill based)
			}
		}
	}

	return num;
}

int C_FriendChallenges::GetNumDestructionVehicles(E_DestructionMode_AI_Level type)
{
	int num=0;

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );

	if(pBaseEvent)
	{
		AmaxRaceModeDestruction *data=(AmaxRaceModeDestruction*)pBaseEvent->event->raceMode;

		if(data)
		{
			for(int i=0;i<data->waves.count;i++)
			{
				DestructionMode_Wave* wave = &data->waves[i];

				if(wave->waveAI.slot1 == type)
					num++;

				if(wave->waveAI.slot2 == type)
					num++;

				if(wave->waveAI.slot3 == type)
					num++;

				if(wave->waveAI.slot4 == type)
					num++;
			}
		}
	}

	return num;
}

FRIEND_CHALLENGE_PARAM_USE C_FriendChallenges::GetParamUse(FRIEND_CHALLENGE_PARAM_TYPE param)
{
	const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	switch(param)
	{
		case FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_SHUNT);

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_BARGE);

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_BOMB);

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_BLINDFIRE) * 3; //need to check for 4 bolt mod as well

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;

			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_NITROUS];
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_NITROUS);

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE:
			{
				int score=results.mRaceStatistics.mGoldenFanDemandScore;

				if(score == 0)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				else
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME:
			{
				return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED:
			{
				int score=results.mRaceStatistics.mFansEarned;

				if(score == 0)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				else
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY:
			{
				int acc=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE] ? int(((float)results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE] / (float)results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE]) * 100) : 0;

				if(acc == 0)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				else if(acc == 100)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
				else
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED:
			{
				int wrecked=results.mRaceStatistics.mDestruction_TotalCarsWrecked;

				if(wrecked == 0)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				else
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED:
			{
				int wrecked=results.mRaceStatistics.mDestruction_VHardCarsWrecked;

				if(GetNumDestructionVehicles(DM_AI__VHard))
				{
					if(wrecked == 0)
						return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
					else
						return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				}
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED:
			{
				int wrecked=results.mRaceStatistics.mDestruction_HardCarsWrecked;

				if(GetNumDestructionVehicles(DM_AI__Hard))
				{
					if(wrecked == 0)
						return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
					else
						return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				}
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED:
			{
				int wrecked=results.mRaceStatistics.mDestruction_MediumCarsWrecked;

				if(GetNumDestructionVehicles(DM_AI__Medium))
				{
					if(wrecked == 0)
						return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
					else
						return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				}
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED:
			{
				int wrecked=results.mRaceStatistics.mDestruction_EasyCarsWrecked;

				if(GetNumDestructionVehicles(DM_AI__Easy))
				{
					if(wrecked == 0)
						return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
					else
						return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
				}
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED:
			{
				int pickups=results.mCars[0].mStatistics.mStopwatchPickupsCollected;
				int max_pickups=GetNumPickups(E_PICKUP_TYPE_STOPWATCH);

				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY:
			{
				float agressive_pickups_fired=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BARGE];
				agressive_pickups_fired+=results.mCars[0].mStatistics.mNumPerksFired[E_PICKUP_TYPE_BOMB];
				
				float agressive_pickups_hit=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				agressive_pickups_hit+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];

				int acc=agressive_pickups_fired ? int((agressive_pickups_hit/agressive_pickups_fired) * 100) : 0;

				if(acc == 0)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				else if(acc == 100)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS;
				else
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
			}
			break;
		case FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS:
			{
				int pickups=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHOCK];
				pickups+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_SHUNT];
				pickups+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE];
				pickups+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BARGE];
				pickups+=results.mCars[0].mStatistics.mNumPerksHit[E_PICKUP_TYPE_BOMB];

				int max_pickups=GetNumPickups(E_PICKUP_TYPE_SHOCK);
				max_pickups+=GetNumPickups(E_PICKUP_TYPE_SHUNT);
				max_pickups+=GetNumPickups(E_PICKUP_TYPE_BLINDFIRE);
				max_pickups+=GetNumPickups(E_PICKUP_TYPE_BARGE);
				max_pickups+=GetNumPickups(E_PICKUP_TYPE_BOMB);
				
				if(pickups > 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN;
				
				if(pickups == 0 && pickups < max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN;
				
				if(pickups > 0 && pickups == max_pickups)
					return FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS;
			}
			break;
	};

	return FRIEND_CHALLENGE_PARAM_USE_NONE;
}

void C_FriendChallenges::FriendsUpdated()
{
	for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
	{
		//valid store but invalid user ID
		if(mFriendDemandStore[a].mEventID != 0 && mFriendDemandUserIDStore[a] == 0)
		{
			for(int b=0;b<C_FriendsManager::Instance().GetNumFriends();b++)
			{
#ifdef PLATFORM_PS3
				if(!wcscmp(mFriendDemandStore[a].mName,C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->Name()))
				{
					mFriendDemandUserIDStore[a]=BLUR_USER_ID(C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->m_Uid);
#else
				if(!wcsicmp(mFriendDemandStore[a].mName,C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->Name()))
				{
					mFriendDemandUserIDStore[a]=C_FriendsManager::Instance().GetFriendProfileFromIndex(b)->m_Uid;
#endif
				}
			}
		}
	}
}

bool C_FriendChallenges::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case GameMessaging::E_GameMsg_Update:
			//mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			//Update();
			break;

		case Game::E_GameflowMessage_ResultsCalculated:
			CreateFriendDemandPostRace();
			break;

		case Game::E_GameflowMessage_StartUILoad:
			mAttempting=-1;
			mAttemptingFriend=-1;
			break;

		default :
			handledMessage = false;
			break;
	}

	return handledMessage;
}

}; // namespace Game
