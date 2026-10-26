#ifndef UIAUTOMATION_H
#define UIAUTOMATION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Automation
{
	class CommandContext;
}

namespace UIAutomation
{

bool Callback(Automation::CommandContext& commandContext);

}

#endif
