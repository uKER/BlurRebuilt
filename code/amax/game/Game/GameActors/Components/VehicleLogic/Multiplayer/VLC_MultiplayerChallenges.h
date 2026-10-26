#ifndef VLC_MULTIPLAYER_CHALLENGES_H
#define VLC_MULTIPLAYER_CHALLENGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "gamelogic/gamedata/multiplayerdata.h"
#include <game\GameActors\Components\GameComponents.h>

namespace Game
{
	class C_RaceLogicLayer;
	class C_RaceBookLayer;
	class C_LevelState; 
	class C_LevelResult; 
	class C_MultiplayerProfile; 

	namespace Actors
	{
		class RaceLogicCore;
		class VehicleLogicActor;
	};
}

class C_VehicleLogicMsg_AggressiveScore; 
class C_VehicleLogicMsg_DestroyedPickupWithPickup;
class C_VehicleLogicMsg_FiredPerkMsg;

namespace Components
{
	class BaseMessageHandler;
	class C_VehicleComponent_NewScoring;

	class I_VehicleComponent_MP_Challenges_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_MP_Challenges_CreateInfo() {}

		static Component::C_ComponentTypeId			ClassType() {return Components::E_VehicleComponent_MP_Challenges;}
		virtual Component::C_ComponentTypeId		GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Challenges;}

		Actor::C_ActorHandle						mParentActorHandle;
		Game::C_RaceLogicLayer						*mRaceLogicLayerPtr;
		Game::C_RaceBookLayer						*mRaceBookLayerPtr;
		Game::Actors::RaceLogicCore					*m_pRLC;
		Game::Actors::C_VehicleLogicActor			*m_pLogicActor;
		
		int											mVehicleIndex;

	};

	class C_VehicleComponent_MP_Challenges : public I_GameActorComponent
	{
	public:
		C_VehicleComponent_MP_Challenges();

		static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Challenges;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Challenges;} // from Component::I_GameComponent

		static Component::C_ComponentInterfaceId	ClassType() {return Components::E_VehicleComponent_MP_Challenges;}
		virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return Components::E_VehicleComponent_MP_Challenges;} // from Component::I_GameComponent

		virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
				void								CheckPostRaceChallenges();

		void										CheckRivalry( const U8 wreckedVehicleIndex );

	protected:
		virtual bool								CreateImpl();

		void										UpdateRewards(MultiplayerChallengeTier* pChallenge);
		

	private:
		enum E_PICKUP_USED
		{
			E_PICKUP_USED_NITRO,
			E_PICKUP_USED_BLINDFIRE,
			E_PICKUP_USED_BARGE,
			E_PICKUP_USED_SHUNT,
			E_PICKUP_USED_SHOCK,
			E_PICKUP_USED_BOMB,
			E_PICKUP_USED_REPAIR,
			E_PICKUP_USED_SHIELD,
			//This is currently being used for a 8 bit bitset on the blackboard. Need to change it if you add more.
		};

		void										UpdateDebugValues();

		//Check to see if the challenge is already complete, we might need to add versions for carID/challenge type and pickupType/challengeType, we don't need them yet though
		const bool									ChallengeCurrentlyCompletable(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType) const;	
		const bool									AlreadyComplete(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType, const bool overallProgress = true) const;	
		const bool									CurrentlyUnlocked(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType) const;	
		
		void										IncrementCarChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType);
		void										IncreaseChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType, const S32 sAmount = 1 );
		void										ResetChallengeProgress(const enum UI::E_MULTIPLAYER_CHALLENGE_TYPE eChallengeType);

		bool										SkillContainerContainsSkill(const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills, const E_BLUR_SCORE_MOVE_SKILL desiredSkill) const;

		const bool									CheckChallenges(const Messaging::C_MessageBase *pMessage );

		void										CheckCarCrazy(); // Use X different cars in a day
		void										CheckPiedPiper(); //Earn the most fans in a 20 player race

		void										CheckBaseMoves (E_BLUR_SCORE_MOVE_BASE base, const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills);
		void										CheckSkillMoves(E_BLUR_SCORE_MOVE_BASE base, const Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>& skills);
		void										CheckBonusMoves(const Btl::Vector<E_BLUR_SCORE_MOVE_BONUS>& bonus);

		void										CheckTearaway();					//Win when at least 1 lap ahead of everyone else
		void										CheckUntouchable();					//Finish without taking any damage
		void										CheckCausalNexus(int chainLength);//Do a X combo chain of scoring moves
		void										CheckHighSlider(float driftDistance); // Pull a drift of more than X metres
		void										CheckBottomsUp(); //Shunt X opponents into the air and drive under them
		void										CheckMasterExploder(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg); // Destroy X mines with mines

		void										CheckPunisher();					//Wreck X Cars In race
		void										CheckBruteForce(const int wreckeeIndex);	//Wreck X cars by ramming them
		void										CheckCarnage();						//Do most damage in a race
		void										CheckRetribution();					//Perform 5 revenge attacks in one race
		void										CheckPileup();						//Crash 5 cars in a single chain

		void										CheckSkeetShooter(const int victimVehicleIndex);	//Wreck an Airborne vehicle
		void										CheckPacifist();					//Win the race without using a pickup
		
		void										CheckVendetta(U16 wreckeeIdx);		//Wreck same vehicle twice in one race
		void										CheckPickupUseChallenges(const enum E_PICKUP_TYPE ePickupType, const E_PICKUP_SUB_TYPE eSubType);	//Counts use of a single pickup type
		void										CheckWreckingBall();				// Wreck X cars (lifetime)
		void										CheckShowOff();						// Place 3rd or better in a race

		void										CheckMineSpecialist(); // only use mines
		void										CheckBargeSpecialist(); // only use barges
		void										CheckAfterburner();		// only use nitros
		void										CheckBlindfireSpecialist(); //only use blindfires

		void										CheckCauseAndEffect(); // Hit X cars with one double barge
		void										CheckFTL(); // Drive at X MPH for 5 Seconds 

		void										CheckMasochist();	 // Dont use shields for X races. 
		void										CheckThickSkin();	 // Dont use repairs for X races.
		void										CheckRampage();		 // Wreck X cars in a race without being wrecked yourself.

		void										CheckExtremelyExtreme(); // Jump X Meters in a single Jump
		void										CheckJumper();			// Perform X Nitro Jumps
		void										CheckQuickDraw();		// Wreck x cars in x seconds
		void										CheckChainReaction(const S16 chainLength); //Get a chain score of x or above
		void										CheckHeadInTheClouds();	//Spend x seconds in the air

		void										CheckUltimateComeback(); // From first place, fall to last place, then win in first place. 
		void										CheckSidewinder();		 // Collect pickups whilst drifting
		void										CheckDistance();		//Drive x miles a day
		void										CheckBackwardsThinking(); // Only use backwards pickups in a race

		void										CheckChosenOne(); //Hit with X successive reverse blindfires

		//Helpers
		bool										GetFinishedLast(const int vehicleIndex); // Did this vehicleindex come last within those cars in the Game::E_VEHICLE_STATE_FINISHED state

		void										UpdatePerkFiredData(const C_VehicleLogicMsg_FiredPerkMsg* pMsg);
		void										UpdateRecentlyFiredMineIDs(float timeStep);
		void										UpdateUltimateComebackStatus();

		void										UpdateDominator();
		void										UpdateCasualNexus();
		void										CheckChainReaction(const int comboLength);
		void										UpdateDominatorRunningTotal();
		void										UpdateWellTraveled();
		void										HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg);
		void										HandleDestroyedPickupWithPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg);
		void										CheckReverseFireChallenges(E_PICKUP_TYPE type);

		void										CheckForValidMinesweeper(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg);

		void										CheckForValidRearGuard(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg);
		bool										CheckRearGuardMineBlocksProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const;
		bool									    CheckRearGuardReverseProjectileBlocksProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const;
		bool									    CheckRearGuardBargeNeutralisesProjectile(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg) const;

		void										CheckModActivatedChallenges(const UI::E_MultiplayerVehicleAbilities modActivated);
		void										UpdateChosenOneBlindfires();
		bool										CheckFansInTimePeriod( S32 fansRequired, F32 timePeriod );

		void										IncrementLastGaspChallenge(const int numVehiclesHitWithLastGaspThisLife);

	private:
		Game::C_RaceLogicLayer*						m_pRaceLogicLayerPtr;
		Game::C_RaceBookLayer*						m_pRaceBookLayerPtr;
		const C_VehicleComponent_NewScoring*		m_pScoring;
		Game::Actors::RaceLogicCore*				m_pRLC;
		Game::Actors::C_VehicleLogicActor*			m_pLogicActor;

		int											mVehicleIndex;
		bool										mEnabled;
		Game::C_LevelState*							m_pLevelState; 
		Game::C_LevelResult*						m_pLevelResult; 
		Game::C_MultiplayerProfile* 				m_pMultiplayerProfile;

		U32											mCarID;

		S32											mStartingLevel;
		F32											mLastGaspChallengeDelayTimer;
		void										CheckForDelayedLastGaspChallengeProgress(); // jbunner (22/03/2010): BUG: 36866 - delay incrementing last gasp challenge progress until after respawn. Challenge complete Stomps all over wrecked hud. 

		struct BlindfireInformation
		{
			U32		id;
			float	timeDestroyed;
			BlindfireInformation()
			{
				timeDestroyed = 0.0f;
			}
		};

		Btl::List<BlindfireInformation>				m_RecentBlindfiresFired;

		static const int							s_mTerminator_CarsToWreck = 5;
		static const int							s_mTimeToAchieveFTL = 5;

		static const int							s_mQuickDrawCarsToWreck;
		static const float							s_mMetresInAMile;
	};

}

#endif // VLC_MULTIPLAYER_CHALLENGES_H
