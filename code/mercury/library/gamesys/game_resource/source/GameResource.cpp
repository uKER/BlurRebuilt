#include "GameResourcePch.h"

#include <game_resource/GameResource.h>
#include <system/misc/Platform.h>
#include <core/utility/endian.h>

namespace GameResource
{

Debug::Logger &g_Log = Debug::GetLogger("gamesys.game_resource");


bool ValidatePlatformId( Sys::I_File* pFile )
{
	Sys::C_StreamPos beginFilePos = pFile->GetPosition();
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( *pFile );
	U32 platformFourCC;
	ar >> platformFourCC;

	U32 platformFourCCEndianSwapped;
	
	Ut::DoEndianSwap::EndianSwapD(&platformFourCCEndianSwapped, &platformFourCC);

	if(platformFourCC != Util::GetPlatformFourCC(Util::GetPlatform()))
	{
		Util::E_Platform isPlatform = Util::E_Platform_Unknown;
		for( Util::E_Platform platform=Util::E_Platform_Begin; platform<Util::E_Platform_End; platform++ )
		{			
			if( platformFourCC == Util::GetPlatformFourCC(platform))
			{
				isPlatform = platform;
				break;
			}

			if( platformFourCCEndianSwapped == Util::GetPlatformFourCC(platform))
			{
				isPlatform = platform;
				break;
			}
		}

		if(isPlatform != Util::E_Platform_Unknown)
		{
			LOGERROR( g_Log, "Trying to load resource '%s' on incorrect platform.\nThis is '%s', trying to load file for '%s'", pFile->Name().Chars(), Util::GetPlatformString(Util::GetPlatform()).Chars(), Util::GetPlatformString(isPlatform).Chars());
			return false;
		}

		// file is from before platform fourcc was implemented
		LOGWARN( g_Log, "File is out of date, please re-export '%s'", pFile->Name().Chars());

		// rewind file pointer
		pFile->SetPosition(beginFilePos);
	}

	return true;
}

} // namespace GameResource
