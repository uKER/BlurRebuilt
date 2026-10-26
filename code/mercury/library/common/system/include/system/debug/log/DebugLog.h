#ifndef __SYSTEM_DEBUGLOG_DEBUGLOG_H__
#define __SYSTEM_DEBUGLOG_DEBUGLOG_H__

#ifdef PLATFORM_SPU
#error "You are not allowed to include this file on the SPU!"
#endif

// a more extended list of debug logging system includes

// core includes
#include "core\containers\string.h"
#include "core\containers\vector.h"
#include "core\containers\sortedbinarytree.h"

// common
#include "system\debug\log\debuglevel.h"
#include "system\debug\log\LocationInfo.h"
#include "system\debug\log\DynamicLocationInfo.h"
#include "system\debug\log\LogEvent.h"
#include "system\debug\log\Subscribers\I_LogSubscriber.h"
#include "system\debug\log\ThreadContext.h"

// logger
#include "system\debug\log\Logger.h"
#include "system\debug\log\LogManager.h"
#include "system\debug\log\DebugLogMacro.h"
#include "system\debug\log\GlobalInterface.h"
#include "system\debug\log\AutoConfigure.h"

// subscribers
#include "system\Debug\Log\Singleton.h"
#include "system\debug\log\Subscribers\I_LogSubscriber.h"
#include "system\debug\log\Subscribers\SubscriberManager.h"
#include "system\debug\log\Subscribers\SubscriberCommon.h"

#endif
