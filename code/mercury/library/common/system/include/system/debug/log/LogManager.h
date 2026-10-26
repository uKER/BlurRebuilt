#ifndef __SYSTEM_LOGMANAGER_H__
#define __SYSTEM_LOGMANAGER_H__

#pragma once

namespace Debug
{

// temporary node for loggers that have no parent yet
class TempNode : public Ut::Vector<Logger*>
{
public:
	TempNode()
	{
	}
    TempNode(Logger *pLogger)
    {
        PushBack(pLogger);
    }
};

/// class which keeps track of the hierarchy of loggers
class LogManager
{
	friend class Logger;

public:
	typedef Ut::UtMap<Util::C_String, Logger*> LoggerMap;
	typedef Ut::UtMap<Util::C_String, TempNode> TempNodeMap;

						LogManager();
						~LogManager();

	/// return reference to logger with a given name
	Logger				&GetLogger(const Util::C_String &name);

	/// return singleton instance
	static LogManager	&Instance();

	/// retrieve root logger
	Logger				&GetRootLogger();

	/// delete all the subscribers on all of the loggers
	void				DeleteAllSubscribers();

	LoggerMap			&GetLoggerMap();

	TempNodeMap			&GetTempNodeMap();

protected:
	/// update the cached levels of all loggers beneath a certain node in the hierarchy
	void				UpdateCachedLogLevels(Logger *pLogger);

private:
	/// Link a list of children into this parent node
	void				UpdateChildren(Logger *pLogger, TempNode &children);

	/// Link this logger into it's parent
	void				UpdateParents(Logger *pLogger);

	// generate parent name from a given logger name
	Util::C_String		GenerateParentName(const Util::C_String &name);

	// map which stores our loggers
	LoggerMap			m_LoggerMap;
	// map which stores temporary nodes of parent loggers that haven't been created yet
	TempNodeMap			m_TempNodeMap;

	// log manager singleton instance
	static LogManager	*s_pLogManager;

	// the root logger
	Logger				*m_pRootLogger;

};

}

#endif
