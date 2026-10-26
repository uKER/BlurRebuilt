#include "StdAfx.h"
#include "SinglePlayerImpl.h"
#include <GameLogic/GameData/GameEventData.h>

// Car Restrictions

SP_CarRestriction::SP_CarRestriction(SP_CarRestrictionKind kind_) :
	kind(kind_)
{
}

SP_ClassCarRestriction::SP_ClassCarRestriction() :
	SP_CarRestriction(kSP_ClassCarRestriction)
{
}

SP_SetCarsRestriction::SP_SetCarsRestriction() :
	SP_CarRestriction(kSP_SetCarsRestriction)
{
}

// Events

SP_Event::SP_Event(SP_EventKind kind_) :
	kind(kind_)
{
}

SP_RaceEvent::SP_RaceEvent() :					SP_Event(kSP_RaceEvent) { }
SP_FanEvent::SP_FanEvent() :					SP_Event(kSP_FanEvent) { }
SP_CheckpointEvent::SP_CheckpointEvent() :		SP_Event(kSP_CheckpointEvent) { }
SP_DestructionEvent::SP_DestructionEvent() :	SP_Event(kSP_DestructionEvent) { }
SP_BossEvent::SP_BossEvent() :					SP_Event(kSP_BossEvent) { }

// Helpers

const char* SP_GetEventStateName(SP_EventState state)
{
	switch(state)
	{
		case kSP_Incomplete:	return "none";
		case kSP_Bronze:		return "bronze";
		case kSP_Silver:		return "silver";
		case kSP_Gold:			return "gold";
	}

	return "none";
}
	
SP_Event* SP_GetEvent(SP_SaveGame* sg, SP_Tier* tier, int eventNo)
{
	if (!sg || !tier)
		return 0;

	if ((eventNo < 0) || (eventNo >= tier->events.count))
		return 0;

	return tier->events[eventNo];
}

SP_Event* SP_GetEvent(SP_SaveGame* sg, int tierNo, int eventNo)
{
	if (SP_Tier* tier = SP_GetTier(sg, tierNo))
		return SP_GetEvent(sg, tier, eventNo);

	return 0;
}

SP_Event* SP_GetEventById(SP_SaveGame* sg, XtId eventId)
{
	if (!sg || !gSpTierData || !gSpIndexData)
		return 0;

	SP_EventIndex* ei = At(&gSpIndexData->events, SP_EventIndex(eventId));

	if (!ei)
		return 0;

	return ei->event;
}

SP_Event* SP_GetEventByAmaxEventId(SP_SaveGame* sg, XtId amaxEventId)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	for(int tierNo = 0; tierNo < root->tiers.count; tierNo++)
	{
		if (SP_Tier* tier = root->tiers[tierNo])
		{
			for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
			{
				if (SP_Event* event = tier->events[eventNo])
				{
					if (event->event.Id() == amaxEventId)
						return event;
				}
			}
		}
	}

	return 0;
}

int SP_CountTotalEventStars(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !gSpTierData || !event)
		return 0;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return 0;

	int count = 0;

	if (event->kind == kSP_BossEvent)
	{
		if (se->State() == kSP_Gold)
			count = 8;
	}
	else
	{
		switch(se->State())
		{
			case kSP_Bronze:	count += 3; break;
			case kSP_Silver:	count += 4; break;
			case kSP_Gold:		count += 5; break;
		}

		if (se->_bestFanPar >= event->fanPar)
			count++;

		if (se->_fanRunComplete)
			count++;
	}

	return count;
}

int SP_CountMaxEventStars(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !gSpTierData || !event)
		return 0;

	if (event->kind == kSP_BossEvent)
		return 8;

	return 7;
}

int SP_CountMaxPrimaryEventStars(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !gSpTierData || !event)
		return 0;

	if (event->kind == kSP_BossEvent)
		return 8;

	return 5;
}

XtId SP_GetEventLastVehicle(SP_SaveGame* sg, XtId eventId)
{
	if (!sg)
		return 0;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, eventId);

	if (!se)
		return 0;

	return se->_lastVehicle;
}

void SP_SetEventLastVehicle(SP_SaveGame* sg, XtId eventId, XtId vehicleId)
{
	if (!sg)
		return;

	SP_SaveEvent* se = At(&sg->_events, SP_SaveEvent(eventId));

	if (!se)
		return;

	se->_lastVehicle = vehicleId;
}

SP_EventState SP_GetEventState(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return kSP_Incomplete;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return kSP_Incomplete;

	return se->State();
}

Game::GAME_DIFFICULTY SP_GetEventDifficulty(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return Game::GAME_DIFFICULTY_EASY;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return Game::GAME_DIFFICULTY_EASY;

	return se->Difficulty();
}

bool SP_IsEventFanParComplete(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return false;

	if (event->kind == kSP_BossEvent)
		return false;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return false;

	return se->_bestFanPar >= event->fanPar;
}

bool SP_IsEventFanRunComplete(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return false;

	if (event->kind == kSP_BossEvent)
		return false;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return false;

	return se->_fanRunComplete != 0;
}

int SP_GetEventBestFans(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return 0;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return 0;

	return se->_bestFanPar;
}

int SP_GetEventBestPoints(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return 0;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return 0;

	return se->_bestPoints;
}

float SP_GetEventBestTime(SP_SaveGame* sg, SP_Event* event)
{
	if (!sg || !event)
		return 0.0f;

	SP_SaveEvent* se = SP_GetSaveEvent(sg, event->id);

	if (!se || !se->IsUnlocked())
		return 0.0f;

	return se->_bestTime;
}
