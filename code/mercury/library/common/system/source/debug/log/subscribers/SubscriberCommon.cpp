#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <system/debug/log/Subscribers/SubscriberCommon.h>
#include <system/debug/log/format.h>
#include <system/debug/log/filter.h>

// common functionality for all subscribers

namespace Debug
{

// constructor
SubscriberCommon::SubscriberCommon()
{
	m_pFormatter = new LogFormat();
	m_pFilter = new LogFilter();
	m_pMutex = new MT::Mutex;
}

// destructor
SubscriberCommon::~SubscriberCommon()
{
	delete m_pMutex;
	delete m_pFormatter;
	delete m_pFilter;
}

// send a log event to this subscriber
void SubscriberCommon::Log(LogEvent *pEvent)
{
	if (m_pFilter->Filter(pEvent))
		LogString( m_pFormatter->Format(pEvent) );
}

/// parse a configuration parameter for this subscriber
void SubscriberCommon::SetXMLParameter(const char *pName, const char *pValue)
{
	if (!stricmp(pName, "format"))
	{
		Util::C_String value(pValue);
		m_pFormatter->SetFormatString(value);
	}
	else if (!stricmp(pName, "filter"))
	{
		Util::C_String value(pValue);
		m_pFilter->SetFilterString(value);
	}
}

/// this function is called after all parameters have been set
bool SubscriberCommon::Start()
{
	return true;
}

LogFormat& SubscriberCommon::Format()
{
	return *m_pFormatter;
}

};

