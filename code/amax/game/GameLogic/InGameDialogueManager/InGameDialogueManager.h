#ifndef INGAME_DIALOGUE_MANAGER_H
#define INGAME_DIALOGUE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <core/threads/Mutex.h>
#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <GameLogic\GameData\GameData.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <GameLogic\InGameDialogueManager\InGameDialogueStructures.h>
#include <GameLogic\InGameDialogueManager\InGameDialogueGroup.h>

#include <system/misc/Random.h>
#include <shared_actors\Components\Messages\MovementMessages.h>

namespace RaceBook { class C_RaceBookProfile; };

namespace Game
{
	typedef Btl::Map<U32, U32>	RBCharacterHashMap;

	class InGameDialogueWavebank;

	// ---------------------------------------------------------------------

	class C_InGameDialogueThread : public MT::Thread
	{
	public:
		void ThreadMain();
	};

	// ---------------------------------------------------------------------

	class C_InGameDialogueManager : public Messaging::C_MessageMultiHandler
	{
	public:

		C_InGameDialogueManager() {};
		static C_InGameDialogueManager& Instance(){return s_instance;};

		void Initialise();
		void JoinThread();

		void InitProfiles();
		void InitRace();
		void InitRaceInThread();

		void ClearAllData();
		void ClearRaceData();
		void ClearDataItems(CharacterConversationVec* vec);

		void PlayAllDialogue();

		bool MessageHandler(const Messaging::C_MessageBase* pMessage);
		bool IsEnabled(){return mEnabled;};

		void Serialise( Sys::BinarySerialiserAutoEndian& stream ) ;
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void SerialiseItems( Sys::BinarySerialiserAutoEndian& stream, Game::CharacterConversationVec* item );
		void DeserialiseItems( Sys::BinaryDeserialiserAutoEndian& stream, Game::CharacterConversationVec* item );

		void AllowAudioLoadingNow();

	private:
		static C_InGameDialogueManager s_instance;

		bool mEnabled;
		bool mForcePlayAllDialogue;

		int mDialogueIdx;
		int mProfIdx;
		int mProfDialogueIdx;
		int mGroupIdx;

		// -------------- HASHES ----------------
		RBCharacterHashMap	mCharacterHashMap;
		RBCharacterHashMap	mHashCharacterMap;
		RBCharacterHashMap	mCharacterGroupMap;

		U32 mHash_DriverCollidesWithPlayer;
		U32 mHash_PlayerCollidesWithDriver;
		U32 mHash_PlayerIsFatallyDamaged;
		U32 mHash_DriverIsFatallyDamaged;
		U32 mHash_PlayerIsLowHealth;
		U32 mHash_DriverIsLowHealth;
		U32 mHash_PlayerOvertakesDriver;
		U32 mHash_DriverOvertakesPlayer;
		U32 mHash_PlayerHitByPerk;
		U32 mHash_DriverHitByPerk;
		U32 mHash_PlayerShieldedPerk;
		U32 mHash_DriverShieldedPerk;
		U32 mHash_PlayerMissedPerk;
		U32 mHash_DriverMissedPerk;
		U32 mHash_PlayerCollidesWithEnvironment;
		U32 mHash_PlayerObjectiveSecondLapPassing;
		U32 mHash_PlayerObjectiveLastLapPassing;
		U32 mHash_PlayerObjectiveLastLapLastSplitPassing;
		U32 mHash_PlayerObjectiveSecondLapFailing;
		U32 mHash_PlayerObjectiveLastLapFailing;
		U32 mHash_PlayerObjectiveLastLapLastSplitFailing;
		U32 mHash_PlayerObjectiveIntro;
		U32 mHash_PlayerObjectiveRetry;	
		U32 mHash_PlayerObjectivePassed;	
		U32 mHash_PlayerObjectiveFailed;

		U32 mHash_AIUnhappy;
		U32 mHash_AIHappy;
		U32 mHash_AIMissesPlayer;
		U32 mHash_AIDefendsPlayer;
		// --------------------------------------

		Btl::Set<U32>							mCharactersInvolvedInRace;
		Btl::Set<U32>							mMinorCharIdsInvolvedInRace;
		Btl::Map<U32, C_InGameDialogueGroup*>	mDialogueHashGroupMap;

		// ------------ DIALOGUE ----------------
		CharacterConversationVec		mPlayerCollidesWithEnvironmentItems;
		CharacterConversationVec		mPlayerIsFatallyDamagedItems;
		CharacterConversationVec		mPlayerIsLowHealthItems;
		CharacterConversationVec		mPlayerObjectivePassingSecondLapItems;
		CharacterConversationVec		mPlayerObjectivePassingLastLapItems;
		CharacterConversationVec		mPlayerObjectivePassingLastLapLastSplitItems;
		CharacterConversationVec		mPlayerObjectiveFailingSecondLapItems;
		CharacterConversationVec		mPlayerObjectiveFailingLastLapItems;
		CharacterConversationVec		mPlayerObjectiveFailingLastLapLastSplitItems;
		CharacterConversationVec		mPlayerObjectivePassedItems;
		CharacterConversationVec		mPlayerObjectiveFailedItems;
		CharacterConversationVec		mPlayerObjectiveIntroItems;
		CharacterConversationVec		mPlayerObjectiveRetryItems;

		CharacterConversationTracker	mPlayerCollidesWithEnvironmentRaceItems;
		CharacterConversationTracker	mPlayerIsFatallyDamagedRaceItems;
		CharacterConversationTracker	mPlayerIsLowHealthRaceItems;
		CharacterConversationTracker	mPlayerObjectivePassingSecondLapRaceItems;
		CharacterConversationTracker	mPlayerObjectivePassingLastLapRaceItems;
		CharacterConversationTracker	mPlayerObjectivePassingLastLapLastSplitRaceItems;
		CharacterConversationTracker	mPlayerObjectiveFailingSecondLapRaceItems;
		CharacterConversationTracker	mPlayerObjectiveFailingLastLapRaceItems;
		CharacterConversationTracker	mPlayerObjectiveFailingLastLapLastSplitRaceItems;
		CharacterConversationTracker	mPlayerObjectivePassedRaceItems;
		CharacterConversationTracker	mPlayerObjectiveFailedRaceItems;
		CharacterConversationTracker	mPlayerObjectiveIntroRaceItems;
		CharacterConversationTracker	mPlayerObjectiveRetryRaceItems;
		// --------------------------------------

		Btl::Vector<Game::CharacterConversationVec*>		mDialogueItems;
		Btl::Vector<Game::CharacterConversationTracker*>	mDialogueRaceItems;

		C_InGameDialogueConversation*	mCurrentConversation;
		int								mCurrentDialogueIdx;
		bool							mCurrentlyPlayingAudio;
		float							mTimeout;

		Util::C_Random					mRandGenerator;
		float							mCurrentEventProbability;
		bool							mRetry;

		C_InGameDialogueThread*			mInGameDialogueThread;

		void CheckObjectiveDialogue(bool lastSplit, int vehIdx, int lapNum, int nextSplitNum);
		void CheckImpactDialogue(const Game::C_GameplayImpactMsg* pMsg);
		void CheckPerkDialogue(U8 firingVehIdx, U8 hitVehIdx, bool special = false);
		void CheckMissedPerkDialogue(U8 firingVehIdx, U8 hitVehIdx);
		void CheckOvertakeDialogue(U8 overtakerIdx, U8 overtakeeIndex, bool withNitro);
		void CheckFatallyDamagedDialogue(U8 vehIdx);
		void CheckShieldedPerkDialogue(U8 firingVehIdx, U8 shieldedVehIdx);
		void CheckLowHealthDialogue(U8 vehIdx, E_HEALTH_LEVEL healthLevel);
		void CheckSideSwipeDialogue(U8 doingIdx, U8 againstIdx);

		void PlayNextDialogue();

		CharacterConversationVec*	GetCharacterConversationMapFromHash(AmaxInGameDialogueItem* basicDialogueItem);
		void						GetTriggerNameFromHash(Util::C_String32& triggerName, U32 hash);

		void BuildAndSortRaceDialogue( Game::CharacterConversationVec* dialogue, Game::CharacterConversationTracker* raceDialogue, Btl::Set<U32>* wavebankList );

		friend class  C_InGameDialogueConversation;
		friend struct CharacterConversationTracker;

		// Emergency catch to stop repeated lines.
		U32 mLastLocId;
	};

} // namespace Game

#endif // INGAME_DIALOGUE_MANAGER_H