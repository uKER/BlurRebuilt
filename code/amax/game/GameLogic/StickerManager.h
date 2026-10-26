#ifndef STICKER_MANAGER_H
#define STICKER_MANAGER_H

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>
#include <GameLogic/Structures/Sticker.h>
#include <GameLogic/GameData/AchievementData.h>
#include <GameLogic/Structures/StickerBlackboardData.h>

#ifdef _MSC_VER
#pragma once
#endif

// Forward decls
namespace UI
{
	enum E_MULTIPLAYER_CHALLENGE_TYPE;
}


namespace Game
{
	class C_MultiplayerProfile;
	class C_LevelState;
	class C_RaceLogicLayer;

	
	namespace Actors
	{
		class C_VehicleLogicActor;
	}
}

class C_VehicleLogicMsg_HitByPerkMsg;
class C_VehicleLogicMsg_FiredPerkMsg;
class C_VehicleLogicMsg_PowerUpDestroyedByPowerUp;
class C_VehicleLogicMsg_EvadedPowerUp;
class C_VehicleLogicMsg_AggressiveScore;
class C_VehicleLogicMsg_DestroyedPickupWithPickup;
class C_VehicleLogicMsg_ShieldedPerkMsg;
class C_VehicleLogicRepairMsg;

namespace Game
{
	class C_StickerManager : public Messaging::C_MessageMultiHandler
	{
	public:

		C_StickerManager()
		{
		};

		void 							Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void 							Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void							Init(int numStickers);
		void							InitForNewRace();
		void							Update(float frameTime);
		void							Shutdown();
		void							ClearStickerProgress();
		Btl::Vector<C_Sticker*>			AddStickersFromSorcery(Btl::Vector<C_Sticker> stickers);

		int								GetControllerIndex(int idx);
		U8								GetPrimaryProfileVehicleIndex();
		C_Sticker*						GetSticker(BLUR_STICKER sticker);
		bool							IsStickerComplete(BLUR_STICKER sticker);

		bool							HasNewStickers() { return s_blackboard.mHasUnlockedNewStickers; }

		const bool						CheckStickers(const Messaging::C_MessageBase *pMessage );

		static C_StickerManager&		Instance(){return s_instance;}
		static C_StickerBlackboardData&	GetStickerBlackboard(){return s_blackboard;}

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		void							DebugDumpSticker(BLUR_STICKER s);
		void							DebugDumpAllStickers();

		void							HandleDamageByPickup( U8 victimIndex, U8 firingIndex, int damage );

		//Public Checks
		void							CheckCustomizeLoadout();
		void							CheckBlurParty();
		void							CheckMpRaceFinishedStickers();
		void							CheckMpChallenges(UI::E_MULTIPLAYER_CHALLENGE_TYPE type, S8 tier);
		void							CheckDominator(const U8 numAwardsLocalPlayerWon); // Win X awards in a row
		void							CheckRankFifty();	// Reach rank 50 + unlocked all 'playlists/mods/challenges'
		void							WreckedRedCarInDestruction(); // Pure addition
		void							HitFanGate();
		void							CompletedFanDemand();
		void							CompletedFanGate();
		void							CheckTeamWinner(S8 resultsIndex);
		void							CheckAmboyOilfieldsTriggerRegion(U8 triggeringVehicleIndex);
		void							CheckGasStationTriggerRegion(U8 triggeringVehicleIndex);
		void							CheckBrightonFireworks();
		void							CheckBossBeaten(int tier);
		void							CheckDestructionLongshot();

		// Motorman Achievement checks ( 3 stickers )
		void							CheckWreckYourRival(const U8 wreckerID); 
		void							CheckMotorman(const bool won, const S32 points);

	public:
		void							CheckSinglePlayerStickers();

	private:
		void							CheckWreckedBizarre(const U8 wreckerId, const U8 wreckedId);
		void							CheckBeatRank50(S8 resultsIndex);
		void							CheckVisitedCities();
		void							CheckPhotoFinish(U8 resultsIndex);
		void							CheckPickupStopwatch();
		void							CheckWinWithCustomLoadout();
		void							CheckWreckYourFirst(U8 wreckerIndex);
		void							CheckWreckFirstBoss(U8 wreckerIndex);
		void							CheckWreckCarsInDestruction(U8 wreckerIdx);
		void							CheckWreckByThreeBolts(U8 wreckerIdx);

		void							CheckCheckpointNitrous();
		void							CheckPickupHitVehicles(const C_VehicleLogicMsg_HitByPerkMsg* pMsg);
		void							CheckShieldMultipleLap();

		void							CheckEvadedOrDestroyedFirstPowerup(U8 vehicleIndex);
		void							CheckAvoidShuntsByDriving(const C_VehicleLogicMsg_EvadedPowerUp* pMsg);
		void							CheckDestroyMines(const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg);
		void							CheckDestroyShunts(const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg);
		void							CheckShieldedAttack(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg);
		void							CheckRepairHealth(const C_VehicleLogicRepairMsg* pMsg);
		void							CheckPickupAggressiveHits(const C_VehicleLogicMsg_HitByPerkMsg* pMsg);
		void							CheckPickupVictimHits(const C_VehicleLogicMsg_HitByPerkMsg* pMsg);
		void							CheckKhansRide();
		void							CheckAvoidNatalyasPowerups();
		void							CheckDoMoreDamageThanCarver();
		void							CheckCloseRepair();
		void							CheckPickupFired(const C_VehicleLogicMsg_FiredPerkMsg* pMsg);
		void							CheckDestroyedPickupWithPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg);
		void							CheckHadPickupDestroyedByPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg);

		void							CheckBaseMoves (E_BLUR_SCORE_MOVE_BASE base, Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skills);
		void							CheckSkillMoves(E_BLUR_SCORE_MOVE_BASE base, Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skills);
		void							CheckBonusMoves(Btl::Vector<E_BLUR_SCORE_MOVE_BONUS> bonus);
		void							CheckAggressiveScoringMoves(const C_VehicleLogicMsg_AggressiveScore* pMsg);

		void							CheckAirbrakeInFirstThenWin();
		void							CheckWinWithRepair();
		void							CheckWinWithoutWrecked();
		void							CheckTopThreeInPinkCar();
		void							CheckWinInPlaylist();

		void							CheckResults();
		void							CheckGoldMedalStickers();

		void							CheckCleanLap();
		void							CheckHighSpeed();

		// ReachRankFifty helper func
		void							CalculateIfFinalUnlocksAchieved(bool& unlockedFinalRankedPlaylist,bool& unlockedAllChallengePacks,bool& unlockedAllVehicleMods, Game::C_MultiplayerProfile* pProfile);
		void							DealOutRankFiftyStickers(bool unlockedFinalRankedPlaylist, bool unlockedAllChallengePacks,bool unlockedAllVehicleMods, bool reachedRankFifty);

		//------------------------------------------------------------------------------------------

		// Timer functions to help us with all the timers we deal with. 
		// Note, these timers are incremented on GAME TIME (i.e. they are paused if you pause the game)
		Btl::Vector<float*>				mTimerVariables;
		void							StartTimer(float* pTimer, float startTime = 0.0f); // Starts a timer counting up from startTime
		void							StopTimer(float* pTimer); // Disposes of the timer from our internal storage

		void							ClearCachedPtrs();

		static C_StickerManager			s_instance;
		static C_StickerBlackboardData	s_blackboard;

		Btl::Vector<C_Sticker, Btl::VectorParameters_Array<BLUR_STICKER_MAX> > mStickers;
		Game::C_LevelState*             mpLevelState;
		Game::Actors::C_VehicleLogicActor* mpLogicActor;
		Game::C_RaceLogicLayer*			mpRaceLogicLayer;

		int								mCurrentEventIdx;
		int								mCurrentTierIdx;
	};

} // namespace Game

#endif // STICKER_MANAGER_H
