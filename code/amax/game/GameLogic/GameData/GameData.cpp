#include "stdafx.h"

#include <Game/Game.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/PerfStats/PerfStats.h>

#include <GameLogic/GameData/LocData.h>
#include <GameLogic/GameData/GameData.h>
#include <GameApp.h>

C_GameData C_GameData::s_instance;

//Debug::Bool g_debugCheckForSorceryUpdatesOnDisk("Game.CheckForSorceryDataExportsWhileRunning", false, Debug::Persist());

#define AMAX_GAMEDATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_GameData::Instance().Member; } };

XT_TYPE(AmaxDLCBase,"DLCBase")
	XT_FIELD(dlcPack,"DLCPack")
	XT_FIELD(dlcLicenseBit,"DLCKey")
	XT_FIELD(dlcUnlockId,"DLCUnlockId")
XT_END_TYPE

XT_TYPE(AmaxGameDataObj,"AmaxGameDataObject")
	XT_BASE(AmaxDLCBase)
	XT_FIELD(tag,"Name")
	XT_FIELD(index,"Index")
	XT_FIELD(loc_string,"LocString")
	XT_FIELD(dlcPack,"DLCPack")
	XT_FIELD(dlcLicenseBit,"DLCKey")
XT_END_TYPE


void TitleStorageSettingsHandler(XtEventType event, XtRecord* record, TitleManagedStorageSettings* object, XtCallbackContext* context)
{
	XtHandle<TitleManagedStorageSettings>& settings=C_GameData::Instance().mTitleManagedStorageSettings;

	switch(event)
	{
		case kXtEtConstruct:
		{
			if (settings!=0)
			{
				Debug::Logger& g_Log = Debug::GetLogger( "Amax.Multiplayer" );
				LOGERROR(g_Log, "Multiple TitleManagedStorageSettings exist in Sorcery - makes no sense, get rid of the redundant ones.");
			}
			settings=XtHandle<TitleManagedStorageSettings>(record);
			break;
		}

		case kXtEtDestruct:
		{
			settings=XtHandle<TitleManagedStorageSettings>(0);
			break;
		}
	}
}

void PickupScriptHandler(XtEventType event, XtRecord* record, PickupFiringScriptManager* object, XtCallbackContext* context)
{
	XtHandle<PickupFiringScriptManager>& pickupScript = C_GameData::Instance().mPickupFiringScriptManager;

	switch(event)
	{
	case kXtEtConstruct:
		{
			if (pickupScript!=0)
			{
				Debug::Logger& g_Log = Debug::GetLogger( "Amax.Multiplayer" );
				LOGERROR(g_Log, "Multiple TitleManagedStorageSettings exist in Sorcery - makes no sense, get rid of the redundant ones.");
			}
			pickupScript=XtHandle<PickupFiringScriptManager>(record);
			break;
		}

	case kXtEtDestruct:
		{
			pickupScript=XtHandle<PickupFiringScriptManager>(0);
			break;
		}
	}
}

void StartLineInfoSpecialFXHandler(XtEventType event, XtRecord* record, StartLineInfoSpecialFXList* object, XtCallbackContext* context)
{
	XtHandle<StartLineInfoSpecialFXList>& list = C_GameData::Instance().mStartLineInfoSpecialFXList;

	switch(event)
	{
	case kXtEtConstruct:
		{
			if (list != 0)
			{
				Debug::Logger& g_Log = Debug::GetLogger( "Amax.Multiplayer" );
				LOGERROR(g_Log, "Multiple StartLineInfoSpecialFXList exist in Sorcery - makes no sense, get rid of the redundant ones.");
			}
			list = XtHandle<StartLineInfoSpecialFXList>(record);
			break;
		}

	case kXtEtDestruct:
		{
			list = XtHandle<StartLineInfoSpecialFXList>(0);
			break;
		}
	}
}

XT_TYPE(TitleManagedStorageSettings,"TitleManagedStorageSettings")
	XT_CALLBACK(TitleStorageSettingsHandler)
	XT_FIELD(mMultiplayerPackFilename,"MultiplayerPackFilename")
	XT_FIELD(mMultiplayerLocFilename,"MultiplayerLocFilename")
XT_END_TYPE

XT_TYPE(StartLineInfoSpecialFXList,	"StartLineInfoSpecialFXList")
	XT_CALLBACK(StartLineInfoSpecialFXHandler)
	XT_FIELD(mStartSpecialFX,		"StartSpecialFX")
	XT_FIELD(mCheckpointSpecialFX,	"CheckpointSpecialFX")
	XT_FIELD(mFinalSpecialFX,		"FinalSpecialFX")
	XT_FIELD(mFinishSpecialFX,		"FinishSpecialFX")
XT_END_TYPE

XT_TYPE(StartLineInfoSpecialFX,		"StartLineInfoSpecialFX")
	XT_FIELD(mEnglish,				"SpecialEffectEnglish")
	XT_FIELD(mFrench,				"SpecialEffectFrench")
	XT_FIELD(mGerman,				"SpecialEffectGerman")
	XT_FIELD(mItalian,				"SpecialEffectItalian")
	XT_FIELD(mSpanish,				"SpecialEffectSpanish")
XT_END_TYPE

XT_TYPE(ReflectionResolution, "ReflectionResolutionEnum")
	XT_OPTION(ReflectionResolution_Medium_512x256,				"Medium_512x256")	
	XT_OPTION(ReflectionResolution_VeryLow_128x64,				"VeryLow_128x64")
	XT_OPTION(ReflectionResolution_Low_256x128,					"Low_256x128")
	XT_OPTION(ReflectionResolution_High_1024x512,				"High_1024x512")	
XT_END_TYPE

XT_TYPE(EnvironmentResolution, "EnvironmentResolutionEnum")
	XT_OPTION(EnvironmentResolution_High_256,				"High_256")	
	XT_OPTION(EnvironmentResolution_Medium_128,				"Medium_128")	
	XT_OPTION(EnvironmentResolution_Low_64,					"Low_64")	
XT_END_TYPE

XT_TYPE(Game::AmaxPlatformAllocation,	"AmaxPlatformAllocation")
	XT_FIELD(platform360,				"platform360")
	XT_FIELD(platformPS3,				"platformPS3")
	XT_FIELD(platformPC,				"platformPC")
XT_END_TYPE

XT_TYPE(Game::E_AmaxMemType,		"AmaxMemType")
	XT_OPTION(Game::E_AmaxMemType_Combined,		"Combined")	
	XT_OPTION(Game::E_AmaxMemType_System,			"System")	
	XT_OPTION(Game::E_AmaxMemType_Graphics,		"Graphics")	
XT_END_TYPE

XT_TYPE(Game::AmaxMemBudget,	"AmaxMemBudget")
	XT_FIELD(renderName,			"renderName")	
	XT_FIELD(trackerName,			"trackerName")
	XT_FIELD(memType,				"memType")	
	XT_FIELD(size,					"size")
	XT_FIELD(dontUse,				"dontUse")
XT_END_TYPE

XT_TYPE(Game::AmaxMemBudgets,	"AmaxMemBudgets")
	XT_FIELD(budgets,				"budgets")
XT_END_TYPE

XT_TYPE(Game::AmaxRoutePerfStats,	"AmaxRoutePerfStats")
	XT_FIELD(routeNumber,				"routeNumber")
	XT_FIELD(dynObjectsMaxNum,			"dynObjectsMaxNum")
	XT_FIELD(physObjectsMaxNum,			"physObjectsMaxNum")
	XT_FIELD(memBudgets,			"memBudgets")
XT_END_TYPE

XT_TYPE(Game::AmaxCityPerfStats,	"AmaxCityPerfStats")
	XT_FIELD(cityDefaults,				"cityDefaults")
	XT_FIELD(routeStats,				"routeStats")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxCity, mCities)
XT_TYPE(AmaxCity,"City")
	XT_CALLBACK((GameDataHandler<AmaxCity,CallbackAmaxCity>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(devOnly, "DevOnly")
	XT_FIELD(icon_name, "IconName")
	XT_FIELD(actorListFolder, "actorListFolder")
	XT_FIELD(actorLists, "ActorLists")
	XT_FIELD(multiplayerActorLists, "MultiplayerActorLists")	
	XT_FIELD(environmentSettings, "EnvironmentSettings")
	XT_FIELD(flashbackSettings, "FlashbackSettings")
	XT_FIELD(headlightIntensityScale, "headlightIntensityScale")
	XT_FIELD(bumperCameraHeadlightIntensityOffset, "bumperCameraHeadlightIntensityOffset")
	XT_FIELD(vehicleDirtSettings, "vehicleDirtSettings")	
	XT_FIELD(reflectionResolution, "reflectionResolution")	
	XT_FIELD(environmentResolution, "environmentResolution")	
	XT_FIELD(environmentFarClip, "environmentFarClip")
	XT_FIELD(perfStats, "perfStats")
	XT_FIELD(surfaceMappings, "surfaceMappings")	
	XT_FIELD(boundsMin, "boundsMin")
	XT_FIELD(boundsMax, "boundsMax")
	XT_FIELD(crowdFlagTextures, "CrowdFlagTextures")
	XT_FIELD(forceFallIntoWater, "ForceFallIntoWater")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxRoute, mRoutes)
XT_TYPE(AmaxRoute,"Route")
	XT_CALLBACK((GameDataHandler<AmaxRoute,CallbackAmaxRoute>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(city,"City")
	XT_FIELD(route_number,"RouteNumber")
	XT_FIELD(thumbnail,"Thumbnail")
	XT_FIELD(actorLists, "ActorLists")
	XT_FIELD(multiplayerActorLists, "MultiplayerActorLists")	
	XT_FIELD(cutsceneActorLists, "CutsceneActorLists")	
	XT_FIELD(preLoadList, "PreLoadedModelList")
	XT_FIELD(minYPosition, "MinimumYPosition")
	XT_FIELD(best_times, "BestTimes")
	XT_FIELD(fansToUnlockInSplitScreen, "FansToUnlockInSplitScreen")
	XT_FIELD(onlyAllowsSingleLap, "OnlyAllowSingleLap")
	XT_FIELD(circuitType, "CircuitType" )
	XT_FIELD(VistaCameraSequence, "VistaCameraSequence" )
	XT_FIELD(PackCameraSequence, "PackCameraSequence" )
	XT_FIELD(PackCameraSequenceMP, "PackCameraSequenceMP" )
	XT_FIELD(PackCameraSequenceSingle, "PackCameraSequenceSingle" )
	XT_FIELD(PackCameraSequenceBoss, "PackCameraSequenceBoss" )
	XT_FIELD(shockWidthWeighting, "ShockWidthWeighting")
	XT_FIELD(CrowdPopulation, "CrowdPopulationData")
	XT_FIELD(handOfGodNoSplineResetSpeedInMPH, "HandOfGodNoSplineResetSpeedInMPH")
	XT_FIELD(mAudioVistaID,"AudioVistaID")
XT_END_TYPE


XT_TYPE(TimeAttackBestTimeEntry,"TimeAttackBestTimeEntry")
	XT_FIELD(name,"Name")
	XT_FIELD(time_in_secs,"TimeInSecs")
XT_END_TYPE

XT_TYPE(AmaxPreLoadList,"PreLoadList")
	XT_FIELD(modelList,"Model")
XT_END_TYPE

XT_TYPE(AmaxPhysicsMaterial,"PhysicsMaterial")
	XT_OPTION(AmaxPhysicsMaterial_Default,"Default")
	XT_OPTION(AmaxPhysicsMaterial_Medium,"Medium")
	XT_OPTION(AmaxPhysicsMaterial_Low,"Low")
	XT_OPTION(AmaxPhysicsMaterial_Bouncy,"Bouncy")
	XT_OPTION(AmaxPhysicsMaterial_Vehicle,"Vehicle")	
	XT_OPTION(AmaxPhysicsMaterial_Object,"Object")
	XT_OPTION(AmaxPhysicsMaterial_ObjectBouncy,"ObjectBouncy")	
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxFrictionSettings, mFrictionSettings)
XT_TYPE(AmaxFrictionSettings,"AmaxFrictionSettings")
	XT_CALLBACK((GameDataHandler<AmaxFrictionSettings,CallbackAmaxFrictionSettings>))
	XT_FIELD(material1,"Material1")
	XT_FIELD(material2,"Material2")
	XT_FIELD(friction,"Friction")
	XT_FIELD(restitution_coefficient,"RestitutionCoefficient")
XT_END_TYPE

XT_TYPE(E_AmaxCharacterDrivingStyle, "AmaxCharacterDrivingStyle")
	XT_OPTION(E_AmaxCharacterDrivingStyle_Neutral, "Neutral")
	XT_OPTION(E_AmaxCharacterDrivingStyle_Relaxed1, "Relaxed1")
	XT_OPTION(E_AmaxCharacterDrivingStyle_Relaxed2, "Relaxed2")
	XT_OPTION(E_AmaxCharacterDrivingStyle_Aggressive1, "Aggressive1")
	XT_OPTION(E_AmaxCharacterDrivingStyle_Aggressive2, "Aggressive2")
XT_END_TYPE

XT_TYPE(AmaxBaseCharacter,"AmaxBaseCharacter")
	XT_FIELD(model,"Model")
	XT_FIELD(skeleton,"Skeleton")
	XT_FIELD(drivingStyle, "DrivingStyle")
	XT_FIELD(seatbeltModel, "SeatbeltModel")
XT_END_TYPE



XT_TYPE(AMAX_SCORING_EVENT, "ScoringEvents")
	XT_OPTION(AMAX_SCORING_EVENT_SHUNT, "ScoringEvent_Shunt")
	XT_OPTION(AMAX_SCORING_EVENT_LEVEL2SHUNT, "ScoringEvent_Level2Shunt")
			  
	XT_OPTION(AMAX_SCORING_EVENT_SHOCK, "ScoringEvent_Shock")
	XT_OPTION(AMAX_SCORING_EVENT_LEVEL2SHOCK, "ScoringEvent_Level2Shock")
			  
	XT_OPTION(AMAX_SCORING_EVENT_BARGE, "ScoringEvent_Barge")
	XT_OPTION(AMAX_SCORING_EVENT_LEVEL2BARGE, "ScoringEvent_Level2Barge")
			  
	XT_OPTION(AMAX_SCORING_EVENT_NITROUS_OVERTAKE, "ScoringEvent_NitrousOvertake")
	XT_OPTION(AMAX_SCORING_EVENT_NITROUS_DRAFT_BOOST, "ScoringEvent_NitrousDraftBoost")
	XT_OPTION(AMAX_SCORING_EVENT_NITROUS_JUMP, "ScoringEvent_NitrousJump")
	XT_OPTION(AMAX_SCORING_EVENT_NITROUS_AIR, "ScoringEvent_NitrousAir")
			  
	XT_OPTION(AMAX_SCORING_EVENT_BOMB, "ScoringEvent_Bomb")
	XT_OPTION(AMAX_SCORING_EVENT_LEVEL2BOMB, "ScoringEvent_Level2Bomb")
			  
	XT_OPTION(AMAX_SCORING_EVENT_CLEAN_RACER, "ScoringEvent_CleanRacer")
	XT_OPTION(AMAX_SCORING_EVENT_HIT_A_CAR, "ScoringEvent_HitACar")
			  
	XT_OPTION(AMAX_SCORING_EVENT_REPAIR, "ScoringEvent_Repair")
	XT_OPTION(AMAX_SCORING_EVENT_CLOSEREPAIR, "ScoringEvent_CloseRepair")
			  
	XT_OPTION(AMAX_SCORING_EVENT_SHIELD, "ScoringEvent_Shield")
	XT_OPTION(AMAX_SCORING_EVENT_SHIELD_PENETRATED, "ScoringEvent_Shield_Penetrated")
	XT_OPTION(AMAX_SCORING_EVENT_ENEMY_SHIELD_PENETRATED, "ScoringEvent_Enemy_Shield_Penetrated")
	XT_OPTION(AMAX_SCORING_EVENT_QUICKSHIELD, "ScoringEvent_QuickShield")
	XT_OPTION(AMAX_SCORING_EVENT_SUPERQUICKSHIELD, "ScoringEvent_SuperQuickShield")
			  
	XT_OPTION(AMAX_SCORING_EVENT_ENEMY_SHIELDED, "ScoringEvent_EnemyShielded")
			  
	XT_OPTION(AMAX_SCORING_EVENT_RECOVERY, "ScoringEvent_Recovery")
			  
	XT_OPTION(AMAX_SCORING_EVENT_FIRST_PLACE_KING, "ScoringEvent_FirstPlaceKing")
	XT_OPTION(AMAX_SCORING_EVENT_SECOND_PLACE, "ScoringEvent_SecondPlace")
	XT_OPTION(AMAX_SCORING_EVENT_THIRD_PLACE, "ScoringEvent_ThirdPlace")

	XT_OPTION(AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR, "ScoringEvent_FirstPlaceDominator")
			  
	XT_OPTION(AMAX_SCORING_EVENT_TARGET_ACHIEVED, "ScoringEvent_TargetAchieved")
			  
	XT_OPTION(AMAX_SCORING_EVENT_FLIPPED_A_CAR, "ScoringEvent_FlippedACar")
	XT_OPTION(AMAX_SCORING_EVENT_CRASHED_OUT_A_CAR, "ScoringEvent_CrashedOutACar")
	XT_OPTION(AMAX_SCORING_EVENT_REVENGE_ATTACK, "ScoringEvent_RevengeAttack")
	XT_OPTION(AMAX_SCORING_EVENT_WRECKED_A_CAR, "ScoringEvent_WreckedACar")
	XT_OPTION(AMAX_SCORING_EVENT_DRIFTING, "ScoringEvent_Drifting")
	XT_OPTION(AMAX_SCORING_EVENT_AIR, "ScoringEvent_Air")
	XT_OPTION(AMAX_SCORING_EVENT_UNDER, "ScoringEvent_Under")
	XT_OPTION(AMAX_SCORING_EVENT_DROWN, "ScoringEvent_Drown")
	XT_OPTION(AMAX_SCORING_EVENT_CLIFF_DIVER, "ScoringEvent_CliffDiver")
	XT_OPTION(AMAX_SCORING_EVENT_DECOY, "ScoringEvent_Decoy")
			  
	XT_OPTION(AMAX_SCORING_EVENT_CHAIN_COMPLETE, "ScoringEvent_ChainComplete")
	XT_OPTION(AMAX_SCORING_EVENT_FAN_DEMAND_COMPLETE, "ScoringEvent_FanDemandComplete")
	XT_OPTION(AMAX_SCORING_EVENT_CHALLENGE_COMPLETE, "ScoringEvent_ChallengeComplete")	  

	XT_OPTION(AMAX_SCORING_EVENT_FAN_PICKED_UP, "ScoringEvent_FanPickedUp")
	XT_OPTION(AMAX_SCORING_EVENT_NITROUS_DRIFT, "ScoringEvent_NitrousDrift")
			  
	XT_OPTION(AMAX_SCORING_EVENT_ALL_FANS_GAINED, "ScoringEvent_AllFansGained")

	XT_OPTION(AMAX_SCORING_EVENT_PICKUP_SHOCKED_OUT, "ScoringEvent_Pickup_Shocked_Out")
	XT_OPTION(AMAX_SCORING_EVENT_NO_PICKUP_SHOCKED, "ScoringEvent_No_Pickup_Shocked")

	XT_OPTION(AMAX_SCORING_EVENT_DOUBLE_BARGE, "ScoringEvent_DoubleBarge")
	XT_OPTION(AMAX_SCORING_EVENT_SHOCKED_PICKUP_HIT, "ScoringEvent_ShockedOutPickupHit")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_NITROUS, "ScoringEvent_SkillShotNitrous")
	XT_OPTION(AMAX_SCORING_EVENT_CAMERA_LOOK, "ScoringEvent_CameraLook")
	XT_OPTION(AMAX_SCORING_EVENT_THE_CAT, "ScoringEvent_TheCat")
	XT_OPTION(AMAX_SCORING_EVENT_VENDETTA, "ScoringEvent_Vendetta")

	XT_OPTION(AMAX_SCORING_EVENT_CHECKPOINT_HIT, "ScoringEvent_CheckpointHit")

	XT_OPTION(AMAX_SCORING_EVENT_SPEED1, "ScoringEvent_Speed1")
	XT_OPTION(AMAX_SCORING_EVENT_SPEED2, "ScoringEvent_Speed2")
	XT_OPTION(AMAX_SCORING_EVENT_SPEED3, "ScoringEvent_Speed3")

	XT_OPTION(AMAX_SCORING_EVENT_DRIFTDOUBLEPICKUP, "ScoringEvent_DriftDoublePickup")
	XT_OPTION(AMAX_SCORING_EVENT_360, "ScoringEvent_360")
	XT_OPTION(AMAX_SCORING_EVENT_SHUNT_OVERTAKE, "ScoringEvent_ShuntOvertake")
	XT_OPTION(AMAX_SCORING_EVENT_SHOCK_OVERTAKE, "ScoringEvent_ShockOvertake")
	XT_OPTION(AMAX_SCORING_EVENT_BATTERING_RAM, "ScoringEvent_BatteringRam")
	XT_OPTION(AMAX_SCORING_EVENT_THE_OLD_ONE_TWO, "ScoringEvent_TheOldOneTwo")

	XT_OPTION(AMAX_SCORING_EVENT_MEGA_JUMP, "ScoringEvent_MegaJump")

	XT_OPTION(AMAX_SCORING_EVENT_DRIFT,			"ScoringEvent_Drift")
	XT_OPTION(AMAX_SCORING_EVENT_MEGA_DRIFT,	"ScoringEvent_MegaDrift")
	XT_OPTION(AMAX_SCORING_EVENT_PERSECUTER,	"ScoringEvent_Persecuter")
	XT_OPTION(AMAX_SCORING_EVENT_TERRORIZER,	"ScoringEvent_Terrorizer")

	XT_OPTION(AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR2, "ScoringEvent_FirstPlaceDominator2")
	XT_OPTION(AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR3, "ScoringEvent_FirstPlaceDominator3")
	XT_OPTION(AMAX_SCORING_EVENT_FIRST_PLACE_DOMINATOR4, "ScoringEvent_FirstPlaceDominator4")

	XT_OPTION(AMAX_SCORING_EVENT_SURVIVED_ELIMINATION, "ScoringEvent_SurvivedElimination")

	XT_OPTION(AMAX_SCORING_EVENT_IMPACT_HEAVY, "ScoringEvent_ImpactHeavy")
	XT_OPTION(AMAX_SCORING_EVENT_IMPACT_LIGHT, "ScoringEvent_ImpactLight")

	XT_OPTION(AMAX_SCORING_EVENT_EVADED_SHOCK, "ScoringEvent_EvadedShock")
	XT_OPTION(AMAX_SCORING_EVENT_EVADED_SHUNT, "ScoringEvent_EvadedShunt")
				
	XT_OPTION(AMAX_SCORING_EVENT_THREE_IN_A_ROW, "ScoringEvent_ThreeInARow")
	XT_OPTION(AMAX_SCORING_EVENT_FIVE_IN_A_ROW, "ScoringEvent_FiveInARow")
	XT_OPTION(AMAX_SCORING_EVENT_TEN_IN_A_ROW, "ScoringEvent_TenInARow")
				
	XT_OPTION(AMAX_SCORING_EVENT_BATTLING, "ScoringEvent_Battling")
				
	XT_OPTION(AMAX_SCORING_EVENT_SUPER_DRIFT, "ScoringEvent_SuperDrift")
				
	XT_OPTION(AMAX_SCORING_EVENT_NEAR_MISSED_CAR, "ScoringEvent_NearMissedCar")
	XT_OPTION(AMAX_SCORING_EVENT_NEAR_MISSED_ENVIRONMENT, "ScoringEvent_NearMissedEnvironment")
				
	XT_OPTION(AMAX_SCORING_EVENT_NO_REPAIR_RACE, "ScoringEvent_NoRepairOrWreckForWholeRace")
	XT_OPTION(AMAX_SCORING_EVENT_NO_REPAIR_LAP, "ScoringEvent_NoRepairOrWreckForWholeLap")
				
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_ALT_FIRE, "ScoringEvent_SkillShotAltFire")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_LONG, "ScoringEvent_SkillShotDistanceLong")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_MEDIUM, "ScoringEvent_SkillShotDistanceMedium")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_DISTANCE_SHORT, "ScoringEvent_SkillShotDistanceShort")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_ENEMY_IN_AIR, "ScoringEvent_SkillShotEnemyInAir")
	XT_OPTION(AMAX_SCORING_EVENT_SKILL_SHOT_HIT_ANOTHER_POWERUP, "ScoringEvent_SkillShotHitAnotherPowerup")
				
	XT_OPTION(AMAX_SCORING_EVENT_TIMES_THREE_KILLING_SPREE, "ScoringEvent_TimesThreeSkillingSpree")
	XT_OPTION(AMAX_SCORING_EVENT_TIMES_FIVE_KILLING_SPREE, "ScoringEvent_TimesFiveSkillingSpree")
	XT_OPTION(AMAX_SCORING_EVENT_TIMES_TEN_KILLING_SPREE, "ScoringEvent_TimesTenSkillingSpree")
	XT_OPTION(AMAX_SCORING_EVENT_TIMES_TWENTY_KILLING_SPREE, "ScoringEvent_TimesTwentySkillingSpree")

	XT_OPTION(AMAX_SCORING_EVENT_BLINDFIRE, "ScoringEvent_Blindfire")

XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxScoringEventType, mScoringEventTypes)
XT_TYPE(AmaxScoringEventType,"ScoringEventType")
	XT_CALLBACK((GameDataHandler<AmaxScoringEventType,CallbackAmaxScoringEventType>))
	XT_BASE(AmaxGameDataObj)
	
	XT_FIELD(type, "Type")

	XT_FIELD(min_score, "MinScore")
	XT_FIELD(max_score, "MaxScore")

	XT_FIELD(append_name, "AppendName")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxScoringConfig, mScoringConfig)
XT_TYPE(AmaxScoringConfig,"ScoringConfig")
	XT_CALLBACK((GameDataHandler<AmaxScoringConfig,CallbackAmaxScoringConfig>))
	XT_BASE(AmaxGameDataObj)
	
	XT_FIELD(message_delay_timer, "MessageDelayTimer")
				
	XT_FIELD(combo_time, "ComboTime")
				
	XT_FIELD(drift_angle_min, "DriftAngleMin")
	XT_FIELD(drift_angle_max, "DriftAngleMax")
				
	XT_FIELD(drift_slip_min, "DriftSlipMin")
	XT_FIELD(drift_slip_max, "DriftSlipMax")
				
	XT_FIELD(using_nitrous_timer, "UsingNitrousTimer")
	XT_FIELD(time_in_air_for_it_to_count, "TimeInAirForItToCount")
				
	XT_FIELD(late_repair_level, "LateRepairLevel")
				
	XT_FIELD(revenge_timer, "RevengeTimer")
	XT_FIELD(hit_vehicle_with_perk_timer, "HitVehicleWithPerkTimer")
				
	XT_FIELD(shield_time_quick, "ShieldTimeQuick")
	XT_FIELD(shield_time_superquick, "ShieldTimeSuperQuick")
				
	XT_FIELD(wrecked_timer, "WreckedTimer")
	XT_FIELD(crashed_out_timer, "CrashedOutTimer")
	XT_FIELD(out_of_bounds_timer, "OutOfBoundsTimer")
				
	XT_FIELD(hit_a_car_damage, "HitACarDamage")

	XT_FIELD(flipped_out_timer, "FlippedOutTimer")
	XT_FIELD(minimum_jump_distance, "MinimumJumpDistance")
	XT_FIELD(minimum_cat_distance, "MinimumCatDistance")

	XT_FIELD(combo_end_multiplier_graph, "MultiplierGraph")
	XT_FIELD(max_achieveble_chain, "MaxAchievableChain")

	XT_FIELD(fan_demand_time, "FansLostPerSecond")
	XT_FIELD(fan_demand_grace_period, "FanDemandGracePeriod")

	XT_FIELD(speed_1_mph, "Speed1MPH")
	XT_FIELD(speed_2_mph, "Speed2MPH")
	XT_FIELD(speed_3_mph, "Speed3MPH")

	XT_FIELD(min_360_angular_speed, "Min360AngularSpeed")
	XT_FIELD(max_360_angular_speed, "Max360AngularSpeed")
	
	XT_FIELD(cooldown360,			"Cooldown360")

	XT_FIELD(vendetta_cooldown,				"VendettaCooldown")
	XT_FIELD(drift_distance_to_score,		"DriftDistanceToScore")
	XT_FIELD(super_drift_distance_to_score,	"SuperDriftDistanceToScore")
	XT_FIELD(mega_drift_distance_to_score,	"MegaDriftDistanceToScore")

	XT_FIELD(first_place_dom_1, "FirstPlace1Time")
	XT_FIELD(first_place_dom_2, "FirstPlace2Time")
	XT_FIELD(first_place_dom_3, "FirstPlace3Time")
	XT_FIELD(first_place_dom_4, "FirstPlace4Time")

	XT_FIELD(long_distance,		"LongDistance")
	XT_FIELD(medium_distance,	"MediumDistance")
	XT_FIELD(short_distance,	"ShortDistance")

	XT_FIELD(sideswipe_impact,	"SideSwipeImpact")

	XT_FIELD(mMinSpeedMPH,			"MinSpeedMPH")
	XT_FIELD(mMinClosingSpeedMPH,	"MinClosingSpeedMPH")
	XT_FIELD(mMinDistanceMetres,	"MinDistanceMetres")
	XT_FIELD(mRegisterNearMissDelay,"RegisterNearMissDelay")
	XT_FIELD(mMinTimeInProximity,	"MinTimeInProximity")
	XT_FIELD(mMaxTimeInProximity,	"MaxTimeInProximity")
	XT_FIELD(mMaxTrackingTime,		"MaxTrackingTime")
	XT_FIELD(mMinMass,				"MinMass")

	//new scoring
	XT_FIELD(mBargeBase,					"BargeBase")
	XT_FIELD(mBargeDrift,					"BargeDrift")
	XT_FIELD(mBargeNitrous,					"BargeNitrous")
	XT_FIELD(mBargeAir,						"BargeAir")
			   
	XT_FIELD(mMineBase,						"MineBase")
	XT_FIELD(mMineLongShot,					"MineLongShot")
	XT_FIELD(mMineAir,						"MineAir")
	XT_FIELD(mMineAltFire,					"MineAltFire")
			   
	XT_FIELD(mShockBase,					"ShockBase")
			   
	XT_FIELD(mShuntBase,					"ShuntBase")
	XT_FIELD(mShuntAltFire,					"ShuntAltFire")
			   
	XT_FIELD(mBoltBase,						"BoltBase")
	XT_FIELD(mBoltLongShot,					"BoltLongShot")
	XT_FIELD(mBoltDrift,					"BoltDrift")
	XT_FIELD(mBoltAir,						"BoltAir")
	XT_FIELD(mBoltAltFire,					"BoltAltFire")
			   
	XT_FIELD(mRammingBase,					"RammingBase")
	XT_FIELD(mRammingNitrous,				"RammingNitrous")
	XT_FIELD(mRammingAir,					"RammingAir")
			   
	XT_FIELD(mNitrousDrift,					"NitrousDrift")
	XT_FIELD(mNitrousAir,					"NitrousAir")
	XT_FIELD(mNitrousAltFire,				"NitrousAltFire")
			   
	XT_FIELD(mShieldBase,					"ShieldBase")
	XT_FIELD(mShieldCloseCall,				"ShieldCloseCall")
			   
	XT_FIELD(mRepairBase,					"RepairBase")
	XT_FIELD(mRepairCloseCall,				"RepairCloseCall")
			   
	XT_FIELD(mDriftLevel1,					"DriftLevel1")
	XT_FIELD(mDriftLevel2,					"DriftLevel2")
	XT_FIELD(mDriftLevel3,					"DriftLevel3")
			   
	XT_FIELD(mJumpLevel1,					"JumpLevel1")
	XT_FIELD(mJumpLevel2,					"JumpLevel2")
	XT_FIELD(mJumpNitrous,					"JumpNitrous")
			   
	XT_FIELD(mOvertake,						"Overtake")
	XT_FIELD(mOvertakeNitrous,				"OvertakeNitrous")
	XT_FIELD(mOvertakeNitrousGood,			"OvertakeNitrousGood")
	XT_FIELD(mOvertakeNitrousGreat,			"OvertakeNitrousGreat")
	XT_FIELD(mOvertakeNitrousMega,			"OvertakeNitrousMega")
			   
	XT_FIELD(mPositionThird,				"PositionThird")
	XT_FIELD(mPositionSecond,				"PositionSecond")
	XT_FIELD(mPositionFirst,				"PositionFirst")
	XT_FIELD(mPositionFirstLevel1,			"PositionFirstLevel1")
	XT_FIELD(mPositionFirstLevel2,			"PositionFirstLevel2")
			   
	XT_FIELD(mEvasionShunt,					"EvasionShunt")
	XT_FIELD(mEvasionShock,					"EvasionShock")
			   
	XT_FIELD(mSpeedLevel1, 					"SpeedLevel1")
	XT_FIELD(mSpeedLevel2, 					"SpeedLevel2")
	XT_FIELD(mSpeedLevel3, 					"SpeedLevel3")
	XT_FIELD(mSpeedLevel4, 					"SpeedLevel4")
			   
	XT_FIELD(mNearMiss,						"NearMiss")
	
	XT_FIELD(mHitAnotherPickup,				"HitAnotherPickup")
			   
	XT_FIELD(mCleanRacingNitrous,			"CleanRacingNitrous")
	XT_FIELD(mCleanRacingNoWreckRepairLap,	"CleanRacingNoWreckRepairLap")
	XT_FIELD(mCleanRacingLap,				"CleanRacingLap")

	XT_FIELD(mBonusKillingSpree,			"BonusKillingSpree")
	XT_FIELD(mBonusWrecked,					"BonusWrecked")
	XT_FIELD(mBonusBullying,				"BonusBullying")
	XT_FIELD(mBonusAccuracy,				"BonusAccuracy")
	XT_FIELD(mBonusRevenge,					"BonusRevenge")
	XT_FIELD(mBonusDrown,					"BonusDrown")
	XT_FIELD(mBonusPlummet,					"BonusPlummet")
	XT_FIELD(mBonusFanDemandsInARow,		"BonusFanDemandsInARow")
	XT_FIELD(mBonusFullyBolted,				"BonusFullyBolted")
	XT_FIELD(mBonusFanDemandsPerfect,		"BonusFanDemandsPerfect")
	XT_FIELD(mBonusFanDemandsHoarder,		"BonusFanDemandsHoarder")
	XT_FIELD(mBonusFanRunFirstTime,			"BonusFanRunFirstTime")
	
	XT_FIELD(mBargeLevel1, 					"BargeLevel1")
	XT_FIELD(mBargeLevel2, 					"BargeLevel2")
	XT_FIELD(mBargeLevel3, 					"BargeLevel3")
	XT_FIELD(mBargeLevel4, 					"BargeLevel4")
	XT_FIELD(mBargeLevel5, 					"BargeLevel5")

	XT_FIELD(mDestructionEasyWreck, 		"DestructionEasyWreck")
	XT_FIELD(mDestructionMediumWreck, 		"DestructionMediumWreck")
	XT_FIELD(mDestructionHardWreck, 		"DestructionHardWreck")
	XT_FIELD(mDestructionAllWreck, 			"DestructionAllWreck")
	XT_FIELD(mDestructionCloseCallWreck, 	"DestructionCloseCallWreck")

	XT_FIELD(mCheckpointCloseCall, 			"CheckpointCloseCall")
	XT_FIELD(mCheckpointStopwatchCloseCall, "CheckpointStopwatchCloseCall")
	XT_FIELD(mCheckpointAirBrakeCollect, 	"CheckpointAirBrakeCollect")
	XT_FIELD(mCheckpointCleanSection, 		"CheckpointCleanSection")
	XT_FIELD(mCheckpointGoldSection, 		"CheckpointGoldSection")
	XT_FIELD(mCheckpointSectionPassed, 		"CheckpointSectionPassed")

	XT_FIELD(mComboFansMultiplier, 			"ComboFansMultiplier")

	XT_FIELD(mComboMax, 					"ComboMax")

	XT_FIELD(mDecoyHitScore,				"DecoyHitScore")

	XT_FIELD(mLastGaspHitScore,				"LastGaspHitScore")

XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxPaintResprays, mPaintResprays)
XT_TYPE(AmaxPaintResprays,				"VEH_Resprays")
	XT_CALLBACK((GameDataHandler<AmaxPaintResprays,CallbackAmaxPaintResprays>))
	XT_FIELD(m_PaintJobs,				"resprayJob")
XT_END_TYPE

//========================  New Social Networking  ========================

XT_TYPE(AMAX_SOCIAL_NETWORKING_TYPE,							"SocialNetworkingTypes")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_TYPE_INCREMENTS,			"Increments")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_TYPE_ONE_OFF,				"OneOff")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_TYPE_SHARE_BUTTON,			"ShareButton")
XT_END_TYPE

XT_TYPE(AMAX_SOCIAL_NETWORKING_ITEM_TYPE,												"SocialNetworkingItemType")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR,							"PlayingBlur")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ACHIEVEMENT,								"Achievement")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_COMPLETED,							"EventCompleted")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_UNLOCKED,							"TierUnlocked")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_COMPLETE,							"TierComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_BOSS_BEATEN,								"BossBeaten")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_GAME_COMPLETE,							"GameComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_GOLDEN_FAN_DEMANDS,					"AllGoldenFanDemands")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_STARS,								"AllStars")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_CARS,								"AllCars")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_MODS,								"AllMods")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TOP_FAN_STATUS,							"TopFanStatus")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TOP_MULTIPLAYER_RANK,					"TopMultiplayerRank")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_ONE_ON_LEADERBOARD,				"NumberOneOnLeaderboard")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_COMPLETE,					"MultiplayerChallengeComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CAR_CHALLENGE_COMPLETE,				"MultiplayerCarChallengeComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEGEND_RANK,								"LegendRank")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_STATUS,								"FanStatus")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_COMPLETION_PERCENTAGE,					"CompletionPercentage")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_FANS,							"NumberOfFans")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_STARS,							"NumberOfStars")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_CARS,							"NumberOfCars")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_BOSSES_FINISHED,							"BossesFinished")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_PERCENT_COMPLETE,					"TierPercentComplete")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_STARS,								"EventStars")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_OWNER,								"EventOwner")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEADERBOARD_POSITION,					"LeaderboardPosition")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_ABOVE,		"RelativeLeaderboardPositionAbove")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_GOLDEN_FAN_DEMAND_COMPLETE,				"GoldenFanDemandComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_PAR_COMPLETE,						"FanParComplete")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_CAR,									"OwnCar")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_MOD,									"OwnMod")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_COMPLETE_PERCENTAGE,		"MPChallengeComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_PACK_COMPLETE_PERCENTAGE,	"MPChallengePackComplete")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CAR_CHALLENGE_COMPLETE_PERCENTAGE,	"MPCarChallengeComplete")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_BLUR_EVENT_RATING,					"MPBlurEventRating")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_WINS,							"MPTotalWins")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_POWER_UP_HIT_RATE,					"MPPowerUpHitRate")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_RACES,							"MPTotalRaces")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_PLAYTIME,						"MPTotalPlaytime")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_FANS,							"MPTotalFans")
																						 
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_FINISHING_POSITION,					"MPFinishingPosition")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RACE_AWARD,							"MPRaceAward")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_PLAYERS_RANK,							"MPPlayersRank")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_ABOVE,		"MPRelativePlayerPositionAbove")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_BELOW,		"MPRelativePlayerPositionBelow")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_BELOW,		"RelativeLeaderboardPositionBelow")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UPLOADED_PHOTO,							"UploadedPhoto")
	
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_TIER,							"UnlockedTier")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_EVENT,							"UnlockedEvent")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_RANK,							"UnlockedRank")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_VEHICLE,						"UnlockedVehicle")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_BOSS_VEHICLE,					"UnlockedBossVehicle")
	
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND,							"FriendDemand")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_ACCEPTED,				"TaleOfTheTapeAccepted")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_WON,					"TaleOfTheTapeWon")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_KOS,					"TaleOfTheTapeKOs")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_MOST_ATTEMPTS,			"TaleOfTheTapeMostAttempts")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_AVG_ATTEMPTS,			"TaleOfTheTapeAverageAttempts")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_STANDINGS,							"SSStandings")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_WINS,									"SSWins")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_POINTS,								"SSPoints")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_GAMES,							"SSTotalGames")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_PLAYTIME,						"SSTotalPlaytime")

	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_REDEMAND,							"FriendReDemand")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_PASSED,					"FriendDemandPassed")
	XT_OPTION(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_FAILED,					"FriendDemandFailed")

XT_END_TYPE

XT_TYPE(AmaxTweetItem,				"TweetItem")
	XT_FIELD(tweet_text,			"Tweets")
	XT_FIELD(tweet_text_singular,	"SingularTweets")

	XT_FIELD(suffix,				"Suffix")
XT_END_TYPE

XT_TYPE(AmaxFacebookStoryItem,	"FacebookStoryItem")
	XT_FIELD(message,			"Message")

	XT_FIELD(name,				"Name")
	XT_FIELD(name_singular,		"SingularName")
	
	XT_FIELD(caption,			"Caption")
	XT_FIELD(description,		"Description")
	XT_FIELD(image,				"Image")
	XT_FIELD(image_href,		"ImageHref")
	XT_FIELD(suffix,			"Suffix")
XT_END_TYPE

XT_TYPE(AmaxBlurbItem,		"BlurbItem")
	XT_FIELD(blurb_text,	"Blurb")
	XT_FIELD(suffix,		"Suffix")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxSocialNetworkingItem, mSocialStories)
XT_TYPE(AmaxSocialNetworkingItem,"SocialNetworkingItem")
	
	XT_CALLBACK((GameDataHandler<AmaxSocialNetworkingItem,CallbackAmaxSocialNetworkingItem>))
	XT_BASE(AmaxGameDataObj)

	XT_FIELD(given_for,			"GivenFor")
	XT_FIELD(type,				"Type")

	XT_FIELD(blurb,				"Blurb")
	XT_FIELD(tweet,				"Tweet")
	XT_FIELD(facebook_story,	"FacebookStory")
	
	XT_FIELD(parameters,		"Parameters")
XT_END_TYPE




XT_TYPE(AmaxDialogueAudio, "AmaxDialogueAudio")
	XT_OPTION(E_AMAXDIALOGUE_EVENT_LASTLAP,						"EventLastLap")
	XT_OPTION(E_AMAXDIALOGUE_EVENT_LASTSPLIT,					"EventLastSplit")
	XT_OPTION(E_AMAXDIALOGUE_EVENT_SECONDLAP,					"EventSecondLap")

	XT_OPTION(E_AMAXDIALOGUE_PLAYER_CLOSETODEATH,				"PlayerCloseToDeath")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_FATALLYDAMAGED,				"PlayerFatallyDamaged")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_COLLIDESWITHAI,				"PlayerCollidesWithAI")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_COLLIDESWITHENVIRONMENT,	"PlayerCollidesWithEnvironment")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_HITSAIWITHPERK,				"PlayerHitsAIWithPerk")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_OVERTAKESAI,				"PlayerOvertakesAI")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_SHIELDSPERK,				"PlayerShieldsPerk")
	XT_OPTION(E_AMAXDIALOGUE_PLAYER_CAUSEDCRASHWITHPERK,		"PlayerCausesCrashWithPerk")


	XT_OPTION(E_AMAXDIALOGUE_AI_FATALLYDAMAGED,					"AIFatallyDamaged")
	XT_OPTION(E_AMAXDIALOGUE_AI_COLLIDESWITHPLAYER,				"AICollidesWithPlayer")
	XT_OPTION(E_AMAXDIALOGUE_AI_HITSPLAYERWITHPERK,				"AIHitsPlayerWithPerk")
	XT_OPTION(E_AMAXDIALOGUE_AI_OVERTAKESPLAYER,				"AIOvertakesPlayer")
	XT_OPTION(E_AMAXDIALOGUE_AI_SHIELDSPERK,					"AIShieldsPerk")
	XT_OPTION(E_AMAXDIALOGUE_AI_CAUSEDCRASHWITHPERK,			"AICausesCrashWithPerk")
XT_END_TYPE

//============================  New Dialogue  =============================

AMAX_GAMEDATA_CALLBACK(AmaxInGameDialogue, mInGameDialogue)
XT_TYPE(AmaxInGameDialogue,"Dialogue")
	XT_CALLBACK((GameDataHandler<AmaxInGameDialogue,CallbackAmaxInGameDialogue>))
	XT_FIELD(items, "Items")
	XT_FIELD(ignorePack, "IgnorePack")
XT_END_TYPE

XT_TYPE(AmaxInGameDialogueItem,"DialogueItem")
	XT_BASE(AmaxInGameDialogueItemBase)
XT_END_TYPE

XT_TYPE(AmaxInGameDialogueItemBase,"DialogueItemBase")
	XT_FIELD(triggerHash,			"TriggerHash")
	XT_FIELD(characterHash,			"CharacterHash")
	XT_FIELD(eventHash,				"EventHash")
	XT_FIELD(locationHash,			"LocationHash")
	XT_FIELD(conversationId,		"ConversationHash")
	XT_FIELD(weighting,				"Weighting")
	XT_FIELD(charactersInvolved,	"CharacterHashesInConversation")
	XT_FIELD(locId,					"TextId")
	XT_FIELD(uid,					"uid")
	XT_FIELD(oneShot,				"OneShot")
	XT_FIELD(splitGroup,			"SplitGroup")
	XT_FIELD(splitIndex,			"SplitIndex")
#ifndef CFG_GOLD
	XT_FIELD(eventName,				"Event")
	XT_FIELD(locationName,			"Location")
	XT_FIELD(triggerName,			"Trigger")
	XT_FIELD(wavFile,				"Wav")
#endif //CFG_GOLD
XT_END_TYPE

//==========================  Dialogue Triggers  ===========================
XT_TYPE(AmaxDialogueEnvironmentImpactProbabilities,"AmaxDialogueTriggerImpactLevelEnvironment")
	XT_FIELD(impactLevel, "ImpactLevel")
	XT_FIELD(probability, "Probability")
XT_END_TYPE

XT_TYPE(AmaxDialogueCharacterImpactProbabilities,"AmaxDialogueTriggerImpactLevelCharacter")
	XT_FIELD(impactLevel, "ImpactLevel")
	XT_FIELD(probabilityPlayer, "PlayerHitDriverProbability")
	XT_FIELD(probabilityAI, "DriverHitPlayerProbability")
XT_END_TYPE

XT_TYPE(AmaxDialogueTriggerImpact,"AmaxDialogueTriggerImpact")
	XT_FIELD(environmentProbabilities, "EnvironmentImpactProbabilities")
	XT_FIELD(characterProbabilities, "CharacterImpactProbabilities")
XT_END_TYPE

XT_TYPE(AmaxDialogueTriggerOvertake,"AmaxDialogueTriggerOvertake")
	XT_FIELD(probabilityPlayer, "PlayerOvertakeDriverProbability")
	XT_FIELD(probabilityAI, "DriverOvertakePlayerProbability")
	XT_FIELD(probabilityWithNitroPlayer, "PlayerOvertakeDriverWithNitroProbability")
	XT_FIELD(probabilityWithNitroAI, "DriverOvertakePlayerWithNitroProbability")
XT_END_TYPE

XT_TYPE(AmaxDialogueTriggerHitByPerk,"AmaxDialogueTriggerHitByPerk")
	XT_FIELD(probabilityPlayer, "PlayerHitDriverProbability")
	XT_FIELD(probabilityAI, "DriverHitPlayerProbability")
	XT_FIELD(probabilityPlayerSpecial, "PlayerHitDriverSpecialProbability")
	XT_FIELD(probabilityAISpecial, "DriverHitPlayerSpecialProbability")
XT_END_TYPE

XT_TYPE(AmaxDialogueTriggerShield,"AmaxDialogueTriggerShielded")
	XT_FIELD(probabilityPlayer, "PlayerProbability")
	XT_FIELD(probabilityAI, "DriverProbability")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxDialogueTriggerControls, mDialogueTriggerControls)
XT_TYPE(AmaxDialogueTriggerControls,"AmaxDialogueTriggerControls")
	XT_CALLBACK((GameDataHandler<AmaxDialogueTriggerControls,CallbackAmaxDialogueTriggerControls>))
	XT_FIELD(nextLineProbability, "NextLineProbability")
	XT_FIELD(impactTriggerDialogue, "ImpactControls")
	XT_FIELD(overtakeTriggerDialogue, "OvertakeControls")
	XT_FIELD(hitByPerkTriggerDialogue, "HitWithPerkControls")
	XT_FIELD(shieldTriggerDialogue, "ShieldControls")
XT_END_TYPE

// ========================= DLC ==========================
AMAX_GAMEDATA_CALLBACK(AmaxDLCPack, mDLCPacks)
XT_TYPE(AmaxDLCPack, "DLCPack")
	XT_CALLBACK((GameDataHandler<AmaxDLCPack,CallbackAmaxDLCPack>))

	XT_FIELD(mPackID, "PackId")
	XT_FIELD(mLoadableItems, "LoadableContent")
XT_END_TYPE

AMAX_GAMEDATA_CALLBACK(AmaxCheatBase, mCheatPacks)
XT_TYPE(AmaxCheatBase, "DLCCheatBase")
	XT_CALLBACK((GameDataHandler<AmaxCheatBase,CallbackAmaxCheatBase>))
	XT_BASE(AmaxGameDataObj)
XT_END_TYPE

XT_TYPE(AmaxCheatUnlockId, "DLCCheatUnlockId")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mId, "UnlockID")
XT_END_TYPE

XT_TYPE(AmaxCheatIncome, "DLCCheatIncome")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mMultiplier, "Multiplier")
	XT_FIELD(mNumRaces,	 "NumRaces")
	XT_FIELD(mSingleplayer, "Singleplayer")
	XT_FIELD(mMultiplayer, "Multiplayer")
XT_END_TYPE

XT_TYPE(AmaxCheatFans, "DLCCheatFans")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mMultiplier, "Multiplier")
	XT_FIELD(mNumRaces,	 "NumRaces")
	XT_FIELD(mRaceTypes, "MultiplayerRaceTypes")
XT_END_TYPE

XT_TYPE(AmaxCheatMultiplayerEvent, "DLCCheatMultiplayerEvent")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mEvent, "Event")
XT_END_TYPE

XT_TYPE(AmaxCheatVehicle, "DLCCheatVehicle")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mRaceBookVehicle,	"RaceBookVehicle")
	XT_FIELD(mAmaxVehicle,		"Vehicle")
XT_END_TYPE

XT_TYPE(AmaxCheatChromeVehicle, "DLCCheatChromeVehicle")
	XT_BASE(AmaxCheatBase)
	XT_FIELD(mAmaxVehicle,				"Vehicle")
	XT_FIELD(mAmaxVehicleUnlock,		"VehicleUnlock")
XT_END_TYPE


// ========================= New Scoring ==========================
XT_TYPE(E_BLUR_SCORE_MOVE_BASE, "NewScoringMoveBase")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_NOTHING, "NewScoringMoveBase_Nothing")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_SHUNT, "NewScoringMoveBase_Shunt")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_SHOCK, "NewScoringMoveBase_Shock")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_BARGE, "NewScoringMoveBase_Barge")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_MINE, "NewScoringMoveBase_Mine")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_BOLT, "NewScoringMoveBase_Bolt")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_POSITION, "NewScoringMoveBase_Position")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_RAMMING, "NewScoringMoveBase_Ramming")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_SHIELD, "NewScoringMoveBase_Shield")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_REPAIR, "NewScoringMoveBase_Repair")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_NITROUS, "NewScoringMoveBase_Nitrous")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_DRIFTING, "NewScoringMoveBase_Drifting")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_JUMPING, "NewScoringMoveBase_Jumping")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_OVERTAKE, "NewScoringMoveBase_Overtake")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_EVASION, "NewScoringMoveBase_Evasion")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_HIT_ANOTHER_PICKUP, "NewScoringMoveBase_HitAnotherPickup")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_SPEED, "NewScoringMoveBase_Speed")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_NEAR_MISS, "NewScoringMoveBase_NearMiss")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_CLEAN_RACING, "NewScoringMoveBase_CleanRacing")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_STOPWATCH, "NewScoringMoveBase_Stopwatch")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_BANKING_TIME, "NewScoringMoveBase_BankingTime")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_FAN_DEMAND, "NewScoringMoveBase_FanDemand")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_MEDALS, "NewScoringMoveBase_Medals")
	XT_OPTION(E_BLUR_SCORE_MOVE_BASE_DESTRUCTION, "NewScoringMoveBase_Destruction")
XT_END_TYPE

XT_TYPE(E_BLUR_SCORE_MOVE_SKILL, "NewScoringMoveSkill")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LONG_SHOT,"NewScoringMoveSkill_LongShot")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_OVERTAKE, "NewScoringMoveSkill_Overtake")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_CLOSE_CALL, "NewScoringMoveSkill_CloseCall")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_DRIFT, "NewScoringMoveSkill_Drift")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_NITROUS, "NewScoringMoveSkill_Nitrous")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_AIR, "NewScoringMoveSkill_Air")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE, "NewScoringMoveSkill_AltFire")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LEVEL_1, "NewScoringMoveSkill_Level1")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LEVEL_2, "NewScoringMoveSkill_Level2")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LEVEL_3, "NewScoringMoveSkill_Level3")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LEVEL_4, "NewScoringMoveSkill_Level4")
	XT_OPTION(E_BLUR_SCORE_MOVE_SKILL_LEVEL_5, "NewScoringMoveSkill_Level5")
XT_END_TYPE

XT_TYPE(E_BLUR_SCORE_MOVE_BONUS, "NewScoringMoveBonus")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_NONE, "NewScoringMoveBonus_None")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_KILLING_SPREE, "NewScoringMoveBonus_KillingSpree")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_WRECKED, "NewScoringMoveBonus_Wrecked")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_BULLYING, "NewScoringMoveBonus_Bullying")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_ACCURACY, "NewScoringMoveBonus_Accuracy")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_REVENGE, "NewScoringMoveBonus_Revenge")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_DROWN, "NewScoringMoveBonus_Drown")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_CLIFF, "NewScoringMoveBonus_Cliff")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_FAN_DEMAND, "NewScoringMoveBonus_FanDemand")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_SURVIVE_BARRAGE, "NewScoringMoveBonus_SurviveBarrage")
			  
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_FULLY_BOLTED, "NewScoringMoveBonus_FullyBolted")
	XT_OPTION(E_BLUR_SCORE_MOVE_BONUS_COMBO, "NewScoringMoveBonus_Combo")
XT_END_TYPE

// --------------
// PICKUP SCRIPTS
// --------------

XT_TYPE(PickupFiringScriptNode,		"PickupTimeNode")
	XT_FIELD(mPickupType,			"PickupType")
	XT_FIELD(mReverseFire,			"ReverseFire")
	XT_FIELD(mTime,					"Time")
	XT_FIELD(mVehIdx,				"VehicleIndex")
	XT_FIELD(mTargetIdx,			"TargetIndex")
XT_END_TYPE

XT_TYPE(PickupFiringScript,			"PickupScript")
	XT_FIELD(mNodes,				"PickupFireTimes")
XT_END_TYPE

XT_TYPE(PickupFiringScriptManager,	"PickupScriptManager")
	XT_CALLBACK(PickupScriptHandler)
	XT_FIELD(mScripts,				"ActiveScripts")
XT_END_TYPE

//========================  Debug Reference Table  ========================
//						Matches Schema\DynamicObjectDef.txt
AMAX_GAMEDATA_CALLBACK(DebugReferences, mDebugReferences)
XT_TYPE(DebugReferences,		"DebugReferences")
	XT_CALLBACK((GameDataHandler<DebugReferences,CallbackDebugReferences>))
	XT_FIELD(m_DropObjectType,		"ObjectTypeToDrop")
XT_END_TYPE

XT_LIBRARY(GAMEDATA)

	XT_REGISTER_TYPE(AmaxGameDataObj)
	XT_REGISTER_TYPE(ReflectionResolution)
	XT_REGISTER_TYPE(EnvironmentResolution)
	
	XT_REGISTER_TYPE(Game::E_AmaxMemType)
	XT_REGISTER_TYPE(Game::AmaxPlatformAllocation)
	XT_REGISTER_TYPE(Game::AmaxMemBudget)
	XT_REGISTER_TYPE(Game::AmaxMemBudgets)
	XT_REGISTER_TYPE(Game::AmaxRoutePerfStats)
	XT_REGISTER_TYPE(Game::AmaxCityPerfStats)
	XT_REGISTER_TYPE(AmaxCity)
	XT_REGISTER_TYPE(AmaxRoute)
	XT_REGISTER_TYPE(AmaxPreLoadList)
	XT_REGISTER_TYPE(C_LocData)

	XT_REGISTER_TYPE(AmaxDialogueAudio)

	XT_REGISTER_TYPE(AmaxDialogueTriggerOvertake)
	XT_REGISTER_TYPE(AmaxDialogueTriggerImpact)
	XT_REGISTER_TYPE(AmaxDialogueCharacterImpactProbabilities)
	XT_REGISTER_TYPE(AmaxDialogueEnvironmentImpactProbabilities)
	XT_REGISTER_TYPE(AmaxDialogueTriggerControls)
	XT_REGISTER_TYPE(AmaxDialogueTriggerHitByPerk)
	XT_REGISTER_TYPE(AmaxDialogueTriggerShield)

	XT_REGISTER_TYPE(AmaxInGameDialogue)
	XT_REGISTER_TYPE(AmaxInGameDialogueItem)
	XT_REGISTER_TYPE(AmaxInGameDialogueItemBase)

	XT_REGISTER_TYPE(AmaxPhysicsMaterial)
	XT_REGISTER_TYPE(AmaxFrictionSettings)

	XT_REGISTER_TYPE(E_AmaxCharacterDrivingStyle)
	XT_REGISTER_TYPE(AmaxBaseCharacter)

	XT_REGISTER_TYPE(TimeAttackBestTimeEntry)

	XT_REGISTER_TYPE(AmaxDLCBase)
	XT_REGISTER_TYPE(AmaxDLCPack)
	XT_REGISTER_TYPE(AmaxCheatBase)
	XT_REGISTER_TYPE(AmaxCheatUnlockId)
	XT_REGISTER_TYPE(AmaxCheatIncome)
	XT_REGISTER_TYPE(AmaxCheatFans)
	XT_REGISTER_TYPE(AmaxCheatMultiplayerEvent)
	XT_REGISTER_TYPE(AmaxCheatVehicle)
	XT_REGISTER_TYPE(AmaxCheatChromeVehicle)
		
	XT_REGISTER_TYPE(DebugReferences)

	XT_REGISTER_TYPE(TitleManagedStorageSettings)
	XT_REGISTER_TYPE(StartLineInfoSpecialFXList)
	XT_REGISTER_TYPE(StartLineInfoSpecialFX)


	XT_REGISTER_TYPE(AmaxScoringEventType)
	XT_REGISTER_TYPE(AmaxScoringConfig)
	XT_REGISTER_TYPE(AmaxPaintResprays)
	XT_REGISTER_TYPE(AMAX_SCORING_EVENT);

	XT_REGISTER_TYPE(AMAX_SOCIAL_NETWORKING_TYPE);
	XT_REGISTER_TYPE(AMAX_SOCIAL_NETWORKING_ITEM_TYPE);

	XT_REGISTER_TYPE(AmaxTweetItem);
	XT_REGISTER_TYPE(AmaxFacebookStoryItem);
	XT_REGISTER_TYPE(AmaxBlurbItem);
	XT_REGISTER_TYPE(AmaxSocialNetworkingItem);

	XT_REGISTER_TYPE(PickupProbability)
	XT_REGISTER_TYPE(PickupProbabilityList)

	XT_REGISTER_TYPE(E_BLUR_SCORE_MOVE_BASE);
	XT_REGISTER_TYPE(E_BLUR_SCORE_MOVE_SKILL);
	XT_REGISTER_TYPE(E_BLUR_SCORE_MOVE_BONUS);

	XT_REGISTER_TYPE(PickupFiringScriptNode);
	XT_REGISTER_TYPE(PickupFiringScript);
	XT_REGISTER_TYPE(PickupFiringScriptManager);

XT_END_LIBRARY

void C_GameData::Init()
{
	// load in the loc
	C_LocData::Instance().Init();

	//this needs to move somewhere nice
	mCities.AlphaSort();
	mRoutes.IndexSort();

	if(mPickupFiringScriptManager.Object())
	{
		for( int i = 0; i < mPickupFiringScriptManager->mScripts.count; i++ )
		{
			PickupFiringScript* pScript = mPickupFiringScriptManager->mScripts[i].Object();

			if(pScript)
			{
				pScript->mCurrentTime = 0.f;
				pScript->mCurrentIdx = -1;
			}
		}
	}

	//Debug Code to print out the Sorcery IDs for Agora
	//printf("Cities\n");
	//for(int a=0;a<mCities.Number();a++)
	//{
	//	printf("ID %x = %S\n",mCities[a]->ID,UI::C_TextStringID(mCities[a]->loc_string.id).String());
	//}

	//printf("Routes\n");
	//for(int a=0;a<mRoutes.Number();a++)
	//{
	//	printf("ID %x = %S (%S)\n",mRoutes[a]->ID,UI::C_TextStringID(mRoutes[a]->loc_string.id).String(),UI::C_TextStringID(mRoutes[a]->city->loc_string.id).String());
	//}
}

U32 C_GameData::GenerateAvailableDlcBits(void)
{
	U32 out = 0;
	
	// by default bit 0 means shipped content so we always have access to this
	out |= (1<<0);

	for (S32 i=0; i<mDLCPacks.Number(); i++)
		out |= (1<<mDLCPacks[i]->mPackID);

	return out;
}

