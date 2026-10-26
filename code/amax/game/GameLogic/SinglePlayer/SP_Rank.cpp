#include "StdAfx.h"
#include "SinglePlayerImpl.h"

int SP_GetRankCount(SP_SaveGame* sg)
{
	if (!sg || !gSpRankData)
		return 0;

	return gSpRankData->ranks.count - 1;
}

SP_Rank* SP_GetRank(SP_SaveGame* sg, int rankNo)
{
	if (!sg || !gSpRankData)
		return 0;

	if ((rankNo < 1) || (rankNo >= gSpRankData->ranks.count))
		return 0;

	return &gSpRankData->ranks[rankNo];
}

int SP_FindRankByFans(SP_SaveGame* sg, XtI64 fans)
{
	if (!sg || !gSpRankData)
		return 0;

	for(int i = 1; i < gSpRankData->ranks.count; i++)
	{
		if (fans < gSpRankData->ranks[i].fans)
			return i - 1;
	}

	return gSpRankData->ranks.count - 1;
}