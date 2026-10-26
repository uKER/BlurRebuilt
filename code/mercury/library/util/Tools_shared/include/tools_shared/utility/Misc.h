#ifndef MERCURY_TOOLS_SHARED_UTILITY_MISC_H
#define MERCURY_TOOLS_SHARED_UTILITY_MISC_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{
	Util::C_String FormatToolPathFor64bit( const Util::C_String& path );
	Util::C_String GetPlatformString();

	S64 GetCurrentExeTime();
	S64 GetFileTime(const char* fileName);
	Util::C_String GetCurrentExe();
}

#endif
