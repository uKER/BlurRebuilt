#ifndef __SYSTEM_CONSOLE_SUBSCRIBER_H__
#define __SYSTEM_CONSOLE_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_File;
}

namespace Debug
{

class LogFormat;

/// implementation of a subscriber that save the log to a file
class ConsoleSubscriber : public SubscriberCommon
{
public:
						DECLARE_SUBSCRIBER(ConsoleSubscriber);

						ConsoleSubscriber();

	/// destructor
	virtual				~ConsoleSubscriber();

	/// send a pre-formatted log string
	virtual	void		LogString(const Util::C_String &string);

	/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue);

	/// startup
	virtual bool		Start();

private:
	bool				m_bConsole;
	bool				m_bDebugOutput;
};

};


#endif
