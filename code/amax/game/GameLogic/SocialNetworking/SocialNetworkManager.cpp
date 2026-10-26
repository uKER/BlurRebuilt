#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <ui/Game/UILocaliser.h>

#include <GameLogic\ScoreboardManager.h>
#include <Network\Core\NetScoreboards.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>
#include "GameLogic/MultiplayerChallengeSystem.h"

#include <Ui\game\Multiplayer\MultiplayerCache.h>

#include <ui\Game\Profiles\ProfileInterface.h>

#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <GameLogic\Structures\MpResults.h>

#include <FriendsChallenges/MessagingManager.h>
#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include "GameLogic/SinglePlayer/SinglePlayer.h"

#include "GameLogic/Structures/MultiplayerAwards.h"
#include <GameLogic\GameModes\GameModeBase.h>
#include <GameLogic\GameModes\GameModeMultiplayer.h>

#include <GameLogic\StickerManager.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <GameLogic\SocialNetworking\FriendsManager.h>

#include <GameLogic\AchievementsManager.h>
#include <Game\achievements\AchievementSystem.h>

#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

namespace Game
{

void AmaxSocialNetworkItem::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	out << (int &)mType;
}

void AmaxSocialNetworkItem::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	in >> (int &)mType;
}

C_SocialNetworkManager C_SocialNetworkManager::s_instance;

void C_SocialNetworkManager::Init()
{

}

void C_SocialNetworkManager::Update()
{

}

void C_SocialNetworkManager::Clear()
{
	//clear the race history
	mRaceHistoryStoreIndex=0;
	for(int a=0;a<MAX_SOCIAL_NETWORK_RACE_HISTORY;a++)
	{
		mRaceHistoryStore[a].mNumCars=0;
	}

	mSocialStore.Clear();
}

const WCHAR *C_SocialNetworkManager::GetBlurbServerName(int index)
{
	bdMailBody* body = C_MessagingManager::Instance().GetMailBody(index);

	if(!body)
		return L"";

	FriendChallenge *fc=(FriendChallenge *)body->m_mailBody;

	if(!fc)
		return L"";

	return fc->mName;
}

const WCHAR *C_SocialNetworkManager::GetBlurbServerDesc(int index)
{
	bdMailBody* body = C_MessagingManager::Instance().GetMailBody(index);

	if(!body)
		return L"";

	FriendChallenge *fc=(FriendChallenge *)body->m_mailBody;

	if(!fc)
		return L"";

	return fc->mDescription;
}

const WCHAR *C_SocialNetworkManager::GetBlurbServerSender(int index)
{
	bdMailBody* body = C_MessagingManager::Instance().GetMailBody(index);

	if(!body)
		return L"";

	FriendChallenge *fc=(FriendChallenge *)body->m_mailBody;

	if(!fc)
		return L"";

	return fc->mChallenger;	
}

const WCHAR *C_SocialNetworkManager::GetBlurbServerDaysLeft(int index)
{
	bdMailBody* body = C_MessagingManager::Instance().GetMailBody(index);

	if(!body)
		return L"";

	FriendChallenge *fc=(FriendChallenge *)body->m_mailBody;

	if(!fc)
		return L"";

	const Time::C_DecomposedTime time( fc->mDate );
	const Time::C_DecomposedTime now( Time::GetCurrentTime() );

	char leap_year=0;
	if ( ( time.m_year % 400 ) == 0 )
		leap_year=1;
	
	if ( ( time.m_year % 100 ) == 0 )
		leap_year=0;
	
	if ( ( time.m_year % 4 ) == 0 )
		leap_year=1;

	int days_left=(time.m_yday + 7);

	//handle the wrap
	if( (days_left > (365 + leap_year) ) && (now.m_yday < 8) )
	{
		days_left=(365 + leap_year) - time.m_yday ;		//get how many days till the end of year
		days_left+=now.m_yday;							//add on next years days
	}
	else
	{
		days_left-=now.m_yday;
	}

	static UI::C_WideString64 str;

	if(days_left < 0 || days_left > 7)
	{
		str.Format( UI::C_TextStringID( "MP_UNKNOWN" ).String() );
	}
	else
	{
		if (days_left==1)
			str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_LEFT_ONE" ).String(), days_left );
		else
			str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_LEFT" ).String(), days_left );
	}

	return str.Chars();
}

const WCHAR *C_SocialNetworkManager::GetBlurbServerTimeSent(int index)
{
	bdMailBody* body = C_MessagingManager::Instance().GetMailBody(index);

	if(!body)
		return L"";

	FriendChallenge *fc=(FriendChallenge *)body->m_mailBody;

	if(!fc)
		return L"";

	const Time::C_DecomposedTime time( fc->mDate );

	static UI::C_WideString64 str;

	if(fc->mReChallenge)
		str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_SENT_RE" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );
	else
		str.Format( UI::C_TextStringID( "FDE_SERVER_TIME_SENT" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );

	return str.Chars();
}

const WCHAR *C_SocialNetworkManager::GetBlurbString(int index)
{
	UI::C_WideString256 str;

	str.Clear();

	AmaxSocialNetworkingItem *gd_sni=GetSocialStoryData(mSocialStore[index].mType);

	static UI::C_TextStringID id( gd_sni->blurb.blurb_text.id );

	str.Format( L"%s", id.String() );
	
	return str.Chars();
}

int C_SocialNetworkManager::GetRemainingTweetSpace()
{
	int len=mCurrentItemText.Length() + wcslen(UI::C_TextStringID("CMN_TWEET_STRING").String());

	return MAX_TWEET_LENGTH-len;
}

const WCHAR *C_SocialNetworkManager::GetTweetString(int index)
{
	mTweetString.Clear();
	
	if(mCurrentMessage.Length())
		mTweetString.Format( UI::C_TextStringID("CMN_TWEET_STRING").String(), mCurrentMessage.Chars(), mCurrentItemText.Chars() );
	else
		mTweetString.Format( L"%s", mCurrentItemText.Chars() );
	
	return mTweetString.Chars();
}

void C_SocialNetworkManager::AddOneOff(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 param)
{
	if(mSocialStore.Size() >= MAX_SOCIAL_NETWORK_ITEMS)
		return;

	AmaxSocialNetworkItem sni;

	for(int a=0;a<C_GameData::Instance().mSocialStories.Number();a++)
	{
		AmaxSocialNetworkingItem *gd_sni=C_GameData::Instance().mSocialStories[a];

		if(gd_sni->type == type)
		{
			if(type == AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR)
			{
				sni.mType=type;
				mSocialStore.PushBack(sni);
				return;
			}
		}
	}
}

void C_SocialNetworkManager::AddIncrement(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 id, int value, bool suffix)
{

}

void C_SocialNetworkManager::AddShare(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 id, int value, bool suffix)
{

}

void C_SocialNetworkManager::PostRace(bool had_passed,bool multiplayer)
{
	if(!multiplayer)
		return;

	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	const bool teamGame = levelState.mTeams;

	AmaxSocialNetworkRaceStore& raceHistory = mRaceHistoryStore[mRaceHistoryStoreIndex];

	raceHistory.mNumCars	= levelState.mNumCars;
	raceHistory.mDate		= Time::GetCurrentTime();
	raceHistory.mRoute		= levelState.mRoute;
	raceHistory.mRaceType	= levelState.mUiRaceType;

	//store the grid away
	for(U32 a=0;a<levelState.mNumCars;a++)
	{
		NetVehicleData result;
		C_MpResults::Instance().GetPlayerResults(a,result);

		AmaxSocialNetworkRaceVehicleStore& vehicleHistory = raceHistory.mVehicles[a];
		wcsncpy(vehicleHistory.mName,levelState.mCars[a].mNameAboveCar,32);
		vehicleHistory.mVehicleId = levelState.mCars[a].mCarID;

#if defined(PLATFORM_PC)
		// [Blur2PC] on PC m_Uid is already a bdOnlineUserID; the handle.data form below is the PS3 SceNpId
		vehicleHistory.mSystemID=levelState.mCars[a].mSystemPlayerId.m_Uid;
#else
		vehicleHistory.mSystemID=BLUR_USER_ID(levelState.mCars[a].mSystemPlayerId.m_Uid);
#endif
		vehicleHistory.mPosition=result.m_Position;


		vehicleHistory.mNetJoinId = levelState.mCars[a].mNetJoinId;
		vehicleHistory.mSysProfile.m_Uid = levelState.mCars[a].mSystemPlayerId.m_Uid;
		vehicleHistory.mRank = levelState.mCars[a].mMultiplayerRank;
		vehicleHistory.mLegend = levelState.mCars[a].mMultiplayerLegendLevel;
		vehicleHistory.mPadIndex =  levelState.mCars[a].mController;

		if(teamGame)
			vehicleHistory.mTeam = levelState.mCars[a].mTeam;
		else
			vehicleHistory.mTeam = -1;

		if(raceHistory.mRaceType == UI::E_RaceType_Destruction)
			vehicleHistory.mStatistic=result.m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];
		else
			vehicleHistory.mStatistic=result.m_Statistics[UI::E_MultiplayerStat_RaceTime];
	}

	// store the primary player's prey and nemesis from this race
	raceHistory.mMyPrey = C_MpResults::Instance().m_LastPreyIndex;
	raceHistory.mNumTimesHitPrey = C_MpResults::Instance().m_NumTimesHitPrey;
	raceHistory.mPreyNetJoinId = C_MpResults::Instance().m_PreyJoinId;
	raceHistory.mPreyUid.m_Uid = C_MpResults::Instance().m_PreyUid.m_Uid;

	
	raceHistory.mMyNemesis = C_MpResults::Instance().m_LastNemesisIndex;
	raceHistory.mNumTimesHitByNemesis = C_MpResults::Instance().m_NumTimesHitByNemesis;
	raceHistory.mNemesisNetJoinId = C_MpResults::Instance().m_NemesisJoinId;
	raceHistory.mNemesisUid.m_Uid = C_MpResults::Instance().m_NemesisUid.m_Uid;


	//store the players race awards
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
	const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

	//clear out the array
	for ( U8 i = 0; i < MAX_SOCIAL_NETWORK_RACE_AWARDS; i++ )
	{
		raceHistory.mPlayerAwards[i]=UI::E_MultiplayerAward_None;
		raceHistory.mPlayerAwardValues[i] = 0.f;
	}

	//now add the awards
	int num=0;
	for ( U8 i = 0; i < awards.NumAwardsDelt() && i < MAX_SOCIAL_NETWORK_RACE_AWARDS; i++ )
	{
		const UI::E_MultiplayerAward type = awardSelection.m_Awards[i];
		F32 value = awardSelection.m_AwardValues[i];

		if(awardSelection.m_Owners[i] == 0)
		{
			raceHistory.mPlayerAwards[num]=type;
			raceHistory.mPlayerAwardValues[num]=value;
			num++;
		}
	}

	mRaceHistoryStoreIndex++;

	//circular buffer, so wrap around
	if(mRaceHistoryStoreIndex == MAX_SOCIAL_NETWORK_RACE_HISTORY)
	{
		mRaceHistoryStoreIndex=0;
	}
}

int C_SocialNetworkManager::GetPrevRaceHistoryStoreIndex()
{
	int returnValue = (mRaceHistoryStoreIndex-1);

	if(returnValue < 0)
	{
		returnValue = (MAX_SOCIAL_NETWORK_RACE_HISTORY -1);
	}

	return returnValue;
}

void C_SocialNetworkManager::StoreDataPreRace()
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	mPreviousRoute=levelState.mRoute;
}

bool C_SocialNetworkManager::OnIncrement(float old_val, float new_val, int increment, int *value)
{
	float old_value=old_val/increment;
	float new_value=new_val/increment;

	int old_step=int(old_value);
	int new_step=int(new_value);

	if(new_step > old_step) //must have passed increment threshold
	{
		*value=(new_step*increment);
		return true;
	}
	
	*value=0;
	return false;
}

AmaxSocialNetworkingItem *C_SocialNetworkManager::GetSocialStoryData(int type)
{
	for(int a=0;a<C_GameData::Instance().mSocialStories.Number();a++)
	{
		if(type == C_GameData::Instance().mSocialStories[a]->type)
		{
			return C_GameData::Instance().mSocialStories[a];
		}
	}

	return NULL;
}

void C_SocialNetworkManager::CreateFromIndex(AMAX_SOCIAL_NETWORK network, U32 index)
{
	if((int)index >= mSocialStore.Size())
		return;

	BIZ_ASSERT(GetSocialStoryData(mSocialStore[index].mType));

	memcpy(&mCurrentItem, GetSocialStoryData(mSocialStore[index].mType), sizeof(AmaxSocialNetworkingItem));
	
	if(network == AMAX_SOCIAL_NETWORK_TWITTER)
	{
		UI::C_TextStringID id( mCurrentItem.tweet.tweet_text.id );
		UI::C_TextStringID singular_id( mCurrentItem.tweet.tweet_text_singular.id );
		CreateStory(-1, id, singular_id);
	}
	else
	{
		UI::C_TextStringID id( mCurrentItem.facebook_story.name.id );
		UI::C_TextStringID singular_id( mCurrentItem.facebook_story.name_singular.id );
		CreateStory(-1, id, singular_id);
	}
}

void C_SocialNetworkManager::Create(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, AMAX_SOCIAL_NETWORK network, U32 index)
{
	BIZ_ASSERT(GetSocialStoryData(type));

	memcpy(&mCurrentItem, GetSocialStoryData(type), sizeof(AmaxSocialNetworkingItem));
	
	if(network == AMAX_SOCIAL_NETWORK_TWITTER)
	{
		UI::C_TextStringID id( mCurrentItem.tweet.tweet_text.id );
		UI::C_TextStringID singular_id( mCurrentItem.tweet.tweet_text_singular.id );
		CreateStory(index, id, singular_id);
	}
	else
	{
		UI::C_TextStringID id( mCurrentItem.facebook_story.name.id );
		UI::C_TextStringID singular_id( mCurrentItem.facebook_story.name_singular.id );
		CreateStory(index, id, singular_id);
	}
}

void C_SocialNetworkManager::CreateStory(U32 index, UI::C_TextStringID id, UI::C_TextStringID singular_id)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	BIZ_ASSERT(pMultiplayerProfile);

	mCurrentItemText.Clear();

	switch(mCurrentItem.type)
	{
		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR:
			mCurrentItemText=id.String();
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_BLUR_EVENT_RATING:
			mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumRacePoints ) );
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_WINS:
			{
				int wins=pMultiplayerProfile->GetMultiplayerStatistics().mNumEventsWon;

				if(wins == 1)
					mCurrentItemText.Format(singular_id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( wins ) );
				else
					mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( wins ) );
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_POWER_UP_HIT_RATE:
			if(pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsFired > 0)
				mCurrentItemText.Format( id.String(), ((float)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsHit / (float)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPickupsFired) * 100.0f );
			else
				mCurrentItemText.Format( id.String(), 0.f);
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_RACES:
			{
				int races=pMultiplayerProfile->GetMultiplayerStatistics().mNumEventsPlayed;
				if(races == 1)
					mCurrentItemText.Format( singular_id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( races ) );
				else
					mCurrentItemText.Format( id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( races ) );
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_PLAYTIME:
			{
				F32 time = ( (F32)pMultiplayerProfile->GetMultiplayerStatistics().mTotalPlaytime / SYS_USEC_PER_SEC );
				mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( time ) );
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_FANS:
			mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( pMultiplayerProfile->GetMultiplayerStatistics().mNumLifetimeFans ) );
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEADERBOARD_POSITION:
			{
				int pos=Game::C_ScoreboardManager::Instance().GetCurrentRowData(index-1).m_Rank;

				mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToPosition(pos), GetScoreboardName());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_ABOVE:
		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_BELOW:
			{
				Scoreboard::RowData rd=Game::C_ScoreboardManager::Instance().GetCurrentRowData(index-1);
				
				int pos=rd.m_Rank;
				int my_pos=-1;

				U32 padNo = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
				SysProfile::FullProfile& profile = UI::C_ProfileInterface::Instance().GetPadProfile( padNo );

				//find you
				for(int a=0; a < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount(); a++)
				{
					Scoreboard::RowData rd=Game::C_ScoreboardManager::Instance().GetCurrentRowData(a);

					if(!wcscmp(rd.m_Userdata.Name(),profile.Name()))
					{
						my_pos=rd.m_Rank;
						break;
					}
				}

				if(abs(pos-my_pos) == 1)
					mCurrentItemText.Format(singular_id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( abs(pos-my_pos) ), rd.m_Userdata.Name(),GetScoreboardName());
				else
					mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( abs(pos-my_pos) ), rd.m_Userdata.Name(),GetScoreboardName());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_PACK_COMPLETE_PERCENTAGE:
			{
				const MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[index];
				if(pPack == NULL)
				{
					mCurrentItemText.Clear();
					return;
				}

				// Get the challenge system associated with the active profile
				const Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();

				UI::C_TextStringID pack_id( pPack->mName.id );

				float a=0.f;
				if(pPack->mType == UI::E_ChallengePack_Car)
				{
					a=challengeSystem.GetCarPackCounter() / (float)challengeSystem.GetCarPackTotal();
				}
				else
				{
					a=challengeSystem.GetPackProgressCounter( pPack->mType ) / (float)pPack->mChallenges.count;
				}
				int b=a*100;

				mCurrentItemText.Format(id.String(), 
											UI::C_UILocaliser::Instance().ConvertNumberToCommas( b ), 
												pack_id.String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_MOD:
			{
				bool isRanked = true; //Only look at ranked mods
				const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( (UI::E_MULTIPLAYER_LOADOUT_TYPE)index);

				const MultiplayerAbilityData* pAbility1 = C_MultiplayerData::Instance().FindAbility( loadOut.slots[0] );
				const MultiplayerAbilityData* pAbility2 = C_MultiplayerData::Instance().FindAbility( loadOut.slots[1] );
				const MultiplayerAbilityData* pAbility3 = C_MultiplayerData::Instance().FindAbility( loadOut.slots[2] );

				mCurrentItemText.Format(id.String(), UI::C_TextStringID( pAbility1->mAbilityName.id ).String(),
														UI::C_TextStringID( pAbility2->mAbilityName.id ).String(),
															UI::C_TextStringID( pAbility3->mAbilityName.id ).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_CAR:
			{
				AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)index ];

				if(pVehicle)
					mCurrentItemText.Format(id.String(), UI::C_TextStringID(pVehicle->loc_string.id).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CAR_CHALLENGE_COMPLETE_PERCENTAGE:
			{
				AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)index ];

				if(pVehicle)
				{
					// Get the challenge system associated with the active profile
					Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();

					float completed=challengeSystem.GetCarChallenge(pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES).GetProgress(true);
					float winner=challengeSystem.GetCarChallenge(pVehicle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER).GetProgress(true);
		
					int b=((completed+winner)/2.f)*100;

					mCurrentItemText.Format(id.String(), 
											UI::C_UILocaliser::Instance().ConvertNumberToCommas( b ), 
												UI::C_TextStringID(pVehicle->loc_string.id).String());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_PLAYERS_RANK:
			{
				int rank=pMultiplayerProfile->GetCurrentLevel()+1;
				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( rank ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEGEND_RANK:
			{
				int legend_rank=pMultiplayerProfile->GetLegendLevel();
				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( legend_rank ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_STATUS:
			{
				int var=0;

				var=SP_GetRank( SP_GetCurrentSaveGame() );

				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_COMPLETION_PERCENTAGE:
			{
				int var=0;

				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				var=((float)SP_CountTotalStars( sg )/(float)SP_CountMaxStars( sg )) * 100;

				mCurrentItemText.Format(id.String(), var);
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_FANS:
			{
				int var=0;

				var=SP_GetFans( SP_GetCurrentSaveGame() );

				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_STARS:
			{
				int var=0;

				var=SP_CountTotalStars( SP_GetCurrentSaveGame() );

				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_CARS:
			{
				int var=SP_GetVehiclesAvailableCount(SP_GetCurrentSaveGame());

				if(var == 1)
					mCurrentItemText.Format(singular_id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
				else
					mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_BOSSES_FINISHED:
			{
				int var=0;

				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				for(int i=0;i<SP_GetTierCount(sg);i++)
				{
					SP_Tier *tier=SP_GetTier(sg,i);

					if(tier && tier->boss)
					{
						if (SP_IsBossDefeated(sg, tier->boss))
							var++;
					}
				}

				if(var == 1)
					mCurrentItemText.Format(singular_id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
				else
					mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas( var ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_PERCENT_COMPLETE:
			{
				int var=0;

				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				SP_Tier* pTier = SP_GetTier( SP_GetCurrentSaveGame(), index-1 );

				if (!pTier)
				{
					mCurrentItemText.Format(id.String(), 0, L"ERROR");
					break;
				}

				var=((float)SP_CountTotalTierStars(sg, pTier)/(float)SP_CountMaxTierStars(sg, pTier)) * 100;
				if( pTier )
					mCurrentItemText.Format(id.String(), var, UI::C_TextStringID(pTier->name.id).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_STARS:
			{
				int var=0;

				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), index );

				if(pEvent)
				{
					SP_EventState state=SP_GetEventState(sg,pEvent);

					if(pEvent->kind == kSP_BossEvent)
					{
						if(state == kSP_Gold)
							var=8;
					}
					else
					{
						if(state == kSP_Bronze)
							var=3;
						if(state == kSP_Silver)
							var=4;
						if(state == kSP_Gold)
							var=5;

						if(SP_IsEventFanParComplete(sg, pEvent))
							var++;

						if(SP_IsEventFanRunComplete(sg, pEvent))
							var++;
					}

					UI::C_WideString64 str;
					str.Format(UI::C_TextStringID("CMN_SN_S_EVENT_D").String() ,UI::C_TextStringID(pEvent->tier->name.id).String(),pEvent->index+1);

					mCurrentItemText.Format(id.String(), var, str.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_GOLDEN_FAN_DEMAND_COMPLETE:
			{
				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), index );

				if(pEvent)
				{
					UI::C_WideString64 str;
					str.Format(UI::C_TextStringID("CMN_SN_S_EVENT_D").String() ,UI::C_TextStringID(pEvent->tier->name.id).String(),pEvent->index+1);

					mCurrentItemText.Format(id.String(), str.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_PAR_COMPLETE:
			{
				SP_SaveGame* sg = SP_GetCurrentSaveGame();
				SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), index );

				if(pEvent)
				{
					UI::C_WideString64 str;
					str.Format(UI::C_TextStringID("CMN_SN_S_EVENT_D").String(), UI::C_TextStringID(pEvent->tier->name.id).String(),pEvent->index+1);

					mCurrentItemText.Format(id.String(), str.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_FINISHING_POSITION:
			{
				U32 route=Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[Game::C_SocialNetworkManager::Instance().mLookingAtRaceHistoryStoreIndex].mRoute;

				AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)route];
				BIZ_ASSERT(pRoute);

				mCurrentItemText.Format(id.String(),UI::C_UILocaliser::Instance().ConvertNumberToPosition(index),UI::C_TextStringID( pRoute->loc_string.id ).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_ABOVE:
			{
				int player_index=-1;

				for(int a=0;a<mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mNumCars;a++)
				{
					if(mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[a].mPosition == index)
					{
						player_index=a;
						break;
					}
				}

				if(player_index != -1)
				{
					int pos=mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mPosition;
					int my_pos=GetMyRaceHistoryPos(mLookingAtRaceHistoryStoreIndex);

					if((pos-my_pos) == 1)
						mCurrentItemText.Format(singular_id.String(),pos-my_pos,mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mName);
					else
						mCurrentItemText.Format(id.String(),pos-my_pos,mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mName);
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_BELOW:
			{
				int player_index=-1;

				for(int a=0;a<mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mNumCars;a++)
				{
					if(mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[a].mPosition == index)
					{
						player_index=a;
						break;
					}
				}

				if(player_index != -1)
				{
					int pos=mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mPosition;
					int my_pos=GetMyRaceHistoryPos(mLookingAtRaceHistoryStoreIndex);

					if((my_pos-pos) == 1)
						mCurrentItemText.Format(singular_id.String(),my_pos-pos,mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mName);
					else
						mCurrentItemText.Format(id.String(),my_pos-pos,mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mVehicles[player_index].mName);
				}			
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RACE_AWARD:
			{
				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

				UI::E_MultiplayerAward award_type=mRaceHistoryStore[mLookingAtRaceHistoryStoreIndex].mPlayerAwards[index];

				const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
				const MultiplayerAwardData* pAward = awards.GetAward(award_type).GetData();

				mCurrentItemText.Format(id.String(), UI::C_TextStringID( pAward->mAwardName.id ).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UPLOADED_PHOTO:
			{
				mCurrentItemText.Format(id.String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_TIER:
			{
				if(SP_Tier* tier = SP_GetTierById(SP_GetCurrentSaveGame(), index))
					mCurrentItemText.Format(id.String(), UI::C_TextStringID( tier->name.id ).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_EVENT:
			{
				if (SP_Event* sp_event = SP_GetEventById(SP_GetCurrentSaveGame(), index))
					mCurrentItemText.Format(id.String(), UI::C_TextStringID( sp_event->event->loc_string.id ).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_RANK:
			{
				mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(index));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_VEHICLE:
			{
				AmaxVehicle *v=C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)index];

				if(v)
					mCurrentItemText.Format(id.String(), UI::C_TextStringID( v->loc_string.id).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_BOSS_VEHICLE:
			{
				AmaxVehicle *v=C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)index];

				if(v)
					mCurrentItemText.Format(id.String(), UI::C_TextStringID( v->loc_string.id).String());
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND:
			{
				const WCHAR *friend_str=UI::C_TextStringID("CMN_SN_SOME_FRIENDS").String();
				if(Game::C_FriendChallenges::Instance().NumFriendsAdded() == 1)
				{
					for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
					{
						int slot=Game::C_FriendChallenges::Instance().GetFriendSlot(a);
						if(slot != -1)
						{
							friend_str=Game::C_FriendsManager::Instance().GetFriendProfileFromID(slot)->Name();
							break;
						}
					}
				}

				mCurrentItemText.Format(id.String(), C_FriendChallenges::Instance().GetFriendDemand(-1)->mName, friend_str);
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ACHIEVEMENT:
			{
				Achievement* achievement = Game::C_AchievementsManager::Instance().mStickerBookLocations[index];

				int complete=0;
				for(int a=0;a<achievement->mStickers.Size();a++)
				{
					if(achievement->mStickers[a]->IsComplete())
						complete++;
				}

				int percentage=(int)(((float)complete/(float)achievement->mStickers.Size()) * 100);
			
				mCurrentItemText.Format(id.String(), percentage, C_AchievementSystem::Instance().GetAchievementName(achievement->mType) );
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_ACCEPTED:
			{
				static UI::C_WideString16 num1;
				static UI::C_WideString16 num2;
				
				int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
				if(stats_slot != -1)
				{
					num1.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mSent));
					num2.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mSent));

					if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mSent == 1)
						mCurrentItemText.Format(singular_id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
					else
						mCurrentItemText.Format(id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_WON:
			{
				static UI::C_WideString16 num1;
				static UI::C_WideString16 num2;
				
				int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
				if(stats_slot != -1)
				{
					num1.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted));
					num2.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted));

					if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted == 1)
						mCurrentItemText.Format(singular_id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
					else
						mCurrentItemText.Format(id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_KOS:
			{
				static UI::C_WideString16 num1;
				static UI::C_WideString16 num2;
				
				int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
				if(stats_slot != -1)
				{
					num1.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mKOs));
					num2.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mKOs));

					if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mKOs == 1)
						mCurrentItemText.Format(singular_id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
					else
						mCurrentItemText.Format(id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_MOST_ATTEMPTS:
			{
				static UI::C_WideString16 num1;
				static UI::C_WideString16 num2;

				int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
				if(stats_slot != -1)
				{
					if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted)
						num1.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mMostAttempts));
					else
						num1.Format( L"0" );

					if(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted)
						num2.Format(UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mMostAttempts));
					else
						num2.Format( L"0" );

					mCurrentItemText.Format(id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_AVG_ATTEMPTS:
			{
				static UI::C_WideString16 num1;
				static UI::C_WideString16 num2;
				
				int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
				if(stats_slot != -1)
				{
					float avg=0.f;
					if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted)
					{
						avg=(float)Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mTotalAttempts / (float)Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted; 
						num1.Format( L"%0.3f", avg );
					}
					else
					{
						num1.Format( L"0" );
					}

					avg=0.f;
					if(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted)
					{
						avg=(float)Game::C_FriendChallenges::Instance().mTaleOfTheTape.mTotalAttempts / (float)Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted; 
						num2.Format( L"%0.3f", avg );
					}
					else
					{
						num2.Format( L"0" );
					}

					mCurrentItemText.Format(id.String(), num1.Chars(), Game::C_FriendsManager::Instance().GetFriendProfileFromID(index)->Name(), num2.Chars());	
				}
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_WINS:
			{
				//I�m currently %s with %s wins in %d player Split-Screen
				Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

				int host_index=0;

				UI::C_WideString16 wins_str(UI::C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalWins(host_index) ));

				if(pGameMode->GetTotalWins(host_index) == 1)
					mCurrentItemText.Format(singular_id.String(), wins_str.Chars(), pGameMode->GetNumberOfPlayers());	
				else
					mCurrentItemText.Format(id.String(), wins_str.Chars(), pGameMode->GetNumberOfPlayers());	
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_POINTS:
			{
				//I�m currently %s with %s pts in %d player Split-Screen
				Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

				int host_index=0;
				int pos=pGameMode->GetNumberOfPlayers();
				int pts=pGameMode->GetTotalScore(host_index);

				for(int a=0;a<pGameMode->GetNumberOfPlayers();a++)
				{
					if(a==host_index)
						continue;

					if(pts >= pGameMode->GetTotalScore(a))
						pos--;
				}

				UI::C_WideString16 place(UI::C_UILocaliser::Instance().ConvertNumberToPosition(pos));
				UI::C_WideString16 points(UI::C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalScore(host_index) ));

				if(pGameMode->GetTotalScore(host_index) == 1)
					mCurrentItemText.Format(singular_id.String(), place.Chars(), points.Chars(), pGameMode->GetNumberOfPlayers());	
				else
					mCurrentItemText.Format(id.String(), place.Chars(), points.Chars(), pGameMode->GetNumberOfPlayers());	
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_GAMES:
			{
				//I�ve now played %s games in a Split-Screen session
				Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
				if(pGameMode->GetTotalRaces() == 1)
					mCurrentItemText.Format(singular_id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalRaces() ));
				else
					mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( pGameMode->GetTotalRaces() ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_PLAYTIME:
			{
				//I�ve now played for %s in a Split-Screen session
				Game::C_GameModeSplitScreen* pGameMode = (Game::C_GameModeSplitScreen*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
				mCurrentItemText.Format(id.String(), UI::C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( pGameMode->GetTotalRaceTime() ));
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_REDEMAND:
			{
				//I've just beaten and sent back %s to %s
				mCurrentItemText.Format(id.String(), C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mName, C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mChallenger);
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_PASSED:
			{
				//I've just passed %s's %s Friend Challenge after %d attempts
				mCurrentItemText.Format(id.String(), C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mChallenger, C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mName, C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mAttempts);
			}
			break;

		case AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_FAILED:
			{
				//I've just failed %s's %s Friend Challenge
				mCurrentItemText.Format(id.String(), C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mChallenger, C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mName);
			}
			break;

		default:
			mCurrentItemText.Clear();
			break;
	};
}

const WCHAR *C_SocialNetworkManager::GetScoreboardName()
{
	static UI::C_WideString64 str;
	UI::C_TextStringID sb_textid;

	switch(Game::C_ScoreboardManager::Instance().GetLookAtScoreboard())
	{
		case RacerNet::eMpScoreboardPerkRatio:
			sb_textid=UI::C_TextStringID( "MP_SCOREBOARD_TOTAL_POWERUPS" );
			break;
		case RacerNet::eMpScoreboardTotalWins:
			sb_textid=UI::C_TextStringID( "MP_SCOREBOARD_NEW_TOTAL_POINTS" );
			break;
		case RacerNet::eMpScoreboardTotalFans:
			sb_textid=UI::C_TextStringID( "MP_SCOREBOARD_NEW_FANS_TOTAL" );
			break;
		case RacerNet::eMpScoreboardLegends:
			sb_textid=UI::C_TextStringID( "MP_SCOREBOARD_NEW_LEGENDS" );
			break;
		case RacerNet::eMpScoreboardLobby:
			sb_textid=UI::C_TextStringID( "MP_SCOREBOARD_NEW_LOBBY" );
			break;
		default:
			{
				Game::C_GameProfile* prof = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
				BIZ_ASSERT_MSG(prof, "Could Not Get Profile in Check Scoreboard Type");

				AmaxEvent* ev = prof->GetCurrentEvent();
				BIZ_ASSERT_MSG(ev, "COULD NOT GET SORCERY DATA FOR SP LEADERBOARD READ");

				int sb=ev->mLeaderboardSettings.mScoreboardToWriteTo-RacerNet::eSP_Level_1_A;

				int sptier=sb/7;
				int spevent=sb%7;

				SP_SaveGame* sg=SP_GetCurrentSaveGame();
				SP_Tier *tier=SP_GetTier(sg,sptier);

				if(tier)
				{
					str.Format(UI::C_TextStringID( "SP_LEADERBOARD_T1E1_NAME" ).String(), UI::C_TextStringID(tier->name.id).String(), spevent + 1);
					
					return str.Chars();
				}
			}
			break;
	}

	return sb_textid.String();
}

int C_SocialNetworkManager::GetMyRaceHistoryPos(int index)
{
	int my_pos=-1;

	SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	for(int a=0;a<mRaceHistoryStore[index].mNumCars;a++)
	{
		bdOnlineUserID id=BLUR_USER_ID(profileMe.m_Uid);
		if(mRaceHistoryStore[index].mVehicles[a].mSystemID == id)
		{
			my_pos=mRaceHistoryStore[index].mVehicles[a].mPosition;
			break;
		}
	}

	return my_pos;
}

int C_SocialNetworkManager::GetLastNetworkRaceIndex()
{
	return (mRaceHistoryStoreIndex > 0) ? mRaceHistoryStoreIndex - 1 : MAX_SOCIAL_NETWORK_RACE_HISTORY - 1;
}

}; // namespace Game

/*
void C_TwitterManager::AddOneOffTweet(AMAX_TWEET_TYPE type, U32 param)
{
	if(mTweetStore.Size() >= MAX_TWEETS)
		return;

	AmaxTweetItem tweet;

	for(int a=0;a<C_GameData::Instance().mTweets.Number();a++)
	{
		AmaxTweet *gd_tweet=C_GameData::Instance().mTweets[a];

		if(gd_tweet->give_type == AMAX_TWEET_GIVE_TYPE_ONE_OFF && gd_tweet->type == type)
		{
			int t=rand()&(gd_tweet->tweets.count-1);

			if(type == AMAX_TWEET_TYPE_JUST_PLAYING)
			{
				tweet.mType=type;
				if(param == 0)
					tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String());
				else
					tweet.mText.Format(UI::gDashboardInterface.GetString());

				mTweetStore.PushBack(tweet);
				return;
			}

			if(type == AMAX_TWEET_TYPE_WRECKED || type == AMAX_TWEET_TYPE_PHOTO_TAKEN || type == AMAX_TWEET_TYPE_PHOTO_UPLOAD || type == AMAX_TWEET_TYPE_STORY_COMPLETE)
			{
				tweet.mType=type;
				tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String());
				mTweetStore.PushBack(tweet);
				return;
			}

			if(type == AMAX_TWEET_TYPE_ACHIEVEMENT)
			{
				tweet.mType=type;
				tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String(),C_AchievementSystem::Instance().GetAchievementName((BLUR_ACHIEVEMENT)param));
				mTweetStore.PushBack(tweet);
				return;
			}

			if(type == AMAX_TWEET_TYPE_COMPLETED_EVENT)
			{
				const WCHAR* pDifficulty = NULL;
				Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
				if( pGameProfile )
				{
					Game::GAME_DIFFICULTY eDifficulty = (Game::GAME_DIFFICULTY)pGameProfile->GetDifficulty();
					switch ( eDifficulty )
					{
						case Game::GAME_DIFFICULTY_VERY_EASY:	pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_1" ).String(); break;
						case Game::GAME_DIFFICULTY_EASY:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_2" ).String(); break;
						case Game::GAME_DIFFICULTY_NORMAL:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_3" ).String(); break;
						case Game::GAME_DIFFICULTY_HARD:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_4" ).String(); break;
						case Game::GAME_DIFFICULTY_HARDCORE:	pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_5" ).String(); break;
					}
				}

				tweet.mType=type;
				tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String(),UI::C_TextStringID(C_GameEventData::Instance().mEvents[(GAMEDATA_ID)param]->loc_string.id).String(),pDifficulty);
				mTweetStore.PushBack(tweet);
				return;
			}

			if(type == AMAX_TWEET_TYPE_BOUGHT_CAR)
			{
				RaceBook::C_RaceBookVehicle *veh=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)param);
				if(veh)
				{
					AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)veh->mBaseModelID ];
					if(pVehicle)
					{
						UI::C_WideString64 str;
						str.Format(L"%s %s",UI::C_TextStringID(pVehicle->manufacturer->loc_string.id).String(),UI::C_TextStringID(pVehicle->loc_string.id).String());

						tweet.mType=type;
						tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String(), str.Chars());
						mTweetStore.PushBack(tweet);
						return;
					}
				}
			}

			if(type == AMAX_TWEET_TYPE_BOUGHT_SHOP_ITEM)
			{
				RaceBookShopUpgrades upgrade=(RaceBookShopUpgrades)((int)param);

				for(int a=0;a<C_GameData::Instance().mShopItems.Number();a++)
				{
					if(C_GameData::Instance().mShopItems[a]->type == upgrade)
					{
						tweet.mType=type;
						tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String(),UI::C_TextStringID(C_GameData::Instance().mShopItems[a]->loc_string.id).String());
						mTweetStore.PushBack(tweet);
						return;
					}
				}
			}

			if(type == AMAX_TWEET_TYPE_MULTIPLAYER_CREATED_A_GROUP)
			{
				tweet.mType=type;
				tweet.mText.Format(UI::C_TextStringID(gd_tweet->tweets[t].tweets.id).String(),UI::C_UserEventManager::Instance().GetEvent( param )->Name());
				mTweetStore.PushBack(tweet);
				return;
			}
		}
	}
}

void C_TwitterManager::ClearTweet(int index)
{
	mTweetStore.EraseByIndex(index);
}


void C_TwitterManager::StoreDataPreRace()
{
	Game::C_GameProfile *profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ((profile == NULL) || (profile->mRaceBookProfile == NULL))
		return;

	memcpy(&mPreRaceStats, &profile->mRaceBookProfile->mLifetimeStatistics, sizeof(RaceBook::C_RaceBookProfileStatistics));

	mCareerEarnings=profile->mRaceBookProfile->mTotalCareerEarnings;

	mMPRank=profile->GetMultiplayerProfile()->GetCurrentLevel()+1;
	mMPLegend=profile->GetMultiplayerProfile()->GetLegendLevel();
	
	mMPDriversEdProgress=0;
	mMPDemolitionProgress=0;
	mMPSpecialistProgress=0;
	mMPAceProgress=0;

	const C_MultiplayerChallengeSystem& challengeSystem = profile->GetMultiplayerProfile()->GetChallengeSystem();

	for(int i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++)
	{
		MultiplayerChallengePack *pChallengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		if(pChallengePack == NULL)
			continue;

		int num=0;
		float perc=0.f;

		for(int j = 0; j < pChallengePack->mChallenges.count; j++)
		{
			perc+=challengeSystem.GetChallenge( pChallengePack->mChallenges[j]->mType ).GetProgress();
			num++;
		}

		if(num)
		{
			if(pChallengePack->mType == UI::E_ChallengePack_Ace)
				mMPAceProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_Demolition)
				mMPDemolitionProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_DriversED)
				mMPDriversEdProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_Specialist)
				mMPSpecialistProgress=(perc/(float)num)*100;
		}
	}
}

bool C_TwitterManager::TweetOnIncrement(float old_val, float new_val, int increment, int *value)
{
	float old_value=old_val/increment;
	float new_value=new_val/increment;

	int old_step=int(old_value);
	int new_step=int(new_value);

	if(new_step > old_step) //must have passed increment threshold
	{
		*value=(new_step*increment);
		return true;
	}
	
	*value=0;
	return false;
}

void C_TwitterManager::AddIncrementTweet(AMAX_TWEET_TYPE type, U32 id, int value, bool suffix)
{
	if(mTweetStore.Size() >= MAX_TWEETS)
		return;

	AmaxTweetItem tweet;
	tweet.mType=type;

	if(suffix)
		tweet.mText.Format(UI::C_TextStringID(id).String(),UI::C_UILocaliser::Instance().ConvertNumberToPosition(value));
	else
		tweet.mText.Format(UI::C_TextStringID(id).String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas(value));

	mTweetStore.PushBack(tweet);
}

void C_TwitterManager::AddTweetsPostRace(bool had_passed,bool multiplayer)
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();

	Game::C_GameProfile *profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if( results.mFake )
		return;

	// Handle case where we're in a debug race without a profile
	if ((profile == NULL) || (profile->mRaceBookProfile == NULL))
		return;

	RaceBook::C_RaceBookProfileStatistics& stats = profile->mRaceBookProfile->mLifetimeStatistics;

	int MPRank=profile->GetMultiplayerProfile()->GetCurrentLevel()+1;
	int MPLegend=profile->GetMultiplayerProfile()->GetLegendLevel();
	
	int MPDriversEdProgress=0;
	int MPDemolitionProgress=0;
	int MPSpecialistProgress=0;
	int MPAceProgress=0;

	for(int i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++)
	{
		MultiplayerChallengePack *pChallengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		if(pChallengePack == NULL)
			continue;

		int num=0;
		float perc=0.f;

		for(int j = 0; j < pChallengePack->mChallenges.count; j++)
		{
			perc+=profile->GetMultiplayerProfile()->GetChallengeSystem().GetChallenge(pChallengePack->mChallenges[j]->mType).GetProgress();
			num++;
		}

		if(num)
		{
			if(pChallengePack->mType == UI::E_ChallengePack_Ace)
				MPAceProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_Demolition)
				MPDemolitionProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_DriversED)
				MPDriversEdProgress=(perc/(float)num)*100;
			
			if(pChallengePack->mType == UI::E_ChallengePack_Specialist)
				MPSpecialistProgress=(perc/(float)num)*100;
		}
	}


	for(int a=0;a<C_GameData::Instance().mTweets.Number();a++)
	{
		AmaxTweet *gd_tweet=C_GameData::Instance().mTweets[a];

		if(gd_tweet->tweets.count == 0)
		{
			BIZ_ASSERT_FAIL("No text for Tweet, ignoring");
			continue;
		}

		if(gd_tweet->give_type == AMAX_TWEET_GIVE_TYPE_ONE_OFF)
		{
			
			int t=rand()&(gd_tweet->tweets.count-1);

			switch(gd_tweet->type)
			{
				case AMAX_TWEET_TYPE_WRECKED:
					{
						if(results.mCars[0].mStatistics.mWrecked)
						{
							AddOneOffTweet(AMAX_TWEET_TYPE_WRECKED,0);
						}
					}
					break;

				case AMAX_TWEET_TYPE_COMPLETED_EVENT:
					{
						if(had_passed && !multiplayer)
						{
							AddOneOffTweet(AMAX_TWEET_TYPE_COMPLETED_EVENT,levelState.mEvent);
						}
					}
					break;

				default:
					break;
			}
		}
		else if(gd_tweet->give_type == AMAX_TWEET_GIVE_TYPE_INCREMENTS)
		{
			if(gd_tweet->parameters.count == 0)
			{
				BIZ_ASSERT_FAIL("No increment number for Tweet, ignoring");
				continue;
			}

			int t=rand()&(gd_tweet->tweets.count-1);
			int increment=gd_tweet->parameters[0];
			int value=0;

			switch(gd_tweet->type)
			{
				case AMAX_TWEET_TYPE_BANKBALANCE:
					if(TweetOnIncrement(mCareerEarnings,profile->mRaceBookProfile->mTotalCareerEarnings,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_RACES_ATTEMPTED:
					if(TweetOnIncrement(mPreRaceStats.mRacesAttempted,stats.mRacesAttempted,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_NITROUS_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_NITROUS],stats.mPickupsFired[E_PICKUP_TYPE_NITROUS],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHOCK_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHOCK],stats.mPickupsFired[E_PICKUP_TYPE_SHOCK],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHUNT_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHUNT],stats.mPickupsFired[E_PICKUP_TYPE_SHUNT],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_BARGE_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_BARGE],stats.mPickupsFired[E_PICKUP_TYPE_BARGE],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MINE_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_BOMB],stats.mPickupsFired[E_PICKUP_TYPE_BOMB],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHOCK_HIT:
					if(TweetOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_SHOCK],stats.mPickupsHit[E_PICKUP_TYPE_SHOCK],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHUNT_HIT:
					if(TweetOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_SHUNT],stats.mPickupsHit[E_PICKUP_TYPE_SHUNT],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_BARGE_HIT:
					if(TweetOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_BARGE],stats.mPickupsHit[E_PICKUP_TYPE_BARGE],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MINE_HIT:
					if(TweetOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_BOMB],stats.mPickupsHit[E_PICKUP_TYPE_BOMB],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_REPAIRS:
					if(TweetOnIncrement(mPreRaceStats.mTimesRepaired,stats.mTimesRepaired,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHIELD_USED:
					if(TweetOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHIELD],stats.mPickupsFired[E_PICKUP_TYPE_SHIELD],increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_SHIELDED:
					if(TweetOnIncrement(mPreRaceStats.mTimesShielded,stats.mTimesShielded,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_FANS:
					if(TweetOnIncrement(mPreRaceStats.mNumFans,stats.mNumFans,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_DISTANCE_TRAVELLED:
					if(TweetOnIncrement((mPreRaceStats.mDistanceDriven/1000.f)*0.621371192f,(stats.mDistanceDriven/1000.f)*0.621371192f,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;


				case AMAX_TWEET_TYPE_MULTIPLAYER_RANK:
					if(TweetOnIncrement(mMPRank,MPRank,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MULTIPLAYER_LEGEND:
					if(TweetOnIncrement(mMPLegend,MPLegend,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MULTIPLAYER_ACE_PACK:
					if(TweetOnIncrement(mMPAceProgress,MPAceProgress,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MULTIPLAYER_DRIVERS_ED_PACK:
					if(TweetOnIncrement(mMPDriversEdProgress,MPDriversEdProgress,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MULTIPLAYER_DEMOLITION_PACK:
					if(TweetOnIncrement(mMPDemolitionProgress,MPDemolitionProgress,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				case AMAX_TWEET_TYPE_MULTIPLAYER_SPECIALIST_PACK:
					if(TweetOnIncrement(mMPSpecialistProgress,MPSpecialistProgress,increment,&value))
						AddIncrementTweet(gd_tweet->type, gd_tweet->tweets[t].tweets.id, value, gd_tweet->tweets[t].suffix);
					break;

				default:
					break;
			}
		}
	}
}



AmaxFacebookStory *C_FacebookManager::GetFacebookDataStory(int type)
{
	for(int a=0;a<C_GameData::Instance().mFacebookStories.Number();a++)
	{
		if(type == C_GameData::Instance().mFacebookStories[a]->type)
		{
			return C_GameData::Instance().mFacebookStories[a];
		}
	}

	return NULL;
}

void C_FacebookManager::AddOneOffFacebookStory(AMAX_FACEBOOK_TYPE type, U32 param)
{
	if(mStoryStore.Size() >= MAX_FACEBOOK_STORIES)
		return;

	AmaxFacebookStoryItem facebook_story;
	facebook_story.mMessage.Clear();

	for(int a=0;a<C_GameData::Instance().mFacebookStories.Number();a++)
	{
		AmaxFacebookStory *gd_facebookstory=C_GameData::Instance().mFacebookStories[a];

		if(gd_facebookstory->give_type == AMAX_FACEBOOK_GIVE_TYPE_ONE_OFF && gd_facebookstory->type == type)
		{
			int t=rand()&(gd_facebookstory->stories.count-1);

			if(type == AMAX_FACEBOOK_TYPE_JUST_PLAYING)
			{
				if(param == 0)
					facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String());
				else
					facebook_story.mName.Format(UI::gDashboardInterface.GetString());

				facebook_story.mType=gd_facebookstory->type;
				facebook_story.mStringIndex=t;

				mStoryStore.PushBack(facebook_story);
				return;

			}

			if(type == AMAX_FACEBOOK_TYPE_WRECKED || type == AMAX_FACEBOOK_TYPE_PHOTO_TAKEN || type == AMAX_FACEBOOK_TYPE_PHOTO_UPLOAD || type == AMAX_FACEBOOK_TYPE_STORY_COMPLETE)
			{
				facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String());

				facebook_story.mType=gd_facebookstory->type;
				facebook_story.mStringIndex=t;

				mStoryStore.PushBack(facebook_story);
				return;
			}

			if(type == AMAX_FACEBOOK_TYPE_ACHIEVEMENT)
			{
				facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(),C_AchievementSystem::Instance().GetAchievementName((BLUR_ACHIEVEMENT)param));

				facebook_story.mType=gd_facebookstory->type;
				facebook_story.mStringIndex=t;

				mStoryStore.PushBack(facebook_story);
				return;
			}

			if(type == AMAX_FACEBOOK_TYPE_COMPLETED_EVENT)
			{
				const WCHAR* pDifficulty = NULL;
				Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
				if( pGameProfile )
				{
					Game::GAME_DIFFICULTY eDifficulty = (Game::GAME_DIFFICULTY)pGameProfile->GetDifficulty();
					switch ( eDifficulty )
					{
						case Game::GAME_DIFFICULTY_VERY_EASY:	pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_1" ).String(); break;
						case Game::GAME_DIFFICULTY_EASY:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_2" ).String(); break;
						case Game::GAME_DIFFICULTY_NORMAL:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_3" ).String(); break;
						case Game::GAME_DIFFICULTY_HARD:		pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_4" ).String(); break;
						case Game::GAME_DIFFICULTY_HARDCORE:	pDifficulty = UI::C_TextStringID( "CMN_DIFFICULTY_5" ).String(); break;
					}
				}

				
				facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(),UI::C_TextStringID(C_GameEventData::Instance().mEvents[(GAMEDATA_ID)param]->loc_string.id).String(),pDifficulty);

				facebook_story.mType=gd_facebookstory->type;
				facebook_story.mStringIndex=t;

				mStoryStore.PushBack(facebook_story);
				return;
			}

			if(type == AMAX_FACEBOOK_TYPE_BOUGHT_CAR)
			{
				RaceBook::C_RaceBookVehicle *veh=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)param);
				if(veh)
				{
					AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)veh->mBaseModelID ];
					if(pVehicle)
					{
						UI::C_WideString64 str;
						str.Format(L"%s %s",UI::C_TextStringID(pVehicle->manufacturer->loc_string.id).String(),UI::C_TextStringID(pVehicle->loc_string.id).String());

						facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(), str.Chars());

						facebook_story.mType=gd_facebookstory->type;
						facebook_story.mStringIndex=t;

						mStoryStore.PushBack(facebook_story);
						return;
					}
				}
			}

			if(type == AMAX_FACEBOOK_TYPE_BOUGHT_SHOP_ITEM)
			{
				RaceBookShopUpgrades upgrade=(RaceBookShopUpgrades)((int)param);

				for(int a=0;a<C_GameData::Instance().mShopItems.Number();a++)
				{
					if(C_GameData::Instance().mShopItems[a]->type == upgrade)
					{
						facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(),UI::C_TextStringID(C_GameData::Instance().mShopItems[a]->loc_string.id).String());

						facebook_story.mType=gd_facebookstory->type;
						facebook_story.mStringIndex=t;

						mStoryStore.PushBack(facebook_story);
						return;
					}
				}
			}

			if(type == AMAX_FACEBOOK_TYPE_MULTIPLAYER_CREATED_A_GROUP)
			{
				facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(), UI::C_UserEventManager::Instance().GetEvent( param )->Name());

				facebook_story.mType=gd_facebookstory->type;
				facebook_story.mStringIndex=t;

				mStoryStore.PushBack(facebook_story);
				return;
			}

		}
	}
}

bool C_FacebookManager::FacebookStoryOnIncrement(float old_val, float new_val, int increment, int *value)
{
	float old_value=old_val/increment;
	float new_value=new_val/increment;

	int old_step=int(old_value);
	int new_step=int(new_value);

	if(new_step > old_step) //must have passed increment threshold
	{
		*value=(new_step*increment);
		return true;
	}
	
	*value=0;
	return false;
}

void C_FacebookManager::AddIncrementFacebookStory(AmaxFacebookStory *gd_facebookstory, int t, int value)
{
	if(mStoryStore.Size() >= MAX_FACEBOOK_STORIES)
		return;

	AmaxFacebookStoryItem facebook_story;
	facebook_story.mMessage.Clear();

	if(gd_facebookstory->stories[t].suffix)
		facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(),UI::C_UILocaliser::Instance().ConvertNumberToPosition(value));
	else
		facebook_story.mName.Format(UI::C_TextStringID(gd_facebookstory->stories[t].name.id).String(),UI::C_UILocaliser::Instance().ConvertNumberToCommas(value));

	facebook_story.mType=gd_facebookstory->type;
	facebook_story.mStringIndex=t;

	mStoryStore.PushBack(facebook_story);
}

void C_FacebookManager::StoreDataPreRace()
{
	Game::C_GameProfile *profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ((profile == NULL) || (profile->mRaceBookProfile == NULL))
		return;

	memcpy(&mPreRaceStats, &profile->mRaceBookProfile->mLifetimeStatistics, sizeof(RaceBook::C_RaceBookProfileStatistics));

	mCareerEarnings=profile->mRaceBookProfile->mTotalCareerEarnings;

	mMPRank=profile->GetMultiplayerProfile()->GetCurrentLevel()+1;
	mMPLegend=profile->GetMultiplayerProfile()->GetLegendLevel();
	
	mMPDriversEdProgress=0;
	mMPDemolitionProgress=0;
	mMPSpecialistProgress=0;
	mMPAceProgress=0;

	for(int i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++)
	{
		MultiplayerChallengePack *pChallengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		if(pChallengePack == NULL)
			continue;

		int num=0;
		float perc=0.f;

		for(int j = 0; j < pChallengePack->mChallenges.count; j++)
		{
			perc+=profile->GetMultiplayerProfile()->GetChallengeSystem().GetChallenge(pChallengePack->mChallenges[j]->mType).GetProgress();
			num++;
		}

		if(num)
		{
			switch( pChallengePack->mType )
			{
				case UI::E_ChallengePack_Ace :
				{
					mMPAceProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_Demolition :
				{
					mMPDemolitionProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_DriversED :
				{
					mMPDriversEdProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_Specialist :
				{
					mMPSpecialistProgress=(int)(perc/(float)num)*100;
					break;
				}
			}
		}
	}
}

void C_FacebookManager::AddStoriesPostRace(bool had_passed,bool multiplayer)
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();

	Game::C_GameProfile *profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if( results.mFake )
		return;

	// Handle case where we're in a debug race without a profile
	if ((profile == NULL) || (profile->mRaceBookProfile == NULL))
		return;

	RaceBook::C_RaceBookProfileStatistics& stats = profile->mRaceBookProfile->mLifetimeStatistics;

	int MPRank=profile->GetMultiplayerProfile()->GetCurrentLevel()+1;
	int MPLegend=profile->GetMultiplayerProfile()->GetLegendLevel();
	
	int MPDriversEdProgress=0;
	int MPDemolitionProgress=0;
	int MPSpecialistProgress=0;
	int MPAceProgress=0;

	for(int i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++)
	{
		MultiplayerChallengePack *pChallengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		if(pChallengePack == NULL)
			continue;

		int num=0;
		float perc=0.f;

		for(int j = 0; j < pChallengePack->mChallenges.count; j++)
		{
			perc+=profile->GetMultiplayerProfile()->GetChallengeSystem().GetChallenge(pChallengePack->mChallenges[j]->mType).GetProgress();
			num++;
		}

		if(num)
		{
			switch( pChallengePack->mType )
			{
				case UI::E_ChallengePack_Ace :
				{
					MPAceProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_Demolition :
				{
					MPDemolitionProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_DriversED :
				{
					MPDriversEdProgress=(int)(perc/(float)num)*100;
					break;
				}
				case UI::E_ChallengePack_Specialist :
				{
					MPSpecialistProgress=(int)(perc/(float)num)*100;
					break;
				}
			}
		}
	}

	for(int a=0;a<C_GameData::Instance().mFacebookStories.Number();a++)
	{
		AmaxFacebookStory *gd_facebookstory=C_GameData::Instance().mFacebookStories[a];

		if(gd_facebookstory->stories.count == 0)
		{
			BIZ_ASSERT_FAIL("No text for Facebook Story, ignoring");
			continue;
		}

		if(gd_facebookstory->give_type == AMAX_FACEBOOK_GIVE_TYPE_ONE_OFF)
		{
			int t=rand()&(gd_facebookstory->stories.count-1);

			switch(gd_facebookstory->type)
			{
				case AMAX_FACEBOOK_TYPE_WRECKED:
					{
						if(results.mCars[0].mStatistics.mWrecked)
						{
							AddOneOffFacebookStory(AMAX_FACEBOOK_TYPE_WRECKED,0);
						}
					}
					break;

				case AMAX_FACEBOOK_TYPE_COMPLETED_EVENT:
					{
						if(had_passed && !multiplayer)
						{
							AddOneOffFacebookStory(AMAX_FACEBOOK_TYPE_COMPLETED_EVENT,levelState.mEvent);
						}
					}
					break;

				default:
					break;
			}
		}
		else if(gd_facebookstory->give_type == AMAX_FACEBOOK_GIVE_TYPE_INCREMENTS)
		{
			if(gd_facebookstory->parameters.count == 0)
			{
				BIZ_ASSERT_FAIL("No increment number for Facebook Story, ignoring");
				continue;
			}

			int t=rand()&(gd_facebookstory->stories.count-1);
			int increment=gd_facebookstory->parameters[0];
			int value=0;

			switch(gd_facebookstory->type)
			{
				case AMAX_FACEBOOK_TYPE_BANKBALANCE:
					if(FacebookStoryOnIncrement(mCareerEarnings,profile->mRaceBookProfile->mTotalCareerEarnings,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_RACES_ATTEMPTED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mRacesAttempted,stats.mRacesAttempted,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_NITROUS_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_NITROUS],stats.mPickupsFired[E_PICKUP_TYPE_NITROUS],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHOCK_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHOCK],stats.mPickupsFired[E_PICKUP_TYPE_SHOCK],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHUNT_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHUNT],stats.mPickupsFired[E_PICKUP_TYPE_SHUNT],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_BARGE_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_BARGE],stats.mPickupsFired[E_PICKUP_TYPE_BARGE],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MINE_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_BOMB],stats.mPickupsFired[E_PICKUP_TYPE_BOMB],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHOCK_HIT:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_SHOCK],stats.mPickupsHit[E_PICKUP_TYPE_SHOCK],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHUNT_HIT:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_SHUNT],stats.mPickupsHit[E_PICKUP_TYPE_SHUNT],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_BARGE_HIT:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_BARGE],stats.mPickupsHit[E_PICKUP_TYPE_BARGE],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MINE_HIT:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsHit[E_PICKUP_TYPE_BOMB],stats.mPickupsHit[E_PICKUP_TYPE_BOMB],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_REPAIRS:
					if(FacebookStoryOnIncrement(mPreRaceStats.mTimesRepaired,stats.mTimesRepaired,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHIELD_USED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mPickupsFired[E_PICKUP_TYPE_SHIELD],stats.mPickupsFired[E_PICKUP_TYPE_SHIELD],increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_SHIELDED:
					if(FacebookStoryOnIncrement(mPreRaceStats.mTimesShielded,stats.mTimesShielded,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_FANS:
					if(FacebookStoryOnIncrement(mPreRaceStats.mNumFans,stats.mNumFans,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_DISTANCE_TRAVELLED:
					if(FacebookStoryOnIncrement((mPreRaceStats.mDistanceDriven/1000.f)*0.621371192f,(stats.mDistanceDriven/1000.f)*0.621371192f,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;


				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_RANK:
					if(FacebookStoryOnIncrement(mMPRank,MPRank,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_LEGEND:
					if(FacebookStoryOnIncrement(mMPLegend,MPLegend,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_ACE_PACK:
					if(FacebookStoryOnIncrement(mMPAceProgress,MPAceProgress,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_DRIVERS_ED_PACK:
					if(FacebookStoryOnIncrement(mMPDriversEdProgress,MPDriversEdProgress,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_DEMOLITION_PACK:
					if(FacebookStoryOnIncrement(mMPDemolitionProgress,MPDemolitionProgress,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				case AMAX_FACEBOOK_TYPE_MULTIPLAYER_SPECIALIST_PACK:
					if(FacebookStoryOnIncrement(mMPSpecialistProgress,MPSpecialistProgress,increment,&value))
						AddIncrementFacebookStory(gd_facebookstory, t, value);
					break;

				default:
					break;
			}
		}
	}
}

void C_FacebookManager::ClearStory(int index)
{
	mStoryStore.EraseByIndex(index);
}
*/
