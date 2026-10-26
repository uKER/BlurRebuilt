#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#include "stdafx.h"
#include "UiMessageStack.h"


namespace UI
{
	IMPLEMENT_SINGLETON(MessageStack);

	MessageStack::MessageStack()
	{}

	MessageStack::~MessageStack()
	{}

	void MessageStack::clear()
	{
		m_list.clear();
	}

	bool MessageStack::hasMessages() const 
	{ 
		return m_list.isEmpty(); 
	}

	UI::MessageRef MessageStack::popMessage() 
	{ 
		if(m_list.isEmpty())
			return NULL;

		UI::MessageRef msg = m_list.getTail(); 

		if(msg.notNull())
		{
			m_list.removeTail();
		}
		return msg; 
	}
	
	void MessageStack::pushMessage(UI::MessageRef msg) 
	{ 
		m_list.addHead(msg); 
	}

};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

