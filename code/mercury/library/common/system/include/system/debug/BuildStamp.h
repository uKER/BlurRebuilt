#ifndef __SYSTEM_DEBUG_BUILDSTAMP_H__
#define __SYSTEM_DEBUG_BUILDSTAMP_H__

// contains function to get information about when the current build was linked

namespace Debug
{

/// returns the build time
const char *GetBuildTime();

/// returns the build date
const char *GetBuildDate();

/// returns the name of the user that performed the build
const char *GetBuildUser();

/// returns the name of the build pc that the image was built on
const char *GetBuildPc();

/// returns the name of the SVN revision of the app
const char *GetBuildRevision();

/// returns true if this EXE was built on a build server (i.e. luntbuild) or false if it was built locally
bool GetServerBuild();

/// returns the version of the build (only valid if built by one of the servers)
const char *GetBuildVersion();

};

#endif
