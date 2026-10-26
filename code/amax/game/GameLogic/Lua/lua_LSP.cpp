#include "stdafx.h"

#include <Game/Game.h>
#include <ui/Game/UILua.h>

#include <GameApp.h>

#include <ContentServer/ContentServerManager.h>

#include <Network/Core.h>
#include <Network/DemonwareTask.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>

#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include "Ui/UiHelpers.h"
#include "UI/Game/Multiplayer/MultiplayerCache.h"
#include "Ui/Game/Multiplayer/LspData.h"

#include <Gamelogic\Photomode\PhotomodeManager.h>
#include "GameLogic/Profile/MultiplayerProfile.h"
#include <GameLogic\Profile\GameProfileManager.h>

#include <jpeghelper/JpegDecoder.h>
#include <jpeghelper/JpegEncoder.h>
#include "AppPhotoCapture.h"

#include <ui\Game\UILocaliser.h>
#include <system/time/time.h>

#include <GameLogic\ContentServer\LinkCodeManager.h>

extern Debug::Logger &gLUAMessageLogger;

namespace Game
{

static int Enable(lua_State* pState)
{
	bool ok = false;
	if (lua_isboolean(pState, 1))
		ok = C_ContentServerManager::Instance().Enable(lua_toboolean(pState, 1) ? true : false);

	lua_pushboolean(pState, ok);
	return 1;
}

static int SetFileName( lua_State* pState )
{
	if(!lua_isstring(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::SetFileName(name) - incorrect arguments\n");
		return 0;
	}

	C_ContentServerManager::Instance().SetFileName(lua_tostring(pState,1));

	return 0;
} 

static int SetFileSlot( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::SetFileSlot(slot) - incorrect arguments\n");
		return 0;
	}

	C_ContentServerManager::Instance().SetSlot((int)lua_tonumber(pState,1));

	return 0;
} 

static int SetUserIndex( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::SetTags(num,...) - incorrect arguments\n");
		return 0;
	}

	U8 num=(U8)lua_tointeger(pState,1);

	C_ContentServerManager::Instance().SetUserIndex(num);

	return 0;
} 

static int UploadCurrentPhoto( lua_State* pState )
{
	bool ok=C_PhotomodeManager::Instance().UploadPhoto();

	lua_pushboolean(pState,ok);

	return 1;
}

static int PumpCurrentTask( lua_State* pState )
{
	int error=BD_NO_ERROR;

	bool still_pumping=C_ContentServerManager::Instance().PumpCurrentTask(&error);

	lua_pushboolean(pState,still_pumping);
	lua_pushnumber(pState,error);

	return 2;
} 

static int CheckCurrentTask( lua_State* pState )
{
	int error=BD_NO_ERROR;
	bool stillWorking = false;

	C_ContentServerManager::Instance().GetCurrentTaskInfo(stillWorking, error);

	lua_pushboolean(pState,stillWorking);
	lua_pushnumber(pState,error);

	return 2;
}

static int RemoveFile( lua_State* pState )
{
	bool ok=C_ContentServerManager::Instance().RemoveFile();

	lua_pushboolean(pState,ok);

	return 1;
} 

static int KillCurrentTask( lua_State* pState )
{
	C_ContentServerManager::Instance().KillCurrentTask();
	return 0;
} 


static int SetTimeOut( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::SetTimeOut(seconds) - incorrect arguments\n");
		return 0;
	}

	float seconds=(float)lua_tonumber(pState,1);

	C_ContentServerManager::Instance().SetTimeOut(seconds);
	
	return 0;
}

static int SearchPhotos( lua_State* pState )
{
	C_ContentServerManager::Instance().SearchByTags();

	return 0;
}

static int ListFilesByOwner( lua_State* pState )
{
	C_ContentServerManager::Instance().ListFileByOwner(0,CONTENT_SERVER_TYPE_PHOTO,false);

	return 0;
}

static int ClearTags( lua_State* pState )
{
	C_ContentServerManager::Instance().ClearTags();
	return 0;
}

static int AddTag( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"LSP::AddTag(primary, secondary) - incorrect arguments\n");
		return 0;
	}

	bdUInt32 primary_tag=(bdUInt32)lua_tointeger(pState,1);
	bdUInt32 secondary_tag=(bdUInt32)lua_tointeger(pState,2);

	bdTag tag;
	tag.m_priTag=primary_tag;
	tag.m_secTag=secondary_tag;
	C_ContentServerManager::Instance().AddTag(tag);

	return 0;
}

static int GetFileMetaData( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::GetFileMetaData(search_index) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	bdFileID file_ID=C_ContentServerManager::Instance().GetSearchResultFileID(index);

	C_ContentServerManager::Instance().GetMetaDataFromID(file_ID);

	return 0;
}

static int DownloadPhoto( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::DownloadPhoto(index) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);

	C_PhotomodeManager::Instance().DownloadPhoto(index);
	return 0;
}

static int DecodeDownloadedPhoto( lua_State* pState )
{
	C_PhotomodeManager::Instance().DecodePhotoBuffer();
	return 0;
}

static int SetInPhotoUI( lua_State* pState )
{
	if(!lua_isboolean(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::SetInPhotoUI(bool) - incorrect arguments\n");
		return 0;
	}

	C_PhotomodeManager::Instance().InUI(lua_toboolean(pState,1) ? true : false);

	return 0;
}

static int GetMetaDataFromSearch( lua_State* pState )
{
	C_ContentServerManager::Instance().GetMetaDataFromPreviousSearch();
	return 0;
}

static int GetReturnedSearchData( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_ContentServerManager::Instance().GetNumSearchResults(); i++)
	{
		bdFileMetaData *md=C_ContentServerManager::Instance().GetMetaData(i);

		lua_newtable(pState);
			Lua::AddDataMember(pState,"index", i);
		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int GetReturnedSlotData( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;

	for(int a=1;a<11;a++)		//we don't use 0, its been reserved for facebook
	{
		int has_photo=-1;
		for(S32 i = 0; i < C_ContentServerManager::Instance().GetNumSearchResults(); i++)
		{
			bdFileMetaData *md=C_ContentServerManager::Instance().GetMetaData(i);

			if(md->m_fileSlot == a)
			{
				has_photo=i;
			}
		}

		lua_newtable(pState);
			Lua::AddDataMember(pState,"empty", ( has_photo == -1 ));
			Lua::AddDataMember(pState,"index", a );
		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int Vote( lua_State* pState )
{
	if(!lua_isnumber(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::Vote(vote) - incorrect arguments\n");
		return 0;
	}

	U64 id=C_ContentServerManager::Instance().GetDownloadFileID();
	U8 vote=(U8)lua_tointeger(pState,1);

	Game::C_GameProfile *prof=Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	//test to see if you've already voted on this content
	bool already_voted=false;
	for(int a=0;a<MAX_PROFILE_VOTE_BUFFER;a++)
	{
		if(prof->mPhotoVotes[a] == id)
		{
			already_voted=true;
			break;
		}
	}
	if(already_voted)
	{
		lua_pushboolean(pState, false);
		return 1;
	}

	C_ContentServerManager::Instance().Vote(id,vote);

	//save away your vote, so you can't vote again
	prof->mPhotoVotes[prof->mPhotoVoteIndex]=id;
	prof->mPhotoVoteIndex++;
	if(prof->mPhotoVoteIndex == MAX_PROFILE_VOTE_BUFFER)
	{
		prof->mPhotoVoteIndex=0;
	}

	lua_pushboolean(pState, true);

	return 1;
}

static int GetTop20Leaderboard( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isboolean(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"LSP::GetTop20Leaderboard(type,votes) - incorrect arguments\n");
		return 0;
	}

	int type=(int)lua_tonumber(pState,1);
	bool votes=(int)lua_toboolean(pState,2) ? true : false;

	C_ContentServerManager::Instance().GetLeaderboard(votes, (CONTENT_SERVER_TYPE)type);

	return 0;
}

static int GetReturnedLeaderboardData( lua_State* pState )
{
	if(!lua_isboolean(pState,1))
	{
		LOGINFO( gLUAMessageLogger,"LSP::GetReturnedLeaderboardData(votes) - incorrect arguments\n");
		return 0;
	}

	bool votes=(int)lua_toboolean(pState,1) ? true : false;

	lua_newtable(pState);
	int index=1;

	for(S32 i = 0; i < C_ContentServerManager::Instance().GetNumLeaderboardRows(); i++)
	{
		if(votes)
		{
			csVotingStatsInfo *si=C_ContentServerManager::Instance().GetVotingLeaderboardRow(i);

			lua_newtable(pState);
				Lua::AddDataMember(pState,"index", i);
			lua_rawseti(pState,-2,index);
			index++;
		}
		else
		{
			bdContentStreamingStatsInfo *si=C_ContentServerManager::Instance().GetLeaderboardRow(i);

			lua_newtable(pState);
				Lua::AddDataMember(pState,"index", i);
			lua_rawseti(pState,-2,index);
			index++;
		}
	}

	return 1;
}


static int GetMetaDataFromLeaderboardIndex( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isboolean(pState,2))
	{
		LOGINFO( gLUAMessageLogger,"LSP::GetMetaDataFromLeaderboard(index, votes) - incorrect arguments\n");
		return 0;
	}

	int index=(int)lua_tonumber(pState,1);
	bool votes=(int)lua_toboolean(pState,2) ? true : false;

	if(votes)
	{
		csVotingStatsInfo *si=C_ContentServerManager::Instance().GetVotingLeaderboardRow(index);
		bdFileID fileID;
		fileID.m_fileID=si->m_entityID;
		C_ContentServerManager::Instance().GetMetaDataFromID(fileID);
	}
	else
	{
		bdContentStreamingStatsInfo *si=C_ContentServerManager::Instance().GetLeaderboardRow(index);
		bdFileID fileID;
		fileID.m_fileID=si->m_entityID;
		C_ContentServerManager::Instance().GetMetaDataFromID(fileID);
	}

	return 0;
}


static S32 GetNumSearchResults( lua_State* pState )
{
	lua_pushnumber( pState, C_ContentServerManager::Instance().GetNumSearchResults() );
	return 1;
}


static S32 GetNumDownloadedFiles( lua_State* pState )
{
	lua_pushnumber( pState, C_ContentServerManager::Instance().GetNumDownloadedFiles() );
	return 1;
}


static S32 UploadUserEvent( lua_State* pState )
{
	/*
	const S32 size = sizeof( UI::LspEvent );

	// Maximum size is 8k
	BIZ_ASSERT_MSG( size < 1024 * 8, "Event size %i must not exceed 8k", size );

	Game::C_ContentServerManager::Instance().ClearTags();

	UI::LspEvent* pEvent = UI::C_MultiplayerCache::Instance().DecodeEvent( UI::C_UserEventManager::Instance().GetCurrentEvent() );

	bdTag tag;
	tag.m_priTag=Game::CONTENT_SERVER_TAGS_EVENT_CONTENT_TYPE;
	tag.m_secTag=Game::CONTENT_SERVER_TYPE_EVENT;
	Game::C_ContentServerManager::Instance().AddTag(tag);

	Game::C_ContentServerManager::Instance().UploadData( pEvent, sizeof( UI::LspEvent ), NULL, 0, Game::CONTENT_SERVER_TYPE_EVENT);
	*/
	return 0;
}


static S32 FinishUserEventUpload( lua_State* pState )
{
//	UI::C_UserEventManager::Instance().TagEvent( Game::C_ContentServerManager::Instance().GetUploadFileID().m_fileID );

	return 0;
}


static S32 SearchEvents( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SearchEvents( num(filter) ) - incorrect arguments\n" );
		return 0;
	}

	const UI::E_EventFilter filter = (UI::E_EventFilter)(S32)lua_tonumber( pState, 1 );

	switch( filter )
	{
		case UI::E_EventFilter_HighestRated :
		{
			UI::C_MultiplayerCache::Instance().ClearEventCache();
			C_ContentServerManager::Instance().GetLeaderboard( false, CONTENT_SERVER_TYPE_EVENT);
			break;
		}
		case UI::E_EventFilter_Newest :
		{
			UI::C_MultiplayerCache::Instance().ClearEventCache();
			C_ContentServerManager::Instance().SearchByTags();
			break;
		}
		default :
		{
			BIZ_ASSERT_FAIL( "Unsupported event filter" );
			break;
		}
	}
	return 0;
}

static S32 GetNumSearchEventsResult( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetNumSearchEventsResult( num(filter) ) - incorrect arguments\n" );
		return 0;
	}

	const UI::E_EventFilter filter = (UI::E_EventFilter)(S32)lua_tonumber( pState, 1 );
	S32 results = 0;	

	switch( filter )
	{
		case UI::E_EventFilter_HighestRated :
		{
			results = C_ContentServerManager::Instance().GetNumLeaderboardRows();
			break;
		}
		case UI::E_EventFilter_Newest :
		{
			results = C_ContentServerManager::Instance().GetNumSearchResults();
			break;
		}
		default :
		{
			BIZ_ASSERT_FAIL( "Unsupported event filter" );
			break;
		}
	}

	lua_pushnumber( pState, results );
	return 1;
}

static S32 GetEventMetaData( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetEventMetaData( num(filter) ) - incorrect arguments\n" );
		return 0;
	}

	const UI::E_EventFilter filter = (UI::E_EventFilter)(S32)lua_tonumber( pState, 1 );

	switch( filter )
	{
		case UI::E_EventFilter_HighestRated :
		{
			const S32 size = ( C_ContentServerManager::Instance().GetNumLeaderboardRows() < MAX_EVENT_CACHE ) ? C_ContentServerManager::Instance().GetNumLeaderboardRows() : MAX_EVENT_CACHE;

			// Add leaderboard entry ids to the array and pass them to the content server so we can request the data.
			U64 fileIDs[MAX_EVENT_CACHE];
			for ( S32 i = 0; i < size; i++ )
				fileIDs[i] = C_ContentServerManager::Instance().GetLeaderboardRow( i )->m_entityID;

			C_ContentServerManager::Instance().GetMetaDataFromID( size, fileIDs );
			break;
		}
		case UI::E_EventFilter_Favourties :
		{
			//Need a profile
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			BIZ_ASSERT_MSG( pGameProfile, "No game profile" );

			Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
			Game::FavouriteGroups& favourties = pMultiplayerProfile->GetFavouriteEvents();

			// Add all recent event ids to the array and pass them to the content server so we can request the data.
			U64 fileIDs[MAX_RECENT_EVENTS];
			for ( S32 i = 0; i < favourties.Size(); i++ )
				fileIDs[i] = favourties.GetFavouriteGroupByIndex( i );

			C_ContentServerManager::Instance().GetMetaDataFromID( favourties.Size(), fileIDs );
			break;
		}
		case UI::E_EventFilter_Newest :
		{
			C_ContentServerManager::Instance().GetMetaDataFromPreviousSearch();
			break;
		}
		case UI::E_EventFilter_RecentlyPlayed :
		{
			//Need a profile
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
			BIZ_ASSERT_MSG( pGameProfile, "No game profile" );

			Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
			Game::RecentEventsStore& recentEvents = pMultiplayerProfile->GetRecentEvents();

			// Add all recent event ids to the array and pass them to the content server so we can request the data.
			U64 fileIDs[MAX_RECENT_EVENTS];
			for ( S32 i = 0; i < recentEvents.Size(); i++ )
				fileIDs[i] = recentEvents[i];

			C_ContentServerManager::Instance().GetMetaDataFromID( recentEvents.Size(), fileIDs );
			break;
		}
		default :
		{
			BIZ_ASSERT_FAIL( "Unsupported event filter" );
			break;
		}
	}
	return 0;
}

static S32 DownloadUserEvents( lua_State* pState )
{
	//C_ContentServerManager::Instance().DownloadThumbnailData( UI::C_MultiplayerCache::Instance().LspBuffer(), sizeof( UI::LspEvent ), C_ContentServerManager::Instance().GetNumSearchResults() );

	//C_ContentServerManager::Instance().DownloadData( UI::C_MultiplayerCache::Instance().LspBuffer(), sizeof( UI::LspEvent ), C_ContentServerManager::Instance().GetNumSearchResults() );

	return 0;
}


static S32 CanVoteOnUserEvent( lua_State* pState )
{
	/*
	const UI::C_MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	BIZ_ASSERT( pEvent );

	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT( pProfile );

	// Check we havent already voted on this event
	//test to see if you've already voted on this content
	bool can_vote = true;
	for(int i = 0; i < MAX_PROFILE_VOTE_BUFFER; i++)
	{
		if( pProfile->mEventVotes[i] == pEvent->ID() )
		{
			can_vote=false;
			break;
		}
	}

	lua_pushboolean( pState, can_vote );
	return 1;
	*/
	return 0;
}

static S32 VoteUserEvent( lua_State* pState )
{
	/*
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "VoteUserEvent( num(vote) ) - incorrect arguments\n" );
		return 0;
	}	

	const UI::C_MultiplayerEvent* pEvent = UI::C_MultiplayerCache::Instance().CurrentEvent();
	BIZ_ASSERT( pEvent );

	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT( pProfile );

	if ( pEvent && pProfile )
	{
		const U8 vote = Maths::Clamp( (U8)lua_tointeger( pState, 1 ) * 2, 0, 10 );

		C_ContentServerManager::Instance().Vote( pEvent->ID(), vote, true);

		pProfile->mEventVotes[pProfile->mEventVoteIndex] = pEvent->ID();
		pProfile->mEventVoteIndex++;

		// Wrap back around
		if(pProfile->mEventVoteIndex == MAX_PROFILE_VOTE_BUFFER)
			pProfile->mEventVoteIndex = 0;
	}
	*/
	return 0;
}

static int GetLinkCode( lua_State* pState )
{
	Game::C_LinkCodeManager::Instance().SetUserIndex(SysProfile::C_Manager::Instance().GetPrimaryPadNo());
	Game::C_LinkCodeManager::Instance().GetLinkCode();

	return 0;
}

Debug::Bool g_debugContentServerOverideConnections("ContentServer.OverideConnections", false);
Debug::Bool g_debugContentServerIsConnected("ContentServer.IsContentServerConnected", true);
Debug::Bool g_debugContentServerVotingConnected("ContentServer.IsVotingServerConnected", true);
Debug::Bool g_debugTwitterConnected("ContentServer.IsTwitterConnected", true);
Debug::Bool g_debugStatsConnected("ContentServer.AreScoreboardsConnected", true);
Debug::Bool g_debugFacebookConnected("ContentServer.IsFacebookConnected", true);

static int IsConnected(lua_State* pState)
{
	bool connected=true;

	if (!RacerNet::C_NetServices::Instance().IsInitialised())
	{
		connected=false;
	}

	if(RacerNet::C_NetServices::Instance().Router().GetContentStreamingServer() == NULL)
	{
		connected=false;
	}

#ifndef CFG_GOLD
	if(g_debugContentServerOverideConnections)
	{
		connected=g_debugContentServerIsConnected;
	}
#endif

	lua_pushboolean(pState, connected);

	return 1;
}

static int IsBasicConnected(lua_State* pState)
{
	bool con = false;
	if (RacerNet::C_NetServices::Instance().IsInitialised())
	{
		// title utilities come from the basic demonware lobby connection, if we can't get this then we have no connection at all th the LSP.
		if(RacerNet::C_NetServices::Instance().Router().GetTitleUtilities() != NULL)
			con = true;
	}
	lua_pushboolean(pState, con);
	return 1;
}

static int IsVotingConnected(lua_State* pState)
{
	bool connected=true;

	if (!RacerNet::C_NetServices::Instance().IsInitialised())
	{
		connected=false;
	}

	if(RacerNet::C_NetServices::Instance().GetVoteRank() == NULL)
	{
		connected=false;
	}

#ifndef CFG_GOLD
	if(g_debugContentServerOverideConnections)
	{
		connected=g_debugContentServerVotingConnected;
	}
#endif

	//By Design - voting has now been removed
	connected=false;

	lua_pushboolean(pState, connected);

	return 1;
}

static int IsTwitterConnected(lua_State* pState)
{
	bool connected=true;

	if (!RacerNet::C_NetServices::Instance().IsInitialised())
	{
		connected=false;
	}

	if(RacerNet::C_NetServices::Instance().Router().GetTwitter() == NULL)
	{
		connected=false;
	}

#ifndef CFG_GOLD
	if(g_debugContentServerOverideConnections)
	{
		connected=g_debugTwitterConnected;
	}
#endif

	lua_pushboolean(pState, connected);

	return 1;
}

static int IsFacebookConnected(lua_State* pState)
{
	bool connected=true;

	if (!RacerNet::C_NetServices::Instance().IsInitialised())
	{
		connected=false;
	}

	if(RacerNet::C_NetServices::Instance().Router().GetFacebook() == NULL)
	{
		connected=false;
	}

#ifndef CFG_GOLD
	if(g_debugContentServerOverideConnections)
	{
		connected=g_debugFacebookConnected;
	}
#endif

	lua_pushboolean(pState, connected);

	return 1;
}

static int IsStatsConnected(lua_State* pState)
{
	bool connected=true;

	bdNetImpl *const net = bdNet::getInstance();
	bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
	if (NULL == lobbyService || net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
	{
		connected=false;
		lua_pushboolean(pState, connected);
		return 1;
	}

	bdStats* pStatsService = lobbyService->getStats();
	if(pStatsService == NULL)
	{
		connected=false;
	}

#ifndef CFG_GOLD
	if(g_debugContentServerOverideConnections)
	{
		connected=g_debugStatsConnected;
	}
#endif

	lua_pushboolean(pState, connected);

	return 1;
}


/**********************************************************************************************************************************************************************/
/**********************************************************************************************************************************************************************/
static const luaL_reg lsp_lib[] = 
{
	{"Enable",					Enable},
	{"SetFileName",				SetFileName},
	{"SetFileSlot",				SetFileSlot},
	{"SetUserIndex",			SetUserIndex},

	{"UploadCurrentPhoto",		UploadCurrentPhoto},
	{"UploadUserEvent",			UploadUserEvent},
	{"FinishUserEventUpload",	FinishUserEventUpload},

	{"PumpCurrentTask",			PumpCurrentTask},

	{"RemoveFile",				RemoveFile},
	{"KillCurrentTask",			KillCurrentTask},

	{"SetTimeOut",				SetTimeOut},

	{"ClearTags",				ClearTags},
	{"AddTag",					AddTag},

	{"SearchPhotos",			SearchPhotos},
	{"SearchEvents",			SearchEvents},
	{"ListFilesByOwner",		ListFilesByOwner},

	{"GetFileMetaData",			GetFileMetaData},
	{"GetEventMetaData",		GetEventMetaData},

	{"DownloadPhoto",			DownloadPhoto},
	{"DownloadUserEvents",		DownloadUserEvents},
	{"DecodeDownloadedPhoto",	DecodeDownloadedPhoto},
	{"SetInPhotoUI",			SetInPhotoUI},

	{"GetMetaDataFromSearch",	GetMetaDataFromSearch},
	{"GetReturnedSearchData",	GetReturnedSearchData},
	{"GetNumSearchResults",		GetNumSearchResults},
	{"GetNumDownloadedFiles",	GetNumDownloadedFiles},
	{"GetReturnedSlotData",		GetReturnedSlotData},
	{"GetNumSearchEventsResult",GetNumSearchEventsResult},

	{"Vote",					Vote},
	{"CanVoteOnUserEvent",		CanVoteOnUserEvent},
	{"VoteUserEvent",			VoteUserEvent},

	{"GetTop20Leaderboard",				GetTop20Leaderboard},
	{"GetReturnedLeaderboardData",		GetReturnedLeaderboardData},
	{"GetMetaDataFromLeaderboardIndex",	GetMetaDataFromLeaderboardIndex},

	{"IsConnected",				IsConnected},
	{"IsBasicConnected",		IsBasicConnected},

	{"IsVotingConnected",		IsVotingConnected},
	{"IsTwitterConnected",		IsTwitterConnected},
	{"IsFacebookConnected",		IsFacebookConnected},
	{"IsStatsConnected",		IsStatsConnected},

	{"GetLinkCode",				GetLinkCode},

	{NULL,						NULL}		// end of list
};  

LUALIB_API int luaopen_lsp(lua_State *pState)
{
	luaL_openlib(pState,"LSP",lsp_lib,0);
	return 1;
}

} // end namespace Game
