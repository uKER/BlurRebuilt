#include "SystemPch.h"
#include <core/core.h>
#include <System/Misc/Platform.h>

namespace Util
{

// prefix operator ++
E_Platform operator ++( E_Platform& platform, int i )
{
	platform = (E_Platform)((S32)platform + 1);
	const E_Platform returnVal = platform;
	return returnVal;
}

// postfix operator ++
E_Platform operator ++( E_Platform& platform )
{
	const E_Platform returnVal = platform;
	platform = (E_Platform)((S32)platform + 1);
	return returnVal;
}

E_Platform GetPlatform()
{

	#ifdef PLATFORM_PS3
	return E_Platform_PS3;
	#endif

	#ifdef PLATFORM_PC
	return E_Platform_PC;
	#endif
}

const char* g_PlatformStringLookup[] =
{
	"360",
	"ps3",
	"pc",
	"\0"
};

Util::C_String GetPlatformString( const E_Platform platform )
{
	return g_PlatformStringLookup[ platform ];
}

E_Platform GetPlatform( const Util::C_String& platformString )
{
	for(E_Platform platform = E_Platform_Begin; platform != E_Platform_End; platform++ )
	{
		if( stricmp( platformString.Chars(), g_PlatformStringLookup[platform] ) == 0 )
			return platform;
	}

	return E_Platform_Unknown;
}

E_PlatformFourCC GetPlatformFourCC( const E_Platform platform )
{
	switch( platform )
	{
		case E_Platform_360:
			return E_PlatformFourCC_360;
		break;
		
		case E_Platform_PS3:
			return E_PlatformFourCC_PS3;
		break;
		
		case E_Platform_PC:
			return E_PlatformFourCC_PC;
		break;
	}
	return E_PlatformFourCC_Unknown;
}

E_Platform GetPlatform( const E_PlatformFourCC platformFourCC )
{
	switch( platformFourCC )
	{
		case E_PlatformFourCC_360:
			return E_Platform_360;
		break;
		
		case E_PlatformFourCC_PS3:
			return E_Platform_PS3;
		break;

		case E_PlatformFourCC_PC:
			return E_Platform_PC;
		break;
	}

	return E_Platform_Unknown;
}

} // namespace Util


