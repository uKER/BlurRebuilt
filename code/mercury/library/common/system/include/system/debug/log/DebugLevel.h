#ifndef __SYSTEM_DEBUGLEVEL_H__
#define __SYSTEM_DEBUGLEVEL_H__

namespace DebugLevel
{

enum Level
{
	None	= 0,
	Fatal	= 1,
	Error	= 2,
	Warn	= 3,
	Info	= 4,
    Debug	= 5,
};

}

namespace Debug
{

extern char *g_pszDebugLevelNames[6];
extern char *g_pszDebugLevelNamesAlternate[6];

/// convert a string into a debug level
::DebugLevel::Level DebugLevelFromString(const char *pszValue);

class _DebugLevel
{
public:
	::DebugLevel::Level m_DebugLevel;
};

}

#endif
