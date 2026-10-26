#include "StdAfx.h"
#include "SinglePlayerImpl.h"

int SP_GetTierCount(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return false;

	return gSpTierData->tierRoot->tiers.count;
}

SP_Tier* SP_GetTier(SP_SaveGame* sg, int tierNo)
{
	if (!sg || !gSpTierData)
		return 0;

	if ((tierNo < 0) || (tierNo >= gSpTierData->tierRoot->tiers.count))
		return 0;

	return gSpTierData->tierRoot->tiers[tierNo];
}

SP_Tier* SP_GetTierById(SP_SaveGame* sg, XtId tierId)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	for(XtI32 i = 0; i < root->tiers.count; i++)
	{
		SP_Tier* tier = root->tiers[i];

		if (tier && (tier->id == tierId))
			return tier;
	}

	return 0;
}

int SP_CountTotalTierStars(SP_SaveGame* sg, SP_Tier* tier)
{
	if (!sg || !gSpTierData || !tier)
		return 0;

	SP_SaveTier* st = SP_GetSaveTier(sg, tier->id);

	if (!st || !st->IsUnlocked())
		return 0;

	int count = 0;

	for(int i = 0; i < tier->events.count; i++)
		count += SP_CountTotalEventStars(sg, tier->events[i]);

	return count;
}

int SP_CountMaxTierStars(SP_SaveGame* sg, SP_Tier* tier)
{
	if (!sg || !gSpTierData || !tier)
		return 0;

	int count = 0;

	for(int i = 0; i < tier->events.count; i++)
		count += SP_CountMaxEventStars(sg, tier->events[i]);

	return count;
}