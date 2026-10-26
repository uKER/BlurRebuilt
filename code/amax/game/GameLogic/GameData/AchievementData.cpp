#include "stdafx.h"

#include "AchievementData.h"

#define AMAX_ACHIEVEMENTDATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_AchievementData::Instance().Member; } };

C_AchievementData C_AchievementData::s_instance;

XT_TYPE(StickerData, "Sticker")
	XT_FIELD(mDescription,							"Description")
	XT_FIELD(mShortDescription,						"ShortDescription")
	XT_FIELD(mType,									"Type")
	XT_FIELD(mGoal,									"Goal")
	XT_FIELD(mOneShot,								"OneShot")
	XT_FIELD(mQuadTopLeft,							"QuadrantTopLeft")
	XT_FIELD(mQuadTopRight,							"QuadrantTopRight")
	XT_FIELD(mQuadBottomLeft,						"QuadrantBottomLeft")
	XT_FIELD(mQuadBottomRight,						"QuadrantBottomRight")
XT_END_TYPE

XT_TYPE(AchievementData, "Achievement")
	XT_FIELD(mName,									"Name")
	XT_FIELD(mType,									"Type")
	XT_FIELD(mCategory,								"Category")
	XT_FIELD(mTextureIndex,							"TextureIndex")
	XT_FIELD(mSinglePlayer,							"CanCompleteInSinglePlayer")
	XT_FIELD(mMultiplayer,							"CanCompleteInMultiplayer")
	XT_FIELD(mSplitScreen,							"CanCompleteInSplitScreen")
	XT_FIELD(mDisableOnPS3,							"DisableOnPS3")
	XT_FIELD(mStickers,								"Stickers")
XT_END_TYPE

XT_TYPE(MiscStickerData, "MiscStickerData")
	XT_FIELD(mRouteForCheckpointGoldWithoutDamage, "RouteForCheckpointGoldWithoutDamage")

	XT_FIELD(mChallengeX,		"challenge_x")
	XT_FIELD(mChallengeY,		"challenge_y")

	XT_FIELD(mMaxPhotoFinishWinTimeDifference,	"MaxPhotoFinishWinTimeDifference")

	XT_FIELD(mMaxTimeBetweenBargingGuyAndOffCliffScoringMessage,"MaxTimeBetweenBargingGuyAndOffCliffScoringMessage")

	XT_FIELD(mNumBoltsNeededToHitWhileAirbraking, "NumBoltsNeededToHitWhileAirbraking")

	XT_FIELD(mMaxTimeForBombToStopIncomingAttack, "MaxTimeForBombToStopIncomingAttack")
	XT_FIELD(mMaxTimeForShieldPowerupToBeCloseRange, "MaxTimeForShieldPowerupToBeCloseRange")
	XT_FIELD(mMaxTimeForPowerupHitAfterCriticalRepair, "MaxTimeForPowerupHitAfterCriticalRepair")

	XT_FIELD(mSmokingOilfieldsCity,				"SmokingOilfieldsCity")
	XT_FIELD(mGasStationCity,					"GasStationCity")
	XT_FIELD(mHollywoodHillsSignCity,			"HollywoodHillsSignCity")
	XT_FIELD(mFireworksCity,					"FireworksCity")

	XT_FIELD(mMaxFireworksViewingAngle,			"MaxAngleToViewFireworks")
	XT_FIELD(mFireworksViewingVehicle,			"FireworksVehicle")

	XT_FIELD(mClassAHighSpeedCity,				"ClassAHighSpeedCity")
	XT_FIELD(mClassBHighSpeedCity,				"ClassBHighSpeedCity")
	XT_FIELD(mClassCHighSpeedCity,				"ClassCHighSpeedCity")
	XT_FIELD(mClassDHighSpeedCity,				"ClassDHighSpeedCity")

	XT_FIELD(mAllCitiesInTheGame,				"AllCitiesInTheGame")

	XT_FIELD(mEventForShockLeonard,				"EventForShockLeonard")
	XT_FIELD(mEventForSmashKhansRide,			"EventForSmashKhansRide")
	XT_FIELD(mEventForNatalyaAvoidance,			"EventForNatalyaAvoidance")
	XT_FIELD(mEventForCarverDamage,				"EventForCarverDamage")

	XT_FIELD(mAllPinkPaintJobs,					"WhatIsPink")

	XT_FIELD(mSuccessfulPowerUpHitOnEveryCarTier,	"SuccessfulPowerUpHitOnEveryCarTier")
	XT_FIELD(mSuccessfulPowerUpHitOnEveryCarEvent,	"SuccessfulPowerUpHitOnEveryCarEvent")
	XT_FIELD(mAvoidOwnShockTier,					"AvoidOwnShockTier")
	XT_FIELD(mAvoidOwnShockEvent,					"AvoidOwnShockEvent")

	XT_FIELD(mTimeLobbedConsideredRecentlyComeToRest, "TimeLobbedConsideredRecentlyComeToRest")
XT_END_TYPE

AMAX_ACHIEVEMENTDATA_CALLBACK(AchievementsList, mAchievementsList)
XT_TYPE(AchievementsList, "AchievementsList")
	XT_CALLBACK((GameDataHandler<AchievementsList,CallbackAchievementsList>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(mAchievements,							"Achievements")
	XT_FIELD(mMiscStickerData,						"StickerData")
XT_END_TYPE


XT_TYPE(Game::BLUR_STICKER, "BlurSticker")
	XT_OPTION( Game::BLUR_STICKER_WIN_ANY_PLAYLIST,						"WinAnyPlaylist" )			// AllRounder
	XT_OPTION( Game::BLUR_STICKER_WIN_HARDCORE_RACING,					"WinHardcoreRacing" )
	XT_OPTION( Game::BLUR_STICKER_WIN_TEAM,								"WinChampionship" )
																		
	XT_OPTION( Game::BLUR_STICKER_BARGE_CARS,							"BargeCars" )				// BargeMaster
	XT_OPTION( Game::BLUR_STICKER_BARGE_OFF_WATER_OR_CLIFF,				"BargeOffWaterOrCliff" )
	XT_OPTION( Game::BLUR_STICKER_BARGE_INCOMING_ATTACK,				"BargeIncomingAttack" )
	XT_OPTION( Game::BLUR_STICKER_BARGE_THREE_AT_ONCE,					"BargeThreeAtOnce" )
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_BIZ_DEV,							"BeatBizDev" )				// BeatBizarre
																		
	XT_OPTION( Game::BLUR_STICKER_FIRST_FRIEND_DEMAND,					"FirstFriendDemand" )		// BetterEnemies (Friends make better enemies)
	XT_OPTION( Game::BLUR_STICKER_ACCEPT_FRIEND_DEMAND,					"AcceptFriendDemand" )
	XT_OPTION( Game::BLUR_STICKER_BEAT_FRIEND_DEMAND,					"BeatFriendDemand" )
	XT_OPTION( Game::BLUR_STICKER_SEND_CHALLENGE_BACK,					"SendChallengeBack" )
																		
	XT_OPTION( Game::BLUR_STICKER_WIN_SPLIT_SCREEN,						"WinSplitScreen" )			// BlurParty
																		
	XT_OPTION( Game::BLUR_STICKER_THREE_BOLT_WRECK,						"ThreeBoltWreck" )			// BoltMaster
	XT_OPTION( Game::BLUR_STICKER_FOUR_BOLT_HIT,						"FourBoltHit" )
	XT_OPTION( Game::BLUR_STICKER_BOLT_INCOMING_POWERUPS,				"BoltIncomingPowerups" )
	XT_OPTION( Game::BLUR_STICKER_THREE_BOLT_DIFFERENT,					"ThreeBoltDifferent" )
																		
	XT_OPTION( Game::BLUR_STICKER_WRECK_FIRST_BOSS,						"WreckFirstBoss" )			// BreakYou
																		
	XT_OPTION( Game::BLUR_STICKER_MAXI_LEADERBOARD_FRIEND,				"MaxiLeaderboardFriend" )	// ChallengeMaster
																		
	XT_OPTION( Game::BLUR_STICKER_CHALLENGE_PACK_FULL_DONE,				"ChallengePackFullDone" )	// Challenger
	XT_OPTION( Game::BLUR_STICKER_CHALLENGE_PACK_THREE_HALF_DONE,		"ChallengePackThreeHalfDone" )
	XT_OPTION( Game::BLUR_STICKER_CHALLENGE_X_DONE,						"ChallengeXDone" )
	XT_OPTION( Game::BLUR_STICKER_CHALLENGE_Y_DONE,						"ChallengeYDone" )
																		
	XT_OPTION( Game::BLUR_STICKER_STOPWATCH_POWERUPS,					"StopwatchPowerups" )		// CheckpointMaster
	XT_OPTION( Game::BLUR_STICKER_NITRO_EVERY_CHECKPOINT,				"NitroEveryCheckpoint" )
	XT_OPTION( Game::BLUR_STICKER_GOLD_WITHOUT_NITRO,					"GoldWithoutNitro" )
	XT_OPTION( Game::BLUR_STICKER_GOLD_WITHOUT_STOPWATCH,				"GoldWithoutStopwatch" )
																		
	XT_OPTION( Game::BLUR_STICKER_EVERY_PRIMARY_LIGHT,					"EveryPrimaryLight" )		// Completist
	XT_OPTION( Game::BLUR_STICKER_EVERY_FAN_PAR,						"EveryFanPar" )
	XT_OPTION( Game::BLUR_STICKER_EVERY_LIGHT,							"EveryLight" )
																		
	XT_OPTION( Game::BLUR_STICKER_FIRST_FAN_RUN,						"FirstFanRun" )				// ConeGates
																		
	XT_OPTION( Game::BLUR_STICKER_FIRST_AWARD,							"FirstAward" )				// Dominator
	XT_OPTION( Game::BLUR_STICKER_WIN_TWO_AWARDS,						"WinTwoAwards" )
	XT_OPTION( Game::BLUR_STICKER_WIN_THREE_AWARDS,						"WinThreeAwards" )
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_RHYMER,							"BeatRhymer" )				// FanBoy
																		
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_FAN_DEMANDS,					"CompleteFanDemands" )		// FanDemandMaster
	XT_OPTION( Game::BLUR_STICKER_ALL_FAN_DEMANDS_IN_EVENT,				"AllFanDemandsInEvent" )
																		
	XT_OPTION( Game::BLUR_STICKER_UNLOCK_SHANNON,						"UnlockShannon" )			// FirstTest
																		
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_FAN_RUNS,					"CompleteFanRuns" )			// ForTheFans
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_ALL_FAN_RUNS,				"CompleteAllFanRuns" )
	XT_OPTION( Game::BLUR_STICKER_THROUGH_FAN_GATES,					"ThroughFanGates" )
																		
	XT_OPTION( Game::BLUR_STICKER_PHOTO_FINISH,							"PhotoFinish" )	// Friendly Rivalry
																		
	XT_OPTION( Game::BLUR_STICKER_FIRST_FAN_PAR,						"FirstFanPar" )				// GettingStarted
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_A_RANK_FIFTY,					"BeatARankFifty" )			// GiantKiller
																		
	XT_OPTION( Game::BLUR_STICKER_RACE_EACH_LOCATION,					"RaceEachLocation" )		// Globetrotter
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_CARVER,							"BeatCarver" )				// GoingGrey
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_SHANNON,							"BeatShannon" )				// LearningTheRopes
																		
	XT_OPTION( Game::BLUR_STICKER_DAMAGE_CARVER_MORE,					"DamageCarverMore" )		// LightsOutMeatball
	XT_OPTION( Game::BLUR_STICKER_AVOID_NATALYAS_POWERUPS,				"AvoidNatalyasPowerups" )
	XT_OPTION( Game::BLUR_STICKER_SMASH_KHAN,							"SmashKhan" )
	XT_OPTION( Game::BLUR_STICKER_SHOCK_LEONARD,						"ShockLeonard" )
																		
	XT_OPTION( Game::BLUR_STICKER_HARUNA_GOLD_NO_DAMAGE,				"HarunaGoldNoDamage" )		// MasterOfTheMachine
																		
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_GAME_HARD,					"CompleteGameHard" )		// MeetTheMitchells
																		
	XT_OPTION( Game::BLUR_STICKER_MINE_CARS,							"MineCars" )				// MineMaster
	XT_OPTION( Game::BLUR_STICKER_MINE_WITH_LOBBED,						"MineWithLobbed" )
	XT_OPTION( Game::BLUR_STICKER_DESTROY_MINES,						"DestroyMines" )
	XT_OPTION( Game::BLUR_STICKER_MINE_INCOMING_ATTACKS,				"MineIncomingAttacks" )
																		
	XT_OPTION( Game::BLUR_STICKER_CUSTOMISE_A_LOADOUT,					"CustomiseALoadout" )		// Modder
	XT_OPTION( Game::BLUR_STICKER_WIN_WITH_CUSTOM_LOADOUT,				"WinWithCustomLoadout" )
																		
	XT_OPTION( Game::BLUR_STICKER_WIN_MOTOR_MASH,						"WinMotorMash" )			// MotorMan
	XT_OPTION( Game::BLUR_STICKER_WRECK_RIVAL,							"WreckRival" )
	XT_OPTION( Game::BLUR_STICKER_POINTS_IN_ONE_MASH,					"PointsInOneMash" )
																		
	XT_OPTION( Game::BLUR_STICKER_NITRO_OVERTAKE_CARS,					"NitroOvertakeCars" )		// NitroMaster
	XT_OPTION( Game::BLUR_STICKER_AIRBRAKE_RIFT_CARS,					"AirbrakeRiftCars" )
	XT_OPTION( Game::BLUR_STICKER_AIRBRAKE_AND_BOLT,					"AirbrakeAndBolt" )
	XT_OPTION( Game::BLUR_STICKER_WIN_AFTER_AIRBRAKES,					"WinAfterAirbrakes" )
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_KHAN,							"BeatKhan" )				// NotSoPrettyBoy
																		
	XT_OPTION( Game::BLUR_STICKER_RESPRAY_A_CAR,						"ResprayACar" )				// PaintJob
	XT_OPTION( Game::BLUR_STICKER_TOP_THREE_IN_PINK,					"TopThreeInPink" )
	XT_OPTION( Game::BLUR_STICKER_RESPRAY_WITH_UNLOCKED,				"ResprayWithUnlocked" )

	XT_OPTION( Game::BLUR_STICKER_USE_EACH_POWERUP,						"UseEachPowerup" )			// RaceMaster
	XT_OPTION( Game::BLUR_STICKER_LAP_WITHOUT_GETTING_HIT,				"LapWithoutGettingHit" )
	XT_OPTION( Game::BLUR_STICKER_WIN_SINGLE_PLAYER_RACES,				"WinSinglePlayerRaces" )
	XT_OPTION( Game::BLUR_STICKER_HIT_EVERY_CAR,						"HitEveryCar" )
																		
	XT_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_PLAYLISTS,					"UnlockAllPlaylists" )		// RankFifty
	XT_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_CHALLENGE_PACKS,			"UnlockAllChallengePacks" )
	XT_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_MODS,						"UnlockAllMods" )
	XT_OPTION( Game::BLUR_STICKER_REACH_RANK_FIFTY,						"ReachRankFifty" )
																		
	XT_OPTION( Game::BLUR_STICKER_REPAIR_HEALTH,						"RepairHealth" )			// RepairMaster
	XT_OPTION( Game::BLUR_STICKER_REPAIR_JUST_IN_TIME,					"RepairJustInTime" )
	XT_OPTION( Game::BLUR_STICKER_WIN_WITH_REPAIR_AND_HALF_PIP,			"WinWithRepairAndHalfPip" )
	XT_OPTION( Game::BLUR_STICKER_REPAIR_MORE_THAN_MAX_HEALTH,			"RepairMoreThanMaxHealth" )
																		
	XT_OPTION( Game::BLUR_STICKER_SHIELD_ATTACKS,						"ShieldAttacks" )			// ShieldMaster
	XT_OPTION( Game::BLUR_STICKER_SHIELD_ATTACKS_SINGLE_SHIELD,			"ShieldAttacksSingleShield" )
	XT_OPTION( Game::BLUR_STICKER_SHIELD_CLOSE_RANGE,					"ShieldCloseRange" )
	XT_OPTION( Game::BLUR_STICKER_HOLD_SHIELD_FOR_LAP,					"HoldShieldForLap" )
																		
	XT_OPTION( Game::BLUR_STICKER_SHOCK_CARS,							"ShockCars" )				// ShockMaster
	XT_OPTION( Game::BLUR_STICKER_SHOCK_EVERY_DOME,						"ShockEveryDome" )
	XT_OPTION( Game::BLUR_STICKER_SHOCK_YOURSELF_AND_AVOID,				"ShockYourselfAndAvoid" )
	XT_OPTION( Game::BLUR_STICKER_SHOCK_CAR_TWICE,						"ShockCarTwice" )
																		
	XT_OPTION( Game::BLUR_STICKER_FIRST_BOSS_DEMAND,					"FirstBossDemand" )			// ShowWhosBoss
	XT_OPTION( Game::BLUR_STICKER_MEET_BOSS_DEMANDS,					"MeetBossDemands" )
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_FIRST_LEONARD_DEMAND,		"CompleteFirstLeonardDemand" )
	XT_OPTION( Game::BLUR_STICKER_COMPLETE_THIRD_SHOWDOWN_DEMAND,		"CompleteThirdShowdownDemand" )
																		
	XT_OPTION( Game::BLUR_STICKER_SHUNT_CARS,							"ShuntCars" )				// ShuntMaster
	XT_OPTION( Game::BLUR_STICKER_REVERSE_SHUNT_CARS,					"ReverseShuntCars" )
	XT_OPTION( Game::BLUR_STICKER_DESTROY_INCOMING_SHUNTS,				"DestroyIncomingShunts" )
	XT_OPTION( Game::BLUR_STICKER_AVOID_SHUNTS_BY_DRIVING,				"AvoidShuntsByDriving" )
																		
	XT_OPTION( Game::BLUR_STICKER_EVADE_FIRST_POWERUP,					"EvadeFirstPowerup" )		// Skills
																		
	XT_OPTION( Game::BLUR_STICKER_SHARE_FIRST_EXPERIENCE,				"ShareFirstExperience" )	// Socialiser
																		
	XT_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_D,					"HighSpeedClassD" )			// SpeedDemon
	XT_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_C,					"HighSpeedClassC" )
	XT_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_B,					"HighSpeedClassB" )
	XT_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_A,					"HighSpeedClassA" )
																		
	XT_OPTION( Game::BLUR_STICKER_WRECK_YOUR_FIRST,						"WreckYourFirst" )			// SwingOfThings
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_DRAKE,							"BeatDrake" )				// SyberianExpress	
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_TALON,							"BeatTalon" )				// Talonted" )			
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_SHANNON_AGAIN,					"BeatShannonAgain" )		// TheRealHustle
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_NATALYA,							"BeatNatalya" )				// ThornyIssue
																		
	XT_OPTION( Game::BLUR_STICKER_VIEW_BRIGHTON_FIREWORKS,				"ViewBrightonFireworks" )	// Tourist
	XT_OPTION( Game::BLUR_STICKER_AMBOY_OILFIELDS_SMOKING_CAR,			"AmboyOilfieldsSmokingCar" )
	XT_OPTION( Game::BLUR_STICKER_NITRO_THROUGH_GAS_STATION,			"NitroThroughGasStation" )
	XT_OPTION( Game::BLUR_STICKER_HOLLYWOOD_NITRO_NO_DAMAGE,			"HollywoodNitroNoDamage" )
																		
	XT_OPTION( Game::BLUR_STICKER_BEAT_AYUMI_HARUMI,					"BeatAyumiHarumi" )			// TwinTrouble
																		
	XT_OPTION( Game::BLUR_STICKER_DESTRUCTION_EVERY_BOLT_GOLD,			"DestructionEveryBoltGold" )// WreckingMachine
	XT_OPTION( Game::BLUR_STICKER_WRECK_CARS_IN_DESTRUCTION,			"WreckCarsInDestruction" )
	XT_OPTION( Game::BLUR_STICKER_LONGSHOTS_IN_DESTRUCTION,				"LongshotsInDestruction" )
	XT_OPTION( Game::BLUR_STICKER_WRECK_RED_CARS,						"WreckRedCars" )
	
XT_END_TYPE

XT_TYPE(Game::BLUR_ACHIEVEMENT, "BlurAchievement")
	XT_OPTION( Game::BLUR_ACHIEVEMENT_ALL_ROUNDER,						"AllRounder" )	
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BARGE_MASTER,						"BargeMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BEAT_BIZARRE,						"BeatBizarre" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BETTER_ENEMIES,					"BetterEnemies" )			
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BLUR_PARTY,						"BlurParty" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BOLT_MASTER,						"BoltMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_BREAK_YOU,						"BreakYou" )				
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CHALLENGE_MASTER,					"ChallengeMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CHALLENGER,						"Challenger" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CHECKPOINT_MASTER,				"CheckpointMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_COMPLETIST,						"Completist" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CONE_GATES,						"ConeGates" )				
	XT_OPTION( Game::BLUR_ACHIEVEMENT_DOMINATOR,						"Dominator" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_FAN_BOY,							"FanBoy" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_FAN_DEMAND_MASTER,				"FanDemandMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_FIRST_TEST,						"FirstTest" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_FOR_THE_FANS,						"ForTheFans" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_PHOTO_FINISH,						"PhotoFinish" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_GETTING_STARTED,					"GettingStarted" ) 		
	XT_OPTION( Game::BLUR_ACHIEVEMENT_GIANT_KILLER,						"GiantKiller" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_GLOBETROTTER,						"Globetrotter" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_GOING_GREY,						"GoingGrey" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_LEARNING_THE_ROPES,				"LearningTheRopes" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_LIGHTS_OUT_MEATBALL,				"LightsOutMeatball" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_MASTER_OF_THE_MACHINE,			"MasterOfTheMachine" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_MEET_THE_MITCHELLS,				"MeetTheMitchells" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_MINE_MASTER,						"MineMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_MODDER,							"Modder" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_MOTOR_MAN,						"MotorMan" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_NITRO_MASTER,						"NitroMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_NOT_SO_PRETTY_BOY,				"NotSoPrettyBoy" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_PAINT_JOB,						"PaintJob" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_RACE_MASTER,						"RaceMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_RANK_FIFTY,						"RankFifty" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_REPAIR_MASTER,					"RepairMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SHIELD_MASTER,					"ShieldMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SHOCK_MASTER,						"ShockMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SHOW_WHOS_BOSS,					"ShowWhosBoss" )			
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SHUNT_MASTER,						"ShuntMaster" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SKILLS,							"Skills" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SOCIALISER,						"Socialiser" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SPEED_DEMON,						"SpeedDemon" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SWING_OF_THINGS,					"SwingOfThings" )			
	XT_OPTION( Game::BLUR_ACHIEVEMENT_SYBERIAN_EXPRESS,					"SyberianExpress" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_TALONTED,							"Talonted" )				
	XT_OPTION( Game::BLUR_ACHIEVEMENT_THE_REAL_HUSTLE,					"TheRealHustle" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_THORNY_ISSUE,						"ThornyIssue" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_TOURIST,							"Tourist" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_TWIN_TROUBLE,						"TwinTrouble" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_WRECKING_MACHINE,					"WreckingMachine" )		
XT_END_TYPE

XT_TYPE( Game::BLUR_ACHIEVEMENT_CATEGORY, "BlurAchievementCategory" )
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CATEGORY_NORMAL,					"Normal" )		
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CATEGORY_MULTIPLAYER,				"Multiplayer" )		
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CATEGORY_POWERUP,					"Powerup" )		
	XT_OPTION( Game::BLUR_ACHIEVEMENT_CATEGORY_BOSS,					"Boss" )
XT_END_TYPE

XT_LIBRARY(ACHIEVEMENTDATA)
	XT_REGISTER_TYPE(StickerData)
	XT_REGISTER_TYPE(AchievementData)
	XT_REGISTER_TYPE(AchievementsList)
	XT_REGISTER_TYPE(MiscStickerData)

	XT_REGISTER_TYPE(Game::BLUR_STICKER)
	XT_REGISTER_TYPE(Game::BLUR_ACHIEVEMENT)
	XT_REGISTER_TYPE(Game::BLUR_ACHIEVEMENT_CATEGORY)
XT_END_LIBRARY
