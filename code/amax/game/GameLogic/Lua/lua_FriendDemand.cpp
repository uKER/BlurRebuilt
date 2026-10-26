#include "stdafx.h"

#include <ui/Game/UILua.h>

#include <GameLogic\SocialNetworking\FriendsManager.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <GameLogic\FriendsChallenges\MessagingManager.h>

#include <ui/DashboardInterface.h>

#include "Game/Game.h"
#include "GameLogic/Structures/LevelResult.h"

#include <ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

#include <GameLogic/SinglePlayer/SinglePlayer.h>

#include <GameLogic\SocialNetworking\SocialNetworkManager.h>

#ifdef PLATFORM_PS3
#include <cell/l10n.h>
#endif

extern Debug::Logger &gLUAMessageLogger;

#ifndef CFG_GOLD
Debug::Bool g_debugAlwaysChooseFriendChallengeSlot("FriendChallenge.AllSlotsFull", false);
#endif

namespace Game
{

static int FriendDemand_ChangeSwitch( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_ChangeSwitch(index, dir) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	int switch_enum=(int)lua_tonumber(pState,2);

	C_FriendChallenges::Instance().ChangeSwitch(index, switch_enum);

	return 0;
}

static S32 FriendDemand_GetNumActiveSwitches( lua_State* pState )
{
	lua_pushnumber( pState, C_FriendChallenges::Instance().NumActiveSwitches() );
	
	return 1;
}

static S32 FriendDemand_GetSwitch( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_GetSwitch(index) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	
	lua_pushnumber( pState, C_FriendChallenges::Instance().GetSwitch(index) );

	return 1;
}

static S32 FriendDemand_GetParamType( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetParamType( index ) - incorrect arguments" );
		return 0;
	}

	int index = (int)lua_tonumber( pState, 1 );

	lua_pushnumber( pState, C_FriendChallenges::Instance().GetParamType( index ) );

	return 1;
}

static S32 FriendDemand_Create( lua_State* pState )
{
	C_FriendChallenges::Instance().CreateActualFriendDemand();
	return 0;
}

static S32 FriendDemand_StoreName( lua_State* pState )
{
	C_FriendChallenges::Instance().SetFriendDemandName(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 FriendDemand_StoreDescription( lua_State* pState )
{
	C_FriendChallenges::Instance().SetFriendDemandDescription(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 FriendDemand_AddFriend( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_AddFriend(index) - incorrect arguments\n");
		return 0;
	}

	int fd_index=(int)lua_tonumber(pState,1);
	int friend_list_index=(int)lua_tonumber(pState,2);
	
	C_FriendChallenges::Instance().AddFriend(friend_list_index, fd_index);

	return 0;
}

static S32 FriendDemand_IsFriend( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_AddFriend(index) - incorrect arguments\n");
		return 0;
	}

	int fd_index=(int)lua_tointeger(pState,1);

	if(C_FriendsManager::Instance().GetFriendProfileFromID(fd_index))
		lua_pushboolean( pState, true );
	else
		lua_pushboolean( pState, false );
	
	return 1;
}


static S32 FriendDemand_GetNumFriends( lua_State* pState )
{
	lua_pushnumber( pState, C_FriendChallenges::Instance().NumFriendsAdded() );
	
	return 1;
}

static S32 FriendDemand_RequestRemotePicture( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_RequestRemotePicture( friend_index, ref ) - incorrect arguments" );
		return 0;
	}

	S32 friend_slot = (S32)lua_tonumber( pState, 1 );
	S32 ref = (S32)lua_tonumber( pState, 2 );

	C_FriendChallenges::Instance().RequestRemotePicture( friend_slot, ref );

	return 0;
}

static S32 FriendDemand_Send( lua_State* pState )
{
	bdOnlineUserID ids[MAX_FRIEND_DEMAND_FRIENDS];

	int index=0;

	for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
	{
		int slot=C_FriendChallenges::Instance().GetFriendSlot(a);

		if(slot != -1)
		{
#ifdef PLATFORM_PS3
			ids[index]=bdAuthUtility::getUserID(C_FriendsManager::Instance().GetFriendProfileFromID(slot)->m_Uid.handle.data);
#else
			ids[index]=C_FriendsManager::Instance().GetFriendProfileFromID(slot)->m_Uid;
#endif
			index++;
		}
	}

	C_MessagingManager::Instance().SendMessage(ids,C_FriendChallenges::Instance().NumFriendsAdded(),C_FriendChallenges::Instance().GetFriendDemand(), sizeof(FriendChallenge));
	
	return 0;
}

static S32 FriendDemand_PumpSend( lua_State* pState )
{
	int error=BD_NO_ERROR;

	bool still_pumping=C_MessagingManager::Instance().PumpCurrentTask(&error);

	lua_pushboolean(pState,still_pumping);
	lua_pushnumber(pState,error);

	return 2;
}

static S32 FriendDemand_RetrieveFromServer( lua_State* pState )
{
	if(!lua_isboolean(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_RetrieveFromServer(silent) - incorrect arguments\n");
		return 0;
	}

	C_MessagingManager::Instance().GetMail(lua_toboolean(pState,1) ? true : false);

	return 0;
}


static int FriendDemand_GetSlotData( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;

	for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
	{
		lua_newtable(pState);
			Lua::AddDataMember(pState,"empty", ( C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID == 0 ));
			Lua::AddDataMember(pState,"index", a);
		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static S32 FriendDemand_GetFreeSlot( lua_State* pState )
{
	int free_slot=-1;

	for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
	{
		if( C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID == 0 )
		{
			free_slot=a;
			break;
		}
	}
#ifndef CFG_GOLD
	if(g_debugAlwaysChooseFriendChallengeSlot)
		free_slot=-1;
#endif

	lua_pushnumber(pState, free_slot);

	return 1;
}

static S32 FriendDemand_SaveMessageToSlot( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_SaveMessageToSlot(message_index, slot) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	int slot=(int)lua_tonumber(pState,2);
	
	Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

	memcpy(&Game::C_FriendChallenges::Instance().mFriendDemandStore[slot],fc,sizeof(Game::FriendChallenge));
	Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[slot]=Game::C_MessagingManager::Instance().GetMailHeader(index)->m_senderID;

	return 0;
}

static S32 FriendDemand_RemoveMessageFromSlot( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_RemoveMessageFromSlot(slot) - incorrect arguments\n");
		return 0;
	}

	int slot=(int)lua_tonumber(pState,1);
	
	Game::C_FriendChallenges::Instance().mFriendDemandStore[slot].mEventID=0;

	return 0;
}

static S32 FriendDemand_NumFriendDemandsForEvent( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_NumFriendDemandsForEvent(eventID) - incorrect arguments\n");
		return 0;
	}

	U32 eventID=(int)lua_tonumber(pState,1);
	
	lua_newtable(pState);

	int num=0;
	int index=1;

	for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
	{
		if(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID == eventID)
		{
			lua_newtable(pState);
				Lua::AddDataMember(pState,"index", a);
			lua_rawseti(pState,-2,index);
			index++;

			num++;
		}
	}

	Lua::AddDataMember(pState,"total", num);

	return 1;
}

static S32 FriendDemand_DeleteMessage( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_DeleteMessage(index) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	
	lua_pushboolean( pState, C_MessagingManager::Instance().DeleteMail(index) );
	
	return 1;
}

static S32 FriendDemand_MessagesStillOnServer( lua_State* pState )
{
	lua_pushboolean( pState, C_MessagingManager::Instance().MailStillOnServer() );
	
	return 1;
}

static S32 FriendDemand_GetInfo( lua_State* pState )
{
	int index = -1;

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
		index = (int)lua_tonumber( pState, 1 );

	bool bServer = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 ) );
	bool bPostRace = ( ( lua_type( pState, 3 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 3 ) != 0 ) );

	lua_newtable( pState );

	if ( bServer )
	{
		bdMailBody* pBody = Game::C_MessagingManager::Instance().GetMailBody( index );
		
		if ( pBody )
		{
			Game::FriendChallenge* pFC = (Game::FriendChallenge*)pBody->m_mailBody;

			if(pFC)
			{
				Lua::AddDataMember( pState, "eventid", pFC->mEventID );

				SP_Event* pEvent = SP_GetEventByAmaxEventId( SP_GetCurrentSaveGame(), pFC->mEventID );

				if ( pEvent )
					Lua::AddDataMember( pState, "eventid_sp", pEvent->id );

				Lua::AddDataMember( pState, "state", SP_GetEventStateName( (SP_EventState)pFC->mMedal ) );
				Lua::AddDataMember( pState, "vehicleid", pFC->mVehicleUsed );
				Lua::AddDataMember( pState, "difficulty", pFC->mDifficulty );

				const FriendChallengeParameter& extra = pFC->mParameters[ 0 ];
				Lua::AddDataMember( pState, "extra_modifier", extra.mModifier );
				Lua::AddDataMember( pState, "extra_param", extra.mParamType );

				Lua::AddDataMember( pState, "rechallenge", pFC->mReChallenge );
			}
		}
	}
	else
	{
		if(bPostRace)
		{
			Lua::AddDataMember( pState, "eventid", Game::C_FriendChallenges::Instance().GetEventID( -1 ) );
			SP_Event* pEvent = SP_GetEventByAmaxEventId( SP_GetCurrentSaveGame(), Game::C_FriendChallenges::Instance().GetEventID( -1 ) );
			if ( pEvent )
				Lua::AddDataMember( pState, "eventid_sp", pEvent->id );
		}
		else
		{
			Lua::AddDataMember( pState, "eventid", Game::C_FriendChallenges::Instance().GetEventID( index ) );
			SP_Event* pEvent = SP_GetEventByAmaxEventId( SP_GetCurrentSaveGame(), Game::C_FriendChallenges::Instance().GetEventID( index ) );
			if ( pEvent )
				Lua::AddDataMember( pState, "eventid_sp", pEvent->id );
		}

		Lua::AddDataMember( pState, "state", SP_GetEventStateName( (SP_EventState)Game::C_FriendChallenges::Instance().GetMedal( index ) ) );
		Lua::AddDataMember( pState, "vehicleid", Game::C_FriendChallenges::Instance().GetVehicleUsed( index ) );
		Lua::AddDataMember( pState, "difficulty", Game::C_FriendChallenges::Instance().GetDifficulty( index ) );

		const FriendChallengeParameter& extra = Game::C_FriendChallenges::Instance().GetExtraChallenge( index );
		Lua::AddDataMember( pState, "extra_modifier", extra.mModifier );
		Lua::AddDataMember( pState, "extra_param", extra.mParamType );

		Lua::AddDataMember( pState, "rechallenge", Game::C_FriendChallenges::Instance().IsRechallenge( index ) );
	}
	
	return 1;
}

static S32 FriendDemand_GetResult( lua_State* pState )
{
	lua_newtable( pState );

	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();
	const FriendChallenge* pFC = C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	if ( pFC )
	{
		bool bPassedMedal = ( results.mRaceStatistics.mMedal >= pFC->mMedal );
		bool bPassedCriteria = Game::C_FriendChallenges::Instance().GetCriteriaResults();
		bool bPassedParam = Game::C_FriendChallenges::Instance().GetParamResults();

		Lua::AddDataMember( pState, "passed_state", bPassedMedal );
		Lua::AddDataMember( pState, "passed_criteria", bPassedCriteria );
		Lua::AddDataMember( pState, "passed_param", bPassedParam );
	}

	return 1;
}

static S32 FriendDemand_SetupRaceMode( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"FriendDemand_SetupRaceMode(index, friend_index) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	int friend_index=(int)lua_tonumber(pState,2);
		
	Game::C_FriendChallenges::Instance().AttemptingFriendDemand(index, friend_index);
	
	return 0;
}

static S32 FriendDemand_IsComplete( lua_State* pState )
{
	Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();

	lua_pushnumber( pState, levelResult.mFriendDemandComplete );
	
	return 1;
}

static S32 FriendDemand_ReAttempt( lua_State* pState )
{
	Game::C_FriendChallenges::Instance().ReAttemptFriendDemand();
	return 0;
}

static S32 FriendDemand_ReChallenge( lua_State* pState )
{
	Game::C_FriendChallenges::Instance().CreateReChallengeFriendDemand();
	return 0;
}

static S32 FriendDemand_PopulateChallengeText( lua_State* pState )
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(levelState.mRaceMode == E_RaceMode_BossBattle || levelState.mRaceMode == E_RaceMode_FriendDemand_BossBattle)
		C_FriendChallenges::Instance().SetFriendDemandName(UI::C_TextStringID( "FDE_BOSS" ).String(), false);
	else if(levelState.mRaceMode == E_RaceMode_Checkpoint || levelState.mRaceMode == E_RaceMode_FriendDemand_Checkpoint)
		C_FriendChallenges::Instance().SetFriendDemandName(UI::C_TextStringID( "FDE_CHECKPOINT" ).String(), false);
	else if(levelState.mRaceMode == E_RaceMode_Destruction || levelState.mRaceMode == E_RaceMode_FriendDemand_Destruction)
		C_FriendChallenges::Instance().SetFriendDemandName(UI::C_TextStringID( "FDE_DESTRUCTION" ).String(), false);
	else if(levelState.mRaceMode == E_RaceMode_StreetRace || levelState.mRaceMode == E_RaceMode_FriendDemand_StreetRace)
		C_FriendChallenges::Instance().SetFriendDemandName(UI::C_TextStringID( "FDE_RACE" ).String(), false);
	else
		C_FriendChallenges::Instance().SetFriendDemandName(UI::C_TextStringID( "CMN_NOWT" ).String(), false);

	C_FriendChallenges::Instance().SetFriendDemandDescription(UI::C_TextStringID( "FDE_DEFAULT_CHALLENGE_MESSAGE" ).String(), false);

	return 0;
}

static S32 FriendDemand_PopulateRechallengeText( lua_State* pState )
{
	C_FriendChallenges::Instance().SetFriendDemandDescription(UI::C_TextStringID( "FDE_DEFAULT_RECHALLENGE_MESSAGE" ).String(), false);
	return 0;
}

static S32 FriendDemand_GetParamCount( lua_State* pState )
{
	lua_pushnumber(pState, C_FriendChallenges::Instance().GetParamCount());
	return 1;
}

static S32 FriendDemand_GetFriendNewCount( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetFriendNewCount( friend_index ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	int count=0;

	for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
	{
		if(Game::C_FriendsManager::Instance().GetFriendProfileFromID(index) )
		{
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
			const bdOnlineUserID fc_id=Game::C_MessagingManager::Instance().GetMailHeader(i)->m_senderID;

			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(index);

#ifdef PLATFORM_PS3
			bool identical_ids=fc_id == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
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

	lua_pushnumber(pState, count);
	return 1;
}

static S32 FriendDemand_GetFriendActiveCount( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetFriendNewCount( friend_index ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	int count=0;

	for(int a=0;a<Game::MAX_FRIEND_DEMANDS;a++)
	{
		if(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID != 0)
		{
			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(index);

#ifdef PLATFORM_PS3
			bool identical_ids=Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a] == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
			if(!wcscmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids)
#else
			bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a]) ? true : false;
			if(!wcsicmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids)
#endif
			{
				count++;
			}
		}
	}

	lua_pushnumber(pState, count);
	return 1;
}

static S32 FriendDemand_IsThereNewMail( lua_State* pState )
{
	lua_pushboolean( pState, (Game::C_MessagingManager::Instance().GetNumberOfMails() || Game::C_MessagingManager::Instance().IsThereNewMail()) );

	return 1;
}

static S32 FriendDemand_IsThereNoMail( lua_State* pState )
{
	lua_pushboolean( pState, (Game::C_MessagingManager::Instance().GetNumberOfMails() == 0 && Game::C_MessagingManager::Instance().IsThereNewMail() == false) );

	return 1;
}

static S32 FriendDemand_Accepted( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_Accepted( friend_index ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	C_FriendChallenges::Instance().AcceptDemand(index);

	return 0;
}

static S32 FriendDemand_GetTaleOfTheTape( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetTaleOfTheTape( friend_index ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	Game::C_FriendChallenges::Instance().GetFriendStats(index);

	return 0;
}

static S32 FriendDemand_GetTaleOfTheTapeData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetTaleOfTheTapeData( friend_index ) - incorrect arguments" );
		return 0;
	}

	S32 friend_index = (S32)lua_tonumber( pState, 1 );
	S32 stats_slot = Game::C_FriendChallenges::Instance().GetStatSlot( friend_index );

	if(stats_slot != -1)
	{
		lua_newtable( pState );

		Lua::AddDataMember( pState, "sent_you", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mSent );
		Lua::AddDataMember( pState, "sent_them", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mSent );

		Lua::AddDataMember( pState, "won_you", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mCompleted );
		Lua::AddDataMember( pState, "won_them", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted );

		Lua::AddDataMember( pState, "aces_you", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mKOs );
		Lua::AddDataMember( pState, "aces_them", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mKOs );

		Lua::AddDataMember( pState, "most_attempts_you", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mMostAttempts );
		Lua::AddDataMember( pState, "most_attempts_them", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mMostAttempts );

		U32 completed_you = Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mCompleted;
		U32 completed_them = Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted;

		Lua::AddDataMember( pState, "avg_attempts_you", ( completed_you != 0 ) ? ( Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[ stats_slot ].mTotalAttempts / completed_you ) : 0 );
		Lua::AddDataMember( pState, "avg_attempts_them", ( completed_them != 0 ) ? ( Game::C_FriendChallenges::Instance().mTaleOfTheTape.mTotalAttempts / completed_them  ) : 0 );
	}
	else
	{
		lua_pushnil(pState);
	}

	return 1;
}

static S32 FriendDemand_SortFriends( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_SortFriends( type ) - incorrect arguments" );
		return 0;
	}

	int type=(int)lua_tonumber(pState,1);

	Game::C_FriendChallenges::Instance().Sort((Game::FRIEND_CHALLENGE_SORT_TYPE)type);

	lua_newtable(pState);

	for(int a=0;a<Game::C_FriendChallenges::Instance().mFriendChallengeSorted.Size();a++)
	{
		lua_pushnumber(pState,Game::C_FriendChallenges::Instance().mFriendChallengeSorted[a].mFriendIndex);
		lua_rawseti(pState,-2,a+1);
	}

	return 1;
}

static S32 FriendDemand_Remove( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetTaleOfTheTape( friend_index ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	Game::C_FriendChallenges::Instance().mFriendDemandStore[index].mEventID=0;

	return 0;
}

static S32 FriendDemand_GetParamUsage( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FriendDemand_GetParamUsage( param ) - incorrect arguments" );
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	FRIEND_CHALLENGE_PARAM_USE usage=Game::C_FriendChallenges::Instance().GetParamUse(Game::C_FriendChallenges::Instance().GetParameter(index)->mParamType);

	lua_pushnumber(pState, (int)usage);

	return 1;
}

static S32 FriendDemand_ShowFriendGamerCard(lua_State* pState)
{
	return 0;
}

#if defined ( PLATFORM_PS3 )
	SceNpId gFriendDemandNpIds[MAX_FRIEND_DEMAND_FRIENDS];
#endif

static S32 FriendDemand_SendSystemMessage(lua_State* pState)
{
	static UI::C_TextStringID title_id( "FDE_FRIEND_DEMAND" );

#if defined ( PLATFORM_PS3_GOFISH )

	SceNpBasicMessageDetails msg;
	int ret;

	char mUtfTitle[SCE_NP_BASIC_SUBJECT_CHARACTER_MAX];
	char mUtfSubject[SCE_NP_BASIC_BODY_CHARACTER_MAX];

	memset(mUtfTitle, 0, sizeof(mUtfTitle));
	memset(mUtfSubject, 0, sizeof(mUtfSubject));

	int num_friends=0;
	for(int a=0;a<MAX_FRIEND_DEMAND_FRIENDS;a++)
	{
		int friend_index=Game::C_FriendChallenges::Instance().GetFriendSlot(a);

		if(friend_index != -1)
		{
			gFriendDemandNpIds[a]=C_FriendsManager::Instance().GetFriendProfileFromID(friend_index)->m_Uid;
			num_friends++;
		}
	}

	L10nResult res;
	size_t len  = SCE_NP_BASIC_SUBJECT_CHARACTER_MAX;

	size_t wlen = wcslen( title_id.String() );
	res = UCS2stoUTF8s( (uint16_t*)title_id.String(), &wlen, (uint8_t*)mUtfTitle, &len );

	len  = SCE_NP_BASIC_BODY_CHARACTER_MAX;
	wlen = wcslen( C_FriendChallenges::Instance().GetFriendDemandDescription() );
	res = UCS2stoUTF8s( (uint16_t*)C_FriendChallenges::Instance().GetFriendDemandDescription(), &wlen, (uint8_t*)mUtfSubject, &len );

	msg.msgId = 0;
	msg.mainType = SCE_NP_BASIC_MESSAGE_MAIN_TYPE_CUSTOM_DATA;
	msg.subType = SCE_NP_BASIC_MESSAGE_CUSTOM_DATA_SUBTYPE_ACTION_USE;
	msg.msgFeatures = SCE_NP_BASIC_MESSAGE_FEATURES_BOOTABLE | SCE_NP_BASIC_MESSAGE_FEATURES_EXP_MIN(60*24*7);
	msg.npids = gFriendDemandNpIds;
	msg.count = num_friends;
	msg.subject = &mUtfTitle[0];
	msg.body = &mUtfSubject[0];
	msg.data = &C_FriendChallenges::Instance().GetFriendDemand(-1)->mKey[0];
	msg.size = sizeof(BYTE) * Game::MAX_FRIEND_DEMAND_KEY;

	ret = sceNpBasicSendMessageGui(&msg, SYS_MEMORY_CONTAINER_ID_INVALID);

	if ( ret < 0 ) 
	{
		printf("%x", ret);
	}
#endif
	return 0;
}

static S32 FriendDemand_GetKeyedIndex( lua_State* pState )
{
	bool from_server=true;
	int index=C_FriendChallenges::Instance().GetKeyedServerIndex();

	if(index == -1)	//check local
	{
		from_server=false;
		index=C_FriendChallenges::Instance().GetKeyedLocalIndex();
	}

	lua_pushnumber(pState, index);
	lua_pushboolean(pState, from_server);

	return 2;
}

static S32 FriendDemand_GetReChallengeFriend( lua_State* pState )
{
	lua_pushnumber(pState, C_FriendChallenges::Instance().GetFriendSlot(0));
	return 1;
}

static S32 FriendDemand_HasPrivilege( lua_State* pState )
{
	const U8 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(padIndex);

	if((fp.m_ViewUserContentPrivilege | fp.m_ViewUserContentFriendsOnlyPrivilege) && (fp.m_CommunicationsPrivilege | fp.m_CommunicationsFriendsOnlyPrivilege))
		lua_pushboolean(pState, true);
	else
		lua_pushboolean(pState, false);

	return 1;
}

static S32 FriendDemand_HasOldChallenges( lua_State* pState )
{
	lua_pushnumber(pState, C_FriendChallenges::Instance().HaveWeOldChallenges());
	return 1;
}

static S32 FriendDemand_CleanOutOldChallenges( lua_State* pState )
{
	C_FriendChallenges::Instance().CleanOutOldChallenges();
	return 0;
}

static S32 FriendDemand_ReadFinished( lua_State* pState )
{
	bool finished = false;

	Game::C_MessagingManager& messagingManager = Game::C_MessagingManager::Instance();

	if(messagingManager.GetCurrentTask() == Game::MESSAGING_TASK_TYPE_NOTHING)
	{
		finished = true;
	}

	lua_pushboolean( pState, finished );		

	return 1;
}

static S32 FriendDemand_NewFriendDemandsRecieved( lua_State* pState )
{
	bool newFriendDemandsRecieved = false;

	for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
	{
		Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
		//dont want to check re challenges here
		if(fc->mReChallenge == false)
		{
			newFriendDemandsRecieved = true;
		}		
	}

	lua_pushboolean( pState, newFriendDemandsRecieved );

	return 1;
}

static S32 FriendDemand_NewFriendDemandsReChallengeRecieved( lua_State* pState )
{
	bool newFriendDemandsReChallengeRecieved = false;

	for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
	{
		Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
		//dont want to check re challenges here
		if(fc->mReChallenge == true)
		{
			newFriendDemandsReChallengeRecieved = true;		
		}		
	}

	lua_pushboolean( pState, newFriendDemandsReChallengeRecieved );

	return 1;
}

static S32 FriendDemand_GetNewFriendDemandsRecieved( lua_State* pState )
{
	bool newFriendDemandsRecieved = false;

	lua_newtable(pState);
	int index=1;

	for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
	{
		Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
			
		//dont want to check re challenges here
		if(fc->mReChallenge == false)
		{
			lua_newtable(pState);
					
			Lua::AddDataMember(pState,"index", i);
			Lua::AddDataMember(pState,"type", Game::AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE);
			
			lua_rawseti(pState,-2,index);
			index++;			
		}		
	}

	return 1;
}

static S32 FriendDemand_GetNewFriendDemandsReChallengeRecieved( lua_State* pState )
{
	bool newFriendDemandsReChallengeRecieved = false;

	lua_newtable(pState);
	int index=1;

	for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
	{
		Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
		//dont want to check re challenges here
		if(fc->mReChallenge == true)
		{
			lua_newtable(pState);
					
			Lua::AddDataMember(pState,"index", i);
			Lua::AddDataMember(pState,"type", Game::AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE);
			
			lua_rawseti(pState,-2,index);
			index++;				
		}		
	}

	return 1;
}

static S32 FriendDemand_QuitAttempt( lua_State* pState )
{
	Game::C_FriendChallenges::Instance().FinishedAttempt(false);
	return 0;
}

static S32 FriendDemand_CheckingName( lua_State* pState )
{
	lua_pushboolean( pState, false );
	return 1;
}

static S32 FriendDemand_CheckingDesc( lua_State* pState )
{
	lua_pushboolean( pState, false );
	return 1;
}


/**********************************************************************************************************************************************************************/
/**********************************************************************************************************************************************************************/
static const luaL_reg friend_demand_lib[] = 
{
	{"GetNumActiveSwitches",					FriendDemand_GetNumActiveSwitches},

	{"GetSwitch",								FriendDemand_GetSwitch},
	{"ChangeSwitch",							FriendDemand_ChangeSwitch},

	{"GetParamType",							FriendDemand_GetParamType},
	{"Create",									FriendDemand_Create},


	{"StoreName",								FriendDemand_StoreName},
	{"StoreDescription",						FriendDemand_StoreDescription},

	{"AddFriend",								FriendDemand_AddFriend},
	{"IsFriend",								FriendDemand_IsFriend},
	{"GetNumFriends",							FriendDemand_GetNumFriends},

	{"RequestRemotePicture",					FriendDemand_RequestRemotePicture},

	{"Send",									FriendDemand_Send},
	{"PumpSend",								FriendDemand_PumpSend},

	{"RetrieveFromServer",						FriendDemand_RetrieveFromServer},

	{"GetSlotData",								FriendDemand_GetSlotData},
	{"GetFreeSlot",								FriendDemand_GetFreeSlot},
	{"SaveMessageToSlot",						FriendDemand_SaveMessageToSlot},
	{"RemoveMessageFromSlot",					FriendDemand_RemoveMessageFromSlot},

	{"NumFriendDemandsForEvent",				FriendDemand_NumFriendDemandsForEvent},

	{"DeleteMessage",							FriendDemand_DeleteMessage},
	{"MessagesStillOnServer",					FriendDemand_MessagesStillOnServer},

	{"GetInfo",									FriendDemand_GetInfo},
	{"GetResult",								FriendDemand_GetResult},

	{"SetupRaceMode",							FriendDemand_SetupRaceMode},

	{"IsComplete",								FriendDemand_IsComplete},
	{"ReAttempt",								FriendDemand_ReAttempt},
	{"ReChallenge",								FriendDemand_ReChallenge},

	{"PopulateChallengeText",					FriendDemand_PopulateChallengeText},
	{"PopulateRechallengeText",					FriendDemand_PopulateRechallengeText},

	{"GetParamCount",							FriendDemand_GetParamCount},

	{"GetFriendNewCount",						FriendDemand_GetFriendNewCount},
	{"GetFriendActiveCount",					FriendDemand_GetFriendActiveCount},
	{"IsThereNewMail",							FriendDemand_IsThereNewMail},
	{"IsThereNoMail",							FriendDemand_IsThereNoMail},

	{"Accepted",								FriendDemand_Accepted},

	{"GetTaleOfTheTape",						FriendDemand_GetTaleOfTheTape},
	{"GetTaleOfTheTapeData",					FriendDemand_GetTaleOfTheTapeData},

	{"SortFriends",								FriendDemand_SortFriends},
	{"Remove",									FriendDemand_Remove},

	{"GetParamUsage",							FriendDemand_GetParamUsage},

	{"ShowFriendGamerCard",						FriendDemand_ShowFriendGamerCard},

	{"SendSystemMessage",						FriendDemand_SendSystemMessage},
	
	{"GetKeyedIndex",							FriendDemand_GetKeyedIndex},

	{"GetReChallengeFriend",					FriendDemand_GetReChallengeFriend},

	{"HasPrivilege",							FriendDemand_HasPrivilege},

	{"HasOldChallenges",						FriendDemand_HasOldChallenges},
	{"CleanOutOldChallenges",					FriendDemand_CleanOutOldChallenges},

	{"ReadFinished",							FriendDemand_ReadFinished},
	{"NewFriendDemandsRecieved",				FriendDemand_NewFriendDemandsRecieved},
	{"GetNewFriendDemandsRecieved",				FriendDemand_GetNewFriendDemandsRecieved},
	{"NewFriendDemandsReChallengeRecieved",		FriendDemand_NewFriendDemandsReChallengeRecieved}, 
	{"GetNewFriendDemandsReChallengeRecieved",	FriendDemand_GetNewFriendDemandsReChallengeRecieved},

	{"QuitAttempt",								FriendDemand_QuitAttempt},

	{"CheckingName",							FriendDemand_CheckingName},
	{"CheckingDesc",							FriendDemand_CheckingDesc},

	{NULL,						NULL}		// end of list
};  

LUALIB_API int luaopen_friend_demand(lua_State *pState)
{
	luaL_openlib(pState,"FriendDemand",friend_demand_lib,0);
	return 1;
}

} // end namespace Game

