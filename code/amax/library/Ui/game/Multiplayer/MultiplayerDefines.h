
#ifndef MULTIPLAYER_DEFINES_H
#define MULTIPLAYER_DEFINES_H

namespace UI
{

#define EVENT_SERVER_VERSION	4
#define INVALID_EVENT_ID		0xffffffffffffffff
#define MAX_USER_EVENTS			5
#define MAX_EVENT_CACHE			20 // If you increase this make sure that you update the IMP file
#define MAX_OWNER_NAME			17 // PS3 has 16 actual characters for gamertags
#define MAX_EVENT_NAME			17
#define MAX_EVENT_DESC			32 // users actually have 16(17=\0) chars but we need this size for ranked events since we use spaces and can support line wrapping
#define MAX_EVENT_RACES			6

typedef U64 TEventID;		// lsp wont be writing back to these

enum E_RaceType
{
	E_RaceType_Racing = 0,
	E_RaceType_Elimination,
	E_RaceType_Points,
	E_RaceType_Destruction,
	E_RaceType_Hardcore,
	
	E_RaceType_Max,
};

enum E_RaceModeFlag
{
	E_RaceModeFlag_Racing		= E_RaceType_Racing,
	E_RaceModeFlag_Elimination	= E_RaceType_Elimination,
	E_RaceModeFlag_Points		= E_RaceType_Points,
	E_RaceModeFlag_Destruction	= E_RaceType_Destruction,
};

enum E_EliminationEvent
{
	E_EliminationEvent_LapFirst = 0,
	E_EliminationEvent_LapSecondLast,
	E_EliminationEvent_Time,
	E_EliminationEvent_Wrecked,
};

enum E_EliminationType
{
	E_EliminationType_LastPlace = 0,
	E_EliminationType_FirstPlace,
	E_EliminationType_LeastHealth,
	E_EliminationType_MostHealth,
	E_EliminationType_LeastFans,
	E_EliminationType_MostFans,
};

enum E_MpFinishType
{
	E_MpFinishType_Laps = 0,
	E_MpFinishType_Time,
	E_MpFinishType_PointsTarget,
};

enum E_Pickup
{
	E_Pickup_Nitro = 0,
	E_Pickup_Shock,
	E_Pickup_Barge,
	E_Pickup_Mine,
	E_Pickup_Shunt,
	E_Pickup_Bolt,
	E_Pickup_Shield,
	E_Pickup_Repair,

	E_Pickup_Max,
};

enum E_VehicleClass
{
	E_VehicleClass_A = 0,
	E_VehicleClass_B,
	E_VehicleClass_C,
	E_VehicleClass_D,
	E_VehicleClass_E,
	E_VehicleClass_Any,

	E_VehicleClass_Max = E_VehicleClass_E,
};

enum E_DamageMode
{
	E_DamageMode_Off = 0,
	E_DamageMode_Low,
	E_DamageMode_Medium,
	E_DamageMode_High,
	E_DamageMode_OneHit,

	E_DamageMode_Count,
};

enum E_RandomPowerups
{
	E_RandomPowerups_Off= 0,
	E_RandomPowerups_On,
};

enum E_PowerupRespawnMode
{
	E_PowerupRespawnMode_Normal= 0,
	E_PowerupRespawnMode_AlwaysOn,
	E_PowerupRespawnMode_Long,
	E_PowerupRespawnMode_VeryLong,
	E_PowerupRespawnMode_None,
};

enum E_PowerupMods
{
	E_PowerupMods_Off,
	E_PowerupMods_On,
};

enum E_CarUpgrades
{
	E_CarUpgrades_Off= 0,
	E_CarUpgrades_On,
};

enum E_AiMode
{
	E_AiOff,
	E_AiEasy,
	E_AiMedium,
	E_AiHard,
};

enum E_LastManStanding
{
	E_LastManStanding_Off= 0,
	E_LastManStanding_On,
};

enum E_Handicap
{
	E_HandicapOff,
	E_HandicapLow,
	E_HandicapMedium,
	E_HandicapHigh,
};

enum E_MiscSettings
{
	E_MiscSettings_Voting = 0,
	E_MiscSettings_HandOfGod,
	E_MiscSettings_Abilities,
	E_MiscSettings_RandomCars,
	E_MiscSettings_ShufflePowerupOnRespawn,
	E_MiscSettings_RandomMods,
	E_MiscSettings_AddAI,
};

// NOTE : ENSURE THESE E_ScoringType ENUMS MATCH ORDER GIVEN IN MULTIPLAYER.TXT SORCERY SCHEMA, 
//        AND MATCH ORDER OF SCORING RULES AS CREATED IN MULTIPLAYERCONFIG SCORING TAB IN SORCERY!.
//		  THINGS WILL BREAK IF THEY DONT MATCH :( - Jon Bunner 
enum E_ScoringType
{
	E_ScoringType_HitByPickup_Shunt = 0,
	E_ScoringType_HitByPickup_Shock,
	E_ScoringType_HitByPickup_Barge,
	E_ScoringType_HitByPickup_Bomb,
	E_ScoringType_Impact_Heavy,
	E_ScoringType_Impact_Medium,
	E_ScoringType_Impact_Light,
	E_ScoringType_HitByPickup_Level2Shunt,
	E_ScoringType_HitByPickup_Level2Shock,
	E_ScoringType_HitByPickup_Level2Barge,
	E_ScoringType_HitByPickup_Level2Bomb,
	E_ScoringType_Double_Barge,
	E_ScoringType_Drifting, 
	E_ScoringType_Air,
	E_ScoringType_Shunt_Overtake,
	E_ScoringType_Nitrous_Overtake,
	E_ScoringType_Shock_Overtake,
	E_ScoringType_Skill_Shot_Nitrous,
	E_ScoringType_Battering_Ram,
	E_ScoringType_Crossed_Line_First,
	E_ScoringType_Picked_Up_Shunt,
	E_ScoringType_Picked_Up_Barge,
	E_ScoringType_Picked_Up_Repair,
	E_ScoringType_Picked_Up_Nitrous,
	E_ScoringType_Picked_Up_Shock,
	E_ScoringType_Picked_Up_Bomb,

	E_ScoringType_Count,
};

enum E_EventFilter
{
	E_EventFilter_Bizarre = 0,
	E_EventFilter_QuickRace,
	E_EventFilter_UserCreated,
	E_EventFilter_Favourties,
	E_EventFilter_RecentlyPlayed,
	E_EventFilter_HighestRated,
	E_EventFilter_Newest,

	E_EventFilter_Max,
};

enum E_LobbyState
{
	E_LobbyState_FindingPlayers = 0,
	E_LobbyState_Waiting,		
	E_LobbyState_WaitingToVote,
	E_LobbyState_Voting,
	E_LobbyState_Countdown,
	E_LobbyState_TeamCountdown,
	E_LobbyState_TeamBalanceWait,
	E_LobbyState_WaitingForHost,
	E_LobbyState_Ready,
};

enum E_MultiplayerVehicleUpgrades
{
	E_MultiplayerVehicleUpgrades_None			= 0,

	E_MultiplayerVehicleUpgrades_HealthSlot,
	E_MultiplayerVehicleUpgrades_FanBonus,
	E_MultiplayerVehicleUpgrades_SpeedBoost,
	E_MultiplayerVehicleUpgrades_Metallic,
	E_MultiplayerVehicleUpgrades_Pearlescent,
	E_MultiplayerVehicleUpgrades_Candy,

	E_MultiplayerVehicleUpgrades_Num		= 6,
};

enum E_ChallengePack
{
	E_ChallengePack_Ace = 0,
	E_ChallengePack_Car,
	E_ChallengePack_Daily,
	E_ChallengePack_Demolition,
	E_ChallengePack_DriversED,
	E_ChallengePack_Specialist,

	E_ChallengePack_Count,	// Keep at back for counter

	E_ChallengePack_None = -1,
};

enum E_MULTIPLAYER_CHALLENGE_TYPE
{
	E_MULTIPLAYER_CHALLENGE_TYPE_NONE = -1,

	//Ace challenges
	E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK,
	E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME,
	E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER,
	E_MULTIPLAYER_CHALLENGE_TYPE_SIDEWINDER,
	E_MULTIPLAYER_CHALLENGE_TYPE_FTL,
	E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE,
	E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE,
	E_MULTIPLAYER_CHALLENGE_TYPE_MASTER_EXPLODER,
	E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW,
	E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS,
	E_MULTIPLAYER_CHALLENGE_TYPE_SKEET_SHOOTER,
	E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY,
	E_MULTIPLAYER_CHALLENGE_TYPE_UNTOUCHABLE,
								 
	//Car challenges			 
	E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES,
	E_MULTIPLAYER_CHALLENGE_TYPE_WINNER,
								 
	//Daily challenges			 
	E_MULTIPLAYER_CHALLENGE_TYPE_OVERTAKE,
	E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT,
	E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK,
	E_MULTIPLAYER_CHALLENGE_TYPE_DEMOLITIONS_EXPERT,
	E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK,
	E_MULTIPLAYER_CHALLENGE_TYPE_STYLE,
	E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT,
	E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER,
	E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS,
	E_MULTIPLAYER_CHALLENGE_TYPE_HOARDER,
	E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION,
	E_MULTIPLAYER_CHALLENGE_TYPE_AGGRESSIVE_PERSONALITY,
	E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR,
	E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE,
	E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY,
	E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED,
								 
	//Demolition challenges		 
	E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION,
	E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER,
	E_MULTIPLAYER_CHALLENGE_TYPE_WRECKING_BALL,
	E_MULTIPLAYER_CHALLENGE_TYPE_BRUISER,
	E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE,
	E_MULTIPLAYER_CHALLENGE_TYPE_BRUTAL_DELUXE,
	E_MULTIPLAYER_CHALLENGE_TYPE_VENDETTA,
	E_MULTIPLAYER_CHALLENGE_TYPE_BRUTE_FORCE,
	E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT,
	E_MULTIPLAYER_CHALLENGE_TYPE_BATTERING_RAM,
	E_MULTIPLAYER_CHALLENGE_TYPE_SCRAMBLER,
	E_MULTIPLAYER_CHALLENGE_TYPE_LAST_GASP,
	E_MULTIPLAYER_CHALLENGE_TYPE_DECEIVER,
								 
	//Driver's Ed. challenges	 
	E_MULTIPLAYER_CHALLENGE_TYPE_BARGE,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHIELD,
	E_MULTIPLAYER_CHALLENGE_TYPE_MINE,
	E_MULTIPLAYER_CHALLENGE_TYPE_NITRO,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT,
	E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT_EVADER,
	E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK_EVADER,
	E_MULTIPLAYER_CHALLENGE_TYPE_MINESWEEPER,
	E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_SHUNT,
	E_MULTIPLAYER_CHALLENGE_TYPE_AIRBRAKE,
	E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_BLIND_FIRE,
	E_MULTIPLAYER_CHALLENGE_TYPE_MINE_LOBBER,
	E_MULTIPLAYER_CHALLENGE_TYPE_REAR_GUARD,
								 
	//Specialist challenges		 
	E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST,
	E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN,
	E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST,
	E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST,
	E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST,
	E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE_SPECIALIST,
	E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING,
	E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER,
	E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER,

	E_MULTIPLAYER_CHALLENGE_TYPE_ELF_EYES,
	E_MULTIPLAYER_CHALLENGE_TYPE_SAFETY_NET,
	E_MULTIPLAYER_CHALLENGE_TYPE_ROAD_SWEEPER,
	E_MULTIPLAYER_CHALLENGE_TYPE_DRIFTER,
	E_MULTIPLAYER_CHALLENGE_TYPE_MASTERMINE,
	E_MULTIPLAYER_CHALLENGE_TYPE_STEALTH,
	E_MULTIPLAYER_CHALLENGE_TYPE_BULLY,
	E_MULTIPLAYER_CHALLENGE_TYPE_FULLY_BOLT,
	E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RAM,
	E_MULTIPLAYER_CHALLENGE_TYPE_RIVALRY,
	E_MULTIPLAYER_CHALLENGE_TYPE_LIGHT_WEIGHT,
	E_MULTIPLAYER_CHALLENGE_TYPE_NITRO_RIFT,

	E_MULTIPLAYER_CHALLENGE_TYPE_MAX
};



enum E_MultiplayerVehicleAbilities
{
	E_MultiplayerVehicleAbilities_None = -1,

	E_MultiplayerVehicleAbilities_Iron_Fist = 0,
	E_MultiplayerVehicleAbilities_Jump_The_Gun,
	E_MultiplayerVehicleAbilities_Front_Runner,
	E_MultiplayerVehicleAbilities_Drifter,

	E_MultiplayerVehicleAbilities_Titanium_Armour,
	E_MultiplayerVehicleAbilities_Showy_Flourish,
	E_MultiplayerVehicleAbilities_Stable_Frame,
	E_MultiplayerVehicleAbilities_Battering_Ram,

	E_MultiplayerVehicleAbilities_Decoy_Drop,
	E_MultiplayerVehicleAbilities_Road_Sweeper,
	E_MultiplayerVehicleAbilities_Scrambler,
	E_MultiplayerVehicleAbilities_Splash_Damage,

	E_MultiplayerVehicleAbilities_Shielding_Efficiency,
	E_MultiplayerVehicleAbilities_Safety_Net,
	E_MultiplayerVehicleAbilities_Shielded_Boosters,
	E_MultiplayerVehicleAbilities_Shielded_Bay,

	E_MultiplayerVehicleAbilities_ECM,
	E_MultiplayerVehicleAbilities_Vampiric_Wreck,
	E_MultiplayerVehicleAbilities_Bribe,
	E_MultiplayerVehicleAbilities_Fan_Favourite,

	E_MultiplayerVehicleAbilities_Laser_Sight,
	E_MultiplayerVehicleAbilities_Silent_Running,
	E_MultiplayerVehicleAbilities_Last_Gasp,
	E_MultiplayerVehicleAbilities_Mastermine,

	E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes,
	E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots,
	E_MultiplayerVehicleAbilities_Boss_DoubleShield,
	E_MultiplayerVehicleAbilities_Boss_FansNitroGift,
	E_MultiplayerVehicleAbilities_Boss_NitroRift,
	E_MultiplayerVehicleAbilities_Boss_RepairBonus,
	E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire,
	E_MultiplayerVehicleAbilities_Boss_ShuntDamage,

	// these aren't currently used - need to be at the bottom so textures still line up
	E_MultiplayerVehicleAbilities_Advanced_Radar,

	E_MultiplayerVehicleAbilities_NumberOfAbilities,
};

enum E_MultiplayerAbilityCategory
{
	E_MultiplayerAbilityCategory_Slot1, 
	E_MultiplayerAbilityCategory_Slot2,
	E_MultiplayerAbilityCategory_Slot3,

	E_MultiplayerAbilityCategory_Max
};

enum E_MULTIPLAYER_LOADOUT_SLOT
{
	E_MULTIPLAYER_LOADOUT_SLOT1,
	E_MULTIPLAYER_LOADOUT_SLOT2,
	E_MULTIPLAYER_LOADOUT_SLOT3,
	E_MULTIPLAYER_LOADOUT_SLOT_MAX,
};

enum E_MULTIPLAYER_LOADOUT_TYPE
{
	E_MULTIPLAYER_LOADOUT_TYPE_NONE = -1,

	E_MULTIPLAYER_LOADOUT_TYPE_DEFAULT1, 

	E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM1, 
	E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM2, 
	E_MULTIPLAYER_LOADOUT_TYPE_CUSTOM3, 

	E_MULTIPLAYER_LOADOUT_TYPE_MAX,
};

enum E_MultiplayerStat
{
	E_MultiplayerStat_RaceTime,							
	E_MultiplayerStat_FastestLap,						
	E_MultiplayerStat_SlowestLap,						
	E_MultiplayerStat_Fans,								
	E_MultiplayerStat_HealthRemaining,					
	E_MultiplayerStat_LogicPositionCombined,			
	E_MultiplayerStat_DistanceTravelled,				
	E_MultiplayerStat_LapsCompleted,					
	E_MultiplayerStat_OnTrackPoints,					
	E_MultiplayerStat_EliminationPoints,
	E_MultiplayerStat_PowerupHits,						
	E_MultiplayerStat_HitsByPowerups,					
	E_MultiplayerStat_KillStreak,						
	E_MultiplayerStat_DeathStreak,
	E_MultiplayerStat_Accuracy,
	E_MultiplayerStat_LongestChain,
	E_MultiplayerStat_PowerupsEvaded,
	E_MultiplayerStat_TopSpeed,
	E_MultiplayerStat_Car2CarAggressiveHits,
	E_MultiplayerStat_Car2CarVictimHits,
	E_MultiplayerStat_EnvironmentalCrashes,
	E_MultiplayerStat_PickupsCollected,
	E_MultiplayerStat_LongestDrift,
	E_MultiplayerStat_TimesInWater,
	E_MultiplayerStat_TimesOffEdge,
	E_MultiplayerStat_DamageDealt,						
	E_MultiplayerStat_LongestTimeWithoutBeingWrecked,
	E_MultiplayerStat_TotalCarsWrecked,
	E_MultiplayerStat_TimesHit,
	E_MultiplayerStat_MinesFragmentedWithMastermine,
	E_MultiplayerStat_TimeInvisibleWithSilentRunning,
	E_MultiplayerStat_PowerupsAbsorbedWithSafetyNet,
	E_MultiplayerStat_PlayersScrambled,
	E_MultiplayerStat_PlayersHitwithDecoys,
	E_MultiplayerStat_CarsBatteringRammed,
	E_MultiplayerStat_TimesProtectedByStableFrame,
	E_MultiplayerStat_PlayersHitWithIronFist,
	E_MultiplayerStat_PowerupsDropped,
	E_MultiplayerStat_LongestTimeNotDamaged,
	E_MultiplayerStat_TimeInFirstPlace,
	E_MultiplayerStat_LongestJump,
	E_MultiplayerStat_PlayersHitWithBarge,
	E_MultiplayerStat_NitroOvertakes,
	E_MultiplayerStat_BlockedPowerups,
	E_MultiplayerStat_PowerupsShielded,
	E_MultiplayerStat_PlayersHitWithMine,
	E_MultiplayerStat_PlayersHitWithShunt,
	E_MultiplayerStat_PlayersHitWithBolt,

	E_MultiplayerStat_Count,
};

enum E_MultiplayerAward
{
	E_MultiplayerAward_None			= -1,
	E_MultiplayerAward_SpeedDemon,
	E_MultiplayerAward_Tortoise,
	E_MultiplayerAward_FanTastic,
	E_MultiplayerAward_OldNews,
	E_MultiplayerAward_Marksman,
	E_MultiplayerAward_TriggerHappy,
	E_MultiplayerAward_Squirrel,
	E_MultiplayerAward_Gladiator,
	E_MultiplayerAward_Drifter,
	E_MultiplayerAward_CrashTestDummy,
	E_MultiplayerAward_BreakNeck,
	E_MultiplayerAward_Agile,
	E_MultiplayerAward_Brawler,
	E_MultiplayerAward_CleanRacer,
	E_MultiplayerAward_WreckingBall, // New name needed
	E_MultiplayerAward_Survivor,
	E_MultiplayerAward_Relentless,
	E_MultiplayerAward_WriteOff,
	E_MultiplayerAward_PunchingBag,
	E_MultiplayerAward_DeepSeaDiver,
	E_MultiplayerAward_Cliffhanger,
	E_MultiplayerAward_Environmentalist,

	E_MultiplayerAward_Mastermine,
	E_MultiplayerAward_SilentRunning,
	E_MultiplayerAward_SafetyNet,
	E_MultiplayerAward_Scrambler,
	E_MultiplayerAward_DecoyDrop,
	E_MultiplayerAward_Batterer,
	E_MultiplayerAward_StableFrame,
	E_MultiplayerAward_IronFist,
	E_MultiplayerAward_ButterFingers,
	E_MultiplayerAward_Dexterous,
	E_MultiplayerAward_Dominator,
	E_MultiplayerAward_LongJumper,
	E_MultiplayerAward_CloseEncounters,
	E_MultiplayerAward_Booster,
	E_MultiplayerAward_SafeHands,
	E_MultiplayerAward_Defender,
	E_MultiplayerAward_LitterBug,
	E_MultiplayerAward_CrimsonCarapace,
	E_MultiplayerAward_Hunter,

	E_MultiplayerAward_Count,
};

enum E_MultiplayerAwardCategory
{
	E_MultiplayerAwardCategory_None = -1,

	E_MultiplayerAwardCategory_Award,
	E_MultiplayerAwardCategory_WoodenSpoon,

	E_MultiplayerAwardCategory_Count,
};

enum E_MultiplayerStatSortingStrategy
{
	E_MultiplayerStatSortingStrategy_HighestWins,
	E_MultiplayerStatSortingStrategy_LowestWins,

	E_MultiplayerStatSortingStrategy_Count,

};

enum SkillshotType
{
	E_SKILLSHOT_DRIFTING,
	E_SKILLSHOT_AIR,
	E_SKILLSHOT_ANY,
};

enum E_GroupGenre
{
	E_GroupGenre_Racing = 0,
	E_GroupGenre_Elimination,
	E_GroupGenre_Points,
	E_GroupGenre_Destruction,

	E_GroupGenre_Mixed, // used as counter so keep at back
};

enum E_GroupAccess
{
	E_GroupAccess_Private = 0,
	E_GroupAccess_Friends,
	E_GroupAccess_Public,
};

enum E_MultiplayerFeature
{
	E_MultiplayerFeature_Default = 0,
	E_MultiplayerFeature_ModShop,
	E_MultiplayerFeature_ResprayCar,

	E_MultiplayerFeature_Count
};

}; // namespace UI

#endif // MULTIPLAYER_DEFINES_H
