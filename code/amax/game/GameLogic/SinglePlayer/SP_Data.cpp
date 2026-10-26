#include "StdAfx.h"
#include "SinglePlayerImpl.h"

Debug::Logger& gSpLog = Debug::GetLogger("SinglePlayer");

SP_RankData*	gSpRankData;
SP_TierData*	gSpTierData;
SP_IndexData*	gSpIndexData;

SP_VehicleIndex::SP_VehicleIndex(XtU32 id_) :
	id(id_),
	rewardRank(0),
	rewardOrder(0),
	rewardBoss(0)
{
}

SP_ModIndex::SP_ModIndex(XtU32 id_) :
	id(id_),
	rewardRank(0),
	rewardOrder(0),
	rewardBoss(0)
{
}

SP_EventIndex::SP_EventIndex(XtU32 id_, SP_Event* event_) :
	id(id_),
	event(event_)
{
}

void SP_RecordVehicleIndex(SP_IndexData* idx, XtId id, int rewardRank, int rewardOrder, XtId rewardBoss)
{
	if (XtUt::Exists(&idx->vehicles, SP_VehicleIndex(id)))
	{
		LOGWARN(gSpLog, "SP: Duplicate vehicle reward id %08X", id);
		return;
	}

	SP_VehicleIndex vi(id);

	vi.rewardRank	= rewardRank;
	vi.rewardOrder	= rewardOrder;
	vi.rewardBoss	= rewardBoss;

	XtUt::Insert(&idx->vehicles, vi);
}

void SP_RecordModIndex(SP_IndexData* idx, XtId id, int rewardRank, int rewardOrder, XtId rewardBoss)
{
	if (XtUt::Exists(&idx->mods, SP_ModIndex(id)))
	{
		LOGWARN(gSpLog, "SP: Duplicate mod reward id %08X", id);
		return;
	}

	SP_ModIndex vi(id);

	vi.rewardRank	= rewardRank;
	vi.rewardOrder	= rewardOrder;
	vi.rewardBoss	= rewardBoss;

	XtUt::Insert(&idx->mods, vi);
}

void SP_RecordReward(SP_IndexData* idx, SP_Reward* reward, int rewardRank, int rewardOrder, XtId rewardBoss)
{
	if (!reward)
		return;

	if (reward->kind == kSP_VehicleReward)
		SP_RecordVehicleIndex(idx, ((SP_VehicleReward*)reward)->vehicle.Id(), rewardRank, rewardOrder, rewardBoss);
	else if (reward->kind == kSP_ModReward)
		SP_RecordModIndex(idx, ((SP_ModReward*)reward)->mod.Id(), rewardRank, rewardOrder, rewardBoss);
}

void SP_GenerateIndex()
{
	delete gSpIndexData;

	gSpIndexData = 0;

	if (!gSpRankData || !gSpTierData)
		return;

	XtUt::Ptr<SP_IndexData> idx = new SP_IndexData;

	if (!idx)
		return;

	int vehicleCount = 0;
	int modCount = 0;

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		vehicleCount	+= rank->vehicles.count;
		modCount		+= rank->mods.count;
	}

	XtUt::Reserve(&idx->vehicles.elements, vehicleCount);
	XtUt::Reserve(&idx->mods.elements, modCount);
	XtUt::Reserve(&idx->events.elements, gSpTierData->events.count);

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		for(int i = 0; i < rank->vehicles.count; i++)
			SP_RecordVehicleIndex(idx, rank->vehicles[i].vehicle.Id(), rankNo, i, 0);

		for(int i = 0; i < rank->mods.count; i++)
			SP_RecordModIndex(idx, rank->mods[i].mod.Id(), rankNo, i, 0);
	}

	for(int i = 0; i < gSpTierData->bosses.count; i++)
	{
		SP_Boss* boss = &gSpTierData->bosses[i];

		int tierIndex = -1;

		if (boss->tier)
			tierIndex = boss->tier->index;

		for(int rewardNo = 0; rewardNo < boss->rewardsForUnlock.count; rewardNo++)
		{
			if (SP_Reward* reward = boss->rewardsForUnlock[rewardNo])
				SP_RecordReward(idx, reward, 0, tierIndex, boss->id);
		}

		for(int rewardNo = 0; rewardNo < boss->rewardsForDefeat.count; rewardNo++)
		{
			if (SP_Reward* reward = boss->rewardsForDefeat[rewardNo])
				SP_RecordReward(idx, reward, 0, tierIndex, boss->id);
		}
	}

	for(int i = 0; i < gSpTierData->events.count; i++)
	{
		SP_Event* event = gSpTierData->events[i];

		if (!event)
			continue;

		if (XtUt::Exists(&idx->events, SP_EventIndex(event->id)))
		{
			LOGWARN(gSpLog, "SP: Duplicate event id %08X", event->id);
			continue;
		}

		XtUt::Insert(&idx->events, SP_EventIndex(event->id, event));
	}

	gSpIndexData = Pop(&idx);

	// TODO: Verify each rank requires more fans than the last
	// TODO: Verify rank zero is empty

	// TODO: Verify tier root exists
	// TODO: Verify each tier is only used once
	// TODO: Verify each event is only used once

	// TODO: Store index in Tiers
	// TODO: Store index and tier in Event
}

void SpRankDataProc(XtEventType event, XtRecord* record, SP_RankData* data, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
		gSpRankData = data;
	else if (event == kXtEtDestruct)
		gSpRankData = 0;

	SP_GenerateIndex();
}

void SpTierDataProc(XtEventType event, XtRecord* record, SP_TierData* data, XtCallbackContext* context)
{
	gSpTierData = 0;

	if (event == kXtEtConstruct)
	{
		if (SP_TierRoot* root = data->tierRoot)
		{
			// Number tiers and events

			for(int i = 0; i < root->tiers.count; i++)
			{
				if (SP_Tier* tier = root->tiers[i])
				{
					tier->index = i;

					for(int j = 0; j < tier->events.count; j++)
					{
						if (SP_Event* event = tier->events[j])
						{
							event->index	= j;
							event->tier		= tier;
						}
					}

					// Mark which bosses belong to which tiers

					if (SP_Boss* boss = tier->boss)
						boss->tier = tier;
				}
			}

			// Number boss demands

			for(int i = 0; i < data->bosses.count; i++)
			{
				SP_Boss* boss = &data->bosses[i];

				for(int j = 0; j < boss->demands.count; j++)
				{
					if (SP_BossDemand* demand = boss->demands[j])
					{
						demand->index	= j;
						demand->boss	= boss;
					}
				}
			}

			gSpTierData = data;
		}
	}

	SP_GenerateIndex();
}

// Rewards

XT_ABSTRACT_TYPE(SP_Reward, "SP_Reward")
XT_END_TYPE

XT_TYPE(SP_VehicleReward, "SP_VehicleReward")
	XT_BASE(SP_Reward)
	XT_FIELD(vehicle, "vehicle")
XT_END_TYPE

XT_TYPE(SP_ModReward, "SP_ModReward")
	XT_BASE(SP_Reward)
	XT_FIELD(mod, "mod")
XT_END_TYPE

// Ranks

XT_TYPE(SP_Rank, "SP_Rank")
	XT_FIELD(fans, "fans")
	XT_FIELD(name, "name")
	XT_FIELD(vehicles, "vehicles")
	XT_FIELD(mods, "mods")
XT_END_TYPE

XT_TYPE(SP_RankData, "SP_RankData")
	XT_CALLBACK(SpRankDataProc)
	XT_FIELD(ranks, "ranks")
XT_END_TYPE

// Car Restrictions

XT_ABSTRACT_TYPE(SP_CarRestriction, "SP_CarRestriction")
	XT_FIELD(carClass, "carClass")
XT_END_TYPE

XT_TYPE(SP_ClassCarRestriction, "SP_ClassCarRestriction")
	XT_BASE(SP_CarRestriction)
XT_END_TYPE

XT_TYPE(SP_SetCarsRestriction, "SP_SetCarsRestriction")
	XT_BASE(SP_CarRestriction)
	XT_FIELD(vehicles, "vehicles")
	XT_FIELD(description, "description")
XT_END_TYPE

// Events

XT_ABSTRACT_TYPE(SP_Event, "SP_Event")
	XT_FIELD(id, "id")
	XT_FIELD(event, "event")
	XT_FIELD(carRestriction, "carRestriction")
	XT_FIELD(fanPar, "fanPar")
XT_END_TYPE

XT_TYPE(SP_RaceEvent, "SP_RaceEvent")
	XT_BASE(SP_Event)
XT_END_TYPE

XT_TYPE(SP_FanEvent, "SP_FanEvent")
	XT_BASE(SP_Event)
XT_END_TYPE

XT_TYPE(SP_CheckpointEvent, "SP_CheckpointEvent")
	XT_BASE(SP_Event)
XT_END_TYPE

XT_TYPE(SP_DestructionEvent, "SP_DestructionEvent")
	XT_BASE(SP_Event)
XT_END_TYPE

XT_TYPE(SP_BossEvent, "SP_BossEvent")
	XT_BASE(SP_Event)
XT_END_TYPE

// Boss demands

XT_ABSTRACT_TYPE(SP_BossDemand, "SP_BossDemand")
	XT_FIELD(event, "event")
XT_END_TYPE

XT_ABSTRACT_TYPE(SP_BossDemand_Count, "SP_BossDemand_Count")
	XT_BASE(SP_BossDemand)
	XT_FIELD(count, "count")
XT_END_TYPE

XT_TYPE(SP_BossDemand_Tier_Use_Powerups,			"SP_BossDemand_Tier_Use_Powerups")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Earn_Fans,				"SP_BossDemand_Tier_Earn_Fans")					XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Finish_In_Different_Car,	"SP_BossDemand_Tier_Finish_In_Different_Car")	XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Finish_Events,			"SP_BossDemand_Tier_Finish_Events")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Shield_Powerups,			"SP_BossDemand_Tier_Shield_Powerups")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Shock_Opponents,			"SP_BossDemand_Tier_Shock_Opponents")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Repair_Car,				"SP_BossDemand_Tier_Repair_Car")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Seconds_In_First,		"SP_BossDemand_Tier_Seconds_In_First")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Time_Remaining,			"SP_BossDemand_Event_Time_Remaining")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Collect_Stopwatches,		"SP_BossDemand_Tier_Collect_Stopwatches")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Complete_FanDemands,		"SP_BossDemand_Tier_Complete_FanDemands")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Complete_FanRuns,		"SP_BossDemand_Tier_Complete_FanRuns")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Complete_FanPars,		"SP_BossDemand_Tier_Complete_FanPars")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Get_5X_Combo,			"SP_BossDemand_Event_Get_5X_Combo")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Ram_Into_Sea,			"SP_BossDemand_Tier_Ram_Into_Sea")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Wreck,					"SP_BossDemand_Tier_Wreck")						XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Barge_Hits,				"SP_BossDemand_Tier_Barge_Hits")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Get_All_Lights,			"SP_BossDemand_Event_Get_All_Lights")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Barge_Off_Cliff,			"SP_BossDemand_Tier_Barge_Off_Cliff")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Shunt_Hits,				"SP_BossDemand_Tier_Shunt_Hits")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Forward_Mine_Hits,		"SP_BossDemand_Tier_Forward_Mine_Hits")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Complete_FanRun,		"SP_BossDemand_Event_Complete_FanRun")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Destruction_Bonus_Hits,	"SP_BossDemand_Tier_Destruction_Bonus_Hits")	XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Drift,					"SP_BossDemand_Tier_Drift")						XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Use_Weapon_Defence,		"SP_BossDemand_Tier_Use_Weapon_Defence")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Clean_Nitro,				"SP_BossDemand_Tier_Clean_Nitro")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Clean_Triple_Nitros,		"SP_BossDemand_Tier_Clean_Triple_Nitros")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Get_Lights,				"SP_BossDemand_Tier_Get_Lights")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Mine_Hits,				"SP_BossDemand_Tier_Mine_Hits")					XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Nitro_Ram,				"SP_BossDemand_Tier_Nitro_Ram")					XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Multiple_Barge_Hits,		"SP_BossDemand_Tier_Multiple_Barge_Hits")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Finish_In_Nats_BMW,		"SP_BossDemand_Event_Finish_In_Nats_BMW")		XT_BASE(SP_BossDemand_Count) XT_FIELD(vehicle, "vehicle") XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Shield_Mines,			"SP_BossDemand_Tier_Shield_Mines")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_3_Bolt_Hits,				"SP_BossDemand_Tier_3_Bolt_Hits")				XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Air_Time,				"SP_BossDemand_Tier_Air_Time")					XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Win_With_1_Health,		"SP_BossDemand_Tier_Win_With_1_Health")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Finish_Without_SorR,	"SP_BossDemand_Event_Finish_Without_SorR")		XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Event_Wreck_And_Win,			"SP_BossDemand_Event_Wreck_And_Win")			XT_BASE(SP_BossDemand_Count) XT_FIELD(numcars, "numcars") XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Reach_Speed,				"SP_BossDemand_Tier_Reach_Speed")				XT_BASE(SP_BossDemand_Count) XT_FIELD(minspeed, "minspeed") XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Reverse_Bolt_Hits,		"SP_BossDemand_Tier_Reverse_Bolt_Hits")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Evade_Full_Shocks,		"SP_BossDemand_Tier_Evade_Full_Shocks")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE
XT_TYPE(SP_BossDemand_Tier_Win_All_Events,			"SP_BossDemand_Tier_Win_All_Events")			XT_BASE(SP_BossDemand_Count) XT_END_TYPE

// Bosses

XT_TYPE(SP_Boss, "SP_Boss")
	XT_FIELD(id, "id")
	XT_FIELD(name, "name")
	XT_FIELD(bio, "bio")
	XT_FIELD(rewardsForUnlock, "rewardsForUnlock")
	XT_FIELD(rewardsForDefeat, "rewardsForDefeat")
	XT_FIELD(demands, "demands")
XT_END_TYPE

// Tiers

XT_TYPE(SP_Tier, "SP_Tier")
	XT_FIELD(id, "id")
	XT_FIELD(name, "name")
	XT_FIELD(starRequirement, "starRequirement")
	XT_FIELD(events, "events")
	XT_FIELD(boss, "boss")
XT_END_TYPE

XT_TYPE(SP_TierRoot, "SP_TierRoot")
	XT_FIELD(tiers, "tiers")
XT_END_TYPE

XT_TYPE(SP_TierData, "SP_TierData")
	XT_CALLBACK(SpTierDataProc)
	XT_FIELD(tierRoot, "tierRoot")
	XT_FIELD(tiers, "tiers")
	XT_FIELD(events, "events")
	XT_FIELD(bosses, "bosses")
XT_END_TYPE

// SinglePlayerAbilityLoadOut

XT_TYPE(SinglePlayerAbilityLoadOut, "SinglePlayerAbilityLoadOut")
	XT_BASE(MultiplayerAbilityLoadOut)
	XT_FIELD(mAbilityBossSlot, "BossSlot")
XT_END_TYPE

// Library

XT_LIBRARY(SinglePlayer)

	// Rewards

	XT_REGISTER_TYPE(SP_Reward)
	XT_REGISTER_TYPE(SP_VehicleReward)
	XT_REGISTER_TYPE(SP_ModReward)

	// Ranks

	XT_REGISTER_TYPE(SP_Rank)
	XT_REGISTER_TYPE(SP_RankData)

	// Car Restrictions

	XT_REGISTER_TYPE(SP_CarRestriction)
	XT_REGISTER_TYPE(SP_ClassCarRestriction)
	XT_REGISTER_TYPE(SP_SetCarsRestriction)

	// Events

	XT_REGISTER_TYPE(SP_Event)
	XT_REGISTER_TYPE(SP_RaceEvent)
	XT_REGISTER_TYPE(SP_FanEvent)
	XT_REGISTER_TYPE(SP_CheckpointEvent)
	XT_REGISTER_TYPE(SP_DestructionEvent)
	XT_REGISTER_TYPE(SP_BossEvent)

	// Boss demands

	XT_REGISTER_TYPE(SP_BossDemand)
	XT_REGISTER_TYPE(SP_BossDemand_Count)

	XT_REGISTER_TYPE(SP_BossDemand_Tier_Use_Powerups)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Earn_Fans)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Finish_In_Different_Car)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Finish_Events)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Shield_Powerups)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Shock_Opponents)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Repair_Car)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Seconds_In_First)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Time_Remaining)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Collect_Stopwatches)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Complete_FanDemands)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Complete_FanRuns)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Complete_FanPars)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Get_5X_Combo)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Ram_Into_Sea)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Wreck)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Barge_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Get_All_Lights)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Barge_Off_Cliff)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Shunt_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Forward_Mine_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Complete_FanRun)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Destruction_Bonus_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Drift)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Use_Weapon_Defence)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Clean_Nitro)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Clean_Triple_Nitros)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Get_Lights)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Mine_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Nitro_Ram)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Multiple_Barge_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Finish_In_Nats_BMW)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Shield_Mines)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_3_Bolt_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Air_Time)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Win_With_1_Health)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Finish_Without_SorR)
	XT_REGISTER_TYPE(SP_BossDemand_Event_Wreck_And_Win)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Reach_Speed)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Reverse_Bolt_Hits)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Evade_Full_Shocks)
	XT_REGISTER_TYPE(SP_BossDemand_Tier_Win_All_Events)

	// Bosses

	XT_REGISTER_TYPE(SP_Boss)

	// Tiers

	XT_REGISTER_TYPE(SP_Tier)
	XT_REGISTER_TYPE(SP_TierRoot)
	XT_REGISTER_TYPE(SP_TierData)

	// SinglePlayerAbilityLoadOut

	XT_REGISTER_TYPE(SinglePlayerAbilityLoadOut)

XT_END_LIBRARY
