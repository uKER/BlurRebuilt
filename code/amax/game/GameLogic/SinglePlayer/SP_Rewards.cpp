#include "StdAfx.h"
#include "SinglePlayerImpl.h"
#include <GameLogic/SinglePlayer/SP_BossDemands.h>
#include <Automation/GameDataLogging.h>

SP_Reward::SP_Reward(SP_RewardKind kind_) :
	kind(kind_)
{
}

SP_VehicleReward::SP_VehicleReward() :
	SP_Reward(kSP_VehicleReward)
{
}

SP_ModReward::SP_ModReward() :
	SP_Reward(kSP_ModReward)
{
}

const char* SP_GetUnlockName(SP_UnlockKind kind)
{
	switch(kind)
	{
		case kSP_UnlockTier:			return "tier";
		case kSP_UnlockBoss:			return "boss";
		case kSP_UnlockEvent:			return "event";
		case kSP_UnlockRank:			return "rank";
		case kSP_UnlockVehicle:			return "vehicle";
		case kSP_UnlockMod:				return "mod";
		case kSP_UnlockBossDefeated:	return "boss-defeated";
	}

	return "unknown";
}

template<typename Type>
bool SP_ImplSetUnlockedState(XtUt::Set<Type>* set, SP_UnlockList& unlocks, SP_UnlockKind unlockKind, XtU32 id, bool unlocked)
{
	bool didUnlock = false;

	if (Type* st = Acquire(set, Type(id)))
	{
		if (st->IsUnlocked() != unlocked)
		{
			if (unlocked)
				unlocks.PushBack(SP_Unlock(unlockKind, id));

			st->_unlocked	= unlocked ? kSP_NewlyUnlocked : kSP_Locked;
			didUnlock		= unlocked;
		}
	}

	return didUnlock;
}

void SP_ProcessUnlocks_DoReward(SP_SaveGame* sg, SP_UnlockList& unlocks, SP_Reward* reward, bool unlocked)
{
	if (!reward)
		return;

	// Mark the rewards as locked/unlocked

	switch(reward->kind)
	{
		case kSP_VehicleReward:	SP_ImplSetUnlockedState(&sg->_vehicles,	unlocks, kSP_UnlockVehicle,	((SP_VehicleReward*)reward)->vehicle.Id(),	unlocked); break;
		case kSP_ModReward:		SP_ImplSetUnlockedState(&sg->_mods,		unlocks, kSP_UnlockMod,		((SP_ModReward*)reward)->mod.Id(),			unlocked); break;
	}
}

void SP_ProcessUnlocks_DoBossDemands(SP_SaveGame* sg, SP_Tier* tier)
{
	SP_BossDemands::Instance().UpdateBossDemandsForTier(sg, tier);
}

void SP_ProcessUnlocks(SP_SaveGame* sg, SP_UnlockList& unlocks)
{
	if (!sg || !gSpRankData || !gSpTierData)
		return;

	// Calculate new rank and unlock any new rewards

	int newRankNo = SP_FindRankByFans(sg, sg->_totalFans);

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank*	rank		= &gSpRankData->ranks[rankNo];
		bool		unlocked	= rankNo <= newRankNo;

		// Add rank to unlock list

		if (unlocked && (sg->_rank < rankNo))
		{
			unlocks.PushBack(SP_Unlock(kSP_UnlockRank, rankNo));

			sg->_lastSessionRankedUp = 1;
		}

		// Sync vehicles and mods with new unlock state

		for(int i = 0; i < rank->vehicles.count; i++)
			SP_ImplSetUnlockedState(&sg->_vehicles, unlocks, kSP_UnlockVehicle, rank->vehicles[i].vehicle.Id(), unlocked);

		for(int i = 0; i < rank->mods.count; i++)
			SP_ImplSetUnlockedState(&sg->_mods, unlocks, kSP_UnlockMod, rank->mods[i].mod.Id(), unlocked);
	}

	sg->_rank = newRankNo;

	// Synchronise event unlocks for each tier

	int stars = 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	for(int tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		SP_Tier* tier = root->tiers[tierNo];

		if (!tier)
			continue;

		// If tier is locked all events are locked

		bool tierUnlocked = stars >= tier->starRequirement;

		if (SP_ImplSetUnlockedState(&sg->_tiers, unlocks, kSP_UnlockTier, tier->id, tierUnlocked))
		{
			sg->_lastSessionTier = tierNo + 1;
#ifndef CFG_GOLD
			AmaxDataLogging::LogTierUnlocked(tierNo + 1);
#endif
		}

		// Standard events

		int completeCount = 0;

		for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
		{
			if (SP_Event* event = tier->events[eventNo])
			{
				if (event->kind == kSP_BossEvent)
					continue;

				if (SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id))
				{
					// Decide if this event is unlocked

					int requiredCount = (eventNo / 3) * 2;

					bool eventUnlocked = tierUnlocked && (completeCount >= requiredCount);

					// Mark the event as locked/unlocked

					SP_ImplSetUnlockedState(&sg->_events, unlocks, kSP_UnlockEvent, event->id, eventUnlocked);

					// Count complete events

					if (se->IsComplete())
						completeCount++;

					// Count stars

					stars += SP_CountTotalEventStars(sg, event);
				}
			}
		}

		// Process any boss demands which are dependant on event completion

		SP_ProcessUnlocks_DoBossDemands(sg, tier);

		// The boss is unlocked if all their demands are met

		bool bossUnlocked = true;

		if (SP_Boss* boss = tier->boss)
		{
			for(int demandNo = 0; demandNo < boss->demands.count; demandNo++)
			{
				if (SP_BossDemand* demand = boss->demands[demandNo])
				{
					if (!SP_IsBossDemandComplete(sg, demand))
					{
						bossUnlocked = false;
						break;
					}
				}
			}

			// Mark the boss as locked/unlocked

			if (SP_ImplSetUnlockedState(&sg->_bosses, unlocks, kSP_UnlockBoss, boss->id, bossUnlocked))
			{
				sg->_lastSessionBossUnlocked = boss->id;
#ifndef CFG_GOLD
				AmaxDataLogging::LogBossUnlocked(tierNo + 1);
#endif
			}
		}
		else
			bossUnlocked = false;

		// Boss events

		bool rewardsUnlocked = bossUnlocked;

		for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
		{
			if (SP_Event* event = tier->events[eventNo])
			{
				if (event->kind != kSP_BossEvent)
					continue;

				if (SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id))
				{
					// Mark the boss event as locked/unlocked

					SP_ImplSetUnlockedState(&sg->_events, unlocks, kSP_UnlockEvent, event->id, bossUnlocked);

					// Rewards are only unlocked if all boss events are complete in this tier

					if (!se->IsComplete())
						rewardsUnlocked = false;

					// Count stars

					stars += SP_CountTotalEventStars(sg, event);
				}
			}
		}

		// Boss rewards

		if (SP_Boss* boss = tier->boss)
		{
			if (SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id))
			{
				if (sb->_defeated != (XtU8)rewardsUnlocked)
				{
					sb->_defeated = rewardsUnlocked;

					if (rewardsUnlocked)
						unlocks.PushBack(SP_Unlock(kSP_UnlockBossDefeated, boss->id));
				}
			}

			// Unlock rewards

			for(int rewardNo = 0; rewardNo < boss->rewardsForUnlock.count; rewardNo++)
				SP_ProcessUnlocks_DoReward(sg, unlocks, boss->rewardsForUnlock[rewardNo], bossUnlocked);

			// Completion rewards

			for(int rewardNo = 0; rewardNo < boss->rewardsForDefeat.count; rewardNo++)
				SP_ProcessUnlocks_DoReward(sg, unlocks, boss->rewardsForDefeat[rewardNo], rewardsUnlocked);
		}
	}
}

void SP_MarkNotNew(SP_SaveGame* sg, XtId id)
{
	if (!sg || !id)
		return;

	if (SP_SaveTier* tier = At(&sg->_tiers, SP_SaveTier(id)))
	{
		if (tier->IsNew())
			tier->_unlocked = kSP_Unlocked;
	}

	if (SP_SaveEvent* event = At(&sg->_events, SP_SaveEvent(id)))
	{
		if (event->IsNew())
			event->_unlocked = kSP_Unlocked;
	}

	if (SP_SaveVehicle* vehicle = At(&sg->_vehicles, SP_SaveVehicle(id)))
	{
		if (vehicle->IsNew())
			vehicle->_unlocked = kSP_Unlocked;
	}

	if (SP_SaveMod* mod = At(&sg->_mods, SP_SaveMod(id)))
	{
		if (mod->IsNew())
			mod->_unlocked = kSP_Unlocked;
	}
}
