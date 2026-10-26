#include "stdafx.h"

#include <ui/UIDefines.h>
#include <ui/UILogger.h>
#include <ui/Game/UILua.h>
#include <ui/game/UIXt.h>
#include <GameLogic/Lua/LuaVariableHelper.h>
#include <xt/xtgame.h>

#include <Game\Messaging\GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/GameData/GameData.h>

#include <UI/UiMessages.h>
#include <Network/Router/NetRouterDefines.h>
#include <GameLogic\ContentServer\ContentServerManager.h>

XT_TYPE( Game::E_GameflowMessage, "GameFlowMessage" )
	XT_OPTION( Game::E_GameflowMessage_StartScreenDone, "StartScreenDone" )
	XT_OPTION( Game::E_GameflowMessage_SaveGameLoaded, "SaveGameLoaded" )
	XT_OPTION( Game::E_GameflowMessage_StartRaceLoad, "StartRaceLoad" )
	XT_OPTION( Game::E_GameflowMessage_RaceLoaded, "RaceLoaded" )
	XT_OPTION( Game::E_GameflowMessage_IntroStarted, "IntroStarted" )
	XT_OPTION( Game::E_GameflowMessage_IntroFinished, "IntroFinished" )
	XT_OPTION( Game::E_GameflowMessage_RaceStarted, "RaceStarted" )
	XT_OPTION( Game::E_GameflowMessage_Pause, "Pause" )
	XT_OPTION( Game::E_GameflowMessage_UnPause, "UnPause" )
	XT_OPTION( Game::E_GameflowMessage_QuitRace, "QuitRace" )
	XT_OPTION( Game::E_GameflowMessage_RestartRace, "RestartRace" )
	XT_OPTION( Game::E_GameflowMessage_RaceFinished, "RaceFinished" )
	XT_OPTION( Game::E_GameflowMessage_Failed, "RaceFailed" )
	XT_OPTION( Game::E_GameflowMessage_Wrecked, "Wrecked" )
	XT_OPTION( Game::E_GameflowMessage_Respawn, "Respawn" )
	XT_OPTION( Game::E_GameflowMessage_OutroStarted, "OutroStarted" )
	XT_OPTION( Game::E_GameflowMessage_OutroFinished, "OutroFinished" )
	XT_OPTION( Game::E_GameflowMessage_StartUILoad, "StartUILoad" )
	XT_OPTION( Game::E_GameflowMessage_UILoaded, "UILoaded" )
	XT_OPTION( Game::E_GameflowMessage_QuitAndWinRace, "QuitAndWinRace" )
	XT_OPTION( Game::E_GameflowMessage_EnteredDebugMenu, "EnteredDebugMenu" )
	XT_OPTION( Game::E_GameflowMessage_StartGame, "StartGame" )
	XT_OPTION( Game::E_GameflowMessage_QuitLevel, "QuitLevel" )
	XT_OPTION( Game::E_GameflowMessage_DumpLevel, "DumpLevel" )	
	XT_OPTION( Game::E_GameflowMessage_LevelDumped, "LevelDumped" )	
	XT_OPTION( Game::E_GameflowMessage_StreamerFull, "StreamerFull" )	
	XT_OPTION( Game::E_GameflowMessage_DumpUI, "DumpUI" )	
	XT_OPTION( Game::E_GameflowMessage_RestartLevel, "RestartLevel" )
	XT_OPTION( Game::E_GameflowMessage_SyncBootAssets, "SyncBootAssets" )

	XT_OPTION( Game::E_GameflowMessage_StartGameRendering,	"StartGameRendering" )
	XT_OPTION( Game::E_GameflowMessage_StopGameRendering,	"StopGameRendering" )
	XT_OPTION( Game::E_GameflowMessage_StopGameUpdate,	"StopGameUpdate" )	

	XT_OPTION( Game::E_GameflowMessage_LoadFrontendAudio,	"LoadFrontendAudio" )	
	XT_OPTION( Game::E_GameflowMessage_StartFrontendAudio,	"StartFrontendAudio" )	
	XT_OPTION( Game::E_GameflowMessage_StopFrontendAudio,	"StopFrontendAudio" )	
	XT_OPTION( Game::E_GameflowMessage_StartInGameAudio,	"StartIngameAudio" )	
	XT_OPTION( Game::E_GameflowMessage_StopInGameAudio,		"StopIngameAudio" )	

	// Probally do this another way some day...
	XT_OPTION( Game::E_GameflowMessage_VideoPresent, "VideoPresent" )
	XT_OPTION( Game::E_GameflowMessage_VideoStarted, "VideoStarted" )
	XT_OPTION( Game::E_GameflowMessage_VideoFinished, "VideoFinished" )
	XT_OPTION( Game::E_GameflowMessage_CutsceneVideoStarted, "CutsceneVideoStarted" )
	XT_OPTION( Game::E_GameflowMessage_CutsceneVideoFinished, "CutsceneVideoFinished" )

	XT_OPTION( Game::E_GameflowMessage_VehicleFinished, "VehicleFinished" )
	XT_OPTION( Game::E_GameflowMessage_RaceTimeoutStart, "RaceTimeoutStart" )
	XT_OPTION( Game::E_GameflowMessage_BeginRollingStart, "BeginRollingStart" )
	XT_OPTION( Game::E_GameflowMessage_EndRollingStart, "EndRollingStart" )
	XT_OPTION( Game::E_GameflowMessage_E3ControllerInput, "E3ControllerInput" )
	XT_OPTION( Game::E_GameflowMessage_ToggleToPlayer, "ToggleToPlayer" )
	XT_OPTION( Game::E_GameflowMessage_StartSpectatorMode, "StartSpectatorMode" )

	XT_OPTION( Game::E_GameflowMessage_PrepareOutro, "PrepareOutro" )
	XT_OPTION( Game::E_GameflowMessage_LoadUIScene, "LoadUIScene" )
	XT_OPTION( Game::E_GameflowMessage_EliminationTimeoutStart, "EliminationTimeoutStart" )
	XT_OPTION( Game::E_GameflowMessage_EliminationTimeoutEnded, "EliminationTimeoutEnd" )
	XT_OPTION( Game::E_GameflowMessage_LoadSPProfileCar, "LoadSPProfileCar" )
	XT_OPTION( Game::E_GameflowMessage_LoadMPProfileCar, "LoadMPProfileCar" )
		
XT_END_TYPE

XT_TYPE( Game::E_AIMessage, "AIMessage" )
	XT_OPTION( Game::E_AIMessage_InitialiseIntro, "InitialiseIntro" )
XT_END_TYPE

XT_TYPE( Game::E_GameMode, "GameMode" )
	XT_OPTION( Game::E_GameMode_Nothing,		"Nothing" )
	XT_OPTION( Game::E_GameMode_Debug,			"Debug" )
	XT_OPTION( Game::E_GameMode_SinglePlayer,	"SinglePlayer" )
	XT_OPTION( Game::E_GameMode_MultiPlayer,	"MultiPlayer" )
	XT_OPTION( Game::E_GameMode_Network,		"Network" )
	XT_OPTION( Game::E_GameMode_Online,			"Online" )
	XT_OPTION( Game::E_GameMode_SplitScreen,	"SplitScreen" )
	XT_OPTION( Game::E_GameMode_SystemLink,		"SystemLink" )
XT_END_TYPE

XT_TYPE( Game::E_RaceMode, "RaceMode" )
	XT_OPTION( Game::E_RaceMode_Nothing,					"Nothing" )
	XT_OPTION( Game::E_RaceMode_StreetRace,					"StreetRace" )
	XT_OPTION( Game::E_RaceMode_Multiplayer_Racing,			"MpRacing" )
	XT_OPTION( Game::E_RaceMode_Multiplayer_Elimination,	"MpElimination" )
	XT_OPTION( Game::E_RaceMode_Multiplayer_Points,			"MpPoints" )
	XT_OPTION( Game::E_RaceMode_Multiplayer_Destruction,	"MpDestruction" )
	XT_OPTION( Game::E_RaceMode_Destruction,				"SpDestruction" )
	XT_OPTION( Game::E_RaceMode_Checkpoint,					"SpCheckpoint" )
	XT_OPTION( Game::E_RaceMode_FanDemand,					"SpFanDemand" )
	XT_OPTION( Game::E_RaceMode_BossBattle,					"SpBossBattle" )
XT_END_TYPE

XT_TYPE( Game::E_PlayMode, "PlayMode" )
	XT_OPTION( Game::E_PlayMode_Nothing,		"Nothing" )
	XT_OPTION( Game::E_PlayMode_CustomRace,		"CustomRace" )
	XT_OPTION( Game::E_PlayMode_Playlist,		"Playlist" )
XT_END_TYPE

XT_TYPE( Game::CONTENT_SERVER_TYPE, "ContentServerType" )
	XT_OPTION( Game::CONTENT_SERVER_TYPE_PHOTO,		"Photo" )
	XT_OPTION( Game::CONTENT_SERVER_TYPE_TWITTER,	"Twitter" )
	XT_OPTION( Game::CONTENT_SERVER_TYPE_GROUP,		"Group" )
	XT_OPTION( Game::CONTENT_SERVER_TYPE_EVENT,		"Event" )
XT_END_TYPE

XT_LIBRARY( GAMELOGIC )
	XT_REGISTER_TYPE( Game::E_GameflowMessage )
	XT_REGISTER_TYPE( Game::E_GameMode )
	XT_REGISTER_TYPE( Game::E_RaceMode )
	XT_REGISTER_TYPE( Game::E_PlayMode )
	XT_REGISTER_TYPE( Game::E_AIMessage )
	XT_REGISTER_TYPE( Game::CONTENT_SERVER_TYPE )
XT_END_LIBRARY

namespace Game
{

LuaVariableHelper LuaVariableHelper::s_instance;

void LuaVariableHelper::CreateLuaTables()
{
	lua_State* pState = UI::g_pUILua->GetState();

	lua_getglobal( UI::g_pUILua->GetState(), LUA_ENUM_TABLE_NAME );
	if ( lua_type( pState, -1 ) != LUA_TTABLE )
	{
		LOGFATAL( UI::g_Log, DBSTR "%s - table not found", LUA_ENUM_TABLE_NAME );
		lua_pop( pState, 1 );
		return;
	}

	UI::UiXt::CreateLuaTableFromEnumType( pState, "GameFlowMessage" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "GameMode" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "RaceMode" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "PlayMode" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "UIGameMessage" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "AIMessage" );

	UI::UiXt::CreateLuaTableFromEnumType( pState, "ContentServerType" );
	UI::UiXt::CreateLuaTableFromEnumType( pState, "SocialNetworkingItemType" );

	lua_pop( pState, 1 );
}

} // namespace Game
