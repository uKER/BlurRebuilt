#ifndef FRIENDS_CHALLENGES_H
#define	FRIENDS_CHALLENGES_H

#include <misc_shared/Messaging/Messaging.h>
#include <ui/Util/WideString.h>
#include <network\SystemHeaders.h>

#include "GameLogic\RaceLogicCore\Data\RLEnums.h"
#include "GameLogic\GameData\GameEventData.h"

namespace Game
{
const int MAX_FRIEND_DEMANDS=50;

const int MAX_FRIEND_CHALLENGE_USERNAME=16;

const int MAX_FRIEND_CHALLENGE_NAME=32;
const int MAX_FRIEND_CHALLENGE_DESC=64;

const int MAX_FRIEND_DEMAND_INTERNAL_PARAMS=9;

const int MAX_FRIEND_DEMAND_SENT_PARAMS=1;

const int MAX_FRIEND_DEMAND_FRIENDS=3;

const int MAX_FRIEND_DEMAND_STATS=MAX_FRIENDS + 10;	//add some padding, so if people delete and re-add they won't lose stats

const int MAX_FRIEND_DEMAND_KEY=4;

enum FRIEND_CHALLENGE_MODIFIER
{
	FRIEND_CHALLENGE_MODIFIER_NOT_USED,

	FRIEND_CHALLENGE_MODIFIER_LESS_THAN,
	FRIEND_CHALLENGE_MODIFIER_GREATER_THAN,
	FRIEND_CHALLENGE_MODIFIER_EQUALS,

	FRIEND_CHALLENGE_MODIFIER_MAX,
};

enum FRIEND_CHALLENGE_TYPE
{
	FRIEND_CHALLENGE_TYPE_RACE,
	FRIEND_CHALLENGE_TYPE_FAN_DEMAND,
	FRIEND_CHALLENGE_TYPE_DESTRUCTION,
	FRIEND_CHALLENGE_TYPE_CHECKPOINT,
	FRIEND_CHALLENGE_TYPE_BOSS,

	FRIEND_CHALLENGE_TYPE_MAX,
};

enum FRIEND_CHALLENGE_PARAM_TYPE
{
	FRIEND_CHALLENGE_PARAM_TYPE_NONE,

	FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT,
	FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT,
	FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT,
	FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT,
	FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED,	
	FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE,
	FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME,	
	FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED,
	FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY,
	FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED,
	FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED,
	FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED,
	FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED,
	FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED,
	FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED,
	FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY,
	FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS,

	FRIEND_CHALLENGE_PARAM_TYPE_MAX,
};

enum FRIEND_CHALLENGE_RESULT_TYPE
{
	FRIEND_CHALLENGE_RESULT_TYPE_FAILED,
	FRIEND_CHALLENGE_RESULT_TYPE_FAILING,

	FRIEND_CHALLENGE_RESULT_TYPE_PASSING,
	FRIEND_CHALLENGE_RESULT_TYPE_PASSED,

	FRIEND_CHALLENGE_RESULT_TYPE_MAX,
};

enum FRIEND_CHALLENGE_TOTT_STAGE
{
	FRIEND_CHALLENGE_TOTT_STAGE_NOTHING,
	FRIEND_CHALLENGE_TOTT_STAGE_GETTING_NUMBER,
	FRIEND_CHALLENGE_TOTT_STAGE_GETTING_STATS,

	FRIEND_CHALLENGE_TOTT_STAGE_ALL_DONE,
	FRIEND_CHALLENGE_TOTT_STAGE_ERROR,
};

enum FRIEND_CHALLENGE_PARAM_USE
{
	FRIEND_CHALLENGE_PARAM_USE_NONE,

	FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN,
	FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS,
	FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN,
	FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS,
};

struct FriendChallengeParameter
{
	void Clear()
	{
		mU32Value=0;
		mModifier=FRIEND_CHALLENGE_MODIFIER_NOT_USED;
		mParamType=FRIEND_CHALLENGE_PARAM_TYPE_NONE;
	};

	union
	{
		U32 mU32Value;
		float mFloatValue;
	};

	int mModifier;
	FRIEND_CHALLENGE_PARAM_TYPE mParamType;
};

struct FriendChallenge
{
	FRIEND_CHALLENGE_TYPE mType;

	WCHAR mChallenger[MAX_FRIEND_CHALLENGE_USERNAME];

	Time::C_Time mDate;

	WCHAR mName[MAX_FRIEND_CHALLENGE_NAME];
	WCHAR mDescription[MAX_FRIEND_CHALLENGE_DESC];

	U32 mVehicleUsed;

	U8 mMedal;
	U8 mDifficulty;

	U16 mAttempts;

	U32 mEventID;

	bool mReChallenge;

	union
	{
		U32 mU32Criteria;
		float mFloatCriteria;
	};

	FriendChallengeParameter mParameters[MAX_FRIEND_DEMAND_SENT_PARAMS];

	BYTE mKey[MAX_FRIEND_DEMAND_KEY];

	S32 mBossMod;
};

struct FriendChallengeStats
{
	void Clear()
	{
		mUserId=0;
		mSent=mCompleted=mMostAttempts=mKOs=mTotalAttempts=0;

		mSpare=0;
	}

	bdOnlineUserID mUserId;
	
	U32 mSent;
	U32 mCompleted;
	U32 mMostAttempts;
	U32 mKOs;
	U32 mTotalAttempts;
	U32 mSpare;
};



enum FRIEND_CHALLENGE_SORT_TYPE
{
	FRIEND_CHALLENGE_SORT_TYPE_NEW,
	FRIEND_CHALLENGE_SORT_TYPE_ACTIVE,
	FRIEND_CHALLENGE_SORT_TYPE_ALPHA,
};

struct FriendChallengeSort
{
	int		mFriendIndex;
	int		mValue;
	WCHAR*	mName;
};

struct FriendChallengeSorter
{
public:
	FriendChallengeSorter() {};

	bool operator()(FriendChallengeSort lhs, FriendChallengeSort rhs)
	{
		return (lhs.mValue > rhs.mValue);
	}
};

struct FriendChallengeAlphaSorter
{
public:
	FriendChallengeAlphaSorter() {};

	bool operator()(FriendChallengeSort lhs, FriendChallengeSort rhs)
	{
		int b=0;

		while(lhs.mName[b] && rhs.mName[b] && (tolower(lhs.mName[b]) == tolower(rhs.mName[b])))
		{
			b++;

			if(rhs.mName[b] == NULL)
			{
				return true;
			}
		}

		if(lhs.mName[b] && rhs.mName[b] && (tolower(lhs.mName[b]) < tolower(rhs.mName[b])))
		{
			return true;
		}
		
		return false;
	}
};

class C_FriendChallenges : public Messaging::C_MessageHandler
{
public:
	void Init();

	void Update();

	void Clear();

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	static C_FriendChallenges& Instance(){return s_instance;};

	const WCHAR *GetCriteriaTitleString(int index, int store_index=-1, const FriendChallenge *fc=NULL);
	const WCHAR *GetCriteriaVariableString(int index, int store_index=-1, const FriendChallenge *fc=NULL);

	FRIEND_CHALLENGE_PARAM_TYPE GetParamType(int index, int store_index=-1, const FriendChallenge *fc=NULL);
	const WCHAR *GetParamTitleString(int index, int store_index=-1, const FriendChallenge *fc=NULL);
	const WCHAR *GetParamVariableString(int index, int store_index=-1, const FriendChallenge *fc=NULL);
	const WCHAR *GetParamSwitchString(int index, int store_index=-1, const FriendChallenge *fc=NULL);

	int GetParamCount();

	U32	GetEventID(int store_index=-1, const FriendChallenge *fc=NULL);
	U8	GetMedal(int store_index=-1, const FriendChallenge *fc=NULL);
	U8	GetDifficulty(int store_index=-1, const FriendChallenge *fc=NULL);
	U32	GetVehicleUsed(int store_index=-1, const FriendChallenge *fc=NULL);
	const FriendChallengeParameter& GetExtraChallenge(int store_index=-1, const FriendChallenge *fc=NULL);
	bool IsRechallenge(int store_index=-1, const FriendChallenge *fc=NULL);

	FRIEND_CHALLENGE_TYPE GetType(){return mType;};

	FRIEND_CHALLENGE_MODIFIER GetSwitch(int index);

	void ChangeSwitch(int index, int switch_enum);
	int NumActiveSwitches();

	void CreateActualFriendDemand();

	const WCHAR *GetFriendDemandName(int store_index=-1);
	const WCHAR *GetFriendDemandDescription(int store_index=-1);

	void SetFriendDemandName(const WCHAR *name, bool do_check=true);
	void SetFriendDemandDescription(const WCHAR *desc, bool do_check=true);

	void AddFriend(int friend_slot, int friend_demand_slot, bool request_gamerpic=true);
	int NumFriendsAdded();
	int GetFriendSlot(int index);

	void RequestRemotePicture(int friend_slot, int ref);

	FriendChallenge *GetFriendDemand(int store_index=-1);

	Btl::Vector< FriendChallenge, Btl::VectorParameters_Array< MAX_FRIEND_DEMANDS > > mFriendDemandStore;
	Btl::Vector< bdOnlineUserID, Btl::VectorParameters_Array< MAX_FRIEND_DEMANDS > > mFriendDemandUserIDStore;

	Btl::Vector< FriendChallengeStats, Btl::VectorParameters_Array< MAX_FRIEND_DEMAND_STATS > > mFriendDemandStatsStore;

	int GetAttemptingFriendDemandIndex(){return mAttempting;};
	int GetAttemptingFriendDemandFriendIndex(){return mAttemptingFriend;};

	const FriendChallenge *GetAttemptingFriendDemand(){return &mFriendDemandStore[mAttempting];};
	void AttemptingFriendDemand(int index, int friend_index);
	bool IsAttemptingFriendDemand(){return (mAttempting!=-1);};

	FRIEND_CHALLENGE_RESULT_TYPE AttemptingMedalResult();
	FRIEND_CHALLENGE_RESULT_TYPE AttemptingCriteriaResult();
	FRIEND_CHALLENGE_RESULT_TYPE AttemptingParamResult(UI::C_WideString64 *score_string=NULL);

	bool GetCriteriaResults( UI::C_WideString512* title = NULL, UI::C_WideString512* info_you = NULL, UI::C_WideString512* info_them = NULL );
	bool GetParamResults( UI::C_WideString512* title = NULL, UI::C_WideString512* info_you = NULL, UI::C_WideString512* info_them = NULL );

	void ReAttemptFriendDemand();

	const WCHAR *GetPrimaryReChallengeString();
	const WCHAR *GetMedalReChallengeString();
	
	const WCHAR *GetSecondaryTitleReChallengeString();
	const WCHAR *GetSecondarySwitchReChallengeString();
	const WCHAR *GetSecondaryVarReChallengeString();

	const WCHAR *GetStoreName(int index);
	const WCHAR *GetStoreDesc(int index);
	const WCHAR *GetStoreSender(int index);
	const WCHAR *GetStoreAttempts(int index);
	const WCHAR *GetStoreTimeSent(int index);

	bool CreateReChallengeFriendDemand();

	int GetStatSlot(int friend_index);

	void AcceptDemand(int friend_slot);
	void UploadStats();

	void FinishedAttempt(int passed);

	void GetFriendStats(int friend_index);
	FriendChallengeStats mTaleOfTheTape;

	void Sort(FRIEND_CHALLENGE_SORT_TYPE type);
	Btl::Vector< FriendChallengeSort > mFriendChallengeSorted;

	FRIEND_CHALLENGE_PARAM_USE GetParamUse(FRIEND_CHALLENGE_PARAM_TYPE param);

	const FriendChallengeParameter *GetParameter(int index){return &mParameters[index];};

	int GetKeyedServerIndex();
	int GetKeyedLocalIndex();

	int HaveWeOldChallenges();
	void CleanOutOldChallenges();


	void FriendsUpdated();

private:

	void CreateFriendDemandPostRace();

	void CreateRaceDemand();
	void CreateFanAttackDemand();
	void CreateDestructionDemand();
	void CreateCheckpointDemand();
	void CreateBossBattleDemand();

	FRIEND_CHALLENGE_RESULT_TYPE GetParamResult(int value);
	FRIEND_CHALLENGE_RESULT_TYPE GetParamResult(float value);

	int GetNumPickups(E_PICKUP_TYPE type);
	int GetNumDestructionVehicles(E_DestructionMode_AI_Level type);

	static C_FriendChallenges s_instance;

	int ClearStatsForRemovedFriends();

	float mDeltaTime;

	//used to store the race variables
	FRIEND_CHALLENGE_TYPE mType;

	union
	{
		U32 mU32Criteria;
		float mFloatCriteria;
	};

	U32 mVehicleUsed;
	U8 mMedal;
	U8 mDifficulty;
	U32 mEventID;

	FriendChallengeParameter mParameters[MAX_FRIEND_DEMAND_INTERNAL_PARAMS];

	//a created friend demand
	FriendChallenge mFriendDemand;
	
	Btl::Vector<int, Btl::VectorParameters_Array< MAX_FRIEND_DEMAND_FRIENDS > > mFriendsAdded;

	int mAttempting;
	int mAttemptingFriend;

	bool mNeedToUploadStats;

	bdOnlineUserID mTOTTUser;
	U64 mNumberofTOTT;
	FRIEND_CHALLENGE_TOTT_STAGE mGettingTaleOfTheTape;

	bool mIsNameOk;
	bool mIsDescOk;

};

};

#endif //FRIENDS_CHALLENGES_H