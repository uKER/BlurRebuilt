#ifndef SCOREBOARD_MANAGER_H
#define SCOREBOARD_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>
#include <Network\Misc\NetDefines.h>
#include <Network/ScoreboardReaderByRank.h>
#include <Network/ScoreboardReaderByUsers.h>

#include <network/ScoreboardStructures.h>

#include <network\FriendEnumerator.h>

namespace NetPlay
{
	class C_PlayerSkillWriter;
	class C_Session;
}

namespace Scoreboard
{
	class C_Writer;
}

namespace Game
{

const int MAX_SCOREBOARD_FRIENDS=101;
const int MAX_SCOREBOARD_LOBBYPROFILES=20;
const int MAX_FRIENDS_SCOREBOARD_ROWS=3;

typedef enum 
{
	eSPFansEarned = 0,
	eSPLapTime,
	eSPCompTime,
	eSPWrecks,
	eSPVehicleId,
	eSPDifficultyLevel,
}eSPScoreboardOrder;

struct SessionScoreboardData
{
	WCHAR* m_Name; // Gamertag

#if defined(LIVE_ON_WINDOWS)
	XUID m_Uid; // User Id
#elif defined (PLATFORM_PS3)
	SceNpId m_Uid;
#endif

	bool m_IsLocal;
	U8	m_Rank;	// current online rank.
	U8	m_Legend;	// Number the rank wrapped around
	U16 m_RacePointsThisSession;	//	Number of "race points" gained in this network session.
	U16 m_RaceWinsThisSession;	//	Number of "race wins" in this network session.
	U16 m_RacesPlayedThisSession;	//	Number of "races played" in this network session.
	S8 m_Team;			// Team either 1 or 0, -1 means not a team game

	SessionScoreboardData& operator = (const SessionScoreboardData &sessionData)
	{
		m_Name = sessionData.m_Name;
#if defined(PLATFORM_PS3)
		m_Uid = sessionData.m_Uid;
#endif
		m_IsLocal = sessionData.m_IsLocal;
		m_Rank = sessionData.m_Rank;
		m_Legend = sessionData.m_Legend;
		m_RacePointsThisSession = sessionData.m_RacePointsThisSession;
		m_RaceWinsThisSession = sessionData.m_RaceWinsThisSession;
		m_RacesPlayedThisSession = sessionData.m_RacesPlayedThisSession;
		m_Team = sessionData.m_Team;

		return *this;
	}
};

class C_ScoreboardManager : public Messaging::C_MessageHandler
{
public:

	typedef enum
	{
		eScoreboardTypeTop,
		eScoreboardTypeFriends,
		eScoreboardTypeYou,
		eScoreboardTypeLobby,

	}eScoreboardTypes;

	typedef enum
	{
		eWriteNotActive,

		eWriteSkillStart,
		eWriteSkillWait,

		eWriteStatsStart,
		eWriteStatsWait,

		eWriteEndSessionStart,
		eWriteEndSessionWait,

		eWriteFailed,

	}eScoreboardWriteStates;

	typedef enum
	{
		eReadNotActive,

		eReadStatsStart,
		eReadStatsWait,

		eReadFriendsStart,
		eReadFriendsWait,

		eReadFriendsStatsStart,
		eReadFriendsStatsWait,

		eReadLobbyStart,
		eReadLobbyWait,

		eReadLobbyStatsStart,
		eReadLobbyStatsWait,

		eReadPivotStatsStart,
		eReadPivotStatsWait,

	}eScoreboardReadStates;

	C_ScoreboardManager()
	{
		mScoreboardReadState = eReadNotActive;
		mScoreboardWriteState = eWriteNotActive;
		mScoreboardWriteUserIndex = 0;
		mLookingAtScoreboard = 0;
		mNumLobbyProfiles = 0;
		mNumFriendRows = 0;
	};

	void Init();
	void Update();
	static C_ScoreboardManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	void StartScoreboardWrite(void);
	bool ScoreboardWriteComplete(void);

	void StartSPScoreboardWrite(void);
	bool SPScoreboardWriteComplete(bool reTry = false);

	void StartScoreboardRead(eScoreboardTypes type);
	bool ScoreboardReadComplete(void);

	void ScoreboardSkillStart(NetPlay::C_PlayerSkillWriter& writer, NetPlay::C_Session& session);
	void ScoreboardSkillContinue(NetPlay::C_PlayerSkillWriter& writer);

	void ScoreboardStatsStart(Scoreboard::C_Writer& writer, NetPlay::C_Session& session);
	void ScoreboardStatsContinue(Scoreboard::C_Writer& writer);

	void SPScoreboardStatsStart(Scoreboard::C_Writer& writer, NetPlay::C_Session& session, bool RetryingEvent= false);
	void SPScoreboardStatsContinue(Scoreboard::C_Writer& writer);

	void ScoreboardEndSessionStart(NetPlay::C_Session& session);
	void ScoreboardEndSessionContinue(NetPlay::C_Session& session);

	void ScoreboardReadStatsStart(Scoreboard::C_ReaderByRank& reader);
	void ScoreboardReadStatsContinue(Scoreboard::C_ReaderByRank& reader);

	void ScoreboardReadFriendsStatsStart(Scoreboard::C_ReaderByUsers& reader);
	void ScoreboardReadFriendsStatsContinue(Scoreboard::C_ReaderByUsers& reader);

	void ScoreboardReadFriendsStart();
	void ScoreboardReadFriendsContinue();

	void ScoreboardReadLobbyStatsStart(Scoreboard::C_ReaderByUsers& reader);
	void ScoreboardReadLobbyStatsContinue(Scoreboard::C_ReaderByUsers& reader);

	void ScoreboardReadLobbyStart();
	void ScoreboardReadLobbyContinue();

	void ScoreboardReadPivotStatsStart(Scoreboard::C_ReaderByRank& reader);
	void ScoreboardReadPivotStatsContinue(Scoreboard::C_ReaderByRank& reader);

	void LookAtScoreboard(int sb){mLookingAtScoreboard=sb;};
	int GetLookAtScoreboard(){return mLookingAtScoreboard;};

	Scoreboard::RowData GetCurrentRowData(int row){return mRowData[mSortList[row]];};
	
	Scoreboard::RowData GetFriendsRowData(int row){return mFriendsRowData[row];};	//already sorted

	int GetCurrentRowAmount(){return mTotalRows;};
	int GetCurrentFriendRowAmount(){return mNumFriendRows;}
	int GetCurrentScoreboardSize(){return mTotalUsers;};

	void SetScoreboardRankOffset(int sbro){mScoreboardRankOffset=sbro;};
	int GetScoreboardRankOffset(){return mScoreboardRankOffset;};

	NetPlay::C_FriendsEnumerator mFriendsEnumerator;

	void UpdateSessionLeaderboard();
	Btl::Vector<SessionScoreboardData>& GetSessionScoreboardData();

	S64	GetCurrentScoreOnLeaderboard();
	void UpdateLocalLeaderboard(const S64 rating,  const Scoreboard::ColumnData* pColumnData, const U32 numColumns);

	bool GetWriteSuccess();

private:
	static C_ScoreboardManager s_instance;
	void StoreFriendLeaderboardLocally(SysProfile::UserIdentifier& profileMe);

	float mDeltaTime;

	// scoreboard writing stuff..
	eScoreboardWriteStates						mScoreboardWriteState;
	eScoreboardReadStates						mScoreboardReadState;

	U32											mScoreboardWriteUserIndex;
	U32											mScoreboardRankOffset;

	int											mLookingAtScoreboard;

	U8											mSortList[MAX_SCOREBOARD_FRIENDS];

	Scoreboard::RowData							mRowData[MAX_SCOREBOARD_FRIENDS];
	Scoreboard::RowData							mFriendsRowData[MAX_FRIENDS_SCOREBOARD_ROWS];
	U32											mNumFriendRows;

	U32											mTotalUsers;
	U32											mTotalRows;

	NetPlay::FriendProfile						mFriends[MAX_SCOREBOARD_FRIENDS];
	int											mNumFriends;

	SysProfile::BaseProfile						mLobbyProfiles[MAX_SCOREBOARD_LOBBYPROFILES];
	int											mNumLobbyProfiles;

	Btl::Vector<SessionScoreboardData>			mSessionScoreboard; 
	
	bool										IsCurrentScoreboardSP();

};

} // namespace Game

#endif // CONTROLLER_RUMBLE_MANAGER_H