#ifndef UIMESSAGESTACK_H
#define UIMESSAGESTACK_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netCore/netEnums.h"
#include "UiMessage.h"
#include <bdCore/bdContainers/bdLinkedList.h>

namespace UI
{
	// message sent from lua scripts and handled in the game
	class MessageStack
	{
		DECLARE_SINGLETON(MessageStack);
	
	public:	
		void clear();
		bool hasMessages() const;

		UI::MessageRef popMessage();
		void pushMessage(UI::MessageRef msg);
		void logOutput();

	private:
		bdLinkedList<UI::MessageRef> m_list;
	};

};
#endif //UIMESSAGESTACK_H