#ifndef __SYSTEM_SUBSCRIBER_COMMON_H__
#define __SYSTEM_SUBSCRIBER_COMMON_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "I_LogSubscriber.h"

namespace MT
{
	class Mutex;
}

namespace Debug
{

class LogFormat;
class LogFilter;

/// default implementation of some of the common functions for subscribers
class SubscriberCommon : public I_LogSubscriber
{
public:
						/// constructor
						SubscriberCommon();
						/// destructor
	virtual				~SubscriberCommon();

						/// send a log event to this subscriber
	virtual void		Log(LogEvent *pEvent);

						/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue);

						/// log the formatted string to a device
	virtual void		LogString(const Util::C_String &string) = 0;

						/// this function is called after all parameters have been set
	virtual bool		Start();

	LogFormat&			Format();

protected:
	LogFormat	*m_pFormatter;
	LogFilter	*m_pFilter;
	MT::Mutex	*m_pMutex;
};

};


#endif
