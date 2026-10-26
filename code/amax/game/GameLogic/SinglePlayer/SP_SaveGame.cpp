#include "StdAfx.h"
#include "SinglePlayerImpl.h"

SP_SaveTier::SP_SaveTier(XtId id) :
	_id(id),
	_unlocked(0)
{
}

SP_SaveBoss::SP_SaveBoss(XtId id) :
	_id(id),
	_unlocked(0),
	_defeated(0)
{
	for(int i = 0; i < kSP_MaxBossDemands; i++)
		_demands[i] = 0;
}

SP_SaveEvent::SP_SaveEvent(XtId id) :
	_id(id),
	_bestFanPar(0),
	_bestPoints(0),
	_bestTime(0.0f),
	_lastVehicle(0),
	_unlocked(0),
	_state(kSP_Incomplete),
	_difficulty(Game::GAME_DIFFICULTY_VERY_EASY),
	_fanRunComplete(0)
{
}

SP_SaveVehicle::SP_SaveVehicle(XtId id) :
	_id(id),
	_unlocked(0)
{
}

SP_SaveMod::SP_SaveMod(XtId id) :
	_id(id),
	_unlocked(0)
{
}

SP_SaveGame::SP_SaveGame() :
	_rank(0),
	_totalFans(0),
	_totalFanEventFans(0),
	_lastVehicle(0),
	_lastSessionTier(0),
	_lastSessionBossUnlocked(0),
	_lastSessionBossDefeated(0),
	_lastSessionRankedUp(0)
{
}

namespace XtUt
{
	template<typename Type, typename Pred>
	Type* Acquire(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		size_t index = FirstOf(set, value);

		if ((index >= set->elements.count) || set->predicate(value, set->elements[index]))
			return Insert(set, value);

		return &set->elements[index];
	}
}

SP_SaveTier*	SP_GetSaveTier(SP_SaveGame* sg, XtId id)	{ return sg ? Acquire(&sg->_tiers,		SP_SaveTier(id)) : 0;		}
SP_SaveBoss*	SP_GetSaveBoss(SP_SaveGame* sg, XtId id)	{ return sg ? Acquire(&sg->_bosses,		SP_SaveBoss(id)) : 0;		}
SP_SaveEvent*	SP_GetSaveEvent(SP_SaveGame* sg, XtId id)	{ return sg ? Acquire(&sg->_events,		SP_SaveEvent(id)) : 0;		}
SP_SaveVehicle*	SP_GetSaveVehicle(SP_SaveGame* sg, XtId id)	{ return sg ? Acquire(&sg->_vehicles,	SP_SaveVehicle(id)) : 0;	}
SP_SaveMod*		SP_GetSaveMod(SP_SaveGame* sg, XtId id)		{ return sg ? Acquire(&sg->_mods,		SP_SaveMod(id)) : 0;		}

template<typename Type>
void SP_ImplAddToSaveMap(XtUt::Set<Type>* set, const Type& data, const char* typeName)
{
	if (XtUt::Exists(set, data))
	{
		LOGWARN(gSpLog, "SP: Duplicate %s id %08X", typeName, data._id);
	}
	else
	{
		XtUt::Insert(set, data);
	}
}

SP_SaveGame* SP_CreateSaveGame()
{
	if (!gSpRankData || !gSpTierData)
		return 0;

	SP_SaveGame* sg = new SP_SaveGame;

	if (!sg)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	// Create id<->tier map and count total events

	int eventCount = 0;

	XtUt::Reserve(&sg->_tiers.elements, root->tiers.count);
	XtUt::Reserve(&sg->_bosses.elements, root->tiers.count);

	for(int tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		if (SP_Tier* tier = root->tiers[tierNo])
		{
			eventCount += tier->events.count;

			SP_ImplAddToSaveMap(&sg->_tiers, SP_SaveTier(tier->id), "tier");

			if (SP_Boss* boss = tier->boss)
			{
				SP_ImplAddToSaveMap(&sg->_bosses, SP_SaveBoss(boss->id), "boss");
			}
		}
	}

	// Create id<->vehicle and id<->mod map

	int vehicleCount	= 0;
	int modCount		= 0;

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		vehicleCount	+= rank->vehicles.count;
		modCount		+= rank->mods.count;
	}

	XtUt::Reserve(&sg->_vehicles.elements, vehicleCount);
	XtUt::Reserve(&sg->_mods.elements, modCount);

	for(int rankNo = 1; rankNo < gSpRankData->ranks.count; rankNo++)
	{
		SP_Rank* rank = &gSpRankData->ranks[rankNo];

		for(int i = 0; i < rank->vehicles.count; i++)
			SP_ImplAddToSaveMap(&sg->_vehicles, SP_SaveVehicle(rank->vehicles[i].vehicle.Id()), "vehicle reward");

		for(int i = 0; i < rank->mods.count; i++)
			SP_ImplAddToSaveMap(&sg->_mods, SP_SaveMod(rank->mods[i].mod.Id()), "mod reward");
	}

	for(int tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		if (SP_Tier* tier = root->tiers[tierNo])
		{
			if (SP_Boss* boss = tier->boss)
			{
				for(int rewardNo = 0; rewardNo < boss->rewardsForUnlock.count; rewardNo++)
				{
					if (SP_Reward* reward = boss->rewardsForUnlock[rewardNo])
					{
						switch(reward->kind)
						{
							case kSP_VehicleReward:	SP_ImplAddToSaveMap(&sg->_vehicles,	SP_SaveVehicle(((SP_VehicleReward*)reward)->vehicle.Id()), "vehicle reward");	break;
							case kSP_ModReward:		SP_ImplAddToSaveMap(&sg->_mods,		SP_SaveMod(((SP_ModReward*)reward)->mod.Id()), "mod reward");					break;
						}
					}
				}

				for(int rewardNo = 0; rewardNo < boss->rewardsForDefeat.count; rewardNo++)
				{
					if (SP_Reward* reward = boss->rewardsForDefeat[rewardNo])
					{
						switch(reward->kind)
						{
							case kSP_VehicleReward:	SP_ImplAddToSaveMap(&sg->_vehicles,	SP_SaveVehicle(((SP_VehicleReward*)reward)->vehicle.Id()), "vehicle reward");	break;
							case kSP_ModReward:		SP_ImplAddToSaveMap(&sg->_mods,		SP_SaveMod(((SP_ModReward*)reward)->mod.Id()), "mod reward");					break;
						}
					}
				}
			}
		}
	}

	// Create id<->event map and add additional mod and vehicle rewards into relevant maps

	XtUt::Reserve(&sg->_events.elements, eventCount);

	for(int tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		if (SP_Tier* tier = root->tiers[tierNo])
		{
			for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
			{
				if (SP_Event* event = tier->events[eventNo])
					SP_ImplAddToSaveMap(&sg->_events, SP_SaveEvent(event->id), "event");
			}
		}
	}

	return sg;
}

void SP_ExportSaveGame(SP_SaveGame* sg, Sys::BinarySerialiserAutoEndian& s)
{
	if (!sg)
		return;

	XtU32 tierCount		= Count(&sg->_tiers);
	XtU32 bossCount		= Count(&sg->_bosses);
	XtU32 eventCount	= Count(&sg->_events);
	XtU32 vehicleCount	= Count(&sg->_vehicles);
	XtU32 modCount		= Count(&sg->_mods);

	s << sg->_rank;
	s << sg->_totalFans;
	s << sg->_totalFanEventFans;
	s << sg->_lastVehicle;
	s << sg->_lastSessionTier;
	s << sg->_lastSessionBossUnlocked;
	s << sg->_lastSessionBossDefeated;
	s << sg->_lastSessionRankedUp;

	s << tierCount;
	s << bossCount;
	s << eventCount;
	s << vehicleCount;
	s << modCount;

	s.Write(sg->_tiers.elements.elements, sizeof(SP_SaveTier) * tierCount);
	s.Write(sg->_bosses.elements.elements, sizeof(SP_SaveBoss) * bossCount);
	s.Write(sg->_events.elements.elements, sizeof(SP_SaveEvent) * eventCount);
	s.Write(sg->_vehicles.elements.elements, sizeof(SP_SaveVehicle) * vehicleCount);
	s.Write(sg->_mods.elements.elements, sizeof(SP_SaveMod) * modCount);
}

void SP_ImportSaveGame(SP_SaveGame* sg, Sys::BinaryDeserialiserAutoEndian& s)
{
	if (!sg)
		return;

	// Clear old saved game

	sg->_rank						= 0;
	sg->_totalFans					= 0;
	sg->_totalFanEventFans			= 0;
	sg->_lastVehicle				= 0;
	sg->_lastSessionTier			= 0;
	sg->_lastSessionBossUnlocked	= 0;
	sg->_lastSessionBossDefeated	= 0;
	sg->_lastSessionRankedUp		= 0;

	XtUt::Free(&sg->_tiers);
	XtUt::Free(&sg->_bosses);
	XtUt::Free(&sg->_events);
	XtUt::Free(&sg->_vehicles);
	XtUt::Free(&sg->_mods);

	XtU32 tierCount		= 0;
	XtU32 bossCount		= 0;
	XtU32 eventCount	= 0;
	XtU32 vehicleCount	= 0;
	XtU32 modCount		= 0;

	// Import new save game

	s >> sg->_rank;
	s >> sg->_totalFans;
	s >> sg->_totalFanEventFans;
	s >> sg->_lastVehicle;
	s >> sg->_lastSessionTier;
	s >> sg->_lastSessionBossUnlocked;
	s >> sg->_lastSessionBossDefeated;
	s >> sg->_lastSessionRankedUp;

	s >> tierCount;
	s >> bossCount;
	s >> eventCount;
	s >> vehicleCount;
	s >> modCount;

	SP_SaveTier*	tiers		= XtUt::AppendExN(&sg->_tiers.elements, tierCount);
	SP_SaveBoss*	bosses		= XtUt::AppendExN(&sg->_bosses.elements, bossCount);
	SP_SaveEvent*	events		= XtUt::AppendExN(&sg->_events.elements, eventCount);
	SP_SaveVehicle*	vehicles	= XtUt::AppendExN(&sg->_vehicles.elements, vehicleCount);
	SP_SaveMod*		mods		= XtUt::AppendExN(&sg->_mods.elements, modCount);

	if (
			((tierCount == 0) || tiers) &&
			((bossCount == 0) || bosses) &&
			((eventCount == 0) || events) &&
			((vehicleCount == 0) || vehicles) &&
			((modCount == 0) || mods)
		)
	{
		s.Read(tiers, sizeof(SP_SaveTier) * tierCount);
		s.Read(bosses, sizeof(SP_SaveBoss) * bossCount);
		s.Read(events, sizeof(SP_SaveEvent) * eventCount);
		s.Read(vehicles, sizeof(SP_SaveVehicle) * vehicleCount);
		s.Read(mods, sizeof(SP_SaveMod) * modCount);
	}
}