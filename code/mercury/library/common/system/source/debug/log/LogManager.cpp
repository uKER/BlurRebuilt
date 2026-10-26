#include "SystemPch.h"

#include <system\debuglog.h>

///////////////////
/// linker bug: this is neccesary to stop the linker from discarding these subscribers
#include <core/threads.h>
#include "system/debug/log/debuglog.h"
#include "system/debug/log/Subscribers/logfilesubscriber.h"
#include "system/debug/log/Subscribers/consolesubscriber.h"
#include "system/debug/log/subscribers/socketsubscriber.h"
#include "system/debug/log/subscribers/e-mailsubscriber.h"
#include "system/debug/log/subscribers/socketserversubscriber.h"
#include "system/memory/MemoryTracker.h"

namespace Debug
{
	static void BugBashingBodge()
	{
	LogFileSubscriber		dummy1;
	ConsoleSubscriber		dummy2;
	SocketSubscriber		dummy3;
	SocketServerSubscriber	dummy4;
	EMailSubscriber			dummy5;
	}
}
//////////////////


namespace Debug
{

using Util::C_String;

Singleton<LogManager>	g_LogManagerSingleton;
template<> LogManager	*Singleton<LogManager>::s_pInstance = 0;

// retrieve a logger reference given a name
Logger &GetLogger(const char *pszName)
{
	MEMORY_TRACKER_PUSH("debug.logging");

	C_String name = Util::C_String(pszName).Lower();

	Logger& retLog = (name == "root" || name == "") ? GetRootLogger() : LogManager::Instance().GetLogger(name);

	MEMORY_TRACKER_POP("debug.logging");

	return retLog;
}

// retrieve rootlogger
Logger &GetRootLogger()
{
	return LogManager::Instance().GetRootLogger();
}

// return singleton
LogManager &LogManager::Instance()
{
	return Singleton<LogManager>::Instance();
}

// constructor
LogManager::LogManager()
{
	BugBashingBodge();

	m_pRootLogger = new Logger("", ::DebugLevel::Info);

	m_pRootLogger->CalculateCachedDebugLevel();	
}

/// delete all the subscribers on all of the loggers
void LogManager::DeleteAllSubscribers()
{
	// destroy all loggers
	for (LoggerMap::Iterator it = m_LoggerMap.Begin(); it!=m_LoggerMap.End(); ++it)
	{
		it->GetValue()->DeleteAllSubscribers();
	}
}

// get logger map - for iteration the loggers
LogManager::LoggerMap &LogManager::GetLoggerMap()
{
	return m_LoggerMap;
}

LogManager::TempNodeMap &LogManager::GetTempNodeMap()
{
	return m_TempNodeMap;
}


// destructor
LogManager::~LogManager()
{
	// destroy all loggers
	for (LoggerMap::Iterator it = m_LoggerMap.Begin(); it!=m_LoggerMap.End(); ++it)
		delete it->GetValue();

	delete m_pRootLogger;
}

// get logger, or create if it doesn't exist already
Logger &LogManager::GetLogger(const C_String &name)
{
	// TODO: mutex

	LoggerMap::Iterator iter = m_LoggerMap.Find(name);
	Logger *pLogger;

	if (iter != m_LoggerMap.End())
	{
		// found it
		pLogger = iter->GetValue();
	}
	else
	{
		// create new logger
		pLogger = new Logger(name, ::DebugLevel::None);

		m_LoggerMap[name] = pLogger;

		// check whether a temporary node exists for this logger
		TempNodeMap::Iterator it2 = m_TempNodeMap.Find(name);
		if (it2 != m_TempNodeMap.End())
		{
			// link previously created children into this node
			UpdateChildren(pLogger, it2->GetValue());
			// remove temporary node
			m_TempNodeMap.Erase(it2);
		}

		UpdateParents(pLogger);
	}

	pLogger->CalculateCachedDebugLevel();

	return *pLogger;
}

// link children into this logger
void LogManager::UpdateChildren(Logger *pLogger, TempNode &children)
{
	// update all children
	for (int i=0; i<children.Size(); i++)
	{
		Logger *pChild = children[i];

		// Unless this child already points to a correct (lower) parent,
		// make cat.parent point to l.parent and l.parent to cat.
		C_String name = GenerateParentName( pChild->Name() );
		if (pChild->GetParent()->Name() != name)
		{
			pLogger->SetParent( pChild->GetParent() );
			pChild->SetParent( pLogger );
		}
	}
}

// generate parent name from a given logger name
C_String LogManager::GenerateParentName(const C_String &name)
{
	S32 i = name.RevSearch('.');
	if (i == -1)
		return C_String("");
	
	return name(0,i);
}

// link this logger into it's parent
void LogManager::UpdateParents(Logger *pLogger)
{
	C_String name = pLogger->Name();
	int length = name.Length();
	bool bFoundParent = false;

	// iterate through parent node names
	while (1)
	{
		name = GenerateParentName( name );
		if (name == "")
			break;

		// find direct ancestor
		LoggerMap::Iterator it = m_LoggerMap.Find(name);
		if(it != m_LoggerMap.End())
		{
			// link into direct parent and finish
			bFoundParent = true;
			pLogger->SetParent( it->GetValue() );
			break;
		}
		else
		{
			// find direct parent temporary node
			TempNodeMap::Iterator it2 = m_TempNodeMap.Find(name);
			if (it2 != m_TempNodeMap.End())
			{
				// add to list of nodes to be linked into this parent at a later date
				it2->GetValue().PushBack(pLogger);
			}
			else
			{
				// this parent does not exist - create a tempnode for it
				TempNode node(pLogger);
				m_TempNodeMap[name] = node;
			}
		}
	}

	// If we could not find any existing parents, then link with root.
	if (!bFoundParent)
	{
		pLogger->SetParent( &GetRootLogger() );
	}
}

// retrieve root logger
Logger &LogManager::GetRootLogger()
{
	return *m_pRootLogger;
}

// update the cached levels of all loggers beneath a certain node in the hierarchy
void LogManager::UpdateCachedLogLevels(Logger *pLogger)
{
	// TODO: optimize this so it doesn't do it for every single logger
	for (LoggerMap::Iterator it = m_LoggerMap.Begin(); it!=m_LoggerMap.End(); ++it)
	{
		it->GetValue()->CalculateCachedDebugLevel();
	}
}

}
