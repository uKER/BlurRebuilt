#ifndef GAMEFLOW_MESSAGES_H
#define GAMEFLOW_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

namespace Game
{

enum E_GameflowMessage
{
	E_GameflowMessage_HiBits = FOURCC_HI20('AGFM'),	

	//User has pressed Start on the Start Screen
	// At this point we know the pad, system name etc
	E_GameflowMessage_StartScreenDone,

	// Sent when LoadXt() has been called and all packs are in memory.
	E_GameflowMessage_XtInitialised,

	//User has chosen the Save Game he would like to use
	// At this point we now everything about the user
	E_GameflowMessage_SaveGameLoaded,

	//Once the AmaxWorldLayer is created,
	E_GameflowMessage_LoadResources, 

	//All RLC vehicles are loaded, RLC will be post initialising...
	E_GameflowMessage_RLCResourcesLoaded,

	//RLC ready to rock
	E_GameflowMessage_RLCReadyToRock,

	//All data has loaded and we are about to enter the first ingame screen
	E_GameflowMessage_RaceLoaded,

	//Intro has just begun
	E_GameflowMessage_IntroStarted,

	//Intro has finished and the 3,2,1 Go is about to commence
	E_GameflowMessage_IntroFinished,

	//All pre race stuff has been done and the user is in control of the vehicle
	E_GameflowMessage_RaceStarted,

	//user has paused the game
	E_GameflowMessage_Pause,

	//user has unpaused the game
	E_GameflowMessage_UnPause,

	//User has chosen the Quit option
	E_GameflowMessage_QuitRace,

	//User has chosen restart
	E_GameflowMessage_RestartRace,

	//The race has come to it's logical end, post race is about to start
	E_GameflowMessage_RaceFinished,

	//Failed race
	E_GameflowMessage_Failed,
	E_GameflowMessage_Wrecked,
	E_GameflowMessage_Respawn,

	// At this point we know that the LevelResults class is valid
	E_GameflowMessage_ResultsCalculated,

	//Outro has just begun
	E_GameflowMessage_OutroStarted,

	//Outro has finished
	E_GameflowMessage_OutroFinished,

	//User has decided what to do in the game
	//At this point we know that the LevelSetup class is valid time to create the AmaxWorldLayer
	E_GameflowMessage_StartRaceLoad,

	//All post race has been done, we are now going back to the UI
	E_GameflowMessage_StartUILoad,

	//The UI is loaded and the user is back in control
	E_GameflowMessage_UILoaded,

	//Dump the level without the ui loading back to the front end.
	E_GameflowMessage_DumpLevel,
	E_GameflowMessage_LevelDumped,
	E_GameflowMessage_StreamerFull,
	E_GameflowMessage_FillStreamer,
	E_GameflowMessage_StreamerStarted,
	E_GameflowMessage_DumpUI,
	E_GameflowMessage_RestartLevel,
	E_GameflowMessage_SyncBootAssets,

	// Audio trigger messags
  	E_GameflowMessage_LoadFrontendAudio,
	E_GameflowMessage_StartFrontendAudio,
	E_GameflowMessage_StopFrontendAudio,
	E_GameflowMessage_StartInGameAudio,
	E_GameflowMessage_StopInGameAudio,
	E_GameflowMessage_MusicLoadingComplete,
	/****************************************************************************\
	|						Misc Flow Messages									 |
	\****************************************************************************/

	//Sorcery has changed on the PC and has been reloaded
	E_GameflowMessage_SorceryReloaded,

	// Sent when the game wants to start and stop rendering.
	E_GameflowMessage_StartGameRendering,
	E_GameflowMessage_StopGameRendering,
	
	E_GameflowMessage_StopGameUpdate,

	E_GameflowMessage_StartSpectatorMode,

	E_GameflowMessage_ClearStoredActorPtrs,

	E_GameflowMessage_IdlePlayerKicked,

	E_GameflowMessage_DailyChallengesReset,

	E_GameflowMessage_FriendsUpdated,

	/****************************************************************************\
	|						Debug Flow Messages									 |
	\****************************************************************************/

	//User has chosen the Quit and Win option
	E_GameflowMessage_QuitAndWinRace,
	//User has gone into the debug menu from the start screen
	E_GameflowMessage_EnteredDebugMenu,

	E_GameflowMessage_LoadUIScene,

	E_GameflowMessage_LoadSPProfileCar,
	E_GameflowMessage_LoadMPProfileCar,

	//Old Mercury Game stuff
	E_GameflowMessage_StartGame,
	E_GameflowMessage_QuitLevel,


	/****************************************************************************\
	|						Theres a better way of doing this									 |
	\****************************************************************************/
	//So audio know to wait for video finihed messsages
	E_GameflowMessage_VideoPresent,

	// FMV has started
	E_GameflowMessage_VideoStarted,

	//FMV has finished
	E_GameflowMessage_VideoFinished,

	// Frontend video has started
	E_GameflowMessage_CutsceneVideoStarted,

	//Frontend video has finished
	E_GameflowMessage_CutsceneVideoFinished,

	/****************************************************************************\
	|						Multiplayer Flow Messages									 |
	\****************************************************************************/

	//	So the game displays the "Waiting for other racers to finish" screen when local vehicle finishes event
	E_GameflowMessage_VehicleFinished,

	//	Starts the timeout countdown when the first player passes a RACE event
	E_GameflowMessage_RaceTimeoutStart,

	//	Sent at the start of the MP rolling start
	E_GameflowMessage_BeginRollingStart,

	//  Sent during rolling start once the vehicle has crossed the start line
	E_GameflowMessage_EndRollingStart,

	//  Sent to inform the ui that the ai or player has taken over
	E_GameflowMessage_E3ControllerInput,

	//  Infrom VLC_E3Component that the player would like control back
	E_GameflowMessage_ToggleToPlayer,

	E_GameflowMessage_PrepareOutro,

	//	Starts the elimination countdown when an elimination is iminent
	E_GameflowMessage_EliminationTimeoutStart,

	//	Stops the elimination countdown 
	E_GameflowMessage_EliminationTimeoutEnded,

	E_GameflowMessage_UpdateDeferredAudio,

	E_GameflowMessage_CachePostBootFxResourceBag,

	E_GameflowMessage_End, 
	E_GameflowMessage_Begin = E_GameflowMessage_HiBits + 1, 
	E_GameflowMessage_Count = E_GameflowMessage_End - E_GameflowMessage_Begin
};


} // namespace Game

#endif // GAMEFLOW_MESSAGES_H
