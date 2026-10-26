#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include "system/debug/log/DebugLevel.h"
#include "system/debug/log/Filter.h"

#if !defined( COMPILER_VC ) && !defined( PLATFORM_PS3 )
#include <sys/unistd.h>
#endif

namespace Debug
{

/// set default filter string
LogFilter::LogFilter()
{
	for (int i=0; i<6; i++)
		m_bLevels[i] = true;
}

LogFilter::~LogFilter()
{
}

/// interpret a part of the filter string
void LogFilter::InterpretString(Util::C_String str)
{
	::DebugLevel::Level lvl = DebugLevelFromString(str);
	if (lvl != ::DebugLevel::None)
	{
        m_bLevels[lvl] = true;		
	}
}

/// set string to use for filtering
void LogFilter::SetFilterString(Util::C_String &s)
{
	for (int i=0; i<6; i++)
		m_bLevels[i] = false;

	// split by | character
	int iStart = 0, iEnd = 0;
	while (1)
	{
		if (s[iEnd] == 0 || s[iEnd] == '|')
		{
			InterpretString(s(iStart,iEnd));
			iStart = iEnd+1;
			if (s[iEnd] == 0)
				return;
		}
		iEnd++;
	}
}

/// returns true if the event passes the filter
bool LogFilter::Filter(LogEvent *pEvent)
{
	if (!m_bLevels[pEvent->level])
		return false;

	return true;
}

}
