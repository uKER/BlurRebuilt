#ifndef GAME_MULTIPLAYER_PROFILE_H
#define GAME_MULTIPLAYER_PROFILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Structures/MiscellaneousChallengeBlackboardData.h>
#include <GameLogic/Structures/FavouriteGroups.h>
#include <GameLogic/Structures/RecentEventsStore.h>
#include "MultiplayerLoadouts.h"
#include "MultiplayerChallengeTracker.h"
#include "GameLogic/Profile/MultiplayerUnlockHistory.h"
#include "GameLogic/MultiplayerChallengeSystem.h"
#include <GameLogic/Profile/MultiplayerStatistics.h>
#include <GameLogic\Profile\MultiplayerPreviousSessionStats.h>

#define MAX_MULTIPLAYER_VEHICLES 50

namespace RacerNet { struct NetPlayerSharedProfile; }

namespace Game
{	
class C_MpResults;

typedef Btl::Pair< XtId, S32 >			ResprayData;
typedef Btl::Map< XtId, ResprayData >	ResprayMap;
	
	struct MpRaceSummary
	{
		U32		m_OnTrackFans;
		U32		m_TotalChallengeFans;
		U32		m_FinishBonus;
		U32		m_TeamBonus;
		U32		m_TotalNewFans;
		U32		m_ShowyFlourishContribution;
		U32		m_FanDemandContribution;
		S8		m_Position;
		U32		m_CarRewardFanBonus;
		bool	m_RankUp;
		U32		m_Rank;
	};

	class C_MultiplayerChallengeSystem;

	class C_MultiplayerProfile : public C_ProfileBase
	{

	public:
		C_MultiplayerProfile() {};
		~C_MultiplayerProfile() {};
		void									Init();

		const S32								NumPickupSlots() const;
		const bool								AllowAnyPerkType() const;
		const bool								AllowManualPerkChange() const;
		const bool								AllowShield() const;
		const S32								NumShieldSlots() const;
		const S32								NumAdditionalHealthSlots() const;

		const S32								NumFans() const				{ return mNumFans; }
		const S32								NumPreviousFans() const		{ return mNumPreviousFans; }

		void									PreRaceUpdate();
		void									PostRaceUpdate();
		const S32								GetNumRaceFans() const { return mOnTrackFans; }
		const S32								GetRaceFinishBonus() const { return mFinishBonus; }
		const S32								GetTeamBonus() const;
		void									CalculateCarRewardFanBonus( const S32 mOnTrackFans );
		const int								GetCurrentLevel() const;
		const int								GetPreviousLevel() const;
		int										CalcCurrentLevel();
		
		const MpRaceSummary&					GetRaceSummary() const;

		void									BuildNetworkSharedProfileData(RacerNet::NetPlayerSharedProfile& out, bool joinedPartyOrSession = false);
		void									BuildNetworkVehicleUpgradesData(RacerNet::NetPlayerVehicle& out, UI::E_CarUpgrades upgrades);

		void									ResetSessionStats();
	
		void									Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void									Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		// Legend stuff
		const bool								CanUnlockLegend() const;
		const S32								GetLegendLevel() const;
		void									UnlockLegendMode();

		// debug
		static void								DebugOverrideFans(void* par);
		void									DebugOverrideFansNonStatic(int fans);

		static void								DebugAddToChallenge(void* par);
		static void								DebugAddToCarChallenge(C_MultiplayerChallengeSystem& challengeSystem);

		MiscellaneousChallengeBlackboardData&	GetChallengeBlackboard();
		FavouriteGroups&						GetFavouriteEvents();
		RecentEventsStore&						GetRecentEvents();
		C_MultiplayerUnlockHistory&				GetRecentUnlockHistory() { return mRecentUnlockHistory; }
		C_MultiplayerChallengeSystem&			GetChallengeSystem() { return mChallengeSystem; }
		C_MultiplayerLoadouts&					GetLoadouts(bool ranked);
		C_MultiplayerPreviousSessionStats&		GetPreviouslyOnBlurStats() { return mPreviouslyOnBlurStats; }

		bool									GetFeatureUnlocked(const UI::E_MultiplayerFeature feature) const;

		MultiplayerStatistics&					GetMultiplayerStatistics();
		void SetLastCarUsed(U32 val);
		U32 GetLastCarUsed() const;
		U32 GetLastCarUsed( const AmaxCarClass carClass ) const;

		bool									IsBlurDeveloper();
		bool									HasBeatenBizarre();

		ResprayMap& GetRespraysMP();
		ResprayData& GetRespraysLastUsed();

		static S32								GetMaxLevellingRank();
		static S32								GetMaxLevellingFans();

	private:
		// garage resprays
		ResprayMap								mResprays_MP;
		ResprayData								mRespraysLastUsed;

		void									CalculateTeamBonus(const Game::C_LevelState& levelState, const Game::C_MpResults& result, const MultiplayerConfig* config);
		void									CalculateEventFinishedBonus(const S32 currentLevel);

		void									Reset();
		void									InitFavouriteGroups();
		void									CheckLegendMode();
		void									UnlockLegendCarChallengeUpgrades();
		void									ClearTempRaceFans();

		MultiplayerStatistics					mMultiplayerStatistics;

		int										mCurrentLevel;
		S32										mLegendLevel;
		int										mNumFans;

		U32										mLastCarUsed;
		Btl::Vector< U32, Btl::VectorParameters_Array< AmaxCarClass_Count > >	mLastCarUsedClass;

		bool									mCanUnlockLegend;

		MiscellaneousChallengeBlackboardData	mMiscChallengeDataBlackboard;
		C_MultiplayerUnlockHistory				mRecentUnlockHistory;		// Allow recent unlocks to be flagged for some nice UI action
		FavouriteGroups							mFavouriteGroups;
		RecentEventsStore						mRecentEvents;
		C_MultiplayerChallengeSystem			mChallengeSystem;
		C_MultiplayerLoadouts					mRankedLoadouts;
		C_MultiplayerLoadouts					mUnrankedLoadouts;
		C_MultiplayerChallengeTracker			mChallengeTracker;
		C_MultiplayerPreviousSessionStats		mPreviouslyOnBlurStats;

		//-----------------------------	Temp members that we do NOT save/load -------------------------//

		//	These help us keep track of how many ranks you've gone
		int										mPreviousLevel;
		int										mNumPreviousFans;

		//	Stuff we calculate at the end of a race/when the player quits and then use in the UI etc (ranked events only)
		int										mRaceFinishBonus;
		int										mOnTrackFans;
		int										mTotalChallengeFans;
		int										mFinishBonus;
		int										mTeamBonus;
		int										mFanFavouriteBonus;
		int										mTotalNewFans;
		bool									mCarRewardFanBonusEquipped;
		F32										mCarRewardFanBonusPercentage;
		int										mCarRewardFanBonus;
		MpRaceSummary							mRaceSummary;
		//---------------------------------------------------------------------------------------------//


#if !defined ( CFG_GOLD )
		void									DumpNewlyUnlockedVehicles(int startLevel, int endLevel);
		void									DumpNewlyUnlockedChallenges(int startLevel, int endLevel);
		void									DumpNewlyUnlockedAbilities(int startLevel, int endLevel);
#endif 

	};
}; // namespace Game

#endif // GAME_MULTIPLAYER_PROFILE_H
