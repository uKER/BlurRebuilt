#ifndef AUTOMATION_MESSAGES_H
#define AUTOMATION_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include "system/automation/Automation.h"

namespace Automation
{
	enum E_AutomationMessage
	{
		E_AutomationMessage_HiBits			= FOURCC_HI20('AUTM'),
		
		E_AutomationMessage_ProblemFound,

		E_AutomationMessage_End,
		E_AutomationMessage_Begin = E_AutomationMessage_HiBits + 1, 
		E_AutomationMessage_Count = E_AutomationMessage_End - E_AutomationMessage_Begin
	};

	class C_SendProblemFoundMsg : public Messaging::C_MessageDerived<C_SendProblemFoundMsg, E_AutomationMessage_ProblemFound>
	{
	public:

		C_SendProblemFoundMsg(const ProblemString& n, const Automation::ProblemString& a) : name(n), index(-1), area(a) {}
		C_SendProblemFoundMsg(const ProblemString& n, int i, const Automation::ProblemString& a) : name(n), index(i), area(a) {}

	public:

		Util::C_String	name;
		Util::C_String	area;
		int				index;
	};

	void RegisterMessageDebugInfo();
}

#endif
