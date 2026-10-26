#include "SystemPch.h"

#include "system\debug\log\DebugLog.h"
#include "system\debug\log\Subscribers\ConsoleSubscriber.h"

namespace Debug
{

IMPLEMENT_SUBSCRIBER(ConsoleSubscriber, "console");

// constructor
ConsoleSubscriber::ConsoleSubscriber()
{
	m_bConsole = true;
	m_bDebugOutput = true;
}

// destructor
ConsoleSubscriber::~ConsoleSubscriber()
{
}

// send a log event to this subscriber
void ConsoleSubscriber::LogString(const Util::C_String &string)
{
	if (m_bConsole)
		printf("%s", string.Chars());
#ifdef COMPILER_VC
	if (m_bDebugOutput)
		OutputDebugString(string);
#endif
}

/// parse a configuration parameter for this subscriber
void ConsoleSubscriber::SetXMLParameter(const char *pName, const char *pValue)
{
	if (!stricmp(pName,"mode"))
	{
		if (!stricmp(pValue, "console"))			// console only
			m_bConsole = true, m_bDebugOutput = false;
		else if (!stricmp(pValue, "debugoutput"))	// debugoutput only
			m_bConsole = false, m_bDebugOutput = true;
		else										// both
			m_bConsole = m_bDebugOutput = true;
	}

	SubscriberCommon::SetXMLParameter(pName, pValue);
}

/// start - dummy function
bool ConsoleSubscriber::Start()
{
	return true;
}

}
