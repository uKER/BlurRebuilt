#include <stdafx.h>
#include <misc_shared/messaging/messagebase.h>
#include "Game/Game.h"
#include "GameLogic/Messaging/GameLogicMessages.h"
#include "GameMessages.h"
#include "GameLogic\ActorStudioActors\DynamicObjectMessages.h"
#include "Game/GameActors/Components/Messages/AudioMessages.h"
#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
#include "Game/Messaging/GameflowMessages.h"
#include "GameLogic/ActorStudioActors/CutSceneMessages.h"
#include "Game/AmaxCam/DirectorActor.h"
 
Debug::Logger &g_GameMsgLog( Debug::GetLogger("amax.messaging") );

namespace Game
{

void RegisterMessageDebugInfo(Debug::Logger* pLogger)
{
	Messaging::SetDefaultLog(&Game::g_Log);

	// Game::E_GameMsg
	{
		const char* pMessageNames [] = 
		{
			"E_GameMsg_ActivePhysicsBodyUpdate",
			"E_GameMsg_PhysicalObjectImpact",
			"E_GameMsg_PostCreateInit",
			"E_GameMsg_UpdateLifeTime",
			"E_GameMsg_KillObjectsForCutScene",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_GameMsg_Count, Game::E_GameMsg_Begin);
	}

	// Game::E_GameflowMessage
	{
		const char* pMessageNames [] = 
		{
			"E_GameflowMessage_StartScreenDone",
			"E_GameflowMessage_XtInitialised",
			"E_GameflowMessage_SaveGameLoaded",
			"E_GameflowMessage_StartRaceLoad",
			"E_GameflowMessage_LoadResources", 
			"E_GameflowMessage_RLCResourcesLoaded",
			"E_GameflowMessage_RLCReadyToRock",
			"E_GameflowMessage_RaceLoaded",
			"E_GameflowMessage_IntroStarted",
			"E_GameflowMessage_IntroFinished",
			"E_GameflowMessage_RaceStarted",
			"E_GameflowMessage_Pause",
			"E_GameflowMessage_UnPause",
			"E_GameflowMessage_QuitRace",
			"E_GameflowMessage_RestartRace",
			"E_GameflowMessage_RaceFinished",
			"E_GameflowMessage_Failed",
			"E_GameflowMessage_Wrecked",
			"E_GameflowMessage_Respawn",
			"E_GameflowMessage_ResultsCalculated",
			"E_GameflowMessage_OutroStarted",
			"E_GameflowMessage_OutroFinished",
			"E_GameflowMessage_StartUILoad",
			"E_GameflowMessage_UILoaded",
			"E_GameflowMessage_DumpLevel",
			"E_GameflowMessage_LevelDumped",
			"E_GameflowMessage_StreamerFull",
			"E_GameflowMessage_FillStreamer",
			"E_GameflowMessage_StreamerStarted",
			"E_GameflowMessage_DumpUI",
			"E_GameflowMessage_RestartLevel",
			"E_GameflowMessage_SyncBootAssets",
			"E_GameflowMessage_LoadFrontendAudio",
			"E_GameflowMessage_StartFrontendAudio",
			"E_GameflowMessage_StopFrontendAudio",
			"E_GameflowMessage_StartInGameAudio",
			"E_GameflowMessage_StopInGameAudio",
			"E_GameflowMessage_MusicLoadingComplete",
			"E_GameflowMessage_SorceryReloaded",
			"E_GameflowMessage_StartGameRendering",
			"E_GameflowMessage_StopGameRendering",
			"E_GameflowMessage_StopGameUpdate",
			"E_GameflowMessage_StartSpectatorMode",
			"E_GameflowMessage_IdlePlayerKicked",
			"E_GameflowMessage_DailyChallengesReset",
			"E_GameflowMessage_FriendsUpdated",
			"E_GameflowMessage_ClearStoredActorPtrs",
			"E_GameflowMessage_QuitAndWinRace",
			"E_GameflowMessage_EnteredDebugMenu",
			"E_GameflowMessage_LoadUIScene",
			"E_GameflowMessage_LoadSPProfileCar",
			"E_GameflowMessage_LoadMPProfileCar",
			"E_GameflowMessage_StartGame",
			"E_GameflowMessage_QuitLevel",
			"E_GameflowMessage_VideoPresent",
			"E_GameflowMessage_VideoStarted",
			"E_GameflowMessage_VideoFinished",		
			"E_GameflowMessage_CutsceneVideoStarted",
			"E_GameflowMessage_CutsceneVideoFinished",
			"E_GameflowMessage_VehicleFinished",
			"E_GameflowMessage_RaceTimeoutStart",
			"E_GameflowMessage_BeginRollingStart",
			"E_GameflowMessage_EndRollingStart",
			"E_GameflowMessage_E3ControllerInput",
			"E_GameflowMessage_ToggleToPlayer",
			"E_GameflowMessage_PrepareOutro",
			"E_GameflowMessage_EliminationTimeoutStart",
			"E_GameflowMessage_EliminationTimeoutEnded",
			"E_GameflowMessage_UpdateDeferredAudio",
			"E_GameflowMessage_CachePostBootFxResourceBag,"
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_GameflowMessage_Count, Game::E_GameflowMessage_Begin);
	}

	// Game::E_VehicleMessage
	{
		const char* pMessageNames [] = 
		{
			"E_VehicleMessage_Loaded",
			"E_VehicleMessage_UpdatePlayerControls",	
			"E_VehicleMessage_UpdateAIControls",
			"E_VehicleMessage_UpdateNetworkControls",
			"E_VehicleMessage_PostUpdate",
			"E_VehicleMessage_UpdatePerformanceModifiers",
			"E_VehicleMessage_UpdateDriver",
			"E_VehicleMessage_ResetDriver",
			"E_VehicleMessage_ForceAIControl",
			"E_VehicleMessage_ForceHumanControl",
			"E_VehicleMessage_ResetAtSpawn",
			"E_VehicleMessage_ResetAtWorldPosition",
			"E_VehicleMessage_ResetOnTrack",
			"E_VehicleMessage_ResetAtVehiclePosition",
			"E_VehicleMessage_ResetAtCameraPosition",
			"E_VehicleMessage_ResetEnteredCameraZone",
			"E_VehicleMessage_ResetExitedCameraZone",
			"E_VehicleMessage_ResetForceCameraZone",
			"E_VehicleMessage_ResetEntryZone",
			"E_VehicleMessage_ResetExitZone",
			"E_VehicleMessage_ResetTwoWayZone",
			"E_VehicleMessage_ResetForceZone",
			"E_VehicleMessage_ResetAfterWrecked",
			"E_VehicleMessage_CancelWreckedTimer",
			"E_VehicleMessage_EnableDisableAutoHOGPlacement",
			"E_VehicleMessage_VehicleWorldPosition",
			"E_VehicleMessage_VehicleFocusPosition",
			"E_VehicleMessage_Enable",
			"E_VehicleMessage_Disable",
			"E_VehicleMessage_DebugRender",
			"E_VehicleMessage_RaceStarted",
			"E_VehicleMessage_ImpactEvent",
			"E_VehicleMessage_ShieldedImpactEvent",
			"E_VehicleMessage_ControlsDisable",
			"E_VehicleMessage_ControlsEnable",						
			"E_VehicleMessage_ChangedHealthLevel",
			"E_VehicleMessage_FatallyDamaged",
			"E_VehicleMessage_ResetFatalDamage",
			"E_VehicleMessage_RepairDamage",
			"E_VehicleMessage_Eliminated",
			"E_VehicleMessage_Overtake",
			"E_VehicleMessage_OvertakeWithNitro",
			"E_VehicleMessage_CloseProximity",
			"E_VehicleMessage_ExhaustPopEvent",
			"E_VehicleMessage_FirePickup",
			"E_VehicleMessage_ApplyShockedEffect",
			"E_VehicleMessage_LeftRace",
			"E_VehicleMessage_NearlyFinishedRace",
			"E_VehicleMessage_TakeOut",		
			"E_VehicleMessage_AudioImpactEvent",
			"E_VehicleMessage_CheckTriggerRegion",
			"E_VehicleMessage_TriggerActorEvent",
			"E_VehicleMessage_Freeze",
			"E_VehicleMessage_LoseGrip",
			"E_VehicleMessage_DontLoseGrip",
			"E_VehicleMessage_StopWreckedEffect",
			"E_VehicleMessage_KillNitro",
			"E_VehicleMessage_AudioVehiclePartSlamming",
			"E_VehicleMessage_ApplyDebugDamage",
			"E_VehicleMessage_AudioWaterSplash",
			"E_VehicleMessage_Respray",
			"E_VehicleMessage_SetVisibility",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_VehicleMessage_Count, Game::E_VehicleMessage_Begin);
	}

	// Game::E_PickupMessage
	{
		const char* pMessageNames [] = 
		{
			"E_PickupMessage_PickupPickedUp",
			"E_PickupMessage_GrantPickup",
			"E_PickupMessage_MissileExploded",
			"E_PickupMessage_MissileHitWorld",
			"E_PickupMessage_MineExploded",
			"E_PickupMessage_PickupInitiated",
			"E_PickupMessage_PickupFinished",
			"E_PickupMessage_HitByPickup",
			"E_PickupMessage_HitDecoy",
			"E_PickupMessage_EnablePickupType",
			"E_PickupMessage_DisablePickupType",
			"E_PickupMessage_EnableAllPickupTypes",
			"E_PickupMessage_DisableAllPickupTypes",
			"E_PickupMessage_SetPickupPositionId",
			"E_PickupMessage_SetPickup",
			"E_PickupMessage_RemovePower",
			"E_PickupMessage_ClearHitUpdaters",
			"E_PickupMessage_Shuffle",
			"E_PickupMessage_EnteredPickup",
			"E_PickupMessage_ExitedPickup",
			"E_PickupMessage_UpdateFanDemandState",
			"E_PickupMessage_ActivatePickup",
			"E_PickupMessage_ResetPickup",
			"E_PickupMessage_ShockedAPickup",
			"E_PickupMessage_WakeAudio",
			"E_PickupMessage_SleepAudio",
			"E_PickupMessage_ActivatePickups",
			"E_PickupMessage_DebugForcePickupRespawn",
			"E_PickupMessage_CollectStopwatch",
			"E_PickupMessage_KillAllPickups",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_PickupMessage_Count, Game::E_PickupMessage_Begin);
	}

	// ActorMsg::E_DynamicObjMsg
	{
		const char* pMessageNames [] = 
		{
			"E_DynamicObjMsg_CameraUpdate",
			"E_DynamicObjMsg_VehicleEnteredTrigger",
			"E_DynamicObjMsg_VehicleExitedTrigger",
			"E_DynamicObjMsg_VehicleHandOfGodCameraEnteredArea",
			"E_DynamicObjMsg_VehicleHandOfGodCameraExitedArea",
			"E_DynamicObjMsg_VehicleHandOfGodCameraForceArea",
			"E_DynamicObjMsg_VehicleHandOfGodResetForce",
			"E_DynamicObjMsg_VehicleActivatedTrigger",
			"E_DynamicObjMsg_VehicleHandOfGodEntryArea",
			"E_DynamicObjMsg_VehicleHandOfGodExitArea",
			"E_DynamicObjMsg_VehicleHandOfGodTwoWayArea",
			"E_DynamicObjMsg_VehicleAchieveAmboyOilfields",
			"E_DynamicObjMsg_VehicleAchieveGasStation",
			"E_DynamicObjMsg_ImpactEvent",
			"E_DynamicObjMsg_RenderDebug",
			"E_DynamicObjMsg_SorceryUpdate",
			"E_DynamicObjMsg_AnimationUpdate",
			"E_DynamicObjMsg_VehicleSloMoEntry",
			"E_DynamicObjMsg_VehicleSloMoExit",
			"E_DynamicObjMsg_ChangeSpecialEffect",
			"E_DynamicObjMsg_SetAlphaValue",
			"E_DynamicObjMsg_SelectLightingRig",
			"E_DynamicObjMsg_StartDynamicObjectAnimation",
			"E_DynamicObjMsg_DynamicObjectApplyImpulse",
			"E_DynamicObjMsg_DynamicObjectDisableElectricalEffect",
			"E_DynamicObjMsg_DynamicObjectSetPhysicsType",
			"E_DynamicObjMsg_ChangeCameraBehaviour",
			"E_DynamicObjMsg_SimpleCameraBehaviour",
			"E_DynamicObjMsg_FreezeCameraState",
			"E_DynamicObjMsg_FadeDynamicCamera",
			"E_DynamicObjMsg_ResetDynamicCamera",
			"E_DynamicObjMsg_CameraCut",
			"E_DynamicObjMsg_StartAnimatedCamera",
			"E_DynamicObjMsg_FinishedAnimatedCamera",
			"E_DynamicObjMsg_BlendAnimatedCamera",
			"E_DynamicObjMsg_AutoSelectAnimatedCamera",
			"E_DynamicObjMsg_UICameraControl",
			"E_DynamicObjMsg_ChangeEnvironmentSettings",
			"E_DynamicObjMsg_SetAIRoute",
			"E_DynamicObjMsg_SetVehicleLogic",
			"E_DynamicObjMsg_EnableLevelRoute",
			"E_DynamicObjMsg_DisableLevelRoute",
			"E_DynamicObjMsg_SplineCtrl_StopAndWait",
			"E_DynamicObjMsg_SplineCtrl_SetSpeed",
			"E_DynamicObjMsg_SplineCtrl_ReverseDirection",
			"E_DynamicObjMsg_SplineCtrl_SetBankAngle",
			"E_DynamicObjMsg_OutputDebug",
			"E_DynamicObjMsg_EnableTVCameras",
			"E_DynamicObjMsg_DisableTVCameras",

			"E_ActorLogicMsg_AttachToPuller",
			"E_ActorLogicMsg_PullerUpdate",
			"E_DynamicObjMsg_VehicleAudioSloMoEntry",
			"E_DynamicObjMsg_VehicleAudioSloMoExit",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, ActorMsg::E_DynamicObjMsg_Count, ActorMsg::E_DynamicObjMsg_Begin);
	}

	// E_VehicleLogicMsg
	{
		const char* pMessageNames [] = 
		{
			//"E_VehicleLogicMsg_PickUpInfo",
			"E_VehicleLogicMsg_Skillshot",
			"E_VehicleLogicMsg_Wrecked",
			"E_VehicleLogicMsg_FansAwarded",
			"E_VehicleLogicMsg_FiredPerk",
			"E_VehicleLogicMsg_HitByPerk",
			"E_VehicleLogicMsg_TargettedHitByPerk",
			"E_VehicleLogicMsg_ShieldedPerk",
			"E_VehicleLogicMsg_MissedPerk",
			"E_VehicleLogicMsg_DropPickupOnTrack",
			"E_VehicleLogicMsg_InstaPickupFire",
			"E_VehicleLogicMsg_DecoyPickupDropOnTrack",
			"E_VehicleLogicMsg_HitByPerkSpecialEventRevenge",
			"E_VehicleLogicMsg_HitByPerkSpecialEventCollided",
			"E_VehicleLogicMsg_HitByPerkSpecialEventFlipped",
			"E_VehicleLogicMsg_HitByPerkSpecialEventFatallyDamaged",
			"E_VehicleLogicMsg_ResetWarningMsg",
			"E_VehicleLogicMsg_ForceDamage",
			"E_VehicleLogicMsg_SetHealth",
			"E_VehicleLogicMsg_Repair",
			"E_VehicleLogicMsg_ShieldTime",
			"E_VehicleLogicMsg_ShockTarget",
			"E_VehicleLogicMsg_Ghosted",
			"E_VehicleLogicMsg_Deghosted",
			"E_VehicleLogicMsg_PerkUpdateMatrix",
			"E_VehicleLogicMsg_DroppedPerk",
			"E_VehicleLogicMsg_PerkAudioTrigger",
			"E_VehicleLogicMsg_AddOnTrackScore",
			"E_VehicleLogicMsg_AggressiveScore",
			"E_VehicleLogicMsg_Domination",
			"E_VehicleLogicMsg_DominationScore",
			"E_VehicleLogicMsg_PerkEnded",
			"E_VehicleLogicMsg_Recovered",
			"E_VehicleLogicMsg_StartDelayToPerk",
			"E_VehicleLogicMsg_ToggleAiControl",
			"E_VehicleLogicMsg_TogglePlayerControl",
			"E_VehicleLogicMsg_PerkUpdate",
			"E_VehicleLogicMsg_EnableRumble",
			"E_VehicleLogicMsg_DisableRumble",
			"E_VehicleLogicMsg_EnableDisablePickupSystem",
			"E_VehicleLogicMsg_HOGReset",
			"E_VehicleLogicMsg_Score",
			"E_VehicleLogicMsg_Fan_Demand_Passed",
			"E_VehicleLogicMsg_Challenge_Completed",
			"E_VehicleLogicMsg_SurvivedElimination",
			"E_VehicleLogicMsg_ChainCompleted",

			"E_VehicleLogicMsg_VehicleMurdered",
			"E_VehicleLogicMsg_VehicleSuicided",
			"E_VehicleLogicMsg_PickupDestroyedByPickup",
			"E_VehicleLogicMsg_Landed",

			"E_VehicleLogicMsg_HitWithAllDomes",
			"E_VehicleLogicMsg_AvoidedAllDomes",
			"E_VehicleLogicMsg_CrossedCheckpoint",
			
			"E_VehicleLogicMsg_EvadedPowerUp",
			"E_VehicleLogicMsg_PowerUpDeleted",
			"E_VehicleLogicMsg_SideSwipe",

			"E_VehicleLogicMsg_PowerUpDestroyedByPowerUp",

			"E_VehicleLogicMsg_ModActivated",
			"E_VehicleLogicMsg_PowerUpFXDurations",

		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, E_VehicleLogicMsg_Count, E_VehicleLogicMsg_Begin);
	}

	// ActorMsg::E_CutSceneMsg
	{
		const char* pMessageNames [] = 
		{
			"E_CutSceneMsg_Start_Cutscene",
			"E_CutSceneMsg_Start_ScriptedCamera",
			"E_CutSceneMsg_Start_RestartScriptedCamera",
			"E_CutSceneMsg_Start_PassedCutscene",
			"E_CutSceneMsg_Start_PassedScriptedCamera",
			"E_CutSceneMsg_Start_FailedScriptedCamera",
			"E_CutSceneMsg_Finished",
			"E_CutSceneMsg_StartFatalDamageCam",
			"E_CutSceneMsg_Start_GarageCamera",
			"E_CutSceneMsg_Start_GarageCameraReverse",

			"E_CutSceneMsg_StartTheIntro",
			"E_CutSceneMsg_InitialiseProxy",
			"E_CutSceneMsg_EnableDisableProxy",
			"E_CutSceneMsg_PlaceCarOnPoint",
			"E_CutSceneMsg_SetAIOnSpline",
			"E_CutSceneMsg_SetAISpeed",
			"E_CutSceneMsg_SetAIRelativeSpeed",
			"E_CutSceneMsg_ApplyImpulse",
			"E_CutSceneMsg_RepairAllDamage",
			"E_CutSceneMsg_StartTheRace",
			"E_CutSceneMsg_RaceEnded",
			"E_CutSceneMsg_SetDriverAnim",
			"E_CutSceneMsg_SetCarBehaviour",
			"E_CutSceneMsg_PlayConversationAndSubtitle",
			"E_CutSceneMsg_FullScreenEffect",

			"E_CutSceneMsg_Replay_SetSaveFilename",
			"E_CutSceneMsg_Replay_StartRecordingCs",
			"E_CutSceneMsg_Replay_Load",
			"E_CutSceneMsg_Replay_Play",
			"E_CutSceneMsg_Replay_Stop",
			"E_CutSceneMsg_Replay_Pause",
			"E_CutSceneMsg_Replay_Resume",
			"E_CutSceneMsg_Damage_EnableDisable",
			"E_CutSceneMsg_Damage_Apply",
			"E_CutSceneMsg_Damage_Repair",
			"E_CutSceneMsg_DriveRelative",
			"E_CutSceneMsg_DisableAI",
			"E_CutSceneMsg_SetVehicleControl",
			"E_CutSceneMsg_SetVehicleHandbrake",
//			"E_CutSceneMsg_SetVehicleSpeed",
			"E_CutSceneMsg_SetVehicleRevs",
			"E_CutSceneMsg_SetVehicleBrake",
			"E_CutSceneMsg_SetVehicleSteering",
			"E_CutSceneMsg_SetDoorPosition",
			"E_CutSceneMsg_SetWindowPosition",
			"E_CutSceneMsg_SetBonnetPosition",
			"E_CutSceneMsg_SetLightState",
			"E_CutSceneMsg_DisplayVehicle",
			"E_CutSceneMsg_DisplayDriver",
			"E_CutSceneMsg_CreateVehicleEffects",
			"E_CutSceneMsg_LoadResources",
			"E_CutSceneMsg_DumpResources",
			"E_CutSceneMsg_SwapVehicleModel",
			"E_CutSceneMsg_Start321",
			"E_CutSceneMsg_Start321_2_Player",
			"E_CutSceneMsg_Start321_3_Player",
			"E_CutSceneMsg_Start321_4_Player",
			"E_CutSceneMsg_WreckCam",
			"E_CutSceneMsg_EnableRaceAI",
			"E_CutSceneMsg_DisableRaceAI",
			"E_CutSceneMsg_SetVehiclesToRaceSpeed",
			"E_CutSceneMsg_Dialogue",
			"E_CutSceneMsg_DialogueFinished",
			"E_CutSceneMsg_Skipped",
			"E_CutSceneMsg_DisplayCredit",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, ActorMsg::E_CutSceneMsg_Count, ActorMsg::E_CutSceneMsg_Begin);
	}

	// AmaxCam::E_DirectorMsg
	{
		const char* pMessageNames [] = 
		{
			"E_DirectorMsg_ShowGameCamera",
			"E_DirectorMsg_ShowLevelCamera",
			"E_DirectorMsg_ShowEffect",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, AmaxCam::E_DirectorMsg_Count, AmaxCam::E_DirectorMsg_Begin);
	}

	// ActorMsg::E_AudioMsg
	{
		const char* pMessageNames [] = 
		{
			"E_AudioMsg_Event",
			"E_AudioMsg_SetParameter",
			"E_AudioMsg_SetVector",
			"E_AudioMsg_TriggerActionFromHandle",
			"E_AudioMsg_TriggerActionFromStrHash",
			"E_AudioMsg_SetParamFromStrHash",
			"E_AudioMsg_ConnectEntity",
			"E_AudioMsg_DialogueFinish",
			"E_AudioMsg_DialogueLoadEntity",
			"E_AudioMsg_GraphicalEQ",
			"E_AudioMsg_EntityAudioStopped",	
			"E_AudioMsg_InstanceAudioStopped",
			"E_AudioMsg_WaitForEvent",
			"E_AudioMsg_HotSwitchEntity",
			"E_AudioMsg_PlayUIDialogue",
			"E_AudioMsg_StopUIDialogue",
			"E_AudioMsg_UIDialogueFinish",
			"E_AudioMsg_ReleaseEntity",

		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, ActorMsg::E_AudioMsg_Count, ActorMsg::E_AudioMsg_Begin);
	}

	// Game::E_AIMessage
	{
		const char* pMessageNames [] = 
		{
			"E_AIMessage_Simulate",
			"E_AIMessage_StartSimulate",
			"E_AIMessage_StopSimulate",
			"E_AIMessage_Start",
			"E_AIMessage_Stop",
			"E_AIMessage_StartSetSpeed",
			"E_AIMessage_EndSetSpeed",
			"E_AIMessage_AllLoaded",
			"E_AIMessage_LoadFailed",
			"E_AIMessage_TrackLoaded",
			"E_AIMessage_DriverLoaded",
			"E_AIMessage_LoadAITrackResources",
			"E_AIMessage_LoadAIDriverResources",
			"E_AIMessage_ReLoadAIDriverResources",
			"E_AIMessage_SetRoute",
			"E_AIMessage_InitialiseIntro",
			"E_AIMessage_CreateIntroSpline",
			"E_AIMessage_SetIntroOutroSpeed",
			"E_AIMessage_SetIntroOutroSpline",
			"E_AIMessage_DisableAI",
			"E_AIMessage_DriveTowardsPoint",
			"E_AIMessage_AtEndIntroOutroSpline",
			"E_AIMessage_AttemptedSideSwipe",
			"E_AIMessage_CreatePerkLink"
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_AIMessage_Count, Game::E_AIMessage_Begin);
	}

	// Game::E_GameLogicMessage
	{
		const char* pMessageNames [] = 
		{
			"E_GameLogicMessage_Scoring_Event",
			"E_GameLogicMessage_Failed_Rule",
			"E_GameLogicMessage_Passed_All_Rule",
			"E_GameLogicMessage_LapIncremented",
			"E_GameLogicMessage_LapDecremented",
			"E_GameLogicMessage_SplitCrossed",
			"E_GameLogicMessage_Impact",
			"E_GameLogicMessage_GameplayImpact",
			"E_GameLogicMessage_MastermineSpawnedMinis",
			"E_GameLogicMessage_GiftDelayedPickupWithAnim",
			
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, Game::E_GameLogicMessage_Count, Game::E_GameLogicMessage_Begin);
	}
}

} // namespace GameMessaging
