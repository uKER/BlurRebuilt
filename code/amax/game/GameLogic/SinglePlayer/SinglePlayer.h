#ifndef AMAX_GAMELOGIC_SINGLEPLAYER_H
#define AMAX_GAMELOGIC_SINGLEPLAYER_H

#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic/Structures/LevelState.h>

// Rewards

struct RaceBookCharacter;

enum SP_RewardKind
{
	kSP_VehicleReward,
	kSP_ModReward
};

struct SP_Reward
{
	SP_RewardKind kind;

public:
	SP_Reward(SP_RewardKind kind_);
};

struct SP_VehicleReward : public SP_Reward
{
	XtHandle<AmaxVehicle> vehicle;

public:
	SP_VehicleReward();
};

struct SP_ModReward : public SP_Reward
{
	XtHandle<MultiplayerAbilityData> mod;

public:
	SP_ModReward();
};

// Ranks

struct SP_Rank
{
	XtI64						fans;
	XtLocId						name;
	XtArray<SP_VehicleReward>	vehicles;
	XtArray<SP_ModReward>		mods;
};

struct SP_RankData
{
	XtArray<SP_Rank> ranks;
};

// Car Restrictions

enum SP_CarRestrictionKind
{
	kSP_ClassCarRestriction,
	kSP_SetCarsRestriction
};

struct SP_CarRestriction
{
	SP_CarRestrictionKind kind;
	AmaxCarClass carClass;

public:
	SP_CarRestriction(SP_CarRestrictionKind kind_);
};

struct SP_ClassCarRestriction : public SP_CarRestriction
{
public:
	SP_ClassCarRestriction();
};

struct SP_SetCarsRestriction : public SP_CarRestriction
{
	XtArray<XtHandle<AmaxVehicle> >	vehicles;
	XtLocId							description;

public:
	SP_SetCarsRestriction();
};

// Events

struct SP_Tier;

enum SP_EventState
{
	kSP_Incomplete,
	kSP_Bronze,
	kSP_Silver,
	kSP_Gold
};

enum SP_EventKind
{
	kSP_RaceEvent,
	kSP_FanEvent,
	kSP_CheckpointEvent,
	kSP_DestructionEvent,
	kSP_BossEvent
};

struct SP_Event
{
	SP_EventKind	kind;
	int				index;
	SP_Tier*		tier;

public:
	XtU32				id;
	XtHandle<AmaxEvent>	event;
	SP_CarRestriction*	carRestriction;
	XtI32				fanPar;

public:
	SP_Event(SP_EventKind kind_);
};

struct SP_RaceEvent : public SP_Event			{ SP_RaceEvent();			};
struct SP_FanEvent : public SP_Event			{ SP_FanEvent();			};
struct SP_CheckpointEvent : public SP_Event		{ SP_CheckpointEvent();		};
struct SP_DestructionEvent : public SP_Event	{ SP_DestructionEvent();	};
struct SP_BossEvent : public SP_Event			{ SP_BossEvent();			};

// Boss demands

struct SP_Boss;

enum SP_BossDemandKind
{
	kSP_BossDemand_Tier_Use_Powerups,
	kSP_BossDemand_Tier_Earn_Fans,
	kSP_BossDemand_Tier_Finish_In_Different_Car,
	kSP_BossDemand_Tier_Finish_Events,

	kSP_BossDemand_Tier_Shield_Powerups,
	kSP_BossDemand_Tier_Shock_Opponents,
	kSP_BossDemand_Tier_Repair_Car,
	kSP_BossDemand_Tier_Seconds_In_First,
	kSP_BossDemand_Event_Time_Remaining,
	kSP_BossDemand_Tier_Collect_Stopwatches,

	kSP_BossDemand_Tier_Complete_FanDemands,
	kSP_BossDemand_Tier_Complete_FanRuns,
	kSP_BossDemand_Tier_Complete_FanPars,
	kSP_BossDemand_Event_Get_5X_Combo,

	kSP_BossDemand_Tier_Ram_Into_Sea,
	kSP_BossDemand_Tier_Wreck,
	kSP_BossDemand_Tier_Barge_Hits,
	kSP_BossDemand_Event_Get_All_Lights,

	kSP_BossDemand_Tier_Barge_Off_Cliff,
	kSP_BossDemand_Tier_Shunt_Hits,
	kSP_BossDemand_Tier_Forward_Mine_Hits,
	kSP_BossDemand_Event_Complete_FanRun,
	kSP_BossDemand_Tier_Destruction_Bonus_Hits,

	kSP_BossDemand_Tier_Drift,
	kSP_BossDemand_Tier_Use_Weapon_Defence,
	kSP_BossDemand_Tier_Clean_Nitro,
	kSP_BossDemand_Tier_Clean_Triple_Nitros,
	kSP_BossDemand_Tier_Get_Lights,

	kSP_BossDemand_Tier_Mine_Hits,
	kSP_BossDemand_Tier_Nitro_Ram,
	kSP_BossDemand_Tier_Multiple_Barge_Hits,
	kSP_BossDemand_Event_Finish_In_Nats_BMW,

	kSP_BossDemand_Tier_Shield_Mines,
	kSP_BossDemand_Tier_3_Bolt_Hits,
	kSP_BossDemand_Tier_Air_Time,
	kSP_BossDemand_Tier_Win_With_1_Health,
	kSP_BossDemand_Event_Finish_Without_SorR,
	kSP_BossDemand_Event_Wreck_And_Win,

	kSP_BossDemand_Tier_Reach_Speed,
	kSP_BossDemand_Tier_Reverse_Bolt_Hits,
	kSP_BossDemand_Tier_Evade_Full_Shocks,
	kSP_BossDemand_Tier_Win_All_Events,

};

struct SP_BossDemand
{
	SP_Event*			event;

public:
	SP_BossDemandKind	kind;
	int					index;
	SP_Boss*			boss;

public:
	SP_BossDemand(SP_BossDemandKind kind_);
};

struct SP_BossDemand_Count : public SP_BossDemand
{
	XtI32 count;

public:
	SP_BossDemand_Count(SP_BossDemandKind kind_);
};

struct SP_BossDemand_Tier_Use_Powerups				: public SP_BossDemand_Count { SP_BossDemand_Tier_Use_Powerups(); };
struct SP_BossDemand_Tier_Earn_Fans					: public SP_BossDemand_Count { SP_BossDemand_Tier_Earn_Fans(); };
struct SP_BossDemand_Tier_Finish_In_Different_Car	: public SP_BossDemand_Count { SP_BossDemand_Tier_Finish_In_Different_Car(); };
struct SP_BossDemand_Tier_Finish_Events				: public SP_BossDemand_Count { SP_BossDemand_Tier_Finish_Events(); };
struct SP_BossDemand_Tier_Shield_Powerups			: public SP_BossDemand_Count { SP_BossDemand_Tier_Shield_Powerups(); };
struct SP_BossDemand_Tier_Shock_Opponents			: public SP_BossDemand_Count { SP_BossDemand_Tier_Shock_Opponents(); };
struct SP_BossDemand_Tier_Repair_Car				: public SP_BossDemand_Count { SP_BossDemand_Tier_Repair_Car(); };
struct SP_BossDemand_Tier_Seconds_In_First			: public SP_BossDemand_Count { SP_BossDemand_Tier_Seconds_In_First(); };
struct SP_BossDemand_Event_Time_Remaining			: public SP_BossDemand_Count { SP_BossDemand_Event_Time_Remaining(); };
struct SP_BossDemand_Tier_Collect_Stopwatches		: public SP_BossDemand_Count { SP_BossDemand_Tier_Collect_Stopwatches(); };
struct SP_BossDemand_Tier_Complete_FanDemands		: public SP_BossDemand_Count { SP_BossDemand_Tier_Complete_FanDemands(); };
struct SP_BossDemand_Tier_Complete_FanRuns			: public SP_BossDemand_Count { SP_BossDemand_Tier_Complete_FanRuns(); };
struct SP_BossDemand_Tier_Complete_FanPars			: public SP_BossDemand_Count { SP_BossDemand_Tier_Complete_FanPars(); };
struct SP_BossDemand_Event_Get_5X_Combo				: public SP_BossDemand_Count { SP_BossDemand_Event_Get_5X_Combo(); };
struct SP_BossDemand_Tier_Ram_Into_Sea				: public SP_BossDemand_Count { SP_BossDemand_Tier_Ram_Into_Sea(); };
struct SP_BossDemand_Tier_Wreck						: public SP_BossDemand_Count { SP_BossDemand_Tier_Wreck(); };
struct SP_BossDemand_Tier_Barge_Hits				: public SP_BossDemand_Count { SP_BossDemand_Tier_Barge_Hits(); };
struct SP_BossDemand_Event_Get_All_Lights			: public SP_BossDemand_Count { SP_BossDemand_Event_Get_All_Lights(); };
struct SP_BossDemand_Tier_Barge_Off_Cliff			: public SP_BossDemand_Count { SP_BossDemand_Tier_Barge_Off_Cliff(); };
struct SP_BossDemand_Tier_Shunt_Hits				: public SP_BossDemand_Count { SP_BossDemand_Tier_Shunt_Hits(); };
struct SP_BossDemand_Tier_Forward_Mine_Hits			: public SP_BossDemand_Count { SP_BossDemand_Tier_Forward_Mine_Hits(); };
struct SP_BossDemand_Event_Complete_FanRun			: public SP_BossDemand_Count { SP_BossDemand_Event_Complete_FanRun(); };
struct SP_BossDemand_Tier_Destruction_Bonus_Hits	: public SP_BossDemand_Count { SP_BossDemand_Tier_Destruction_Bonus_Hits(); };
struct SP_BossDemand_Tier_Drift						: public SP_BossDemand_Count { SP_BossDemand_Tier_Drift(); };
struct SP_BossDemand_Tier_Use_Weapon_Defence		: public SP_BossDemand_Count { SP_BossDemand_Tier_Use_Weapon_Defence(); };
struct SP_BossDemand_Tier_Clean_Nitro				: public SP_BossDemand_Count { SP_BossDemand_Tier_Clean_Nitro(); };
struct SP_BossDemand_Tier_Clean_Triple_Nitros		: public SP_BossDemand_Count { SP_BossDemand_Tier_Clean_Triple_Nitros(); };
struct SP_BossDemand_Tier_Get_Lights				: public SP_BossDemand_Count { SP_BossDemand_Tier_Get_Lights(); };
struct SP_BossDemand_Tier_Mine_Hits					: public SP_BossDemand_Count { SP_BossDemand_Tier_Mine_Hits(); };
struct SP_BossDemand_Tier_Nitro_Ram					: public SP_BossDemand_Count { SP_BossDemand_Tier_Nitro_Ram(); };
struct SP_BossDemand_Tier_Multiple_Barge_Hits		: public SP_BossDemand_Count { SP_BossDemand_Tier_Multiple_Barge_Hits(); };
struct SP_BossDemand_Event_Finish_In_Nats_BMW		: public SP_BossDemand_Count { SP_BossDemand_Event_Finish_In_Nats_BMW(); XtHandle<AmaxVehicle> vehicle;	};
struct SP_BossDemand_Tier_Shield_Mines				: public SP_BossDemand_Count { SP_BossDemand_Tier_Shield_Mines(); };
struct SP_BossDemand_Tier_3_Bolt_Hits				: public SP_BossDemand_Count { SP_BossDemand_Tier_3_Bolt_Hits(); };
struct SP_BossDemand_Tier_Air_Time					: public SP_BossDemand_Count { SP_BossDemand_Tier_Air_Time(); };
struct SP_BossDemand_Tier_Win_With_1_Health			: public SP_BossDemand_Count { SP_BossDemand_Tier_Win_With_1_Health(); };
struct SP_BossDemand_Event_Finish_Without_SorR		: public SP_BossDemand_Count { SP_BossDemand_Event_Finish_Without_SorR(); };
struct SP_BossDemand_Event_Wreck_And_Win			: public SP_BossDemand_Count { SP_BossDemand_Event_Wreck_And_Win(); S32 numcars; };
struct SP_BossDemand_Tier_Reach_Speed				: public SP_BossDemand_Count { SP_BossDemand_Tier_Reach_Speed(); F32 minspeed;};
struct SP_BossDemand_Tier_Reverse_Bolt_Hits			: public SP_BossDemand_Count { SP_BossDemand_Tier_Reverse_Bolt_Hits(); };
struct SP_BossDemand_Tier_Evade_Full_Shocks			: public SP_BossDemand_Count { SP_BossDemand_Tier_Evade_Full_Shocks(); };
struct SP_BossDemand_Tier_Win_All_Events			: public SP_BossDemand_Count { SP_BossDemand_Tier_Win_All_Events(); };

// Bosses

struct SP_Boss
{
	XtU32	id;
	XtLocId	name;
	XtLocId	bio;

	XtArray<SP_Reward*>		rewardsForUnlock;
	XtArray<SP_Reward*>		rewardsForDefeat;
	XtArray<SP_BossDemand*>	demands;

public:
	SP_Tier* tier;
};

// Tiers

struct SP_Tier
{
	XtU32				id;
	XtLocId				name;
	XtI32				starRequirement;
	XtArray<SP_Event*>	events;
	SP_Boss*			boss;

public:
	int index;
};

struct SP_TierRoot
{
	XtArray<SP_Tier*> tiers;
};

struct SP_TierData
{
	SP_TierRoot*		tierRoot;
	XtArray<SP_Tier>	tiers;
	XtArray<SP_Event*>	events;
	XtArray<SP_Boss>	bosses;
};

// Save game

enum SP_UnlockKind
{
	kSP_UnlockTier,
	kSP_UnlockBoss,
	kSP_UnlockEvent,
	kSP_UnlockRank,
	kSP_UnlockVehicle,
	kSP_UnlockMod,
	kSP_UnlockBossDefeated
};

struct SP_Unlock
{
	SP_UnlockKind	kind;
	XtU32			id;

public:
	SP_Unlock(SP_UnlockKind kind_, XtU32 id_) : kind(kind_), id(id_) { }
};

typedef Btl::Vector<SP_Unlock> SP_UnlockList;

struct SP_SaveGame;

// SinglePlayerAbilityLoadOut

struct SinglePlayerAbilityLoadOut : public MultiplayerAbilityLoadOut
{
	XtHandle<MultiplayerAbilityData> mAbilityBossSlot;
};

// Rank

int SP_GetRankCount(SP_SaveGame* sg);
SP_Rank* SP_GetRank(SP_SaveGame* sg, int rankNo);

int SP_FindRankByFans(SP_SaveGame* sg, XtI64 fans);

// Events

const char* SP_GetEventStateName(SP_EventState state);

SP_Event* SP_GetEvent(SP_SaveGame* sg, SP_Tier* tier, int eventNo);
SP_Event* SP_GetEvent(SP_SaveGame* sg, int tierNo, int eventNo);
SP_Event* SP_GetEventById(SP_SaveGame* sg, XtId eventId);
SP_Event* SP_GetEventByAmaxEventId(SP_SaveGame* sg, XtId amaxEventId);

int SP_CountTotalEventStars(SP_SaveGame* sg, SP_Event* event);
int SP_CountMaxEventStars(SP_SaveGame* sg, SP_Event* event);
int SP_CountMaxPrimaryEventStars(SP_SaveGame* sg, SP_Event* event);

XtId SP_GetEventLastVehicle(SP_SaveGame* sg, XtId eventId);
void SP_SetEventLastVehicle(SP_SaveGame* sg, XtId eventId, XtId vehicleId);

SP_EventState SP_GetEventState(SP_SaveGame* sg, SP_Event* event);
Game::GAME_DIFFICULTY SP_GetEventDifficulty(SP_SaveGame* sg, SP_Event* event);
bool SP_IsEventFanParComplete(SP_SaveGame* sg, SP_Event* event);
bool SP_IsEventFanRunComplete(SP_SaveGame* sg, SP_Event* event);

int SP_GetEventBestFans(SP_SaveGame* sg, SP_Event* event);
int SP_GetEventBestPoints(SP_SaveGame* sg, SP_Event* event);
float SP_GetEventBestTime(SP_SaveGame* sg, SP_Event* event);

// Tiers

int SP_GetTierCount(SP_SaveGame* sg);
SP_Tier* SP_GetTier(SP_SaveGame* sg, int tierNo);
SP_Tier* SP_GetTierById(SP_SaveGame* sg, XtId tierId);

int SP_CountTotalTierStars(SP_SaveGame* sg, SP_Tier* tier);
int SP_CountMaxTierStars(SP_SaveGame* sg, SP_Tier* tier);

// Bosses

SP_Boss* SP_GetBossById(SP_SaveGame* sg, XtId bossId);
bool SP_IsBossDefeated(SP_SaveGame* sg, SP_Boss* boss);
bool SP_IsBossUnlocked(SP_SaveGame* sg, SP_Boss* boss);

// Game

int SP_GetRank(SP_SaveGame* sg);
float SP_GetRankProgress(SP_SaveGame* sg);
XtU64 SP_GetFans(SP_SaveGame* sg);
XtU64 SP_GetFanEventFans(SP_SaveGame* sg);
int SP_CountTotalStars(SP_SaveGame* sg);
int SP_CountMaxStars(SP_SaveGame* sg);
void SP_GameSetLastVehicle(SP_SaveGame* sg, XtId vehicleId);
XtId SP_GameGetLastVehicle(SP_SaveGame* sg);

// Vehicles

bool SP_IsVehicleUnlocked(SP_SaveGame* sg, XtId vehicleId);
bool SP_IsVehicleNew(SP_SaveGame* sg, XtId vehicleId);
int SP_GetVehiclesAvailableCount(SP_SaveGame* sg);

// Mods

bool SP_IsModUnlocked(SP_SaveGame* sg, XtId modId);
bool SP_IsModNew(SP_SaveGame* sg, XtId modId);

// Save game

SP_SaveGame* SP_CreateSaveGame();
void SP_ExportSaveGame(SP_SaveGame* sg, Sys::BinarySerialiserAutoEndian& s);
void SP_ImportSaveGame(SP_SaveGame* sg, Sys::BinaryDeserialiserAutoEndian& s);

// Unlocks

void SP_ProcessUnlocks(SP_SaveGame* sg, SP_UnlockList& unlocks);
void SP_MarkNotNew(SP_SaveGame* sg, XtId id);

// Boss demands

const char* SP_GetBossDemandName(SP_BossDemandKind kind);
SP_BossDemand* SP_GetBossDemand(SP_SaveGame* sg, SP_Boss* boss, int index);
bool SP_DoesBossDemandUseIntValue(SP_SaveGame* sg, SP_BossDemand* demand);
bool SP_DoesBossDemandUseFloatValue(SP_SaveGame* sg, SP_BossDemand* demand);
XtU32 SP_GetBossDemandTargetInt(SP_SaveGame* sg, SP_BossDemand* demand);
float SP_GetBossDemandTargetFloat(SP_SaveGame* sg, SP_BossDemand* demand);
XtU32 SP_GetBossDemandValueInt(SP_SaveGame* sg, SP_BossDemand* demand);
float SP_GetBossDemandValueFloat(SP_SaveGame* sg, SP_BossDemand* demand);
void SP_SetBossDemandValueInt(SP_SaveGame* sg, SP_BossDemand* demand, XtU32 value);
void SP_SetBossDemandValueFloat(SP_SaveGame* sg, SP_BossDemand* demand, float value);
bool SP_IsBossDemandComplete(SP_SaveGame* sg, SP_BossDemand* demand);
float SP_GetBossDemandPercentageComplete(SP_SaveGame* sg, SP_BossDemand* demand);

// Misc

SP_SaveGame* SP_GetCurrentSaveGame();

#endif // AMAX_GAMELOGIC_SINGLEPLAYER_H
