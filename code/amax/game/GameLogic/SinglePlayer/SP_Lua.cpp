#include "StdAfx.h"
#include "SinglePlayerImpl.h"
#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <Ui/UiHelpers.h>
#include <GameLogic/Structures/VehicleScoringInfo.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

extern bool MP_IsLockedLegendCar(XtId vehicleId);

SP_SaveGame*	gSaveGame;
SP_UnlockList	gSpUnlocks;

int SP_GameInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	lua_newtable(lua);

	Lua::AddDataMember(lua, "rank",			sg->_rank);
	Lua::AddDataMember(lua, "rankProgress",	SP_GetRankProgress(sg));
	Lua::AddDataMember(lua, "fans",			(double)sg->_totalFans);
	Lua::AddDataMember(lua, "totalStars",	SP_CountTotalStars(sg));
	Lua::AddDataMember(lua, "maxStars",		SP_CountMaxStars(sg));
	Lua::AddDataMember(lua, "lastVehicle",	sg->_lastVehicle);

	Lua::AddDataMember(lua, "lastSessionTier",			sg->_lastSessionTier);
	Lua::AddDataMember(lua, "lastSessionBossUnlocked",	sg->_lastSessionBossUnlocked);
	Lua::AddDataMember(lua, "lastSessionBossDefeated",	sg->_lastSessionBossDefeated);
	Lua::AddDataMember(lua, "lastSessionRankedUp",		sg->_lastSessionRankedUp != false);

	return 1;
}

int SP_TierInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	lua_newtable(lua);

	for(XtI32 tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		SP_Tier* tier = root->tiers[tierNo];

		if (!tier)
			continue;

		SP_SaveTier* st = SP_GetSaveTier(sg, tier->id);

		if (!st)
			continue;

		lua_newtable(lua);

		int totalStars		= SP_CountTotalTierStars(sg, tier);
		int maxStars		= SP_CountMaxTierStars(sg, tier);

		Lua::AddDataMember(lua, "tier",				tier->index + 1);
		Lua::AddDataMember(lua, "tierId",			tier->id);
		Lua::AddDataMember(lua, "name",				tier->name.id);
		Lua::AddDataMember(lua, "unlocked",			st->IsUnlocked());
		Lua::AddDataMember(lua, "starRequirement",	tier->starRequirement);
		Lua::AddDataMember(lua, "totalStars",		totalStars);
		Lua::AddDataMember(lua, "maxStars",			maxStars);
		Lua::AddDataMember(lua, "availableStars",	maxStars - totalStars);
		Lua::AddDataMember(lua, "new",				st->IsNew());

		bool hasNewEvents		= false;
		int incompleteEvents	= 0;

		{
			lua_pushstring(lua, "events");
			lua_newtable(lua);

			for(int j = 0; j < tier->events.count; j++)
			{
				if (SP_Event* event = tier->events[j])
				{
					lua_pushinteger(lua, event->id);
					lua_rawseti(lua, -2, j + 1);

					if (SP_SaveEvent* se = At(&sg->_events, SP_SaveEvent(event->id)))
					{
						if (se->IsUnlocked() && !se->IsComplete())
							incompleteEvents++;

						if (se->IsNew())
							hasNewEvents = true;
					}
				}
			}

			lua_rawset(lua, -3);
		}

		Lua::AddDataMember(lua, "containsNewEvents",	hasNewEvents);
		Lua::AddDataMember(lua, "incompleteEvents",		incompleteEvents);

		if (tier->boss)
		{
			lua_pushstring(lua, "bossId");
			lua_pushinteger(lua, tier->boss->id);
			lua_rawset(lua, -3);
		}

		lua_rawseti(lua, -2, tierNo + 1);
	}

	return 1;
}

int SP_EventInfo(lua_State* lua, SP_SaveGame* sg, XtId eventId)
{
	if (!sg || !gSpTierData || !gSpIndexData)
		return 0;

	SP_Event* event = SP_GetEventById(sg, eventId);

	if (!event)
		return 0;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, eventId);

	if (!se)
		return 0;

	lua_newtable(lua);

	char const* kindText = "unknown";

	switch(event->kind)
	{
		case kSP_RaceEvent:			kindText = "race";			break;
		case kSP_FanEvent:			kindText = "fan";			break;
		case kSP_CheckpointEvent:	kindText = "checkpoint";	break;
		case kSP_DestructionEvent:	kindText = "destruction";	break;
		case kSP_BossEvent:			kindText = "boss";			break;
	}

	const char* diffText = "unknown";

	switch(se->Difficulty())
	{
		case Game::GAME_DIFFICULTY_VERY_EASY:	diffText = "very-easy";	break;
		case Game::GAME_DIFFICULTY_EASY:		diffText = "easy";		break;
		case Game::GAME_DIFFICULTY_NORMAL:		diffText = "normal";	break;
		case Game::GAME_DIFFICULTY_HARD:		diffText = "hard";		break;
		case Game::GAME_DIFFICULTY_HARDCORE:	diffText = "hardcore";	break;
	}

	Lua::AddDataMember(lua, "kind",				kindText);
	Lua::AddDataMember(lua, "event",			event->index + 1);
	Lua::AddDataMember(lua, "eventId",			event->event.Id());

	Lua::AddDataMember(lua, "unlocked",			se->IsUnlocked());
	Lua::AddDataMember(lua, "new",				se->IsNew());
	Lua::AddDataMember(lua, "state",			SP_GetEventStateName(se->State()));
	Lua::AddDataMember(lua, "difficulty",		diffText);
	Lua::AddDataMember(lua, "lastVehicle",		se->_lastVehicle);

	Lua::AddDataMember(lua, "totalStars",		SP_CountTotalEventStars(sg, event));
	Lua::AddDataMember(lua, "maxStars",			SP_CountMaxEventStars(sg, event));

	Lua::AddDataMember(lua, "fanParBest",		se->_bestFanPar);
	Lua::AddDataMember(lua, "fanParTarget",		event->fanPar);
	Lua::AddDataMember(lua, "fanParComplete",	SP_IsEventFanParComplete(sg, event));

	Lua::AddDataMember(lua, "pointsBest",		se->_bestPoints);
	Lua::AddDataMember(lua, "timeBest",			se->_bestTime);

	Lua::AddDataMember(lua, "fanRunComplete",	SP_IsEventFanRunComplete(sg, event));

	if (SP_Tier* tier = event->tier)
	{
		Lua::AddDataMember(lua, "tier",		event->tier->index + 1);
		Lua::AddDataMember(lua, "tierId",	event->tier->id);
	}

	if (SP_CarRestriction* cr = event->carRestriction)
	{
		switch(cr->kind)
		{
			case kSP_ClassCarRestriction:
				Lua::AddDataMember(lua, "restriction",	"class");
				Lua::AddDataMember(lua, "carClass",		cr->carClass);
			break;

			case kSP_SetCarsRestriction:
			{
				SP_SetCarsRestriction* scr = (SP_SetCarsRestriction*)cr;

				Lua::AddDataMember(lua, "restriction",	"set");
				Lua::AddDataMember(lua, "description",	scr->description.id);
				Lua::AddDataMember(lua, "carClass",		cr->carClass);

				lua_pushstring(lua, "carList");
				lua_newtable(lua);

				for(int j = 0; j < scr->vehicles.count; j++)
				{
					lua_pushinteger(lua, scr->vehicles[j].Id());
					lua_pushboolean(lua, true);
					lua_rawset(lua, -3);
				}

				lua_rawset(lua, -3);
			}
			break;
		}
	}
	else
		Lua::AddDataMember(lua, "restriction", "none");

	return 1;
}

int SP_VehicleInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpRankData || !gSpTierData || !gSpIndexData)
		return 0;

	lua_newtable(lua);

	for(XtUt::Set<SP_VehicleIndex>::Iterator it(&gSpIndexData->vehicles); it; ++it)
	{
		if (!MP_IsLockedLegendCar(it->id))
		{
			lua_pushinteger(lua, it->id);
			lua_newtable(lua);

			Lua::AddDataMember(lua, "id", it->id);

			if (SP_Rank* rank = SP_GetRank(sg, it->rewardRank))
			{
				Lua::AddDataMember(lua, "method",	"rank");
				Lua::AddDataMember(lua, "rank",		it->rewardRank);
				Lua::AddDataMember(lua, "order",	it->rewardOrder);
				Lua::AddDataMember(lua, "fans",		(double)rank->fans);
			}
			else
			{
				Lua::AddDataMember(lua, "method",	"boss");
				Lua::AddDataMember(lua, "bossId",	it->rewardBoss);
				Lua::AddDataMember(lua, "order",	it->rewardOrder);
			}

			Lua::AddDataMember(lua, "unlocked",	SP_IsVehicleUnlocked(sg, it->id));
			Lua::AddDataMember(lua, "new",		SP_IsVehicleNew(sg, it->id));

			lua_rawset(lua, -3);
		}
	}

	return 1;
}

int SP_ModInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpRankData || !gSpTierData || !gSpIndexData)
		return 0;

	lua_newtable(lua);

	for(XtUt::Set<SP_ModIndex>::Iterator it(&gSpIndexData->mods); it; ++it)
	{
		lua_pushinteger(lua, it->id);
		lua_newtable(lua);

		Lua::AddDataMember(lua, "id", it->id);

		if (SP_Rank* rank = SP_GetRank(sg, it->rewardRank))
		{
			Lua::AddDataMember(lua, "method",	"rank");
			Lua::AddDataMember(lua, "rank",		it->rewardRank);
			Lua::AddDataMember(lua, "order",	it->rewardOrder);
			Lua::AddDataMember(lua, "fans",		(double)rank->fans);
		}
		else
		{
			Lua::AddDataMember(lua, "method",	"boss");
			Lua::AddDataMember(lua, "bossId",	it->rewardBoss);
			Lua::AddDataMember(lua, "order",	it->rewardOrder);
		}

		Lua::AddDataMember(lua, "unlocked",	SP_IsModUnlocked(sg, it->id));
		Lua::AddDataMember(lua, "new",		SP_IsModNew(sg, it->id));

		lua_rawset(lua, -3);
	}

	return 1;
}

int SP_RankInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpRankData)
		return 0;

	lua_newtable(lua);

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		lua_newtable(lua);

		Lua::AddDataMember(lua, "index",	rankNo);
		Lua::AddDataMember(lua, "name",		rank->name.id);
		Lua::AddDataMember(lua, "fans",		(double)rank->fans);

		{
			lua_pushstring(lua, "vehicles");
			lua_newtable(lua);

			for(int j = 0; j < rank->vehicles.count; j++)
			{
				lua_pushinteger(lua, rank->vehicles[j].vehicle.Id());
				lua_pushboolean(lua, true);
				lua_rawset(lua, -3);
			}

			lua_rawset(lua, -3);
		}

		{
			lua_pushstring(lua, "mods");
			lua_newtable(lua);

			for(int j = 0; j < rank->mods.count; j++)
			{
				lua_pushinteger(lua, rank->mods[j].mod.Id());
				lua_pushboolean(lua, true);
				lua_rawset(lua, -3);
			}

			lua_rawset(lua, -3);
		}

		lua_rawseti(lua, -2, rankNo);
	}

	return 1;
}

int SP_BossInfo(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	lua_newtable(lua);

	for(int bossNo = 0; bossNo < gSpTierData->bosses.count; bossNo++)
	{
		SP_Boss* boss = &gSpTierData->bosses[bossNo];

		if (SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id))
		{
			lua_newtable(lua);

			Lua::AddDataMember(lua, "index",	bossNo + 1);
			Lua::AddDataMember(lua, "id",		boss->id);
			Lua::AddDataMember(lua, "name",		boss->name.id);
			Lua::AddDataMember(lua, "bio",		boss->bio.id);
			Lua::AddDataMember(lua, "unlocked",	sb->IsUnlocked());
			Lua::AddDataMember(lua, "defeated",	sb->IsDefeated());

			{
				lua_pushstring(lua, "demands");
				lua_newtable(lua);

				for(int j = 0; j < boss->demands.count; j++)
				{
					SP_BossDemand* bd = boss->demands[j];

					if (!bd)
						continue;

					lua_newtable(lua);

					Lua::AddDataMember(lua, "index",	j + 1);
					Lua::AddDataMember(lua, "kind",		SP_GetBossDemandName(bd->kind));
					Lua::AddDataMember(lua, "complete",	SP_IsBossDemandComplete(sg, bd));
					Lua::AddDataMember(lua, "progress",	SP_GetBossDemandPercentageComplete(sg, bd));

					if (SP_DoesBossDemandUseFloatValue(sg, bd))
					{
						Lua::AddDataMember(lua, "target",	SP_GetBossDemandTargetFloat(sg, bd));
						Lua::AddDataMember(lua, "value",	SP_GetBossDemandValueFloat(sg, bd));
					}
					else
					{
						Lua::AddDataMember(lua, "target",	SP_GetBossDemandTargetInt(sg, bd));
						Lua::AddDataMember(lua, "value",	SP_GetBossDemandValueInt(sg, bd));
					}

					lua_rawseti(lua, -2, j + 1);
				}

				lua_rawset(lua, -3);
			}

			for(int j = 0; j < boss->rewardsForDefeat.count; j++)
			{
				SP_Reward* reward = boss->rewardsForDefeat[j];

				if (!reward)
					continue;

				if(reward->kind == kSP_VehicleReward)
				{
					SP_VehicleReward* vehicleReward = (SP_VehicleReward*)reward;
					if(vehicleReward->vehicle)
					{
						AmaxVehicle* amaxVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)vehicleReward->vehicle->ID];
						if(amaxVehicle)
						{
							Lua::AddDataMember(lua, "vehicle_id", amaxVehicle->ID );
							Lua::AddDataMember(lua, "vehicle_ability", (S32)amaxVehicle->bossAbilitySlot );
							break;
						}
					}
				}
			}

			lua_rawseti(lua, -2, bossNo + 1);
		}
	}

	return 1;
}

int SP_UpcomingUnlocks(lua_State* lua, SP_SaveGame* sg)
{
	if (!sg || !gSpRankData || !gSpTierData || !gSpIndexData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	int totalStars = SP_CountTotalStars(sg);

	int unlockIndex = 1;

	lua_newtable(lua);

	for(XtI32 tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		SP_Tier* tier = root->tiers[tierNo];

		if (!tier)
			continue;

		SP_SaveTier* st = SP_GetSaveTier(sg, tier->id);

		if (!st)
			continue;

		// Boss unlock

		if (SP_Boss* boss = tier->boss)
		{
			if (SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id))
			{
				if (!sb->IsUnlocked())
				{
					float current	= 0.0f;
					float total		= 0.0f;

					for(int j = 0; j < boss->demands.count; j++)
					{
						SP_BossDemand* bd = boss->demands[j];

						if (!bd)
							continue;

						current	+= SP_GetBossDemandPercentageComplete(sg, bd);
						total	+= 1.0f;
					}

					if (current >= (total * 0.75f))
					{
						lua_pushinteger(lua, unlockIndex++);
						lua_newtable(lua);

						Lua::AddDataMember(lua, "kind",		"boss");
						Lua::AddDataMember(lua, "bossId",	boss->id);
						Lua::AddDataMember(lua, "tier",		tierNo + 1);
						Lua::AddDataMember(lua, "tierId",	tier->id);
						Lua::AddDataMember(lua, "score",	current / total);

						lua_rawset(lua, -3);
					}
				}
			}
		}

		// Tier unlock

		if (!st->IsUnlocked())
		{
			if (totalStars >= (tier->starRequirement * 0.75f))
			{
				lua_pushinteger(lua, unlockIndex++);
				lua_newtable(lua);

				Lua::AddDataMember(lua, "kind",				"tier");
				Lua::AddDataMember(lua, "tier",				tierNo + 1);
				Lua::AddDataMember(lua, "tierId",			tier->id);
				Lua::AddDataMember(lua, "score",			(float)totalStars / (float)tier->starRequirement);
				Lua::AddDataMember(lua, "lightsNeeded",		tier->starRequirement - totalStars);

				lua_rawset(lua, -3);
			}
		}
	}

	// Rank

	XtU64 totalFans = SP_GetFans(sg);

	for(int rankNo = SP_GetRank(sg) + 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		if (totalFans >= (rank->fans * 0.75f))
		{
			SP_Rank* lastRank = &gSpRankData->ranks[rankNo - 1];

			lua_pushinteger(lua, unlockIndex++);
			lua_newtable(lua);

			Lua::AddDataMember(lua, "kind",					"rank");
			Lua::AddDataMember(lua, "rank",					rankNo);
			Lua::AddDataMember(lua, "score",				(float)totalFans / (float)rank->fans);
			Lua::AddDataMember(lua, "current_fans",			(U32)totalFans);
			Lua::AddDataMember(lua, "next_rank_fans",		(U32)rank->fans);
			Lua::AddDataMember(lua, "previous_rank_fans",	(U32)lastRank->fans);

			lua_rawset(lua, -3);
		}
	}

	return 1;
}

void SP_ImplRewardUnlockInfo(lua_State* lua, SP_SaveGame* sg, int rewardRank, XtId rewardBoss)
{
	if (SP_Boss* boss = SP_GetBossById(sg, rewardBoss))
	{
		Lua::AddDataMember(lua, "method",	"boss");
		Lua::AddDataMember(lua, "bossId",	rewardBoss);

		if (SP_Tier* tier = boss->tier)
		{
			Lua::AddDataMember(lua, "tier",		tier->index + 1);
			Lua::AddDataMember(lua, "tierId",	tier->id);
		}
	}
	else
	{
		Lua::AddDataMember(lua, "method",	"rank");
		Lua::AddDataMember(lua, "rank",		rewardRank);
	}
}

int SP_GenerateUnlockTable(lua_State* lua, SP_SaveGame* sg, SP_UnlockList& unlocks)
{
	if (!sg || !gSpRankData || !gSpTierData || !gSpIndexData)
		return 0;

	lua_newtable(lua);

	for(int i = 0; i < unlocks.Size(); i++)
	{
		SP_Unlock* unlock = &unlocks[i];

		lua_newtable(lua);

		Lua::AddDataMember(lua, "kind", SP_GetUnlockName(unlock->kind));

		switch(unlock->kind)
		{
			case kSP_UnlockTier:
			{
				Lua::AddDataMember(lua, "tierId", unlock->id);

				if (SP_Tier* tier = SP_GetTierById(gSaveGame, unlock->id))
				{
					Lua::AddDataMember(lua, "tier", tier->index + 1);
				}
			}
			break;

			case kSP_UnlockBoss:
			{
				Lua::AddDataMember(lua, "bossId", unlock->id);

				if (SP_Boss* boss = SP_GetBossById(gSaveGame, unlock->id))
				{
					if (SP_Tier* tier = boss->tier)
					{
						Lua::AddDataMember(lua, "tier",		tier->index + 1);
						Lua::AddDataMember(lua, "tierId",	tier->id);
					}
				}
			}
			break;

			case kSP_UnlockEvent:
			{
				Lua::AddDataMember(lua, "eventId", unlock->id);

				if (SP_Event* event = SP_GetEventById(gSaveGame, unlock->id))
				{
					Lua::AddDataMember(lua, "event", event->index + 1);

					if (SP_Tier* tier = event->tier)
					{
						Lua::AddDataMember(lua, "tier",		tier->index + 1);
						Lua::AddDataMember(lua, "tierId",	tier->id);
					}

					Lua::AddDataMember(lua, "boss", event->kind == kSP_BossEvent);
				}
			}
			break;

			case kSP_UnlockRank:
				Lua::AddDataMember(lua, "rank", unlock->id);
			break;

			case kSP_UnlockVehicle:
				Lua::AddDataMember(lua, "vehicleId", unlock->id);

				if (SP_VehicleIndex* vi = At(&gSpIndexData->vehicles, SP_VehicleIndex(unlock->id)))
					SP_ImplRewardUnlockInfo(lua, sg, vi->rewardRank, vi->rewardBoss);
			break;

			case kSP_UnlockMod:
				Lua::AddDataMember(lua, "modId", unlock->id);

				if (SP_ModIndex* mi = At(&gSpIndexData->mods, SP_ModIndex(unlock->id)))
					SP_ImplRewardUnlockInfo(lua, sg, mi->rewardRank, mi->rewardBoss);
			break;

			case kSP_UnlockBossDefeated:
			{
				Lua::AddDataMember(lua, "bossId", unlock->id);

				if (SP_Boss* boss = SP_GetBossById(gSaveGame, unlock->id))
				{
					if (SP_Tier* tier = boss->tier)
					{
						Lua::AddDataMember(lua, "tier",		tier->index + 1);
						Lua::AddDataMember(lua, "tierId",	tier->id);
					}
				}
			}
			break;
		}

		lua_rawseti(lua, -2, i + 1);
	}

	return 1;
}

int SP_GenerateVehicleUnlockTable(lua_State* lua, SP_SaveGame* sg, SP_UnlockList& unlocks)
{
	if (!sg || !gSpRankData || !gSpTierData || !gSpIndexData)
		return 0;

	lua_pushstring( lua, "unlocks" );
	lua_newtable( lua );

	lua_pushstring( lua, "cars" );
	lua_newtable( lua );

	for(int i = 0, j = 1; i < unlocks.Size(); i++)
	{
		SP_Unlock* unlock = &unlocks[i];

		if (unlock->kind != kSP_UnlockVehicle)
			continue;

		if (MP_IsLockedLegendCar(unlock->id))
			continue;

		lua_newtable(lua);
		Lua::AddDataMember(lua, "id", unlock->id);
		lua_rawseti(lua, -2, j++);
	}

	lua_rawset( lua, -3 );

	return 1;
}


// [Blur2PC] BLUR2PC_COMPLETE_CAREER=1: whenever the career save is loaded or created, give
// every event gold with its fan targets met, fill every boss demand and raise fans to the top rank,
// then run the normal unlock pass (tiers, events, bosses, boss rewards, cars and mods follow from
// that state). The game writes it to the save the next time it saves.
static void Blur2PCCompleteCareerIfRequested()
{
	// applied on every load/create while the variable is set (a profile load replaces the career
	// created at boot); repeating it is harmless
	const char* e = getenv("BLUR2PC_COMPLETE_CAREER");
	if (!e || e[0] == 0 || e[0] == '0' || !gSaveGame || !gSpTierData || !gSpRankData)
		return;

	SP_SaveGame* sg = gSaveGame;
	SP_TierRoot* root = gSpTierData->tierRoot;
	int events = 0;
	for (XtI32 tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		SP_Tier* tier = root->tiers[tierNo];
		if (!tier)
			continue;
		for (int j = 0; j < tier->events.count; j++)
		{
			if (SP_Event* event = tier->events[j])
			{
				if (SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id))
				{
					se->_state			= kSP_Gold;
					se->_bestFanPar		= event->fanPar;
					se->_fanRunComplete	= 1;
					events++;
				}
			}
		}
		if (SP_Boss* boss = tier->boss)
		{
			for (int i = 0; i < boss->demands.count; i++)
			{
				if (SP_BossDemand* demand = boss->demands[i])
				{
					if (SP_DoesBossDemandUseFloatValue(sg, demand))
						SP_SetBossDemandValueFloat(sg, demand, SP_GetBossDemandTargetFloat(sg, demand));
					else
						SP_SetBossDemandValueInt(sg, demand, SP_GetBossDemandTargetInt(sg, demand));
				}
			}
		}
	}
	if (SP_Rank* rank = SP_GetRank(sg, SP_GetRankCount(sg) - 1))
		sg->_totalFans = rank->fans;

	// two passes: boss demands derived from event results settle on the first
	SP_UnlockList unlocks;
	SP_ProcessUnlocks(sg, unlocks);
	unlocks.Clear();
	SP_ProcessUnlocks(sg, unlocks);

	LOGINFO(gSpLog, "[Blur2PC] career completed: %d events gold, rank %d, %d/%d stars", events, sg->_rank, SP_CountTotalStars(sg), SP_CountMaxStars(sg));
}

//
// Interface
//

int SP_LuaNewSaveGame(lua_State* lua)
{
	delete gSaveGame;

	gSaveGame = SP_CreateSaveGame();

	Blur2PCCompleteCareerIfRequested();	// [Blur2PC]

	return 0;
}

int SP_LuaGameInfo(lua_State* lua)
{
	return SP_GameInfo(lua, gSaveGame);
}

int SP_LuaTierInfo(lua_State* lua)
{
	return SP_TierInfo(lua, gSaveGame);
}

int SP_LuaEventInfo(lua_State* lua)
{
	if (lua_type(lua, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:EventInfo(eventId) - incorrect arguments\n");
		return 0;
	}

	XtId eventId = lua_tointeger(lua, 1);

	return SP_EventInfo(lua, gSaveGame, eventId);
}

int SP_LuaVehicleInfo(lua_State* lua)
{
	return SP_VehicleInfo(lua, gSaveGame);
}

int SP_LuaModInfo(lua_State* lua)
{
	return SP_ModInfo(lua, gSaveGame);
}

int SP_LuaRankInfo(lua_State* lua)
{
	return SP_RankInfo(lua, gSaveGame);
}

int SP_LuaBossInfo(lua_State* lua)
{
	return SP_BossInfo(lua, gSaveGame);
}

int SP_LuaClearUnlocks(lua_State* lua)
{
	gSpUnlocks.Destroy();

	return 0;
}

int SP_LuaProcessUnlocks(lua_State* lua)
{
	SP_ProcessUnlocks(gSaveGame, gSpUnlocks);

	return SP_GenerateUnlockTable(lua, gSaveGame, gSpUnlocks);
}

int SP_LuaVehicleUnlocks(lua_State* lua)
{
	SP_ProcessUnlocks(gSaveGame, gSpUnlocks);

	return SP_GenerateVehicleUnlockTable(lua, gSaveGame, gSpUnlocks);
}

int SP_LuaEventSetLastVehicle(lua_State* lua)
{
	if ((lua_type(lua, 1) != LUA_TNUMBER) || (lua_type(lua, 2) != LUA_TNUMBER))
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:EventSetLastVehicle(eventId, vehicleId) - incorrect arguments\n");
		return 0;
	}

	XtId eventId = lua_tointeger(lua, 1);
	XtId vehicleId = lua_tointeger(lua, 2);

	SP_SetEventLastVehicle(gSaveGame, eventId, vehicleId);

	return 0;
}

int SP_LuaGameSetLastVehicle(lua_State* lua)
{
	if (lua_type(lua, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:GameSetLastVehicle(vehicleId) - incorrect arguments\n");
		return 0;
	}

	XtId vehicleId = lua_tointeger(lua, 1);

	SP_GameSetLastVehicle(gSaveGame, vehicleId);

	return 0;
}

int SP_LuaMarkNotNew(lua_State* lua)
{
	if (lua_type(lua, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:MarkNotNew(id) - incorrect arguments\n");
		return 0;
	}

	XtId id = lua_tointeger(lua, 1);

	SP_MarkNotNew(gSaveGame, id);

	return 0;
}

int SP_LuaUpcomingUnlocks(lua_State* lua)
{
	return SP_UpcomingUnlocks(lua, gSaveGame);
}

int SP_LuaClearLastSessionInfo(lua_State* lua)
{
	if (SP_SaveGame* sg = gSaveGame)
	{
		sg->_lastSessionTier			= 0;
		sg->_lastSessionBossUnlocked	= 0;
		sg->_lastSessionBossDefeated	= 0;
		sg->_lastSessionRankedUp		= 0;
	}

	return 0;
}

int SP_LuaCompleteEvent(lua_State* lua)
{
	SP_SaveGame* sg = gSaveGame;

	if (!sg)
		return 0;

	if (lua_type(lua, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:SP_LuaCompleteEvent(eventId) - incorrect arguments\n");
		return 0;
	}

	XtId eventId = lua_tointeger(lua, 1);

	if (SP_Event* event = SP_GetEventById(sg, eventId))
	{
		if (SP_SaveEvent* se = SP_GetSaveEvent(sg, eventId))
		{
			if (se->IsUnlocked())
			{
				if (const Game::C_GameProfile* profile = Game::C_GameProfileManager::Instance().GetPrimaryProfile())
				{
					const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

					// Always increase total fans

					sg->_totalFans += results.mRaceStatistics.mFansEarned;

					if (event->kind == kSP_FanEvent)
						sg->_totalFanEventFans += results.mRaceStatistics.mFansEarned;

					// Other awards only count when passing with at least bronze

					if (results.mRaceStatistics.mMedal >= kSP_Bronze)
					{
						// Medal

						if (results.mRaceStatistics.mMedal >= se->_state)
						{
							if ((results.mRaceStatistics.mMedal > se->_state) || (profile->mOptions.mGameDifficulty > se->_difficulty))
							{
								se->_state		= results.mRaceStatistics.mMedal;
								se->_difficulty	= profile->mOptions.mGameDifficulty;
							}
						}

						// Fan par

						if (results.mRaceStatistics.mFansEarned > se->_bestFanPar)
							se->_bestFanPar = results.mRaceStatistics.mFansEarned;

						// Gfd

						if (results.mRaceStatistics.mGoldenFanDemandComplete)
							se->_fanRunComplete = 1;

						// Per event mode records

						switch(event->kind)
						{
							case kSP_RaceEvent:
								if ((se->_bestTime <= 0.0f) || (se->_bestTime > results.mRaceStatistics.mTotalRaceTime))
									se->_bestTime = results.mRaceStatistics.mTotalRaceTime;
							break;

							case kSP_FanEvent:
								if (se->_bestPoints < results.mRaceStatistics.mDestruction_TotalCarsWrecked)
									se->_bestPoints = results.mRaceStatistics.mDestruction_TotalCarsWrecked; // TODO: Waiting for tony to fill this value out
							break;

							case kSP_BossEvent:	//deliberate fall through
							case kSP_CheckpointEvent:
								if (se->_bestTime < results.mRaceStatistics.mTotalRaceTime)
									se->_bestTime = results.mRaceStatistics.mTotalRaceTime;
							break;

							case kSP_DestructionEvent:
								if (se->_bestPoints < results.mRaceStatistics.mDestruction_NumPoints)
									se->_bestPoints = results.mRaceStatistics.mDestruction_NumPoints;
							break;
						}
					}
				}
			}
		}
	}

	return 0;
}

int SP_LuaAddToTotalFans(lua_State* lua)
{
	SP_SaveGame* sg = gSaveGame;

	if (!sg)
		return 0;

	if (Game::Actors::C_VehicleLogicActor* vehicle = UI::GetVehicleLogicActor(0))
	{
		VehicleScoringInfo& scoring = vehicle->GetScoring();

		sg->_totalFans += scoring.mFinalScore;
	}

	return 0;
}

//
// Lua debug functions
//

int SP_LuaDebugClear(lua_State* lua)
{
	delete gSaveGame;

	if ((gSaveGame = SP_CreateSaveGame()) != 0)
	{
		gSaveGame->_rank				= 0;
		gSaveGame->_totalFans			= 0;
		gSaveGame->_totalFanEventFans	= 0;
		gSaveGame->_lastVehicle			= 0;

		gSaveGame->_lastSessionTier			= 0;
		gSaveGame->_lastSessionBossUnlocked	= 0;
		gSaveGame->_lastSessionBossDefeated	= 0;
		gSaveGame->_lastSessionRankedUp		= 0;

		XtUt::Free(&gSaveGame->_tiers);
		XtUt::Free(&gSaveGame->_bosses);
		XtUt::Free(&gSaveGame->_events);
		XtUt::Free(&gSaveGame->_vehicles);
		XtUt::Free(&gSaveGame->_mods);
	}

	return 0;
}

int SP_LuaDebugCompleteEvent(lua_State* lua)
{
	SP_SaveGame* sg = gSaveGame;

	if (!sg)
		return 0;

	if (lua_type(lua, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "SinglePlayer:SP_LuaDebugCompleteEvent(eventId) - incorrect arguments\n");
		return 0;
	}

	XtId eventId = lua_tointeger(lua, 1);

	if (SP_Event* event = SP_GetEventById(sg, eventId))
	{
		if (SP_SaveEvent* se = SP_GetSaveEvent(sg, eventId))
		{
			if (se->IsUnlocked())
			{
				se->_state			= kSP_Gold;
				se->_bestFanPar		= event->fanPar;
				se->_fanRunComplete	= 1;
			}
			else
			{
				if (event->kind == kSP_BossEvent)
				{
					if (SP_Tier* tier = event->tier)
					{
						if (SP_Boss* boss = tier->boss)
						{
							for(int i = 0; i < boss->demands.count; i++)
							{
								if (SP_BossDemand* demand = boss->demands[i])
								{
									if (SP_DoesBossDemandUseFloatValue(sg, demand))
									{
										SP_SetBossDemandValueFloat(sg, demand, SP_GetBossDemandTargetFloat(sg, demand));
									}
									else
									{
										SP_SetBossDemandValueInt(sg, demand, SP_GetBossDemandTargetInt(sg, demand));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

int SP_LuaDebugRankUp(lua_State* lua)
{
	SP_SaveGame* sg = gSaveGame;

	if (!sg)
		return 0;

	if (SP_Rank* rank = SP_GetRank(sg, sg->_rank + 1))
	{
		DebugLn("Ranking fans to %I64i (rank %i)", rank->fans, sg->_rank + 1);

		sg->_totalFans = rank->fans;
	}

	return 0;
}

int SP_LuaFanStatusForTooltip(lua_State* lua)
{
	SP_SaveGame* sg = gSaveGame;

	XtU64 totalFans = SP_GetFans(sg);

	S32 rankNo = SP_GetRank(sg);

	if((rankNo+1) < gSpRankData->ranks.count)
	{

		SP_Rank* rank = &gSpRankData->ranks[(rankNo +1)];

		if (totalFans < (rank->fans * 0.33f) )
		{
			lua_pushinteger(lua, 1);
		}
		else if (totalFans >= (rank->fans * 0.33f) && totalFans < (rank->fans * 0.66f))
		{
			lua_pushinteger(lua, 2);
		}
		else
		{
			lua_pushinteger(lua, 3);
		}	

		lua_pushinteger(lua, rankNo);

		return 2;
	}
	else
	{
		return 0;
	}
}

namespace UI
{
	LUALIB_API int luaopen_sp(lua_State* lua)
	{
		static const luaL_reg lib[] = 
		{
			{ "NewSaveGame",			SP_LuaNewSaveGame			},
			{ "GameInfo",				SP_LuaGameInfo				},
			{ "TierInfo",				SP_LuaTierInfo				},
			{ "EventInfo",				SP_LuaEventInfo				},
			{ "VehicleInfo",			SP_LuaVehicleInfo			},
			{ "ModInfo",				SP_LuaModInfo				},
			{ "RankInfo",				SP_LuaRankInfo				},
			{ "BossInfo",				SP_LuaBossInfo				},
			{ "ClearUnlocks",			SP_LuaClearUnlocks			},
			{ "ProcessUnlocks",			SP_LuaProcessUnlocks		},
			{ "VehicleUnlocks",			SP_LuaVehicleUnlocks		},
			{ "EventSetLastVehicle",	SP_LuaEventSetLastVehicle	},
			{ "GameSetLastVehicle",		SP_LuaGameSetLastVehicle	},
			{ "MarkNotNew",				SP_LuaMarkNotNew			},
			{ "UpcomingUnlocks",		SP_LuaUpcomingUnlocks		},
			{ "ClearLastSessionInfo",	SP_LuaClearLastSessionInfo	},
			{ "CompleteEvent",			SP_LuaCompleteEvent			},
			{ "AddToTotalFans",			SP_LuaAddToTotalFans		},
			{ "DebugClear",				SP_LuaDebugClear			},
			{ "DebugCompleteEvent",		SP_LuaDebugCompleteEvent	},
			{ "DebugRankUp",			SP_LuaDebugRankUp			},
			{ "FanStatusForTooltip",	SP_LuaFanStatusForTooltip	},
			{ 0, 0 }
		};

		luaL_openlib(lua, "SinglePlayer", lib, 0);

		return 1;
	}
}

void SP_ExportSaveGameLua(Sys::BinarySerialiserAutoEndian& s)
{
	SP_ExportSaveGame(gSaveGame, s);
}

void SP_ImportSaveGameLua(Sys::BinaryDeserialiserAutoEndian& s)
{
	SP_ImportSaveGame(gSaveGame, s);
	Blur2PCCompleteCareerIfRequested();	// [Blur2PC]
}

SP_SaveGame* SP_GetCurrentSaveGame()
{
	return gSaveGame;
}

//
// Cheats
//

void SinglePlayer_RankUp(void*)
{
	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		if (SP_Rank* rank = SP_GetRank(sg, sg->_rank + 1))
		{
			DebugLn("Ranking fans to %I64i (rank %i)", rank->fans, sg->_rank + 1);

			sg->_totalFans = rank->fans;

			SP_ProcessUnlocks(sg, gSpUnlocks);
		}
	}
}

void SinglePlayer_UnlockAllCars(void*)
{
	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		if (SP_Rank* rank = SP_GetRank(sg, SP_GetRankCount(sg) - 1))
		{
			DebugLn("Ranking fans to %I64i (rank %i)", rank->fans, sg->_rank + 1);

			sg->_totalFans = rank->fans;

			SP_ProcessUnlocks(sg, gSpUnlocks);
		}
	}
}

void SinglePlayer_CompleteAllTiers(void*)
{
	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		SP_TierRoot* root = gSpTierData->tierRoot;

		for(XtI32 tierNo = 0; tierNo < root->tiers.count; tierNo++)
		{
			if (SP_Tier* tier = root->tiers[tierNo])
			{
				for(int j = 0; j < tier->events.count; j++)
				{
					if (SP_Event* event = tier->events[j])
					{
						if (SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id))
						{
							se->_state			= kSP_Gold;
							se->_bestFanPar		= event->fanPar;
							se->_fanRunComplete	= 1;
						}
					}
				}

				if (SP_Boss* boss = tier->boss)
				{
					for(int i = 0; i < boss->demands.count; i++)
					{
						if (SP_BossDemand* demand = boss->demands[i])
						{
							if (SP_DoesBossDemandUseFloatValue(sg, demand))
							{
								SP_SetBossDemandValueFloat(sg, demand, SP_GetBossDemandTargetFloat(sg, demand));
							}
							else
							{
								SP_SetBossDemandValueInt(sg, demand, SP_GetBossDemandTargetInt(sg, demand));
							}
						}
					}
				}
			}
		}

		SP_ProcessUnlocks(sg, gSpUnlocks);
	}
}

Debug::Function gSinglePlayer_RankUp("SinglePlayer.RankUp", SinglePlayer_RankUp, 0);
Debug::Function gSinglePlayer_UnlockAllCars("SinglePlayer.UnlockAllCars", SinglePlayer_UnlockAllCars, 0);
Debug::Function gSinglePlayer_CompleteAllTiers("SinglePlayer.CompleteAllTiers", SinglePlayer_CompleteAllTiers, 0);
