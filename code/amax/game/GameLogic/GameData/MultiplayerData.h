#ifndef RACEBOOK_MULTIPLAYER_DATA_H
#define RACEBOOK_MULTIPLAYER_DATA_H

#include "GameLogic/GameData/GameData.h"
#include "GameLogic/GameData/GameVehicleData.h"
#include "Ui/Game/Multiplayer/MultiplayerDefines.h"
#include <GameLogic/Structures/MultiplayerAIDifficulty.h>
#include "Game/GameActors/Vehicles/VehicleActorEnums.h"
#include "specialFX/SpecialFX.h"
#include "framework/application/ApplicationTypes.h"
#include <GameLogic\ContentServer\RecordEventManager.h>
#include <GameLogic\DataGathering.h>

namespace Messaging
{
	class C_MessageBase;
};

// Forward decls
namespace Game
{
	class C_GameProfile;
}

struct RLPickupSpecialFX;

struct MultiplayerRace
{
	MultiplayerRace();

	U8										min_players;
	U8										max_players;
	XtHandle<AmaxRoute>						route;
	XtHandle<C_LevelActorCreationList>		powerupActorListOverride;	

	UI::E_RaceType							type;
	S32										laps;
	S32										time;
	S32										score_limit;

	XtFlags<XtU8, UI::E_VehicleClass>		classes;
	XtFlags<XtU16, UI::E_Pickup>			powerups;
	XtHandle<PickupProbabilityList>			pickupProbabilities;
	UI::E_DamageMode						damage_mode;
	UI::E_Handicap							handicap;
	UI::E_LastManStanding					lastManStanding;
	UI::E_AiMode							aiMode;
	UI::E_CarUpgrades						carUpgrades;
	UI::E_PowerupMods						powerupMods;
	UI::E_PowerupRespawnMode				powerupRespawnMode;
	UI::E_RandomPowerups					randomPowerups;
	bool									lock_to_host_car;
	bool									timeout;
};

struct MultiplayerTimeoutSettings
{
	U8										mMinPlayers;
	U8										mPlayersFinishedToActivateInsivibleTimer;
	F32										mInvisibleTimerTime;
	U8										mPlayersFinishedToActivateVisibleTimer;
	F32										mVisibleTimerTime;
	bool									mOnlyUseFailsafeTimer;
	F32										mFailsafeTimerTime;
	F32										mMaxRaceTimerTime;
};


struct MultiplayerEvent : AmaxDLCBase
{
	MultiplayerEvent();

	U32										ID;
	XtLocId									name;
	XtLocId									desc;
	XtSz8									icon_name;
	XtSz8									image_name;
	bool									not_available;
	
	F32										fan_multiplier;
	U8										min_players;
	U8										max_players;
	U8										min_rank;
	U8										max_rank;
	U8										max_legend;
	bool									teams;

	XtFlags<XtU16, UI::E_MiscSettings>		misc;
	E_MULTIPLAYER_AIDIFFICULTY				aiDifficulty;

	XtArray< XtHandle<MultiplayerRace> >	races;

	bool									is_championship;
	U32										championship_length;
	XtArray<F32>							championship_scores;

	XtArray< MultiplayerTimeoutSettings > timeout_settings;
};


struct MultiplayerGroup : AmaxDLCBase
{
	MultiplayerGroup();

	U32										ID;
	XtLocId									name;
	XtLocId									description;

	UI::E_GroupGenre						genre;

	XtFlags<XtU8, UI::E_RaceModeFlag>		modes;
	XtFlags<XtU8, UI::E_Pickup>				powerups;
	XtFlags<XtU8, UI::E_VehicleClass>		classes;
	XtFlags<XtU8, UI::E_MiscSettings>		misc;

	XtArray< XtHandle<AmaxCity> >			cities;

	XtArray< XtHandle<MultiplayerEvent> >	events;
};

struct MultiplayerCarUpgrade
{
	UI::E_MultiplayerVehicleUpgrades type;
	XtLocId						name;
};


//---------------------------------------------------------------------------------------

struct MultiplayerAbilityType
{
	XtFlags<XtU8, UI::E_MultiplayerVehicleAbilities>	mAbilityType;
};

struct MultiplayerAbilityData
{
	XtLocId								mAbilityName;
	XtLocId								mAbilityDescription;
	XtSz8								mIconName;
	UI::E_MultiplayerVehicleAbilities	mAbilityId;
	UI::E_MultiplayerAbilityCategory	mAbilityCategory;
	XtF32								mAbilityValue;
	XtArray<RLPickupSpecialFX>			mAbilitySpecialFX;
};

//---------------------------------------------------------------------------------------

struct MultiplayerAwardData
{
	XtLocId								mAwardName;
	XtLocId								mAwardDescription;
	XtSz8								mIconName;
	UI::E_MultiplayerAward				mAwardId;
	UI::E_MultiplayerAwardCategory		mAwardCategory;
	UI::E_MultiplayerStat				mAwardStat;
	UI::E_MultiplayerStatSortingStrategy mAwardStatSortingStrategy;
	XtF32								mThresholdValue;
	XtF32								mMotormashThresholdMultiplier;
	XtF32								mRaceThresholdMultiplier;
	bool								mRequiresAISplineData;
	bool								mDisabledInMotormash;		
	bool								mDisabledInRace;
	XtF32								mReducedProbabilityPositionPercentage;
};

struct MultiplayerAwardDistributionSpec
{
	int									mMinNumPlayersRequired;				
	int									mNumRandomAwardsOrSpoonsToGift;		
	int									mNumSpoonsToGift;						
	int									mNumAwardsToGift;	
	int									mMaxNumAwardsPerPlayer;
};

//---------------------------------------------------------------------------------------

struct MultiplayerAbilitySlot
{
	UI::E_MultiplayerAbilityCategory			mType;
	XtLocId										mName;
	XtArray<XtHandle<MultiplayerAbilityData> >	mAbilities;
};

struct MultiplayerAbilityLoadOut
{
	UI::E_MULTIPLAYER_LOADOUT_TYPE				mType;

	XtHandle<MultiplayerAbilityData>			mAbilitySlot1;
	XtHandle<MultiplayerAbilityData>			mAbilitySlot2;
	XtHandle<MultiplayerAbilityData>			mAbilitySlot3;
};

struct MultiplayerChallengeTier
{
	F32												mTargetAmount; 
	S32												mFansReward; 
	S32												mData;
	UI::E_MultiplayerVehicleUpgrades				mReward;
};

// All these functions are defined in MultiplayerChallenges, so MultiplayerData doesn't get bogged down.
struct MultiplayerChallenge
{
	XtLocId									mName;
	XtLocId									mDescription;
	UI::E_MULTIPLAYER_CHALLENGE_TYPE		mType;
	bool									mHideProgress;
			
	XtArray<MultiplayerChallengeTier>		mTiers;
};

struct MultiplayerChallengePack 
{
	XtLocId										mName;
	XtLocId										mUnlockName;
	XtLocId										mDesc;
	UI::E_ChallengePack							mType;
	XtSz8										mIconName;
	XtArray<XtHandle<MultiplayerChallenge> >	mChallenges;
};

//---------------------------------------------------------------------------------------

struct MultiplayerFeature
{
	XtLocId								mFeatureName;
	XtLocId								mFeatureDescription;
	XtSz8								mIconName;
	XtSz8								mIconSheet;
	XtSz8								mTexture;
	UI::E_MultiplayerFeature			mFeatureId;
	bool								mIsPlaylist;
};

struct MultiplayerLevel
{
	  int                                               mFansNeeded;
	  bool                                              mUnlockGroupCreator;
	  XtArray< XtHandle<AmaxVehicle> >                  mVehicleUnlocks;
	  XtArray< XtHandle<MultiplayerAbilityData> >       mAbilityUnlocks;
	  XtArray< XtHandle<MultiplayerChallengePack> >		mChallengeUnlocks;
	  XtArray< XtHandle<MultiplayerFeature> >			mFeatureUnlocks;
};

//---------------------------------------------------------------------------------------

// Defined in ScoringRules.cpp
struct MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex) { return 0; };

	float mNumPointsScored;
};

struct MultiplayerScoringRuleImpact : public MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);

	E_IMPACT_LEVEL mImpactLevel;
};

struct MultiplayerScoringRuleHitByPerk : public MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);

	E_PICKUP_TYPE mPickupType;
};

struct MultiplayerScoringScoringEvent : public MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);

	AMAX_SCORING_EVENT mScoringEvent;
};


struct MultiplayerScoringRulePickedUpPickup : public MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);

	E_PICKUP_TYPE mPickupType;
};

struct MultiplayerScoringRuleCrossedLineFirst : public MultiplayerScoringRuleBase
{
	virtual float				ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex);
};

//---------------------------------------------------------------------------------------

struct MotorMashScoringValues
{
	U8												m_WreckScore;
	U8												m_SuicideScore;
	U8												m_LowDamageScore;
	U8												m_HighDamageScore;
	U8												m_RivalDamageMultiplier;
};

//---------------------------------------------------------------------------------------

struct DamageMultiplierValues
{
	F32												m_High;
	F32												m_Medium;
	F32												m_Low;
};

//---------------------------------------------------------------------------------------


struct MiscModSettings
{
	bool											m_SafetyNetAbsorbEnabledBomb;
	bool											m_SafetyNetAbsorbEnabledShock;
	bool											m_SafetyNetAbsorbEnabledBolt;
	bool											m_SafetyNetAbsorbEnabledShunt;
	bool											m_SafetyNetAbsorbEnabledBarge;
	bool											m_BribeOnActivatePickups;
	
	bool											m_ScramblerEnabledBomb;
	bool											m_ScramblerEnabledShock;
	bool											m_ScramblerEnabledBolt;
	bool											m_ScramblerEnabledShunt;
	bool											m_ScramblerEnabledBarge;
	bool 											m_ScramblerEnabledBombReverse;
	bool 											m_ScramblerEnabledBoltReverse;
	bool 											m_ScramblerEnabledShuntReverse;
	bool											m_ScramblerEnabledNitroRift;

	int												m_ScramblerNumBoltRequiredInChain;
	F32												m_ScramblerModBoltChainTimerIncrement;
	F32												m_ScramblerPowerupBayDisableTime;
	bool											m_ScramblerBoltsStack;
	F32												m_ScramblerBoltDisableTime;

	F32												m_ShieldedBoostersBargeDamageAbsorbed;
	F32												m_ShieldedBoostersBoltDamageAbsorbed;
	F32												m_ShieldedBoostersBombDamageAbsorbed;
	F32												m_ShieldedBoostersShockDamageAbsorbed;
	F32												m_ShieldedBoostersShuntDamageAbsorbed;

	bool											m_FrontrunnerGiftNitroOnRespawn;

    bool											m_FrontrunnerNitroOnRespawn;		
	bool 											m_FrontrunnerNitroOnSplit;		
	bool 											m_FrontrunnerNitroOnRaceStart;		
	bool 											m_FrontrunnerNitroOnLapCompleted;		
	bool											m_FrontrunnerNitroOnFinalLap;	
	F32												m_FrontrunnerMotormashTimer;

	bool											m_TitaniumArmourBlocksBarge;
	bool											m_TitaniumArmourBlocksBolt;
	bool											m_TitaniumArmourBlocksBomb;
	bool											m_TitaniumArmourBlocksShock;
	bool											m_TitaniumArmourBlocksShunt;

	F32												m_JumpTheGunWreckedSequenceReductionMultiplier;
	F32												m_JumpTheGunRespawnSpeedWreck;
	F32												m_JumpTheGunRespawnSpeed;

	F32												m_IronFistBargeDamage;
	F32												m_IronFistBoltDamage;
	F32												m_IronFistBombDamage;
	F32												m_IronFistShockDamage;
	F32												m_IronFistShuntDamage;
	U8												m_IronFistNumBoltsRequired;

	F32												m_DecoyDropbrakeTarget;
	U8												m_DecoyDropsAllowedPerPlayer;
	F32												m_DecoyDropToleranceDistance;

	F32												m_ShieldingEfficiencyExtraHealth;

	XtArray<F32>									m_DrifterLengths;

	F32												m_laserSightMaxAngle;
	F32												m_laserSightMinNormal;

};

//---------------------------------------------------------------------------------------

struct PickupPercentages
{
	F32												m_ChanceOfNitro;
	F32												m_ChanceOfShunt;
	F32												m_ChanceOfShock;
	F32												m_ChanceOfBarge;
	F32												m_ChanceOfMine;
	F32												m_ChanceOfBlindfire;
	F32												m_ChanceOfRepair;
	F32												m_ChanceOfShield;
};

//---------------------------------------------------------------------------------------

struct BribeModSettings
{
	PickupPercentages								m_MotorMashStart;
	PickupPercentages								m_MotorMashRespawn;

	PickupPercentages								m_RaceStart;
	PickupPercentages								m_RaceRespawn;
	F32												m_GiftDelayTime;
};

//---------------------------------------------------------------------------------------

struct BatteringRamFakePhysicsSettings
{
	XtVec3											m_ImpulseScale;
	XtVec3											m_KickForce;
	XtVec3											m_ConstantKickForce;
};

//---------------------------------------------------------------------------------------

struct BatteringRamCollisionConsequences
{
	F32												m_CoolDownTimer;
	F32												m_TargettingCone;
	F32												m_MinimumImpactSpeedInMPH;

	BatteringRamFakePhysicsSettings					m_LightHitSettings;
	BatteringRamFakePhysicsSettings					m_HeavyHitSettings;
};

//---------------------------------------------------------------------------------------

struct AlphaGraph
{
	XtArray<SpecialFX::C_HermiteKeyFrame>			m_AlphaGraph;
};

//---------------------------------------------------------------------------------------

struct CloakingAlphaGraphs
{
	AlphaGraph										m_CloakingAlphaGraph;
	AlphaGraph										m_UnCloakingAlphaGraph;
};

//---------------------------------------------------------------------------------------

struct SilentRunningModSettings
{
	F32												m_LocalGhostOpacity;	
	F32												m_RemoteGhostOpacity;
	
	F32												m_UnCloakFadeUpTime;
	F32												m_CloakFadeDownTime;
	
	F32												m_PostEnvironmentHitUnCloakTime;
	F32												m_PostVehicularHitUnCloakTime;
	F32												m_PostPowerUpHitUnCloakTime;
	F32												m_PostPowerUpFireUnCloakTime;
	F32												m_PostRaceStartUnCloakTime;
	
	bool											m_VisibleOnRadar;

	CloakingAlphaGraphs								m_AlphaGraphs;
};

//---------------------------------------------------------------------------------------

struct PostRaceBonusData
{
	F32												m_BaseScore;
	F32												m_RaceTimeAverage;
	F32												m_RaceTimeMultiplierMax;
	F32												m_RankMultiplier;
	XtArray<F32>									m_PositionMultipliers;
};

//---------------------------------------------------------------------------------------
struct LastGaspModSettings
{
	float   m_LastGaspJumpTheGunCombinedWreckedSequenceReductionMultiplier;
	float	m_LastGaspWreckedSequenceReductionMultiplier;
	float	m_wreckImpulseGlobalScale;
	float	m_wreckAngularVelocityModifier;
	float	m_wreckUpVelocityModifier;
	S32		m_bargeDamage;
	float	m_bargeRadiusModifier;	
	bool	m_automaticallyFireOnTimeout;
	float	m_timeTillForceFire;
	bool	m_timerEnabled;
	bool    m_delayWreckedBrokenGlassScreenEffect;

	float   m_detonateAllowedDelay;
	float   m_damageCooldown;
	float   m_wreckedSequenceDelay;
};


//---------------------------------------------------------------------------------------

struct PlayerRubberBandingSettings
{
	F32												m_MaxDistanceBetweenGroupMembers;
	F32												m_MinDistanceForFullBanding;
	bool											m_UseRaceDistance;
	F32												m_LightAssistMultiplier; // TODO: Unused until we get UI support for changing
	F32												m_MediumAssistMultiplier;//			banding to light/medium/heavy in split screen  
	XtArray<SpecialFX::C_HermiteKeyFrame>			m_ImpulseMagnitudeMapping;
};

//---------------------------------------------------------------------------------------

struct SplitScreenScores
{
	XtArray< U8 >									mOneViewport;
	XtArray< U8 >									mTwoViewports;
	XtArray< U8 >									mThreeViewports;
	XtArray< U8 >									mFourViewports;
};

//---------------------------------------------------------------------------------------

struct CustomRaceDefaults
{
	UI::E_RaceType				game_mode;
	XtHandle<AmaxRoute>			track;
	S32							laps;
	S32							time;
	AmaxCarClass				car_class;
	UI::E_DamageMode			damage;
	UI::E_LastManStanding		respawn;		
	bool						timeout;
	UI::E_PowerupMods			mods;			
	UI::E_CarUpgrades			upgrades;
	bool						lock_to_host_car;
	UI::E_AiMode				ai;
	S8							ai_padding;
	UI::E_Handicap				handicap;
	UI::E_RandomPowerups		random_powerups;
	UI::E_PowerupRespawnMode	powerup_respawn;

	S32							max_laps;
	S32							max_time;
	S32							time_step;
};

//---------------------------------------------------------------------------------------

struct IdleTimeoutData
{	
	F32														m_MotorMashIdleTimeout;
	F32														m_RacingIdleTimeout;
};	

//---------------------------------------------------------------------------------------

struct DataGatheringRecords
{
	XtFlags<XtU32, Game::DATA_GATHERING_RECORD_TYPE>		m_ActivatedRecords;
};

struct MultiplayerConfig
{
	// General tabs
	bool													m_OnlineModeDisabled;
	bool													m_PeerProxyEnabled;
	bool													m_JoinSessionsInRace;
	U32														m_PeerProxyMaxConnections;
	U16														m_PackVersion;
	U16														m_MatchingVersion;
	XtArray< XtHandle<AmaxVehicle> >						m_SplitScreenVehicles;
	U32														m_MinTotalPlayerShowCount;
	U32														m_MinPlaylistPlayerShowCount;
	U8														m_MotorMashDamageMultiplier;
	MotorMashScoringValues									m_MotormashScoringValues;
	XtArray< F32 >											m_MotormashRespawnDelayMultipliers; 
	F32														m_CampingPreventionTimeout;
	F32														m_CarRewardFanBonusPercentage;
	DamageMultiplierValues									m_DamageMultiplierValues;

	IdleTimeoutData											m_IdleTimeoutData;

	bool													m_ShowDeveloperIcon;

	XtHandle<DataGatheringRecords>							m_DataGatheringRecords;

	// Playlist tab
	XtArray< XtHandle<MultiplayerEvent > >					m_OnlineEvents;
	XtArray< XtHandle<MultiplayerEvent > >					m_SplitscreenEvents;
	XtArray< XtHandle<MultiplayerEvent > >					m_SystemLinkEvents;
	XtArray< int >											m_RacePositionPoints;
	U8														m_FindPlayersTimer;
	U8														m_PlaylistAiPaddingLimit;
	F32														m_OverRankPunitiveMultiplier;
	U32														m_ChampionshipCompletionBonus;
	U8														m_ChampionshipWaitTime;
	U32														m_ChampionshipPlayerLeftPenalty;

	// Custom Race tab
	XtArray< XtHandle<AmaxRoute> >							m_Routes;
	XtArray< XtHandle<AmaxRoute> >							m_Arenas;
	CustomRaceDefaults										m_CustomRaceDefaults;
	XtArray< MultiplayerTimeoutSettings >					m_CustomRaceTimeout;


	// Scoring tab
	XtArray< MultiplayerScoringRuleBase* >					m_ScoringRules;
	PostRaceBonusData										m_PostRaceBonus;

	// Profile tab
	XtArray< XtHandle<MultiplayerChallengePack> >			m_ChallengePacks;
	XtArray< XtHandle<MultiplayerAbilitySlot> >				m_AbilitySlots;
	XtArray< MultiplayerAbilityLoadOut >					m_LoadOuts;
	XtArray< MultiplayerAbilityLoadOut >					m_SystemLinkLoadOuts;
	XtArray< MultiplayerCarUpgrade >						m_CarUpgrades;
	XtArray< MultiplayerLevel >								m_LevelingData;
	XtArray< MultiplayerLevel >								m_LegendLevelingData;

	// Awards tab
	bool													m_MultiplayerAwardsEnabled;
	XtArray< XtHandle<MultiplayerAwardData> >				m_MultiplayerAwards;
	XtArray< XtHandle<MultiplayerAwardDistributionSpec> >	m_MultiplayerAwardDistributionSpecs;

	// Mod tab
	U8														m_ShowyFlourishBlindfireExtraFans;
	MiscModSettings											m_MiscModSettings;
	LastGaspModSettings										m_LastGaspModSettings;

	BribeModSettings										m_BribeModSettings;
	BatteringRamCollisionConsequences						m_BatteringRamCollisionConsequences;

	SilentRunningModSettings								m_SilentRunningModSettings;

	F32														m_RivalsCommencementDelayTimer; 
	F32														m_RivalsCountdownTimer;

	//	AI Racers tab
	XtArray< XtHandle<RaceBookCharacter> >					m_AiCharacters;
	U8														m_MaxAiPerConsole;
	U8														m_AiMpRank_Easy;
	U8														m_AiMpRank_Medium;
	U8														m_AiMpRank_Hard;
	U8														m_AiMpRankVariance;

	//Splitscreen tab
	F32														m_PowerupRespawnToggleLong;
	F32														m_PowerupRespawnToggleVeryLong;
	XtArray< U8 >											m_NumberOfSplitScreenAIPlayers;
	PlayerRubberBandingSettings								m_PlayerRubberBandingSettings;
	XtHandle<SplitScreenScores>								m_ScoresWithAI;
	XtHandle<SplitScreenScores>								m_ScoresWithoutAI;
	XtArray< XtHandle<SpecialFX::C_SpecialFX> >				m_SplitScreenLightPillars;
	F32														m_TimeOutTimeHidden;
	F32														m_TimeOutTimeVisible;

	// challenge progress
	U32														m_minChallengeSize;	
	U32														m_introZone;		
	U32														m_outroZone;			
	U32														m_percentInterval;	

	// Teams
	XtArray< XtHandle<AmaxVehicle> >						m_InvalidTeamVehicles;
	U8														m_BalancingRange;
	XtLocId													m_TeamAName;
	XtLocId													m_TeamBName;
	XtSz8													m_TeamAColour;
	XtSz8													m_TeamBColour;
	XtHandle< Vehicle::XtPaintJob >							m_TeamAPaint;
	XtHandle< Vehicle::XtPaintJob >							m_TeamBPaint;
	XtHandle<SpecialFX::C_SpecialFX>						m_TeamAPillarEffect;
	XtHandle<SpecialFX::C_SpecialFX>						m_TeamBPillarEffect;
	U32														m_WinnerTeamFanBonus;
	F32														m_WinnerTeamFanBonusBalancingMultiplier;

	U16														m_LatestBlurNewsItemId;
	U16														m_LatestDlcItemId;
	U16														m_LatestCommunityItemId;

	//	Viral Achievements
	XtArray< U64 >											m_DeveloperXUIDs;			//	Bizarre developer XUIDs that match their Gamertags - must be our home RETAIL ones when we ship
};

/*---------------------------------------------------------------------------------------
	Class : C_MultiplayerData

	Desc : Container for multiplayer specific group data
---------------------------------------------------------------------------------------*/
class C_MultiplayerData
{
private:
	// Private and undefined
	C_MultiplayerData( const C_MultiplayerData& lhv );
	C_MultiplayerData operator = ( const C_MultiplayerData& lhv );

public:
	C_MultiplayerData()
	:	m_pMultiplayerPack( NULL ), m_pMultiplayerData ( NULL )
	{};

	static C_MultiplayerData&			Instance()
										{
											return m_sInstance;
										}

	const MultiplayerGroup*				FindGroup( const U32 uID ) const;

	const AmaxRoute*					FindRoute( const U32 uID ) const;
	
	const AmaxVehicle*					FindVehicle( const U32 uID ) const;

	const MultiplayerAbilityData*		FindAbility( const UI::E_MultiplayerVehicleAbilities eAbilityType ) const;
	const MultiplayerAbilityData*		FindAbility( const XtId id ) const;

	const HRESULT						LoadMultiplayerPackFile( const char* pFileName );
	const bool							LoadMultiplayerPackData( U8* pData, const U32 uLength, bool overwrite = false );
	const bool							ReloadMultiplayerPackData();

	void								PreCacheData();

	U8									GetFinalPlaylistUnlockRank() const {return m_FinalPlaylistUnlockRank;}								
	U8									GetFinalChallengePackUnlockRank() const {return m_FinalChallengePackUnlockRank;}
	U8									GetFinalModUnlockRank() const {return m_FinalModUnlockRank;}

	const bool							ValidateVehicleForTeams(U32 vehicleXtId);

private:
	const HRESULT						LoadPackFile( XtPack*& pPack, const char* pFileName );
	const bool							LoadPackData( XtPack*& pPack, void* pData, const U32 uLength );
	void								InitSubsystems();
	void								CacheUnlockRanks(Game::C_GameProfile* pProfile); 

	U8*									m_pMultiplayerData;
	U32									m_MultiplayerDataLength;	
public:
	MultiplayerConfig*					m_pConfig;
	
	// These are not pulled in, but useful stuff we precache for easy access
	Btl::Map<U32, int>					m_MultiplayerVehicleMap;
	Btl::Map<U32, int>					m_MultiplayerAbilityMap;
	Btl::Vector< XtHandle<AmaxVehicle> >	m_MultiplayerVehicles;
	Btl::Vector<U32>					m_InvalidTeamVehicleIds;

private:
	static C_MultiplayerData			m_sInstance;

	// Cache useful unlock rank data here from sorcery data
	U8									m_FinalPlaylistUnlockRank;
	U8									m_FinalChallengePackUnlockRank;
	U8									m_FinalModUnlockRank;

	XtPack*								m_pMultiplayerPack;
};


typedef XtArray< MultiplayerLevel > TLevelData;


#endif // RACEBOOK_MULTIPLAYER_DATA_H
