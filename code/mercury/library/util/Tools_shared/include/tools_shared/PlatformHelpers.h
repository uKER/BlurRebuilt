#ifndef MERCURY_TOOLS_SHARED_PLATFORM_H
#define MERCURY_TOOLS_SHARED_PLATFORM_H

#ifdef _MSCVER
#pragma once
#endif

#include <system/misc/Platform.h>

namespace Tools
{
	bool ShouldEndianSwap( const Util::E_Platform platfom );
}

/*
namespace Tools
{
	enum E_Platform
	{
		E_Platform_Unknown=-1,
		E_Platform_360 = 0,
		E_Platform_PS3,
		E_Platform_PC,
		E_Platform_Count,

		E_Platform_Initial = E_Platform_360
	};

	E_Platform operator ++( E_Platform& platform ); // postfix operator ++
	E_Platform operator ++( E_Platform& platform, int i ); // prefix operator ++

	class C_PlatformHelper
	{
	public:
		static const char* GetPlatformString( const E_Platform platform );

		static E_Platform GetPlatform( const char* platform );

		static E_Platform GetPlatform( Util::E_Platform platform );

		static bool ShouldEndianSwap( const E_Platform platfom );

	private:
		static const char* m_pPlatformStringTable[E_Platform_Count];
	};
}
*/

#endif
