#ifndef __SYSTEM_DEBUGLOG_H__
#define __SYSTEM_DEBUGLOG_H__

#pragma once

namespace Debug
{
class I_LogSubscriber;
struct LocationInfo;
struct LogEvent;
class RepeaterConnection;

class Logger : public _DebugLevel
{
	friend class LogManager;

public:
	/// retrieve logger name
	const Util::C_String			&Name();

	/// send a log message
	void							Log( ::DebugLevel::Level level, const LocationInfo &location, const Util::C_String &msg);

	/// add a subscriber to this logger
	void							AddSubscriber( I_LogSubscriber *p );

	/// remove a subscriber
	void							RemoveSubscriber( I_LogSubscriber *p );

	/// delete all subscribers
	void							DeleteAllSubscribers();

	/// set the level for this logger
	void							SetLevel(const ::DebugLevel::Level level);

	/// retrieve the logger's effective level
	inline ::DebugLevel::Level		GetEffectiveLevel()
	{
		assert(m_DebugLevel);
		return m_DebugLevel;
	}

	/// retrieve the logger's true level
	inline ::DebugLevel::Level		GetTrueLevel()
	{
		return m_Level;
	}

	/// set whether this logger will propagate messages down the hierarchy (default is true)
	void							SetPropagateMessages(bool bOn);

	/// send an event structure to this logger and it's parents (internal use)
	bool							Log( LogEvent &event );
protected:
	/// constructor
									Logger(const Util::C_String &name, ::DebugLevel::Level level);
	/// destructor
									~Logger();

	/// forward a log event to this logger (internal use)
	bool							Event( LogEvent *pEvent );
	/// set this logger's parent (used by LogManager)
	void							SetParent(Logger *pParent);
	/// retrieve this logger's parent (used by LogManager)
	Logger							*GetParent();

	/// calculate the value for the cached debug level
	void							CalculateCachedDebugLevel();

private:
	::DebugLevel::Level				m_Level;

	/// this logger's parent
	Logger							*m_pParent;
	/// determines whether messages get propagated to the parent logger. default is true
	bool							m_bPropagateMessages;

	/// name
	Util::C_String					m_Name;

	/// list of subscribers for this logger
	Ut::Vector<I_LogSubscriber*>	m_Subscribers;
};

}

#endif
