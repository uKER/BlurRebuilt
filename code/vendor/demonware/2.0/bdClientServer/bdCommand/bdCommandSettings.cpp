// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdCommand/bdCommandSettings.h>

#define BD_LOG_LEVEL "bdClientServer/commandsettings"

void BD_CALL bdCheckCommandSettings()
{
#ifdef BD_DEBUG

BD_COMPILE_ASSERT (BD_COMMAND_HISTORY_SIZE > 0,  BD_COMMAND_HISTORY_SIZE_must_be_greater_than_0 );

BD_COMPILE_ASSERT (BD_IS_POWER_OF_2(BD_COMMAND_HISTORY_SIZE) , BD_COMMAND_HISTORY_SIZE_is_not_a_power_of_2);

	bdCommandSeqNum minusOne = 0;
	minusOne--;
	BD_ASSERT(minusOne > 0, "bdCheckCommandSettings, "
		"bdCommandSeqNum should be an unsigned type.");

	bdCommandSeqNum maxCommandSeqNum = minusOne;
	
	BD_ASSERT(BD_COMMAND_HISTORY_SIZE < maxCommandSeqNum, "bdCheckCommandSettings, "
		"BD_COMMAND_HISTORY_SIZE is greater or equal to the max bdCommandSeqNum.");

	if(bdUInt(BD_COMMAND_HISTORY_SIZE) > bdUInt(maxCommandSeqNum/2))
	{
		BD_WARN(BD_LOG_LEVEL,"BD_COMMAND_HISTORY_SIZE is greater than half the max bdCommandSeqNum. BD_COMMAND_HISTORY_SIZE should be a small fraction of the max bdCommandSeqNum.");
	}

#endif // BD_DEBUG
}
