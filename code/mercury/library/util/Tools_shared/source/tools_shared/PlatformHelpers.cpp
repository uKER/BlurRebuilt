#include "stdafx.h"
#include "tools_shared/PlatformHelpers.h"

namespace Tools
{

bool ShouldEndianSwap( const Util::E_Platform platform )
{
	switch(platform)
	{
		case Util::E_Platform_360:
			return true;
		break;

		case Util::E_Platform_PS3:
			return true;
		break;

		case Util::E_Platform_PC:
			return false;
		break;

		case Util::E_Platform_Count:
		case Util::E_Platform_Unknown:
		break;
	}

	return true;
}

}

/*
namespace Tools
{

const char* C_PlatformHelper::GetPlatformString( const E_Platform platform )
{
	if (platform >= 0 && platform < E_Platform_Count)
		return m_pPlatformStringTable[platform];

	//Unknown!
	LOGWARN( g_log,  "C_PlatformHelper::GetPlatformString(): Unrecognised platform %s\n");
	return 0;
}

E_Platform C_PlatformHelper::GetPlatform(const char* platform)
{
	for (U32 i = 0; i < E_Platform_Count; i++)
		if (!_stricmp(m_pPlatformStringTable[i], platform))
			return (E_Platform)i;

	return E_Platform_Unknown;
}

bool C_PlatformHelper::ShouldEndianSwap( const E_Platform platform )
{
	switch(platform)
	{
        case E_Platform_360:
			return true;
        break;
        
        case E_Platform_PS3:
			return true;
        break;

		case E_Platform_PC:
			return false;
		break;

		case E_Platform_Count:
		case E_Platform_Unknown:
		break;
	}

	return true;
}

const char* C_PlatformHelper::m_pPlatformStringTable[] =
{
	"360",
	"ps3",
	"pc"
};

// prefix operator ++
Util::E_Platform operator ++( Util::E_Platform& platform, int i )
{
	platform = (Util::E_Platform)((S32)platform + 1);
	const Util::E_Platform returnVal = platform;
	return returnVal;
}

// postfix operator ++
Util::E_Platform operator ++( Util::E_Platform& platform )
{
	const Util::E_Platform returnVal = platform;
	platform = (Util::E_Platform)((S32)platform + 1);
	return returnVal;
}

E_Platform C_PlatformHelper::GetPlatform( Util::E_Platform platform )
{
	switch(platform)
	{
		case Util::E_Platform_360:	return Tools::E_Platform_360;
		case Util::E_Platform_PC:	return Tools::E_Platform_PC;
		case Util::E_Platform_PS3:	return Tools::E_Platform_PS3;
		default:					return Tools::E_Platform_Unknown;
	}
}

}
*/