#ifndef SOCIAL_NETWORK_MANAGER_H
#define SOCIAL_NETWORK_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\GameData\GameData.h>
#include <Racebook\Profile\RaceBookProfile.h>

#include <Ui\game\Multiplayer\MultiplayerDefines.h>

namespace Game
{
	const int MAX_SOCIAL_NETWORK_ITEMS=100;

	const int MAX_SOCIAL_NETWORK_RACE_HISTORY=10;

	const int MAX_SOCIAL_NETWORK_RACE_AWARDS=5;

	const int MAX_TWEET_LENGTH=140;
	
	enum AMAX_SOCIAL_NETWORK
	{
		AMAX_SOCIAL_NETWORK_TWITTER,
		AMAX_SOCIAL_NETWORK_FACEBOOK,
		AMAX_SOCIAL_NETWORK_BLURB,
	};

	enum AMAX_SOCIAL_NETWORK_BLURB_TYPE
	{
		AMAX_SOCIAL_NETWORK_BLURB_TYPE_NORMAL,
		AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE,
		AMAX_SOCIAL_NETWORK_BLURB_TYPE_TUTORIAL,
	};

	typedef struct AmaxSocialNetworkItem
	{
		AMAX_SOCIAL_NETWORKING_ITEM_TYPE mType;

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	}AmaxSocialNetworkItem;

	typedef struct AmaxSocialNetworkRaceVehicleStore
	{
		WCHAR mName[32];
		U64 mSystemID;
		U8 mPosition;
		U8 mRank;
		U8 mLegend;
		F32 mStatistic;
		U8 mNetJoinId;
		U32 mPadIndex;
		S8 mTeam;
		U32 mVehicleId;
		SysProfile::UserIdentifier	mSysProfile;

	}AmaxSocialNetworkRaceVehicleStore;

	typedef struct AmaxSocialNetworkRaceStore
	{
		U8 mNumCars;
		Time::C_Time mDate;
		U32 mRoute;
		UI::E_RaceType mRaceType;

		S8	mMyNemesis;
		S32 mNumTimesHitByNemesis;
		U8	mNemesisNetJoinId;	
		SysProfile::UserIdentifier	mNemesisUid;


		S8	mMyPrey;
		S32 mNumTimesHitPrey;
		U8	mPreyNetJoinId;
		SysProfile::UserIdentifier	mPreyUid;


		Btl::Vector<AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> > mVehicles;
		Btl::Vector<UI::E_MultiplayerAward, Btl::VectorParameters_Array<MAX_SOCIAL_NETWORK_RACE_AWARDS> > mPlayerAwards;
		Btl::Vector< F32, Btl::VectorParameters_Array<MAX_SOCIAL_NETWORK_RACE_AWARDS> > mPlayerAwardValues;

	}AmaxSocialNetworkRaceStore;

	class C_SocialNetworkManager
	{
	public:

		C_SocialNetworkManager()
		{
			mRaceHistoryStoreIndex=0;
			for(int a=0;a<MAX_SOCIAL_NETWORK_RACE_HISTORY;a++)
			{
				mRaceHistoryStore[a].mNumCars=0;
			}
		};

		void Init();
		void Update();

		void Clear();

		void AddOneOff(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 param);
		void AddIncrement(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 id, int value, bool suffix);
		void AddShare(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, U32 id, int value, bool suffix);

		void PostRace(bool had_passed,bool multiplayer);

		void StoreDataPreRace();

		int GetNumSocialItems(){return mSocialStore.Size();};
		AmaxSocialNetworkItem *GetSocialItem(int i){return &mSocialStore[i];};

		static C_SocialNetworkManager& Instance(){return s_instance;};

		Btl::Vector<AmaxSocialNetworkItem> mSocialStore;

		const WCHAR *GetTweetString(int index);
		const WCHAR *GetBlurbString(int index);

		const WCHAR *GetBlurbServerName(int index);
		const WCHAR *GetBlurbServerDesc(int index);
		const WCHAR *GetBlurbServerSender(int index);
		const WCHAR *GetBlurbServerDaysLeft(int index);
		const WCHAR *GetBlurbServerTimeSent(int index);
		
		AmaxSocialNetworkingItem *GetSocialStoryData(int type);

		UI::C_WideString256 mCurrentMessage;

		//used when we create a facebook/tweet/blurb to send out
		const WCHAR *GetCurrentItemText(){return mCurrentItemText.Chars();};
		const AmaxSocialNetworkingItem *GetCurrentItem(){return &mCurrentItem;};

		void Create(AMAX_SOCIAL_NETWORKING_ITEM_TYPE type, AMAX_SOCIAL_NETWORK network, U32 index);

		void CreateFromIndex(AMAX_SOCIAL_NETWORK network, U32 index);

		void CreateStory(U32 index, UI::C_TextStringID id, UI::C_TextStringID singular_id);

		int GetRemainingTweetSpace();

		int GetMyRaceHistoryPos(int index);

		int GetLastNetworkRaceIndex();

		Btl::Vector<AmaxSocialNetworkRaceStore, Btl::VectorParameters_Array<MAX_SOCIAL_NETWORK_RACE_HISTORY> > mRaceHistoryStore;
		int mRaceHistoryStoreIndex;
		int mLookingAtRaceHistoryStoreIndex;

		int GetPrevRaceHistoryStoreIndex();

	private:

		static C_SocialNetworkManager s_instance;

		RaceBook::C_RaceBookProfileStatistics mPreRaceStats;

		bool OnIncrement(float old_val, float new_val, int increment, int *value);

		//used when we create a facebook/tweet/blurb to send out
		UI::C_WideString256 mCurrentItemText;
		AmaxSocialNetworkingItem mCurrentItem;

		const WCHAR *GetScoreboardName();

		UI::C_WideString256 mTweetString;

		//pre race store
		U32 mPreviousRoute;
	};

} // namespace Game

#endif // SOCIAL_NETWORK_MANAGER_H