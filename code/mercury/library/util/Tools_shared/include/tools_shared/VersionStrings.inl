#include "tools_shared\VersionStrings.h"

/// this file is to be included by the client application
/// the tools_shared library will query this structure to get the applications SVN revision etc.
/// this is needed to generate the .dmp filename amongst other things
namespace Tools
{
	VersionStrings g_VersionStrings = 
	{
		"SVN Rev: <unknown>				  ",
		"Version: ABCD.EF.GH (HH:MM)",
		"BuildPC: <unknown>               ",
	};
};
