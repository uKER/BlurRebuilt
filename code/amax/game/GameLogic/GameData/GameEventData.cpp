#include "stdafx.h"

#include <GameApp.h>

#include <Game/Game.h>

#include <GameLogic/GameData/GameEventData.h>
#include <GameLogic/RaceRules/RaceRule_FinishInXPosition.h>
#include <GameLogic/RaceRules/RaceRule_BeatDriver.h>
#include <GameLogic/RaceRules/RaceRule_GetLapTimeOfX.h>
#include <GameLogic/RaceRules/RaceRule_HitDriverXTimesWithPerk.h>
#include <GameLogic/RaceRules/RaceRule_HitXMph.h>
#include <GameLogic/RaceRules/RaceRule_DriftingInTriggerRegion.h>
#include <GameLogic/RaceRules/RaceRule_DriverHasHealth.h>
#include <GameLogic/RaceRules/RaceRule_CauseXDamageToDriver.h>
#include <GameLogic/RaceRules/RaceRule_CollectXFanPickups.h>
#include <GameLogic/RaceRules/RaceRule_EarnXP.h>
#include <GameLogic/RaceRules/RaceRule_GetFastestLap.h>
#include <GameLogic/RaceRules/RaceRule_DistanceFromDriver.h>
#include <GameLogic/RaceRules/RaceRule_HitXObjects.h>
#include <GameLogic/RaceRules/RaceRule_PerformActionYTimes.h>
#include <GameLogic/RaceRules/RaceRule_DriverLastInRace.h>
#include <GameLogic/RaceRules/RaceRule_TeamPosition.h>
#include <GameLogic/RaceRules/RaceRule_FireXPerks.h>
#include <GameLogic/RaceRules/RaceRule_TimerExpired.h>
#include <GameLogic/RaceRules/RaceRule_PerformXTrickShots.h>

#include <GameLogic/RaceRules/RaceAction_EliminateLastDriver.h>
#include <GameLogic/RaceRules/RaceAction_EliminateHitByPerks.h>
#include <GameLogic/RaceRules/RaceAction_TimeAttackDisplay.h>
#include <GameLogic/RaceRules/RaceAction_ManageGhosts.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>
#include <RaceBook/Data/RaceBookData.h>


C_GameEventData C_GameEventData::s_instance;

#define AMAX_GAMEEVENTDATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_GameEventData::Instance().Member; } };

AMAX_GAMEEVENTDATA_CALLBACK(AmaxWinCondition, mWinConditions)

XT_TYPE(AmaxCutScene,	"AmaxCutScene")
	XT_FIELD(m_UseSetupList,			"UseSetupList")
	XT_FIELD(m_IsDummy,					"IsDummy")
	XT_FIELD(m_Setup_ActorList,			"SetupActorList")
	XT_FIELD(m_Scene_ActorList,			"SceneActorList")
	XT_FIELD(m_Audio_ActorList,			"AudioActorList")
	XT_FIELD(m_Origin,					"Origin")
	XT_FIELD(m_Radius,					"Radius")
	XT_FIELD(m_VehiclesToUnload,		"VehiclesToUnload")
	XT_FIELD(m_GraphicsMemorySize,		"GraphicsMemorySize")
	XT_FIELD(m_SystemMemorySize,		"SystemMemorySize")
	XT_FIELD(m_SystemMemoryBudget,		"SystemMemoryBudget")
	XT_FIELD(m_GraphicsMemoryBudget,	"GraphicsMemoryBudget")
XT_END_TYPE

XT_TYPE(AmaxWinCondition,"AmaxWinCondition")
	XT_CALLBACK((GameDataHandler<AmaxWinCondition,CallbackAmaxWinCondition>))
	XT_BASE(AmaxGameDataObj)
XT_END_TYPE

XT_TYPE(AmaxStoryIntroOutro,"AmaxStoryIntroOutro")
	XT_FIELD(story,"Story")
XT_END_TYPE

XT_TYPE(AmaxIntroSection,"AmaxStoryIntroSection")
	XT_FIELD(dialogue,"Dialogue")
	XT_FIELD(spoken_line,"SpokenLine")
	XT_FIELD(section_time,"SectionTime")
	XT_FIELD(colour,"Colour")
	XT_FIELD(car_index,"CarIndex")
XT_END_TYPE

XT_TYPE(AmaxOpponentSetup,"OpponentSetup")
	XT_FIELD(opponent, "Opponent")
	XT_FIELD(vehicleOverride, "VehicleOveride")
	XT_FIELD(detailOverride, "DetailOverride")	
	XT_FIELD(alwaysShields, "AlwaysShields")	
	XT_FIELD(onlyAttacksPlayer, "OnlyAttacksPlayer")	
	XT_FIELD(perkSlotsOverride, "PerkSlotsOverride")
	XT_FIELD(perkGiftTypes, "PerkGiftTypes")
	XT_FIELD(perkGiftTimer, "PerkGiftTimer")
	XT_FIELD(perkGiftTimerInitial, "PerkGiftTimerInitial")
	XT_FIELD(perkGiftLaps, "PerkGiftLaps")
	XT_FIELD(healthOverride, "HealthOverride")
XT_END_TYPE

XT_TYPE(AmaxPerkAI, "PerkAI")
	XT_FIELD(maxPerkLevel, "MaxPerkLevel")
	XT_FIELD(grantTimeMin, "GrantTimeMin")
	XT_FIELD(grantTimeMax, "GrantTimeMax")
	XT_FIELD(opponentIdx, "OpponentIndex")
	XT_FIELD(perkToGrant, "PerkToGrant")
	XT_FIELD(charactersNeverTargetted, "CharactersNeverTargetted")
	XT_FIELD(preferredPickups, "PreferredPickups")
XT_END_TYPE


XT_TYPE(RacerNet::eSPScoreboardTypes, "SPLeaderboardType")
	XT_OPTION(RacerNet::eSPRacesScoreboard,"SPRacesScoreboard")
	XT_OPTION(RacerNet::eSPFanDemandsScoreboard, "SPFanDemandsScoreboard")
	XT_OPTION(RacerNet::eSPCheckpointsScoreboard, "SPCheckpointsScoreboard")
	XT_OPTION(RacerNet::eSPDestructionScoreboard, "SPDestructionScoreboard")
	XT_OPTION(RacerNet::eSP1on1Scoreboard, "SP1on1Scoreboard")
XT_END_TYPE

XT_TYPE(RacerNet::eScoreboards, "eLeaderboards")
	XT_OPTION(RacerNet::eMpScoreboardTotalWins,	"MP_TotalWins")		
	XT_OPTION(RacerNet::eMpScoreboardPerkRatio,	"MP_PerkRatio")	
	XT_OPTION(RacerNet::eMpScoreboardTotalFans,	"MP_TotalFans")	
	XT_OPTION(RacerNet::eMpScoreboardLegends,	"MP_Legends")	
	XT_OPTION(RacerNet::eMpScoreboardLobby,		"MP_Lobby")
	XT_OPTION(RacerNet::eSP_Level_1_A,  "SP_Level_1")
	XT_OPTION(RacerNet::eSP_Level_2_A,  "SP_Level_2")
	XT_OPTION(RacerNet::eSP_Level_3_A,  "SP_Level_3")
	XT_OPTION(RacerNet::eSP_Level_4_A,  "SP_Level_4")
	XT_OPTION(RacerNet::eSP_Level_5_A,  "SP_Level_5")
	XT_OPTION(RacerNet::eSP_Level_6_A,  "SP_Level_6")
	XT_OPTION(RacerNet::eSP_Level_7_A,  "SP_Level_7")
	XT_OPTION(RacerNet::eSP_Level_8_A,  "SP_Level_8")
	XT_OPTION(RacerNet::eSP_Level_9_A,  "SP_Level_9")
	XT_OPTION(RacerNet::eSP_Level_10_A, "SP_Level_10")
	XT_OPTION(RacerNet::eSP_Level_11_A, "SP_Level_11")
	XT_OPTION(RacerNet::eSP_Level_12_A, "SP_Level_12")
	XT_OPTION(RacerNet::eSP_Level_13_A, "SP_Level_13")
	XT_OPTION(RacerNet::eSP_Level_14_A, "SP_Level_14")
	XT_OPTION(RacerNet::eSP_Level_15_A, "SP_Level_15")
	XT_OPTION(RacerNet::eSP_Level_16_A, "SP_Level_16")
	XT_OPTION(RacerNet::eSP_Level_17_A, "SP_Level_17")
	XT_OPTION(RacerNet::eSP_Level_18_A, "SP_Level_18")
	XT_OPTION(RacerNet::eSP_Level_19_A, "SP_Level_19")
	XT_OPTION(RacerNet::eSP_Level_20_A, "SP_Level_20")
	XT_OPTION(RacerNet::eSP_Level_21_A, "SP_Level_21")
	XT_OPTION(RacerNet::eSP_Level_22_A, "SP_Level_22")
	XT_OPTION(RacerNet::eSP_Level_23_A, "SP_Level_23")
	XT_OPTION(RacerNet::eSP_Level_24_A, "SP_Level_24")
	XT_OPTION(RacerNet::eSP_Level_25_A, "SP_Level_25")
	XT_OPTION(RacerNet::eSP_Level_26_A, "SP_Level_26")
	XT_OPTION(RacerNet::eSP_Level_27_A, "SP_Level_27")
	XT_OPTION(RacerNet::eSP_Level_28_A, "SP_Level_28")
	XT_OPTION(RacerNet::eSP_Level_29_A, "SP_Level_29")
	XT_OPTION(RacerNet::eSP_Level_30_A, "SP_Level_30")
	XT_OPTION(RacerNet::eSP_Level_31_A, "SP_Level_31")
	XT_OPTION(RacerNet::eSP_Level_32_A, "SP_Level_32")
	XT_OPTION(RacerNet::eSP_Level_33_A, "SP_Level_33")
	XT_OPTION(RacerNet::eSP_Level_34_A, "SP_Level_34")
	XT_OPTION(RacerNet::eSP_Level_35_A, "SP_Level_35")
	XT_OPTION(RacerNet::eSP_Level_36_A, "SP_Level_36")
	XT_OPTION(RacerNet::eSP_Level_37_A, "SP_Level_37")
	XT_OPTION(RacerNet::eSP_Level_38_A, "SP_Level_38")
	XT_OPTION(RacerNet::eSP_Level_39_A, "SP_Level_39")
	XT_OPTION(RacerNet::eSP_Level_40_A, "SP_Level_40")
	XT_OPTION(RacerNet::eSP_Level_41_A, "SP_Level_41")
	XT_OPTION(RacerNet::eSP_Level_42_A, "SP_Level_42")
	XT_OPTION(RacerNet::eSP_Level_43_A, "SP_Level_43")
	XT_OPTION(RacerNet::eSP_Level_44_A, "SP_Level_44")
	XT_OPTION(RacerNet::eSP_Level_45_A, "SP_Level_45")
	XT_OPTION(RacerNet::eSP_Level_46_A, "SP_Level_46")
	XT_OPTION(RacerNet::eSP_Level_47_A, "SP_Level_47")
	XT_OPTION(RacerNet::eSP_Level_48_A, "SP_Level_48")
	XT_OPTION(RacerNet::eSP_Level_49_A, "SP_Level_49")
	XT_OPTION(RacerNet::eSP_Level_50_A, "SP_Level_50")
	XT_OPTION(RacerNet::eSP_Level_51_A, "SP_Level_51")
	XT_OPTION(RacerNet::eSP_Level_52_A, "SP_Level_52")
	XT_OPTION(RacerNet::eSP_Level_53_A, "SP_Level_53")
	XT_OPTION(RacerNet::eSP_Level_54_A, "SP_Level_54")
	XT_OPTION(RacerNet::eSP_Level_55_A, "SP_Level_55")
	XT_OPTION(RacerNet::eSP_Level_56_A, "SP_Level_56")
	XT_OPTION(RacerNet::eSP_Level_57_A, "SP_Level_57")
	XT_OPTION(RacerNet::eSP_Level_58_A, "SP_Level_58")
	XT_OPTION(RacerNet::eSP_Level_59_A, "SP_Level_59")
	XT_OPTION(RacerNet::eSP_Level_60_A, "SP_Level_60")
	XT_OPTION(RacerNet::eSP_Level_61_A, "SP_Level_61")
	XT_OPTION(RacerNet::eSP_Level_62_A, "SP_Level_62")
	XT_OPTION(RacerNet::eSP_Level_63_A, "SP_Level_63")
	XT_OPTION(RacerNet::eSP_Level_64_A, "SP_Level_64")
	XT_OPTION(RacerNet::eSP_Level_65_A, "SP_Level_65")
	XT_OPTION(RacerNet::eSP_Level_66_A, "SP_Level_66")
	XT_OPTION(RacerNet::eSP_Level_67_A, "SP_Level_67")
	XT_OPTION(RacerNet::eSP_Level_68_A, "SP_Level_68")
	XT_OPTION(RacerNet::eSP_Level_69_A, "SP_Level_69")
	XT_OPTION(RacerNet::eSP_Level_70_A, "SP_Level_70")
	XT_OPTION(RacerNet::eSP_Level_71_A, "SP_Level_71")
	XT_OPTION(RacerNet::eSP_Level_72_A, "SP_Level_72")
	XT_OPTION(RacerNet::eSP_Level_73_A, "SP_Level_73")
	XT_OPTION(RacerNet::eSP_Level_74_A, "SP_Level_74")
	XT_OPTION(RacerNet::eSP_Level_75_A, "SP_Level_75")
	XT_OPTION(RacerNet::eSP_Level_76_A, "SP_Level_76")
	XT_OPTION(RacerNet::eSP_Level_77_A, "SP_Level_77")
	XT_OPTION(RacerNet::eSP_Level_78_A, "SP_Level_78")
	XT_OPTION(RacerNet::eSP_Level_79_A, "SP_Level_79")
	XT_OPTION(RacerNet::eSP_Level_80_A, "SP_Level_80")
	XT_OPTION(RacerNet::eSP_Level_81_A, "SP_Level_81")
	XT_OPTION(RacerNet::eSP_Level_82_A, "SP_Level_82")
	XT_OPTION(RacerNet::eSP_Level_83_A, "SP_Level_83")
	XT_OPTION(RacerNet::eSP_Level_84_A, "SP_Level_84")
	XT_OPTION(RacerNet::eSP_Level_85_A, "SP_Level_85")
	XT_OPTION(RacerNet::eSP_Level_86_A, "SP_Level_86")
	XT_OPTION(RacerNet::eSP_Level_87_A, "SP_Level_87")
	XT_OPTION(RacerNet::eSP_Level_88_A, "SP_Level_88")
	XT_OPTION(RacerNet::eSP_Level_89_A, "SP_Level_89")
	XT_OPTION(RacerNet::eSP_Level_90_A, "SP_Level_90")
XT_END_TYPE

XT_TYPE(RacerNet::LeaderboardSettings, "LeaderboardSettings")	
	XT_FIELD(mScoreboardToWriteTo, "LeaderboardToWrite")
	XT_FIELD(mScoreboardType, "TypeOfLeaderboard")
XT_END_TYPE

XT_TYPE(Game::LeaderboardType, "AmaxLeaderboard")
	XT_OPTION(Game::E_LEADERBOARDTYPE_POSITION, "Position")
	XT_OPTION(Game::E_LEADERBOARDTYPE_LOWDAMAGE, "LowDamage")
XT_END_TYPE

XT_TYPE(DifficultySettings, "DifficultySettings")
	XT_FIELD(instaPass, "PassImmediatelyOnSuccess")
	XT_FIELD(instaFail, "FailImmediately")
	XT_FIELD(sequentialRules, "SequentialRules")
	XT_FIELD(leaderboardType, "LeaderboardType")
	XT_FIELD(passRules,"PassRules")
	XT_FIELD(failRules,"FailRules")
	XT_FIELD(lapActions,"LapActions")
	XT_FIELD(timeActions,"TimeActions")
	XT_FIELD(aiPerkUse,"PerkAI")
	
	XT_FIELD(ruleStrOverride, "StringOverride")
	XT_FIELD(ruleSeqStrOverride, "SequentialStringOverride")
	XT_FIELD(timeLimit,"TimeLimit")

	XT_FIELD(damageModifier, "DamageModifier")
	XT_FIELD(powerUpDamageModifier, "PowerUpDamageModifier")

	XT_FIELD(aiBalancingFileName, "AIBalancingFileName")

	XT_FIELD(aiRetryInterval,"AIRetryInterval")
	XT_FIELD(aiRetryMinTrackSpeed,"AIRetryMinTrackSpeed")


XT_END_TYPE

XT_TYPE(PickupProbability, "PickupProbability")
	XT_FIELD(pickupType, "PickupType")
	XT_FIELD(probability, "Probability")
XT_END_TYPE

XT_TYPE(PickupProbabilityList, "PickupProbabilityList")
	XT_FIELD(pickupProbabilities, "PickupProbabilities")
XT_END_TYPE

XT_TYPE(UiCircuitType, "UiCircuitType")
	XT_OPTION(UiCircuitType_Sweeping,	"Sweeping")
	XT_OPTION(UiCircuitType_Tight,		"Tight")
	XT_OPTION(UiCircuitType_OffRoad,	"OffRoad")
	XT_OPTION(UiCircuitType_Hilly,		"Hilly")
	XT_OPTION(UiCircuitType_Fast,		"Fast")
	XT_OPTION(UiCircuitType_Technical,	"Technical")
	XT_OPTION(UiCircuitType_Urban,		"Urban")
	XT_OPTION(UiCircuitType_Drifty,		"Drifty")
	XT_OPTION(UiCircuitType_Varied,		"Varied")
	XT_OPTION(UiCircuitType_Oval,		"Oval")
	XT_OPTION(UiCircuitType_Straight,	"Straight")
XT_END_TYPE

XT_TYPE(E_DestructionMode_AI_Level,				"DestructionMode_AI_Level")
	XT_OPTION(DM_AI__None,						"DM_AI_None")
	XT_OPTION(DM_AI__Easy,						"DM_AI_Easy")
	XT_OPTION(DM_AI__Medium,					"DM_AI_Medium")
	XT_OPTION(DM_AI__Hard,						"DM_AI_Hard")
	XT_OPTION(DM_AI__VHard,						"DM_AI_VHard")
XT_END_TYPE

XT_TYPE(E_DestructionMode_Wave_Formation,		"DestructionMode_Wave_Formation")
	XT_OPTION(DM_WF__Line,						"DM_WF_Line")
	XT_OPTION(DM_WF__Queue,						"DM_WF_Queue")
	XT_OPTION(DM_WF__Box,						"DM_WF_Box")
	XT_OPTION(DM_WF__BackLeft_FrontRight,		"DM_WF_BackLeft_FrontRight")
	XT_OPTION(DM_WF__BackRight_FrontLeft,		"DM_WF_BackRight_FrontLeft")
	XT_OPTION(DM_WF__Arrow,						"DM_WF_Arrow")
	XT_OPTION(DM_WF__Diamond,					"DM_WF_Diamond")
XT_END_TYPE

XT_TYPE(DestructionMode_AI,				"DestructionMode_AI")
	XT_FIELD(slot1,						"slot1")
	XT_FIELD(slot2,						"slot2")
	XT_FIELD(slot3,						"slot3")
	XT_FIELD(slot4,						"slot4")
XT_END_TYPE

XT_TYPE(DestructionMode_Wave,			"DestructionMode_Wave")
	XT_FIELD(waveAI,					"waveAI")
	XT_FIELD(waveFormation,				"waveFormation")
	XT_FIELD(waveSeparationDistance,	"waveSeparationDistance")
XT_END_TYPE

XT_TYPE(AmaxRaceModeBase, "AmaxRaceModeBase")
XT_END_TYPE

XT_TYPE(AmaxRaceModeDestruction, "AmaxRaceModeDestruction")
	XT_BASE(AmaxRaceModeBase)
//	XT_FIELD(initialTime,			"initialTime")
	XT_FIELD(timeForWreckEasy,		"timeForWreckEasy")
	XT_FIELD(timeForWreckMedium,	"timeForWreckMedium")
	XT_FIELD(timeForWreckHard,		"timeForWreckHard")
	XT_FIELD(timeForWreckVHard,		"timeForWreckVHard")
	XT_FIELD(behindCutOffDistance,	"BehindCutOffDistance")
	XT_FIELD(vehicleEasy,			"vehicle_Easy")
	XT_FIELD(vehicleMedium,			"vehicle_Medium")
	XT_FIELD(vehicleHard,			"vehicle_Hard")
	XT_FIELD(vehicleVHard,			"vehicle_VHard")
	XT_FIELD(waves,					"waves")
	XT_FIELD(easy,					"easy")
	XT_FIELD(medium,				"medium")
	XT_FIELD(hard,					"hard")
XT_END_TYPE

XT_TYPE(AmaxRaceModeDestructionVars, "AmaxRaceModeDestructionVars")
	XT_FIELD(initialTime, "initialTime")
	XT_FIELD(EasyCarsUsePerks, "EasyCarsUsePerks")
	XT_FIELD(MediumCarsUsePerks, "MediumCarsUsePerks")
	XT_FIELD(HardCarsUsePerks, "HardCarsUsePerks")
	XT_FIELD(VHardCarsUsePerks, "VHardCarsUsePerks")
	XT_FIELD(bronzePoints, "bronzePoints")
	XT_FIELD(silverPoints, "silverPoints")
	XT_FIELD(goldPoints, "goldPoints")
XT_END_TYPE

XT_TYPE(AmaxRaceModeStreetRace, "AmaxRaceModeStreetRace")
	XT_BASE(AmaxRaceModeBase)
XT_END_TYPE

XT_TYPE(AmaxRaceModeCheckpoint, "AmaxRaceModeCheckpoint")
	XT_BASE(AmaxRaceModeBase)

	XT_FIELD(easy,			"easy")
	XT_FIELD(medium,		"medium")
	XT_FIELD(hard,			"hard")
XT_END_TYPE

XT_TYPE(AmaxRaceModeCheckpointVars, "AmaxRaceModeCheckpointVars")
	XT_FIELD(initialTime,			"initialTime")
	XT_FIELD(stopwatchIncTime,		"stopwatchIncTime")
	XT_FIELD(timeAwarded,			"timeAwarded")

//	XT_FIELD(bronzeTimeRemaining,	"bronzeTimeRemaining")
	XT_FIELD(silverTimeRemaining,	"silverTimeRemaining")
	XT_FIELD(goldTimeRemaining,		"goldTimeRemaining")
XT_END_TYPE

XT_TYPE(AmaxRaceModeFanDemand, "AmaxRaceModeFanDemand")
	XT_BASE(AmaxRaceModeBase)

	XT_FIELD(easy,			"easy")
	XT_FIELD(medium,		"medium")
	XT_FIELD(hard,			"hard")
XT_END_TYPE

XT_TYPE(AmaxRaceModeFanDemandVars, "AmaxRaceModeFanDemandVars")

	XT_FIELD(BronzeNumToComplete,	"BronzeNumToComplete")
	XT_FIELD(SilverNumToComplete,	"SilverNumToComplete")
	XT_FIELD(GoldNumToComplete,		"GoldNumToComplete")
XT_END_TYPE



XT_TYPE(AmaxRaceModeBossBattle,		"AmaxRaceModeBossBattle")
	XT_BASE(AmaxRaceModeBase)

	XT_FIELD(distanceFromMinders,		"distanceFromMinders")
	XT_FIELD(timeInFrontOfMinders,		"timeInFrontOfMinders")
	XT_FIELD(wreckMindersToWin,			"wreckMindersToWin")

	XT_FIELD(distanceFromBoss,			"distanceFromBoss")
	XT_FIELD(timeInFrontOfBoss,			"timeInFrontOfBoss")
	XT_FIELD(wreckBossToWin,			"wreckBossToWin")

//	XT_FIELD(boss,						"boss")
//	XT_FIELD(minders,					"minders")
XT_END_TYPE

AMAX_GAMEEVENTDATA_CALLBACK(AmaxEvent, mEvents)
XT_TYPE(AmaxEvent,"AmaxEvent")
	XT_CALLBACK((GameDataHandler<AmaxEvent,CallbackAmaxEvent>))
	XT_BASE(AmaxGameDataObj)

	XT_FIELD(route, "Route")
	XT_FIELD(allocateCash,"AllocateCash")
	XT_FIELD(moveToEv, "MoveDirectlyToEvent")
	XT_FIELD(postRaceMessage, "PostRaceMessage")

	XT_FIELD(aiDontPickupPickups, "AIDontGoForPickups")
	XT_FIELD(pickupProbabilities, "PickupProbabilities")
	XT_FIELD(pickupDisableTime, "PickupDisableTime")
	XT_FIELD(pickupSpawnLevel, "PickupSpawnLevel")
	XT_FIELD(disableFans, "DisableFans")
	XT_FIELD(specificAudioLinesOnly, "SpecificAudioLinesOnly")
	XT_FIELD(pickupsDoNotRespawn, "PickupsDoNotRespawn")
	XT_FIELD(pickupsDropOnUse, "PickupsDropOnUse")
	XT_FIELD(dontShowStartFinishInfo, "DontShowStartFinishInfo")
	XT_FIELD(dialogueProbability, "DialogueProbability")
	XT_FIELD(mLeaderboardSettings, "Leaderboards")
	XT_FIELD(raceMode,				"RaceMode")

	XT_FIELD(laps,"Laps")
	XT_FIELD(carClass, "CarClass")

	XT_FIELD(easyVersion,"Easy")
	XT_FIELD(normalVersion,"Medium")
	XT_FIELD(hardVersion,"Hard")

	XT_FIELD(actorLists, "ActorLists")

	XT_FIELD(opponents,"Opponents")
	XT_FIELD(mAudioMusicID,"AudioMusicID")

	XT_FIELD(overrideAISurface,"OverrideAISurface")
	XT_FIELD(overrideAISurfaceValue,"OverrideAISurfaceValue")
	

XT_END_TYPE

AMAX_GAMEEVENTDATA_CALLBACK(AmaxSplitScreenEvent, mSplitScreenEvents)
XT_TYPE(AmaxSplitScreenEvent,					"AmaxSplitScreenEvent")
	XT_CALLBACK((GameDataHandler<AmaxSplitScreenEvent, CallbackAmaxSplitScreenEvent>))
	XT_BASE(AmaxEvent)

	XT_FIELD(mFansToUnlock,						"FansToUnlock")
	
	XT_FIELD(mAllowRoute,						"AllowRoute")
	XT_FIELD(mAllowCarClass,					"AllowCarClass")
	XT_FIELD(mAllowLaps,						"AllowLaps")
	XT_FIELD(mAllowPadAIVehicles,				"AllowPadAIVehicles")
	XT_FIELD(mDisableRacingContextFanRewards,	"DisableRacingContextFanRewards")
	XT_FIELD(mAllowHandicap,					"AllowHandicap")
	XT_FIELD(mNoRollingStart,					"NoRollingStart")
	XT_FIELD(mAllowPickupsOnLapZero,			"AllowPickupsOnLapZero")
	XT_FIELD(mDisableWorldHUDPositionInfo,		"DisableWorldHUDPositionInfo")
	XT_FIELD(mIconSheet,						"icon_sheet")
	XT_FIELD(mIconName,							"icon_name")
	XT_FIELD(mModeName,							"mode_name")
	XT_FIELD(mDisableRaceLineUIElement,			"DisableRaceLineUIElement")
	XT_FIELD(mShufflePowerupsOnRespawn,			"ShufflePowerUpsOnOnRespawn")
XT_END_TYPE
// ------------------------------------------------------------------

XT_TYPE(E_STUNT_ACTION, "StuntMove")
	XT_OPTION(E_STUNT_ACTION_DRIFTING,	"Drifting")
XT_END_TYPE

XT_TYPE(AmaxEventActionBase,"AmaxEventActionBase")
	XT_FIELD(timer, "Timer")
XT_END_TYPE

XT_TYPE(AmaxEliminateDrivers,"EliminateDrivers")
	XT_BASE(AmaxEventActionBase)
	XT_FIELD(numDrivers, "NumDrivers")
XT_END_TYPE

XT_TYPE(AmaxEliminateHitByPerks, "EliminateHitByPerks")
	XT_BASE(AmaxEventActionBase)
	XT_FIELD(numHits, "NumHits")
	XT_FIELD(firingDriver, "FiringDriver")
	XT_FIELD(pickups, "Pickups")
XT_END_TYPE

XT_TYPE(AmaxTimeAttackDisplay,"TimeAttackDisplay")
	XT_BASE(AmaxEventActionBase)
XT_END_TYPE

XT_TYPE(AmaxManageGhosts,"ManageGhosts")
	XT_BASE(AmaxEventActionBase)
XT_END_TYPE

// ------------------------------------------------------------------

XT_TYPE( Game::RaceModeRuleType, "RaceRuleType" )
	XT_OPTION( Game::E_RaceRule_FinishInPosition, "FinishInPosition" )
	XT_OPTION( Game::E_RaceRule_GetLapTime, "GetLapTime" )
	XT_OPTION( Game::E_RaceRule_HitMph, "HitMph" )
	XT_OPTION( Game::E_RaceRule_BeatDriver, "BeatDriver" )
	XT_OPTION( Game::E_RaceRule_DriverLastInRace, "DriverLastInRace" )
	XT_OPTION( Game::E_RaceRule_PerformActionYTimes, "PerformActionYTimes" )
	XT_OPTION( Game::E_RaceRule_DriverHasHealth, "DriverHasHealth" )
	XT_OPTION( Game::E_RaceRule_DistanceFromDriver, "DistanceFromDriver" )
	XT_OPTION( Game::E_RaceRule_CauseDamageToDriver, "CauseDamageToDriver" )
	XT_OPTION( Game::E_RaceRule_HitDriverWithPerk, "HitDriverWithPerk" )
	XT_OPTION( Game::E_RaceRule_EarnXP, "EarnXP" )
	XT_OPTION( Game::E_RaceRule_CollectFanPickups, "CollectFanPickups" )
	XT_OPTION( Game::E_RaceRule_GetFastestLap, "GetFastestLap" )
	XT_OPTION( Game::E_RaceRule_InTriggerRegion, "InTriggerRegion" )
	XT_OPTION( Game::E_RaceRule_HitObjects, "HitObjects" )
	XT_OPTION( Game::E_RaceRule_TeamPosition, "TeamPosition" )
XT_END_TYPE

XT_TYPE(AmaxRuleBase,"AmaxRuleBase")
	XT_FIELD(persistent, "Persistent")
XT_END_TYPE

XT_TYPE(AmaxTeamPosition, "TeamPosition")
	XT_BASE(AmaxRuleBase)
XT_END_TYPE

XT_TYPE(AmaxDriverLastInRace, "DriverLastInRace")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
XT_END_TYPE

XT_TYPE(AmaxDriverCollectXFanPickups,"DriverCollectXFanPickups")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(numPickups, "NumberOfPickups")
XT_END_TYPE

XT_TYPE(AmaxDriverFireXPerks, "DriverFiredXPerks")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(numFired, "NumFired")
	XT_FIELD(pickups, "Pickups")
XT_END_TYPE

XT_TYPE(AmaxDriverFinishInXPosition,"DriverFinishInXPosition")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driver, "DriverOverride")
	XT_FIELD(position, "PositionNeeded")
XT_END_TYPE

XT_TYPE(AmaxBeatDriver,"BeatDriver")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(driverToBeat, "DriverToBeat")
XT_END_TYPE

XT_TYPE(AmaxPerformActionYTimes,"DriverPerformXMoveYTimes")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(numTimes, "NumTimes")
	XT_FIELD(action, "Move")
XT_END_TYPE

XT_TYPE(AmaxDriverPerformXTrickShots, "DriverPerformedXTrickShots")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(numShots, "NumShots")
XT_END_TYPE

XT_TYPE(AmaxDistanceFromDriver, "DistanceFromDriver")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(driverDistance, "DriverDistance")
	XT_FIELD(quantifier, "Condition")
	XT_FIELD(distance, "Distance")
	XT_FIELD(time, "Time")
XT_END_TYPE

XT_TYPE(AmaxDriverGetLapTimeOfX,"DriverGetLapTimeOfX")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(lapTime, "LapTimeNeeded")
XT_END_TYPE

XT_TYPE(AmaxDriverHitDriverXTimesWithPerks,"HitDriverXTimesWithPerks")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(driverToHit, "DriverToHit")
	XT_FIELD(numHits, "NumHits")
	XT_FIELD(pickupTypes, "PickupType")
	XT_FIELD(requireDoubleHits, "RequireDoubleHits")
XT_END_TYPE

XT_TYPE(AmaxDriverHitXMph, "DriverHitXMph")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(speed, "Speed")
	XT_FIELD(areaRef, "TriggerAreaNumber")
	XT_FIELD(quantifier, "Condition")
XT_END_TYPE

XT_TYPE(AmaxDriverDriftingInTriggerRegion, "DriverDriftingInTriggerRegion")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride,	"DriverOverride")
	XT_FIELD(areaRef,			"TriggerAreaNumber")
XT_END_TYPE

XT_TYPE(AmaxDriverHasHealth, "DriverHasXHealth")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(quantifier, "Condition")
	XT_FIELD(driver, "DriverOverride")
	XT_FIELD(health, "Health")
XT_END_TYPE

XT_TYPE(AmaxCauseDamageToDriver, "CauseXDamageToDriver")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(quantifier, "Condition")
	XT_FIELD(driver, "DriverToHurt")
	XT_FIELD(damage, "Damage")
XT_END_TYPE

XT_TYPE(AmaxDriverEarnXP, "DriverEarnedXP")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(xpNeeded, "XP")
XT_END_TYPE

XT_TYPE(AmaxDriverFastestLap, "DriverGetFastestLap")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
XT_END_TYPE

XT_TYPE(AmaxDriverHitXObjects, "DriverHitXObjects")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(driverOverride, "DriverOverride")
	XT_FIELD(groupID, "GroupID")
XT_END_TYPE

XT_TYPE(AmaxDriverTimerExpired, "TimerExpired")
	XT_BASE(AmaxRuleBase)
	XT_FIELD(initialTime,	"InitialTime")
	XT_FIELD(numZones,		"NumTriggerZones")
	XT_FIELD(timeAwarded,	"TimeAwarded")
XT_END_TYPE

XT_TYPE(E_AnimaticTrigger, "AnimaticTrigger" )
	XT_OPTION( E_AnimaticTrigger_None,		"None" )
	XT_OPTION( E_AnimaticTrigger_PreEvent,	"PreEvent" )
	XT_OPTION( E_AnimaticTrigger_PostEvent, "PostEvent" )
XT_END_TYPE

XT_TYPE(AnimaticSubtitle,		"AnimaticSubtitle" )
	XT_FIELD( subtitle,		"Subtitle" )
	XT_FIELD( starttime,		"StartTime" )
	XT_FIELD( life,			"Life" )
XT_END_TYPE

XT_LIBRARY(GAMEEVENTDATA)
	XT_REGISTER_TYPE(AmaxCutScene)

	XT_REGISTER_TYPE(AmaxWinCondition)
	XT_REGISTER_TYPE(AmaxRaceModeBase)
	XT_REGISTER_TYPE(AmaxRaceModeStreetRace)
	XT_REGISTER_TYPE(AmaxRaceModeDestruction)
	XT_REGISTER_TYPE(AmaxRaceModeDestructionVars)
	XT_REGISTER_TYPE(AmaxRaceModeCheckpoint)
	XT_REGISTER_TYPE(AmaxRaceModeCheckpointVars)
	XT_REGISTER_TYPE(AmaxRaceModeFanDemand)
	XT_REGISTER_TYPE(AmaxRaceModeFanDemandVars)
	XT_REGISTER_TYPE(AmaxRaceModeBossBattle)
	XT_REGISTER_TYPE(AmaxEvent)
	XT_REGISTER_TYPE(AmaxSplitScreenEvent)
	XT_REGISTER_TYPE(AmaxOpponentSetup)
	XT_REGISTER_TYPE(Game::LeaderboardType)
	XT_REGISTER_TYPE(DifficultySettings)
	XT_REGISTER_TYPE(AmaxPerkAI)
	XT_REGISTER_TYPE(UiCircuitType)

	XT_REGISTER_TYPE(E_STUNT_ACTION)
	XT_REGISTER_TYPE(Game::RaceModeRuleType)
	XT_REGISTER_TYPE(AmaxRuleBase)
	XT_REGISTER_TYPE(AmaxDriverCollectXFanPickups)
	XT_REGISTER_TYPE(AmaxDriverLastInRace)
	XT_REGISTER_TYPE(AmaxDriverFinishInXPosition)
	XT_REGISTER_TYPE(AmaxBeatDriver)
	XT_REGISTER_TYPE(AmaxDriverGetLapTimeOfX)
	XT_REGISTER_TYPE(AmaxDriverHitDriverXTimesWithPerks)
	XT_REGISTER_TYPE(AmaxDriverHitXMph)
	XT_REGISTER_TYPE(AmaxDriverDriftingInTriggerRegion)
	XT_REGISTER_TYPE(AmaxDriverHasHealth)
	XT_REGISTER_TYPE(AmaxDriverEarnXP)
	XT_REGISTER_TYPE(AmaxDriverFastestLap)
	XT_REGISTER_TYPE(AmaxCauseDamageToDriver)
	XT_REGISTER_TYPE(AmaxDistanceFromDriver)
	XT_REGISTER_TYPE(AmaxDriverHitXObjects)
	XT_REGISTER_TYPE(AmaxPerformActionYTimes)
	XT_REGISTER_TYPE(AmaxTeamPosition)
	XT_REGISTER_TYPE(AmaxDriverFireXPerks)
	XT_REGISTER_TYPE(AmaxDriverTimerExpired)
	XT_REGISTER_TYPE(AmaxDriverPerformXTrickShots)

	XT_REGISTER_TYPE(AmaxEventActionBase)
	XT_REGISTER_TYPE(AmaxEliminateDrivers)
	XT_REGISTER_TYPE(AmaxEliminateHitByPerks)
	XT_REGISTER_TYPE(AmaxTimeAttackDisplay)
	XT_REGISTER_TYPE(AmaxManageGhosts)
	
	XT_REGISTER_TYPE(AmaxIntroSection)
	XT_REGISTER_TYPE(AmaxStoryIntroOutro)
	XT_REGISTER_TYPE(E_AnimaticTrigger)
	XT_REGISTER_TYPE(AnimaticSubtitle)

	XT_REGISTER_TYPE(RacerNet::eScoreboards)
	XT_REGISTER_TYPE(RacerNet::eSPScoreboardTypes)
	XT_REGISTER_TYPE(RacerNet::LeaderboardSettings)	

	XT_REGISTER_TYPE(E_DestructionMode_AI_Level)	
	XT_REGISTER_TYPE(E_DestructionMode_Wave_Formation)	
	XT_REGISTER_TYPE(DestructionMode_AI)	
	XT_REGISTER_TYPE(DestructionMode_Wave)	

XT_END_LIBRARY

void C_GameEventData::Init()
{
	mEvents.TagSort();

	//printf("Events\n");
	//for(int a=0;a<mEvents.Number();a++)
	//{
	//	printf("%x = %S\n",mEvents[a]->ID,UI::C_TextStringID(mEvents[a]->loc_string.id).String());
	//}
}

// ---------------------------------------------------------------
// Creation Instructions for Actions
// ---------------------------------------------------------------

Game::RaceActionBase* AmaxEliminateDrivers::CreateAction()
{
	Game::RaceAction_EliminateLastDriver* raceAction = new Game::RaceAction_EliminateLastDriver();
	raceAction->mNumDrivers = numDrivers;

	SetBaseValues(raceAction);
	return raceAction;
}

Game::RaceActionBase* AmaxEliminateHitByPerks::CreateAction()
{
	Game::RaceAction_EliminateHitByPerks* raceAction = new Game::RaceAction_EliminateHitByPerks();

	if(firingDriver.Object())
		raceAction->SetFirerID(firingDriver->ID);

	for( int i = 0; i < pickups.count; i++ )
		raceAction->AddPickup(pickups[i]);

	raceAction->SetNumberOfLives(numHits);

	SetBaseValues(raceAction);
	return raceAction;
}

Game::RaceActionBase* AmaxTimeAttackDisplay::CreateAction()
{
	Game::RaceAction_TimeAttackDisplay* raceAction = new Game::RaceAction_TimeAttackDisplay();

	SetBaseValues(raceAction);
	return raceAction;
}

Game::RaceActionBase* AmaxManageGhosts::CreateAction()
{
	Game::RaceAction_ManageGhosts* raceAction = new Game::RaceAction_ManageGhosts();

	SetBaseValues(raceAction);
	return raceAction;
}

// ---------------------------------------------------------------
// Creation Instructions for Rules
// ---------------------------------------------------------------

Game::RaceModeRuleBase* AmaxTeamPosition::CreateRule()
{
	Game::RaceModeRule_TeamPosition* raceRule = new Game::RaceModeRule_TeamPosition();
	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverPerformXTrickShots::CreateRule()
{
	Game::RaceModeRule_DriverPerformedXTrickShots* raceRule = new Game::RaceModeRule_DriverPerformedXTrickShots();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetNumTrickShots(numShots);
	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverTimerExpired::CreateRule()
{
	Game::RaceModeRule_TimerExpired* raceRule = new Game::RaceModeRule_TimerExpired();
	raceRule->SetTimer(initialTime);
	raceRule->SetNumTriggerZones(numZones);

	for( int i = 0; i < timeAwarded.count; i++ )
		raceRule->AddTimeAwarded(timeAwarded[i]);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverFireXPerks::CreateRule()
{
	Game::RaceModeRule_FireXPerks* raceRule = new Game::RaceModeRule_FireXPerks();

	raceRule->SetNumFiredRule(numFired);

	for( int i = 0; i < pickups.count; i++ )
		raceRule->AddPickupType(pickups[i]);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverLastInRace::CreateRule()
{
	Game::RaceModeRule_DriverLastInRace* raceRule = new Game::RaceModeRule_DriverLastInRace();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);
	else
		raceRule->SetDriverRule(0);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverCollectXFanPickups::CreateRule()
{
	Game::RaceModeRule_CollectXFanPickups* raceRule = new Game::RaceModeRule_CollectXFanPickups();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);
	else
		raceRule->SetDriverRule(0);

	raceRule->SetNumPickupsRule(numPickups);
	raceRule->SetPersistent(persistent);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverHitXObjects::CreateRule()
{
	Game::RaceModeRule_DriverHitXObjects* raceRule = new Game::RaceModeRule_DriverHitXObjects();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);
	else
		raceRule->SetDriverRule(0);

	raceRule->SetGroupID(groupID);
	raceRule->SetPersistent(persistent);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverFinishInXPosition::CreateRule()
{
	Game::RaceModeRule_DriverFinishInXPosition* raceRule = new Game::RaceModeRule_DriverFinishInXPosition();

	if(position != -1)
		raceRule->SetPositionRule(position);
	else
		raceRule->SetPositionRule(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mLastSavedRacePosition);

	if(driver.Object())
	{
		raceRule->SetDriverRule(driver->ID);
		raceRule->SetDriverToBeatLoc(driver->loc_string);
		raceRule->CacheStrings();
	}
	else
		raceRule->SetDriverRule(0);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDistanceFromDriver::CreateRule()
{
	Game::RaceModeRule_DriverDistanceFromDriver* raceRule = new Game::RaceModeRule_DriverDistanceFromDriver();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetDriverDistanceRule(driverDistance->ID);
	raceRule->SetDriverDistanceLocId(driverDistance->loc_string);
	raceRule->SetQuantifier(quantifier);
	raceRule->SetDistanceRule(distance);
	raceRule->SetTimeRule(time);
	raceRule->CacheStrings();

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxBeatDriver::CreateRule()
{
	Game::RaceModeRule_BeatDriver* raceRule = new Game::RaceModeRule_BeatDriver();

	if(driverOverride.Object())
		raceRule->SetDriveroverrideRule(driverOverride->ID);

	raceRule->SetDriverToBeatRule(driverToBeat->ID);
	raceRule->SetDriverToBeatLoc(driverToBeat->loc_string);
	raceRule->CacheStrings();

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxPerformActionYTimes::CreateRule()
{
	Game::RaceModeRule_PerformActionYTimes* raceRule = new Game::RaceModeRule_PerformActionYTimes();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetNumTimes(numTimes);
	raceRule->SetAction(action);
	raceRule->CacheStrings();

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverGetLapTimeOfX::CreateRule()
{
	Game::RaceModeRule_DriverGetLapTimeOfX* raceRule = new Game::RaceModeRule_DriverGetLapTimeOfX();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetLapTimeRule(lapTime);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverHitDriverXTimesWithPerks::CreateRule()
{
	Game::RaceModeRule_DriverHitDriverXTimesWithPerk* raceRule = new Game::RaceModeRule_DriverHitDriverXTimesWithPerk();

	if(driverOverride.Object())
		raceRule->SetDriverOverrideRule(driverOverride->ID);

	if(driverToHit.Object())
	{
		raceRule->SetDriverRule(driverToHit->ID);
		raceRule->SetDriverLoc(driverToHit->loc_string);
	}

	raceRule->SetNumHitsRule(numHits);
	raceRule->SetRequireDoubleRule(requireDoubleHits);

	for( int i = 0; i < pickupTypes.count; i++ )
		raceRule->AddPickupType(pickupTypes[i]);

	raceRule->CacheStrings();
	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverHitXMph::CreateRule()
{
	Game::RaceModeRule_DriverHitXMph* raceRule = new Game::RaceModeRule_DriverHitXMph();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetSpeedRule(speed);
	raceRule->SetQuantifier(quantifier);
	raceRule->SetAreaRefRule(areaRef);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverDriftingInTriggerRegion::CreateRule()
{
	Game::RaceModeRule_DriverDriftingInTriggerRegion* raceRule = new Game::RaceModeRule_DriverDriftingInTriggerRegion();

	if(driverOverride.Object())
		raceRule->SetDriverRule(driverOverride->ID);

	raceRule->SetAreaRefRule(areaRef);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverHasHealth::CreateRule()
{
	Game::RaceModeRule_DriverHasHealth* raceRule = new Game::RaceModeRule_DriverHasHealth();

	raceRule->SetHealthRule(health);
	raceRule->SetQuantifier(quantifier);
	if( driver.Object() )
	{
		raceRule->SetDriverRule(driver->ID);
		raceRule->SetDriverLocId(driver->loc_string);
		raceRule->CacheStrings();
	}

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxCauseDamageToDriver::CreateRule()
{
	Game::RaceModeRule_CauseXDamageToDriver* raceRule = new Game::RaceModeRule_CauseXDamageToDriver();

	raceRule->SetDamageRule(damage);
	raceRule->SetQuantifier(quantifier);
	if( driver.Object() )
	{
		raceRule->SetDriverRule(driver->ID);
		raceRule->SetDriverLocId(driver->loc_string);
		raceRule->CacheStrings();
	}

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverEarnXP::CreateRule()
{
	Game::RaceModeRule_DriverEarnXP* raceRule = new Game::RaceModeRule_DriverEarnXP();

	raceRule->SetXpRule(xpNeeded);

	if( driverOverride.Object() )
		raceRule->SetDriverRule(driverOverride->ID);

	SetBaseValues(raceRule);

	return raceRule;
}

Game::RaceModeRuleBase* AmaxDriverFastestLap::CreateRule()
{
	Game::RaceModeRule_DriverGetFastestLap* raceRule = new Game::RaceModeRule_DriverGetFastestLap();

	if( driverOverride.Object() )
		raceRule->SetDriverRule(driverOverride->ID);

	SetBaseValues(raceRule);

	return raceRule;
}
