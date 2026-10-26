#ifndef __SYSTEM_LOGSUBSCRIBER_H__
#define __SYSTEM_LOGSUBSCRIBER_H__

#pragma once

namespace Debug
{
struct LogEvent;

class I_LogSubscriber
{
public:
	/// destructor
	virtual							~I_LogSubscriber() {}

	/// send a log event to this subscriber
	virtual void					Log(LogEvent *pEvent) = 0;

	/// parse a configuration parameter for this subscriber
	virtual void					SetXMLParameter(const char *pName, const char *pValue) = 0;

	/// this function is called after SetXMLParameter, but before any events are logger
	virtual bool					Start() = 0;
};

}

#endif
