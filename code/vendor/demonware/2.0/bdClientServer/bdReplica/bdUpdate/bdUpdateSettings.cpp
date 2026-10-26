// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateSettings.h>

#define BD_LOG_LEVEL "bdClientServer/updatesettings"

void BD_CALL bdCheckUpdateSettings()
{
#ifdef BD_DEBUG

	BD_COMPILE_ASSERT (BD_UPDATE_HISTORY_SIZE > 0,  BD_UPDATE_HISTORY_SIZE_must_be_greater_than_0);

	BD_COMPILE_ASSERT (BD_IS_POWER_OF_2(BD_UPDATE_HISTORY_SIZE),  BD_UPDATE_HISTORY_SIZE_is_not_a_power_of_2);

	bdUpdateSeqNum minusOne = 0;
	minusOne--;
	BD_ASSERT(minusOne > 0, "bdCheckUpdateSettings, "
		"bdUpdateSeqNum should be an unsigned type.");

	bdUpdateSeqNum maxUpdateSeqNum = minusOne;

	BD_ASSERT(BD_UPDATE_HISTORY_SIZE < maxUpdateSeqNum, "bdCheckUpdateSettings, "
		"BD_UPDATE_HISTORY_SIZE is greater or equal to the max bdUpdateSeqNum.");

	if(bdUInt(BD_UPDATE_HISTORY_SIZE) > bdUInt(maxUpdateSeqNum/2))
	{
		BD_WARN(BD_LOG_LEVEL,"BD_UPDATE_HISTORY_SIZE is greater than half the max bdUpdateSeqNum. BD_UPDATE_HISTORY_SIZE should be a small fraction on the max bdUpdateSeqNum.");
	}

#endif // BD_DEBUG
}
