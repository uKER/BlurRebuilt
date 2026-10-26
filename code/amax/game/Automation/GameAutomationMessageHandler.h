#ifndef _GAME_AUTOMATION_MESSAGEHANDLER_H
#define _GAME_AUTOMATION_MESSAGEHANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageHandler.h>
#include "system/automation/Automation.h"

class GameAutomationMessageController : public Messaging::C_MessageHandler
{
public:

	GameAutomationMessageController( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
	virtual ~GameAutomationMessageController();

	bool MessageHandler(const Messaging::C_MessageBase *pMessage);
};

namespace GameAutomation
{

bool Callback(Automation::CommandContext& commandContext);

}

#endif

