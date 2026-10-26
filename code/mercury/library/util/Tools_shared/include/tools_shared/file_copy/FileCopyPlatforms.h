#ifndef FILECOPY_FILE_LIST_INCLUDE
#define FILECOPY_FILE_LIST_INCLUDE

#ifdef _MSCVER
#pragma once
#endif

namespace Tools
{
	extern Debug::Logger& g_log;

	namespace FileCopy
	{
		enum E_FileCopyPlatformBits
		{
			E_FileCopyPlatformBit_PC		= 0x00000001,
			E_FileCopyPlatformBit_360		= 0x00000002,
			E_FileCopyPlatformBit_PS3		= 0x00000004,
			E_FileCopyPlatformBit_All		= 0xffffffff    
		};

		INLINE const Util::E_Platform GetPlatform(const Util::C_String platform)
		{
			if (platform.Lower() == "pc")
				return Util::E_Platform_PC;
			else if (platform.Lower()(0,5) == "xenon")
				return Util::E_Platform_360;
			else if (platform.Lower() == "ps3")
				return Util::E_Platform_PS3;
			else
				LOGWARN( g_log,  "GetBitForPlatform(): Unrecognised platform %s\n", platform.Chars());
			    
			return Util::E_Platform_Unknown;
		};

		INLINE const U32 GetBitForPlatform(const Util::E_Platform platform)
		{
			switch(platform)
			{
				case Util::E_Platform_PC:
					return E_FileCopyPlatformBit_PC;
				break;
		        
				case Util::E_Platform_360:
					return E_FileCopyPlatformBit_360;
				break;

				case Util::E_Platform_PS3:
					return E_FileCopyPlatformBit_PS3;
				break;
		        
				default:
					LOGWARN( g_log,  "GetBitForPlatform(): Unrecognised platform\n");
				break;
			}    
			return 0;
		};

		INLINE const U32 GetBitForPlatform(const Util::C_String platform)
		{
			if (platform.Lower() == "pc")
				return E_FileCopyPlatformBit_PC;
			else if (platform.Lower()(0,5) == "xenon")
				return E_FileCopyPlatformBit_360;
			else if (platform.Lower() == "ps3")
				return E_FileCopyPlatformBit_PS3;
			else
				LOGWARN( g_log,  "GetBitForPlatform(): Unrecognised platform %s\n", platform.Chars());
			    
			return 0;
		};

		INLINE const bool PlatformBitSet( const U32 platformBits, const Util::E_Platform platform )
		{
			switch(platform)
			{
				case Util::E_Platform_PC:
					return(( platformBits & E_FileCopyPlatformBit_PC ) != 0);
				break;
		        
				case Util::E_Platform_360:
					return(( platformBits & E_FileCopyPlatformBit_360 ) != 0);
				break;
		        
				case Util::E_Platform_PS3:
					return(( platformBits & E_FileCopyPlatformBit_PS3 ) != 0);
				break;

				default:
					LOGWARN( g_log,  "PlatformBitSet(): Unrecognised platform\n");
				break;
			}
		    
			return false;
		}

		INLINE Util::C_String GetPlatformName(const Util::E_Platform platform)
		{
			switch(platform)
			{
				case Util::E_Platform_PC:
					return Util::C_String("PC");
				break;
		        
				case Util::E_Platform_360:
					return Util::C_String("Xenon");
				break;

				case Util::E_Platform_PS3:
					return Util::C_String("PS3");
				break;
		        
				default:
					LOGWARN( g_log,  "GetPlatformName(): Unrecognised platform\n");
				break;
			}    
			return Util::C_String("UnknownPlatform");
		};
	}
}

#endif
