#ifndef AMAX_GAMELOGIC_SINGLEPLAYERIMPL_H
#define AMAX_GAMELOGIC_SINGLEPLAYERIMPL_H

// THIS IS AN IMPLEMENTATION HEADER, YOU SHOULD NOT BE INCLUDING IT, PLEASE INCLUDE "GameLogic/SinglePlayer/SinglePlayer.h" INSTEAD!

#include <Game/Game.h>
#include <SinglePlayer/SinglePlayer.h>
#include <Xt/Impl/XtUt.h>

//
// Index data
//

struct SP_VehicleIndex
{
	XtId	id;
	XtI16	rewardRank;
	XtI16	rewardOrder;
	XtId	rewardBoss;

public:
	SP_VehicleIndex(XtId id_ = 0);

public:
	bool operator<(const SP_VehicleIndex& rhs) const { return id < rhs.id; }
};

struct SP_ModIndex
{
	XtId	id;
	XtI16	rewardRank;
	XtI16	rewardOrder;
	XtId	rewardBoss;

public:
	SP_ModIndex(XtId id_ = 0);

public:
	bool operator<(const SP_ModIndex& rhs) const { return id < rhs.id; }
};

struct SP_EventIndex
{
	XtU32		id;
	SP_Event*	event;

public:
	SP_EventIndex(XtU32 id_ = 0, SP_Event* event_ = 0);

public:
	bool operator<(const SP_EventIndex& rhs) const { return id < rhs.id; }
};

struct SP_IndexData
{
	XtUt::Set<SP_VehicleIndex>	vehicles;
	XtUt::Set<SP_ModIndex>		mods;
	XtUt::Set<SP_EventIndex>	events;
};

//
// Save game
//

enum SP_UnlockState
{
	kSP_Locked,
	kSP_NewlyUnlocked,
	kSP_Unlocked
};

static const int kSP_MaxBossDemands = 4;

inline bool SP_IsComplete(SP_EventState state)
{
	return (state == kSP_Bronze) || (state == kSP_Silver) || (state == kSP_Gold);
}

inline bool SP_IsUnlocked(SP_UnlockState state)
{
	return (state != kSP_Locked);
}

struct SP_SaveTier
{
	XtU32	_id;
	XtU8	_unlocked; // SP_UnlockState

public:
	SP_SaveTier(XtId id = 0);

public:
	SP_UnlockState Unlocked() const { return (SP_UnlockState)_unlocked; }

	bool IsUnlocked() const	{ return SP_IsUnlocked(Unlocked()); }
	bool IsNew() const		{ return Unlocked() == kSP_NewlyUnlocked; }

public:
	bool operator<(const SP_SaveTier& rhs) const { return _id < rhs._id; }
};

struct SP_SaveBoss
{
	XtU32	_id;
	XtU32	_demands[kSP_MaxBossDemands];	// based on demand type
	XtU8	_unlocked;						// SP_UnlockState
	XtU8	_defeated;						// bool

public:
	SP_SaveBoss(XtId id = 0);

public:
	SP_UnlockState Unlocked() const { return (SP_UnlockState)_unlocked; }

	bool IsUnlocked() const	{ return SP_IsUnlocked(Unlocked()); }
	bool IsNew() const		{ return Unlocked() == kSP_NewlyUnlocked; }
	bool IsDefeated() const	{ return IsUnlocked() && (_defeated != 0); }

public:
	bool operator<(const SP_SaveBoss& rhs) const { return _id < rhs._id; }
};

struct SP_SaveEvent
{
	XtU32	_id;
	XtI32	_bestFanPar;
	XtI32	_bestPoints;
	float	_bestTime;
	XtId	_lastVehicle;
	XtU8	_unlocked;			// SP_UnlockState
	XtU8	_state;				// SP_EventState
	XtU8	_difficulty;		// Game::GAME_DIFFICULTY
	XtU8	_fanRunComplete;	// bool

public:
	SP_SaveEvent(XtId id = 0);

public:
	SP_UnlockState			Unlocked() const	{ return (SP_UnlockState)_unlocked; }
	SP_EventState			State() const		{ return (SP_EventState)_state; }
	Game::GAME_DIFFICULTY	Difficulty() const	{ return (Game::GAME_DIFFICULTY)_difficulty; }

	bool IsUnlocked() const			{ return SP_IsUnlocked(Unlocked()); }
	bool IsNew() const				{ return Unlocked() == kSP_NewlyUnlocked; }
	bool IsComplete() const			{ return IsUnlocked() && SP_IsComplete(State()); }
	bool IsFanRunComplete() const	{ return _fanRunComplete != 0; }

public:
	bool operator<(const SP_SaveEvent& rhs) const { return _id < rhs._id; }
};

struct SP_SaveVehicle
{
	XtU32	_id;
	XtU8	_unlocked; // SP_UnlockState

public:
	SP_SaveVehicle(XtId id = 0);

public:
	SP_UnlockState Unlocked() const { return (SP_UnlockState)_unlocked; }

	bool IsUnlocked() const	{ return SP_IsUnlocked(Unlocked()); }
	bool IsNew() const		{ return Unlocked() == kSP_NewlyUnlocked; }

public:
	bool operator<(const SP_SaveVehicle& rhs) const { return _id < rhs._id; }
};

struct SP_SaveMod
{
	XtU32	_id;
	XtU8	_unlocked; // SP_UnlockState

public:
	SP_SaveMod(XtId id = 0);

public:
	SP_UnlockState Unlocked() const { return (SP_UnlockState)_unlocked; }

	bool IsUnlocked() const	{ return SP_IsUnlocked(Unlocked()); }
	bool IsNew() const		{ return Unlocked() == kSP_NewlyUnlocked; }

public:
	bool operator<(const SP_SaveMod& rhs) const { return _id < rhs._id; }
};

struct SP_SaveGame
{
	XtI32	_rank;
	XtI64	_totalFans;
	XtI64	_totalFanEventFans;
	XtId	_lastVehicle;
	XtU32	_lastSessionTier;
	XtU32	_lastSessionBossUnlocked;
	XtU32	_lastSessionBossDefeated;
	XtU32	_lastSessionRankedUp;

	XtUt::Set<SP_SaveTier>		_tiers;
	XtUt::Set<SP_SaveBoss>		_bosses;
	XtUt::Set<SP_SaveEvent>		_events;
	XtUt::Set<SP_SaveVehicle>	_vehicles;
	XtUt::Set<SP_SaveMod>		_mods;

public:
	SP_SaveGame();
};

//
// Helpers
//

const char* SP_GetUnlockName(SP_UnlockKind kind);

SP_SaveTier* SP_GetSaveTier(SP_SaveGame* sg, XtId id);
SP_SaveBoss* SP_GetSaveBoss(SP_SaveGame* sg, XtId id);
SP_SaveEvent* SP_GetSaveEvent(SP_SaveGame* sg, XtId id);
SP_SaveVehicle* SP_GetSaveVehicle(SP_SaveGame* sg, XtId id);
SP_SaveMod* SP_GetSaveMod(SP_SaveGame* sg, XtId id);

//
// Globals
//

extern Debug::Logger& gSpLog;

extern SP_RankData*		gSpRankData;
extern SP_TierData*		gSpTierData;
extern SP_IndexData*	gSpIndexData;

#endif // AMAX_GAMELOGIC_SINGLEPLAYERIMPL_H