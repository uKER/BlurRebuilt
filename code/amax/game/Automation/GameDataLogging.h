
#ifndef GAME_DATA_LOGGING_H
#define GAME_DATA_LOGGING_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	class C_LevelState;
	struct MpRaceSummary;
}

namespace AmaxDataLogging
{
	struct PerkGainInfo
	{
		float		mTime;

		U8			mPlayer;
		const char*	mPerkType;
		float		mDist;
	};

	extern Btl::List<PerkGainInfo>		g_PerkUsedList;

	void Create();

	void LogRaceStart(const char* gameMode, const Game::C_LevelState *levelState, const char* gamerTag, const char* eventName, const char* raceMode);

	void LogRaceFinished(unsigned int health, unsigned int racePosition, float distanceToLeader, float distanceToNearest, bool passed, bool fatallyDamaged, bool quit, bool restart, bool quitAndWin, float time,
		U8 nitrosCollected, U8 nitrosUsed, U8 clocksCollected);

	void LogRaceEnd();

	void LogSinglePlayerRaceFinished(int fans, float cash, float progression, U8 totalFans);

	void LogRaceCancelled();

	void LogRaceLap(unsigned int health, float lapTime, unsigned int currentLap, unsigned int racePosition, float distanceToLeader, float distanceToNearest, float time);

	void LogPlayerInfo(Maths::HVec3 pos, float speed, int lap, int numA, int numB, int numX, int numY, int leftStick, int rightStick, int numRB,
					   int numLB, int numLT, int numRT, float time, bool wheelsOnGround, F32 frameRate);

	void LogAIInfo(int vehicleIndex, float distance, float time);

	void LogPickupUse(Maths::HVec3 pos, U32 carIndex, const char* pickupType, unsigned int level, int numFreeSlots, int numSlots, float time); // , unsigned int carsHit, bool playerHit);

	void LogPickupUsageGain(PerkGainInfo* gainInfo, float dist);

	void LogRaceBookMessageSent(const char* sender, const char* receiver, const char* messageName, const char* messageType, float priority, 
								float probability, const char* locStr);

	void LogFireDialogue(const char* speaker, const char* trigger, const char* locStr);

	void LogCollision(const char* objectA, const char* objectB, Maths::HVec3 collPoint, float severity, int damage, F32 raceTime);

	void LogReset(Maths::HVec3 originalPoint,Maths::HVec3 resetPoint, bool isHuman, bool isManual);

	void LogPerkBlockedByShield(int firingIdx, int blockingIdx, U32 uniqueId, float responseTime);

	void LogHitByPickup(int firingIdx, int hitIdx, U32 uniqueId, const char* pickupType);

	void LogPickedUpPickup(int vehIdx, const char* pickupType, Maths::HVec3 pos, S32 droppedIdx, S32 pickupId);

	void LogShieldUse(int vehicleIdx, bool start);

	void LogPlacesGainedFromPerk(int vehIdx, int oldPosition, int newPosition, const char* pickup );

	void LogPerkCausedCrash(int firingIdx, int hitIdx, const char* pickup);

	void LogPerkCausedFlip(int firingIdx, int hitIdx, const char* pickup);

	void LogHealthChanged(int vehIdx, int prevHealth, int newHealth, Maths::HVec3 position, float time);

	void LogBrake(int controllerIdx, float time, Maths::HVec3 position, bool handbrake);

	void LogCameraChanged(int controllerIdx, float time);
	
	void LogCameraDuration(const char* cameraName, float time);

	void LogShopItemAcquired(const char* locStr, bool purchased, float progress, const char* lastEvStr);

	void LogVehicleAcquired(const char* locStr, bool purchased, float progress, const char* lastEvStr);

	void LogEventUnlocked(const char* locStr, float progress, const char* lastEvStr);

	void LogCheckpointHit(Maths::HVec3 pos, float oldTime, float newTime, float difference, float timeFromLastPoint, int zoneId, U8 nitrosGained, U8 nitrosUsed, U8 clocksGained);

	void LogCheckpointMissed(Maths::HVec3 pos, int zonesCrossed, U8 nitrosGained, U8 nitrosUsed, U8 clocksGained);

	void LogScore(int vehIdx, Maths::HVec3 pos, char* score, S8 scoreType, char* skill1, char* skill2, char* skill3, char* bonus, int fans, 
		int chain_fans, int chain_amount, int current_total);

	void LogMultiPlayerRaceFinished(int position, bool wrecked, int fans_scored, int current_rank, int mod1, int mod2, int mod3, int mod4, int mod5, int bonus);

	void LogMultiPlayerChallengeComplete(int challenge, int part);
	
	void LogRetry(int number, const char* lastEvStr);

	void LogTimeOnScreen(float time, const char* screen);

	void LogEventSkipped(const char* cityName, const char* routeName, int numLaps);

	void LogPlayerCareerInformation(const char* gamertag, U8 rank, U32 driverScore, F32 powerupHitRatio, U64 playTime, U32 fans, F32 averagePos, U32 eventsPlayed, U32 eventsWon);

	void LogMpRaceInformation(U8 raceMode, U8 timeoutLength, F32 averageLapTime,  const Game::MpRaceSummary* summary, U8 timesWrecked, F32 distanceToFinish, bool timedOut);

	void LogMpRankUp(U8 rank, U64 totalPlayTime, U32 totalRaces);

	void LogMpModUsage(U8 slot1, U8 slot2, U8 slot3, U32 slotActivated1, U32 slotActivated2, U32 slotActivated3);

	void LogMpAwardReceived(U8 award, F32 score);

	void LogMpChallengeCompleted(U8 rank, U32 careerRaces, U64 playTime, U8 challenge, U8 stage, U32 vehicleId);

	void LogMpChallengePackCompleted(U8 rank, U32 careerRaces, U64 playTime, U8 challengePack, U32 vehicleId);

	void LogMpVoteResults(U32 route, U8 votes);

	void LogMpWinningVoteCast(U32 route);

	void LogWrecked(U8 vehicle, Maths::HVec3 position, F32 raceTime, S8 wrecker);

	void LogDestructionInfo(U32 normalShots, U32 longShots, U32 skillShots, U32 wrecks, U32 points, U32 greenWrecks, U32 yellowWrecks, U32 redWrecks, U32 mVHardCarsWrecked, F32 accuracy, U32 totalShots, U32 bonusPoints);

	void LogFanDemandOver(const char* type, U32 fans, bool passed);

	void LogEventStatus(const char* eventName, U8 lightsEarnt, S32 oldFanTotal, S32 newFanTotal, F32 oldPlaytime, F32 newPlaytime, U8 lightsGainedThisEvent);

	void LogTierUnlocked(int tierUnlocked);

	void LogBossUnlocked(int bossTier);

	void LogDifficultySelected(const char* screenName, U8 oldDifficulty, U8 newDifficulty);

	void LogBossEventFinished(const char* bossName, bool wreckedBoss);

	void LogControllerLayoutChanged(U8 oldLayout, U8 newLayout);

	void LogBrightnessChanged(U8 oldBrightness, U8 newBrightness);

	void LogRegionLanguageUsed(U8 region, U8 language);

	void LogDisplaySettings(U32 width, U32 height);
}

#endif
