#ifndef GAME_DATA_H
#define GAME_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>

#include <GameLogic/GameData/GameDataDefs.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_actors/XtActorCreation.h>

#include <GameLogic\RaceRules\RaceRuleBase.h>

#include <Game/GameWorld/EnvironmentSettings.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <Game\GameDef\Materials.h>
#include <GameLogic/RaceLogicCore/Data/RLEnums.h>

#include <GameLogic\GameData\AmaxDialogue.h>
#include <Game\DLC\DLC_Types.h>

#include <Vehicle\db\VehicleXt.h>
#include <SpecialFX/SpecialFX.h>

#include "GameDataEnums.h"

namespace Actors
{
	class AnimatedCameraSequence;
}

namespace RacingPhysics { namespace Surfaces { struct RP_SurfaceMappingList; } }

namespace AnimatedCrowd
{
	class C_XtCrowdPopulation;
	class C_XtCrowdTextureList;
};

namespace Game
{
struct EnvironmentSettings;
struct AmaxCityPerfStats;
}

struct AmaxVehicleDirtSettings;

struct RaceBookCharacter;
struct RaceBookVehicle;

struct RLHermiteKeyFrame;

enum ReflectionResolution
{
	ReflectionResolution_Medium_512x256,
	ReflectionResolution_VeryLow_128x64,
	ReflectionResolution_Low_256x128,
	ReflectionResolution_High_1024x512
};

enum EnvironmentResolution
{
	EnvironmentResolution_High_256,
	EnvironmentResolution_Medium_128,
	EnvironmentResolution_Low_64
};

enum UiCircuitType
{
	UiCircuitType_Sweeping,
	UiCircuitType_Tight,
	UiCircuitType_OffRoad,
	UiCircuitType_Hilly,
	UiCircuitType_Fast,
	UiCircuitType_Technical,
	UiCircuitType_Urban,
	UiCircuitType_Drifty,
	UiCircuitType_Varied,
	UiCircuitType_Oval,
	UiCircuitType_Straight,
};

struct PickupProbability
{
	E_PICKUP_TYPE	pickupType;
	float			probability;
};

struct PickupProbabilityList
{
	XtArray<PickupProbability> pickupProbabilities;
};

struct AmaxCity : AmaxGameDataObj
{
	bool devOnly;
	XtSz8 icon_name;
	XtSz8 actorListFolder;						//Where on the platform to start looking for thse actor list files
	XtArray< XtHandle<C_LevelActorCreationList> > actorLists;
	XtArray< XtHandle<C_LevelActorCreationList> > multiplayerActorLists;		
	Game::EnvironmentSettings environmentSettings;
	Game::EnvironmentSettings flashbackSettings;
	float					headlightIntensityScale;
	float					bumperCameraHeadlightIntensityOffset;
	XtHandle<AmaxVehicleDirtSettings>	vehicleDirtSettings;
	enum ReflectionResolution  reflectionResolution;
	enum EnvironmentResolution  environmentResolution;
	float					environmentFarClip;
	XtHandle<Game::AmaxCityPerfStats>	perfStats;
	XtHandle<RacingPhysics::Surfaces::RP_SurfaceMappingList>	surfaceMappings;
	XtVec2	boundsMin;
	XtVec2	boundsMax;
	XtHandle<AnimatedCrowd::C_XtCrowdTextureList>	crowdFlagTextures;
	bool	forceFallIntoWater;
};

struct AmaxPreLoadList
{
	XtArray< XtHandle<ResourceUtils::C_GameModel> > modelList;
};

struct TimeAttackBestTimeEntry
{
	XtHandle<RaceBookCharacter> name;
	float time_in_secs;
};

struct AmaxRoute : AmaxGameDataObj
{
	XtHandle<AmaxCity> city;
	int route_number;
	XtSz8 thumbnail;
	XtArray< XtHandle<C_LevelActorCreationList> > actorLists;
	XtArray< XtHandle<C_LevelActorCreationList> > multiplayerActorLists;	
	XtArray< XtHandle<C_LevelActorCreationList> > cutsceneActorLists;
	XtHandle<AmaxPreLoadList> preLoadList;
	float	minYPosition;						// Dynamic objects will be killed off below this height (usually because they've been knocked off the world)
	XtArray<TimeAttackBestTimeEntry> best_times;
	U32 fansToUnlockInSplitScreen;
	bool onlyAllowsSingleLap;
	UiCircuitType	circuitType;
	float shockWidthWeighting;
	U8 handOfGodNoSplineResetSpeedInMPH;
	int mAudioVistaID;

	XtHandle<Actors::AnimatedCameraSequence>			VistaCameraSequence;
	XtHandle<Actors::AnimatedCameraSequence>			PackCameraSequence;
	XtHandle<Actors::AnimatedCameraSequence>			PackCameraSequenceMP;
	XtHandle<Actors::AnimatedCameraSequence>			PackCameraSequenceSingle;
	XtHandle<Actors::AnimatedCameraSequence>			PackCameraSequenceBoss;

	XtHandle<AnimatedCrowd::C_XtCrowdPopulation>		CrowdPopulation;
};

enum AmaxPhysicsMaterial
{
	AmaxPhysicsMaterial_Default			= Game::E_PhysicsMaterial_Default,
	AmaxPhysicsMaterial_Medium			= Game::E_PhysicsMaterial_MediumFriction,
	AmaxPhysicsMaterial_Low				= Game::E_PhysicsMaterial_LowFriction,
	AmaxPhysicsMaterial_Bouncy			= Game::E_PhysicsMaterial_Bouncy,
	AmaxPhysicsMaterial_Vehicle			= Game::E_PhysicsMaterial_Vehicle,
	AmaxPhysicsMaterial_Object			= Game::E_PhysicsMaterial_Object,
	AmaxPhysicsMaterial_ObjectBouncy	= Game::E_PhysicsMaterial_ObjectBouncy,
};

struct AmaxFrictionSettings
{
	XtId ID;

	AmaxPhysicsMaterial material1;
	AmaxPhysicsMaterial material2;

	float friction;
	float restitution_coefficient;
};

enum E_AmaxCharacterDrivingStyle
{
	E_AmaxCharacterDrivingStyle_Neutral = 0,
	E_AmaxCharacterDrivingStyle_Relaxed1,
	E_AmaxCharacterDrivingStyle_Relaxed2,
	E_AmaxCharacterDrivingStyle_Aggressive1,
	E_AmaxCharacterDrivingStyle_Aggressive2
};

struct AmaxBaseCharacter
{
	XtSz8 model;
	XtSz8 skeleton;
	E_AmaxCharacterDrivingStyle	drivingStyle;
	XtHandle<ResourceUtils::C_GameModel> seatbeltModel;
};

struct AmaxEvent;

enum AmaxDialogueItemType
{
	E_AmaxDialogueItemType_None,
	E_AmaxDialogueItemType_Basic,
};

enum AmaxDialogueType
{
	E_AmaxDialogueType_None,
	E_AmaxDialogueType_Objective,
	E_AmaxDialogueType_Impact,
	E_AmaxDialogueType_Perk,
	E_AmaxDialogueType_Shield,
	E_AmaxDialogueType_Overtake,
	E_AmaxDialogueType_Health,
};

struct AmaxScoringEventType : AmaxGameDataObj
{
	AMAX_SCORING_EVENT type;

	int min_score;
	int max_score;

	bool append_name;
};

struct AmaxScoringConfig : AmaxGameDataObj
{
	float				message_delay_timer;
	
	float				combo_time;
	
	float				drift_angle_min;
	float				drift_angle_max;

	float				drift_slip_min;
	float				drift_slip_max;
	
	float				using_nitrous_timer;
	float				time_in_air_for_it_to_count;
	
	int					late_repair_level;
	
	float				revenge_timer;
	float				hit_vehicle_with_perk_timer;
	
	float				shield_time_quick;
	float				shield_time_superquick;
	
	float				wrecked_timer;
	float				crashed_out_timer;
	float				out_of_bounds_timer;
	
	int					hit_a_car_damage;	

	float				flipped_out_timer;
	float				minimum_jump_distance;
	float				minimum_cat_distance;

	XtArray<RLHermiteKeyFrame>	combo_end_multiplier_graph;
	int					max_achieveble_chain;

	float				fan_demand_time;
	float				fan_demand_grace_period;

	float				speed_1_mph;
	float				speed_2_mph;
	float				speed_3_mph;

	float				min_360_angular_speed;
	float				max_360_angular_speed;

	float				cooldown360;

	float				vendetta_cooldown;
	float				drift_distance_to_score;
	float				super_drift_distance_to_score;
	float				mega_drift_distance_to_score;

	float				first_place_dom_1;
	float				first_place_dom_2;
	float				first_place_dom_3;
	float				first_place_dom_4;

	float				long_distance;
	float				medium_distance;
	float				short_distance;

	float				sideswipe_impact;

	float				mMinSpeedMPH;
	float				mMinClosingSpeedMPH;
	float				mMinDistanceMetres;
	float				mRegisterNearMissDelay;
	float				mMinTimeInProximity;
	float				mMaxTimeInProximity;
	float				mMaxTrackingTime;
	float				mMinMass;

	//New Scoring
	int 				mBargeBase;
	int 				mBargeDrift;
	int 				mBargeNitrous;
	int 				mBargeAir;

	int 				mMineBase;
	int 				mMineLongShot;
	int 				mMineAir;
	int 				mMineAltFire;

	int 				mShockBase;

	int 				mShuntBase;
	int 				mShuntAltFire;
	
	int 				mBoltBase;
	int 				mBoltLongShot;
	int 				mBoltDrift;
	int 				mBoltAir;
	int 				mBoltAltFire;

	int 				mRammingBase;
	int 				mRammingNitrous;
	int 				mRammingAir;

	int 				mNitrousDrift;
	int 				mNitrousAir;
	int 				mNitrousAltFire;

	int 				mShieldBase;
	int 				mShieldCloseCall;

	int 				mRepairBase;
	int 				mRepairCloseCall;
	
	int 				mDriftLevel1;
	int 				mDriftLevel2;
	int 				mDriftLevel3;
	
	int 				mJumpLevel1;
	int 				mJumpLevel2;
	int 				mJumpNitrous;

	int 				mOvertake;
	int 				mOvertakeNitrous;
	int 				mOvertakeNitrousGood;
	int 				mOvertakeNitrousGreat;
	int 				mOvertakeNitrousMega;

	int 				mPositionThird;
	int 				mPositionSecond;
	int 				mPositionFirst;
	int 				mPositionFirstLevel1;
	int 				mPositionFirstLevel2;

	int 				mEvasionShunt;
	int 				mEvasionShock;

	int 				mSpeedLevel1;
	int 				mSpeedLevel2;
	int 				mSpeedLevel3;
	int 				mSpeedLevel4;

	int 				mNearMiss;
	
	int 				mHitAnotherPickup;

	int 				mCleanRacingNitrous;
	int 				mCleanRacingNoWreckRepairLap;
	int 				mCleanRacingLap;

	int 				mBonusKillingSpree;
	int 				mBonusWrecked;
	int 				mBonusBullying;
	int 				mBonusAccuracy;
	int 				mBonusRevenge;
	int 				mBonusDrown;
	int 				mBonusPlummet;
	int 				mBonusFanDemandsInARow;
	int 				mBonusFullyBolted;
	int					mBonusFanDemandsPerfect;
	int					mBonusFanDemandsHoarder;
	int					mBonusFanRunFirstTime;

	int 				mBargeLevel1;
	int 				mBargeLevel2;
	int 				mBargeLevel3;
	int 				mBargeLevel4;
	int 				mBargeLevel5;

	int 				mDestructionEasyWreck;
	int 				mDestructionMediumWreck;
	int 				mDestructionHardWreck;
	int 				mDestructionAllWreck;
	int 				mDestructionCloseCallWreck;

	int 				mCheckpointCloseCall;
	int 				mCheckpointStopwatchCloseCall;
	int 				mCheckpointAirBrakeCollect;
	int 				mCheckpointCleanSection;
	int 				mCheckpointGoldSection;
	int					mCheckpointSectionPassed;

	int 				mComboFansMultiplier;
	int 				mComboMax;

	int					mDecoyHitScore;

	int					mLastGaspHitScore;
};

struct AmaxPaintResprays : AmaxGameDataObj
{
	XtArray<XtHandle<Vehicle::XtPaintJob> >	m_PaintJobs;
};

//========================  New Social Networking  ========================

enum AMAX_SOCIAL_NETWORKING_TYPE
{
	AMAX_SOCIAL_NETWORKING_TYPE_INCREMENTS,
	AMAX_SOCIAL_NETWORKING_TYPE_ONE_OFF,
	AMAX_SOCIAL_NETWORKING_TYPE_SHARE_BUTTON,
};

enum AMAX_SOCIAL_NETWORKING_ITEM_TYPE
{
//MILESTONES
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ACHIEVEMENT,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_COMPLETED,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_UNLOCKED,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_COMPLETE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_BOSS_BEATEN,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_GAME_COMPLETE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_GOLDEN_FAN_DEMANDS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_STARS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_CARS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_ALL_MODS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TOP_FAN_STATUS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TOP_MULTIPLAYER_RANK,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_ONE_ON_LEADERBOARD,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_COMPLETE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CAR_CHALLENGE_COMPLETE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEGEND_RANK,

//SHARE BUTTON
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_STATUS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_COMPLETION_PERCENTAGE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_FANS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_STARS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_NUMBER_OF_CARS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_BOSSES_FINISHED,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TIER_PERCENT_COMPLETE,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_STARS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_EVENT_OWNER,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_LEADERBOARD_POSITION,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_ABOVE,	//28

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_GOLDEN_FAN_DEMAND_COMPLETE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FAN_PAR_COMPLETE,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_CAR,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_OWN_MOD,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_COMPLETE_PERCENTAGE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CHALLENGE_PACK_COMPLETE_PERCENTAGE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_CAR_CHALLENGE_COMPLETE_PERCENTAGE,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_BLUR_EVENT_RATING,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_WINS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_POWER_UP_HIT_RATE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_RACES,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_PLAYTIME,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_TOTAL_FANS,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_FINISHING_POSITION,					//42
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RACE_AWARD,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_PLAYERS_RANK,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_ABOVE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MP_RELATIVE_PLAYER_POSITION_BELOW,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_RELATIVE_LEADERBOARD_POSITION_BELOW,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UPLOADED_PHOTO,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_TIER,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_EVENT,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_RANK,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_VEHICLE,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_UNLOCKED_BOSS_VEHICLE,
	
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_ACCEPTED,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_WON,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_KOS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_MOST_ATTEMPTS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_TALE_OF_THE_TAPE_AVG_ATTEMPTS,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_STANDINGS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_WINS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_POINTS,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_GAMES,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_SS_TOTAL_PLAYTIME,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_REDEMAND,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_PASSED,
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_FRIEND_DEMAND_FAILED,

	AMAX_SOCIAL_NETWORKING_ITEM_TYPE_MAX,
};

struct AmaxTweetItem
{
	XtLocId	tweet_text;
	XtLocId	tweet_text_singular;
	
	bool suffix;
};

struct AmaxFacebookStoryItem
{
	XtLocId	message;
	
	XtLocId	name;
	XtLocId	name_singular;
	
	XtLocId	caption;
	XtLocId	description;
	XtSz8	image;
	XtLocId	image_href;
	bool suffix;
};

struct AmaxBlurbItem
{
	XtLocId	blurb_text;
	bool suffix;
};

struct AmaxSocialNetworkingItem : AmaxGameDataObj
{
	AMAX_SOCIAL_NETWORKING_TYPE			given_for;
	AMAX_SOCIAL_NETWORKING_ITEM_TYPE	type;

	AmaxBlurbItem				blurb;
	AmaxTweetItem				tweet;
	AmaxFacebookStoryItem		facebook_story;
	
	XtArray<int>				parameters;
};

//========================  New In Game Dialogue   ========================

struct AmaxInGameDialogueItemBase
{
	virtual AmaxDialogueItemType GetType() { return E_AmaxDialogueItemType_None; };

	XtLocId			locId;

	U32				triggerHash;
	U32				characterHash;
	U32				eventHash;
	U32				locationHash;

	U32				conversationId;
	U32				uid;
	bool			oneShot;

	float			weighting;
	XtArray<U32>	charactersInvolved;

	U32				splitGroup;
	U32				splitIndex;

#ifndef CFG_GOLD
	XtSz8			eventName;
	XtSz8			locationName;
	XtSz8			triggerName;
	XtSz8			wavFile;
#endif //CFG_GOLD
};

struct AmaxInGameDialogueItem : AmaxInGameDialogueItemBase
{
	virtual AmaxDialogueItemType GetType() { return E_AmaxDialogueItemType_Basic; };
};

struct AmaxInGameDialogue
{
	XtId									ID;
	bool									ignorePack;
	XtArray<AmaxInGameDialogueItemBase*>	items;
};

// ------------------------------------------------------------------------

struct AmaxDialogueTriggerOvertake
{
	float	probabilityAI;
	float	probabilityPlayer;
	float	probabilityWithNitroAI;
	float	probabilityWithNitroPlayer;
};

struct AmaxDialogueEnvironmentImpactProbabilities
{
	E_IMPACT_LEVEL	impactLevel;
	float			probability;
};

struct AmaxDialogueCharacterImpactProbabilities
{
	E_IMPACT_LEVEL	impactLevel;
	float			probabilityAI;
	float			probabilityPlayer;
};

struct AmaxDialogueTriggerImpact
{
	XtArray<AmaxDialogueEnvironmentImpactProbabilities>	environmentProbabilities;
	XtArray<AmaxDialogueCharacterImpactProbabilities>	characterProbabilities;
};

struct AmaxDialogueTriggerHitByPerk
{
	float			probabilityPlayer;
	float			probabilityAI;

	float			probabilityPlayerSpecial;
	float			probabilityAISpecial;
};

struct AmaxDialogueTriggerShield
{
	float			probabilityAI;
	float			probabilityPlayer;
};

struct AmaxDialogueTriggerControls
{
	AmaxDialogueTriggerImpact		impactTriggerDialogue;
	AmaxDialogueTriggerOvertake		overtakeTriggerDialogue;
	AmaxDialogueTriggerShield		shieldTriggerDialogue;
	AmaxDialogueTriggerHitByPerk	hitByPerkTriggerDialogue;
	float							nextLineProbability;
	XtId							ID;
};

//========================  Debug Reference Stuff  ========================


	struct DebugReferences : AmaxGameDataObj
	{
		XtHandle<Game::DynamicObjectDef>		m_DropObjectType;
	};


struct TitleManagedStorageSettings
{
	XtSz8 mMultiplayerPackFilename;
	XtSz8 mMultiplayerLocFilename;
};

struct StartLineInfoSpecialFX
{
	XtHandle<SpecialFX::C_SpecialFX>	mEnglish;
	XtHandle<SpecialFX::C_SpecialFX>	mFrench;
	XtHandle<SpecialFX::C_SpecialFX>	mGerman;
	XtHandle<SpecialFX::C_SpecialFX>	mItalian;
	XtHandle<SpecialFX::C_SpecialFX>	mSpanish;
};

struct StartLineInfoSpecialFXList
{	
	XtHandle<StartLineInfoSpecialFX>	mStartSpecialFX;
	XtHandle<StartLineInfoSpecialFX>	mCheckpointSpecialFX;
	XtHandle<StartLineInfoSpecialFX>	mFinalSpecialFX;
	XtHandle<StartLineInfoSpecialFX>	mFinishSpecialFX;
};

struct PickupFiringScriptNode
{
	E_PICKUP_TYPE	mPickupType;
	bool			mReverseFire;
	F32				mTime;
	int				mVehIdx;
	int				mTargetIdx;
};

struct PickupFiringScript
{
	XtArray<PickupFiringScriptNode> mNodes;

	// Not loaded by sorcery
	int mCurrentIdx;
	F32	mCurrentTime;
};

struct PickupFiringScriptManager
{
	XtArray< XtHandle<PickupFiringScript> > mScripts;
};


class C_GameData
{
public:
	//void Init(XtDb* db);
	void Init();
	void Update();

	static C_GameData& Instance() {return s_instance;}

	U32 GenerateAvailableDlcBits(void);

	AmaxGameData<AmaxCity*>						mCities;
	AmaxGameData<AmaxRoute*>					mRoutes;
	AmaxGameData<AmaxFrictionSettings*>			mFrictionSettings;
	AmaxGameData<DebugReferences*>				mDebugReferences;
	AmaxGameData<AmaxInGameDialogue*>			mInGameDialogue;
	AmaxGameData<AmaxDialogueTriggerControls*>	mDialogueTriggerControls;
	AmaxGameData<AmaxDLCPack*>					mDLCPacks;
	AmaxGameData<AmaxCheatBase*>				mCheatPacks;

	AmaxGameData<AmaxScoringEventType*>			mScoringEventTypes;
	AmaxGameData<AmaxScoringConfig*>			mScoringConfig;
	AmaxGameData<AmaxPaintResprays*>			mPaintResprays;

	AmaxGameData<AmaxSocialNetworkingItem*>		mSocialStories;
	XtHandle<TitleManagedStorageSettings>		mTitleManagedStorageSettings;
	XtHandle<StartLineInfoSpecialFXList>		mStartLineInfoSpecialFXList;

	XtHandle<PickupFiringScriptManager>			mPickupFiringScriptManager;

	XtPack*	mXtAmaxPack;

private:
	static C_GameData s_instance;
};

#endif // GAME_DATA_H
