#ifndef MERCURY_TOOLS_SHARED_VERSIONSTRINGS_H
#define MERCURY_TOOLS_SHARED_VERSIONSTRINGS_H

namespace Tools
{
	struct VersionStrings
	{
		char *szSVNVersion;
		char *szBuildDate;
		char *szBuildMachine;
	};

	extern VersionStrings g_VersionStrings;
};

#endif
