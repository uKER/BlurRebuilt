// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#include <bdPlatform/bdPlatformLog/bdMessageProxy.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

bdMessageProxy::bdMessageProxy(const bdLogMessageType type,
							   const bdNChar8 *const file, 
							   const bdNChar8 *const function, 
							   const bdUInt line, 
							   const bdNChar8 *const baseChannel)
: m_logMessageType(type),
  m_file(file),
  m_function(function),
  m_line(line),
  m_baseChannel(baseChannel)
{
};

void bdMessageProxy::log(const bdNChar8 *const channel, 
						 const bdNChar8 *const format, 
						 ...) const
{
	const bdUInt messageSize = 256;
	bdNChar8 message[messageSize];
	bdMemset(message, BD_NULL, messageSize);

	bdVaList args;
	bdVaStart(args, format);

	bdVsnprintf(message, messageSize, format, args);

	bdVaEnd(args);

	bdLogMessage(m_logMessageType, m_baseChannel, channel, m_file, m_function, m_line, message);
}

void bdMessageProxy::handleAssert(const bdBool expr, 
								  const bdNChar8 *const format, 
								  ...) const
{
	if(!expr)
	{
		const bdUInt messageSize = 256;
		bdNChar8 message[messageSize];
		bdMemset(message, BD_NULL, messageSize);

		bdVaList args;
		bdVaStart(args, format);

		bdVsnprintf(message, messageSize, format, args);

		bdVaEnd(args);

		bdHandleAssert(expr, BD_NULL, m_file, m_function, m_line, message);
	}
}
