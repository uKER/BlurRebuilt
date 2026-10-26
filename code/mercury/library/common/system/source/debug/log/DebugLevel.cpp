#include "SystemPch.h"

#include <system/debug/log/debuglevel.h>

namespace Debug
{

char *g_pszDebugLevelNames[6]=
{
	"undefined",
	"Fatal",
	"Error",
	"Warn",
	"Info",
    "Debug",
};

char *g_pszDebugLevelNamesAlternate[6]=
{
	"",
	"FATAL ERROR: ",
	"ERROR: ",
	"WARNING: ",
	"",
    "",
};

// convert a string into a debug level
::DebugLevel::Level DebugLevelFromString(const char *psz)
{
	if (!stricmp(psz, g_pszDebugLevelNames[1]))
		return ::DebugLevel::Fatal;
	if (!stricmp(psz, g_pszDebugLevelNames[2]))
		return ::DebugLevel::Error;
	if (!stricmp(psz, g_pszDebugLevelNames[3]))
		return ::DebugLevel::Warn;
	if (!stricmp(psz, g_pszDebugLevelNames[4]))
		return ::DebugLevel::Info;
	if (!stricmp(psz, g_pszDebugLevelNames[5]))
		return ::DebugLevel::Debug;
	if (!stricmp(psz, "none"))
		return ::DebugLevel::None;

	// here when an invalid debug level was specified
	return ::DebugLevel::None;
}

}
