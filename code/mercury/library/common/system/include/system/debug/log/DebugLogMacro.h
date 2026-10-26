#ifndef __SYSTEM_DEBUGLOGMACRO_H__
#define __SYSTEM_DEBUGLOGMACRO_H__

#pragma once

namespace Debug
{

#ifndef GLOBAL_DEBUG_MESSAGE_LEVEL
/// use this macro to disable all messages below a certain level
/// the levels are as follows
///  0 < FATAL < 1 < ERROR < 2 < WARN < 3 < INFO < 4 < DEBUG < 5
/// TODO: change default depending on build config
#define GLOBAL_DEBUG_MESSAGE_LEVEL 5
#endif

#ifdef __LINT__
#undef GLOBAL_DEBUG_MESSAGE_LEVEL 
#define GLOBAL_DEBUG_MESSAGE_LEVEL 0
#endif

#ifndef ASSERT_DEBUG_LEVEL
/// this macro defines the debug level for asserts
#define ASSERT_DEBUG_LEVEL ::DebugLevel::Warn
#define ASSERT_DEBUG_LEVEL_INT 3
#endif

// define function location macro
#ifdef __LINT__
#define __DEBUG_FUNC__ ""
#else
#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __DEBUG_FUNC__ __FUNCSIG__
#endif
#elif defined(__GNUC__)
#define __DEBUG_FUNC__ __PRETTY_FUNCTION__
#else
#define __DEBUG_FUNC__ ""
#endif
#endif

// define full debug location macro
#ifndef NO_DEBUG_LOCATION_INFO
#define DEBUG_LOCATION			__LINE__, __FILE__, ""
#else
#define DEBUG_LOCATION			0, "", ""
#endif


/// wrapper macros that call the logger function with line/file info

/// we create a static object that contains the line information and then call the logging function on that

/// class which creates a temporary object containing location info and a call to do the logging
class _dbgMessageProxy
{
public:
	inline _dbgMessageProxy( ::DebugLevel::Level level, ::Debug::StaticLocationInfo loc )
	{
		m_Info = loc;
		m_level = level;
	}

	inline _dbgMessageProxy( ::Debug::Logger &logger, ::DebugLevel::Level level, ::Debug::StaticLocationInfo loc )
	{
		m_Info = loc;
		m_level = level;
		m_pLogger = &logger;
	}

	// format string
	void Log(Logger &logger, const char *format, ...);
	void LogL(const char *format, ...);
	static int LogS(Logger &logger, ::DebugLevel::Level level, const StaticLocationInfo &info, const char *format, ...);

	::DebugLevel::Level m_level;
	StaticLocationInfo	m_Info;
	::Debug::Logger		*m_pLogger;
};

#if 1 //set to 0 if variadic macros are not supported

// stream the message into a std::stringstream and then call the logger directly
#define _sdbgMessage(LOGGER, LEVEL, MSG) { \
if (::Debug::GetEffectiveLevel(LOGGER) >= LEVEL) { \
	std::stringstream tmp; tmp << MSG; \
	::Debug::_dbgMessageProxy::LogS( LOGGER, LEVEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), "%s", tmp.str().c_str() ); \
} }

// variadic printf style log macro
#define _dbgMessageX(LOGGER, LEVEL, ...) \
	((::Debug::GetEffectiveLevel(LOGGER) >= LEVEL)?\
		::Debug::_dbgMessageProxy::LogS( LOGGER, LEVEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), __VA_ARGS__ ):0)

/// the actual logging macros

// DEBUG
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 4
#define LOGDEBUG( LOGGER, ...)	_dbgMessageX ( LOGGER, ::DebugLevel::Debug, __VA_ARGS__ )
#define S_DEBUG(LOGGER, MSG)	_sdbgMessage( LOGGER, ::DebugLevel::Debug, MSG)
#else
#define LOGDEBUG(LOGGER, ...)	do {} while (0)
#define S_DEBUG(LOGGER, MSG)	do {} while (0)
#endif

// INFO
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 3
#define LOGINFO( LOGGER, ...)	_dbgMessageX ( LOGGER, ::DebugLevel::Info, __VA_ARGS__ )
#define S_INFO(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Info, MSG)
#else
#define LOGINFO(LOGGER, ...)	do {} while (0)
#define S_INFO(LOGGER, MSG)		do {} while (0)
#endif

// WARN
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 2
#define LOGWARN( LOGGER, ...)	_dbgMessageX ( LOGGER, ::DebugLevel::Warn, __VA_ARGS__ )
#define S_WARN(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Warn, MSG)
#else
#define LOGWARN(LOGGER, ...)	do {} while (0)
#define S_WARN(LOGGER, MSG)		do {} while (0)
#endif

// ERROR
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 1
#define LOGERROR( LOGGER, ...)	_dbgMessageX ( LOGGER, ::DebugLevel::Error, __VA_ARGS__ )
#define S_ERROR(LOGGER, MSG)	_sdbgMessage( LOGGER, ::DebugLevel::Error, MSG)
#else
#define LOGERROR(LOGGER, ...)	do {} while (0)
#define S_ERROR(LOGGER, MSG)	do {} while (0)
#endif

// FATAL
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 0
#define LOGFATAL( LOGGER, ...)	_dbgMessageX ( LOGGER, ::DebugLevel::Fatal, __VA_ARGS__ )
#define S_FATAL(LOGGER, MSG)	_sdbgMessage( LOGGER, ::DebugLevel::Fatal, MSG)
#else
#define LOGFATAL(LOGGER, ...)	do {} while (0)
#define S_FATAL(LOGGER, MSG)	do {} while (0)
#endif

// ASSERT
#if GLOBAL_DEBUG_MESSAGE_LEVEL > ASSERT_DEBUG_LEVEL_INT
#define LOGASSERT(LOGGER, X, MSG) \
	( (!(X) && (::Debug::GetEffectiveLevel(LOGGER) >= ASSERT_DEBUG_LEVEL))?\
	::Debug::_dbgMessageProxy::LogS( LOGGER, ASSERT_DEBUG_LEVEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), MSG ):0)
#else
#define LOGASSERT(LOGGER, X, MSG)	do {} while (0)
#endif

#else

// generate a temporary local class that contains the location information
// we then call the function on the local class to send the message
#define _dbgMessage(CHANNEL) \
	::Debug::_dbgMessageProxy(CHANNEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION)).Log

// same as above, but the logger is specified inside the macro
#define _dbgMessageL(LOGGER, CHANNEL) \
	::Debug::_dbgMessageProxy(LOGGER, CHANNEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION)).Log

// stream the message into a std::stringstream and then call the logger directly
#define _sdbgMessage(LOGGER, LEVEL, MSG) { \
if (::Debug::GetEffectiveLevel(LOGGER) >= LEVEL) { \
	std::stringstream tmp; tmp << MSG; \
	LOGGER.Log( LEVEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), tmp.str().c_str() ); \
} }


/// the actual logging macros

// DEBUG
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 4
#define LOGDEBUG				_dbgMessage ( ::DebugLevel::Debug )
#define S_DEBUG(LOGGER, MSG)	_sdbgMessage( LOGGER, ::DebugLevel::Debug, MSG)
#else
#define LOGDEBUG (void)			do {} while (0)
#define S_DEBUG(LOGGER, MSG)	do {} while (0)
#endif

// INFO
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 3
#define LOGINFO					_dbgMessage ( ::DebugLevel::Info )
#define S_INFO(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Info, MSG)
#else
#define LOGINFO (void)			do {} while (0)
#define S_INFO(LOGGER, MSG)		do {} while (0)
#endif

// WARN
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 2
#define LOGWARN					_dbgMessage ( ::DebugLevel::Warn )
#define S_WARN(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Warn, MSG)
#else
#define LOGWARN (void)			do {} while (0)
#define S_WARN(LOGGER, MSG)		do {} while (0)
#endif

// ERROR
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 1
#define LOGERROR					_dbgMessage ( ::DebugLevel::Error )
#define S_ERROR(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Error, MSG)
#else
#define LOGERROR (void)				do {} while (0)
#define S_ERROR(LOGGER, MSG)		do {} while (0)
#endif

// FATAL
#if GLOBAL_DEBUG_MESSAGE_LEVEL > 0
#define LOGFATAL					_dbgMessage ( ::DebugLevel::Fatal )
#define S_FATAL(LOGGER, MSG)		_sdbgMessage( LOGGER, ::DebugLevel::Fatal, MSG)
#else
#define LOGFATAL (void)				do {} while (0)
#define S_FATAL(LOGGER, MSG)		do {} while (0)
#endif

// ASSERT
#if GLOBAL_DEBUG_MESSAGE_LEVEL > ASSERT_DEBUG_LEVEL_INT
#define LOGASSERT(LOGGER, X, MSG) \
	( (!(X) && (::Debug::GetEffectiveLevel(LOGGER) >= ASSERT_DEBUG_LEVEL))?\
	::Debug::_dbgMessageProxy::LogS( LOGGER, ASSERT_DEBUG_LEVEL, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), MSG ):0)
#else
#define LOGASSERT(LOGGER, X, MSG)	do {} while (0)
#endif

#endif // VISUAL C++ 8

}

#define sLOGDEBUG(LOGGER,...) LOGDEBUG(LOGGER,__VA_ARGS__)
#define sLOGINFO(LOGGER,...) LOGINFO(LOGGER,__VA_ARGS__)
#define sLOGWARN(LOGGER,...) LOGWARN(LOGGER,__VA_ARGS__)
#define sLOGERROR(LOGGER,...) LOGERROR(LOGGER,__VA_ARGS__)
#define sLOGFATAL(LOGGER,...) LOGFATAL(LOGGER,__VA_ARGS__)

#endif

