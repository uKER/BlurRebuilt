#include "CorePch.h"
#include <Core/Core.h>
#include <Core/BizAssert.h>
#include <System/Debug/log/DebugLog.h>

#ifdef PLATFORM_PC
#include <string.h>
#endif

// Default assert handler

Debug::Logger &gAssertLog = Debug::GetLogger("mercury.system.assert");

bool DefaultBizAssertHandler(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line)
{
	LOGWARN(gAssertLog, "%s(%i): %s%s%s%s%s", file, line,
		GetBizAssertKindText(kind),
		*expr ? " : " : "",	expr,
		*msg ? " : " : "",	msg
		);

	return (kind == kBizAssert_Assert) || (kind == kBizAssert_Panic);
}

// BizAssertHandler interface

BizAssertHandler gBizAssertHandler = DefaultBizAssertHandler;

BizAssertHandler GetBizAssertHandler()
{
	return gBizAssertHandler;
}

void SetBizAssertHandler(BizAssertHandler handler)
{
	gBizAssertHandler = handler ? handler : DefaultBizAssertHandler;
}

const char* GetBizAssertKindText(BizAssertKind kind)
{
	switch(kind)
	{
		case kBizAssert_Assert:		return "assert";
		case kBizAssert_Notice:		return "notice";
		case kBizAssert_Warning:	return "warning";
		case kBizAssert_Panic:		return "panic";
	}

	return "";
}

// BizAssertReport

bool BizAssertReport(BizAssertKind kind, const char* expr, const char* file, int line, const char* msg, ...)
{
	char buf[2048];

	if (msg)
	{
		va_list va;
		va_start(va, msg);

#if defined(PLATFORM_PS3)
		vsnprintf(buf, sizeof(buf), msg, va);
#else
		_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, msg, va);
#endif

		va_end(va);
	}
	else
		*buf = '\0';

	return gBizAssertHandler(kind, expr ? expr : "", buf, file, line);
}
