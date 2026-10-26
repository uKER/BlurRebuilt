#ifndef __SYSTEM_DEBUGVIEW_SUBSCRIBER_H__
#define __SYSTEM_DEBUGVIEW_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "SubscriberCommon.h"
#include "SubscriberManager.h"

namespace Debug
{

/// a subscriber that sends messages via xt to debugview
class DebugViewSubscriber : public SubscriberCommon
{
public:
						DECLARE_SUBSCRIBER(DebugViewSubscriber);

						/// constructor
						DebugViewSubscriber();
						/// destructor
						~DebugViewSubscriber();

						/// send a pre-formatted log string
	virtual void		Log(LogEvent *pEvent);

	virtual void		LogString(const Util::C_String &string) { assert(0); }
};

};


#endif
