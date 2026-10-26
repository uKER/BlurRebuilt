#include "stdafx.h"
#include <tools_shared/utility/Misc.h>

namespace Tools
{

Util::C_String FormatToolPathFor64bit( const Util::C_String& path )
{	
	BOOL b64Process = FALSE;
#ifdef WIN64
	//if we're a 64b app, then we can assume we're running on a 64b machine...
	b64Process = TRUE;
#else
	//if we're a 32b process, work out if we're on a 32b machine
	if (!IsWow64Process(GetCurrentProcess(), &b64Process))
	{
		//Call failed, handle error
		LOGERROR( Tools::g_log , "Failed to determine machine word length... assuming 32b\n");
		b64Process = FALSE;
	}
#endif

	if (b64Process)
	{
		LOGDEBUG( Tools::g_log , "Running 64 bit version of tools if present...\n");
		//otherwise test to see if 64b version of tool exists
		Util::C_String result = path.IReplace( "\\win32\\", "\\win64\\" );
		if (Sys::C_FileSystem::Instance().FileExists(result))
		{
			LOGDEBUG( Tools::g_log , "64 bit version of tool found at %s\n", result.Chars());
			return result;
		}
		else
		{
			LOGDEBUG( Tools::g_log , "64 bit version of tool not found");
		}
	}

	LOGDEBUG( Tools::g_log , "Running 32 bit version of tool at %s\n", path.Chars());
	//return original path		
	return path;
}

Util::C_String GetPlatformString()
{
#ifdef WIN64
	return "win64";
#else
	BOOL bWow64Process = FALSE;
	if (!IsWow64Process(GetCurrentProcess(), &bWow64Process))
	{
		LOGERROR( Tools::g_log , "Failed to determine machine word length... assuming 32b\n");
		return "win32";
	}

	if (bWow64Process)
		return "win64";

	return "win32";
#endif
}

S64 GetFileTime(const char* fileName)
{
	S64 time;
	// Obtain the modification date for the exe
	// Use this value as the build time 

	//std::cout << filename << std::endl;
	WIN32_FILE_ATTRIBUTE_DATA exeFileAttribs;
	if (!GetFileAttributesEx(fileName, GetFileExInfoStandard, &exeFileAttribs))
		return 0;
	
	time = (S64)(exeFileAttribs.ftLastWriteTime.dwHighDateTime)<<32;
	time |= exeFileAttribs.ftLastWriteTime.dwLowDateTime;
	return time;
}

S64 GetCurrentExeTime()
{
	S64 time;
	// Obtain the modification date for the exe
	// Use this value as the build time 
	CHAR filename[256];
	GetModuleFileName(0,filename,256);
	//std::cout << filename << std::endl;
	WIN32_FILE_ATTRIBUTE_DATA exeFileAttribs;
	if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &exeFileAttribs))
		return 0;
	
	time = (S64)(exeFileAttribs.ftLastWriteTime.dwHighDateTime)<<32;
	time |= exeFileAttribs.ftLastWriteTime.dwLowDateTime;
	return time;
}

Util::C_String GetCurrentExe()
{
	CHAR filename[256];
	GetModuleFileName(0,filename,256);
	return filename;
}
}

