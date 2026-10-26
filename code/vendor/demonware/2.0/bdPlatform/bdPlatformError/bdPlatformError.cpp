// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#define BD_LOG_LEVEL "bdPlatformError"

bdBool g_assertFalse = false;

void BD_CALL bdHandleAssert(const bdBool expr,
							const bdNChar8 *const exprString,
							const bdNChar8 *const file,
							const bdNChar8 *const function,
							const bdUInt line,
							const bdNChar8 *const format,
							...)
{
	if(!expr)
	{
		const bdUInt messageSize = 256;
		bdNChar8 message[messageSize];
		bdMemset(message, BD_NULL, messageSize);

		bdVaList args;
		bdVaStart(args, format);

		const bdUWord messageLength = bdVsnprintf(message, messageSize, format, args);
		if(messageLength >= messageSize)
		{
			BD_WARN(BD_LOG_LEVEL, "Assert Message truncated", 0);
		}

		bdVaEnd(args);	
		
		if(exprString)
		{
			bdLogMessage(BD_LOG_ERROR, "err/", BD_NULL, file, function, line, "BD_ASSERT(%s) failed. \"%s\"", exprString, message);	
		}
		else
		{
			bdLogMessage(BD_LOG_ERROR, "err/", BD_NULL, file, function, line, "BD_ASSERT failed. \"%s\"", message);	
		}

#if defined BD_THROW_ASSERTS
	    BD_BREAKPOINT();
#endif // BD_THROW_ASSERTS

	}
}
