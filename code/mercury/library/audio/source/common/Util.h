#ifndef _UTIL_H_
#define _UTIL_H_

#pragma once

#include "..\common\types.h"
#include <core/BizAssert.h>

//	debug logging
//#ifdef CFG_GOLD
#ifndef PLATFORM_SPU
#define AUDIO_ENABLE_DEBUG_MSGS
#endif
//#endif

//void printf2( const char* msg, const char* format, ... );

#if defined( AUDIO_ENABLE_DEBUG_MSGS )
#include <core\core.h>
#include <system\debuglog.h>

namespace Audio
{
	extern Debug::Logger &g_Log;

	namespace Core
	{
		extern Debug::Logger &g_Log;
	}

#ifndef PLATFORM_SPU
	void SetDebugLevels( const ::DebugLevel::Level core, const ::DebugLevel::Level sys );
#endif
}

#define AUDIO_CORE_DEBUG( ... )	LOGDEBUG( Audio::Core::g_Log, __VA_ARGS__ )
#define AUDIO_CORE_INFO( ... )	LOGINFO( Audio::Core::g_Log, __VA_ARGS__ )
#define AUDIO_CORE_WARN( ... )	LOGWARN( Audio::Core::g_Log, __VA_ARGS__ )
#define AUDIO_CORE_ERROR( ... )	LOGERROR( Audio::Core::g_Log, __VA_ARGS__ )
#define AUDIO_CORE_FATAL( ... )	LOGFATAL( Audio::Core::g_Log, __VA_ARGS__ )
#define AUDIO_SYS_DEBUG( ... )	LOGDEBUG( Audio::g_Log, __VA_ARGS__ )
#define AUDIO_SYS_INFO( ... )	LOGINFO( Audio::g_Log, __VA_ARGS__ )
#define AUDIO_SYS_WARN( ... )	LOGWARN( Audio::g_Log, __VA_ARGS__ )
#define AUDIO_SYS_ERROR( ... ) 	LOGERROR( Audio::g_Log, __VA_ARGS__ )
#define AUDIO_SYS_FATAL( ... )	LOGFATAL( Audio::g_Log, __VA_ARGS__ )

#else

#define AUDIO_CORE_DEBUG( ... )
#define AUDIO_CORE_INFO( ... )
#define AUDIO_CORE_DEBUG( ... )
#define AUDIO_CORE_WARN( ... )
#define AUDIO_CORE_ERROR( ... )
#define AUDIO_CORE_FATAL( ... )
#define AUDIO_SYS_DEBUG( ... )
#define AUDIO_SYS_DEBUG( ... )
#define AUDIO_SYS_WARN( ... )
#define AUDIO_SYS_ERROR( ... )
#define AUDIO_SYS_FATAL( ... )

#endif


#ifdef AUDIO_BUILD
#pragma message("################ AUDIO BUILD ################")
#if !defined(BUILD_CONFIG_SPEED) && !defined(BUILD_CONFIG_PROFILE) && !defined(BUILD_CONFIG_MASTER)
#ifndef PLATFORM_SPU
//#define AUDIO_USE_ASSERTS
//#define AUDIO_OPTIMIZE_OFF
//#define AUDIO_SHOW_DEBUG_TEXT
#endif
#endif 
//#define AUDIO_ERRORS_ARE_FATAL
#endif 

#ifdef AUDIO_BUILD
#endif

#if defined (BUILD_CONFIG_DEBUG)
//#define AUDIO_CHECK_SAMPLES
#endif // BUILD_CONFIG_DEBUG

//#define AUDIO_DISABLE_DSP

namespace Audio
{
#define Assert(expr) BIZ_ASSERT(expr)

#if defined (PLATFORM_PS3)
#define AUDIO_ALIGN(x)
#else
#define AUDIO_ALIGN(x) __declspec(align(x))
#endif

};


// SpikeAlarm.. drop it at the start of a code block, initialised with a time (in milliseconds)
// if the code takes longer than that time to run, it will give an error
#if ! defined (CFG_GOLD) && ! defined (PLATFORM_SPU)
#include "StopWatch.h"
class SpikeAlarm
{

public:
	SpikeAlarm(F32 f32TimeInMS) { m_f32TimeThreshold = f32TimeInMS / 1000.f; m_StopWatch.Start(); }
	~SpikeAlarm()
	{
		if (m_StopWatch.Time() > m_f32TimeThreshold)
		{
			AUDIO_SYS_ERROR("This bit of code took %.3f milliseconds to complete!", m_StopWatch.Time() * 1000.f);
			DebugBreak();
		}
	}

private:
	StopWatch m_StopWatch;
	F32 m_f32TimeThreshold;
};
#else
#define SpikeAlarm(X)
#endif


#endif // _UTIL_H_
