#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <system/debug/log/Subscribers/I_LogSubscriber.h>
#include <system/debug/log/Subscribers/DebugViewSubscriber.h>
#include <system/debug/log/filter.h>
#include <system/debug/log/format.h>
#include <system/debug/log/FileSystem.h>
#include <system/debug/variables.h>
#include <system/Serialise.h>
#include <system/Serialise/TextSerialise.h>

namespace Debug
{

static const char *g_pszLogLevels = "loglevels.xml";

IMPLEMENT_SUBSCRIBER(DebugViewSubscriber, "debugview");

#if !CFG_GOLD
/// write all loggers that are specifically set to a particular level out to a file, so we can restore
/// the settings next time the game is run
void WriteLogLevelsFile()
{
	Sys::I_File *pFile = g_pFileSystem->Open(g_pszLogLevels, Sys::E_FileAccess_Write);
	if (!pFile)
		return;

	Sys::TextSerialiser ar(*pFile);

	// send notification of the current level for each logger
	LogManager::LoggerMap &loggerMap = LogManager::Instance().GetLoggerMap();
	for (LogManager::LoggerMap::Iterator it = loggerMap.Begin(); it!=loggerMap.End(); ++it)
	{
		Logger *p = it->GetValue();
		DebugLevel::Level level = p->GetTrueLevel();
		if (level != DebugLevel::None)
		{
			Util::C_String tmp;
			tmp.Printf("<%s level=\"%s\">\n", p->Name().Chars(), g_pszDebugLevelNames[level]);
			ar << tmp;
		}
	}

	delete pFile;
}

/// restore logger settings that were previously written to a file using WriteLogLevelsFile
void ReadLogLevelsFile()
{
	Sys::I_File *pFile = g_pFileSystem->Open(g_pszLogLevels);
	if (!pFile)
		return;

	Sys::TextDeserialiser ar(*pFile);
	while (pFile->GetPosition() < pFile->Length())
	{
		Util::C_String line = "";
		ar >> line;
		char slogger[65];
		char slevel[9];
		sscanf(line.Chars(), "<%64s level=\"%8s\">", slogger, slevel);
		char *p = strrchr(slevel, '\"');
		if (p)
			*p = 0;
		DebugLevel::Level level = DebugLevelFromString(slevel);
		if (level != DebugLevel::None)
			Debug::GetLogger(slogger).SetLevel(level);
	}

	delete pFile;
}

// callback for debugview to set a different logging level
void DebugViewSetLevel(const char* name, Severity reportSeverity)
{
	Logger &l = GetLogger(Util::C_String(name));

	l.SetLevel((DebugLevel::Level)(((int)reportSeverity)));

	RegisterDebugLogChannel(l.Name().Chars(), (Severity)((int)l.GetTrueLevel()));

	WriteLogLevelsFile();
}
#endif

// constructor
DebugViewSubscriber::DebugViewSubscriber()
{
#if !CFG_GOLD
	ReadLogLevelsFile();

	RegisterDebugLogCallback(DebugViewSetLevel);

	// send notification of the current level for each logger
	RegisterDebugLogChannel("", (Severity)((int)GetRootLogger().GetTrueLevel()));
	LogManager::LoggerMap &loggerMap = LogManager::Instance().GetLoggerMap();
	for (LogManager::LoggerMap::Iterator it = loggerMap.Begin(); it!=loggerMap.End(); ++it)
	{
		Logger *p = it->GetValue();
		RegisterDebugLogChannel(p->Name().Chars(), (Severity)((int)p->GetTrueLevel()));
	}

	LogManager::TempNodeMap &tempNodeMap = LogManager::Instance().GetTempNodeMap();
	for (LogManager::TempNodeMap::Iterator it = tempNodeMap.Begin(); it!=tempNodeMap.End(); ++it)
	{
		RegisterDebugLogChannel(it->GetKey().Chars(), (Severity)kSeverityInherit);
	}
#endif
}

DebugViewSubscriber::~DebugViewSubscriber()
{
}

// send a log event to debugview
void DebugViewSubscriber::Log(LogEvent *pEvent)
{
	if (m_pFilter->Filter(pEvent))
	{
		Util::C_String msg = m_pFormatter->Format(pEvent);
		Util::C_String msg2;

		// remove any trailing \n
		if (msg[msg.Length()-1] == '\n')
			msg2.CopyFrom(msg.Chars(), msg.Length()-1);
		else
			msg2 = msg;

		DebugLogLn(pEvent->logger.Chars(), (Severity)((int)pEvent->level), msg2.Chars());
	}
}


};

