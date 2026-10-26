#include "AudioPch.h"
#include "Util.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif
#endif

//void printf2( const char* msg, const char* format, ... )
//{
//	char buffer[256];
//	va_list args;
//	va_start (args, format);
//	vsprintf (buffer, format, args);
//	va_end (args);
//
//	printf ("%s:%s\n", msg, buffer);
//}

//	debug logging
#if defined( AUDIO_ENABLE_DEBUG_MSGS )

#ifndef PLATFORM_SPU
#include <system/debug/log/debuglog.h>

Debug::Logger& Audio::Core::g_Log = Debug::GetLogger("audio.engine.core");
Debug::Logger& Audio::g_Log = Debug::GetLogger("audio.engine.system");

void Audio::SetDebugLevels( const ::DebugLevel::Level core, const ::DebugLevel::Level sys )
{
	Audio::Core::g_Log.SetLevel( core );
	Audio::g_Log.SetLevel( sys );
}
#endif
#endif

#include <stdio.h>  //vsprintf
#include <stdlib.h> //vsprintf
#include <stdarg.h> //vsprintf
#include <string.h>


#ifdef AUDIO_OPTIMIZE_OFF
bool gAudioSystemOptimized = false;
#else
bool gAudioSystemOptimized = true;
#endif

////////////////////////////////////////////////////////////////////////////////
//	assert
////////////////////////////////////////////////////////////////////////////////
//void Audio::Assert( bool exp )
//{
//#ifndef PLATFORM_SPU
//	BIZ_ASSERT( exp );
//#endif
//}
//#if defined (AUDIO_USE_ASSERTS)
//void Assert(bool bExp, char* strMessage)
//{
//	if (!bExp)
//	{
//#ifdef USE_MERCURY_DEBUG
//		if (strMessage)
//			LOGERROR(g_Log, strMessage);
//		else
//			LOGERROR(g_Log, "Unknown audio assert hit!\n");
//#else // USE_MERCURY_DEBUG
//#ifndef PLATFORM_SPU
//		if (System::GetDebugErrorCallback())
//			(Audio::System::GetDebugErrorCallback())(strMessage);
//#endif
//#if defined (PLATFORM_XENON) || defined(PLATFORM_PC)
//		else
//		{
//			OutputDebugString(strMessage);
//		}Game
//#elif defined (PLATFORM_PS3)
//		else
//			printf(strMessage);
//#endif
//#endif // USE_MERCURY_DEBUG
//#if defined (USER_NBYGRAVE) || defined (USER_JONC) || defined (USER_CIANO) || defined (USER_SAMH)
//		DebugBreak();
//#endif
//	}
//}
//#endif // AUDIO_USE_ASSERTS

#ifdef AUDIO_SHOW_DEBUG_TEXT
#if !defined (PLATFORM_SPU)
Debug::Bool g_bLogAudioInfo("Audio.System.LogInfo", false);
#endif

#endif


