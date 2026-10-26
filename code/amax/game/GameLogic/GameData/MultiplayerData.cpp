
#include "stdafx.h"
#include "MultiplayerData.h"

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include "GameLogic/GameData/GameVehicleData.h"
#include <GameLogic/Structures/MultiplayerChallenge.h>
#include <GameLogic/Structures/MultiplayerAwards.h>
#include <GameLogic/Structures/SplitScreenEventType.h>
#include <GameLogic\Profile\GameProfile.h>
#include <GameLogic\Profile\GameProfileManager.h>

#include "UI/UiXtConfig.h"

C_MultiplayerData C_MultiplayerData::m_sInstance;

void MultiplayerConfigCallback( XtEventType event, XtRecord* record, MultiplayerConfig* object, XtCallbackContext* )
{
	switch( event )
	{
	case kXtEtConstruct :
		C_MultiplayerData::Instance().m_pConfig = object;
		break;

	case kXtEtDestruct :
		C_MultiplayerData::Instance().m_pConfig = NULL;
		break;
	}
}

void MultiplayerGroupCallback( XtEventType event, XtRecord* record, MultiplayerGroup* object, XtCallbackContext* )
{
	switch( event )
	{
		case kXtEtConstruct :
			object->ID = record->id;
			break;
	}
}

void MultiplayerEventCallback( XtEventType event, XtRecord* record, MultiplayerEvent* object, XtCallbackContext* )
{
	switch( event )
	{
		case kXtEtConstruct :
			object->ID = record->id;
			break;
	}
}


MultiplayerRace::MultiplayerRace()
:	type( UI::E_RaceType_Racing ), 
	min_players( 2 ),
	max_players( 20 ),
	laps( 1 ), 
	time( 0 ), 
	score_limit( 0 ), 
	classes( 0xff ),
	powerups( 0xff ),
	damage_mode( UI::E_DamageMode_Medium )
{
}

MultiplayerEvent::MultiplayerEvent()
:	ID( 0xffffffff ),
	fan_multiplier( 1.0f ),
	min_players( 2 ),
	max_players( 20 ),
	min_rank( 0 ),
	max_rank( 49 ),
	misc( 0xff ),
	aiDifficulty( E_MULTIPLAYER_AIDIFFICULTY_OFF )
{
}

XT_TYPE( MotorMashScoringValues, "MotorMashScoringValues")
	XT_FIELD( m_WreckScore,			"wreck")
	XT_FIELD( m_SuicideScore,		"suicide")
	XT_FIELD( m_LowDamageScore,		"lowDamage")
	XT_FIELD( m_HighDamageScore,	"highDamage")
	XT_FIELD( m_RivalDamageMultiplier,	"rivalDamageMultiplier")
XT_END_TYPE

XT_TYPE( MiscModSettings, "MiscModSettings")
	XT_FIELD( m_SafetyNetAbsorbEnabledBomb,			"safety_net_absorb_enabled_bomb")
	XT_FIELD( m_SafetyNetAbsorbEnabledShock,		"safety_net_absorb_enabled_shock")
	XT_FIELD( m_SafetyNetAbsorbEnabledBolt,			"safety_net_absorb_enabled_bolt")
	XT_FIELD( m_SafetyNetAbsorbEnabledShunt,		"safety_net_absorb_enabled_shunt")
	XT_FIELD( m_SafetyNetAbsorbEnabledBarge,		"safety_net_absorb_enabled_barge")
	XT_FIELD( m_BribeOnActivatePickups,				"bribe_on_activate_pickups")

	XT_FIELD( m_ScramblerEnabledBomb,				"scrambler_enabled_bomb")
	XT_FIELD( m_ScramblerEnabledShock,				"scrambler_enabled_shock")
	XT_FIELD( m_ScramblerEnabledBolt,				"scrambler_enabled_bolt")
	XT_FIELD( m_ScramblerEnabledShunt,				"scrambler_enabled_shunt")
	XT_FIELD( m_ScramblerEnabledBarge,				"scrambler_enabled_barge")
	XT_FIELD( m_ScramblerEnabledBombReverse,		"scrambler_enabled_bomb_reverse")
	XT_FIELD( m_ScramblerEnabledBoltReverse,		"scrambler_enabled_bolt_reverse")
	XT_FIELD( m_ScramblerEnabledShuntReverse,		"scrambler_enabled_shunt_reverse")
	XT_FIELD( m_ScramblerEnabledNitroRift,			"scrambler_enabled_nitro_rift")

	XT_FIELD( m_ScramblerNumBoltRequiredInChain,	"scrambler_num_bolt_required_in_chain")
	XT_FIELD( m_ScramblerModBoltChainTimerIncrement, "scrambler_mod_bolt_chain_timer_increment")
	XT_FIELD( m_ScramblerPowerupBayDisableTime,		"scrambler_mod_powerup_bay_disable_time")
	XT_FIELD( m_ScramblerBoltsStack,				"scrambler_bolts_stack")
	XT_FIELD( m_ScramblerBoltDisableTime,			"scrambler_bolt_disable_time")

	XT_FIELD( m_ShieldedBoostersBargeDamageAbsorbed,"shielded_boosters_barge_damage_absorbed")
	XT_FIELD( m_ShieldedBoostersBoltDamageAbsorbed,	"shielded_boosters_bolt_damage_absorbed")
	XT_FIELD( m_ShieldedBoostersBombDamageAbsorbed,	"shielded_boosters_bomb_damage_absorbed")
	XT_FIELD( m_ShieldedBoostersShockDamageAbsorbed,"shielded_boosters_shock_damage_absorbed")
	XT_FIELD( m_ShieldedBoostersShuntDamageAbsorbed,"shielded_boosters_shunt_damage_absorbed")

	XT_FIELD( m_FrontrunnerNitroOnRespawn,			"frontrunner_nitro_on_respawn")
	XT_FIELD( m_FrontrunnerNitroOnSplit,			"frontrunner_nitro_on_split")
	XT_FIELD( m_FrontrunnerNitroOnRaceStart,		"frontrunner_nitro_on_racestart")
	XT_FIELD( m_FrontrunnerNitroOnLapCompleted,		"frontrunner_nitro_on_lap_completed")
	XT_FIELD( m_FrontrunnerNitroOnFinalLap,			"frontrunner_nitro_on_final_lap")
	XT_FIELD( m_FrontrunnerMotormashTimer,			"frontrunner_motormash_timer")

	XT_FIELD( m_TitaniumArmourBlocksBarge,			"titanium_armour_blocks_barge")
	XT_FIELD( m_TitaniumArmourBlocksBolt,			"titanium_armour_blocks_bolt")
	XT_FIELD( m_TitaniumArmourBlocksBomb,			"titanium_armour_blocks_bomb")
	XT_FIELD( m_TitaniumArmourBlocksShock,			"titanium_armour_blocks_shock")
	XT_FIELD( m_TitaniumArmourBlocksShunt,			"titanium_armour_blocks_shunt")

	XT_FIELD (m_JumpTheGunWreckedSequenceReductionMultiplier,   "jump_the_gun_wrecked_sequence_reduction_multiplier")
	XT_FIELD (m_JumpTheGunRespawnSpeedWreck,		"jump_the_gun_respawn_speed_wreck")
	XT_FIELD( m_JumpTheGunRespawnSpeed,				"jump_the_gun_respawn_speed")

	XT_FIELD( m_IronFistBargeDamage,				"iron_fist_barge_damage")
	XT_FIELD( m_IronFistBoltDamage,					"iron_fist_bolt_damage")
	XT_FIELD( m_IronFistBombDamage,					"iron_fist_bomb_damage")
	XT_FIELD( m_IronFistShockDamage,				"iron_fist_shock_damage")
	XT_FIELD( m_IronFistShuntDamage,				"iron_fist_shunt_damage")
	XT_FIELD( m_IronFistNumBoltsRequired,			"iron_fist_num_bolts_required")


	XT_FIELD( m_DecoyDropbrakeTarget,				"decoy_drop_brake_target")
	XT_FIELD( m_DecoyDropsAllowedPerPlayer,			"decoy_drops_allowed_per_player")
	XT_FIELD( m_DecoyDropToleranceDistance,			"decoy_drop_tolerance_distance")

	XT_FIELD( m_ShieldingEfficiencyExtraHealth,		"shielding_efficiency_extra_health")

	XT_FIELD( m_DrifterLengths,						"drifter_lengths")

	XT_FIELD( m_laserSightMaxAngle,					"lasersight_max_angle")
	XT_FIELD( m_laserSightMinNormal,				"lasersight_min_normal")
XT_END_TYPE



XT_TYPE( PostRaceBonusData, "PostRaceBonusData")
	XT_FIELD( m_BaseScore,						"baseScore")
	XT_FIELD( m_RaceTimeAverage,				"raceTimeAverage")
	XT_FIELD( m_RaceTimeMultiplierMax,			"raceTimeMultiplierMax")
	XT_FIELD( m_RankMultiplier,					"rankMultiplier")
	XT_FIELD( m_PositionMultipliers,			"positionMultipliers")
XT_END_TYPE

XT_TYPE( PickupPercentages, "PickupPercentages")
	XT_FIELD( m_ChanceOfNitro,						"chance_of_nitro")
	XT_FIELD( m_ChanceOfShunt,						"chance_of_shunt")
	XT_FIELD( m_ChanceOfShock,						"chance_of_shock")
	XT_FIELD( m_ChanceOfBarge,						"chance_of_barge")
	XT_FIELD( m_ChanceOfMine,						"chance_of_mine")
	XT_FIELD( m_ChanceOfBlindfire,					"chance_of_blindfire")
	XT_FIELD( m_ChanceOfRepair,						"chance_of_repair")
	XT_FIELD( m_ChanceOfShield,						"chance_of_shield")
XT_END_TYPE

XT_TYPE( BribeModSettings, "BribeModSettings" )
	XT_FIELD( m_MotorMashStart,							"motor_mash_start")
	XT_FIELD( m_MotorMashRespawn,						"motor_mash_respawn")

	XT_FIELD( m_RaceStart,								"race_start")
	XT_FIELD( m_RaceRespawn,							"race_respawn")
	XT_FIELD( m_GiftDelayTime,							"gift_delay_time")
XT_END_TYPE

XT_TYPE( BatteringRamFakePhysicsSettings, "BatteringRamFakePhysicsSettings" )
	XT_FIELD( m_ImpulseScale,						"impulse_scale")
	XT_FIELD( m_KickForce,							"kick_force")
	XT_FIELD( m_ConstantKickForce,					"constant_kick_force")
XT_END_TYPE

XT_TYPE( BatteringRamCollisionConsequences, "BatteringRamCollisionConsequences")
	XT_FIELD( m_CoolDownTimer,						"cool_down_timer")
	XT_FIELD( m_TargettingCone,						"targetting_cone")
	XT_FIELD( m_MinimumImpactSpeedInMPH,			"minimum_impact_speed_in_MPH")
													
	XT_FIELD( m_LightHitSettings,					"light_hit_settings")
	XT_FIELD( m_HeavyHitSettings,					"heavy_hit_settings")
XT_END_TYPE

XT_TYPE( AlphaGraph,								"AlphaGraph")
	XT_FIELD( m_AlphaGraph,							"alphaGraph")
XT_END_TYPE

XT_TYPE( CloakingAlphaGraphs,						"CloakingAlphaGraphs")
	XT_FIELD( m_CloakingAlphaGraph,					"cloakingAlphaGraph")
	XT_FIELD( m_UnCloakingAlphaGraph,				"unCloakingAlphaGraph")
XT_END_TYPE

XT_TYPE( SilentRunningModSettings,					"SilentRunningModSettings")
	XT_FIELD( m_LocalGhostOpacity,					"localGhostOpacity" )
	XT_FIELD( m_RemoteGhostOpacity,					"remoteGhostOpacity" )
													 
	XT_FIELD( m_UnCloakFadeUpTime,					"unCloakFadeUpTime" )
	XT_FIELD( m_CloakFadeDownTime,					"cloakFadeDownTime" )
													 
	XT_FIELD( m_PostEnvironmentHitUnCloakTime,		"postEnvironmentHitUnCloakTime" )
	XT_FIELD( m_PostVehicularHitUnCloakTime,		"postVehicularHitUnCloakTime" )
	XT_FIELD( m_PostPowerUpHitUnCloakTime,			"postPowerUpHitUnCloakTime" )
	XT_FIELD( m_PostPowerUpFireUnCloakTime,			"postPowerUpFireUnCloakTime" )
	XT_FIELD( m_PostRaceStartUnCloakTime,			"postRaceStartUnCloakTime" )
													 
	XT_FIELD( m_VisibleOnRadar,						"visibleOnRadar" )

	XT_FIELD( m_AlphaGraphs,						"alphaGraphs" )
XT_END_TYPE

XT_TYPE( Game::Actors::E_FakeVehiclePhysicsEffect, "FakePhysicsEventType")
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_NONE,				"FAKE_PHYSICS_NONE"	)	

	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_SPIN_360_LEFT,      "FAKE_PHYSICS_SPIN_360_LEFT"  )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_SPIN_360_RIGHT,		"FAKE_PHYSICS_SPIN_360_RIGHT" )	
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT,		"FAKE_PHYSICS_SPIN_45_LEFT" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT,		"FAKE_PHYSICS_SPIN_45_RIGHT" )

	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_FLIP_FORWARD,		"FAKE_PHYSICS_FLIP_FORWARD" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_FLIP_BACKWARD,		"FAKE_PHYSICS_FLIP_BACKWARD" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_ROLL_LEFT,			"FAKE_PHYSICS_ROLL_LEFT" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_ROLL_RIGHT,			"FAKE_PHYSICS_ROLL_RIGHT" )

	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_LEFT,	"FAKE_PHYSICS_JINK_LEFT" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_RIGHT,	"FAKE_PHYSICS_JINK_RIGHT" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_TIP_FORWARD,		"FAKE_PHYSICS_TIP_FORWARD" )
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_TIP_BACKWARD,		"FAKE_PHYSICS_TIP_BACKWARD" )

	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT,	"FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT")
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT,	"FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT")
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT,	"FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT")
	XT_OPTION( Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT,	"FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT")

XT_END_TYPE

XT_TYPE( LastGaspModSettings, "LastGaspModSettings")


	XT_FIELD (m_LastGaspJumpTheGunCombinedWreckedSequenceReductionMultiplier, "last_gasp_jump_the_gun_combined_wrecked_sequence_reduction_multiplier")
	XT_FIELD (m_LastGaspWreckedSequenceReductionMultiplier, "last_gasp_wrecked_sequence_reduction_multiplier")
	XT_FIELD( m_wreckImpulseGlobalScale,			"wreck_impulse_global_scale")
	XT_FIELD( m_wreckAngularVelocityModifier,		"wreck_angular_velocity_modifier")
	XT_FIELD( m_wreckUpVelocityModifier,			"wreck_up_velocity_modifier")
	XT_FIELD( m_bargeDamage,						"barge_damage")
	XT_FIELD( m_bargeRadiusModifier,				"barge_radius_modifier")
	XT_FIELD( m_automaticallyFireOnTimeout,			"automatically_fire_on_timeout")
	XT_FIELD( m_timeTillForceFire,					"time_till_force_fire" )
	XT_FIELD( m_timerEnabled,						"timer_enabled" )
	XT_FIELD( m_delayWreckedBrokenGlassScreenEffect, "delay_wrecked_broken_glass_screen_effect" )

	XT_FIELD( m_detonateAllowedDelay,				"detonate_allowed_delay" )
	XT_FIELD( m_damageCooldown,						"damage_cooldown" )
	XT_FIELD( m_wreckedSequenceDelay,				"wrecked_sequence_delay" )

XT_END_TYPE

XT_TYPE( PlayerRubberBandingSettings, "PlayerRubberBandingSettings")
	XT_FIELD( m_MaxDistanceBetweenGroupMembers,		"maxDistanceBetweenGroupMembers")
	XT_FIELD( m_MinDistanceForFullBanding,			"minDistanceForFullBanding")
	XT_FIELD( m_UseRaceDistance,					"useRaceDistance")
	XT_FIELD( m_ImpulseMagnitudeMapping,			"impulseMagnitudeMapping")
	XT_FIELD( m_LightAssistMultiplier,				"lightAssistMultiplier")
	XT_FIELD( m_MediumAssistMultiplier,				"mediumAssistMultiplier")
XT_END_TYPE

XT_TYPE( SplitScreenScores,			"SplitScreenScores")
	XT_FIELD( mOneViewport,			"mOneViewport")
	XT_FIELD( mTwoViewports,		"mTwoViewports")
	XT_FIELD( mThreeViewports,		"mThreeViewports")
	XT_FIELD( mFourViewports,		"mFourViewports")
XT_END_TYPE

XT_TYPE( DamageMultiplierValues,			"DamageMultiplierValues" )
	XT_FIELD( m_High,						"High" )
	XT_FIELD( m_Medium,						"Medium" )
	XT_FIELD( m_Low,						"Low" )
XT_END_TYPE

XT_TYPE( IdleTimeoutData, "IdleTimeoutData" )
	XT_FIELD( m_MotorMashIdleTimeout,		"MotorMashIdleTimeout")
	XT_FIELD( m_RacingIdleTimeout,			"RacingIdleTimeout")
XT_END_TYPE

XT_TYPE( DataGatheringRecords, "DataGatheringRecords" )
	XT_FIELD( m_ActivatedRecords, "dataGatheringFlags" )
XT_END_TYPE

XT_TYPE( MultiplayerConfig, "MultiplayerConfig" )
	XT_CALLBACK( MultiplayerConfigCallback )
	XT_FIELD( m_OnlineModeDisabled,			"online_mode_disabled" )	
	XT_FIELD( m_PeerProxyEnabled,			"peer_proxy_enabled" )
	XT_FIELD( m_JoinSessionsInRace, 		"join_sessions_in_race" )
	XT_FIELD( m_PeerProxyMaxConnections,	"peer_proxy_max_connections")
	XT_FIELD( m_PackVersion,				"pack_version" )	
	XT_FIELD( m_MatchingVersion,			"matching_version" )
	XT_FIELD( m_SplitScreenVehicles,		"vehicles" )
	XT_FIELD( m_MinTotalPlayerShowCount,	"min_total_player_show_count" )
	XT_FIELD( m_MinPlaylistPlayerShowCount,	"min_playlist_player_show_count" )
	XT_FIELD( m_MotorMashDamageMultiplier,	"motor_mash_damage_multiplier" )
	XT_FIELD( m_MotormashScoringValues,		"motormashScoringValues" )
	XT_FIELD( m_MotormashRespawnDelayMultipliers, "motormash_respawn_delay_multipliers" )
	XT_FIELD( m_CampingPreventionTimeout,	"camping_prevention_timeout" )
	XT_FIELD( m_CarRewardFanBonusPercentage,"car_reward_fan_bonus_percentage" )
	XT_FIELD( m_DamageMultiplierValues,		"damage_multiplier_values")

	XT_FIELD( m_IdleTimeoutData,			"idle_timeout_data")
	XT_FIELD( m_ShowDeveloperIcon,			"show_developer_icon")

	XT_FIELD( m_DataGatheringRecords,		"data_gathering_records")

	// Playlist tab
	XT_FIELD( m_OnlineEvents,				"online_playlists" )
	XT_FIELD( m_SplitscreenEvents,			"splitscreen_playlists" )
	XT_FIELD( m_SystemLinkEvents,			"system_link_playlists" )
	XT_FIELD( m_RacePositionPoints,			"race_position_points" )
	XT_FIELD( m_FindPlayersTimer,			"find_players_timer" )
	XT_FIELD( m_PlaylistAiPaddingLimit,		"ai_padding_limit" )
	XT_FIELD( m_OverRankPunitiveMultiplier, "over_rank_punitive_multiplier" )
	XT_FIELD( m_ChampionshipCompletionBonus,"championship_completion_bonus" )
	XT_FIELD( m_ChampionshipWaitTime,		"championship_wait_time" )
	XT_FIELD( m_ChampionshipPlayerLeftPenalty,"championship_player_left_penalty" )

	// Custom Race tab
	XT_FIELD( m_Routes,						"routes" )
	XT_FIELD( m_Arenas,						"arenas" )
	XT_FIELD( m_CustomRaceDefaults,			"defaults" )
	XT_FIELD( m_CustomRaceTimeout,			"custom_race_timeout" )

	// Scoring tab
	XT_FIELD( m_ScoringRules,   			"scoring" )
	XT_FIELD( m_PostRaceBonus, 				"postRaceBonus")

	// Profile tab
	XT_FIELD( m_ChallengePacks,				"challenge_packs" )
	XT_FIELD( m_AbilitySlots,				"ability_categories" )
	XT_FIELD( m_LoadOuts,					"load_outs" )
	XT_FIELD( m_SystemLinkLoadOuts,			"system_link_load_outs" )
	XT_FIELD( m_CarUpgrades,				"upgrades" )
	XT_FIELD( m_LevelingData,  				"leveling_data" )
	XT_FIELD( m_LegendLevelingData,  		"legend_leveling_data" )

	// Mod tab
	XT_FIELD( m_MiscModSettings,			"misc_mod_settings" )
	XT_FIELD( m_LastGaspModSettings,			"last_gasp_mod_settings" )
	XT_FIELD( m_ShowyFlourishBlindfireExtraFans, "showy_flourish_blindfire_extra_fans")

	XT_FIELD( m_BribeModSettings,			"bribe_mod_settings")
	XT_FIELD( m_BatteringRamCollisionConsequences, "battering_ram_collision_consequences")

	XT_FIELD( m_SilentRunningModSettings,	"silent_running_mod_settings")

	// Awards tab
	XT_FIELD( m_MultiplayerAwardsEnabled,	"multiplayer_awards_enabled")
	XT_FIELD( m_MultiplayerAwards,			"multiplayer_awards")
	XT_FIELD( m_MultiplayerAwardDistributionSpecs, "multiplayer_award_distribution_specs")
	XT_FIELD( m_RivalsCommencementDelayTimer, "rivals_commencement_delay_timer")
	XT_FIELD( m_RivalsCountdownTimer, "rivals_countdown_timer")		 

	//	AI Racers tab
	XT_FIELD( m_AiCharacters,				"ai_characters" )
	XT_FIELD( m_MaxAiPerConsole,			"max_ai_per_console")
	XT_FIELD( m_AiMpRank_Easy,				"ai_average_mp_rank_easy" )
	XT_FIELD( m_AiMpRank_Medium,			"ai_average_mp_rank_medium" )
	XT_FIELD( m_AiMpRank_Hard,				"ai_average_mp_rank_hard" )
	XT_FIELD( m_AiMpRankVariance,			"ai_mp_rank_spread")

	// Splitscreen
	XT_FIELD( m_PowerupRespawnToggleLong,	  "powerup_respawn_toggle_long")
	XT_FIELD( m_PowerupRespawnToggleVeryLong, "powerup_respawn_toggle_very_long")
	XT_FIELD( m_NumberOfSplitScreenAIPlayers, "number_of_ai_players" )
	XT_FIELD( m_PlayerRubberBandingSettings, "player_rubber_banding_settings")
	XT_FIELD( m_ScoresWithAI, "scores_with_ai")
	XT_FIELD( m_ScoresWithoutAI, "scores_without_ai")
	XT_FIELD( m_SplitScreenLightPillars, "player_pillar_effects")
	XT_FIELD (m_TimeOutTimeHidden, "timeout_time_hidden")
	XT_FIELD (m_TimeOutTimeVisible, "timeout_time_visible")

	// Challenge progress
	XT_FIELD ( m_minChallengeSize,		 "show_all_progress_when_challenge_size_less_than")
	XT_FIELD ( m_introZone,				 "show_intro_zone")
	XT_FIELD ( m_outroZone,			     "show_outro_zone")
	XT_FIELD ( m_percentInterval,		 "show_at_percent_interval")

	// Teams
	XT_FIELD( m_InvalidTeamVehicles, "invalid_team_vehicles" )
	XT_FIELD( m_BalancingRange,	"balancing_range" )
	XT_FIELD( m_TeamAName, "team_a_name" )
	XT_FIELD( m_TeamBName, "team_b_name" )
	XT_FIELD( m_TeamAColour, "team_a_colour" )
	XT_FIELD( m_TeamBColour, "team_b_colour" )
	XT_FIELD( m_TeamAPaint,	 "team_a_paint" )
	XT_FIELD( m_TeamBPaint,	 "team_b_paint" )
	XT_FIELD( m_TeamAPillarEffect, "team_a_pillarEffect" )
	XT_FIELD( m_TeamBPillarEffect, "team_b_pillarEffect" )
	XT_FIELD( m_WinnerTeamFanBonus,	"winner_team_fan_bonus" )
	XT_FIELD( m_WinnerTeamFanBonusBalancingMultiplier, "winner_team_fan_bonus_balancing_multiplier" )

	//	Viral Achievement
	XT_FIELD( m_DeveloperXUIDs,	"developer_retail_xuids")

	//Previously On Blur TMS stuff
	XT_FIELD( m_LatestBlurNewsItemId,	"latest_blur_news_item_id")
	XT_FIELD( m_LatestDlcItemId,		"latest_dlc_item_id")
	XT_FIELD( m_LatestCommunityItemId,	"latest_community_item_id")

XT_END_TYPE

XT_TYPE( MultiplayerCarUpgrade, "MultiplayerCarUpgrade" )
	XT_FIELD( type,	"type" )
	XT_FIELD( name,	"name" )
XT_END_TYPE


XT_TYPE( UI::E_RaceType, "RaceType" )
	XT_OPTION( UI::E_RaceType_Racing,			"racing" )
	XT_OPTION( UI::E_RaceType_Elimination,		"elimination" )
	XT_OPTION( UI::E_RaceType_Points,			"points" )
	XT_OPTION( UI::E_RaceType_Destruction,		"destruction" )
	XT_OPTION( UI::E_RaceType_Hardcore,			"hardcore" )
XT_END_TYPE

XT_TYPE( UI::E_RaceModeFlag, "RaceModeFlag" )
	XT_FLAG_STORAGE(XtU8)
	XT_FLAG( 1 << UI::E_RaceModeFlag_Racing,		"racing" )
	XT_FLAG( 1 << UI::E_RaceModeFlag_Elimination,	"elimination" )
	XT_FLAG( 1 << UI::E_RaceModeFlag_Points,		"points" )
	XT_FLAG( 1 << UI::E_RaceModeFlag_Destruction,	"destruction" )
XT_END_TYPE

XT_TYPE( UI::E_Pickup, "MpPickup" )
	XT_FLAG_STORAGE(XtU16)
	XT_FLAG( 1 << UI::E_Pickup_Nitro,				"nitro" )
	XT_FLAG( 1 << UI::E_Pickup_Shock,				"shock" )
	XT_FLAG( 1 << UI::E_Pickup_Shunt,				"shunt" )
	XT_FLAG( 1 << UI::E_Pickup_Barge,				"barge" )
	XT_FLAG( 1 << UI::E_Pickup_Mine,				"mine" )
	XT_FLAG( 1 << UI::E_Pickup_Bolt,				"bolt" )
	XT_FLAG( 1 << UI::E_Pickup_Shield,				"shield" )
	XT_FLAG( 1 << UI::E_Pickup_Repair,				"repair" )
XT_END_TYPE

XT_TYPE( UI::E_VehicleClass, "MpClass" )
	XT_FLAG_STORAGE(XtU8)
	XT_FLAG( 1 << UI::E_VehicleClass_A,				"class_a" )
	XT_FLAG( 1 << UI::E_VehicleClass_B,				"class_b" )
	XT_FLAG( 1 << UI::E_VehicleClass_C,				"class_c" )
	XT_FLAG( 1 << UI::E_VehicleClass_D,				"class_d" )
XT_END_TYPE

XT_TYPE( UI::E_MiscSettings, "MpMisc" )
	XT_FLAG_STORAGE(XtU16)
	XT_FLAG( 1 << UI::E_MiscSettings_Voting,		"voting" )
	XT_FLAG( 1 << UI::E_MiscSettings_HandOfGod,		"hand_of_god" )
	XT_FLAG( 1 << UI::E_MiscSettings_Abilities,		"abilities" )
	XT_FLAG( 1 << UI::E_MiscSettings_RandomCars,	"random_cars" )
	XT_FLAG( 1 << UI::E_MiscSettings_ShufflePowerupOnRespawn,	"shuffle_powerups_on_respawn" )
	XT_FLAG( 1 << UI::E_MiscSettings_RandomMods,	"random_mods" )
	XT_FLAG( 1 << UI::E_MiscSettings_AddAI,			"add_ai" )
XT_END_TYPE

XT_TYPE( Game::DATA_GATHERING_RECORD_TYPE, "MpDataGathering" )
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_VEHICLE_INFORMATION,		"VehicleInformation" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_RANK_UP,					"RankUp" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_LOBBY_CHOICE,			"LobbyChoice" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_QUIT_RACE,				"QuitRace" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_MOD_SELECTION,			"ModSelection" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_CHECKED_SESSION_BOARD,	"CheckedSessionBoard" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_HOST_MIGRATED,			"HostMigrated" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_VOTE_RESULTS,			"VoteResults" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_CUSTOM_RACE_SETTINGS,	"CustomRaceSettings" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_CHALLENGE_COMPLETED,		"ChallengeCompleted" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_AWARDS_GIVEN,			"AwardsGiven" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_CAREER_INFORMATION,		"CareerInformation" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_POST_RACE,				"PostRaceInformation" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_ON_TRACK_PICKUPS,		"OnTrackPickups" )
	XT_FLAG( 1 << Game::DATA_GATHERING_RECORD_TYPE_REGION_SETTING,			"RegionSetting" )
XT_END_TYPE

XT_TYPE( UI::E_EliminationEvent, "EliminationEvent" )
	XT_OPTION( UI::E_EliminationEvent_LapFirst,			"lap_first" )
	XT_OPTION( UI::E_EliminationEvent_LapSecondLast,	"lap_second_last" )
	XT_OPTION( UI::E_EliminationEvent_Time,				"time" )
	XT_OPTION( UI::E_EliminationEvent_Wrecked,			"wrecked" )
XT_END_TYPE

XT_TYPE( UI::E_EliminationType, "EliminationType" )
	XT_OPTION( UI::E_EliminationType_LastPlace,		"last_place" )
	XT_OPTION( UI::E_EliminationType_FirstPlace,	"first_place" )
	XT_OPTION( UI::E_EliminationType_LeastHealth,	"least_health" )
	XT_OPTION( UI::E_EliminationType_MostHealth,	"most_health" )
	XT_OPTION( UI::E_EliminationType_LeastFans,		"least_fans" )
	XT_OPTION( UI::E_EliminationType_MostFans,		"most_fans" )
XT_END_TYPE

XT_TYPE( UI::E_DamageMode, "DamageMode" )
	XT_OPTION( UI::E_DamageMode_Off,		"off" )
	XT_OPTION( UI::E_DamageMode_Low,		"low" )
	XT_OPTION( UI::E_DamageMode_Medium,		"medium" )
	XT_OPTION( UI::E_DamageMode_High,		"high" )
	XT_OPTION( UI::E_DamageMode_OneHit,		"one_hit" )
XT_END_TYPE

XT_TYPE( UI::E_Handicap, "MpHandicap" )
	XT_OPTION( UI::E_HandicapOff,		"Off" )
	XT_OPTION( UI::E_HandicapLow,		"Low" )
	XT_OPTION( UI::E_HandicapMedium,	"Medium" )
	XT_OPTION( UI::E_HandicapHigh,		"High" )
XT_END_TYPE

XT_TYPE( UI::E_LastManStanding, "MpLastManStanding" )
	XT_OPTION( UI::E_LastManStanding_Off,		"Off" )
	XT_OPTION( UI::E_LastManStanding_On,		"On" )
XT_END_TYPE

XT_TYPE( UI::E_AiMode, "MpAi" )
	XT_OPTION( UI::E_AiOff,			"Off" )
	XT_OPTION( UI::E_AiEasy,		"Easy" )
	XT_OPTION( UI::E_AiMedium,		"Medium" )
	XT_OPTION( UI::E_AiHard,		"Hard" )
XT_END_TYPE

XT_TYPE( UI::E_CarUpgrades, "MpCarUpgrades" )
	XT_OPTION( UI::E_CarUpgrades_Off,		"Off" )
	XT_OPTION( UI::E_CarUpgrades_On,		"On" )
XT_END_TYPE

XT_TYPE( UI::E_PowerupMods, "MpMods" )
	XT_OPTION( UI::E_PowerupMods_Off,		"Off" )
	XT_OPTION( UI::E_PowerupMods_On,		"On" )
XT_END_TYPE

XT_TYPE( UI::E_PowerupRespawnMode,"MpPowerupRespawn" )
	XT_OPTION( UI::E_PowerupRespawnMode_Normal,			"Normal" )
	XT_OPTION( UI::E_PowerupRespawnMode_AlwaysOn,		"AlwaysOn" )
	XT_OPTION( UI::E_PowerupRespawnMode_Long,			"Long" )
	XT_OPTION( UI::E_PowerupRespawnMode_VeryLong,		"VeryLong" )
	XT_OPTION( UI::E_PowerupRespawnMode_None,			"None" )
XT_END_TYPE

XT_TYPE( UI::E_RandomPowerups, "MpRandomPowerups" )
	XT_OPTION( UI::E_RandomPowerups_Off,		"Off" )
	XT_OPTION( UI::E_RandomPowerups_On,			"On" )
XT_END_TYPE

XT_TYPE( MultiplayerTimeoutSettings, "MultiplayerTimeoutSettings" )
	XT_FIELD( mMinPlayers, "min_players" )
	XT_FIELD( mPlayersFinishedToActivateInsivibleTimer, "players_finished_to_activate_invisible_timer" )
	XT_FIELD( mInvisibleTimerTime, "invisible_timer_time" )
	XT_FIELD( mPlayersFinishedToActivateVisibleTimer, "players_finished_to_activate_visible_timer" )
	XT_FIELD( mVisibleTimerTime, "visible_timer_time" )
	XT_FIELD( mOnlyUseFailsafeTimer, "only_use_failsafe_timer" )
	XT_FIELD( mFailsafeTimerTime, "failsafe_timer_time" )
	XT_FIELD( mMaxRaceTimerTime, "max_race_timer_time" )
XT_END_TYPE

XT_TYPE( UI::E_ScoringType, "MultiplayerScoringType" )
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Shunt,			"HitByPerk_Shunt" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Shock,			"HitByPerk_Shock" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Barge,			"HitByPerk_Barge" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Bomb,			"HitByPerk_Mine" )
	XT_FLAG( 1 << UI::E_ScoringType_Impact_Heavy,				"Impact_Heavy" )
	XT_FLAG( 1 << UI::E_ScoringType_Impact_Medium,				"Impact_Medium" )
	XT_FLAG( 1 << UI::E_ScoringType_Impact_Light,				"Impact_Light" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Level2Shunt,	"HitByPerk_Level2Shunt" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Level2Shock,	"HitByPerk_Level2Shock" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Level2Barge,	"HitByPerk_Level2Barge" )
	XT_FLAG( 1 << UI::E_ScoringType_HitByPickup_Level2Bomb,		"HitByPerk_Level2Bomb" )
	XT_FLAG( 1 << UI::E_ScoringType_Double_Barge,				"HitByPerk_Double_Barge" )
	XT_FLAG( 1 << UI::E_ScoringType_Drifting,					"Drifting" )
	XT_FLAG( 1 << UI::E_ScoringType_Air,						"Air" )
	XT_FLAG( 1 << UI::E_ScoringType_Shunt_Overtake,				"ShuntOvertake" )
	XT_FLAG( 1 << UI::E_ScoringType_Nitrous_Overtake,			"NitrousOvertake" )
	XT_FLAG( 1 << UI::E_ScoringType_Shock_Overtake,				"ShockOvertake" )
	XT_FLAG( 1 << UI::E_ScoringType_Skill_Shot_Nitrous,			"SkillShotNitrous" )
	XT_FLAG( 1 << UI::E_ScoringType_Battering_Ram,				"Battering_Ram" )
	XT_FLAG( 1 << UI::E_ScoringType_Crossed_Line_First,			"CrossedLineFirst" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Shunt,			"PickedUpShunt" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Barge,			"PickedUpBarge" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Repair,			"PickedUpRepair" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Nitrous,			"PickedUpNitrous" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Shock,			"PickedUpShock" )
	XT_FLAG( 1 << UI::E_ScoringType_Picked_Up_Bomb,				"PickedUpBomb" )
XT_END_TYPE

XT_TYPE( MultiplayerRace, "MultiplayerRace" )
	XT_FIELD( type,						"type" )
	XT_FIELD( min_players,				"min_players" )
	XT_FIELD( max_players,				"max_players" )
	XT_FIELD( route,					"route" )
	XT_FIELD( powerupActorListOverride,	"powerupActorListOverride" )

	XT_FIELD( laps,						"laps" )
	XT_FIELD( time,						"time" )
	XT_FIELD( score_limit,				"score_limit" )

	XT_FIELD( classes,					"classes" )
	XT_FIELD( powerups,					"powerups" )
	XT_FIELD( pickupProbabilities,		"PickupProbabilities")
	XT_FIELD( damage_mode,				"damage_mode" )
	XT_FIELD( handicap,					"handicap" )
	XT_FIELD( lastManStanding,			"lastmanstanding" )
	XT_FIELD( aiMode,					"ai" )
	XT_FIELD( carUpgrades,				"carupgrades" )
	XT_FIELD( powerupMods,				"mods" )
	XT_FIELD( powerupRespawnMode,		"poweruprespawn" )
	XT_FIELD( randomPowerups,			"randompowerups" )
	XT_FIELD( timeout,					"timeout" )
	XT_FIELD( lock_to_host_car,			"lock_to_host_car" )
XT_END_TYPE

XT_TYPE( MultiplayerEvent, "MultiplayerEvent" )
	XT_CALLBACK( MultiplayerEventCallback )
	XT_BASE(AmaxDLCBase)

	XT_FIELD( name,					"name" )
	XT_FIELD( desc,					"desc" )
	XT_FIELD( icon_name,			"icon_name" )
	XT_FIELD( image_name,			"image_name" )
	XT_FIELD( not_available,		"not_available" )

	XT_FIELD( fan_multiplier,		"fan_multiplier" )
	XT_FIELD( min_players,			"min_players" )
	XT_FIELD( max_players,			"max_players" )
	XT_FIELD( min_rank,				"min_rank" )
	XT_FIELD( max_rank,				"max_rank" )
	XT_FIELD( max_legend,			"max_legend" )
	XT_FIELD( teams,				"teams" )
	XT_FIELD( misc,					"misc" )
	XT_FIELD( aiDifficulty,			"aiDifficulty" )

	XT_FIELD( races,				"races" )

	XT_FIELD( is_championship,		"is_championship" )
	XT_FIELD( championship_length,	"championship_length" )
	XT_FIELD( championship_scores,	"championship_scores" )

	XT_FIELD( timeout_settings,		"timeout_settings" )
XT_END_TYPE

// ---------------------------------------------------------------
XT_TYPE( UI::E_MultiplayerFeature, "MultiplayerFeatureType" )
	XT_OPTION( UI::E_MultiplayerFeature_Default,		"Default" )
	XT_OPTION( UI::E_MultiplayerFeature_ModShop,		"ModShop" )
	XT_OPTION( UI::E_MultiplayerFeature_ResprayCar,		"ResprayCar" )
XT_END_TYPE

// ---------------------------------------------------------------

XT_TYPE( MultiplayerLevel, "MultiplayerLevel" )
	XT_FIELD( mFansNeeded,			"FansNeeded" )
	XT_FIELD( mVehicleUnlocks,		"VehicleUnlocks" )
	XT_FIELD( mUnlockGroupCreator,	"GroupCreator")
	XT_FIELD( mAbilityUnlocks,		"AbilityUnlocks")
	XT_FIELD( mChallengeUnlocks,	"ChallengeUnlocks")
	XT_FIELD( mFeatureUnlocks,		"FeatureUnlocks")
XT_END_TYPE

// ---------------------------------------------------------------

XT_TYPE( MultiplayerScoringRuleBase, "MultiplayerScoringRuleBase" )
	XT_FIELD( mNumPointsScored, "NumPointsScored" )
XT_END_TYPE

XT_TYPE( MultiplayerScoringRuleImpact, "MultiplayerScoringRuleImpact" )
	XT_BASE( MultiplayerScoringRuleBase )
	XT_FIELD( mImpactLevel, "MinImpactLevel" )
XT_END_TYPE

XT_TYPE( MultiplayerScoringRuleHitByPerk, "MultiplayerScoringRuleHitWithPerk" )
	XT_BASE( MultiplayerScoringRuleBase )
	XT_FIELD( mPickupType, "PickupType" )
XT_END_TYPE

XT_TYPE( MultiplayerScoringScoringEvent, "MultiplayerScoringScoringEvent" )
	XT_BASE( MultiplayerScoringRuleBase )
	XT_FIELD( mScoringEvent, "ScoringEvent" )
XT_END_TYPE

XT_TYPE( MultiplayerScoringRulePickedUpPickup, "MultiplayerScoringRulePickedUpPickup" )
	XT_BASE( MultiplayerScoringRuleBase )
	XT_FIELD( mPickupType, "PickupType" )
XT_END_TYPE

XT_TYPE( MultiplayerScoringRuleCrossedLineFirst, "MultiplayerScoringRuleCrossedLineFirst" )
	XT_BASE( MultiplayerScoringRuleBase )
XT_END_TYPE

// ---------------------------------------------------------------

XT_TYPE( UI::E_MultiplayerVehicleAbilities, "MultiplayerAbilityType" )

	XT_OPTION( UI::E_MultiplayerVehicleAbilities_None,					"None" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Iron_Fist,				"IronFist" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Jump_The_Gun,			"JumpTheGun" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Front_Runner,			"FrontRunner" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Drifter,				"Drifter" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Titanium_Armour,		"TitaniumArmour" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Showy_Flourish,		"ShowyFlourish" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Stable_Frame,			"StableFrame" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Battering_Ram,			"BatteringRam" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Decoy_Drop,			"DecoyDrop" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Road_Sweeper,			"RoadSweeper" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Scrambler,				"Scrambler" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Splash_Damage,			"SplashDamage" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency,	"ShieldingEfficiency" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Safety_Net,			"SafetyNet" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Shielded_Boosters,		"ShieldedBoosters" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Shielded_Bay,			"ShieldedBay" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_ECM,					"ECM" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck,		"VampiricWreck" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Bribe,					"Bribe" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Fan_Favourite,			"FanFavourite" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Advanced_Radar,		"AdvancedRadar" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Silent_Running,		"SilentRunning" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Laser_Sight,			"LaserSight" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Last_Gasp,				"LastGasp" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Mastermine,			"Mastermine" )

	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots,	"BossAdditionalBlindfireShots" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes,		"BossAdditionalShockDomes" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_DoubleShield,				"BossDoubleShield" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_FansNitroGift,			"BossFansNitroGift" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_NitroRift,				"BossNitroRift" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_RepairBonus,				"BossRepairBonus")
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire,			"BossShotgunBlindfire" )
	XT_OPTION( UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage,				"BossShuntDamage" )
XT_END_TYPE

XT_TYPE( MultiplayerAbilitySlot, "MultiplayerAbilityCategory" )
	XT_FIELD(mType,					"Slot")
	XT_FIELD(mName,					"Name")
	XT_FIELD(mAbilities,			"Abilities")
XT_END_TYPE

XT_TYPE( MultiplayerAbilityLoadOut, "MultiplayerAbilityLoadOut" )
	XT_FIELD(mType,					"Type")	
	
	XT_FIELD(mAbilitySlot1,			"AbilitySlot1")
	XT_FIELD(mAbilitySlot2,			"AbilitySlot2")
	XT_FIELD(mAbilitySlot3,			"AbilitySlot3")
XT_END_TYPE

// ---------------------------------------------------------------

XT_TYPE( UI::E_MultiplayerStat, "MultiplayerAwardStat" )
	XT_OPTION( UI::E_MultiplayerStat_RaceTime,						"RaceTime" )									
	XT_OPTION( UI::E_MultiplayerStat_FastestLap,					"FastestLap" )
	XT_OPTION( UI::E_MultiplayerStat_SlowestLap,					"SlowestLap" )
	XT_OPTION( UI::E_MultiplayerStat_Fans,							"Fans" )
	XT_OPTION( UI::E_MultiplayerStat_HealthRemaining,				"HealthRemaining" )
	XT_OPTION( UI::E_MultiplayerStat_LogicPositionCombined,			"LogicPositionCombined" )
	XT_OPTION( UI::E_MultiplayerStat_DistanceTravelled,				"DistanceTravelled" )
	XT_OPTION( UI::E_MultiplayerStat_LapsCompleted,					"LapsCompleted" )
	XT_OPTION( UI::E_MultiplayerStat_OnTrackPoints,					"OnTrackPoints" )
	XT_OPTION( UI::E_MultiplayerStat_EliminationPoints,				"EliminationPoints" )
	XT_OPTION( UI::E_MultiplayerStat_PowerupHits,					"PowerupHits" )
	XT_OPTION( UI::E_MultiplayerStat_HitsByPowerups,				"HitsByPowerups" )
	XT_OPTION( UI::E_MultiplayerStat_KillStreak,					"KillStreak" )
	XT_OPTION( UI::E_MultiplayerStat_DeathStreak,					"DeathStreak" )
	XT_OPTION( UI::E_MultiplayerStat_Accuracy,						"Accuracy" )
	XT_OPTION( UI::E_MultiplayerStat_LongestChain,					"LongestChain" )
	XT_OPTION( UI::E_MultiplayerStat_PowerupsEvaded,				"PowerupsEvaded" )
	XT_OPTION( UI::E_MultiplayerStat_TopSpeed,						"TopSpeed" )
	XT_OPTION( UI::E_MultiplayerStat_Car2CarAggressiveHits,			"Car2CarAggressiveHits" )
	XT_OPTION( UI::E_MultiplayerStat_Car2CarVictimHits,				"Car2CarVictimHits" )
	XT_OPTION( UI::E_MultiplayerStat_EnvironmentalCrashes,			"EnvironmentalCrashes" )
	XT_OPTION( UI::E_MultiplayerStat_PickupsCollected,				"PickupsCollected" )
	XT_OPTION( UI::E_MultiplayerStat_LongestDrift,					"LongestDrift" )
	XT_OPTION( UI::E_MultiplayerStat_TimesInWater,					"TimesInWater" )
	XT_OPTION( UI::E_MultiplayerStat_TimesOffEdge,					"TimesOffEdge" )
	XT_OPTION( UI::E_MultiplayerStat_DamageDealt,					"DamageDealt" )
	XT_OPTION( UI::E_MultiplayerStat_LongestTimeWithoutBeingWrecked,"LongestTimeWithoutBeingWrecked" )
	XT_OPTION( UI::E_MultiplayerStat_TotalCarsWrecked,				"TotalCarsWrecked" )
	XT_OPTION( UI::E_MultiplayerStat_TimesHit,						"TimesHit" )
	XT_OPTION( UI::E_MultiplayerStat_MinesFragmentedWithMastermine,	"MinesFragmentedWithMastermine" )
	XT_OPTION( UI::E_MultiplayerStat_TimeInvisibleWithSilentRunning,"TimeInvisibleWithSilentRunning" )
	XT_OPTION( UI::E_MultiplayerStat_PowerupsAbsorbedWithSafetyNet,	"PowerupsAbsorbedWithSafetyNet" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersScrambled,				"PlayersScrambled" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitwithDecoys,			"PlayersHitwithDecoys" )
	XT_OPTION( UI::E_MultiplayerStat_CarsBatteringRammed,			"CarsBatteringRammed" )
	XT_OPTION( UI::E_MultiplayerStat_TimesProtectedByStableFrame,	"TimesProtectedByStableFrame" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitWithIronFist,		"PlayersHitWithIronFist" )
	XT_OPTION( UI::E_MultiplayerStat_PowerupsDropped,				"PowerupsDropped" )
	XT_OPTION( UI::E_MultiplayerStat_LongestTimeNotDamaged,			"LongestTimeNotDamaged" )
	XT_OPTION( UI::E_MultiplayerStat_TimeInFirstPlace,				"TimeInFirstPlace" )
	XT_OPTION( UI::E_MultiplayerStat_LongestJump,					"LongestJump" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitWithBarge,			"PlayersHitWithBarge" )
	XT_OPTION( UI::E_MultiplayerStat_NitroOvertakes,				"NitroOvertakes" )
	XT_OPTION( UI::E_MultiplayerStat_BlockedPowerups,				"BlockedPowerups" )
	XT_OPTION( UI::E_MultiplayerStat_PowerupsShielded,				"PowerupsShielded" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitWithMine,			"PlayersHitWithMine" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitWithShunt,			"PlayersHitWithShunt" )
	XT_OPTION( UI::E_MultiplayerStat_PlayersHitWithBolt,			"PlayersHitWithBolt" )
XT_END_TYPE

XT_TYPE( UI::E_MultiplayerStatSortingStrategy,							"MultiplayerAwardStatSortingStrategy" )
	XT_OPTION( UI::E_MultiplayerStatSortingStrategy_HighestWins,		"HighestWins")
	XT_OPTION( UI::E_MultiplayerStatSortingStrategy_LowestWins,			"LowestWins")
XT_END_TYPE

XT_TYPE( UI::E_MultiplayerAward, "MultiplayerAwardType" )
	XT_OPTION( UI::E_MultiplayerAward_SpeedDemon,		"SpeedDemon")
	XT_OPTION( UI::E_MultiplayerAward_Tortoise,			"Tortoise")
	XT_OPTION( UI::E_MultiplayerAward_FanTastic,		"FanTastic")
	XT_OPTION( UI::E_MultiplayerAward_OldNews,			"OldNews")
	XT_OPTION( UI::E_MultiplayerAward_Marksman,			"Marksman")
	XT_OPTION( UI::E_MultiplayerAward_TriggerHappy,		"TriggerHappy")
	XT_OPTION( UI::E_MultiplayerAward_Squirrel,			"Squirrel")
	XT_OPTION( UI::E_MultiplayerAward_Gladiator,		"Gladiator")
	XT_OPTION( UI::E_MultiplayerAward_Drifter,			"Drifter")
	XT_OPTION( UI::E_MultiplayerAward_CrashTestDummy,	"CrashTestDummy")
	XT_OPTION( UI::E_MultiplayerAward_BreakNeck,		"BreakNeck")
	XT_OPTION( UI::E_MultiplayerAward_Agile,			"Agile")
	XT_OPTION( UI::E_MultiplayerAward_Brawler,			"Brawler")
	XT_OPTION( UI::E_MultiplayerAward_CleanRacer,		"CleanRacer")
	XT_OPTION( UI::E_MultiplayerAward_WreckingBall, 	"WreckingBall") 
	XT_OPTION( UI::E_MultiplayerAward_Survivor,			"Survivor")
	XT_OPTION( UI::E_MultiplayerAward_Relentless,		"Relentless")
	XT_OPTION( UI::E_MultiplayerAward_WriteOff,  		"WriteOff")  
	XT_OPTION( UI::E_MultiplayerAward_PunchingBag,		"PunchingBag")
	XT_OPTION( UI::E_MultiplayerAward_DeepSeaDiver,		"DeepSeaDiver")
	XT_OPTION( UI::E_MultiplayerAward_Cliffhanger,		"Cliffhanger")
	XT_OPTION( UI::E_MultiplayerAward_Environmentalist,	"Environmentalist")

	XT_OPTION( UI::E_MultiplayerAward_Mastermine,		"Mastermine")
	XT_OPTION( UI::E_MultiplayerAward_SilentRunning,	"SilentRunning")
	XT_OPTION( UI::E_MultiplayerAward_SafetyNet,		"SafetyNet")
	XT_OPTION( UI::E_MultiplayerAward_Scrambler,		"Scrambler")
	XT_OPTION( UI::E_MultiplayerAward_DecoyDrop,		"DecoyDrop")
	XT_OPTION( UI::E_MultiplayerAward_Batterer,			"Batterer")
	XT_OPTION( UI::E_MultiplayerAward_StableFrame,		"StableFrame")
	XT_OPTION( UI::E_MultiplayerAward_IronFist,			"IronFist")
	XT_OPTION( UI::E_MultiplayerAward_ButterFingers,	"ButterFingers")
	XT_OPTION( UI::E_MultiplayerAward_Dexterous,		"Dexterous")
	XT_OPTION( UI::E_MultiplayerAward_Dominator,		"Dominator")
	XT_OPTION( UI::E_MultiplayerAward_LongJumper,		"LongJumper")
	XT_OPTION( UI::E_MultiplayerAward_CloseEncounters,	"CloseEncounters")
	XT_OPTION( UI::E_MultiplayerAward_Booster,			"Booster")
	XT_OPTION( UI::E_MultiplayerAward_SafeHands,		"SafeHands")
	XT_OPTION( UI::E_MultiplayerAward_Defender,			"Defender")
	XT_OPTION( UI::E_MultiplayerAward_LitterBug,		"LitterBug")
	XT_OPTION( UI::E_MultiplayerAward_CrimsonCarapace,	"CrimsonCarapace")
	XT_OPTION( UI::E_MultiplayerAward_Hunter,			"Hunter")

XT_END_TYPE

XT_TYPE( UI::E_MultiplayerAwardCategory, "MultiplayerAwardCategory" )
	XT_OPTION( UI::E_MultiplayerAwardCategory_Award,				"Award" )
	XT_OPTION( UI::E_MultiplayerAwardCategory_WoodenSpoon,		"WoodenSpoon" )
XT_END_TYPE

XT_TYPE( MultiplayerAwardData, "MultiplayerAward" )
	XT_FIELD( mAwardId,								"AwardType" )
	XT_FIELD( mAwardCategory,						"AwardCategory" )
	XT_FIELD( mAwardStat,							"AwardStat" )
	XT_FIELD( mAwardStatSortingStrategy,			"AwardStatSortingStrategy" )
	XT_FIELD( mAwardName,							"AwardName" )
	XT_FIELD( mAwardDescription,					"AwardDescription" )
	XT_FIELD( mIconName,							"IconName")
	XT_FIELD( mThresholdValue,						"AwardThreshold" )
	XT_FIELD( mMotormashThresholdMultiplier,		"MotormashThresholdMultiplier" )
	XT_FIELD( mRaceThresholdMultiplier,				"RaceThresholdMultiplier" )
	XT_FIELD( mRequiresAISplineData,				"RequiresAISplineData" )
	XT_FIELD( mDisabledInMotormash, 				"DisabledInMotormash" )
	XT_FIELD( mDisabledInRace, 						"DisabledInRace" )
	XT_FIELD( mReducedProbabilityPositionPercentage,"ReducedProbabilityPositionPercentage" )
XT_END_TYPE

XT_TYPE( MultiplayerAwardDistributionSpec,			"MultiplayerAwardDistributionSpec" )
	XT_FIELD( mMinNumPlayersRequired,				"MinNumPlayersRequired" )
	XT_FIELD( mNumRandomAwardsOrSpoonsToGift,		"NumRandomAwardsOrSpoonsToGift" )
	XT_FIELD( mNumSpoonsToGift,						"NumSpoonsToGift" )
	XT_FIELD( mNumAwardsToGift,						"NumAwardsToGift" )
	XT_FIELD( mMaxNumAwardsPerPlayer,				"MaxNumAwardsPerPlayer" )
XT_END_TYPE
// ---------------------------------------------------------------

XT_TYPE( MultiplayerChallengePack, "MultiplayerChallengePack" )
	XT_FIELD(mName,					"Name")
	XT_FIELD(mUnlockName,			"UnlockName")
	XT_FIELD(mDesc,					"Description")
	XT_FIELD(mType,					"Type")
	XT_FIELD(mChallenges,			"Challenges")
	XT_FIELD(mIconName,				"IconName")
XT_END_TYPE

XT_TYPE( MultiplayerChallenge,		"MultiplayerChallenge" )
	XT_FIELD(mName,					"Name")
	XT_FIELD(mType,					"Type")
	XT_FIELD(mHideProgress,			"HideProgress")
	XT_FIELD(mDescription,			"Description")
	XT_FIELD(mTiers,				"Tiers")
XT_END_TYPE

XT_TYPE( MultiplayerChallengeTier,	"MultiplayerChallengeTier" )
	XT_FIELD( mFansReward,			"FansReward" )
	XT_FIELD( mTargetAmount,		"TargetAmount" )
	XT_FIELD( mData,				"Data" )
	XT_FIELD( mReward,				"Reward" )
XT_END_TYPE



// ---------------------------------------------------------------

XT_TYPE( UI::E_MultiplayerVehicleUpgrades, "MultiplayerChallengeRewardType" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_None,			"None" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_HealthSlot,		"HealthSlot" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_FanBonus,		"FanBonus" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_SpeedBoost,		"SpeedBoost" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_Metallic,		"Metallic" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_Pearlescent,	"Pearlescent" )
	XT_OPTION( UI::E_MultiplayerVehicleUpgrades_Candy,			"Candy" )
XT_END_TYPE

XT_TYPE( UI::E_ChallengePack, "ChallengePackCategory" )
	XT_OPTION( UI::E_ChallengePack_Ace,			"ace" )
	XT_OPTION( UI::E_ChallengePack_Car,			"car" )
	XT_OPTION( UI::E_ChallengePack_Daily,		"daily" )
	XT_OPTION( UI::E_ChallengePack_Demolition,	"demolition" )
	XT_OPTION( UI::E_ChallengePack_DriversED,	"driversEd" )
	XT_OPTION( UI::E_ChallengePack_Specialist,	"specialist" )
XT_END_TYPE


XT_TYPE( CustomRaceDefaults, "CustomRaceDefaults" )
	XT_FIELD( game_mode,		"game_mode" )
	XT_FIELD( track,			"track" )
	XT_FIELD( laps,				"laps" )
	XT_FIELD( time,				"time" )
	XT_FIELD( car_class,		"class" )
	XT_FIELD( damage,			"damage" )
	XT_FIELD( respawn,			"respawn" )
	XT_FIELD( timeout,			"timeout" )
	XT_FIELD( mods,				"mods" )
	XT_FIELD( upgrades,			"upgrades" )
	XT_FIELD( lock_to_host_car,	"lock_to_host_car" )
	XT_FIELD( ai,				"ai" )
	XT_FIELD( ai_padding,		"ai_padding" )
	XT_FIELD( handicap,			"handicap" )

	XT_FIELD( random_powerups,	"random_powerups" )
	XT_FIELD( powerup_respawn,	"powerup_respawn" )

	XT_FIELD( max_laps,			"max_laps" )
	XT_FIELD( max_time,			"max_time" )
	XT_FIELD( time_step,		"time_step" )
XT_END_TYPE


XT_TYPE( UI::E_MULTIPLAYER_CHALLENGE_TYPE, "ChallengeType" )
	//Ace challenges
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK,		"ultimateComeback" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME,		"extremelyExtreme" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER,			"highSlider" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SIDEWINDER,				"sidewinder" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL,					"ftl" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE,				"chosenOne" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE,				"rampage" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASTER_EXPLODER,		"masterExploder" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW,				"quickDraw" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS,			"causalNexus" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SKEET_SHOOTER,			"skeetShooter" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY,				"tearaway" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_UNTOUCHABLE,			"untouchable" )
																		
	//Car challenges													
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES,		"completedRaces" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER,					"winner" )
																		
	//Daily challenges													
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_OVERTAKE,				"overtake" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT,			"participant" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK,			"hardDaysWork" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DEMOLITIONS_EXPERT,		"demolitionsExpert" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK,				"hatTrick" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_STYLE,					"style" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT,				"facePlant" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER,				"piedPiper" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS,		"headInTheClouds" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HOARDER,				"hoarder" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION,			"chainReaction" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AGGRESSIVE_PERSONALITY,	"aggressivePersonality" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR,				"dominator" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE,				"distance" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY,				"carCrazy" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED,			"wellTravelled" )
																		
	//Demolition challenges												
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION,			"retribution" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER,				"punisher" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WRECKING_BALL,			"wreckingBall" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUISER,				"bruiser" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE,				"carnage" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTAL_DELUXE,			"brutalDeluxe" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_VENDETTA,				"vendetta" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTE_FORCE,			"bruteForce" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT,		"causeAndEffect" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BATTERING_RAM,			"batteringRam" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SCRAMBLER,				"scrambler" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_LAST_GASP,				"lastGasp" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DECEIVER,				"deceiver" )
																		
	//Driver's Ed. challenges											
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE,					"barge" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHIELD,					"shield" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE,					"mine" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO,					"nitro" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK,					"shock" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT,					"shunt" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE,				"blindFire" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF,				"showOff" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT_EVADER,			"shuntEvader" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK_EVADER,			"shockEvader" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINESWEEPER,			"minesweeper" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_SHUNT,			"reverseShunt" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AIRBRAKE,				"airbrake" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_BLIND_FIRE,		"reverseBlindFire" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_LOBBER,			"mineLobber" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REAR_GUARD,				"rearGuard" )
																		
	//Specialist challenges												
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST,				"pacifist" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN,				"thickSkin" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST,				"masochist" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST,		"bargeSpecialist" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST,		"mineSpecialist" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE_SPECIALIST,	"blindFireSpecialist" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING,		"backwardsThinking" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER,			"afterburner" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER,					"jumper" )

	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_ELF_EYES,				"elfEyes" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SAFETY_NET,				"safetyNet" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_ROAD_SWEEPER,			"roadSweeper" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DRIFTER,				"drifter" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASTERMINE,				"masterMine" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_STEALTH,				"stealth" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BULLY,					"bully" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FULLY_BOLT,				"fullyBolt" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RAM,				"nitroRam" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_RIVALRY,				"rivalry" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_LIGHT_WEIGHT,			"lightWeight" )
	XT_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RIFT,				"nitroRift" )

XT_END_TYPE

XT_TYPE( UI::E_MULTIPLAYER_LOADOUT_TYPE, "MultiplayerLoadOutType" )
	XT_OPTION(UI::E_MULTIPLAYER_LOADOUT_TYPE_DEFAULT1, "Default1")
					
	XT_OPTION(UI::E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM1, "Custom1")
	XT_OPTION(UI::E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM2, "Custom2")
	XT_OPTION(UI::E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM3, "Custom3")
XT_END_TYPE


XT_TYPE( UI::E_MultiplayerAbilityCategory, "MultiplayerAbilitySlotType" )
	XT_OPTION(UI::E_MultiplayerAbilityCategory_Slot1,	"Slot1")
	XT_OPTION(UI::E_MultiplayerAbilityCategory_Slot2,	"Slot2")
	XT_OPTION(UI::E_MultiplayerAbilityCategory_Slot3,	"Slot3")
XT_END_TYPE


XT_TYPE( E_MULTIPLAYER_AIDIFFICULTY, "MultiplayerAIDifficulty" )
	XT_OPTION(E_MULTIPLAYER_AIDIFFICULTY_OFF, "Off")

	XT_OPTION(E_MULTIPLAYER_AIDIFFICULTY_EASY, "Easy")
	XT_OPTION(E_MULTIPLAYER_AIDIFFICULTY_NORMAL, "Normal")
	XT_OPTION(E_MULTIPLAYER_AIDIFFICULTY_HARD, "Hard")
XT_END_TYPE


// ---------------------------------------------------------------

XT_TYPE( MultiplayerAbilityData,	"MultiplayerAbility" )
	XT_FIELD( mAbilityName,			"AbilityName" )
	XT_FIELD( mAbilityDescription,	"AbilityDescription" )
	XT_FIELD( mIconName,			"IconName")
	XT_FIELD( mAbilityId,			"AbilityId" )
	XT_FIELD( mAbilityCategory,		"AbilitySlot" )
	XT_FIELD( mAbilityValue,		"AbilityValue" )
	XT_FIELD( mAbilitySpecialFX,	"OnActivateSpecialFX" )
XT_END_TYPE

XT_TYPE( MultiplayerFeature,	"MultiplayerFeature" )
	XT_FIELD( mFeatureName,			"FeatureName" )
	XT_FIELD( mFeatureDescription,	"FeatureDescription" )
	XT_FIELD( mIconName,			"IconName")
	XT_FIELD( mIconSheet,			"IconSheet")
	XT_FIELD( mTexture,				"Texture")
	XT_FIELD( mFeatureId,			"FeatureId" )
	XT_FIELD( mIsPlaylist,			"IsPlaylist" )
XT_END_TYPE


XT_TYPE( UI::E_SPLITSCREEN_EVENT_TYPE, "SplitScreenEventType" )
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_CLEAN,		"Clean")
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_ELIMINATOR,	"Eliminator")
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_NITRO,		"Nitro")
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_RACE,		"Race")
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_SURVIVOR,	"Survivor")
	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_TARGET,		"Target")

	XT_OPTION(UI::E_SPLITSCREEN_EVENT_TYPE_WORLD_TOUR,	"WorldTour")
XT_END_TYPE

XT_LIBRARY(MULTIPLAYERDATA)	
	XT_DEPENDS_ON(SpecialFX)
	XT_REGISTER_TYPE(MultiplayerConfig)
	XT_REGISTER_TYPE(MultiplayerEvent)
	XT_REGISTER_TYPE(MultiplayerRace)
	XT_REGISTER_TYPE(CustomRaceDefaults)

	XT_REGISTER_TYPE(UI::E_RaceType)
	XT_REGISTER_TYPE(UI::E_RaceModeFlag)
	XT_REGISTER_TYPE(UI::E_Pickup)
	XT_REGISTER_TYPE(UI::E_VehicleClass)
	XT_REGISTER_TYPE(UI::E_EliminationEvent)
	XT_REGISTER_TYPE(UI::E_EliminationType)
	XT_REGISTER_TYPE(UI::E_DamageMode)
	XT_REGISTER_TYPE(UI::E_MiscSettings)
	XT_REGISTER_TYPE(Game::DATA_GATHERING_RECORD_TYPE)
	XT_REGISTER_TYPE(UI::E_Handicap)
	XT_REGISTER_TYPE(UI::E_LastManStanding)
	XT_REGISTER_TYPE(UI::E_AiMode)
	XT_REGISTER_TYPE(UI::E_CarUpgrades)
	XT_REGISTER_TYPE(UI::E_PowerupMods)
	XT_REGISTER_TYPE(UI::E_PowerupRespawnMode)
	XT_REGISTER_TYPE(UI::E_RandomPowerups)

	XT_REGISTER_TYPE(UI::E_ScoringType)
	XT_REGISTER_TYPE(MultiplayerLevel)

	XT_REGISTER_TYPE(MultiplayerTimeoutSettings)
	XT_REGISTER_TYPE(UI::E_MultiplayerVehicleAbilities)
	XT_REGISTER_TYPE(UI::E_MultiplayerAbilityCategory)
	XT_REGISTER_TYPE(E_MULTIPLAYER_AIDIFFICULTY)
	XT_REGISTER_TYPE(MultiplayerFeature)
	XT_REGISTER_TYPE(MultiplayerAbilityData)
	XT_REGISTER_TYPE(MultiplayerAbilitySlot)
	XT_REGISTER_TYPE(MultiplayerAbilityLoadOut)

	XT_REGISTER_TYPE(MultiplayerChallengePack)
	XT_REGISTER_TYPE(MultiplayerChallenge)
	XT_REGISTER_TYPE(MultiplayerChallengeTier)

	XT_REGISTER_TYPE(UI::E_MultiplayerVehicleUpgrades)
	XT_REGISTER_TYPE(UI::E_ChallengePack)
	XT_REGISTER_TYPE(UI::E_MULTIPLAYER_CHALLENGE_TYPE )
	XT_REGISTER_TYPE(UI::E_MULTIPLAYER_LOADOUT_TYPE);

	XT_REGISTER_TYPE(UI::E_SPLITSCREEN_EVENT_TYPE);

	XT_REGISTER_TYPE(MultiplayerScoringRuleBase)
	XT_REGISTER_TYPE(MultiplayerScoringRuleImpact)
	XT_REGISTER_TYPE(MultiplayerScoringRuleHitByPerk)
	XT_REGISTER_TYPE(MultiplayerScoringScoringEvent)

	XT_REGISTER_TYPE(MultiplayerCarUpgrade)
	XT_REGISTER_TYPE(MultiplayerScoringRulePickedUpPickup)
	XT_REGISTER_TYPE(MultiplayerScoringRuleCrossedLineFirst)
	XT_REGISTER_TYPE(MotorMashScoringValues)
	XT_REGISTER_TYPE(DamageMultiplierValues)
	XT_REGISTER_TYPE(MiscModSettings)
	XT_REGISTER_TYPE(PickupPercentages)
	XT_REGISTER_TYPE(PostRaceBonusData)
	XT_REGISTER_TYPE(BatteringRamFakePhysicsSettings)
	XT_REGISTER_TYPE(BatteringRamCollisionConsequences)

	XT_REGISTER_TYPE(AlphaGraph)
	XT_REGISTER_TYPE(CloakingAlphaGraphs)
	XT_REGISTER_TYPE(SilentRunningModSettings)

	XT_REGISTER_TYPE(Game::Actors::E_FakeVehiclePhysicsEffect)
	XT_REGISTER_TYPE(LastGaspModSettings)
	XT_REGISTER_TYPE(BribeModSettings)
	XT_REGISTER_TYPE(PlayerRubberBandingSettings)

	XT_REGISTER_TYPE(UI::E_MultiplayerFeature)

	XT_REGISTER_TYPE(UI::E_MultiplayerAward)
	XT_REGISTER_TYPE(UI::E_MultiplayerAwardCategory)
	XT_REGISTER_TYPE(UI::E_MultiplayerStat)
	XT_REGISTER_TYPE(UI::E_MultiplayerStatSortingStrategy)
	XT_REGISTER_TYPE(MultiplayerAwardData)
	XT_REGISTER_TYPE(MultiplayerAwardDistributionSpec)
	XT_REGISTER_TYPE(SplitScreenScores)
	XT_REGISTER_TYPE(IdleTimeoutData)
	XT_REGISTER_TYPE(DataGatheringRecords)

XT_END_LIBRARY

const AmaxRoute* C_MultiplayerData::FindRoute( const U32 uID ) const
{
	for ( S32 i = 0; i < m_pConfig->m_Routes.count; i++ )
	{
		if ( m_pConfig->m_Routes[i]->ID == uID )
			return m_pConfig->m_Routes[i].Object();
	}
	return NULL;
}

const AmaxVehicle* C_MultiplayerData::FindVehicle( const U32 uID ) const
{
	for ( S32 i = 0; i < m_MultiplayerVehicles.Size(); i++ )
	{
		if ( m_MultiplayerVehicles[i]->ID == uID )
			return m_MultiplayerVehicles[i].Object();
	}
	return NULL;
}

const MultiplayerAbilityData* C_MultiplayerData::FindAbility( const UI::E_MultiplayerVehicleAbilities eAbilityType ) const
{
	BIZ_ASSERT_MSG(eAbilityType < UI::E_MultiplayerVehicleAbilities_NumberOfAbilities, "C_MultiplayerData::FindAbility( abilityType ) > Invalid ability type");

	MultiplayerAbilitySlot *pAbilitySlot = 0;
	MultiplayerAbilityData *pAbility = 0;
	for( S32 i = 0; i < m_pConfig->m_AbilitySlots.count; i++)
	{
		pAbilitySlot = m_pConfig->m_AbilitySlots[i];
		BIZ_ASSERT_MSG(pAbilitySlot, "C_MultiplayerData::FindAbility( abilityType ) > Invalid ability slot pointer");

		for( S32 j = 0; j < pAbilitySlot->mAbilities.count; j++)
		{
			pAbility = pAbilitySlot->mAbilities[j];
			BIZ_ASSERT_MSG(pAbility, "C_MultiplayerData::FindAbility( abilityType ) > Invalid ability pointer");	

			if(pAbility->mAbilityId == eAbilityType)
			{
				return pAbility;
			}
		}
	}

	BIZ_ASSERT_MSG(pAbility, "C_MultiplayerData::FindAbility( abilityType ) > Couldn't find ability");	
	return (MultiplayerAbilityData*)0;
}

const MultiplayerAbilityData* C_MultiplayerData::FindAbility( const XtId id ) const
{
	for ( S32 i = 0; i < m_pConfig->m_AbilitySlots.count; i++ )
		for ( S32 j = 0; j < m_pConfig->m_AbilitySlots[ i ]->mAbilities.count; j++ )
			if ( m_pConfig->m_AbilitySlots[ i ]->mAbilities[ j ].Id() == id )
				return m_pConfig->m_AbilitySlots[ i ]->mAbilities[ j ];

	return NULL;
}

const HRESULT C_MultiplayerData::LoadPackFile( XtPack*& pPack, const char* pFileName )
{
	HRESULT result = E_FAIL;
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* pFile = fs.Open( pFileName );

	if ( pFile )
	{
		U32 length = (U32)pFile->Length();
		U8* pData = new U8[ length ];

		if ( pData )
		{
			if ( pFile->Read( pData, length ) )
			{
				if( true == LoadPackData( pPack, pData, length) )
				{
					result = S_OK;
				}
			}
			delete [] pData;
		}
		delete pFile;		
	}

	return result;
}


const bool C_MultiplayerData::LoadPackData( XtPack*& pPack, void* pData, const U32 uLength )
{
	if ( pPack )
	{
		XtPackClose( C_GameApp::GetInstance()->GetGlobalXtDatabase(), pPack );
	}

	pPack = XtPackOpen(C_GameApp::GetInstance()->GetGlobalXtDatabase(), pData, uLength, 0);
	if(NULL == pPack)
		return false;

	return true;
}


const HRESULT C_MultiplayerData::LoadMultiplayerPackFile( const char* pFileName )
{
	HRESULT success = LoadPackFile( m_pMultiplayerPack, pFileName );
	InitSubsystems();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if(pLayer)
	{
		pLayer->SetTitleStorageDataActive(false);
		pLayer->GetDataGathering().Init(C_MultiplayerData::Instance().m_pConfig->m_DataGatheringRecords);
	}

	return success;	
}

const bool C_MultiplayerData::LoadMultiplayerPackData( U8* pData, const U32 uLength, bool overwrite )
{
	BIZ_ASSERT_MSG_SPEED(uLength <= 256000, "C_MultiplayerData::LoadMultiplayerPackData - Multiplayer data greater than 256000B (%iB)", uLength);

	if (!pData||!uLength)
		return false;

	if(overwrite)
	{
		//Delete existing data;
		if(m_pMultiplayerData)
		{
			delete [] m_pMultiplayerData;
			m_pMultiplayerData = NULL;
			m_MultiplayerDataLength = 0;
		}

		//And store latest data
		m_pMultiplayerData = pData;
		m_MultiplayerDataLength = uLength;
	}

	bool success = LoadPackData( m_pMultiplayerPack, pData, uLength );
	InitSubsystems();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if(pLayer)
	{
		pLayer->SetTitleStorageDataActive(true);
		pLayer->GetDataGathering().Init(C_MultiplayerData::Instance().m_pConfig->m_DataGatheringRecords);
	}

	return success;	
}

const bool C_MultiplayerData::ReloadMultiplayerPackData()
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	//Only need to reload if not currently active
	if(!pLayer->GetTitleStorageDataActive())
	{
		return LoadMultiplayerPackData(m_pMultiplayerData, m_MultiplayerDataLength);
	}

	return true;
}

void C_MultiplayerData::PreCacheData()
{
	BIZ_ASSERT(m_pConfig);

	UI::C_UiXtConfig::Instance()->m_TeamAColour.SetByName( m_pConfig->m_TeamAColour );
	UI::C_UiXtConfig::Instance()->m_TeamBColour.SetByName( m_pConfig->m_TeamBColour );

	Game::C_GameProfile* pProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	// Precache vehicles
	m_MultiplayerAbilityMap.Clear();
	m_MultiplayerVehicleMap.Clear();
	m_MultiplayerVehicles.Clear();
	for( int i = 0; i < m_pConfig->m_LevelingData.count; i++ )
	{
		for( int j = 0; j < m_pConfig->m_LevelingData[i].mVehicleUnlocks.count; j++ )
		{
			if(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j].Object())
			{
				m_MultiplayerVehicleMap.Insert(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]->ID, i);
				m_MultiplayerVehicles.PushBack(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]);

				//Boss cars fully upgraded
				if(pProf && m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]->m_ParentVehicle)
				{
					pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER).ForceComplete();
					pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES).ForceComplete();
					pProf->GetMultiplayerProfile()->GetChallengeSystem().ForceCarUpgrades(m_pConfig->m_LevelingData[i].mVehicleUnlocks[j]->ID);
				}
			}
		}
	}

	// Add DLC vehicles
	for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
	{
		AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
		if(cheatBase && cheatBase->GetType() == E_CHEAT_TYPE_VEHICLE && cheatBase->dlcLicensed)
		{
			AmaxCheatVehicle* cheatVehicle = (AmaxCheatVehicle*)cheatBase;

			if(cheatVehicle && cheatVehicle->mAmaxVehicle.Object())
			{
				XtId vehId = cheatVehicle->mAmaxVehicle->ID;

				Btl::Map<U32, int>::InsertionReturnType insResult = m_MultiplayerVehicleMap.Insert(vehId, 0);
				m_MultiplayerVehicles.PushBack(cheatVehicle->mAmaxVehicle);

				//Auto-complete challenge progress
				if(pProf)
				{
					pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(vehId, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER).ForceComplete();
					pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(vehId, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES).ForceComplete();
					pProf->GetMultiplayerProfile()->GetChallengeSystem().ForceCarUpgrades(vehId);
				}

				if(!insResult.inserted)
				{
					insResult.iterator->GetValue() = 0;
				}
			}
		}
	}

	// Check our cheat-unlocked cars
	if(pProf)
	{
		for( Btl::Set<U32>::Iterator it = pProf->mVehicleUnlocks.Begin(); it != pProf->mVehicleUnlocks.End(); it++ )
		{
			XtHandle<AmaxVehicle> vehHandle = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), *it));

			if(vehHandle.Object())
			{
				m_MultiplayerVehicleMap.Insert(vehHandle->ID, 0);
				m_MultiplayerVehicles.PushBack(vehHandle);

				//Auto-complete challenge progress
				pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(vehHandle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER).ForceComplete();
				pProf->GetMultiplayerProfile()->GetChallengeSystem().GetCarChallenge(vehHandle->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES).ForceComplete();
				pProf->GetMultiplayerProfile()->GetChallengeSystem().ForceCarUpgrades(vehHandle->ID);
			}
		}
	}

	// Check Legend cars!
	for( int i = 0; i < m_pConfig->m_LegendLevelingData.count; i++ )
	{
		for( int j = 0; j < m_pConfig->m_LegendLevelingData[i].mVehicleUnlocks.count; j++ )
		{
			if(m_pConfig->m_LegendLevelingData[i].mVehicleUnlocks[j].Object())
			{
				m_MultiplayerVehicleMap.Insert(m_pConfig->m_LegendLevelingData[i].mVehicleUnlocks[j]->ID, i);
				m_MultiplayerVehicles.PushBack(m_pConfig->m_LegendLevelingData[i].mVehicleUnlocks[j]);
			}
		}
	}

	//	Cache the XtIds of any vehicles we don't allow in team events
	for( int i = 0; i < m_pConfig->m_InvalidTeamVehicles.count; i++)
	{
		m_InvalidTeamVehicleIds.PushBack(m_pConfig->m_InvalidTeamVehicles[i]->ID);
	}

	// Cache max rank needed to unlock all playlists / mods / challenge packs
	CacheUnlockRanks(pProf); 
}

// Calculate the ranks required to unlock all playlists, challenge packs, vehiclemods
void C_MultiplayerData::CacheUnlockRanks(Game::C_GameProfile* pProfile)
{
	// Clear old safely
	m_FinalPlaylistUnlockRank = 0;
	m_FinalChallengePackUnlockRank = 0;
	m_FinalModUnlockRank = 0;

	
	// Playlists
	// 2 places we need to check..

	// 1st playlist check - look at min rank in individual playlists
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_OnlineEvents.count; i++ )
	{
		const MultiplayerEvent* pEvent = C_MultiplayerData::Instance().m_pConfig->m_OnlineEvents[i];
		if(m_FinalPlaylistUnlockRank < pEvent->min_rank)
		{
			m_FinalPlaylistUnlockRank = pEvent->min_rank;
		}
	}

	for ( S32 rank = 0; rank < C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count; rank++ )
	{
		MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[rank];

		// Mods
		if( levelData.mAbilityUnlocks.count > 0 )
		{
			// Then we unlock some abilites at this rank..
			m_FinalModUnlockRank = rank;  //[0,49]
		}

		// Challenge packs
		if( levelData.mChallengeUnlocks.count > 0 )
		{
			// Then we unlock some challenges at this rank..
			m_FinalChallengePackUnlockRank = rank; //[0,49]
		}

		// 'Playlists' cont...
		// 2nd playlist check - look at each game mode which houses a group of playlists. If you havent unlocked a modes.. you cant
		// have unlocked its playlist (even though tecnhically that playlist may have a rank requirement of 0)
		for( S32 k = 0; k < levelData.mFeatureUnlocks.count; k++)
		{
			if(levelData.mFeatureUnlocks[k].Object()->mIsPlaylist)
			{
				if(m_FinalPlaylistUnlockRank < rank)
				{
					m_FinalPlaylistUnlockRank = rank;
				}
				break;
			}
		}
	}
}


void C_MultiplayerData::InitSubsystems()
{
	for (S32 iProfile = 0; iProfile < Game::MAX_GAME_PROFILES; iProfile++)
	{
		if (Game::C_GameProfileManager::Instance().GetProfile(iProfile) && Game::C_GameProfileManager::Instance().GetProfile(iProfile)->GetMultiplayerProfile())
		{
			Game::C_GameProfileManager::Instance().GetProfile(iProfile)->ReloadSorceryData();
			Game::C_GameProfileManager::Instance().GetProfile(iProfile)->GetMultiplayerProfile()->GetChallengeSystem().ReloadSorceryData();
		}
	}

	C_GameApp::GetInstance()->GetGame()->GetMultiplayerAbilities().Init();
	
	Game::gGameModeStore( Game::E_GameMode_MultiPlayer )->Initialise();

	PreCacheData();
}

//	Returns TRUE if the given vehicle can be used in team events.
const bool C_MultiplayerData::ValidateVehicleForTeams( U32 vehicleXtId )
{
	for(int i = 0; i < m_InvalidTeamVehicleIds.Size(); i++)
	{
		if(vehicleXtId == m_InvalidTeamVehicleIds[i])
			return false;
	}
	return true;
}


