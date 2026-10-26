#ifndef _DATA_GATHERING_H_
#define _DATA_GATHERING_H_

#include "xt\XtCoreTypes.h"
#include "bdLobby\bdLobbyCommon\bdLobbyServicesCommon.h"
#include <ui\Util\WideString.h>
#include <Network/Misc/NetDefines.h>
#include "ContentServer\RecordEventManager.h"

struct DataGatheringRecords;

namespace Game
{

class C_MultiplayerChallengeSystem;
class C_RecordEventManager;
class C_NetGameVars;
class C_LevelResult;
struct NetVehicleData;
class C_RaceModeBase;
class C_RaceBookLayer;
struct AwardSelection;
class C_MultiplayerProfile;
class C_CustomRace;

enum DATA_GATHERING_RECORD_TYPE
{
	DATA_GATHERING_RECORD_TYPE_VEHICLE_INFORMATION = 2,
	DATA_GATHERING_RECORD_TYPE_RANK_UP = 8,
	DATA_GATHERING_RECORD_TYPE_LOBBY_CHOICE = 9,
	DATA_GATHERING_RECORD_TYPE_QUIT_RACE = 12,
	DATA_GATHERING_RECORD_TYPE_MOD_SELECTION = 13,
	DATA_GATHERING_RECORD_TYPE_CHECKED_SESSION_BOARD = 15,
	DATA_GATHERING_RECORD_TYPE_HOST_MIGRATED = 16,
	DATA_GATHERING_RECORD_TYPE_VOTE_RESULTS = 18,
	DATA_GATHERING_RECORD_TYPE_CUSTOM_RACE_SETTINGS = 19,
	DATA_GATHERING_RECORD_TYPE_CHALLENGE_COMPLETED = 20,
	DATA_GATHERING_RECORD_TYPE_AWARDS_GIVEN = 21,
	DATA_GATHERING_RECORD_TYPE_CAREER_INFORMATION = 22,
	DATA_GATHERING_RECORD_TYPE_POST_RACE = 23,
	DATA_GATHERING_RECORD_TYPE_ON_TRACK_PICKUPS = 24,
	DATA_GATHERING_RECORD_TYPE_REGION_SETTING = 26,
};

class DataGathering
{
public:
	DataGathering(C_RecordEventManager& eventManager);
	void Init(DataGatheringRecords* dataGatheringRecords);
	void FlagPostEvent(DATA_GATHERING_RECORD_TYPE category);
	void PostFlaggedEvents(bdOnlineUserID& demonwareUserID);
	void PostEvent(Sys::BinarySerialiserAutoEndian& serial, DATA_GATHERING_RECORD_TYPE category);
	bool IsRecordTypeActive(U32 recordTypeValue);

	void StoreVoteResults(U8 winningVotes, U32 winningRoute, U32 losingRoute, U8 losingVotes);
	void StoreSessionboardInfo(U8 position, U16 racePoints, U8 racesPlayed);
	void StoreLobbyChoice(bool createdLobby);
	void StoreTrackPickupInformation();

#ifndef CFG_GOLD
	void PostVehicleSorceryInformation();
	void PostRouteSorceryInformation();
	void PostAwardSorceryInformation();
	void PostChallengeSorceryInformation();
	void PostAbilitySorceryInformation();
#endif //CFG_GOLD
private:

	struct VoteResults
	{
		U8 winningVotes;
		U32 winningRoute;
		U32 losingRoute;
		U8 losingVotes;

		void Reset()
		{
			winningVotes = winningRoute = losingRoute = losingVotes = 0;
		};
	};
	
	struct SessionboardStats
	{
		U8 position;
		U16 racePoints;
		U8 racesPlayed;

		void Reset()
		{
			position = racePoints = racesPlayed = 0;
		};
	}; 

	struct TrackPickupData
	{
		U32 id;
		U8 timesPickedUp;
		U8 type;

		TrackPickupData()
		{
			id = timesPickedUp = type = 0;
		};
	};

	VoteResults mVoteResults;
	SessionboardStats mSessionboardStats;
	bool mCreatedLobby;
	TrackPickupData* mTrackPickupData;
	U32	mTrackPickupNumber;


	void PostRecord(void *data, int size, EVENT_RECORD_CATEGORY category);

	void PostRouteVoteResults(Sys::BinarySerialiserAutoEndian& serial);
	void PostPostRaceStats(Sys::BinarySerialiserAutoEndian& serial);
	void PostRankedUp(Sys::BinarySerialiserAutoEndian& serial);
	void PostLobbyChoice(Sys::BinarySerialiserAutoEndian& serial);
	void PostQuitRace(Sys::BinarySerialiserAutoEndian& serial);
	void PostModSelection(Sys::BinarySerialiserAutoEndian& serial);
	void PostVehicleInformation(Sys::BinarySerialiserAutoEndian& serial);
	void PostCheckedSessionBoard(Sys::BinarySerialiserAutoEndian& serial);
	void PostHostMigrated(Sys::BinarySerialiserAutoEndian& serial);
	void PostCustomRaceSettings(Sys::BinarySerialiserAutoEndian& serial);
	void PostChallengeCompleted(Sys::BinarySerialiserAutoEndian& serial);
	void PostAwardInformation(Sys::BinarySerialiserAutoEndian& serial);
	void PostPlayerCareerInformation(Sys::BinarySerialiserAutoEndian& serial);
	void PostRegionSetting(Sys::BinarySerialiserAutoEndian& serial);
	void PostOnTrackPickups(Sys::BinarySerialiserAutoEndian& serial);

	DataGatheringRecords*	m_RecordsActive;
	Btl::BitArray32			m_FlaggedEvents;

	C_RecordEventManager& m_eventManager;
	F32 mVehicleCollisionTimes[RacerNet::NET_MAX_VEHICLES]; //Lets use index 0 for environment collisions

#ifndef CFG_GOLD
	enum SORCERY_DATA_TYPE //Add to the end only or the scripts will all need to be changed
	{
		SORCERY_DATA_TYPE_VEHICLE = 0,
		SORCERY_DATA_TYPE_ROUTE,
		SORCERY_DATA_TYPE_AWARD,
		SORCERY_DATA_TYPE_CHALLENGE,
		SORCERY_DATA_TYPE_CHALLENGEPACK,
		SORCERY_DATA_TYPE_ABILITY,
	};
#endif
};


}

#endif // _DATA_GATHERING_H_