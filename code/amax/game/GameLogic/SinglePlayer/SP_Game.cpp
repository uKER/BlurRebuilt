#include "StdAfx.h"
#include "SinglePlayerImpl.h"

int SP_GetRank(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 1;

	return sg->_rank;
}

float SP_GetRankProgress(SP_SaveGame* sg)
{
	if (!sg || !gSpRankData)
		return 0;

	SP_Rank* currRank = SP_GetRank(sg, sg->_rank);
	if( !currRank )
		return 0;

	SP_Rank* nextRank = SP_GetRank(sg, sg->_rank + 1);
	if( !nextRank )
		return 1.0f;

	XtI64 start = currRank->fans;
	XtI64 length = nextRank->fans - start;

	if( length <= 0 )
		return 0;

	return (float)(sg->_totalFans - start) / (float)length;
}

XtU64 SP_GetFans(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	return sg->_totalFans;
}

XtU64 SP_GetFanEventFans(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	return sg->_totalFanEventFans;
}

int SP_CountTotalStars(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	int count = 0;

	for(XtI32 i = 0; i < root->tiers.count; i++)
		count += SP_CountTotalTierStars(sg, root->tiers[i]);

	return count;
}

int SP_CountMaxStars(SP_SaveGame* sg)
{
	if (!sg || !gSpTierData)
		return 0;

	SP_TierRoot* root = gSpTierData->tierRoot;

	int count = 0;

	for(XtI32 i = 0; i < root->tiers.count; i++)
		count += SP_CountMaxTierStars(sg, root->tiers[i]);

	return count;
}

void SP_GameSetLastVehicle(SP_SaveGame* sg, XtId vehicleId)
{
	if (!sg)
		return;

	sg->_lastVehicle = vehicleId;
}

XtId SP_GameGetLastVehicle(SP_SaveGame* sg)
{
	if (!sg)
		return 0;

	return sg->_lastVehicle;
}