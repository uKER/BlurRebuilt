#include "stdafx.h"
#include <GameLogic/StickerManager.h>	// [Blur2PC]

#include <Game/Game.h>

#include <ui/Game/UILua.h>
#include <ui/Game/SaveSystem/SaveSystem.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include "ui/Game/Gadgets/Vinyl/VinylManager.h"
#include "ui/UiManager.h"

#include <GameLogic\Profile\GameProfileManager.h>

#include "RaceBook/RaceBookManager.h"

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameSystems.h>

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_MemoryStream.h>

#include "Ui/SoundEffects.h"
#include <Ui/UiInterface.h>
#include <UI/UiMessages.h>

#include <GameLogic\Twitter\TwitterManager.h>
#include <GameLogic\Facebook\FacebookManager.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>
#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include "Game\DLC\DLC_Controller.h"

#include <GameLogic\SocialNetworking\EventOwnerManager.h>
#include <GameLogic\SocialNetworking\FriendsManager.h>

void SP_ImportSaveGameLua(Sys::BinaryDeserialiserAutoEndian& s);
void SP_ExportSaveGameLua(Sys::BinarySerialiserAutoEndian& s);

#if 0
#define RECORDTIME() U64 timeStart = (U64)(Time::GetPerformanceCounter());
#define SHOWTIME(msg) do { U64 t = (U64)(Time::GetPerformanceCounter()); DebugLn("*** %s %i", msg, (DWORD)((t - timeStart) / 1000)); timeStart = t; } while(0)
#else
#define RECORDTIME()
#define SHOWTIME(msg)
#endif

UI::C_UILua* g_pGameProfileLua;

extern Debug::Logger &gLUAMessageLogger;
const char* saveGameFilename = "debug_save.amax";

#ifdef PLATFORM_PC
// [Blur2PC] 1374: the profile options carry retail Blur PC's keyboard options (T103). 1373 saves still load.
// [Blur2PC] 1375: and retail's audio quality byte (T144). 1373 and 1374 saves still load.
// [Blur2PC] 1378: retail Blur PC 1.2's format (T250; LoadGameImpl 0x00c1d4c0, SaveGameImpl 0x00c1c220): 0x170-byte
// profile options, the unlocked DLC packs after the multiplayer profile, no TU2 friend demand user ids at the end.
// Blur2PC's 1373-1375 saves still load.
const int saveGameVersion = 1378; // Increment if anything saved has changed.
static int s_LoadingSaveGameVersion = saveGameVersion;

static bool IsLoadableSaveGameVersion( const int version )
{
	return version == saveGameVersion || version == 1375 || version == 1374 || version == 1373;
}

const int GetLoadingSaveGameVersion()
{
	return s_LoadingSaveGameVersion;
}
#else
const int saveGameVersion = 1373; // Increment if anything saved has changed.
#endif

const int GetSaveGameVersion()
{
	return saveGameVersion;
}

Debug::Bool g_ForceProfileRebuild("Multiplayer.PrimaryProfile.ForceRebuild", false);

namespace Game
{
static bool SaveGameImpl( Sys::BinarySerialiserAutoEndian& serial );
static bool LoadGameImpl( Sys::BinaryDeserialiserAutoEndian& serial );
}

#include "SaveInfoInterfaces.inl"

namespace UI
{

//
static C_SaveInfoInterfaceProfile* s_pSaveInfoInterfaceProfile = NULL;
static C_SaveInfoInterfacePhotos*  s_pSaveInfoInterfacePhotos  = NULL;
//static C_SaveInfoInterfaceVinyl   s_SaveInfoInterfaceVinyl;
//

// ClearGameProfile
static S32 ClearGameProfile( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ClearGameProfile( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetProfile( sPadNo );

	if ( pProfile )
	{
		pProfile->Init(sPadNo);
		pProfile->SetSystemDefaults();

		// Need to clean up downloaded data in memory
		pProfile->ResetDownloadedMPProfile();
	}

	bool bProfileOnly = ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 2 ) != 0 ) : false;

	if ( bProfileOnly )
		return 0;

	// flush out single player stuff

	// TODO - these should really be profile specific rather than game specific
	// -- will cause problems if we don't force the player back to start screen for single player

	RaceBook::C_RaceBookManager::Instance().Init();
	//Game::C_InGameDialogueManager::Instance().InitProfiles();
	
	//C_UserEventManager::Instance().Destroy(); // doesnt actualy destroy it. just clears the events

	Game::C_StickerManager::Instance().ClearStickerProgress();
	Game::C_SocialNetworkManager::Instance().Clear();
	Game::C_FriendChallenges::Instance().Clear();
	Game::C_FriendsManager::Instance().Clear();
	Game::C_EventOwnerManager::Instance().Clear();

	return 0;
}

// SetSaveInfoInterfaceProfile
static S32 SetSaveInfoInterfaceProfile( lua_State* pState )
{
	if ( !s_pSaveInfoInterfaceProfile )
		s_pSaveInfoInterfaceProfile = new C_SaveInfoInterfaceProfile;

	if ( !s_pSaveInfoInterfaceProfile )
		return 0;

	bool bLoading = ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 1 ) != 0 ) : false;

	s_pSaveInfoInterfaceProfile->Setup( bLoading );

	C_SaveSystem::Instance().SaveInfoInterface( s_pSaveInfoInterfaceProfile );

	return 0;
}

// SetSaveInfoInterfacePhotos
static S32 SetSaveInfoInterfacePhotos( lua_State* pState )
{
	if ( !s_pSaveInfoInterfacePhotos )
		s_pSaveInfoInterfacePhotos = new C_SaveInfoInterfacePhotos;

	if ( !s_pSaveInfoInterfacePhotos )
		return 0;

	bool bLoading		= ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 1 ) != 0 )	 : false;
	S32  sEnumeration	= ( lua_type( pState, 2 ) == LUA_TNUMBER  ) ? ( (S32)lua_tonumber( pState, 2 ) - 1 ) : -1;

	s_pSaveInfoInterfacePhotos->Setup( bLoading, sEnumeration );

	C_SaveSystem::Instance().SaveInfoInterface( s_pSaveInfoInterfacePhotos );

	return 0;
}

};	// namespace UI

namespace Game
{

static bool SaveGameImpl( Sys::BinarySerialiserAutoEndian& serial )
{
	// I have started writing
	// Be aware that this is directly written over in SaveInfoInterfaces.inl
	U8 writing = 1;
	serial << writing;

	serial << saveGameVersion;

	C_GameProfile* gameProf = C_GameProfileManager::Instance().GetPrimaryProfile();	
	serial << *gameProf;

	SP_ExportSaveGameLua(serial);

	serial << Game::C_SocialNetworkManager::Instance().GetNumSocialItems();
	for( int i = 0; i < Game::C_SocialNetworkManager::Instance().GetNumSocialItems(); i++ )
	{
		serial << *Game::C_SocialNetworkManager::Instance().GetSocialItem(i);
	}

	serial << (S32)Game::C_FacebookManager::Instance().IsAuthenticated();
	serial << (S32)Game::C_TwitterManager::Instance().IsAuthenticated();

	serial.Write(&Game::C_FriendChallenges::Instance().mFriendDemandStore, sizeof(Game::FriendChallenge) * MAX_FRIEND_DEMANDS);

	serial.Write(&Game::C_SocialNetworkManager::Instance().mRaceHistoryStore, sizeof(Game::AmaxSocialNetworkRaceStore) * Game::MAX_SOCIAL_NETWORK_RACE_HISTORY);
	serial << Game::C_SocialNetworkManager::Instance().mRaceHistoryStoreIndex;

	serial.Write(&Game::C_FriendChallenges::Instance().mFriendDemandStatsStore, sizeof(Game::FriendChallengeStats) * MAX_FRIEND_DEMAND_STATS);

	serial << (U64)Game::C_EventOwnerManager::Instance().GetFriend();

	serial << gameProf->mRaceBookProfile->mLifetimeStatistics;

#ifndef PLATFORM_PC	// [Blur2PC] T250: retail PC's save ends at the lifetime statistics
	//new for TU2
	serial.Write(&Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore, sizeof(bdOnlineUserID) * MAX_FRIEND_DEMANDS);
#endif

	return true;
}

static int DebugSaveGame(lua_State* pState)
{
	LOGWARN( g_Log, DBSTR "Depracated" );
	return 0;
}

static bool LoadGameImpl( Sys::BinaryDeserialiserAutoEndian& serial )
{
	RECORDTIME();

	RaceBook::C_RaceBookManager::Instance().Init();

	SHOWTIME("Reset");

	// If this is 1, I've failed to finish writing.
	U8 writing;
	serial >> writing;

	if(writing == 1)
		return false;

	int currentVersion;
	serial >> currentVersion;
#ifdef PLATFORM_PC
	if(!IsLoadableSaveGameVersion(currentVersion) || g_ForceProfileRebuild)
#else
	if(currentVersion != saveGameVersion || g_ForceProfileRebuild)
#endif
	{
		LOGDEBUG(gLUAMessageLogger, "Local save game version is %i but your version is %i, so must recreate save.", saveGameVersion, currentVersion);
		return false;
	}
#ifdef PLATFORM_PC
	s_LoadingSaveGameVersion = currentVersion;
#endif

	C_GameProfile* gameProf = C_GameProfileManager::Instance().GetPrimaryProfile();
	serial >> *gameProf;

	SP_ImportSaveGameLua(serial);

	SHOWTIME("Social");

	Game::C_SocialNetworkManager::Instance().mSocialStore.Clear();
	int num_s;
	serial >> num_s;
	for( int i = 0; i < num_s; i++ )
	{
		Game::AmaxSocialNetworkItem s;
		serial >> s;
		Game::C_SocialNetworkManager::Instance().mSocialStore.PushBack(s);
	}

	S32 fb_auth;
	serial >> fb_auth;
	Game::C_FacebookManager::Instance().SetAuthenticated(fb_auth ? true : false);
	
	S32 tw_auth;
	serial >> tw_auth;
	Game::C_TwitterManager::Instance().SetAuthenticated(tw_auth ? true : false);

	serial.Read(&Game::C_FriendChallenges::Instance().mFriendDemandStore, sizeof(Game::FriendChallenge) * MAX_FRIEND_DEMANDS);

	serial.Read(&Game::C_SocialNetworkManager::Instance().mRaceHistoryStore, sizeof(Game::AmaxSocialNetworkRaceStore) * Game::MAX_SOCIAL_NETWORK_RACE_HISTORY);
	serial >> Game::C_SocialNetworkManager::Instance().mRaceHistoryStoreIndex;

	serial.Read(&Game::C_FriendChallenges::Instance().mFriendDemandStatsStore, sizeof(Game::FriendChallengeStats) * MAX_FRIEND_DEMAND_STATS);

	U64 id;
	serial >> id;
	Game::C_EventOwnerManager::Instance().SetFriend(id);

	serial >> gameProf->mRaceBookProfile->mLifetimeStatistics;

	//new for TU2
#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail PC's 1378 save has no user ids; Blur2PC's 1373-1375 saves do
	bool ok=false;
	if(currentVersion < 1378)
		ok=serial.Read(&Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore, sizeof(bdOnlineUserID) * MAX_FRIEND_DEMANDS);
#else
	bool ok=serial.Read(&Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore, sizeof(bdOnlineUserID) * MAX_FRIEND_DEMANDS);
#endif
	if(!ok)	//if its not Ok then clear out the array, in case crap has been written into it, this should only happen on the initial load when the TU has been applied
	{
		for(int a=0;a<MAX_FRIEND_DEMANDS;a++)
		{
			Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a]=0;
		}
	}


	SHOWTIME("Fin");

	UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_AudioOptionsSaved );

	return true;
}

static int DebugLoadGame(lua_State* pState)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* file = fs.Open(saveGameFilename, Sys::E_FileAccess_Read);
	if (file == 0)
	{
		LOGWARN(gLUAMessageLogger, "Save game not found: please press Right Trigger at the start screen to generate one.");
		return 0;
	}
	Sys::BinaryDeserialiserAutoEndian serial(*file);

	LoadGameImpl( serial );

	delete file;
	file = NULL;

	return 0;
}

static int InitPrimary(lua_State *pState)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->Init( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	return 0;
}

void InitGhostCars(lua_State *pState, C_LevelState* levelState)
{
	Lua::GetDataMember(pState,"NumGhosts",levelState->mGhostCarSettings.numGhostCars,true);
	Lua::GetDataMember(pState,"GhostWhen",(S8&)levelState->mGhostCarSettings.ghostCarWhen,true);
	Lua::GetDataMember(pState,"RespawnLength",levelState->mGhostCarSettings.respawnLength,true);
}

const int NUM_DRIVERS = 10; // Will be grabbed from the event eventually
void InitDamageSettings(lua_State *pState, C_LevelState* levelState)
{
	Lua::GetDataMember(pState,"FatalDamage",levelState->mDamageSettings.fatalDamage,true);

	int damage;
	for( int i = 1; i <= NUM_DRIVERS; i++ )
	{
		Util::C_String damageName = Util::C_String::Format("DamageDriver%d", i);

		Lua::GetDataMember(pState,damageName.Chars(),damage,true);
		levelState->mDamageSettings.startingDamage.PushBack(damage);
	}
}

void InitTeams( lua_State* pState, int numTeams, EventState* eventState)
{
	for( int i = 1; i <= numTeams; i++ )
	{
		AmaxRaceTeam* team = new AmaxRaceTeam();
		float colR, colG, colB;

		Util::C_String teamColR			= Util::C_String::Format("Team%iColourR",i);
		Util::C_String teamColG			= Util::C_String::Format("Team%iColourG",i);
		Util::C_String teamColB			= Util::C_String::Format("Team%iColourB",i);
		Util::C_String teamDelay		= Util::C_String::Format("Team%iDelay",i);
		Util::C_String teamNuMDrivers	= Util::C_String::Format("Team%iNumDrivers",i);

		Lua::GetDataMember(pState,teamColR.Chars(),colR,false);  
		Lua::GetDataMember(pState,teamColG.Chars(),colG,false);  
		Lua::GetDataMember(pState,teamColB.Chars(),colB,false); 

		team->colour = Maths::RGBA(colR, colG, colB);

		Util::C_String ruleName = Util::C_String::Format("Rule%d", i);
		Lua::GetDataMember(pState,teamDelay.Chars(),team->delay,false); 
		Lua::GetDataMember(pState,teamNuMDrivers.Chars(),team->numDrivers,false); 

		eventState->mTeams.PushBack(team);
	}

}

void SetCarClasses(lua_State* pState, C_LevelState* levelState, EventState* eventState)
{
	//int rNum;
	for( int i = 1; i <= eventState->mTeams.Size(); i++ )
	{
		Btl::Vector<AmaxCarClass> teamCarList;

		for ( int j = 1; j <= eventState->mTeams[i-1]->numDrivers; j++ )
		{
			AmaxCarClass carEnum;
			Util::C_String className = Util::C_String::Format("Team%dCarClass%d", i, j);
			Lua::GetDataMember(pState, className.Chars(), (S8&)carEnum, false);

			teamCarList.PushBack(carEnum);
		}
		levelState->mCarClassList.PushBack(teamCarList);
	}
}

static C_LevelState* CreateLevelState(lua_State* pState, EventState* eventState)
{
	C_LevelState* levelState = new C_LevelState();

	//int carClass, startType;

	Lua::GetDataMember(pState,"City",levelState->mCity,true);  
	Lua::GetDataMember(pState,"Route",levelState->mRoute,true);  
	Lua::GetDataMember(pState,"Laps",levelState->mLaps,true);	
	//Lua::GetDataMember(pState,"NumRules",numRules,true);	
	//Lua::GetDataMember(pState,"NumElimRules",numElimRules,true);	
	Lua::GetDataMember(pState,"Perks",(S8&)levelState->mUsePerks,true);
	Lua::GetDataMember(pState,"Qualify",(S8&)levelState->mQualify,true);
	Lua::GetDataMember(pState,"PointsBased",(S8&)levelState->mPointsBased,true);
	Lua::GetDataMember(pState,"StartType",(S8&)levelState->mStartType,true);

	InitGhostCars(pState, levelState);
	InitDamageSettings(pState, levelState);
	SetCarClasses(pState, levelState, eventState);

	//int rNum;
	//for( int i = 1; i <= numRules; i++ )
	//{
	//	Util::C_String ruleName = Util::C_String::Format("Rule%d", i);
	//	
	//	Lua::GetDataMember(pState,ruleName.Chars(),rNum,true);
	//	RACE_MODE_RULE rule = (RACE_MODE_RULE)(rNum - 1);

	//	RaceModeRuleBase* ruleMode = GetRuleFromEnum(rule, i, pState, "");
	//	levelState->mRules.PushBack(ruleMode);
	//}

	//for( int i = 1; i <= numElimRules; i++ )
	//{
	//	Util::C_String ruleName = Util::C_String::Format("ElimRule%d", i);

	//	Lua::GetDataMember(pState,ruleName.Chars(),rNum,true);
	//	RACE_MODE_RULE rule = (RACE_MODE_RULE)(rNum - 1);

	//	RaceModeRuleBase* ruleMode = GetRuleFromEnum(rule, i, pState, "Elim");
	//	levelState->mElimRules.PushBack(ruleMode);
	//}

	return levelState;
}

static int CreateEvent(lua_State *pState)
{
	if( lua_type( pState, 1 ) != LUA_TTABLE )
	{
		g_pGameProfileLua->StackDump(pState, "EventTable has incorrect arguments\n");
		return 0;
	}

	//int numTeams;
	//Lua::GetDataMember(pState,"NumTeams",numTeams,false);  
	//Lua::GetDataMember(pState,"NumDrivers",eventState->numDrivers,false);   
	//Lua::GetDataMember(pState,"TournamentType",(S8&)eventState->tournamentType,false); 

	//InitTeams(pState, numTeams, eventState);

	////pop off the vars so we can play with the table
	//lua_settop(pState,1);

	////lua_gettable( pState, -2);
	//S32 size = luaL_getn(pState,-1);

	//for ( U32 i = 1; i <= size; i++ )
	//{
	//	lua_rawgeti(pState,-1, i ); // Push on index [i]

	//	if(lua_type(pState, -1) == LUA_TTABLE) // Type check
	//	{     
	//		C_LevelState* levelState = CreateLevelState(pState, eventState);
	//		eventState->mRaceList.PushBack(levelState);
	//		lua_pop(pState,1); // Pop index [i] off
	//	}
	//}
	//lua_pop(pState,1); // Pop the effects table off

	//C_GameProfileManager::Instance().GetPrimaryProfile()->mUserEventList.PushBack(eventState);

	return 0;
}

static int HasPassedEvent(lua_State *pState)
{
	lua_pushboolean(pState, C_GameProfileManager::Instance().GetPrimaryProfile()->mPassedEvent);
	return 1;
}

static int RetryingEvent(lua_State *pState)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->RetryingEvent();
	return 0;
}

static int IsFinalEvent(lua_State *pState)
{
	//hack for 1st playable
	//if(C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEventID == 0x1DC2C36F)
	//	lua_pushboolean(pState, true);
	//else
		lua_pushboolean(pState, false);

	return 1;
}

static int RaceBookAppsLocked(lua_State* pState)
{
	const U8 app_locked = C_GameProfileManager::Instance().GetPrimaryProfile()->mAppsLocked;

	lua_newtable( pState );
	S32 count = 1;
	for ( S32 i = 0; i < UI::C_UiManager::E_RwCarouselBranch_Count; i++ )
	{
		if( (1 << i) & app_locked )
		{
			lua_pushnumber( pState, i );
			lua_rawseti( pState, -2, count );
			count++;
		}
	}

	return 1;
}

static int RaceBookUnlockApp(lua_State* pState)
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pGameProfileLua->StackDump(pState, "RaceBookUnlockApp( number ) has incorrect arguments\n");
		return 0;
	}

	S32 unlock_app = (S32)lua_tointeger( pState, 1 );

	// Return if it was previously locked and being cleared
	lua_pushboolean( pState, ( C_GameProfileManager::Instance().GetPrimaryProfile()->mAppsLocked & (1<<unlock_app) ) ? 1 : 0 );

	// Clear the unlock bit
	C_GameProfileManager::Instance().GetPrimaryProfile()->mAppsLocked &= ~(1<<unlock_app);
	
	return 1;
}


static int MoveToNextEvent(lua_State *pState)
{
	lua_pushboolean(pState, C_GameProfileManager::Instance().GetPrimaryProfile()->mLoadNextRace);

	return 1;
}

static int DeleteSaveGame(lua_State *pState)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

#ifndef PLATFORM_PS3
	fs.DeleteFileA(saveGameFilename);
#else
	fs.DeleteFile(saveGameFilename);
#endif

	return 0;
}

static int DoesSaveGameExist(lua_State *pState)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	lua_pushboolean(pState, fs.FileExists(saveGameFilename));
	return 1;
}

static int IsSaveGameValid(lua_State *pState)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();;
	Sys::I_File* file = fs.Open(saveGameFilename, Sys::E_FileAccess_Read);
	if (file == 0)
	{
		LOGWARN(gLUAMessageLogger, "Save game not found: please press Right Trigger at the start screen to generate one.");
		return 0;
	}
	Sys::BinaryDeserialiserAutoEndian serial(*file);

	int currentVersion;
	serial >> currentVersion;
#ifdef PLATFORM_PC
	if(IsLoadableSaveGameVersion(currentVersion))
#else
	if(currentVersion == saveGameVersion)
#endif
	{
		lua_pushboolean(pState,true);
	}
	else
	{
		LOGWARN(gLUAMessageLogger, "Save Game Versions Incompatible - Reseting Save");
		lua_pushboolean(pState,false);
	}

	delete file;
	file = NULL;

	return 1;
}

static int SeenHelpPopup( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pGameProfileLua->StackDump(pState, "SeenHelpPopup(num,[bool] has incorrect arguments\n");
		return 0;
	}

	const U32 num = (U32)lua_tointeger( pState, 1 );
	if( num >= 32 )
	{
		LOGERROR( g_Log, "SeenPopup index out of range" );
		return 0;
	}

	C_GameProfile* gameProf = C_GameProfileManager::Instance().GetPrimaryProfile();
	if( !gameProf )
	{
		LOGERROR( g_Log, "SeenPopup profile == NULL" );
		return 0;
	}

	const U32 param = (1<<num);

	// Passed a bool? Set the param
	if( lua_type( pState, 2 ) == LUA_TBOOLEAN )
	{
		if( lua_toboolean( pState, 2 ) == 0 )
			gameProf->mSeenHelpPopups &= ~param;	// Clear
		else
			gameProf->mSeenHelpPopups |= param;		// Set
	}

	// Return the param
	const int set = (gameProf->mSeenHelpPopups & param) ? 1 : 0;
	
	lua_pushboolean( pState, set );
	return 1;
}

static int ClearRaceDifficulty( lua_State* pState )
{
	C_GameProfile* pProfile = C_GameProfileManager::Instance().GetPrimaryProfile();
	
	if (pProfile)
		pProfile->ClearRaceDifficulty();

	return 0;
};

static S32 HasLinkCode( lua_State* pState )
{
	if(	C_GameProfileManager::Instance().GetPrimaryProfile()->mWebsiteLinkCode.Length())
		lua_pushboolean(pState, true);
	else
		lua_pushboolean(pState, false);

	return 1;
}

// StartDLC
static S32 StartDLC( lua_State* pState )
{
	UI::DLCController::Instance().Start( 0, 0, "DLC Controller" );
	return 0;
}

// ContinueDLC
static S32 ContinueDLC( lua_State* pState )
{
	bool bFinished = false;

	if ( !UI::DLCController::Instance().IsRunning() )
	{
		UI::DLCController::Instance().Join();	// wait for thread to terminate

#ifndef PLATFORM_PS3	// PS3 is moaning - linker error
		UI::DLCController::Instance().Close();	// close thread handle
#endif
		
		bFinished = true;
	}

	if ( bFinished )
	{
		lua_pushboolean( pState, true );
		lua_pushboolean( pState, UI::DLCController::Instance().HasErrors() );
	}
	else
	{
		lua_pushboolean( pState, false );
		lua_pushnil( pState );
	}

	return 2;
}

// GetResprays
static S32 GetResprays( lua_State* pState )
{
	lua_newtable( pState );

	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pProfile )
		return 1;

	bool bMultiplayer = ( ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 1 ) != 0 ) : false );

	Game::ResprayMap& map = ( bMultiplayer ? pProfile->mMultiplayerProfile.GetRespraysMP() : pProfile->m_Resprays_SP );

	for ( Game::ResprayMap::Iterator it = map.Begin(); it != map.End(); ++it )
	{
		lua_newtable( pState );
			Lua::AddDataMember( pState, "id_xt", it->GetValue().first );
			Lua::AddDataMember( pState, "index", it->GetValue().second );
		lua_rawseti( pState, -2, it->GetKey() );
	}

	return 1;
}

// SP_EquippedAbility
static S32 SP_EquippedAbility( lua_State* pState )
{
	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pProfile )
		return 0;

	if ( lua_type( pState, 1 ) == LUA_TNONE )
	{
		// get
		lua_pushnumber( pState, (S32)pProfile->mSpEquippedAbility );
		return 1;
	}

	// set
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SP_EquippedAbility( num ) - incorrect arguments" );
		return 0;
	}

	UI::E_MultiplayerVehicleAbilities eAbility = (UI::E_MultiplayerVehicleAbilities)(S32)lua_tonumber( pState, 1 );

	pProfile->mSpEquippedAbility = eAbility;

	return 0;
}

// SP_SeenGarageModPopup
static S32 SP_SeenGarageModPopup( lua_State* pState )
{
	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pProfile )
		return 0;

	if ( lua_type( pState, 1 ) == LUA_TNONE )
	{
		// get
		lua_pushnumber( pState, pProfile->mHasSeenGarageModPopup );
		return 1;
	}

	// set
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SP_SeenGarageModPopup( num ) - incorrect arguments" );
		return 0;
	}

	U8 uHasSeenGarageModPopup = (U8)lua_tonumber( pState, 1 );

	pProfile->mHasSeenGarageModPopup = uHasSeenGarageModPopup;

	return 0;
}

/**********************************************************************************************************************************************************************/
/**********************************************************************************************************************************************************************/
static const luaL_reg game_profile_lib[] = 
{
	{"InitPrimary",				InitPrimary},

	{"CreateEvent",				CreateEvent},

	{"DebugSaveGame",			DebugSaveGame},
	{"DebugLoadGame",			DebugLoadGame},

	{"HasPassedEvent",			HasPassedEvent},
	{"RetryingEvent",			RetryingEvent},

	{"IsFinalEvent",			IsFinalEvent},

	{"DeleteSaveGame",			DeleteSaveGame},
	{"DoesSaveGameExist",		DoesSaveGameExist},

	{"IsSaveGameValid",			IsSaveGameValid},
	{"MoveToNextEvent",			MoveToNextEvent},

	{"SeenHelpPopup",			SeenHelpPopup},
	{"RaceBookAppsLocked",		RaceBookAppsLocked},
	{"RaceBookUnlockApp",		RaceBookUnlockApp},

	{"ClearRaceDifficulty",		ClearRaceDifficulty},

	{ "GetResprays",			GetResprays },

	{ "SP_EquippedAbility",		SP_EquippedAbility },
	{ "SP_SeenGarageModPopup",	SP_SeenGarageModPopup },

	// save interface stuff
	{ "ClearGameProfile",				UI::ClearGameProfile },
	{ "SetSaveInfoInterfaceProfile",	UI::SetSaveInfoInterfaceProfile },
	{ "SetSaveInfoInterfacePhotos",		UI::SetSaveInfoInterfacePhotos },

	{ "HasLinkCode",				HasLinkCode },

	{ "StartDLC",				StartDLC },
	{ "ContinueDLC",			ContinueDLC },

	{NULL,						NULL}		// end of list
};  

LUALIB_API int luaopen_game_profile(lua_State *pState)
{
	luaL_openlib(pState,"GameProfile",game_profile_lib,0);
	return 1;
}

} // end namespace Game


// Used by test to force save/load without going through the proper system
static void ForceDebugSave(void*)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	fs.DeleteFile("debug_save.amax");

	Sys::C_OpenParams params;
	params.m_IsCritical = false;
	Sys::I_File *pFile = fs.Open("debug_save.amax", Sys::E_FileAccess_Write, params);

	if (pFile != 0)
	{
		Sys::C_MemoryStream* pStream = new Sys::C_MemoryStream( 4 * 1024 );
		Sys::BinarySerialiserAutoEndian serial(*pStream);

		Game::SaveGameImpl(serial);
		(pStream->GetBufferPtr())[0] = 0;

		pFile->Write(pStream->GetBufferPtr(), (U32)pStream->GetBufferSize());
		SAFE_DELETE_PTR(pStream);
	}

	SAFE_DELETE_PTR(pFile);	
}

static void ForceDebugLoad(void*)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	const char* kDebugSaveFile = "debug_save.amax";
	if (!fs.FileExists(kDebugSaveFile))
		return;

	Sys::I_File *pFile = fs.Open(kDebugSaveFile);

	if (pFile != 0)
	{
		Sys::BinaryDeserialiserAutoEndian serial(*pFile);
		Game::LoadGameImpl(serial);
	}

	SAFE_DELETE_PTR(pFile);
}

Debug::Function g_debugForceSave( "Cheats.DebugSave", ForceDebugSave, 0 );
Debug::Function g_debugForceLoad( "Cheats.DebugLoad", ForceDebugLoad, 0 );
