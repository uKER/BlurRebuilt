#ifndef LEVEL_STATE_H
#define LEVEL_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\GameActors\Vehicles\VehicleControlType.h>
#include "LevelStateVehicleUpgrades.h"
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include "LevelStateVehiclePickupInfo.h"
#include <SystemProfile\SystemProfile.h>
#include <GameLogic\Profile\ProfileBase.h>
#include "LevelStateMultiplayerSettings.h"
#include <GameLogic\RaceModes\RaceModeDefines.h>
#include <GameLogic\Replays\ReplayTypes.h>
#include <ui\Util\WideString.h>
#include <GameLogic\GameData\GameVehicleData.h>
#include <GameLogic\GameData\GameDataEnums.h>
#include "LevelStateConstants.h"

struct RLPickupType;
struct AIPickupControllerSetup;
struct AmaxOpponentSetup;
struct AmaxEvent;
struct DifficultySettings;
struct RaceBookCharacter;
struct RaceBookVehicle;

namespace Game
{

enum GAME_DIFFICULTY
{
	GAME_DIFFICULTY_VERY_EASY,
	GAME_DIFFICULTY_EASY,
	GAME_DIFFICULTY_NORMAL,
	GAME_DIFFICULTY_HARD,
	GAME_DIFFICULTY_HARDCORE,
};

enum LeaderboardType
{
	E_LEADERBOARDTYPE_NULL = -1,
	E_LEADERBOARDTYPE_POSITION,
	E_LEADERBOARDTYPE_LOWDAMAGE,
};

enum E_VEHICLE_STATE
{
	E_VEHICLE_STATE_UNKNOWN,
	E_VEHICLE_STATE_RACING,
	E_VEHICLE_STATE_FINISHED,
	E_VEHICLE_STATE_TIMED_OUT,
	E_VEHICLE_STATE_LEFT_RACE,
	E_VEHICLE_STATE_ELIMINATED,
	E_VEHICLE_STATE_FATALLY_DAMAGED,	// Only used when fatal damage is fatal
	E_VEHICLE_STATE_WAITING_TO_START,

	E_VEHICLE_STATE_MAX,
};

typedef struct LevelStateCar
{
	enum ::E_VEHICLE_CONTROL_TYPE mControlType;
	U32 mCarID;							//	Sorc ID
	U32 mCarLocID;						//	Sorc localisation ID
	U32 mCarStyle;						//	Style
	U32 mManufacturerID;				//	Sorc Manuf. ID
	U32 mController;					//	Controller port
	U32 mStartPosition;					//	Start position on grid (0-19)
	U32 mCharacterID;					//	Sorc ID of character (driver)
	U32 mAIDriver;						//	AI setting of driver
	U32 mAISteering;					//	Steering style
	U32 mResprayID;						//  Sorc ID of respray paint job
	U32 mResprayColour;					//	Raw colour for custom respray
	U32 mTextureDetail;					//	Texture detail scale (capping mip levels)
	U8	mTeam;							//	Which team this vehicle belongs to

	LevelStateVehicleUpgrades	mUpgrades;		//	Specs for any upgrades this vehicle has (Racebook shop items, MP upgrade abilities etc).
	LevelStateVehiclePickupInfo	mPickupInfo;	//	Specification values for pickups this vehicle fires off

	Btl::Vector<E_PICKUP_TYPE>	mPerksToGrant;
	float						mPerkGrantingTimer;
	float						mPerkGrantingInitialTimer;
	bool						mPerkGrantingLaps;

	int							mHealthOverride;
	
	// AI pickup granting information
	float mGrantTimeMin, mGrantTimeMax;

	// AI pickup personality
	E_PICKUP_TYPE				mPerkToGrantAI;
	int							maxPerkLevel;
	Btl::Vector<U32>			mCharactersNotTargetted;
	Btl::Vector<E_PICKUP_TYPE>	mPreferredPickups;

	int							mShieldsEarnedRepair;
	int							mShieldsEarnedHitByPerk;
	int							mShieldsEarnedFans;
	int							mShieldsEarnedFansRequired;
	int							mShieldsInitial;
	
	// *** Network
	E_VEHICLE_STATE mState;

	SysProfile::UserIdentifier	mSystemPlayerId;
	S32 mMultiplayerRank;
	S32 mMultiplayerLegendLevel;
	U8 mMultiplayerWinningStreak;
	bool mBeatenBizarre;
	S8 mAiAvatarId;
	U8 mNetJoinId;
	U8 mOwnerNetJoinId;		//	Join ID of primary vehicle that is responsible for this vehicle (i.e. this is actually an AI vehicle running on another system).
	U16 mNetMessageNos[2];	//	[0] - The current state message number of this vehicle (as far as we know for remote vehicles)
							//	[1] - Our current state message number, as far as this vehicle knows (likely to be at least 1 less than mNetMessageNos[0] of vehicle 0)

	S32 mNetUpdateTimer;
	// ***

	RaceBookCharacter* mCharacter;
	UI::C_WideString64	mNameAboveCar;
	
	bool mAutoGears;
	bool mRightHandDrive;
	bool mHuman;
	bool mAutomatedHuman;
	bool mStartDisabled;
	bool mCutSceneOnly;
	bool mPickupsAllowed;

	//	World matrix vehicle should start at - used for replays since we won't always be starting at grid positions
	//	(exciting cutscenes, etc)
	Maths::HMat43								m_InitialWorldMatrix; 

	Util::C_String mFilename;
	Util::C_String mWheelFilename;
	Util::C_String mDynamicsFilename;
	Util::C_String mAICharacterName;

	Btl::Vector<int> mLoadOuts;

}LevelStateCar;

enum E_AmaxGhostCarWhen
{
	E_AMAXGHOSTCARWHEN_AFTERRESPAWN,
};


enum E_AmaxPickup
{
	E_AMAXPICKUP_NITRO,
	E_AMAXPICKUP_SHUNT,
	E_AMAXPICKUP_SHOCK,
	E_AMAXPICKUP_REPAIR,
	E_AMAXPICKUP_SHIELD,

	E_AMAXPICKUP_NUMBER,
};

enum E_AmaxTournamentType
{
	E_AMAXTOURNAMENTTYPE_KNOCKOUT,
};

enum E_AmaxWreckedBehaviour
{
	E_AMAXWRECKED_RESPAWN,
	E_AMAXWRECKED_ELIMINATED,
	E_AMAXWRECKED_DO_NOTHING,
};

struct AmaxRespraySettings
{
	XtHandle<AmaxPaintResprays> mResprayPaintJobs;
};

struct AmaxRaceGhostCarSettings
{
	int					numGhostCars;
	E_AmaxGhostCarWhen	ghostCarWhen;
	float				respawnLength;

	void Clear()
	{
		numGhostCars = 0;
		ghostCarWhen = E_AMAXGHOSTCARWHEN_AFTERRESPAWN;
		respawnLength = 0.0f;
	}
};

struct AmaxRaceDamageSettings
{
	bool				fatalDamage;
	Btl::Vector<int>	startingDamage;

	void Clear()
	{
		fatalDamage = false;
		startingDamage.Clear();
	}
};

struct AmaxRaceTeam
{
	Maths::C_RGBA_F32			colour;
	int							numDrivers;
	int							delay;
	Btl::Vector<AmaxCarClass>	carClasses;

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );
};

struct LevelPickupProbability
{
	LevelPickupProbability(){pickupType = E_PICKUP_TYPE_NONE, probability = 0.0f;}
	LevelPickupProbability(E_PICKUP_TYPE p_PickupType, float p_Probability = 0.0f)
		: pickupType(p_PickupType), probability(p_Probability) {}

	E_PICKUP_TYPE	pickupType;
	float			probability;
};

struct LevelSetupOpponentData
{
	int									startPositionIndex;
	const AmaxEvent*					ev;
	const DifficultySettings*			difficultySettings;

	RaceBookCharacter*					opponent;
	RaceBookVehicle*					vehicleOverride;
	int									detailOverride;
	bool								alwaysShields;
	bool								onlyAttacksPlayer;
	int									healthOverride;
	int									perkSlotsOverride;
	float								perkGiftTimer;
	float								perkGiftTimerInitial;
	bool								perkGiftLaps;
	bool								startDisabled;

	XtArray<E_PICKUP_TYPE>				perkGiftTypes;
};

class C_LevelState
{
public:
	C_LevelState()
	{
		
	}
	C_ProfileBase m_DefaultProfile; // Default profile

//Helper functions
	void Clear();
	void Finished();
	void Dump();
	void SetupRaceFromEvent(U32 eventId, S32 spEventId = -1);
	bool SetupOpponent(int car_index, LevelSetupOpponentData& opponentSetup);
	void ClearMultiplayerAbilities();

	LevelStateMultiplayerSettings& GetMultiplayerSettings();

	LevelStateVehiclePickupInfo& GetPickupInfo(U8 vehicleIndex);

	Util::C_String m_LevelFilename;
	Util::C_String m_LevelActorStudioFilename;
	Util::C_String m_LevelAIFilename;
	Util::C_String m_LevelEventAIFilename;

	float			m_AIRetryInterval;
	float			m_AIRetryMinTrackSpeed;
	float			m_RetryCount;
	bool			m_AIDontPickupPickups;
	bool			m_overrideAISurface;
	float			m_overrideAISurfaceValue;

	GAME_DIFFICULTY m_difficulty;

	
	U32 mEvent;									//	Racebook event
	U32 mCity;									//	Sorc ID for city
	U32 mRoute;									//	Sorc ID for route
	S32 mRouteNumber;							//	Route index (eg. LA River 0, 1, 2...)
	U32 mLaps;
	S32 mTimeLimit;
	U32 m_AIGameplayBalancing;			// AI Gameplayer balancing
	bool mTeams;							//	Is this event team-based?
	
	S32	mMpRaceIndex;
	
	S32	mSpEventId;

	U32 mNumViewports;

	F32 mFanMultiplier;							// Fan multiplier for double xp weekends
	bool mPunitiveMeasuresInPlace;				// Are we punishing the player
	F32 mPunitiveFanMultiplier;					// How much are we punishing them (1.f = none, 0.5f = 50%, 0.f = total punishment)
	
	bool mStartOpponentsDisabled;				// Forces all opponents to be hidden at styart of race (Used for Fan Demand events)

	LeaderboardType mLeaderboardType;

	bool mSpecificAudioLinesOnly;
	
	//DEBUG
	U32 mEnvironmentOveride;
	U32 mRandomCars;
	bool mDebugEvent;
	//DEBUG
	
	bool mInUI;

	bool mUsePerks;
	bool mPerksRespawn;
	bool mPerksRespawnRandomType;
	bool mNetworkRace;
	bool mOriginalNetworkHost;
	bool mQualify;
	bool mPointsBased;
	bool mOverrideNonRandomPickups;
	bool mDisableFanDemands;
	bool mAllowPickupsOnLapZero;
	bool mShieldsTiedToPickups;
	bool mPerkDroppingWhenUsed;
	bool mWorldHUDPositionInfoRequired;
	bool mDisableRacingContextFanRewards; //e.g. awards for being in first place (e.g. 'ruling the race' etc)
	bool mPlayerIsAI;
	E_AmaxWreckedBehaviour mWreckedBehaviour;

	U32 m_RandomSeed;					// Seed we use on all games to keep ensure their "random" elements in fact produce identical results (initial pickup types, MP awards etc)
	
	// Cutscene stuff
	bool m_CutSceneWavBank;
	bool m_ScriptedCameraWavBank;

	bool								mPickupsEnabled;
	Btl::Vector<LevelPickupProbability> mPickupProbabilities;
	float								mPickupDisableTime;
	int									mPickupSpawnLevel;
	E_PICKUP_RESPAWN_TYPE				mPickupRespawnType;
	
	E_RaceMode mRaceMode;
	UI::E_RaceType mUiRaceType;

	AIPickupControllerSetup* mAIController;
	E_ReplayMode mReplayMode;		//	State of replay system - the game reads this one rather than the (hopefully) identical one in C_ReplayDirector
	E_DAMAGE_MODE mDamageMode;		//	Describes what modifier (if any) to apply to vehicle damage
	
	F32	mDamageMultiplier;			//	Value damage is scaled by (if applicable)
	F32	mPowerUpDamageMultiplier;	//	Value power up damage is scaled by (if applicable)
	
	AmaxStartType mStartType;

	U8 mHoGResetCriteria;
	U8 mHoGNoSplineResetSpeedInMPH;

	AmaxRaceGhostCarSettings mGhostCarSettings;
	AmaxRaceDamageSettings mDamageSettings;

	LevelStateVehiclePickupInfo mDefaultPickupInfo;

	U32 mNumCars;
	U32 mNumAdditionalCars;
	Btl::Vector<LevelStateCar,Btl::VectorParameters_Array<MAX_LEVELSTATE_CARS> > mCars;
	Btl::Vector<Btl::Vector<AmaxCarClass> > mCarClassList;

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	E_ReplayMode const GetReplayMode()const{return mReplayMode;}
	void SetReplayMode(E_ReplayMode p_mode){ mReplayMode = p_mode;}

	void ResetMPAbilityActivations();

private:
	LevelStateMultiplayerSettings mMultiplayerSettings;
};

struct EventState
{
	int numDrivers;
	E_AmaxTournamentType tournamentType;

	Btl::Vector<C_LevelState*> mRaceList;
	Btl::Vector<AmaxRaceTeam*> mTeams;
};

} // namespace Game

#endif // LEVEL_STATE_H
