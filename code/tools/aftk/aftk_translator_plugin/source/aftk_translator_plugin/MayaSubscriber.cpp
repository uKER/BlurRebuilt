#include "stdafx.h"

#include <system\DebugLog.h>
#include <system\Debug\Log\Subscribers\SubscriberCommon.h>
#include <system\Debug\Log\Filter.h>
#include <system\Debug\Log\Format.h>
#include "MayaSubscriber.h"



// constructor
MayaSubscriber::MayaSubscriber()
{
}

// destructor
MayaSubscriber::~MayaSubscriber()
{
}

// receive a log event
void MayaSubscriber::Log(Debug::LogEvent *pEvent)
{
	if (!m_pFilter->Filter(pEvent))
		return;

	Util::C_String msg = m_pFormatter->Format(pEvent);
	GuiDisplayOutput(pEvent->level, msg.Chars()			);
}

bool MayaSubscriber::Start()
{
	return true; 
}