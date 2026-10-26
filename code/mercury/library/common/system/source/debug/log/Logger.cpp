#include "SystemPch.h"

#include "system/debug/log/debuglog.h"
#include "system/time/time.h"
#include "system/network/address.h"
#include <core/threads.h>

namespace Debug
{

// constructor
Logger::Logger(const Util::C_String &name, ::DebugLevel::Level level) :
	m_Name(name),
	m_pParent(NULL),
	m_Level(level),
	m_bPropagateMessages(true)
{
	m_DebugLevel = ::DebugLevel::None;
}

// destructor
Logger::~Logger()
{
	// delete subscribers
	for (int i=0; i<m_Subscribers.Size(); i++)
		delete m_Subscribers[i];
}

/// add a subscriber to this logger
void Logger::AddSubscriber( I_LogSubscriber *p )
{
	m_Subscribers.PushBack( p );
}

/// remove a subscriber
void Logger::RemoveSubscriber( I_LogSubscriber *p)
{
	for (int i=0; i<m_Subscribers.Size(); i++)
	{
		if (m_Subscribers[i] == p)
			m_Subscribers.EraseByIndex(i);
	}
}

/// remove and delete all subscribers
void Logger::DeleteAllSubscribers()
{
	for (int i=0; i<m_Subscribers.Size(); i++)
		delete m_Subscribers[i];
	m_Subscribers.Clear();
}

/// set whether this logger will propagate messages down the hierarchy (default is true)
void Logger::SetPropagateMessages(bool bOn)
{
	m_bPropagateMessages = bOn;
}

// set this logger's parent (used by LogManager)
void Logger::SetParent(Logger *pParent)
{
	m_pParent = pParent;
}

// retrieve this logger's parent (used by LogManager)
Logger *Logger::GetParent()
{
	return m_pParent;
}

// retrieve logger name
const Util::C_String &Logger::Name()
{
	return m_Name;
}

// send a message to this logger
void Logger::Log( ::DebugLevel::Level level, const LocationInfo &location, const Util::C_String &msg)
{
	static const Util::C_String sHostName = Network::GetHostName();

	// construct a log event
	LogEvent event = {
		level,
		msg,
		location,
		Time::GetCurrentTime(),
		Name(),
		sHostName,
		MT::GetProcessID(),
	};

	// remove any trailing \n
	if (msg[msg.Length()-1] == '\n')
		event.msg.CopyFrom(msg.Chars(), msg.Length()-1);

	bool bSent = Log(event);

	// if no subscriber received this message, print it to debug output
	if (!bSent)
	{
#if defined( COMPILER_VC )
		OutputDebugString(msg.Chars());
#else
		printf("%s\n",msg.Chars());
#endif
	}	
}

// send an event structure to this logger and it's parents
bool Logger::Log(LogEvent &event)
{
	bool bSent = false;

	if (!m_bPropagateMessages)
		return Event(&event);

	// send the event to this logger and all parents
	for (Logger *pLogger = this; pLogger; pLogger = pLogger->GetParent())
		bSent = pLogger->Event(&event) || bSent;

	return bSent;
}


// receive a message
bool Logger::Event( LogEvent *pEvent )
{
	bool bSent = false;
	for (int i=0; i<m_Subscribers.Size(); i++)
	{
		m_Subscribers[i]->Log(pEvent);
		bSent = true;
	}
	return bSent;
}

// set the debug level
void Logger::SetLevel(::DebugLevel::Level level)
{
    m_Level = level;

	CalculateCachedDebugLevel();

	LogManager::Instance().UpdateCachedLogLevels(this);
}

// calculate the value for the cached debug level
void Logger::CalculateCachedDebugLevel()
{
	for (Logger *pLogger=this; pLogger; pLogger=pLogger->GetParent())
	{
		if (pLogger->m_Level != 0)
		{
			m_DebugLevel = pLogger->m_Level;
			return;
		}
	}

	assert(0); // assert here if we didn't find a debug level
}

// format string
void _dbgMessageProxy::Log(Logger &logger, const char *format, ...)
{
	if (&logger == NULL || logger.GetEffectiveLevel() < m_level)
		return;

    char buffer[ 1024 ];
    
    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );
	
	// call logger
	logger.Log(m_level, m_Info, Util::C_String(buffer));
}

// format string
void _dbgMessageProxy::LogL(const char *format, ...)
{
	if (m_pLogger == NULL || m_pLogger->GetEffectiveLevel() < m_level)
		return;

    char buffer[ 1024 ];
    
    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );

	// call logger
	m_pLogger->Log(m_level, m_Info, Util::C_String(buffer));
}

// format string
int _dbgMessageProxy::LogS(Logger &logger, ::DebugLevel::Level level, const StaticLocationInfo &info, const char *format, ...)
{
	if(logger.GetEffectiveLevel() < level)
		return 0;

    char buffer[ 1024 ];
    
    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, 1024, format, arglist );
    va_end( arglist );
	
	// call logger
	logger.Log(level, info, Util::C_String(buffer));
	return 0;
}


};

