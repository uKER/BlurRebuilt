#ifndef MAYA_SUBSCRIBER_H
#define MAYA_SUBSCRIBER_H

#ifdef _MSC_VER
#pragma once
#endif

class LogFormat;

/// implementation of a subscriber that save the log to a file
class MayaSubscriber : public Debug::SubscriberCommon
{
public:
	MayaSubscriber();

	/// destructor
	virtual				~MayaSubscriber();

	/// dummy (send a pre-formatted log string)
	virtual	void		LogString(const Util::C_String &string) {}

	virtual void		Log(Debug::LogEvent *pEvent);

	/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue) { Debug::SubscriberCommon::SetXMLParameter(pName, pValue); }

	/// startup
	virtual bool		Start();
};


#endif
