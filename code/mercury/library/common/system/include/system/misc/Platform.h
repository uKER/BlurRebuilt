#ifndef SYSTEM_MISC_PLATFORM_H
#define SYSTEM_MISC_PLATFORM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{
	enum E_Platform
	{
		E_Platform_Unknown = -1,

		E_Platform_360 = 0,
		E_Platform_PS3,		
		E_Platform_PC,		

		E_Platform_Count,

		E_Platform_Begin = E_Platform_360,
		E_Platform_End = E_Platform_Count
	};

	enum E_PlatformFourCC
	{
		E_PlatformFourCC_Unknown = '??? ',
		E_PlatformFourCC_360 = '360 ',
		E_PlatformFourCC_PS3 = 'PS3 ',
		E_PlatformFourCC_PC  = 'PC  '
	};

	E_Platform operator ++( E_Platform& platform ); // postfix operator ++
	E_Platform operator ++( E_Platform& platform, int i ); // prefix operator ++

	E_Platform GetPlatform();

	Util::C_String GetPlatformString( const E_Platform platform );
	E_Platform GetPlatform( const Util::C_String& platformString );

	E_PlatformFourCC GetPlatformFourCC( const E_Platform platform );
	E_Platform GetPlatform( const E_PlatformFourCC platformFourCC );

} // namespace Util

#endif //end of SYSTEM_MISC_PLATFORM_H

