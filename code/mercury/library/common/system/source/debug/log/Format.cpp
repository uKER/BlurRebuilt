#include "SystemPch.h"

#include <time.h>
#include "system/debug/log/debuglog.h"
#include <system/debug/log/DebugLevel.h>
#include <system/debug/log/Format.h>
#include <system/time/time.h>
#include <system/debug/log/ThreadContext.h>

#if !defined( COMPILER_VC ) && !defined( PLATFORM_PS3 )
#include <sys/unistd.h>
#endif

namespace Debug
{

using Util::C_String;

/// set default format string
LogFormat::LogFormat()
{
	SetTimeStamp( Time::GetCurrentTime() );
	m_FormatString = "%P%c\n";
}

LogFormat::~LogFormat()
{
}

/// set string to use for formatting
void LogFormat::SetFormatString(C_String &s)
{
	m_FormatString = s;
}

// get one char
char LogFormat::Char(CLPSZ &p)
{
	return *(p++);
}

// get one attribute
LogFormat::Attribute LogFormat::GetAttr(CLPSZ &p)
{
	Attribute attr;
	p++;
	int i = 0;

	// get the number value
	C_String num = "";
	char c;
	while (1)
	{
		c = Char(p);
		if ((c>='0' && c<='9') || c=='+' || c=='-')
			num = num + C_String(c);
		else
			break;
	}
	attr.num = atoi(num);
	attr.c = c;
	return attr;	
}

/// generate date string
C_String LogFormat::DateString(U64 time)
{
	time = time / SYS_USEC_PER_SEC;
	struct tm tm = *localtime((const time_t*)&time);

	char buffer[512];
	sprintf(buffer, "%02d-%02d-%02d", tm.tm_mday, tm.tm_mon, 1900 + tm.tm_year);

    return C_String(buffer);
}

/// generate time string
C_String LogFormat::TimeString(U64 time)
{
	time = time / SYS_USEC_PER_SEC;
	struct tm tm = *localtime((const time_t*)&time);

	char buffer[512];
	sprintf(buffer, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

	return C_String(buffer);
}

/// generate seconds elapsed string
C_String LogFormat::SecsElapsedString(U64 time)
{
	S64 diff = (time - m_LastTimeStamp) / SYS_USEC_PER_SEC;
	if (diff<0)
		diff = 0; // can happen if two events get logged almost simultaneously on different threads

	char buffer[512];
	sprintf(buffer, "%d", U32(diff));

	SetTimeStamp(time);

	return C_String(buffer);
}

/// generate string from attribute info
C_String LogFormat::GetAttributeString(char c, LogEvent *pEvent)
{
	switch (c)
	{
	case 'c': case 'C':
		return pEvent->msg;
	case 'F': case 'f':
		return pEvent->location.m_FileName;
	case 'L':
		{
			char buffer[512];
			sprintf(buffer, "%d", pEvent->location.m_LineNumber);
			return C_String(buffer);
		}
	case 'l':
		return pEvent->logger;
	case 'S': case 's':
		return pEvent->location.m_FunctionName;
	case 'P':
		return g_pszDebugLevelNamesAlternate[pEvent->level];
	case 'p':
		return g_pszDebugLevelNames[pEvent->level];

	case 'd': case 'D':
		return DateString(pEvent->time);
	case 't':
		return TimeString(pEvent->time);
	case 'e':
		return SecsElapsedString(pEvent->time);

	case 'x':
		return Debug::ThreadContext::GetContext();
	case 'm':
		return pEvent->host;
	// TODO: attribute for the name of the originating logger
	default:
		return "?";	// here when we specified an invalid %attribute type
	}
}

/// generate padded string from attribute info
C_String LogFormat::FormatAttr(LogFormat::Attribute &attr, LogEvent *pEvent)
{
	C_String out = GetAttributeString(attr.c, pEvent);

	int n = abs(attr.num) - out.Length();
	if (n <= 0)
		return out;

	if (attr.num > 0)
		out = out + C_String(' ', n);
	else
		out = C_String(' ', n) + out;

	return out;
}

/// format an escape character
C_String LogFormat::FormatEscapeCharacter(CLPSZ &p)
{
	p++;
	switch (Char(p))
	{
	case 'n':	return "\n";
	// TODO: more
	default:	return "";
	}
}

/// format a log event into a string
C_String LogFormat::Format(LogEvent *pEvent)
{
	C_String out = "";

	CLPSZ psz = m_FormatString.Chars();
	while (*psz)
	{
		if (*psz == '\\')
		{
			out = out + FormatEscapeCharacter(psz);
		}
		else if (*psz == '%')
		{
			// interpret the %?
			Attribute attr = GetAttr(psz);
			out = out + FormatAttr(attr, pEvent);
		}
		else
			out = out + C_String(Char(psz));
	}

	return out;
}

/// reset time-stamp counter
void LogFormat::SetTimeStamp(U64 time)
{
	m_LastTimeStamp = time;
}

}
