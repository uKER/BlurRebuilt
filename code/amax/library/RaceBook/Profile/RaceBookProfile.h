#ifndef RACEBOOK_PROFILE_H
#define RACEBOOK_PROFILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>

#include <RaceBook\Data\RaceBookData.h>

#include <RaceBook/Profile/RaceBookProfileStatistics.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <GameLogic/Profile/ProfileBase.h>
#include <GameLogic\Structures\LevelStateVehicleUpgrades.h>

#include <ui/Graphics/UIColourClasses.h>

namespace RaceBook
{
	struct RaceBookAIPriorityTriggers;
	class C_RaceBookVehicle;

	const int MAX_RACEBOOK_PROFILE_PERK_SLOTS=10;

	struct GarageVehicle
	{
		GarageVehicle(XtId id)
		{
			mId				= id;
		}

		XtId				mId;
	};

	class C_RaceBookProfile : public Messaging::C_MessageHandler, public Game::C_ProfileBase
	{
	public:
		~C_RaceBookProfile();

		void Create(RaceBookCharacter *c);

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		XtLocId GetName() { return C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)mID]->loc_string; }

		void PostRaceUpdate();
		float CalculateProgress() { return (float)mLifetimeStatistics.mRacesPassed / (float)C_GameEventData::Instance().mEvents.Number(); };

		void ClearRaceAudioData();

		void SortDialogue();
		void BuildRaceDialogue( Btl::Set<U32>* charactersInRace, Btl::Set<U32>* wavebankList );
		void BuildRaceDialogueType( Btl::Set<U32>* charactersInRace, Game::CharacterConversationVec* dialogue, 
			Game::CharacterConversationTracker* dialogueRace, Btl::Set<U32>* wavebankList );

		void SendToLua( lua_State* pState );
		void AddCarToGarage(XtId car, bool purchased);

		void BuildLevelStateVehicleUpgrades( Game::LevelStateVehicleUpgrades& upgrades );

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void SerialiseDialogue( Sys::BinarySerialiserAutoEndian& stream, Game::CharacterConversationVec* vec );
		void DeserialiseDialogue( Sys::BinaryDeserialiserAutoEndian& stream, Game::CharacterConversationVec* vec );

		U32 GetID() { return mID; };

		// ---------------------------------------- Overridden from profile base ----------------------------------------

		const S32 NumFans() const												{ return mNumFans; }
		const S32 NumPreviousFans() const										{ return mNumPreviousFans; }

		// --------------------------------------------------------------------------------------------------------------

		Btl::Vector<C_RaceBookProfile*> mFriendRequests, mFriends, mRivals, mBosses;
		XtLocId mStatus;
		
		int mNumFans, mNumPreviousFans, mCash, mLevel, mPerkLevelPoints, mRacesWon, mRacesLost, mCurrentCarIdx; // currentCarIdx is -1 if not participating
		
		float mBankBalance, mPreviousBankBalance, mTotalCareerEarnings;

		float mUiCashEarned;
		int mUiFansEarned;

		C_RaceBookProfileStatistics mLifetimeStatistics;
		
		XtU16 mAvatarID;
		bool mHasLeveledUp;
				
		Btl::Vector<GarageVehicle> mGarage;

		U32 mLookingAtTimeAttackRoute;

		XtId mCurrentCarID;
		RaceBookCharacterType mType;

		U32 repIdx;

		// ---------------------- In Game Dialogue ----------------------
		Game::CharacterConversationVec	mPlayerCollidesWithDriverItems;
		Game::CharacterConversationVec	mDriverCollidesWithPlayerItems;
		Game::CharacterConversationVec	mDriverIsFatallyDamagedItems;
		Game::CharacterConversationVec	mDriverIsLowHealthItems;
		Game::CharacterConversationVec	mPlayerOvertakesDriverItems;
		Game::CharacterConversationVec	mDriverOvertakesPlayerItems;
		Game::CharacterConversationVec	mPlayerHitByPerkItems;
		Game::CharacterConversationVec	mDriverHitByPerkItems;
		Game::CharacterConversationVec	mPlayerShieldedPerkItems;
		Game::CharacterConversationVec	mDriverShieldedPerkItems;
		Game::CharacterConversationVec	mPlayerMissedPerkItems;
		Game::CharacterConversationVec	mDriverMissedPerkItems;

		Game::CharacterConversationTracker	mPlayerCollidesWithDriverRaceItems;
		Game::CharacterConversationTracker	mDriverCollidesWithPlayerRaceItems;
		Game::CharacterConversationTracker	mDriverIsFatallyDamagedRaceItems;
		Game::CharacterConversationTracker	mDriverIsLowHealthRaceItems;
		Game::CharacterConversationTracker	mPlayerOvertakesDriverRaceItems;
		Game::CharacterConversationTracker	mDriverOvertakesPlayerRaceItems;
		Game::CharacterConversationTracker	mPlayerHitByPerkRaceItems;
		Game::CharacterConversationTracker	mDriverHitByPerkRaceItems;
		Game::CharacterConversationTracker	mPlayerShieldedPerkRaceItems;
		Game::CharacterConversationTracker	mDriverShieldedPerkRaceItems;
		Game::CharacterConversationTracker	mPlayerMissedPerkRaceItems;
		Game::CharacterConversationTracker	mDriverMissedPerkRaceItems;

		Btl::Vector<Game::CharacterConversationVec*>		mDialogueItems;
		Btl::Vector<Game::CharacterConversationTracker*>	mDialogueRaceItems;

		Game::C_InGameDialogueGroup*						mDialogueGroup;
		// --------------------------------------------------------------

	private:
		U32 mID;

	friend class C_RaceBookManager;
	};

} // namespace RaceBook

#endif // RACEBOOK_PROFILE_H
