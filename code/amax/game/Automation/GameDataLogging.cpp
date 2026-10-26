
#include "stdafx.h"

#include "system\datalogging\DataLogging.h"
#include "GameDataLogging.h"

#include <GameApp.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/Structures/MpResults.h>
#include <Gamelogic\Gamedata\GameVehicleData.h>

#include <ui/Game/UILocaliser.h>

//#pragma optimize("", off)

namespace AmaxDataLogging
{
	Btl::List<PerkGainInfo>		g_PerkUsedList;

	// Handle which is used to identify which connection we want to use to write data
	int gDataLogHandle = -1;

	bool g_bDoInGameDataLogging = false;

	const char* gGamerTagTemp = NULL;

// Special table for adding data to the subsession
struct DlSubSession
{
	DataLog::DataLogString*	m_dltGamerTag;
} gDlSubSession = { NULL };

struct DlTimeOnScreen
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltTime;
	DataLog::DataLogString*	m_dltScreenName;
} gDlTimeOnScreen = { NULL };

struct DlEventSkipped
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltCityName;	
	DataLog::DataLogString*	m_dltRouteName;	
	DataLog::DataLogS32*	m_dltNumLaps;	
} gDlEventSkipped = { NULL };

struct DlRaceInfo
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltGameMode;
	DataLog::DataLogU32*	m_dltEventId;
	DataLog::DataLogU32*	m_dltLocationId;
	DataLog::DataLogU32*	m_dltRouteId;
	DataLog::DataLogS8*		m_dltNumCars;
	DataLog::DataLogU32*	m_dltCar0Id;
	DataLog::DataLogU32*	m_dltCar0Class;
	DataLog::DataLogU32*	m_dltCar0ShieldSlots;
	DataLog::DataLogU32*	m_dltCar0StartPosition;
	DataLog::DataLogString*	m_dltEventName;
	DataLog::DataLogU32*	m_dltDifficulty;
	DataLog::DataLogString*	m_dltRaceMode;
} gDlRaceInfo = { NULL };

struct DlRaceFinished
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogU32*	m_dltHealth;
	DataLog::DataLogF32*	m_dltDistanceToLeader;		// How far behind the leader are we?
	DataLog::DataLogF32*	m_dltDistanceToNearest;
	DataLog::DataLogU8*		m_dltPosition;				// Race position
	DataLog::DataLogBool*	m_dltPassed;				// Did we meet criteria for passing race?
	DataLog::DataLogBool*	m_dltFatallyDamaged;
	DataLog::DataLogBool*	m_dltQuit;					// Did the player quit out?
	DataLog::DataLogBool*	m_dltQuitAndWin;
	DataLog::DataLogBool*	m_dltRestart;				// Did the player restart the race?
	DataLog::DataLogU8*		m_dltNitrosPickedUp;
	DataLog::DataLogU8*		m_dltNitrosUsed;
	DataLog::DataLogU8*		m_dltClocksPickedup;

} gDlRaceFinished = { NULL };

struct DlRaceFinishedSinglePlayer
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogS32*	m_dltFans;
	DataLog::DataLogF32*	m_dltCash;
	DataLog::DataLogF32*	m_dltProgression;
	DataLog::DataLogU8*		m_dltTotalFans;

} gDlRaceFinishedSinglePlayer = { NULL };

struct DlRaceFinishedMultiPlayer
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogS32*	m_dltPosition;
	DataLog::DataLogBool*	m_dltFatallyDamaged;
	DataLog::DataLogS32*	m_dltFans;
	DataLog::DataLogS32*	m_dltCurrentRank;
	DataLog::DataLogS32*	m_dltRaceFinishedBonus;
	DataLog::DataLogS32*	m_dltModSlot1;
	DataLog::DataLogS32*	m_dltModSlot2;
	DataLog::DataLogS32*	m_dltModSlot3;
	DataLog::DataLogS32*	m_dltModSlot4;
	DataLog::DataLogS32*	m_dltModSlot5;
} gDlRaceFinishedMultiPlayer = { NULL };

struct DlRaceMultiPlayerChallengeComplete
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogS32*	m_dltChallengeComplete;
	DataLog::DataLogS32*	m_dltChallengePart;
} gDlRaceMultiPlayerChallengeComplete = { NULL };

struct DlRaceLap
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogF32*	m_dltLapTime;
	DataLog::DataLogU32*	m_dltHealth;
	DataLog::DataLogF32*	m_dltDistanceToLeader;		// How far behind the leader are we?
	DataLog::DataLogF32*	m_dltDistanceToNearest;
	DataLog::DataLogU8*		m_dltLap;
	DataLog::DataLogU8*		m_dltPosition;				// Race position

} gDlRaceLap = { NULL };

struct DlCarInfo
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;
	DataLog::DataLogF32*	m_dltSpeed;
	DataLog::DataLogS32*	m_dltLap;

	DataLog::DataLogS32*	m_dltNumA;
	DataLog::DataLogS32*	m_dltNumB;
	DataLog::DataLogS32*	m_dltNumX;
	DataLog::DataLogS32*	m_dltNumY;
	DataLog::DataLogS32*	m_dltNumLS;
	DataLog::DataLogS32*	m_dltNumRS;
	DataLog::DataLogS32*	m_dltNumLB;
	DataLog::DataLogS32*	m_dltNumRB;
	DataLog::DataLogS32*	m_dltNumLT;
	DataLog::DataLogS32*	m_dltNumRT;
	DataLog::DataLogBool*	m_dltOnGround;
	DataLog::DataLogF32*	m_dltFrameRate;
} gDlCarInfo = { NULL };

struct DlAIInfo
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogS32*	m_dltVehicleIndex;	
	DataLog::DataLogF32*	m_dltDistance;
} gDlAIInfo = { NULL };

struct DlPickups
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogU8*		m_dltPlayerIndex;

	DataLog::DataLogString*	m_dltType;
	DataLog::DataLogU8*		m_dltLevel;

	DataLog::DataLogS32*	m_dltNumSlots;
	DataLog::DataLogS32*	m_dltNumFreeSlots;

	//DataLog::DataLogU8*		m_dltCarsHit;
	//DataLog::DataLogBool*	m_dltDidHitPlayer;

	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;
} gDlPickups = { NULL };

struct DlPickupUsageGain
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltPlayerIndex;
	DataLog::DataLogString*	m_dltType;
	DataLog::DataLogF32*	m_dltDistance;
} gDlPickupUsageGain = { NULL };

struct DlRaceBookMessages
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogString*	m_dltSender;
	DataLog::DataLogString* m_dltReceiver;

	DataLog::DataLogString*	m_dltMessageName;
	DataLog::DataLogString*	m_dltMessageType;
	DataLog::DataLogString*	m_dltMessageLoc;

	DataLog::DataLogF32*	m_dltPriority;
	DataLog::DataLogF32*	m_dltProbability;
} gDlRaceBookMessages = { NULL };

struct DlInGameDialogue
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogString*	m_dltSender;
	DataLog::DataLogString*	m_dltTrigger;
	DataLog::DataLogString*	m_dltLocStr;
} gDlInGameDialogue = { NULL };

struct DlCollision
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;

	DataLog::DataLogString*	m_dltObjectA;
	DataLog::DataLogString*	m_dltObjectB;
	DataLog::DataLogF32*	m_dltSeverity;
	DataLog::DataLogS32*	m_dltHealthDeducted;

	DataLog::DataLogF32*	m_dltRaceTime;
} gDlCollision = { NULL };

struct DlReset
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogF32*	m_dltOriginalX;
	DataLog::DataLogF32*	m_dltOriginalY;
	DataLog::DataLogF32*	m_dltOriginalZ;

	DataLog::DataLogF32*	m_dltResetX;
	DataLog::DataLogF32*	m_dltResetY;
	DataLog::DataLogF32*	m_dltResetZ;

	DataLog::DataLogBool*	m_dltIsHuman;
	DataLog::DataLogBool*	m_dltManual;
} gDlReset = { NULL };

struct DlShieldedPerk
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogS32*	m_dltShieldingIdx;
	DataLog::DataLogU32*	m_dltUniqueId;
	DataLog::DataLogF32*	m_dltResponseTime;

} gDlShieldedPerk = { NULL };

struct DlShopItemAcquired
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogString*	m_dltItemName;
	DataLog::DataLogBool*	m_dltPurchased;
	DataLog::DataLogF32*	m_dltProgress;
	DataLog::DataLogString* m_dltLastEventName;

} gDlShopItemAcquired = { NULL };

struct DlVehicleAcquired
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogString*	m_dltVehicleName;
	DataLog::DataLogBool*	m_dltPurchased;
	DataLog::DataLogF32*	m_dltProgress;
	DataLog::DataLogString* m_dltLastEventName;

} gDlVehicleAcquired = { NULL };

struct DlEventUnlocked
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogString*	m_dltEventName;
	DataLog::DataLogF32*	m_dltProgress;
	DataLog::DataLogString* m_dltLastEventName;

} gDlEventUnlocked = { NULL };

struct DlShieldUse
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogBool*	m_dltStart;

} gDlShieldUse = { NULL };

struct DlPerkCausedCrash
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogS32*	m_dltHitIdx;
	DataLog::DataLogString*	m_dltPerk;

} gDlPerkCausedCrash = { NULL };

struct DlPerkCausedFlip
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogS32*	m_dltHitIdx;
	DataLog::DataLogString*	m_dltPerk;

} gDlPerkCausedFlip = { NULL };

struct DlHitByPickup
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogS32*	m_dltHitIdx;
	DataLog::DataLogU32*	m_dltUniqueId;
	DataLog::DataLogString*	m_dltPickupType;
} gDlHitByPickup = { NULL };

struct DlPickedUpPickup
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltVehicleIdx;
	DataLog::DataLogS32*	m_dltDroppedIdx;
	DataLog::DataLogString*	m_dltPickupType;
	DataLog::DataLogS32*	m_dltPickupId;
	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;
} gDlPickedUpPickup = { NULL };

struct DlPlacesGainedFromPerk
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltFiringIdx;
	DataLog::DataLogS32*	m_dltFiredPosition;
	DataLog::DataLogS32*	m_dltNewPosition;
	DataLog::DataLogString*	m_dltPerk;

} gDlPlacesGainedFromPerk = { NULL };

struct DlHealthChanged
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltVehicleIdx;
	DataLog::DataLogS32*	m_dltOldHealth;
	DataLog::DataLogS32*	m_dltNewHealth;
	DataLog::DataLogF32*	m_dltTime;

	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;
} gDlHealthChanged = { NULL };

struct DlBrakeApplied
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogS32*	m_dltControllerIdx;
	DataLog::DataLogBool*	m_dltHandbrake;
	DataLog::DataLogF32*	m_dltTime;

	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;
} gDlBrakeApplied = { NULL };

struct DlCameraChanged
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltTime;

	DataLog::DataLogS32*	m_dltControllerIdx;
} gDlCameraChanged = { NULL };

struct DlCameraDuration
{	
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString* m_dltName;
	DataLog::DataLogF32*	m_dltTime;

} gDlCameraDuration = { NULL };

struct DlCheckpoint
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltPositionX;
	DataLog::DataLogF32*	m_dltPositionY;
	DataLog::DataLogF32*	m_dltPositionZ;
	DataLog::DataLogF32*	m_dltPreviousTime;
	DataLog::DataLogF32*	m_dltNewTime;
	DataLog::DataLogF32*	m_dltTimeDifference;
	DataLog::DataLogF32*	m_dltTimeFromLastPoint;
	DataLog::DataLogS32*	m_dltZoneIndex;
	DataLog::DataLogU8*		m_dltNitrosGained;
	DataLog::DataLogU8*		m_dltNitrosUsed;
	DataLog::DataLogU8*		m_dltClocksGained;
} gDlCheckpoint = { NULL };

struct DlCheckpointMissed
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogF32*	m_dltPositionX;
	DataLog::DataLogF32*	m_dltPositionY;
	DataLog::DataLogF32*	m_dltPositionZ;
	DataLog::DataLogS32*	m_dltZonesCrossed;
	DataLog::DataLogU8*		m_dltNitrosGained;
	DataLog::DataLogU8*		m_dltNitrosUsed;
	DataLog::DataLogU8*		m_dltClocksGained;
} gDlCheckpointMissed = { NULL };

struct DlScore
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltVehicle;
	DataLog::DataLogF32*	m_dltPositionX;
	DataLog::DataLogF32*	m_dltPositionY;
	DataLog::DataLogF32*	m_dltPositionZ;
	DataLog::DataLogString*	m_dltScore;
	DataLog::DataLogString*	m_dltScoreType;
	DataLog::DataLogString*	m_dltSkill1;
	DataLog::DataLogString*	m_dltSkill2;
	DataLog::DataLogString*	m_dltSkill3;
	DataLog::DataLogString*	m_dltBonus;
	DataLog::DataLogS32*	m_dltFans;
	DataLog::DataLogS32*	m_dltChainScore;
	DataLog::DataLogS32*	m_dltChainAmount;
	DataLog::DataLogS32*	m_dltCurrentTotal;
} gDlScore = { NULL };

struct DlRetry
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogS32*	m_dltNumber;
	DataLog::DataLogString* m_dltLastEventName;

} gDlRetry = { NULL };

struct DlPlayer
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltGamertag;
	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogU32*	m_dltDriverScore;
	DataLog::DataLogF32*	m_dltPowerupHitRatio;
	DataLog::DataLogU32*	m_dltPlayTime;
	DataLog::DataLogU32*	m_dltFans;
	DataLog::DataLogF32*	m_dltAveragePos;
	DataLog::DataLogU32*	m_dltEventsPlayed;
	DataLog::DataLogU32*	m_dltEventsWon;
} gDlPlayer = { NULL };

struct DlRaceInfoMp
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogU8*		m_dltVehiclesWrecked;
	DataLog::DataLogU8*		m_dltTimesWrecked;
	DataLog::DataLogU32*	m_dltMMPoints;
	DataLog::DataLogU8*		m_dltPlayersAtStart;
	DataLog::DataLogU8*		m_dltPlayersAtEnd;
	DataLog::DataLogU8*		m_dltFinishPosition;
	DataLog::DataLogBool*	m_dltTimedOut;
	DataLog::DataLogF32*	m_dltDistanceToFinish;
	DataLog::DataLogF32*	m_dltTimeoutTime;
	DataLog::DataLogF32*	m_dltAverageLapTime;
	DataLog::DataLogU32*	m_dltRaceFans;
	DataLog::DataLogU32*	m_dltFinishBonusFans;
	DataLog::DataLogU32*	m_dltShowyFlourishFans;
	DataLog::DataLogU32*	m_dltFanFavFans;
	DataLog::DataLogU32*	m_dltChallengeFans;
	DataLog::DataLogU32*	m_dltCarRewardFans;
	DataLog::DataLogU32*	m_dltTotalFans;
	DataLog::DataLogF32*	m_dltMaxJump;
	DataLog::DataLogF32*	m_dltMaxDrift;
	DataLog::DataLogF32*	m_dltMaxSpeed;
} gDlRaceInfoMp = { NULL };

struct DlRankUpMp
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogU32*	m_dltPlayingTime;
	DataLog::DataLogU32*	m_dltTotalRaces;
} gDlRankUpMp = { NULL };

struct DlModUsageMp
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogU8*		m_dltLoadout1;
	DataLog::DataLogU8*		m_dltLoadout2;
	DataLog::DataLogU8*		m_dltLoadout3;
	DataLog::DataLogU32*	m_dltLoadoutAct1;
	DataLog::DataLogU32*	m_dltLoadoutAct2;
	DataLog::DataLogU32*	m_dltLoadoutAct3;
} gDlModUsageMp = { NULL };

struct DlRaceAwardMp
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogU8*		m_dltAward;
	DataLog::DataLogF32*	m_dltScore;
} gDlAwardMp = { NULL };

struct DlChallengeCompletedMp
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogU32*	m_dltCareerRaces;
	DataLog::DataLogU32*	m_dltCareerTime;
	DataLog::DataLogU8*		m_dltChallenge;
	DataLog::DataLogU8*		m_dltStage;
	DataLog::DataLogU32*	m_dltVehicle;
} gDlChallengeMp = { NULL };

struct DlChallengePackCompletedMp
{
	DataLog::DataLogTable*	m_dlt;

	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogU32*	m_dltCareerRaces;
	DataLog::DataLogU32*	m_dltCareerTime;
	DataLog::DataLogU8*		m_dltChallengePack;
	DataLog::DataLogU32*	m_dltVehicle;
} gDlChallengePackMp = { NULL };

struct DlVoteResultsMp
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU32*	m_dltRoute;
	DataLog::DataLogU8*		m_dltVotes;
} gDlVoteResultsMp = { NULL };

struct DlWinningVoteCastMp
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU32*	m_dltRoute;
} gDlWinningVoteCastMp = { NULL };

struct DlRaceInfoSp
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltEventName;
	DataLog::DataLogString*	m_dltEventType;
	DataLog::DataLogU8*		m_dltDifficulty;
	DataLog::DataLogU8*		m_dltRank;
	DataLog::DataLogBool*	m_dltPassed;
	DataLog::DataLogU8*		m_dltTimesWrecked;
	DataLog::DataLogU8*		m_dltFinishPosition;
	DataLog::DataLogU32*	m_dltRaceFans;
} gDlRaceInfoSp = { NULL };

struct DlWrecked
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltVehicle;
	DataLog::DataLogF32*	m_dltPositionX;
	DataLog::DataLogF32*	m_dltPositionY;
	DataLog::DataLogF32*	m_dltPositionZ;
	DataLog::DataLogF32*	m_dltRaceTime;
	DataLog::DataLogS8*		m_dltWrecker;
} gDlWrecked = { NULL };

struct DlDestruction
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU32*	m_dltNormalShots;
	DataLog::DataLogU32*	m_dltLongShots;
	DataLog::DataLogU32*	m_dltSkillShots;
	DataLog::DataLogU32*	m_dltCarsWrecked;
	DataLog::DataLogU32*	m_dltTotalPoints;
	DataLog::DataLogU32*	m_dltGreenCarsWrecked;
	DataLog::DataLogU32*	m_dltYellowCarsWrecked;
	DataLog::DataLogU32*	m_dltRedCarsWrecked;
	DataLog::DataLogU32*	m_dltBlueCarsWrecked;
	DataLog::DataLogF32*	m_dltAccuracy;
	DataLog::DataLogU32*	m_dltTotalShots;
	DataLog::DataLogU32*	m_dltBonusPoints;
} gDlDestruction = { NULL };

struct DlFanDemand
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString* m_dltType;
	DataLog::DataLogU32*	m_dltFans;
	DataLog::DataLogBool*	m_dltPassed;
} gDlFanDemand = { NULL };

struct DlSpEventProgress
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString* m_dltEventName;
	DataLog::DataLogU8*		m_dltLightsEarnt;
	DataLog::DataLogS32*	m_dltTotalFansBefore;
	DataLog::DataLogS32*	m_dltTotalFansAfter;
	DataLog::DataLogF32*	m_dltTotalPlaytimeBefore;
	DataLog::DataLogF32*	m_dltTotalPlaytimeAfter;
	DataLog::DataLogU8*		m_dltLightsGainedInEvent; //Irrelevant of whether we gained them already in previous attempt
} gDlSpEventProgress = { NULL };

struct DlSpTierProgress
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltTier;
	DataLog::DataLogF32*	m_dltTotalPlaytime;
} gDlSpTierProgress = { NULL };

struct DlSpBossUnlocked
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltBossTier;
	DataLog::DataLogF32*	m_dltTotalPlaytime;
} gDlSpBossUnlocked = { NULL };

struct DlSpDifficulty
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltScreen;
	DataLog::DataLogU8*		m_dltOldDifficulty;
	DataLog::DataLogU8*		m_dltNewDifficulty;
} gDlSpDifficulty = { NULL };

struct DlSpBossCompletion
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltBossName;
	DataLog::DataLogBool*	m_dltWreckedBoss;
} gDlSpBossCompletion = { NULL };

struct DlControllerLayoutChanged
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltOldLayout;
	DataLog::DataLogU8*		m_dltNewLayout;
} gDlControllerLayoutChanged = { NULL };

struct DlBrightnessChanged
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU8*		m_dltOldBrightness;
	DataLog::DataLogU8*		m_dltNewBrightness;
} gDlBrightnessChanged = { NULL };

struct DlRegionLanguageUsed
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogString*	m_dltRegion;
	DataLog::DataLogString*	m_dltLanguage;
} gDlRegionLanguageUsed = { NULL };

struct DlDisplaySettings
{
	DataLog::DataLogTable*	m_dlt;
	DataLog::DataLogU32*	m_dltWidth;
	DataLog::DataLogU32*	m_dltHeight;
} gDlDisplaySettings = { NULL };

void CreateTables()
{
	if( gDlTimeOnScreen.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "TimeOnScreen");

		gDlTimeOnScreen.m_dltScreenName			= dlt->AddString("ScreenName", 64);
		gDlTimeOnScreen.m_dltTime				= dlt->AddF32("Time");

		dlt->Finalise();

		gDlTimeOnScreen.m_dlt					= dlt;
	}

	if( gDlEventSkipped.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "EventSkipped", false);

		gDlEventSkipped.m_dltCityName			= dlt->AddString("CityName", 32);
		gDlEventSkipped.m_dltRouteName			= dlt->AddString("RouteName", 64);
		gDlEventSkipped.m_dltNumLaps			= dlt->AddS32("NumLaps");

		dlt->Finalise();

		gDlEventSkipped.m_dlt					= dlt;
	}

	if (gDlRaceInfo.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceInfo");

		gDlRaceInfo.m_dltGameMode			= dlt->AddString("GameMode", 16);
		gDlRaceInfo.m_dltEventId			= dlt->AddU32("EventId");
		gDlRaceInfo.m_dltLocationId			= dlt->AddU32("LocationId");
		gDlRaceInfo.m_dltRouteId			= dlt->AddU32("RouteId");
		gDlRaceInfo.m_dltNumCars			= dlt->AddS8("NumCars");
		gDlRaceInfo.m_dltCar0Id				= dlt->AddU32("Car0Id");
		gDlRaceInfo.m_dltCar0Class			= dlt->AddU32("Car0Class");
		gDlRaceInfo.m_dltCar0ShieldSlots	= dlt->AddU32("Car0ShieldSlots");
		gDlRaceInfo.m_dltCar0StartPosition	= dlt->AddU32("Car0StartPosition");
		gDlRaceInfo.m_dltEventName			= dlt->AddString("MPEventName", 32);
		gDlRaceInfo.m_dltDifficulty			= dlt->AddU32("Difficulty");
		gDlRaceInfo.m_dltRaceMode			= dlt->AddString("RaceMode", 32);


		dlt->Finalise();
		
		gDlRaceInfo.m_dlt			= dlt;
	}

	if (gDlRaceFinished.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceFinished");

		gDlRaceFinished.m_dltFrameTime			= dlt->AddF32("FrameTime");
		gDlRaceFinished.m_dltHealth				= dlt->AddU32("Health");
		gDlRaceFinished.m_dltDistanceToLeader	= dlt->AddF32("DistanceLeader");
		gDlRaceFinished.m_dltDistanceToNearest	= dlt->AddF32("DistanceNearest");
		gDlRaceFinished.m_dltPosition			= dlt->AddU8("RacePosition");
		gDlRaceFinished.m_dltPassed				= dlt->AddBool("Passed");
		gDlRaceFinished.m_dltFatallyDamaged		= dlt->AddBool("FatallyDamaged");
		gDlRaceFinished.m_dltQuit				= dlt->AddBool("Quit");
		gDlRaceFinished.m_dltQuitAndWin			= dlt->AddBool("QuitAndWin");
		gDlRaceFinished.m_dltRestart			= dlt->AddBool("Restart");
		gDlRaceFinished.m_dltNitrosPickedUp		= dlt->AddU8("NitrosCollected");
		gDlRaceFinished.m_dltNitrosUsed			= dlt->AddU8("NitrosUsed");
		gDlRaceFinished.m_dltClocksPickedup		= dlt->AddU8("ClocksPickedup");

		dlt->Finalise();

		gDlRaceFinished.m_dlt = dlt;
	}

	if (gDlRaceFinishedSinglePlayer.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceFinishedSinglePlayer");

		gDlRaceFinishedSinglePlayer.m_dltCash			= dlt->AddF32("Cash");
		gDlRaceFinishedSinglePlayer.m_dltFans			= dlt->AddS32("Fans");
		gDlRaceFinishedSinglePlayer.m_dltProgression	= dlt->AddF32("Progression");
		gDlRaceFinishedSinglePlayer.m_dltTotalFans		= dlt->AddU8("TotalFans");

		dlt->Finalise();

		gDlRaceFinishedSinglePlayer.m_dlt = dlt;
	}

	if (gDlRaceFinishedMultiPlayer.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceFinishedMultiPlayer");

		gDlRaceFinishedMultiPlayer.m_dltPosition			= dlt->AddS32("Position");
		gDlRaceFinishedMultiPlayer.m_dltFatallyDamaged		= dlt->AddBool("FatallyDamaged");
		gDlRaceFinishedMultiPlayer.m_dltFans				= dlt->AddS32("Fans");
		gDlRaceFinishedMultiPlayer.m_dltRaceFinishedBonus	= dlt->AddS32("RaceFinishedBonus");
		gDlRaceFinishedMultiPlayer.m_dltCurrentRank			= dlt->AddS32("CurrentRank");
		gDlRaceFinishedMultiPlayer.m_dltModSlot1			= dlt->AddS32("ModSlot1");
		gDlRaceFinishedMultiPlayer.m_dltModSlot2			= dlt->AddS32("ModSlot2");
		gDlRaceFinishedMultiPlayer.m_dltModSlot3			= dlt->AddS32("ModSlot3");
		gDlRaceFinishedMultiPlayer.m_dltModSlot4			= dlt->AddS32("ModSlot4");
		gDlRaceFinishedMultiPlayer.m_dltModSlot5			= dlt->AddS32("ModSlot5");

		dlt->Finalise();

		gDlRaceFinishedMultiPlayer.m_dlt = dlt;
	}

	if (gDlRaceMultiPlayerChallengeComplete.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "MultiPlayerChallengeComplete");

		gDlRaceMultiPlayerChallengeComplete.m_dltChallengeComplete		= dlt->AddS32("ChallengeId");
		gDlRaceMultiPlayerChallengeComplete.m_dltChallengePart			= dlt->AddS32("Part");

		dlt->Finalise();

		gDlRaceMultiPlayerChallengeComplete.m_dlt = dlt;
	}

	if (gDlRaceLap.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "EndOfLap");

		gDlRaceLap.m_dltFrameTime			= dlt->AddF32("FrameTime");
		gDlRaceLap.m_dltLapTime				= dlt->AddF32("LapTime");
		gDlRaceLap.m_dltHealth				= dlt->AddU32("Health");
		gDlRaceLap.m_dltDistanceToLeader	= dlt->AddF32("DistanceLeader");
		gDlRaceLap.m_dltDistanceToNearest	= dlt->AddF32("DistanceNearest");
		gDlRaceLap.m_dltLap					= dlt->AddU8("Lap");
		gDlRaceLap.m_dltPosition			= dlt->AddU8("RacePosition");

		dlt->Finalise();

		gDlRaceLap.m_dlt = dlt;
	}

	if (gDlCarInfo.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PlayerPos");

		gDlCarInfo.m_dltFrameTime	= dlt->AddF32("FrameTime");
		gDlCarInfo.m_dltX			= dlt->AddF32("X");
		gDlCarInfo.m_dltY			= dlt->AddF32("Y");
		gDlCarInfo.m_dltZ			= dlt->AddF32("Z");
		gDlCarInfo.m_dltSpeed		= dlt->AddF32("SpeedMPH");
		gDlCarInfo.m_dltLap			= dlt->AddS32("Lap");
		
		gDlCarInfo.m_dltNumA		= dlt->AddS32("NumA");
		gDlCarInfo.m_dltNumB		= dlt->AddS32("NumB");
		gDlCarInfo.m_dltNumX		= dlt->AddS32("NumX");
		gDlCarInfo.m_dltNumY		= dlt->AddS32("NumY");
		gDlCarInfo.m_dltNumLS		= dlt->AddS32("NumLS");
		gDlCarInfo.m_dltNumRS		= dlt->AddS32("NumRS");
		gDlCarInfo.m_dltNumLB		= dlt->AddS32("NumLB");
		gDlCarInfo.m_dltNumRB		= dlt->AddS32("NumRB");
		gDlCarInfo.m_dltNumLT		= dlt->AddS32("NumLT");
		gDlCarInfo.m_dltNumRT		= dlt->AddS32("NumRT");
		gDlCarInfo.m_dltOnGround	= dlt->AddBool("OnGround");
		gDlCarInfo.m_dltFrameRate	= dlt->AddF32("FrameRate");
		
		dlt->Finalise();
		
		gDlCarInfo.m_dlt			= dlt;
	}	

	if (gDlAIInfo.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "DistancesFromPlayer");

		gDlAIInfo.m_dltDistance	= dlt->AddF32("Distance");
		gDlAIInfo.m_dltVehicleIndex	= dlt->AddS32("Index");
		gDlAIInfo.m_dltFrameTime = dlt->AddF32("FrameTime");

		dlt->Finalise();

		gDlAIInfo.m_dlt			= dlt;
	}	

	if (gDlPickups.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PerkUse");

		gDlPickups.m_dltFrameTime		= dlt->AddF32("FrameTime");
		gDlPickups.m_dltX				= dlt->AddF32("X");
		gDlPickups.m_dltY				= dlt->AddF32("Y");
		gDlPickups.m_dltZ				= dlt->AddF32("Z");

		gDlPickups.m_dltNumSlots		= dlt->AddS32("NumPerkSlots");
		gDlPickups.m_dltNumFreeSlots	= dlt->AddS32("NumFreePerkSlots");

		gDlPickups.m_dltPlayerIndex		= dlt->AddU8("Index");
		gDlPickups.m_dltType			= dlt->AddString("Type", 12);
		gDlPickups.m_dltLevel			= dlt->AddU8("Level");

		dlt->Finalise();
		
		gDlPickups.m_dlt				= dlt;
	}

	if (gDlPickupUsageGain.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PerkUsageGain");

		gDlPickupUsageGain.m_dltPlayerIndex	= dlt->AddU8("Index");
		gDlPickupUsageGain.m_dltDistance	= dlt->AddF32("Distance");
		gDlPickupUsageGain.m_dltType		= dlt->AddString("Type", 12);

		dlt->Finalise();

		gDlPickupUsageGain.m_dlt			= dlt;
	}

	if (gDlRaceBookMessages.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceBookMessages", false);

		gDlRaceBookMessages.m_dltReceiver		= dlt->AddString("Receiver", 16);
		gDlRaceBookMessages.m_dltSender			= dlt->AddString("Sender", 16);

		gDlRaceBookMessages.m_dltMessageName	= dlt->AddString("MessageName", 32);
		gDlRaceBookMessages.m_dltMessageType	= dlt->AddString("MessageType", 32);
		gDlRaceBookMessages.m_dltMessageLoc		= dlt->AddString("MessageLoc", 512);

		gDlRaceBookMessages.m_dltPriority		= dlt->AddF32("Priority");
		gDlRaceBookMessages.m_dltProbability	= dlt->AddF32("Probability");

		dlt->Finalise();

		gDlRaceBookMessages.m_dlt				= dlt;
	}

	if (gDlInGameDialogue.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "InGameDialogue");

		gDlInGameDialogue.m_dltSender			= dlt->AddString("Sender", 16);
		gDlInGameDialogue.m_dltTrigger			= dlt->AddString("Trigger", 32);
		gDlInGameDialogue.m_dltLocStr			= dlt->AddString("Dialogue", 64);

		dlt->Finalise();

		gDlInGameDialogue.m_dlt					= dlt;
	}

	if (gDlCollision.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "Collisions");

		gDlCollision.m_dltObjectA				= dlt->AddString("ObjectA", 16);
		gDlCollision.m_dltObjectB				= dlt->AddString("ObjectB", 16);

		gDlCollision.m_dltHealthDeducted		= dlt->AddS32("HealthDeducted");
		gDlCollision.m_dltSeverity				= dlt->AddF32("Severity");

		gDlCollision.m_dltX						= dlt->AddF32("X");
		gDlCollision.m_dltY						= dlt->AddF32("Y");
		gDlCollision.m_dltZ						= dlt->AddF32("Z");

		gDlCollision.m_dltRaceTime				= dlt->AddF32("RaceTime");

		dlt->Finalise();

		gDlCollision.m_dlt						= dlt;
	}

	if (gDlReset.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "Resets");

		gDlReset.m_dltOriginalX				= dlt->AddF32("OriginalX");
		gDlReset.m_dltOriginalY				= dlt->AddF32("OriginalY");
		gDlReset.m_dltOriginalZ				= dlt->AddF32("OriginalZ");

		gDlReset.m_dltResetX				= dlt->AddF32("ResetX");
		gDlReset.m_dltResetY				= dlt->AddF32("ResetY");
		gDlReset.m_dltResetZ				= dlt->AddF32("ResetZ");

		gDlReset.m_dltIsHuman				= dlt->AddBool("IsHuman");
		gDlReset.m_dltManual				= dlt->AddBool("ManualReset");

		dlt->Finalise();

		gDlReset.m_dlt						= dlt;
	}

	if (gDlShieldedPerk.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ShieldedPerk");

		gDlShieldedPerk.m_dltFiringIdx				= dlt->AddS32("FiringVehicleIndex");
		gDlShieldedPerk.m_dltShieldingIdx			= dlt->AddS32("ShieldingVehicleIndex");
		gDlShieldedPerk.m_dltUniqueId				= dlt->AddU32("PerkUniqueId");
		gDlShieldedPerk.m_dltResponseTime			= dlt->AddF32("ResponseTime");

		dlt->Finalise();

		gDlShieldedPerk.m_dlt						= dlt;
	}

	if (gDlShieldUse.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ShieldUse");

		gDlShieldUse.m_dltFiringIdx				= dlt->AddS32("VehicleIndex");
		gDlShieldUse.m_dltStart					= dlt->AddBool("IsStart");

		dlt->Finalise();

		gDlShieldUse.m_dlt						= dlt;
	}

	if (gDlShopItemAcquired.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ShopItemAcquired", false);

		gDlShopItemAcquired.m_dltItemName		= dlt->AddString("ItemName", 32);
		gDlShopItemAcquired.m_dltPurchased		= dlt->AddBool("Purchased");
		gDlShopItemAcquired.m_dltProgress		= dlt->AddF32("Progress");
		gDlShopItemAcquired.m_dltLastEventName	= dlt->AddString("LastEventName", 64);

		dlt->Finalise();

		gDlShopItemAcquired.m_dlt						= dlt;
	}

	if (gDlVehicleAcquired.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "VehicleAcquired", false);

		gDlVehicleAcquired.m_dltVehicleName		= dlt->AddString("VehicleName", 32);
		gDlVehicleAcquired.m_dltPurchased		= dlt->AddBool("Purchased");
		gDlVehicleAcquired.m_dltProgress		= dlt->AddF32("Progress");
		gDlVehicleAcquired.m_dltLastEventName	= dlt->AddString("LastEventName", 64);

		dlt->Finalise();

		gDlVehicleAcquired.m_dlt						= dlt;
	}

	if (gDlEventUnlocked.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "EventUnlocked", false);

		gDlEventUnlocked.m_dltEventName		= dlt->AddString("EventName", 64);
		gDlEventUnlocked.m_dltProgress		= dlt->AddF32("Progress");
		gDlEventUnlocked.m_dltLastEventName	= dlt->AddString("LastEventName", 64);

		dlt->Finalise();

		gDlEventUnlocked.m_dlt						= dlt;
	}

	if (gDlPerkCausedCrash.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PerkCausedCrash");

		gDlPerkCausedCrash.m_dltFiringIdx			= dlt->AddS32("FiringVehicleIndex");
		gDlPerkCausedCrash.m_dltHitIdx				= dlt->AddS32("HitVehicleIndex");
		gDlPerkCausedCrash.m_dltPerk				= dlt->AddString("Perk", 12);

		dlt->Finalise();

		gDlPerkCausedCrash.m_dlt					= dlt;
	}

	if (gDlPerkCausedFlip.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PerkCausedFlip");

		gDlPerkCausedFlip.m_dltFiringIdx			= dlt->AddS32("FiringVehicleIndex");
		gDlPerkCausedFlip.m_dltHitIdx				= dlt->AddS32("HitVehicleIndex");
		gDlPerkCausedFlip.m_dltPerk					= dlt->AddString("Perk", 12);

		dlt->Finalise();

		gDlPerkCausedFlip.m_dlt						= dlt;
	}

	if (gDlHitByPickup.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "HitByPickup");

		gDlHitByPickup.m_dltFiringIdx			= dlt->AddS32("FiringVehicleIndex");
		gDlHitByPickup.m_dltHitIdx				= dlt->AddS32("HitVehicleIndex");
		gDlHitByPickup.m_dltPickupType			= dlt->AddString("PickupType", 12);
		gDlHitByPickup.m_dltUniqueId			= dlt->AddU32("UniqueId");

		dlt->Finalise();

		gDlHitByPickup.m_dlt					= dlt;
	}

	if (gDlPickedUpPickup.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PickedUpPickup");

		gDlPickedUpPickup.m_dltVehicleIdx			= dlt->AddS32("VehicleIndex");
		gDlPickedUpPickup.m_dltDroppedIdx			= dlt->AddS32("DroppedIndex");
		gDlPickedUpPickup.m_dltPickupType			= dlt->AddString("PickupType", 12);
		gDlPickedUpPickup.m_dltPickupId				= dlt->AddS32("PickupID");
		gDlPickedUpPickup.m_dltX					= dlt->AddF32("PosX");
		gDlPickedUpPickup.m_dltY					= dlt->AddF32("PosY");
		gDlPickedUpPickup.m_dltZ					= dlt->AddF32("PosZ");

		dlt->Finalise();

		gDlPickedUpPickup.m_dlt					= dlt;
	}

	if (gDlPlacesGainedFromPerk.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PlacesGainedFromPerk");

		gDlPlacesGainedFromPerk.m_dltNewPosition		= dlt->AddS32("RacePositionAfterFiring");
		gDlPlacesGainedFromPerk.m_dltFiredPosition		= dlt->AddS32("RacePositionWhenFired");

		gDlPlacesGainedFromPerk.m_dltFiringIdx			= dlt->AddS32("VehicleIndex");
		gDlPlacesGainedFromPerk.m_dltPerk				= dlt->AddString("Perk", 12);

		dlt->Finalise();

		gDlPlacesGainedFromPerk.m_dlt					= dlt;
	}

	if (gDlHealthChanged.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "HealthChanged");

		gDlHealthChanged.m_dltNewHealth		= dlt->AddS32("NewHealth");
		gDlHealthChanged.m_dltOldHealth		= dlt->AddS32("OldHealth");
		gDlHealthChanged.m_dltVehicleIdx	= dlt->AddS32("VehicleIndex");
		gDlHealthChanged.m_dltTime			= dlt->AddF32("Time");

		gDlHealthChanged.m_dltX				= dlt->AddF32("X");
		gDlHealthChanged.m_dltY				= dlt->AddF32("Y");
		gDlHealthChanged.m_dltZ				= dlt->AddF32("Z");

		dlt->Finalise();

		gDlHealthChanged.m_dlt					= dlt;
	}

	if (gDlBrakeApplied.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "BrakeApplied");

		gDlBrakeApplied.m_dltControllerIdx		= dlt->AddS32("ControllerIndex");
		gDlBrakeApplied.m_dltHandbrake			= dlt->AddBool("IsHandbrake");
		gDlBrakeApplied.m_dltTime				= dlt->AddF32("Time");

		gDlBrakeApplied.m_dltX					= dlt->AddF32("X");
		gDlBrakeApplied.m_dltY					= dlt->AddF32("Y");
		gDlBrakeApplied.m_dltZ					= dlt->AddF32("Z");

		dlt->Finalise();

		gDlBrakeApplied.m_dlt					= dlt;
	}

	if (gDlCameraChanged.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "CameraChanged");

		gDlCameraChanged.m_dltControllerIdx	= dlt->AddS32("ControllerIndex");
		gDlCameraChanged.m_dltTime			= dlt->AddF32("Time");

		dlt->Finalise();

		gDlCameraChanged.m_dlt					= dlt;
	}

	if (gDlCameraDuration.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "CameraDuration");

		gDlCameraDuration.m_dltName			= dlt->AddString("Type", 64);
		gDlCameraDuration.m_dltTime			= dlt->AddF32("Duration");

		dlt->Finalise();

		gDlCameraDuration.m_dlt					= dlt;
	}

	if (gDlCheckpoint.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "CheckpointReached");

		gDlCheckpoint.m_dltPositionX			= dlt->AddF32("PosX");
		gDlCheckpoint.m_dltPositionY			= dlt->AddF32("PosY");
		gDlCheckpoint.m_dltPositionZ			= dlt->AddF32("PosZ");
		gDlCheckpoint.m_dltPreviousTime			= dlt->AddF32("PreviousTime");
		gDlCheckpoint.m_dltNewTime				= dlt->AddF32("NewTime");
		gDlCheckpoint.m_dltTimeDifference		= dlt->AddF32("TimeDifference");
		gDlCheckpoint.m_dltTimeFromLastPoint	= dlt->AddF32("TimeFromLastPoint");
		gDlCheckpoint.m_dltZoneIndex			= dlt->AddS32("CheckpointId");
		gDlCheckpoint.m_dltNitrosGained			= dlt->AddU8("NitrosGained");
		gDlCheckpoint.m_dltNitrosUsed			= dlt->AddU8("NitrosUsed");
		gDlCheckpoint.m_dltClocksGained			= dlt->AddU8("ClocksGained");

		dlt->Finalise();

		gDlCheckpoint.m_dlt						= dlt;
	}

	if (gDlCheckpointMissed.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "CheckpointMissed");

		gDlCheckpointMissed.m_dltPositionX		= dlt->AddF32("PosX");
		gDlCheckpointMissed.m_dltPositionY		= dlt->AddF32("PosY");
		gDlCheckpointMissed.m_dltPositionZ		= dlt->AddF32("PosZ");
		gDlCheckpointMissed.m_dltZonesCrossed	= dlt->AddS32("ZonesCrossed");
		gDlCheckpointMissed.m_dltNitrosGained	= dlt->AddU8("NitrosGained");
		gDlCheckpointMissed.m_dltNitrosUsed		= dlt->AddU8("NitrosUsed");
		gDlCheckpointMissed.m_dltClocksGained	= dlt->AddU8("ClocksGained");

		dlt->Finalise();

		gDlCheckpointMissed.m_dlt				= dlt;
	}

	if (gDlScore.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "NewScore");

		gDlScore.m_dltVehicle			= dlt->AddU8("Vehicle");
		gDlScore.m_dltPositionX			= dlt->AddF32("PosX");
		gDlScore.m_dltPositionY			= dlt->AddF32("PosY");
		gDlScore.m_dltPositionZ			= dlt->AddF32("PosZ");
		gDlScore.m_dltScore				= dlt->AddString("Base", 32);
		gDlScore.m_dltScoreType			= dlt->AddString("BaseType", 8);
		gDlScore.m_dltSkill1			= dlt->AddString("Skill1", 32);
		gDlScore.m_dltSkill2			= dlt->AddString("Skill2", 32);
		gDlScore.m_dltSkill3			= dlt->AddString("Skill3", 32);
		gDlScore.m_dltBonus				= dlt->AddString("Bonus", 32);
		gDlScore.m_dltFans				= dlt->AddS32("Fans");
		gDlScore.m_dltChainScore		= dlt->AddS32("ChainFans");
		gDlScore.m_dltChainAmount		= dlt->AddS32("ChainAmount");
		gDlScore.m_dltCurrentTotal		= dlt->AddS32("CurrentTotal");

		dlt->Finalise();

		gDlScore.m_dlt						= dlt;
	}

	if (gDlRetry.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "Retry", false);

		gDlRetry.m_dltNumber		= dlt->AddS32("Number");
		gDlRetry.m_dltLastEventName	= dlt->AddString("LastEventName", 64);

		dlt->Finalise();

		gDlRetry.m_dlt						= dlt;
	}

	if (gDlPlayer.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PlayerMp");

		gDlPlayer.m_dltGamertag				= dlt->AddString("Gamertag", 32);
		gDlPlayer.m_dltRank					= dlt->AddU8("Rank");
		gDlPlayer.m_dltDriverScore			= dlt->AddU32("DriverScore");
		gDlPlayer.m_dltPowerupHitRatio		= dlt->AddF32("PowerupHitRatio");
		gDlPlayer.m_dltPlayTime				= dlt->AddU32("PlayTime");
		gDlPlayer.m_dltFans					= dlt->AddU32("Fans");
		gDlPlayer.m_dltAveragePos			= dlt->AddF32("AveragePos");
		gDlPlayer.m_dltEventsPlayed			= dlt->AddU32("EventsPlayed");
		gDlPlayer.m_dltEventsWon			= dlt->AddU32("EventsWon");

		dlt->Finalise();

		gDlPlayer.m_dlt						= dlt;
	}

	if( gDlRaceInfoMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceInfoMp");

		gDlRaceInfoMp.m_dltRank				= dlt->AddU8("Rank");
		gDlRaceInfoMp.m_dltVehiclesWrecked	= dlt->AddU8("VehiclesWrecked");
		gDlRaceInfoMp.m_dltTimesWrecked		= dlt->AddU8("TimesWrecked");
		gDlRaceInfoMp.m_dltMMPoints			= dlt->AddU32("MotorMashPoints");
		gDlRaceInfoMp.m_dltPlayersAtStart	= dlt->AddU8("PlayersAtStart");
		gDlRaceInfoMp.m_dltPlayersAtEnd		= dlt->AddU8("PlayersAtEnd");
		gDlRaceInfoMp.m_dltFinishPosition	= dlt->AddU8("FinishPosition");
		gDlRaceInfoMp.m_dltTimedOut			= dlt->AddBool("TimedOut");
		gDlRaceInfoMp.m_dltDistanceToFinish	= dlt->AddF32("DistanceToFinish");
		gDlRaceInfoMp.m_dltTimeoutTime		= dlt->AddF32("TimeoutTime");
		gDlRaceInfoMp.m_dltAverageLapTime	= dlt->AddF32("AverageLapTime");
		gDlRaceInfoMp.m_dltRaceFans			= dlt->AddU32("RaceFans");
		gDlRaceInfoMp.m_dltFinishBonusFans	= dlt->AddU32("FinishBonusFans");
		gDlRaceInfoMp.m_dltShowyFlourishFans= dlt->AddU32("ShowyFlourishFans");
		gDlRaceInfoMp.m_dltFanFavFans		= dlt->AddU32("FanFavouriteFans");
		gDlRaceInfoMp.m_dltChallengeFans	= dlt->AddU32("ChallengeFans");
		gDlRaceInfoMp.m_dltCarRewardFans	= dlt->AddU32("CarRewardFans");
		gDlRaceInfoMp.m_dltTotalFans		= dlt->AddU32("TotalCarFans");
		gDlRaceInfoMp.m_dltMaxJump			= dlt->AddF32("MaxJump");
		gDlRaceInfoMp.m_dltMaxDrift			= dlt->AddF32("MaxDrift");
		gDlRaceInfoMp.m_dltMaxSpeed			= dlt->AddF32("MaxSpeed");

		dlt->Finalise();

		gDlRaceInfoMp.m_dlt					= dlt;
	}

	if( gDlRankUpMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RankUpMp");

		gDlRankUpMp.m_dltRank				= dlt->AddU8("Rank");
		gDlRankUpMp.m_dltPlayingTime		= dlt->AddU32("TotalPlayTime");
		gDlRankUpMp.m_dltTotalRaces			= dlt->AddU32("TotalRaces");

		dlt->Finalise();

		gDlRankUpMp.m_dlt					= dlt;
	}

	if ( gDlModUsageMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ModUsageMp");

		gDlModUsageMp.m_dltLoadout1			= dlt->AddU8("LoadoutSlot1");
		gDlModUsageMp.m_dltLoadout2			= dlt->AddU8("LoadoutSlot2");
		gDlModUsageMp.m_dltLoadout3			= dlt->AddU8("LoadoutSlot3");
		gDlModUsageMp.m_dltLoadoutAct1		= dlt->AddU32("LoadoutTimes1");
		gDlModUsageMp.m_dltLoadoutAct2		= dlt->AddU32("LoadoutTimes2");
		gDlModUsageMp.m_dltLoadoutAct3		= dlt->AddU32("LoadoutTimes3");

		dlt->Finalise();

		gDlModUsageMp.m_dlt					= dlt;
	}

	if ( gDlAwardMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceAwardsMp");

		gDlAwardMp.m_dltAward				= dlt->AddU8("Award");
		gDlAwardMp.m_dltScore				= dlt->AddF32("Score");

		dlt->Finalise();

		gDlAwardMp.m_dlt					= dlt;
	}

	if ( gDlChallengeMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ChallengeCompletedMp");

		gDlChallengeMp.m_dltRank			= dlt->AddU8("Rank");
		gDlChallengeMp.m_dltCareerRaces		= dlt->AddU32("CareerRaces");
		gDlChallengeMp.m_dltCareerTime		= dlt->AddU32("CareerTime");
		gDlChallengeMp.m_dltChallenge		= dlt->AddU8("Challenge");
		gDlChallengeMp.m_dltStage			= dlt->AddU8("Stage");
		gDlChallengeMp.m_dltVehicle			= dlt->AddU32("VehicleId");

		dlt->Finalise();

		gDlChallengeMp.m_dlt				= dlt;
	}

	if ( gDlChallengePackMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ChallengePackCompletedMp");

		gDlChallengePackMp.m_dltRank		= dlt->AddU8("Rank");
		gDlChallengePackMp.m_dltCareerRaces	= dlt->AddU32("CareerRaces");
		gDlChallengePackMp.m_dltCareerTime	= dlt->AddU32("CareerTime");
		gDlChallengePackMp.m_dltChallengePack= dlt->AddU8("ChallengePack");
		gDlChallengePackMp.m_dltVehicle		= dlt->AddU32("VehicleId");

		dlt->Finalise();

		gDlChallengePackMp.m_dlt			= dlt;
	}

	if ( gDlVoteResultsMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "VoteResultsMp");

		gDlVoteResultsMp.m_dltRoute			= dlt->AddU32("Route");
		gDlVoteResultsMp.m_dltVotes			= dlt->AddU8("Votes");

		dlt->Finalise();

		gDlVoteResultsMp.m_dlt				= dlt;

	}

	if (gDlWinningVoteCastMp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "VoteMp");

		gDlWinningVoteCastMp.m_dltRoute		= dlt->AddU32("Route");

		dlt->Finalise();

		gDlWinningVoteCastMp.m_dlt			= dlt;
	}

	if(gDlRaceInfoSp.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceInfoSp");

		gDlRaceInfoSp.m_dltEventName		= dlt->AddString("EventName", 32);
		gDlRaceInfoSp.m_dltEventType		= dlt->AddString("EventType", 32);
		gDlRaceInfoSp.m_dltDifficulty		= dlt->AddU8("Difficulty");
		gDlRaceInfoSp.m_dltRank				= dlt->AddU8("CurrentRank");
		gDlRaceInfoSp.m_dltPassed			= dlt->AddBool("Passed");
		gDlRaceInfoSp.m_dltTimesWrecked		= dlt->AddU8("TimesWrecked");
		gDlRaceInfoSp.m_dltFinishPosition	= dlt->AddU8("FinishPosition");
		gDlRaceInfoSp.m_dltRaceFans			= dlt->AddU32("RaceFans");

		dlt->Finalise();

		gDlRaceInfoSp.m_dlt					= dlt;
	}

	if(gDlWrecked.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "Wrecked");

		gDlWrecked.m_dltVehicle				= dlt->AddU8("Vehicle");
		gDlWrecked.m_dltPositionX			= dlt->AddF32("PosX");
		gDlWrecked.m_dltPositionY			= dlt->AddF32("PosY");
		gDlWrecked.m_dltPositionZ			= dlt->AddF32("PosZ");
		gDlWrecked.m_dltRaceTime			= dlt->AddF32("RaceTime");
		gDlWrecked.m_dltWrecker				= dlt->AddS8("Wrecker");

		dlt->Finalise();

		gDlWrecked.m_dlt					= dlt;
	}

	if(gDlDestruction.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RaceInfoDestruction");

		gDlDestruction.m_dltNormalShots		= dlt->AddU32("NormalShots");
		gDlDestruction.m_dltLongShots		= dlt->AddU32("LongShots");
		gDlDestruction.m_dltSkillShots		= dlt->AddU32("SkillShots");
		gDlDestruction.m_dltCarsWrecked		= dlt->AddU32("CarsWrecked");
		gDlDestruction.m_dltTotalPoints		= dlt->AddU32("TotalPoints");
		gDlDestruction.m_dltGreenCarsWrecked= dlt->AddU32("GreenWrecks");
		gDlDestruction.m_dltYellowCarsWrecked= dlt->AddU32("YellowWrecks");
		gDlDestruction.m_dltRedCarsWrecked	= dlt->AddU32("RedWrecks");
		gDlDestruction.m_dltBlueCarsWrecked	= dlt->AddU32("BlueWrecks");
		gDlDestruction.m_dltAccuracy		= dlt->AddF32("Accuracy");
		gDlDestruction.m_dltTotalShots		= dlt->AddU32("TotalShots");
		gDlDestruction.m_dltBonusPoints		= dlt->AddU32("BonusPoints");

		dlt->Finalise();

		gDlDestruction.m_dlt					= dlt;
	}

	if(gDlFanDemand.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "FanDemand");

		gDlFanDemand.m_dltType				= dlt->AddString("Type", 64);
		gDlFanDemand.m_dltFans				= dlt->AddU32("Fans");
		gDlFanDemand.m_dltPassed			= dlt->AddBool("Passed");

		dlt->Finalise();

		gDlFanDemand.m_dlt						= dlt;
	}

	if(gDlSpEventProgress.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "EventProgress");

		gDlSpEventProgress.m_dltEventName			= dlt->AddString("EventName", 32);
		gDlSpEventProgress.m_dltLightsEarnt			= dlt->AddU8("LightsEarnt");
		gDlSpEventProgress.m_dltTotalFansBefore		= dlt->AddS32("TotalFansBefore");
		gDlSpEventProgress.m_dltTotalFansAfter		= dlt->AddS32("TotalFansAfter");
		gDlSpEventProgress.m_dltTotalPlaytimeBefore	= dlt->AddF32("TotalPlaytimeBefore");
		gDlSpEventProgress.m_dltTotalPlaytimeAfter	= dlt->AddF32("TotalPlaytimeAfter");
		gDlSpEventProgress.m_dltLightsGainedInEvent = dlt->AddU8("LightsWonThisEvent");

		dlt->Finalise();

		gDlSpEventProgress.m_dlt					= dlt;
	}

	if(gDlSpTierProgress.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "TierProgress");

		gDlSpTierProgress.m_dltTier					= dlt->AddU8("TierUnlocked");
		gDlSpTierProgress.m_dltTotalPlaytime		= dlt->AddF32("TotalPlayTime");

		dlt->Finalise();

		gDlSpTierProgress.m_dlt						= dlt;
	}

	if(gDlSpBossUnlocked.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "BossUnlocked");

		gDlSpBossUnlocked.m_dltBossTier				= dlt->AddU8("BossTier");
		gDlSpBossUnlocked.m_dltTotalPlaytime		= dlt->AddF32("TotalPlayTime");

		dlt->Finalise();

		gDlSpBossUnlocked.m_dlt						= dlt;
	}

	if(gDlSpDifficulty.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "DifficultyChoice", false);

		gDlSpDifficulty.m_dltScreen					= dlt->AddString("SelectionScreen", 64);
		gDlSpDifficulty.m_dltOldDifficulty			= dlt->AddU8("OldDifficulty");
		gDlSpDifficulty.m_dltNewDifficulty			= dlt->AddU8("NewDifficulty");

		dlt->Finalise();

		gDlSpDifficulty.m_dlt						= dlt;
	}

	if(gDlSpBossCompletion.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "BossCompletion");

		gDlSpBossCompletion.m_dltBossName			= dlt->AddString("BossName", 32);
		gDlSpBossCompletion.m_dltWreckedBoss		= dlt->AddBool("WreckedBoss");

		dlt->Finalise();

		gDlSpBossCompletion.m_dlt					= dlt;
	}

	if(gDlControllerLayoutChanged.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "ControllerLayoutChoice", false);

		gDlControllerLayoutChanged.m_dltOldLayout	= dlt->AddU8("OldLayout");
		gDlControllerLayoutChanged.m_dltNewLayout	= dlt->AddU8("NewLayout");

		dlt->Finalise();

		gDlControllerLayoutChanged.m_dlt			= dlt;
	}

	if(gDlBrightnessChanged.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "BrightnessChoice", false);

		gDlBrightnessChanged.m_dltOldBrightness		= dlt->AddU8("OldBrightness");
		gDlBrightnessChanged.m_dltNewBrightness		= dlt->AddU8("NewBrightness");

		dlt->Finalise();

		gDlBrightnessChanged.m_dlt					= dlt;
	}

	if(gDlRegionLanguageUsed.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "RegionLanguageUsed", false);

		gDlRegionLanguageUsed.m_dltRegion			= dlt->AddString("Region", 32);
		gDlRegionLanguageUsed.m_dltLanguage			= dlt->AddString("Language", 32);

		dlt->Finalise();

		gDlRegionLanguageUsed.m_dlt					= dlt;
	}

	if(gDlDisplaySettings.m_dlt == NULL )
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "DisplaySettings", false);

		gDlDisplaySettings.m_dltWidth				= dlt->AddU32("Width");
		gDlDisplaySettings.m_dltHeight				= dlt->AddU32("Height");

		dlt->Finalise();

		gDlDisplaySettings.m_dlt					= dlt;
	}
}

void GDLTableCreation(DataLog::eDataLogType type, DataLog::DataLogTable* dlt)
{
	if (type == DataLog::kDlSubSession)
	{
		gDlSubSession.m_dltGamerTag = dlt->AddString("GamerTag", 32);
	}
}

void GDLTableUpdate(DataLog::eDataLogType type, DataLog::DataLogEvent* e)
{
	if (type == DataLog::kDlSubSession)
	{
		e->Set(gDlSubSession.m_dltGamerTag, gGamerTagTemp);
	}
}

void Create()
{
	gDataLogHandle = DataLog::Connect("Amax", GDLTableCreation, GDLTableUpdate);

	CreateTables();
}

static bool gInSubSession		= false;
static bool gUploadedFinished	= false;

void LogRaceStart(const char* gameMode, const Game::C_LevelState *levelState, const char* gamerTag, const char* eventName, const char* raceMode)
{
	BIZ_ASSERT(gInSubSession == false);
	gInSubSession = true;

	gUploadedFinished = false;

	gGamerTagTemp = gamerTag;

	// Log data about current race
	DataLog::BeginSubSession(gDataLogHandle);

	gGamerTagTemp = NULL;

	if (gDlRaceInfo.m_dlt)
	{
		DataLog::DataLogEvent e(gDlRaceInfo.m_dlt);

		e.Set(gDlRaceInfo.m_dltGameMode,			gameMode);
		e.Set(gDlRaceInfo.m_dltEventId,			levelState->mEvent);
		e.Set(gDlRaceInfo.m_dltLocationId,			levelState->mCity);
		e.Set(gDlRaceInfo.m_dltRouteId,			levelState->mRoute);
		e.Set(gDlRaceInfo.m_dltNumCars,			levelState->mNumCars);
		e.Set(gDlRaceInfo.m_dltDifficulty,			Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetDifficulty());
		e.Set(gDlRaceInfo.m_dltCar0StartPosition,	levelState->mCars[0].mStartPosition);
		e.Set(gDlRaceInfo.m_dltEventName,			eventName);
		e.Set(gDlRaceInfo.m_dltRaceMode,			raceMode);

		if (levelState->mNumCars > 0)
		{
			AmaxVehicle *av = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelState->mCars[0].mCarID];

			e.Set(gDlRaceInfo.m_dltCar0Id,				levelState->mCars[0].mCarID);
			e.Set(gDlRaceInfo.m_dltCar0Class,			av->carClass);
			e.Set(gDlRaceInfo.m_dltCar0ShieldSlots,	levelState->mCars[0].mUpgrades.NumShieldSlots());
		}
		else
		{
			e.Set(gDlRaceInfo.m_dltCar0Id,	0);
			e.Set(gDlRaceInfo.m_dltCar0Class, 0);
			e.Set(gDlRaceInfo.m_dltCar0ShieldSlots, 0);
		}

		e.Send();

		
	}

	g_PerkUsedList.Clear();
}

void LogRaceEnd(void)
{
	if (gInSubSession)
	{
		DataLog::EndSubSession(gDataLogHandle);
		gInSubSession = false;
	}
}

void LogRaceFinished(unsigned int health, unsigned int racePosition, float distanceToLeader, float distanceToNearest, bool passed, bool fatallyDamaged, bool quit, bool restart, bool quitAndWin, float time,
					 U8 nitrosCollected, U8 nitrosUsed, U8 clocksCollected)
{
	// Hack so we only log the end of a race once
	if (gInSubSession && !gUploadedFinished)
	{
		if (gDlRaceFinished.m_dlt)
		{
			DataLog::DataLogEvent e(gDlRaceFinished.m_dlt);

			e.Set(gDlRaceFinished.m_dltFrameTime,			time);
			e.Set(gDlRaceFinished.m_dltHealth,				health);
			e.Set(gDlRaceFinished.m_dltPosition,			racePosition);
			e.Set(gDlRaceFinished.m_dltPassed,				passed);
			e.Set(gDlRaceFinished.m_dltFatallyDamaged,		fatallyDamaged);
			e.Set(gDlRaceFinished.m_dltQuit,				quit);
			e.Set(gDlRaceFinished.m_dltQuitAndWin,			quitAndWin);
			e.Set(gDlRaceFinished.m_dltRestart,			restart);
			e.Set(gDlRaceFinished.m_dltDistanceToLeader,	distanceToLeader);
			e.Set(gDlRaceFinished.m_dltDistanceToNearest,	distanceToNearest);
			e.Set(gDlRaceFinished.m_dltNitrosPickedUp,		nitrosCollected);
			e.Set(gDlRaceFinished.m_dltNitrosUsed,			nitrosUsed);
			e.Set(gDlRaceFinished.m_dltClocksPickedup,		clocksCollected);

			e.Send();

			
		}

		gUploadedFinished = true;
	}
}

void LogSinglePlayerRaceFinished(int fans, float cash, float progression, U8 totalFans)
{
	if (gInSubSession && gDlRaceFinishedSinglePlayer.m_dlt)
	{
		DataLog::DataLogEvent e(gDlRaceFinishedSinglePlayer.m_dlt);

		e.Set(gDlRaceFinishedSinglePlayer.m_dltCash,			cash);
		e.Set(gDlRaceFinishedSinglePlayer.m_dltFans,			fans);
		e.Set(gDlRaceFinishedSinglePlayer.m_dltProgression,	progression);
		e.Set(gDlRaceFinishedSinglePlayer.m_dltTotalFans,		totalFans);

		e.Send();

		
	}
}

void LogMultiPlayerRaceFinished(int position, bool wrecked, int fans_scored, int current_rank, int mod1, int mod2, int mod3, int mod4, int mod5, int bonus)
{
	if (gInSubSession && gDlRaceFinishedMultiPlayer.m_dlt)
	{
		DataLog::DataLogEvent e(gDlRaceFinishedMultiPlayer.m_dlt);

		e.Set(gDlRaceFinishedMultiPlayer.m_dltPosition,			position);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltFatallyDamaged,		wrecked);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltFans,				fans_scored);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltCurrentRank,			current_rank);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltRaceFinishedBonus,	bonus);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltModSlot1,			mod1);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltModSlot2,			mod2);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltModSlot3,			mod3);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltModSlot4,			mod4);
		e.Set(gDlRaceFinishedMultiPlayer.m_dltModSlot5,			mod5);

		e.Send();

		
	}
}

void LogMultiPlayerChallengeComplete(int challenge, int part)
{
	if (gInSubSession && gDlRaceMultiPlayerChallengeComplete.m_dlt)
	{
		DataLog::DataLogEvent e(gDlRaceMultiPlayerChallengeComplete.m_dlt);

		e.Set(gDlRaceMultiPlayerChallengeComplete.m_dltChallengeComplete,		challenge);
		e.Set(gDlRaceMultiPlayerChallengeComplete.m_dltChallengePart,			part);

		e.Send();

		
	}
}

void LogRaceCancelled()
{
	// Handle case where race was ended unexpectedly
	if (gInSubSession)
	{
		DataLog::EndSubSession(gDataLogHandle);
		gInSubSession = false;
	}
}

void LogRaceLap(unsigned int health, float lapTime, unsigned int currentLap, unsigned int racePosition, float distanceToLeader, float distanceToNearest, float time)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlRaceLap.m_dlt && currentLap > 0)
	{
		DataLog::DataLogEvent e(gDlRaceLap.m_dlt);

		e.Set(gDlRaceLap.m_dltFrameTime,			time);
		e.Set(gDlRaceLap.m_dltLapTime,				lapTime);
		e.Set(gDlRaceLap.m_dltHealth,				health);
		e.Set(gDlRaceLap.m_dltLap,					currentLap);
		e.Set(gDlRaceLap.m_dltPosition,			racePosition);
		e.Set(gDlRaceLap.m_dltDistanceToLeader,	distanceToLeader);
		e.Set(gDlRaceLap.m_dltDistanceToNearest,	distanceToNearest);

		e.Send();

		
	}
}

void LogPlayerInfo(Maths::HVec3 pos, float speed, int lap, int numA, int numB, int numX, int numY, int leftStick, int rightStick, int numRB,
				   int numLB, int numLT, int numRT, float time, bool wheelsOnGround, F32 frameRate)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlCarInfo.m_dlt && lap > 0)
	{
		DataLog::DataLogEvent e(gDlCarInfo.m_dlt);

		e.Set(gDlCarInfo.m_dltFrameTime,	time);
		e.Set(gDlCarInfo.m_dltX,			pos.X());
		e.Set(gDlCarInfo.m_dltY,			pos.Y());
		e.Set(gDlCarInfo.m_dltZ,			pos.Z());
		e.Set(gDlCarInfo.m_dltLap,			lap);
		e.Set(gDlCarInfo.m_dltSpeed,		speed);
		
		e.Set(gDlCarInfo.m_dltNumA,		numA);
		e.Set(gDlCarInfo.m_dltNumB,		numB);
		e.Set(gDlCarInfo.m_dltNumX,		numX);
		e.Set(gDlCarInfo.m_dltNumY,		numY);
		e.Set(gDlCarInfo.m_dltNumLS,		leftStick);
		e.Set(gDlCarInfo.m_dltNumRS,		rightStick);
		e.Set(gDlCarInfo.m_dltNumLB,		numLB);
		e.Set(gDlCarInfo.m_dltNumRB,		numRB);
		e.Set(gDlCarInfo.m_dltNumLT,		numLT);
		e.Set(gDlCarInfo.m_dltNumRT,		numRT);
		e.Set(gDlCarInfo.m_dltOnGround,	wheelsOnGround);
		e.Set(gDlCarInfo.m_dltFrameRate,	frameRate);



		// TODO: Need player health/damage?
		//vla->GetCurrentHealth();
		//vla->GetCurrentHealthLevel();
		//vla->GetMaxHealth();

		e.Send();

		
	}
}

void LogAIInfo(int vehicleIndex, float distance, float time)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlAIInfo.m_dlt)
	{
		DataLog::DataLogEvent e(gDlAIInfo.m_dlt);

		e.Set(gDlAIInfo.m_dltFrameTime,	time);
		e.Set(gDlAIInfo.m_dltVehicleIndex,	vehicleIndex);		
		e.Set(gDlAIInfo.m_dltDistance,		distance);

		e.Send();

		
	}	
}

void LogPickupUse(Maths::HVec3 pos, U32 carIndex, const char* pickupType, unsigned int level, int numFreeSlots, int numSlots, float time) // , unsigned int carsHit, bool playerHit)
{
	if (!g_bDoInGameDataLogging)
		return;

	PerkGainInfo perkGain;

	perkGain.mTime		= 10.f;	// 10 = struct needs initialising
	perkGain.mPlayer	= carIndex;
	perkGain.mPerkType	= pickupType;
	perkGain.mDist		= 0;

	g_PerkUsedList.PushBack(perkGain);

	if (gDlPickups.m_dlt)
	{
		DataLog::DataLogEvent e(gDlPickups.m_dlt);

		e.Set(gDlPickups.m_dltFrameTime,		time);
		e.Set(gDlPickups.m_dltPlayerIndex,		carIndex);

		e.Set(gDlPickups.m_dltType,			pickupType);
		e.Set(gDlPickups.m_dltLevel,			level);

		e.Set(gDlPickups.m_dltNumSlots,		numSlots);
		e.Set(gDlPickups.m_dltNumFreeSlots,	numFreeSlots);

		//e.Set(gDlPickups.m_dltCarsHit,			carsHit);
		//e.Set(gDlPickups.m_dltDidHitPlayer,	playerHit);

		e.Set(gDlPickups.m_dltX,				pos.X());
		e.Set(gDlPickups.m_dltY,				pos.Y());
		e.Set(gDlPickups.m_dltZ,				pos.Z());

		e.Send();

		
	}
}

void LogHitByPickup(int firingIdx, int hitIdx, U32 uniqueId, const char* pickupType)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlHitByPickup.m_dlt)
	{
		DataLog::DataLogEvent e(gDlHitByPickup.m_dlt);

		e.Set(gDlHitByPickup.m_dltFiringIdx,		firingIdx);
		e.Set(gDlHitByPickup.m_dltHitIdx,			hitIdx);
		e.Set(gDlHitByPickup.m_dltUniqueId,		uniqueId);
		e.Set(gDlHitByPickup.m_dltPickupType,		pickupType);

		e.Send();

		
	}
}

void LogPickedUpPickup(int vehIdx, const char* pickupType, Maths::HVec3 pos, S32 droppedIdx, S32 pickupId)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlPickedUpPickup.m_dlt)
	{
		DataLog::DataLogEvent e(gDlPickedUpPickup.m_dlt);

		e.Set(gDlPickedUpPickup.m_dltPickupId,		pickupId);
		e.Set(gDlPickedUpPickup.m_dltPickupType,	pickupType);
		e.Set(gDlPickedUpPickup.m_dltVehicleIdx,	vehIdx);
		e.Set(gDlPickedUpPickup.m_dltDroppedIdx,	droppedIdx);
		e.Set(gDlPickedUpPickup.m_dltX,			pos.X());
		e.Set(gDlPickedUpPickup.m_dltY,			pos.Y());
		e.Set(gDlPickedUpPickup.m_dltZ,			pos.Z());

		e.Send();

		
	}
}

void LogPickupUsageGain(PerkGainInfo* gainInfo, float dist)
{
	if (!g_bDoInGameDataLogging)
		return;

	if (gDlPickupUsageGain.m_dlt)
	{
		DataLog::DataLogEvent e(gDlPickupUsageGain.m_dlt);

		e.Set(gDlPickupUsageGain.m_dltPlayerIndex,		gainInfo->mPlayer);
		e.Set(gDlPickupUsageGain.m_dltType,			gainInfo->mPerkType);
		e.Set(gDlPickupUsageGain.m_dltDistance,		gainInfo->mDist - dist);

		e.Send();

		
	}
}

void LogRaceBookMessageSent(const char* sender, const char* receiver, const char* messageName, const char* messageType, float priority, 
							float probability, const char* locStr)
{
	DataLog::DataLogEvent e(gDlRaceBookMessages.m_dlt);
	
	e.Set(gDlRaceBookMessages.m_dltReceiver,		receiver);
	e.Set(gDlRaceBookMessages.m_dltSender,			sender);

	e.Set(gDlRaceBookMessages.m_dltMessageName,	messageName);
	e.Set(gDlRaceBookMessages.m_dltMessageType,	messageType);
	e.Set(gDlRaceBookMessages.m_dltMessageLoc,		locStr);

	e.Set(gDlRaceBookMessages.m_dltPriority,		priority);
	e.Set(gDlRaceBookMessages.m_dltProbability,	probability);

	e.Send();

	
}

void LogCollision(const char* objectA, const char* objectB, Maths::HVec3 collPoint, float severity, int damage, F32 raceTime)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlCollision.m_dlt);

	e.Set(gDlCollision.m_dltObjectA,			objectA);
	e.Set(gDlCollision.m_dltObjectB,			objectB);
	e.Set(gDlCollision.m_dltSeverity,			severity);
	e.Set(gDlCollision.m_dltHealthDeducted,	damage);
	e.Set(gDlCollision.m_dltX,					collPoint.X());
	e.Set(gDlCollision.m_dltY,					collPoint.Y());
	e.Set(gDlCollision.m_dltZ,					collPoint.Z());
	e.Set(gDlCollision.m_dltRaceTime,			raceTime);

	e.Send();

	
}

void LogFireDialogue(const char* speaker, const char* trigger, const char* locStr)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlInGameDialogue.m_dlt);

	e.Set(gDlInGameDialogue.m_dltSender,			speaker);
	e.Set(gDlInGameDialogue.m_dltTrigger,			trigger);
	e.Set(gDlInGameDialogue.m_dltLocStr,			locStr);

	e.Send();

	
}

void LogReset(Maths::HVec3 originalPoint, Maths::HVec3 resetPoint, bool isHuman, bool isManual)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlReset.m_dlt);

	e.Set(gDlReset.m_dltOriginalX,			originalPoint.X());
	e.Set(gDlReset.m_dltOriginalY,			originalPoint.Y());
	e.Set(gDlReset.m_dltOriginalZ,			originalPoint.Z());
	e.Set(gDlReset.m_dltResetX,			resetPoint.X());
	e.Set(gDlReset.m_dltResetY,			resetPoint.Y());
	e.Set(gDlReset.m_dltResetZ,			resetPoint.Z());
	e.Set(gDlReset.m_dltIsHuman,			isHuman);
	e.Set(gDlReset.m_dltManual,			isManual);

	e.Send();

	
}

void LogPerkBlockedByShield(int firingIdx, int blockingIdx, U32 uniqueId, float responseTime)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlShieldedPerk.m_dlt);

	e.Set(gDlShieldedPerk.m_dltFiringIdx,			firingIdx);
	e.Set(gDlShieldedPerk.m_dltShieldingIdx,		blockingIdx);
	e.Set(gDlShieldedPerk.m_dltUniqueId,			uniqueId);
	e.Set(gDlShieldedPerk.m_dltResponseTime,		responseTime);

	e.Send();

	
}

void LogShieldUse(int vehicleIdx, bool start)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlShieldUse.m_dlt);

	e.Set(gDlShieldUse.m_dltFiringIdx,				vehicleIdx);
	e.Set(gDlShieldUse.m_dltStart,					start);

	e.Send();

	
}

void LogPerkCausedCrash(int firingIdx, int hitIdx, const char* pickup)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlPerkCausedCrash.m_dlt);

	e.Set(gDlPerkCausedCrash.m_dltFiringIdx,		firingIdx);
	e.Set(gDlPerkCausedCrash.m_dltHitIdx,			hitIdx);
	e.Set(gDlPerkCausedCrash.m_dltPerk,			pickup);
	e.Send();

	
}

void LogPerkCausedFlip(int firingIdx, int hitIdx, const char* pickup)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlPerkCausedFlip.m_dlt);

	e.Set(gDlPerkCausedFlip.m_dltFiringIdx,		firingIdx);
	e.Set(gDlPerkCausedFlip.m_dltHitIdx,			hitIdx);
	e.Set(gDlPerkCausedFlip.m_dltPerk,				pickup);

	e.Send();

	
}

void LogPlacesGainedFromPerk(int vehIdx, int oldPosition, int newPosition, const char* pickup )
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlPlacesGainedFromPerk.m_dlt);

	e.Set(gDlPlacesGainedFromPerk.m_dltFiredPosition,		oldPosition);
	e.Set(gDlPlacesGainedFromPerk.m_dltNewPosition,		newPosition);
	e.Set(gDlPlacesGainedFromPerk.m_dltFiringIdx,			vehIdx);
	e.Set(gDlPlacesGainedFromPerk.m_dltPerk,				pickup);

	e.Send();

	
}

void LogHealthChanged(int vehIdx, int prevHealth, int newHealth, Maths::HVec3 position, float time)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlHealthChanged.m_dlt);

	e.Set(gDlHealthChanged.m_dltNewHealth,		newHealth);
	e.Set(gDlHealthChanged.m_dltOldHealth,		prevHealth);
	e.Set(gDlHealthChanged.m_dltVehicleIdx,	vehIdx);
	e.Set(gDlHealthChanged.m_dltTime,			time);

	e.Set(gDlHealthChanged.m_dltX,				position.X());
	e.Set(gDlHealthChanged.m_dltY,				position.Y());
	e.Set(gDlHealthChanged.m_dltZ,				position.Z());

	e.Send();

	
}

void LogBrake(int controllerIdx, float time, Maths::HVec3 position, bool handbrake)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlBrakeApplied.m_dlt);

	e.Set(gDlBrakeApplied.m_dltControllerIdx,		controllerIdx);
	e.Set(gDlBrakeApplied.m_dltHandbrake,			handbrake);
	e.Set(gDlBrakeApplied.m_dltTime,				time);

	e.Set(gDlBrakeApplied.m_dltX,					position.X());
	e.Set(gDlBrakeApplied.m_dltY,					position.Y());
	e.Set(gDlBrakeApplied.m_dltZ,					position.Z());

	e.Send();

	
}

void LogCameraChanged(int controllerIdx, float time)
{
	if (!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlCameraChanged.m_dlt);

	e.Set(gDlCameraChanged.m_dltControllerIdx,		controllerIdx);
	e.Set(gDlCameraChanged.m_dltTime,				time);

	e.Send();

	
}

void LogCameraDuration(const char* cameraName, float time)
{
	DataLog::DataLogEvent e(gDlCameraDuration.m_dlt);

	e.Set(gDlCameraDuration.m_dltName,				cameraName);
	e.Set(gDlCameraDuration.m_dltTime,				time);

	e.Send();

	
}

void LogShopItemAcquired(const char* locStr, bool purchased, float progress, const char* lastEvStr)
{
	DataLog::DataLogEvent e(gDlShopItemAcquired.m_dlt);

	e.Set(gDlShopItemAcquired.m_dltItemName,			locStr);
	e.Set(gDlShopItemAcquired.m_dltPurchased,			purchased);
	e.Set(gDlShopItemAcquired.m_dltProgress,			progress);
	e.Set(gDlShopItemAcquired.m_dltLastEventName,		lastEvStr);

	e.Send();

	
}

void LogVehicleAcquired(const char* locStr, bool purchased, float progress, const char* lastEvStr)
{
	DataLog::DataLogEvent e(gDlVehicleAcquired.m_dlt);

	e.Set(gDlVehicleAcquired.m_dltVehicleName,			locStr);
	e.Set(gDlVehicleAcquired.m_dltPurchased,			purchased);
	e.Set(gDlVehicleAcquired.m_dltProgress,			progress);
	e.Set(gDlVehicleAcquired.m_dltLastEventName,		lastEvStr);

	e.Send();

		
}

void LogEventUnlocked(const char* locStr, float progress, const char* lastEvStr)
{
	DataLog::DataLogEvent e(gDlEventUnlocked.m_dlt);

	e.Set(gDlEventUnlocked.m_dltEventName,			locStr);
	e.Set(gDlEventUnlocked.m_dltProgress,			progress);
	e.Set(gDlEventUnlocked.m_dltLastEventName,		lastEvStr);

	e.Send();

	
}

void LogCheckpointHit(Maths::HVec3 pos, float oldTime, float newTime, float difference, float timeFromLastPoint, int zoneId, U8 nitrosGained, U8 nitrosUsed, U8 clocksGained)
{
	if(!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlCheckpoint.m_dlt);

	e.Set(gDlCheckpoint.m_dltPositionX,			pos.X());
	e.Set(gDlCheckpoint.m_dltPositionY,			pos.Y());
	e.Set(gDlCheckpoint.m_dltPositionZ,			pos.Z());
	e.Set(gDlCheckpoint.m_dltPreviousTime,			oldTime);
	e.Set(gDlCheckpoint.m_dltNewTime,				newTime);
	e.Set(gDlCheckpoint.m_dltTimeDifference,		difference);
	e.Set(gDlCheckpoint.m_dltTimeFromLastPoint,	timeFromLastPoint);
	e.Set(gDlCheckpoint.m_dltZoneIndex,			zoneId);
	e.Set(gDlCheckpoint.m_dltNitrosGained,			nitrosGained);
	e.Set(gDlCheckpoint.m_dltNitrosUsed,			nitrosUsed);
	e.Set(gDlCheckpoint.m_dltClocksGained,			clocksGained);

	e.Send();

	
}

void LogCheckpointMissed(Maths::HVec3 pos, int zonesCrossed, U8 nitrosGained, U8 nitrosUsed, U8 clocksGained)
{
	if(!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlCheckpointMissed.m_dlt);

	e.Set(gDlCheckpointMissed.m_dltPositionX,			pos.X());
	e.Set(gDlCheckpointMissed.m_dltPositionY,			pos.Y());
	e.Set(gDlCheckpointMissed.m_dltPositionZ,			pos.Z());
	e.Set(gDlCheckpointMissed.m_dltZonesCrossed,		zonesCrossed);
	e.Set(gDlCheckpointMissed.m_dltNitrosGained,		nitrosGained);
	e.Set(gDlCheckpointMissed.m_dltNitrosUsed,			nitrosUsed);
	e.Set(gDlCheckpointMissed.m_dltClocksGained,		clocksGained);

	e.Send();

	
}

void LogScore(int vehIdx, Maths::HVec3 pos, char* score, S8 scoreType, char* skill1, char* skill2, char* skill3, char* bonus, int fans, int chain_fans, int chain_amount, int current_total)
{
	if(!g_bDoInGameDataLogging)
		return;

	DataLog::DataLogEvent e(gDlScore.m_dlt);

	e.Set(gDlScore.m_dltVehicle,			vehIdx);
	e.Set(gDlScore.m_dltPositionX,			pos.X());
	e.Set(gDlScore.m_dltPositionY,			pos.Y());
	e.Set(gDlScore.m_dltPositionZ,			pos.Z());
	e.Set(gDlScore.m_dltScore,				score);
	e.Set(gDlScore.m_dltScoreType,			scoreType == 0 ? "Minor" : "Major");
	e.Set(gDlScore.m_dltSkill1,			skill1);
	e.Set(gDlScore.m_dltSkill2,			skill2);
	e.Set(gDlScore.m_dltSkill3,			skill3);
	e.Set(gDlScore.m_dltBonus,				bonus);
	e.Set(gDlScore.m_dltFans,				fans);
	e.Set(gDlScore.m_dltChainScore,		chain_fans);
	e.Set(gDlScore.m_dltChainAmount,		chain_amount);
	e.Set(gDlScore.m_dltCurrentTotal,		current_total);

	e.Send();

	
}

void LogRetry(int number, const char* lastEvStr)
{
	DataLog::DataLogEvent e(gDlRetry.m_dlt);

	e.Set(gDlRetry.m_dltNumber,			number);
	e.Set(gDlRetry.m_dltLastEventName,		lastEvStr);

	e.Send();

	
}

void LogTimeOnScreen(float time, const char* screen)
{
	DataLog::DataLogEvent e(gDlTimeOnScreen.m_dlt);

	e.Set(gDlTimeOnScreen.m_dltScreenName,			screen);
	e.Set(gDlTimeOnScreen.m_dltTime,				time);

	e.Send();

}

void LogEventSkipped(const char* cityName, const char* routeName, int numLaps)
{
	DataLog::DataLogEvent e(gDlEventSkipped.m_dlt);

	e.Set(gDlEventSkipped.m_dltCityName,			cityName);
	e.Set(gDlEventSkipped.m_dltRouteName,			routeName);
	e.Set(gDlEventSkipped.m_dltNumLaps,			numLaps);

	e.Send();

	
}

void LogPlayerCareerInformation(const char* gamertag, U8 rank, U32 driverScore, F32 powerupHitRatio, U64 playTime, U32 fans, F32 averagePos, U32 eventsPlayed, U32 eventsWon)
{
	U32 totalPlayTime = (U32)((F32)playTime / 1000.f);
	DataLog::DataLogEvent e(gDlPlayer.m_dlt);

	e.Set(gDlPlayer.m_dltGamertag,				gamertag);
	e.Set(gDlPlayer.m_dltRank,					rank);
	e.Set(gDlPlayer.m_dltDriverScore,			driverScore);
	e.Set(gDlPlayer.m_dltPowerupHitRatio,		powerupHitRatio);
	e.Set(gDlPlayer.m_dltPlayTime,				totalPlayTime);
	e.Set(gDlPlayer.m_dltFans,					fans);
	e.Set(gDlPlayer.m_dltAveragePos,			averagePos);
	e.Set(gDlPlayer.m_dltEventsPlayed,			eventsPlayed);
	e.Set(gDlPlayer.m_dltEventsWon,			eventsWon);

	e.Send();

	
}

void LogMpRaceInformation(U8 raceMode, U8 timeoutLength, F32 averageLapTime,  const Game::MpRaceSummary* summary, U8 timesWrecked, F32 distanceToFinish, bool timedOut)
{
	if(!summary)
	{
		return;
	}

	Game::MpVehiclesData& results = Game::C_MpResults::Instance().m_LiveVehicleData;
	U8 playersAtStart = 0;
	U8 playersAtEnd = 0;
	for (S8 i = 0; i < results.Size(); i++)
	{
		if (results[i].m_State == Game::E_VEHICLE_STATE_UNKNOWN)
		{
			continue;
		}

		playersAtStart++;

		if (results[i].m_State != Game::E_VEHICLE_STATE_LEFT_RACE)
		{
			playersAtEnd++;
		}
	}

	DataLog::DataLogEvent e(gDlRaceInfoMp.m_dlt);

	e.Set(gDlRaceInfoMp.m_dltRank,						summary->m_Rank);
	e.Set(gDlRaceInfoMp.m_dltVehiclesWrecked,			results[0].m_Statistics[UI::E_MultiplayerStat_TotalCarsWrecked]);
	e.Set(gDlRaceInfoMp.m_dltTimesWrecked,				timesWrecked);
	e.Set(gDlRaceInfoMp.m_dltMMPoints,					results[0].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints]);
	e.Set(gDlRaceInfoMp.m_dltPlayersAtStart,			playersAtStart);
	e.Set(gDlRaceInfoMp.m_dltPlayersAtEnd,				playersAtEnd);
	e.Set(gDlRaceInfoMp.m_dltFinishPosition,			summary->m_Position);
	e.Set(gDlRaceInfoMp.m_dltTimedOut,					timedOut);
	e.Set(gDlRaceInfoMp.m_dltDistanceToFinish,			distanceToFinish);
	e.Set(gDlRaceInfoMp.m_dltTimeoutTime,				timeoutLength);
	e.Set(gDlRaceInfoMp.m_dltAverageLapTime,			averageLapTime);
	e.Set(gDlRaceInfoMp.m_dltRaceFans,					summary->m_OnTrackFans);
	e.Set(gDlRaceInfoMp.m_dltFinishBonusFans,			summary->m_FinishBonus);
	e.Set(gDlRaceInfoMp.m_dltShowyFlourishFans,		summary->m_ShowyFlourishContribution);
	e.Set(gDlRaceInfoMp.m_dltFanFavFans,				summary->m_FanDemandContribution);
	e.Set(gDlRaceInfoMp.m_dltChallengeFans,			summary->m_TotalChallengeFans);
	e.Set(gDlRaceInfoMp.m_dltCarRewardFans,			summary->m_CarRewardFanBonus);
	e.Set(gDlRaceInfoMp.m_dltTotalFans,				summary->m_TotalNewFans);
	e.Set(gDlRaceInfoMp.m_dltMaxJump,					results[0].m_Statistics[UI::E_MultiplayerStat_LongestJump]);
	e.Set(gDlRaceInfoMp.m_dltMaxDrift,					results[0].m_Statistics[UI::E_MultiplayerStat_LongestDrift]);
	e.Set(gDlRaceInfoMp.m_dltMaxSpeed,					results[0].m_Statistics[UI::E_MultiplayerStat_TopSpeed]);

	e.Send();

	
}

void LogMpRankUp(U8 rank, U64 totalPlayTime, U32 totalRaces)
{
	U32 playTime = (U32)((F32)totalPlayTime / 1000.f);

	DataLog::DataLogEvent e(gDlRankUpMp.m_dlt);

	e.Set(gDlRankUpMp.m_dltRank,						rank);
	e.Set(gDlRankUpMp.m_dltPlayingTime,				playTime);
	e.Set(gDlRankUpMp.m_dltTotalRaces,					totalRaces);

	e.Send();

	
}

void LogMpModUsage(U8 slot1, U8 slot2, U8 slot3, U32 slotActivated1, U32 slotActivated2, U32 slotActivated3)
{
	DataLog::DataLogEvent e(gDlModUsageMp.m_dlt);

	e.Set(gDlModUsageMp.m_dltLoadout1,					slot1);	
	e.Set(gDlModUsageMp.m_dltLoadout2,					slot2);
	e.Set(gDlModUsageMp.m_dltLoadout3,					slot3);
	e.Set(gDlModUsageMp.m_dltLoadoutAct1,				slotActivated1);
	e.Set(gDlModUsageMp.m_dltLoadoutAct2,				slotActivated2);
	e.Set(gDlModUsageMp.m_dltLoadoutAct3,				slotActivated3);

	e.Send();

	
}

void LogMpAwardReceived(U8 award, F32 score)
{
	DataLog::DataLogEvent e(gDlAwardMp.m_dlt);

	e.Set(gDlAwardMp.m_dltAward,						award);
	e.Set(gDlAwardMp.m_dltScore,						score);

	e.Send();

	
}

void LogMpChallengeCompleted(U8 rank, U32 careerRaces, U64 playTime, U8 challenge, U8 stage, U32 vehicleId)
{
	U32 totalPlayTime = (U32)((F32)playTime / 1000.f);

	DataLog::DataLogEvent e(gDlChallengeMp.m_dlt);

	e.Set(gDlChallengeMp.m_dltRank,					rank);
	e.Set(gDlChallengeMp.m_dltCareerRaces,				careerRaces);
	e.Set(gDlChallengeMp.m_dltCareerTime,				totalPlayTime);
	e.Set(gDlChallengeMp.m_dltChallenge,				challenge);
	e.Set(gDlChallengeMp.m_dltStage,					stage);
	e.Set(gDlChallengeMp.m_dltVehicle,					vehicleId);

	e.Send();

	
}

void LogMpChallengePackCompleted(U8 rank, U32 careerRaces, U64 playTime, U8 challengePack, U32 vehicleId)
{
	U32 totalPlayTime = (U32)((F32)playTime / 1000.f);

	DataLog::DataLogEvent e(gDlChallengePackMp.m_dlt);

	e.Set(gDlChallengePackMp.m_dltRank,				rank);
	e.Set(gDlChallengePackMp.m_dltCareerRaces,			careerRaces);
	e.Set(gDlChallengePackMp.m_dltCareerTime,			totalPlayTime);
	e.Set(gDlChallengePackMp.m_dltChallengePack,		challengePack);
	e.Set(gDlChallengePackMp.m_dltVehicle,				vehicleId);

	e.Send();

	
}

void LogMpVoteResults(U32 route, U8 votes)
{
	DataLog::DataLogEvent e(gDlVoteResultsMp.m_dlt);

	e.Set(gDlVoteResultsMp.m_dltRoute,					route);
	e.Set(gDlVoteResultsMp.m_dltVotes,					votes);

	e.Send();

	
}

void LogMpWinningVoteCast(U32 route)
{
	DataLog::DataLogEvent e(gDlWinningVoteCastMp.m_dlt);

	e.Set(gDlWinningVoteCastMp.m_dltRoute,				route);

	e.Send();

	
}

void LogWrecked(U8 vehicle, Maths::HVec3 position, F32 raceTime, S8 wrecker)
{
	DataLog::DataLogEvent e(gDlWrecked.m_dlt);

	e.Set(gDlWrecked.m_dltVehicle,						vehicle);
	e.Set(gDlWrecked.m_dltPositionX,					position.X());
	e.Set(gDlWrecked.m_dltPositionY,					position.Y());
	e.Set(gDlWrecked.m_dltPositionZ,					position.Z());
	e.Set(gDlWrecked.m_dltRaceTime,					raceTime);
	e.Set(gDlWrecked.m_dltWrecker,						wrecker);

	e.Send();

	
}

void LogDestructionInfo(U32 normalShots, U32 longShots, U32 skillShots, U32 wrecks, U32 points, U32 greenWrecks, U32 yellowWrecks, U32 redWrecks, U32 blueWrecks, F32 accuracy, U32 totalShots, U32 bonusPoints)
{
	DataLog::DataLogEvent e(gDlDestruction.m_dlt);

	e.Set(gDlDestruction.m_dltNormalShots,				normalShots);
	e.Set(gDlDestruction.m_dltLongShots,				longShots);
	e.Set(gDlDestruction.m_dltSkillShots,				skillShots);
	e.Set(gDlDestruction.m_dltCarsWrecked,				wrecks);
	e.Set(gDlDestruction.m_dltTotalPoints,				points);
	e.Set(gDlDestruction.m_dltGreenCarsWrecked,			greenWrecks);
	e.Set(gDlDestruction.m_dltYellowCarsWrecked,		yellowWrecks);
	e.Set(gDlDestruction.m_dltRedCarsWrecked,			redWrecks);
	e.Set(gDlDestruction.m_dltBlueCarsWrecked,			blueWrecks);
	e.Set(gDlDestruction.m_dltAccuracy,					accuracy);
	e.Set(gDlDestruction.m_dltTotalShots,				totalShots);
	e.Set(gDlDestruction.m_dltBonusPoints,				bonusPoints);

	e.Send();

	
}

void LogFanDemandOver(const char* type, U32 fans, bool passed)
{
	DataLog::DataLogEvent e(gDlFanDemand.m_dlt);

	e.Set(gDlFanDemand.m_dltType,						type);
	e.Set(gDlFanDemand.m_dltFans,						fans);
	e.Set(gDlFanDemand.m_dltPassed,					passed);

	e.Send();

	
}

void LogEventStatus(const char* eventName, U8 lightsEarnt, S32 oldFanTotal, S32 newFanTotal, F32 oldPlaytime, F32 newPlaytime, U8 lightsGainedThisEvent)
{
	DataLog::DataLogEvent e(gDlSpEventProgress.m_dlt);

	e.Set(gDlSpEventProgress.m_dltEventName,			eventName);
	e.Set(gDlSpEventProgress.m_dltLightsEarnt,			lightsEarnt);
	e.Set(gDlSpEventProgress.m_dltTotalFansBefore,		oldFanTotal);
	e.Set(gDlSpEventProgress.m_dltTotalFansAfter,		newFanTotal);
	e.Set(gDlSpEventProgress.m_dltTotalPlaytimeBefore,	oldPlaytime);
	e.Set(gDlSpEventProgress.m_dltTotalPlaytimeAfter,	newPlaytime);
	e.Set(gDlSpEventProgress.m_dltLightsGainedInEvent,	lightsGainedThisEvent);

	e.Send();

	
}

void LogTierUnlocked(int tierUnlocked)
{
	F32 totalPlaytime = 0.0f;

	if (const Game::C_GameProfile* profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile())
		totalPlaytime = profile->mSpTotalRaceTime;

	DataLog::DataLogEvent e(gDlSpTierProgress.m_dlt);

	e.Set(gDlSpTierProgress.m_dltTier,					tierUnlocked);
	e.Set(gDlSpTierProgress.m_dltTotalPlaytime,		totalPlaytime);

	e.Send();

	
}

void LogBossUnlocked(int bossTier)
{
	F32 totalPlaytime = 0.0f;

	if (const Game::C_GameProfile* profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile())
		totalPlaytime = profile->mSpTotalRaceTime;

	DataLog::DataLogEvent e(gDlSpBossUnlocked.m_dlt);

	e.Set(gDlSpBossUnlocked.m_dltBossTier,			bossTier);
	e.Set(gDlSpBossUnlocked.m_dltTotalPlaytime,		totalPlaytime);

	e.Send();

	
}

void LogDifficultySelected(const char* screenName, U8 oldDifficulty, U8 newDifficulty)
{
	DataLog::DataLogEvent e(gDlSpDifficulty.m_dlt);

	e.Set(gDlSpDifficulty.m_dltScreen,					screenName);
	e.Set(gDlSpDifficulty.m_dltOldDifficulty,			oldDifficulty);
	e.Set(gDlSpDifficulty.m_dltNewDifficulty,			newDifficulty);

	e.Send();

	
}

void LogBossEventFinished(const char* bossName, bool wreckedBoss)
{
	DataLog::DataLogEvent e(gDlSpBossCompletion.m_dlt);

	e.Set(gDlSpBossCompletion.m_dltBossName,			bossName);
	e.Set(gDlSpBossCompletion.m_dltWreckedBoss,		wreckedBoss);


	e.Send();
}

void LogControllerLayoutChanged(U8 oldLayout, U8 newLayout)
{
	DataLog::DataLogEvent e(gDlControllerLayoutChanged.m_dlt);

	e.Set(gDlControllerLayoutChanged.m_dltOldLayout,		oldLayout);
	e.Set(gDlControllerLayoutChanged.m_dltNewLayout,		newLayout);

	e.Send();
}

void LogBrightnessChanged(U8 oldBrightness, U8 newBrightness)
{
	DataLog::DataLogEvent e(gDlBrightnessChanged.m_dlt);

	e.Set(gDlBrightnessChanged.m_dltOldBrightness,		oldBrightness);
	e.Set(gDlBrightnessChanged.m_dltNewBrightness,		newBrightness);

	e.Send();
}

void LogRegionLanguageUsed(U8 region, U8 language)
{
	DataLog::DataLogEvent e(gDlRegionLanguageUsed.m_dlt);

	switch(region)
	{
	case UI::E_UIRegion_Europe:
		e.Set(gDlRegionLanguageUsed.m_dltRegion,		"Europe");
		break;
	case UI::E_UIRegion_NorthAmerica:
		e.Set(gDlRegionLanguageUsed.m_dltRegion,		"North America");
		break;
	case UI::E_UIRegion_Other:
		e.Set(gDlRegionLanguageUsed.m_dltRegion,		"Other");
		break;
	default:
		e.Set(gDlRegionLanguageUsed.m_dltRegion,		"Unknown");
	}

	switch(language)
	{
	case UI::E_UILanguage_English:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"English");
		break;
	case UI::E_UILanguage_French:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"French");
		break;
	case UI::E_UILanguage_German:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"German");
		break;
	case UI::E_UILanguage_Italian:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Italian");
		break;
	case UI::E_UILanguage_Spanish:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Spanish");
		break;
	case UI::E_UILanguage_Hungarian:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Hungarian");
		break;
	case UI::E_UILanguage_Russian:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Russian");
		break;
	case UI::E_UILanguage_Polish:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Polish");
		break;
	case UI::E_UILanguage_Czech:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Czech");
		break;
	case UI::E_UILanguage_SimplifiedChinese:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Simplified Chinese");
		break;
	case UI::E_UILanguage_TraditionalChinese:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Traditional Chinese");
		break;
	case UI::E_UILanguage_Japanese:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Japanese");
		break;
	case UI::E_UILanguage_Korean:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Korean");
		break;
	case UI::E_UILanguage_Other:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Other");
		break;
	default:
		e.Set(gDlRegionLanguageUsed.m_dltLanguage,		"Unknown");
	}

	e.Send();
}

void LogDisplaySettings(U32 width, U32 height)
{
	DataLog::DataLogEvent e(gDlDisplaySettings.m_dlt);

	e.Set(gDlDisplaySettings.m_dltWidth,			width);
	e.Set(gDlDisplaySettings.m_dltHeight,			height);

	e.Send();
}

}
