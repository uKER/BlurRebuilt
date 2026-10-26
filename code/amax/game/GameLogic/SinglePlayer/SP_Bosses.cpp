#include "StdAfx.h"
#include "SinglePlayerImpl.h"

// Bosses

SP_Boss* SP_GetBossById(SP_SaveGame* sg, XtId bossId)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	for(XtI32 i = 0; i < root->tiers.count; i++)
	{
		if (SP_Tier* tier = root->tiers[i])
		{
			if (SP_Boss* boss = tier->boss)
			{
				if (boss->id == bossId)
					return boss;
			}
		}
	}

	return 0;
}

bool SP_IsBossDefeated(SP_SaveGame* sg, SP_Boss* boss)
{
	if (!sg || !gSpTierData || !boss)
		return false;

	if (SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id))
		return sb->IsDefeated();

	return false;
}

bool SP_IsBossUnlocked(SP_SaveGame* sg, SP_Boss* boss)
{
	if (!sg || !gSpTierData || !boss)
		return false;

	if (SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id))
		return sb->IsUnlocked();

	return false;
}

// Boss demands

SP_BossDemand::SP_BossDemand(SP_BossDemandKind kind_) :				kind(kind_) { }
SP_BossDemand_Count::SP_BossDemand_Count(SP_BossDemandKind kind_) :	SP_BossDemand(kind_) { }

SP_BossDemand_Tier_Use_Powerups::SP_BossDemand_Tier_Use_Powerups() :						SP_BossDemand_Count(kSP_BossDemand_Tier_Use_Powerups) { }
SP_BossDemand_Tier_Earn_Fans::SP_BossDemand_Tier_Earn_Fans() :								SP_BossDemand_Count(kSP_BossDemand_Tier_Earn_Fans) { }
SP_BossDemand_Tier_Finish_In_Different_Car::SP_BossDemand_Tier_Finish_In_Different_Car() :	SP_BossDemand_Count(kSP_BossDemand_Tier_Finish_In_Different_Car) { }
SP_BossDemand_Tier_Finish_Events::SP_BossDemand_Tier_Finish_Events() :						SP_BossDemand_Count(kSP_BossDemand_Tier_Finish_Events) { }
SP_BossDemand_Tier_Shield_Powerups::SP_BossDemand_Tier_Shield_Powerups() :					SP_BossDemand_Count(kSP_BossDemand_Tier_Shield_Powerups) { }
SP_BossDemand_Tier_Shock_Opponents::SP_BossDemand_Tier_Shock_Opponents() :					SP_BossDemand_Count(kSP_BossDemand_Tier_Shock_Opponents) { }
SP_BossDemand_Tier_Repair_Car::SP_BossDemand_Tier_Repair_Car() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Repair_Car) { }
SP_BossDemand_Tier_Seconds_In_First::SP_BossDemand_Tier_Seconds_In_First() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Seconds_In_First) { }
SP_BossDemand_Event_Time_Remaining::SP_BossDemand_Event_Time_Remaining() :					SP_BossDemand_Count(kSP_BossDemand_Event_Time_Remaining) { }
SP_BossDemand_Tier_Collect_Stopwatches::SP_BossDemand_Tier_Collect_Stopwatches() :			SP_BossDemand_Count(kSP_BossDemand_Tier_Collect_Stopwatches) { }
SP_BossDemand_Tier_Complete_FanDemands::SP_BossDemand_Tier_Complete_FanDemands() :			SP_BossDemand_Count(kSP_BossDemand_Tier_Complete_FanDemands) { }
SP_BossDemand_Tier_Complete_FanRuns::SP_BossDemand_Tier_Complete_FanRuns() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Complete_FanRuns) { }
SP_BossDemand_Tier_Complete_FanPars::SP_BossDemand_Tier_Complete_FanPars() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Complete_FanPars) { }
SP_BossDemand_Event_Get_5X_Combo::SP_BossDemand_Event_Get_5X_Combo() :						SP_BossDemand_Count(kSP_BossDemand_Event_Get_5X_Combo) { }
SP_BossDemand_Tier_Ram_Into_Sea::SP_BossDemand_Tier_Ram_Into_Sea() :						SP_BossDemand_Count(kSP_BossDemand_Tier_Ram_Into_Sea) { }
SP_BossDemand_Tier_Wreck::SP_BossDemand_Tier_Wreck() :										SP_BossDemand_Count(kSP_BossDemand_Tier_Wreck) { }
SP_BossDemand_Tier_Barge_Hits::SP_BossDemand_Tier_Barge_Hits() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Barge_Hits) { }
SP_BossDemand_Event_Get_All_Lights::SP_BossDemand_Event_Get_All_Lights() :					SP_BossDemand_Count(kSP_BossDemand_Event_Get_All_Lights) { }
SP_BossDemand_Tier_Barge_Off_Cliff::SP_BossDemand_Tier_Barge_Off_Cliff() :					SP_BossDemand_Count(kSP_BossDemand_Tier_Barge_Off_Cliff) { }
SP_BossDemand_Tier_Shunt_Hits::SP_BossDemand_Tier_Shunt_Hits() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Shunt_Hits) { }
SP_BossDemand_Tier_Forward_Mine_Hits::SP_BossDemand_Tier_Forward_Mine_Hits() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Forward_Mine_Hits) { }
SP_BossDemand_Event_Complete_FanRun::SP_BossDemand_Event_Complete_FanRun() :				SP_BossDemand_Count(kSP_BossDemand_Event_Complete_FanRun) { }
SP_BossDemand_Tier_Destruction_Bonus_Hits::SP_BossDemand_Tier_Destruction_Bonus_Hits() :	SP_BossDemand_Count(kSP_BossDemand_Tier_Destruction_Bonus_Hits) { }
SP_BossDemand_Tier_Drift::SP_BossDemand_Tier_Drift() :										SP_BossDemand_Count(kSP_BossDemand_Tier_Drift) { }
SP_BossDemand_Tier_Use_Weapon_Defence::SP_BossDemand_Tier_Use_Weapon_Defence() :			SP_BossDemand_Count(kSP_BossDemand_Tier_Use_Weapon_Defence) { }
SP_BossDemand_Tier_Clean_Nitro::SP_BossDemand_Tier_Clean_Nitro() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Clean_Nitro) { }
SP_BossDemand_Tier_Clean_Triple_Nitros::SP_BossDemand_Tier_Clean_Triple_Nitros() :			SP_BossDemand_Count(kSP_BossDemand_Tier_Clean_Triple_Nitros) { }
SP_BossDemand_Tier_Get_Lights::SP_BossDemand_Tier_Get_Lights() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Get_Lights) { }
SP_BossDemand_Tier_Mine_Hits::SP_BossDemand_Tier_Mine_Hits() :								SP_BossDemand_Count(kSP_BossDemand_Tier_Mine_Hits) { }
SP_BossDemand_Tier_Nitro_Ram::SP_BossDemand_Tier_Nitro_Ram() :								SP_BossDemand_Count(kSP_BossDemand_Tier_Nitro_Ram) { }
SP_BossDemand_Tier_Multiple_Barge_Hits::SP_BossDemand_Tier_Multiple_Barge_Hits() :			SP_BossDemand_Count(kSP_BossDemand_Tier_Multiple_Barge_Hits) { }
SP_BossDemand_Event_Finish_In_Nats_BMW::SP_BossDemand_Event_Finish_In_Nats_BMW() :			SP_BossDemand_Count(kSP_BossDemand_Event_Finish_In_Nats_BMW) { }
SP_BossDemand_Tier_Shield_Mines::SP_BossDemand_Tier_Shield_Mines() :						SP_BossDemand_Count(kSP_BossDemand_Tier_Shield_Mines) { }
SP_BossDemand_Tier_3_Bolt_Hits::SP_BossDemand_Tier_3_Bolt_Hits() :							SP_BossDemand_Count(kSP_BossDemand_Tier_3_Bolt_Hits) { }
SP_BossDemand_Tier_Air_Time::SP_BossDemand_Tier_Air_Time() :								SP_BossDemand_Count(kSP_BossDemand_Tier_Air_Time) { }
SP_BossDemand_Tier_Win_With_1_Health::SP_BossDemand_Tier_Win_With_1_Health() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Win_With_1_Health) { }
SP_BossDemand_Event_Finish_Without_SorR::SP_BossDemand_Event_Finish_Without_SorR() :		SP_BossDemand_Count(kSP_BossDemand_Event_Finish_Without_SorR) { }
SP_BossDemand_Event_Wreck_And_Win::SP_BossDemand_Event_Wreck_And_Win() :					SP_BossDemand_Count(kSP_BossDemand_Event_Wreck_And_Win) { }
SP_BossDemand_Tier_Reach_Speed::SP_BossDemand_Tier_Reach_Speed() :							SP_BossDemand_Count(kSP_BossDemand_Tier_Reach_Speed) { }
SP_BossDemand_Tier_Reverse_Bolt_Hits::SP_BossDemand_Tier_Reverse_Bolt_Hits() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Reverse_Bolt_Hits) { }
SP_BossDemand_Tier_Evade_Full_Shocks::SP_BossDemand_Tier_Evade_Full_Shocks() :				SP_BossDemand_Count(kSP_BossDemand_Tier_Evade_Full_Shocks) { }
SP_BossDemand_Tier_Win_All_Events::SP_BossDemand_Tier_Win_All_Events() :					SP_BossDemand_Count(kSP_BossDemand_Tier_Win_All_Events) { }

const char* SP_GetBossDemandName(SP_BossDemandKind kind)
{
	switch(kind)
	{
		case kSP_BossDemand_Tier_Use_Powerups:				return "use-powerups";
		case kSP_BossDemand_Tier_Earn_Fans:					return "earn-fans";
		case kSP_BossDemand_Tier_Finish_In_Different_Car:	return "finish-in-different-car";
		case kSP_BossDemand_Tier_Finish_Events:				return "finish-events";
		case kSP_BossDemand_Tier_Shield_Powerups:			return "shield-powerups";
		case kSP_BossDemand_Tier_Shock_Opponents:			return "shock-opponents";
		case kSP_BossDemand_Tier_Repair_Car:				return "repair-car";
		case kSP_BossDemand_Tier_Seconds_In_First:			return "seconds-in-first";			//
		case kSP_BossDemand_Event_Time_Remaining:			return "time-remaining";
		case kSP_BossDemand_Tier_Collect_Stopwatches:		return "collect-stopwatches";
		case kSP_BossDemand_Tier_Complete_FanDemands:		return "complete-fan-demands";
		case kSP_BossDemand_Tier_Complete_FanRuns:			return "complete-fan-runs";
		case kSP_BossDemand_Tier_Complete_FanPars:			return "complete-fan-pars";
		case kSP_BossDemand_Event_Get_5X_Combo:				return "get-5x-combo";
		case kSP_BossDemand_Tier_Ram_Into_Sea:				return "ram-into-sea";
		case kSP_BossDemand_Tier_Wreck:						return "wreck";
		case kSP_BossDemand_Tier_Barge_Hits:				return "barge-hits";
		case kSP_BossDemand_Event_Get_All_Lights:			return "get-all-lights";
		case kSP_BossDemand_Tier_Barge_Off_Cliff:			return "barge-off-cliff";
		case kSP_BossDemand_Tier_Shunt_Hits:				return "shunt-hits";
		case kSP_BossDemand_Tier_Forward_Mine_Hits:			return "forward-mine-hits";
		case kSP_BossDemand_Event_Complete_FanRun:			return "complete-fan-run";
		case kSP_BossDemand_Tier_Destruction_Bonus_Hits:	return "destruction-bonus-hits";
		case kSP_BossDemand_Tier_Drift:						return "drift";
		case kSP_BossDemand_Tier_Use_Weapon_Defence:		return "use-weapon-defence";
		case kSP_BossDemand_Tier_Clean_Nitro:				return "clean-nitro";
		case kSP_BossDemand_Tier_Clean_Triple_Nitros:		return "clean-triple-nitros";		//
		case kSP_BossDemand_Tier_Get_Lights:				return "get-lights";
		case kSP_BossDemand_Tier_Mine_Hits:					return "mine-hits";
		case kSP_BossDemand_Tier_Nitro_Ram:					return "nitro-ram";
		case kSP_BossDemand_Tier_Multiple_Barge_Hits:		return "multiple-barge-hits";
		case kSP_BossDemand_Event_Finish_In_Nats_BMW:		return "finish-in-nats-bmw";
		case kSP_BossDemand_Tier_Shield_Mines:				return "shield-mines";
		case kSP_BossDemand_Tier_3_Bolt_Hits:				return "three-bolt-hits";
		case kSP_BossDemand_Tier_Air_Time:					return "air-time";
		case kSP_BossDemand_Tier_Win_With_1_Health:			return "win-with-1-health";			//
		case kSP_BossDemand_Event_Finish_Without_SorR:		return "finish-without-sorr";
		case kSP_BossDemand_Event_Wreck_And_Win:			return "Wreck-and-win";
		case kSP_BossDemand_Tier_Reach_Speed:				return "reach-speed";
		case kSP_BossDemand_Tier_Reverse_Bolt_Hits:			return "reverse-bolt-hits";
		case kSP_BossDemand_Tier_Evade_Full_Shocks:			return "evade-full-shocks";
		case kSP_BossDemand_Tier_Win_All_Events:			return "win-all-events";
	}

	return "unknown";
}

SP_BossDemand* SP_GetBossDemand(SP_SaveGame* sg, SP_Boss* boss, int index)
{
	if (!sg || !gSpTierData || !boss)
		return 0;

	if ((index < 0) || (index >= boss->demands.count))
		return 0;

	return boss->demands[index];
}

bool SP_DoesBossDemandUseIntValue(SP_SaveGame* sg, SP_BossDemand* demand)
{
	return !SP_DoesBossDemandUseFloatValue(sg, demand);
}

bool SP_DoesBossDemandUseFloatValue(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (!sg || !gSpTierData || !demand)
		return false;

	switch(demand->kind)
	{
		case kSP_BossDemand_Tier_Drift:
		case kSP_BossDemand_Tier_Air_Time:
		return true;
	}

	return false;
}

XtU32 SP_GetBossDemandTargetInt(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (!sg || !gSpTierData || !demand || !SP_DoesBossDemandUseIntValue(sg, demand))
		return 0;

	return ((SP_BossDemand_Count*)demand)->count;
}

float SP_GetBossDemandTargetFloat(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (!sg || !gSpTierData || !demand || !SP_DoesBossDemandUseFloatValue(sg, demand))
		return 0.0f;

	return (float)((SP_BossDemand_Count*)demand)->count;
}

XtU32 SP_GetBossDemandValueInt(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (!sg || !gSpTierData || !demand || !demand->boss || (demand->index >= kSP_MaxBossDemands))
		return 0;

	if (!SP_DoesBossDemandUseIntValue(sg, demand))
		return 0;

	SP_Boss* boss = demand->boss;

	if (!boss->tier)
		return 0;

	SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id);

	if (!sb)
		return 0;

	return sb->_demands[demand->index];
}

float SP_GetBossDemandValueFloat(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (!sg || !gSpTierData || !demand || !demand->boss || (demand->index >= kSP_MaxBossDemands))
		return 0.0f;

	if (!SP_DoesBossDemandUseFloatValue(sg, demand))
		return 0;

	SP_Boss* boss = demand->boss;

	if (!boss->tier)
		return 0.0f;

	SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id);

	if (!sb)
		return 0.0f;

	return *(float*)&sb->_demands[demand->index];
}

void SP_SetBossDemandValueInt(SP_SaveGame* sg, SP_BossDemand* demand, XtU32 value)
{
	if (!sg || !gSpTierData || !demand || !demand->boss || (demand->index >= kSP_MaxBossDemands))
		return;

	if (!SP_DoesBossDemandUseIntValue(sg, demand))
		return;

	SP_Boss* boss = demand->boss;

	if (!boss->tier)
		return;

	SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id);

	if (!sb)
		return;

	XtU32 target = SP_GetBossDemandTargetInt(sg, demand);

	if (value < 0)
		value = 0;
	else if (value > target)
		value = target;

	sb->_demands[demand->index] = value;
}

void SP_SetBossDemandValueFloat(SP_SaveGame* sg, SP_BossDemand* demand, float value)
{
	if (!sg || !gSpTierData || !demand || !demand->boss || (demand->index >= kSP_MaxBossDemands))
		return;

	if (!SP_DoesBossDemandUseFloatValue(sg, demand))
		return;

	SP_Boss* boss = demand->boss;

	if (!boss->tier)
		return;

	SP_SaveBoss* sb = SP_GetSaveBoss(sg, boss->id);

	if (!sb)
		return;

	float target = SP_GetBossDemandTargetFloat(sg, demand);

	if (value < 0.0f)
		value = 0.0f;
	else if (value > target)
		value = target;

	*(float*)&sb->_demands[demand->index] = value;
}

bool SP_IsBossDemandComplete(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (SP_DoesBossDemandUseFloatValue(sg, demand))
		return SP_GetBossDemandValueFloat(sg, demand) >= SP_GetBossDemandTargetFloat(sg, demand);
	else
		return SP_GetBossDemandValueInt(sg, demand) >= SP_GetBossDemandTargetInt(sg, demand);

	return false;
}

float SP_GetBossDemandPercentageComplete(SP_SaveGame* sg, SP_BossDemand* demand)
{
	if (SP_DoesBossDemandUseFloatValue(sg, demand))
		return SP_GetBossDemandValueFloat(sg, demand) / SP_GetBossDemandTargetFloat(sg, demand);
	else
		return (float)SP_GetBossDemandValueInt(sg, demand) / (float)SP_GetBossDemandTargetInt(sg, demand);

	return 0.0f;
}