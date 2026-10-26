#ifndef GAME_EVENTDATA_H
#define GAME_EVENTDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameDataDefs.h>
#include <GameLogic\GameData\AmaxDialogue.h>
#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameLogic\RaceRules\RaceAction_Base.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <Game\PerfStats\PerfStats.h>

#include <Network\Core\NetScoreboards.h>
#include "RaceBook\Data\RaceBookData.h"

class C_LevelActorCreationList;
struct AmaxRoute;

struct RaceBookVehicle;
struct RaceBookCharacter;
struct RLPickupType;


struct AmaxCutScene
{
	bool								m_UseSetupList;
	bool								m_IsDummy;

	XtHandle<C_LevelActorCreationList>	m_Setup_ActorList;
	XtHandle<C_LevelActorCreationList>	m_Scene_ActorList;
	XtHandle<C_LevelActorCreationList>	m_Audio_ActorList;
	XtVec3								m_Origin;
	float								m_Radius;
	XtArray<int>						m_VehiclesToUnload;
	Game::AmaxPlatformAllocation		m_SystemMemorySize;
	Game::AmaxPlatformAllocation		m_GraphicsMemorySize;
	Game::AmaxPlatformAllocation		m_SystemMemoryBudget;
	Game::AmaxPlatformAllocation		m_GraphicsMemoryBudget;
};


struct AmaxWinCondition : AmaxGameDataObj
{

};

struct AmaxIntroSection
{
	XtLocId dialogue;
	AmaxDialogueAudio spoken_line;
	float section_time;
	XtRgba colour;

	int car_index;
};

struct AmaxStoryIntroOutro
{
	XtArray<AmaxIntroSection> story;
};

struct AmaxOpponentSetup
{
	XtHandle<RaceBookCharacter>			opponent;				
	XtHandle<RaceBookVehicle>			vehicleOverride;
	int									detailOverride;
	bool								alwaysShields;
	bool								onlyAttacksPlayer;
	int									healthOverride;
	int									perkSlotsOverride;
	float								perkGiftTimer;
	float								perkGiftTimerInitial;
	bool								perkGiftLaps;

	XtArray<E_PICKUP_TYPE>				perkGiftTypes;
};

// -------------------------------------------------------

enum E_STUNT_ACTION
{
	E_STUNT_ACTION_DRIFTING,
};

struct AmaxEventActionBase
{
	void SetBaseValues(Game::RaceActionBase* rule) { rule->SetTimer(timer); };

	virtual Game::RaceActionBase* CreateAction() { return NULL; };

	float						timer;
};

struct AmaxEliminateDrivers : AmaxEventActionBase
{
	Game::RaceActionBase* CreateAction();

	int							numDrivers;
};

struct AmaxEliminateHitByPerks : AmaxEventActionBase
{
	Game::RaceActionBase* CreateAction();

	int							numHits;
	XtArray<E_PICKUP_TYPE>		pickups;
	XtHandle<RaceBookCharacter>	firingDriver;
};

struct AmaxTimeAttackDisplay : AmaxEventActionBase
{
	Game::RaceActionBase* CreateAction();
};

struct AmaxManageGhosts : AmaxEventActionBase
{
	Game::RaceActionBase* CreateAction();
};

// -------------------------------------------------------

struct AmaxRuleBase
{
	virtual Game::RaceModeRuleBase* CreateRule() { return NULL; };

	void SetBaseValues(Game::RaceModeRuleBase* rule) { rule->SetPersistent(persistent); };

	bool persistent;
};

struct AmaxDriverTimerExpired : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	float			initialTime;
	int				numZones;
	XtArray<float>	timeAwarded;
};

struct AmaxDriverFireXPerks : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();
	
	int numFired;
	XtArray<E_PICKUP_TYPE> pickups;
};

struct AmaxTeamPosition : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();
};

struct AmaxDriverLastInRace : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
};

struct AmaxDriverCollectXFanPickups : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	int							numPickups;
	XtHandle<RaceBookCharacter> driverOverride;
};

struct AmaxDriverHitXObjects : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	int							groupID;
};

struct AmaxDriverFinishInXPosition : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	int position;
	XtHandle<RaceBookCharacter> driver;
};

struct AmaxDistanceFromDriver : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	RaceBookQuantifier			quantifier;
	float						distance;
	float						time;
	XtHandle<RaceBookCharacter> driverOverride;
	XtHandle<RaceBookCharacter> driverDistance;
};

struct AmaxBeatDriver : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	XtHandle<RaceBookCharacter> driverToBeat;
};

struct AmaxPerformActionYTimes : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	E_STUNT_ACTION				action;
	int							numTimes;
};

struct AmaxDriverPerformXTrickShots : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	int numShots;
};

struct AmaxDriverGetLapTimeOfX : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	float lapTime;
};

struct AmaxDriverHitDriverXTimesWithPerks : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	XtHandle<RaceBookCharacter> driverToHit;
	XtArray<E_PICKUP_TYPE>		pickupTypes;
	int							numHits;
	bool						requireDoubleHits;
};

struct AmaxDriverHitXMph : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	int							speed;
	int							areaRef;
	RaceBookQuantifier			quantifier;
};

struct AmaxDriverDriftingInTriggerRegion : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	S32 areaRef;
};

struct AmaxDriverHasHealth : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driver;
	RaceBookQuantifier quantifier;
	float health;
};

struct AmaxCauseDamageToDriver : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driver;
	RaceBookQuantifier quantifier;
	float damage;
};

struct AmaxDriverEliminated : AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driver;
	int elimGroupSize;
};

struct AmaxDriverEarnXP: AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
	int xpNeeded;
};

struct AmaxDriverFastestLap: AmaxRuleBase
{
	Game::RaceModeRuleBase* CreateRule();

	XtHandle<RaceBookCharacter> driverOverride;
};

// -------------------------------------------------------

struct AmaxPerkAI
{
	int										opponentIdx;
	E_PICKUP_TYPE							perkToGrant;
	int										maxPerkLevel;
	XtArray< XtHandle<RaceBookCharacter> >	charactersNeverTargetted;
	XtArray< E_PICKUP_TYPE >				preferredPickups;
	float									grantTimeMin, grantTimeMax;
};

struct DifficultySettings
{
	// Race Rules
	XtArray< AmaxRuleBase* >		passRules;
	XtArray< AmaxRuleBase* >		failRules;
	XtArray< AmaxEventActionBase* >	lapActions;
	XtArray< AmaxEventActionBase* >	timeActions;
	Game::LeaderboardType			leaderboardType;
	bool							instaPass;
	bool							instaFail;
	bool							sequentialRules;
	int								timeLimit;
	float							damageModifier;
	float							powerUpDamageModifier;

	XtLocId								ruleStrOverride;
	XtArray< XtLocId >					ruleSeqStrOverride;
	XtArray< AmaxPerkAI >				aiPerkUse;
	
	XtSz8								aiBalancingFileName;
	float								aiRetryInterval;
	float								aiRetryMinTrackSpeed;

};


enum E_AnimaticTrigger
{
	E_AnimaticTrigger_None = 0,
	E_AnimaticTrigger_PreEvent,
	E_AnimaticTrigger_PostEvent,
};

struct AnimaticSubtitle
{
	XtLocId subtitle;
	float starttime;
	float life;
};

enum E_DestructionMode_AI_Level
{
	DM_AI__None,
	DM_AI__Easy,
	DM_AI__Medium,
	DM_AI__Hard,
	DM_AI__VHard,
};

enum E_DestructionMode_Wave_Formation
{
	DM_WF__Line,
	DM_WF__Queue,
	DM_WF__Box,
	DM_WF__BackLeft_FrontRight,
	DM_WF__BackRight_FrontLeft,
	DM_WF__Arrow,
	DM_WF__Diamond,
	DM_WF__Max,
};

struct  DestructionMode_AI
{
	E_DestructionMode_AI_Level		slot1;
	E_DestructionMode_AI_Level		slot2;
	E_DestructionMode_AI_Level		slot3;
	E_DestructionMode_AI_Level		slot4;
};

struct DestructionMode_Wave
{

	DestructionMode_AI					waveAI;
	E_DestructionMode_Wave_Formation	waveFormation;
	F32									waveSeparationDistance;
};

// -------------------------------------------------------

struct AmaxRaceModeBase
{
	AmaxRaceModeBase() { raceMode = Game::E_RaceMode_Nothing; }

	Game::E_RaceMode raceMode;
};

struct AmaxRaceModeStreetRace : AmaxRaceModeBase
{
	AmaxRaceModeStreetRace() { raceMode = Game::E_RaceMode_StreetRace; }
};

struct AmaxRaceModeDestructionVars
{
	XtF32								initialTime;

	bool								EasyCarsUsePerks;
	bool								MediumCarsUsePerks;
	bool								HardCarsUsePerks;
	bool								VHardCarsUsePerks;
	XtI32								bronzePoints;
	XtI32								silverPoints;
	XtI32								goldPoints;
};

struct AmaxRaceModeDestruction : AmaxRaceModeBase
{
	AmaxRaceModeDestruction() { raceMode = Game::E_RaceMode_Destruction; }

//	XtF32								initialTime;
	XtF32								timeForWreckEasy;
	XtF32								timeForWreckMedium;
	XtF32								timeForWreckHard;
	XtF32								timeForWreckVHard;
	XtF32								behindCutOffDistance;
	XtHandle<RaceBookVehicle>			vehicleEasy;
	XtHandle<RaceBookVehicle>			vehicleMedium;
	XtHandle<RaceBookVehicle>			vehicleHard;
	XtHandle<RaceBookVehicle>			vehicleVHard;
	XtArray< DestructionMode_Wave >		waves;
	AmaxRaceModeDestructionVars			easy;
	AmaxRaceModeDestructionVars			medium;
	AmaxRaceModeDestructionVars			hard;
};

struct AmaxRaceModeCheckpointVars
{
	XtF32		initialTime;
	XtF32		stopwatchIncTime;
	XtArray< XtF32 >	timeAwarded;

//	XtF32		bronzeTimeRemaining;
	XtF32		silverTimeRemaining;
	XtF32		goldTimeRemaining;
};

struct AmaxRaceModeCheckpoint : AmaxRaceModeBase
{
	AmaxRaceModeCheckpoint() { raceMode = Game::E_RaceMode_Checkpoint; }

	AmaxRaceModeCheckpointVars		easy;
	AmaxRaceModeCheckpointVars		medium;
	AmaxRaceModeCheckpointVars		hard;
};

struct AmaxRaceModeFanDemandVars
{
	XtF32		BronzeNumToComplete;
	XtF32		SilverNumToComplete;
	XtF32		GoldNumToComplete;
};

struct AmaxRaceModeFanDemand : AmaxRaceModeBase
{
	AmaxRaceModeFanDemand() { raceMode = Game::E_RaceMode_FanDemand; }

	AmaxRaceModeFanDemandVars		easy;
	AmaxRaceModeFanDemandVars		medium;
	AmaxRaceModeFanDemandVars		hard;
};

struct AmaxRaceModeBossBattle : AmaxRaceModeBase
{
	AmaxRaceModeBossBattle() { raceMode = Game::E_RaceMode_BossBattle; }

	XtF32									distanceFromMinders;
	XtF32									timeInFrontOfMinders;
	bool									wreckMindersToWin;

	XtF32									distanceFromBoss;
	XtF32									timeInFrontOfBoss;
	bool									wreckBossToWin;

//	XtHandle<RaceBookCharacter>				boss;
//	XtArray< XtHandle<RaceBookCharacter> >	minders;
};

struct AmaxEvent : AmaxGameDataObj
{
	XtHandle<AmaxRoute>				route;

	bool							allocateCash;
	XtHandle<AmaxEvent>				moveToEv;
	XtLocId							postRaceMessage;

	int								laps;
	bool							aiDontPickupPickups;
	float							pickupDisableTime;
	int								pickupSpawnLevel;

	XtHandle<PickupProbabilityList>	pickupProbabilities;
	AmaxCarClass					carClass;
	bool							disableFans;
	bool							specificAudioLinesOnly;

	bool							pickupsDoNotRespawn;
	bool							pickupsDropOnUse;
	bool							dontShowStartFinishInfo;
	float							dialogueProbability;
	RacerNet::LeaderboardSettings   mLeaderboardSettings;

	AmaxRaceModeBase*				raceMode;

	DifficultySettings				easyVersion;
	DifficultySettings				normalVersion;
	DifficultySettings				hardVersion;

	XtArray< XtHandle<C_LevelActorCreationList> > actorLists;

	XtArray<AmaxOpponentSetup> opponents;

	int								mAudioMusicID;

	bool							overrideAISurface;
	float							overrideAISurfaceValue;
};

struct AmaxSplitScreenEvent : public AmaxEvent
{
	U32											mFansToUnlock;

	bool										mAllowRoute;
	bool										mAllowCarClass;
	bool										mAllowLaps;
	bool										mAllowPadAIVehicles;
	bool										mDisableRacingContextFanRewards;
	bool										mDisableWorldHUDPositionInfo;
	bool										mAllowHandicap;
	bool										mNoRollingStart;
	bool										mAllowPickupsOnLapZero;
	XtSz8										mIconSheet;
	XtSz8										mIconName;
	XtLocId										mModeName;
	bool										mDisableRaceLineUIElement;
	bool										mShufflePowerupsOnRespawn;
};

class C_GameEventData
{
public:
	void Init();
	void Update();

	static C_GameEventData& Instance() {return s_instance;}

	AmaxGameData<AmaxWinCondition*> mWinConditions;
	AmaxGameData<AmaxEvent*> mEvents;
	AmaxGameData<AmaxSplitScreenEvent*> mSplitScreenEvents;

	XtPack*	mXtAmaxPack;

private:
	static C_GameEventData s_instance;
};

#endif // GAME_EVENTDATA_H
