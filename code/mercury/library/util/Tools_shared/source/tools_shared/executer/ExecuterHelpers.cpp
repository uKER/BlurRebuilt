#include "stdafx.h"

#ifdef PLATFORM_PC
#include <System/DebugLog.h>
#include <System/Debug/Log/ExecuterFeedback.h>
#include <tools_shared/shim/ShimDefinitions.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <system/misc/StringUtil.h>
#include <windows.h>

namespace Tools
{

void C_ExecuterHelpers::FeedbackFatal(
	Debug::ExecuterFeedback& executerFeedback,
	const char *format, ...)
{
	char buffer[ 1024 ];

    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );

	Util::C_String copy(buffer);
	executerFeedback.AppendFeedbackText(copy);
}

void C_ExecuterHelpers::FeedbackError(
	Debug::ExecuterFeedback& executerFeedback,
	const char *format, ...)
{
	char buffer[ 1024 ];

    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );
	
	Util::C_String copy(buffer);
	executerFeedback.AppendFeedbackText(copy);
}

void C_ExecuterHelpers::FeedbackWarn(
	Debug::ExecuterFeedback& executerFeedback,
	const char *format, ...)
{
	char buffer[ 1024 ];

    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );
	
	Util::C_String copy(buffer);
	executerFeedback.AppendFeedbackText(copy);
}

void C_ExecuterHelpers::FeedbackInfo(
	Debug::ExecuterFeedback& executerFeedback,
	const char *format, ...)
{
	char buffer[ 1024 ];

    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );
	
	Util::C_String copy(buffer);
	executerFeedback.AppendFeedbackText(copy);
}

}

#endif
