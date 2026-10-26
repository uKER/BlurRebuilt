#ifndef __SYSTEM_LOG_FILE_SUBSCRIBER_H__
#define __SYSTEM_LOG_FILE_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "SubscriberCommon.h"
#include "SubscriberManager.h"

namespace Sys
{
	class I_File;
}

namespace Debug
{

class LogFormat;

/// implementation of a subscriber that save the log to a file
class LogFileSubscriber : public SubscriberCommon
{
public:
						DECLARE_SUBSCRIBER(LogFileSubscriber);

						LogFileSubscriber();
						LogFileSubscriber( const char *pFilename );

	/// destructor
	virtual				~LogFileSubscriber();

	/// send a pre-formatted log string
	virtual	void		LogString(const Util::C_String &string);

	/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue);

	/// start the subscriber
	virtual bool		Start();

private:
	Util::C_String		m_sFilename;
	Sys::I_File			*m_File;
};

};


#endif
