#include "SystemPch.h"

#if 0 // [Blur2PC][x64] was #ifdef WIN64: the blobs are C definitions (luabundle.py, gen_cmake.py) named _binary_* on every arch
#define BUILDSTAMP_START __binary_buildstamp_txt_start
#define BUILDSTAMP_END	 __binary_buildstamp_txt_end
#else
#define BUILDSTAMP_START _binary_buildstamp_txt_start
#define BUILDSTAMP_END	 _binary_buildstamp_txt_end
#endif

extern "C" char BUILDSTAMP_START[], BUILDSTAMP_END[];

namespace Debug
{

/// returns true if this EXE was built on a build server (i.e. luntbuild) or false if it was built locally
bool GetServerBuild()
{
#ifdef SERVER_BUILD
	return true;
#else
	return false;
#endif
}

// returns the nth line/word from the buildstamp string
const char *GetLine(int n)
{
	char *psz = OCSYMBOL(BUILDSTAMP_START);
	// find the start of the given line
	while (n>0)
	{
		if(psz >= OCSYMBOL(BUILDSTAMP_END))
			return "Unknown";

		if (*psz == 0xd || *psz == 0 || *psz == ' ')
		{
			*psz = 0;
			while (*psz == 0xd || *psz == 0xa || *psz == 0 || *psz == ' ')
				psz++;
			n--;
		}
		else
			psz++;
	}

	// put terminating 0 at the end of the line
	char *pszStart = psz;
	while (*psz != 0xd && *psz != 0 && *psz != ' ')
		psz++;
	*psz = 0;

	return pszStart;
}


/// returns the build time
const char *GetBuildTime()
{
	return GetLine(0);
}

/// returns the build date
const char *GetBuildDate()
{
	return GetLine(1);
}

/// returns the name of the user that performed the build
const char *GetBuildUser()
{
	return GetLine(2);
}

/// returns the name of the build pc that the image was built on
const char *GetBuildPc()
{
	return GetLine(3);
}

/// returns the name of the SVN revision of the app
const char *GetBuildRevision()
{
	// the 4th word is "Change", the 5th word is the actual build number
	return GetLine(6);
}

/// returns the name of the SVN revision of the app
const char *GetBuildVersion()
{
	// the 4th word is "Change", the 5th word is the actual build number
	return GetLine(4);
}

}
